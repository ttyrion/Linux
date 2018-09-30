/* pthread_cond_common -- shared code for condition variable.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <atomic.h>
#include <stdint.h>
#include <pthread.h>

/* We need 3 least-significant bits on __wrefs for something else.  */
#define __PTHREAD_COND_MAX_GROUP_SIZE ((unsigned) 1 << 29)

#if __HAVE_64B_ATOMICS == 1

static uint64_t __attribute__ ((unused))
__condvar_load_wseq_relaxed (pthread_cond_t *cond)
{
  return atomic_load_relaxed (&cond->__data.__wseq);
}

static uint64_t __attribute__ ((unused))
__condvar_fetch_add_wseq_acquire (pthread_cond_t *cond, unsigned int val)
{
  return atomic_fetch_add_acquire (&cond->__data.__wseq, val);
}

static uint64_t __attribute__ ((unused))
__condvar_fetch_xor_wseq_release (pthread_cond_t *cond, unsigned int val)
{
  return atomic_fetch_xor_release (&cond->__data.__wseq, val);
}

static uint64_t __attribute__ ((unused))
__condvar_load_g1_start_relaxed (pthread_cond_t *cond)
{
  return atomic_load_relaxed (&cond->__data.__g1_start);
}

static void __attribute__ ((unused))
__condvar_add_g1_start_relaxed (pthread_cond_t *cond, unsigned int val)
{
  atomic_store_relaxed (&cond->__data.__g1_start,
      atomic_load_relaxed (&cond->__data.__g1_start) + val);
}

#else

/* We use two 64b counters: __wseq and __g1_start.  They are monotonically
   increasing and single-writer-multiple-readers counters, so we can implement
   load, fetch-and-add, and fetch-and-xor operations even when we just have
   32b atomics.  Values we add or xor are less than or equal to 1<<31 (*),
   so we only have to make overflow-and-addition atomic wrt. to concurrent
   load operations and xor operations.  To do that, we split each counter into
   two 32b values of which we reserve the MSB of each to represent an
   overflow from the lower-order half to the higher-order half.

   In the common case, the state is (higher-order / lower-order half, and . is
   basically concatenation of the bits):
   0.h     / 0.l  = h.l

   When we add a value of x that overflows (i.e., 0.l + x == 1.L), we run the
   following steps S1-S4 (the values these represent are on the right-hand
   side):
   S1:  0.h     / 1.L == (h+1).L
   S2:  1.(h+1) / 1.L == (h+1).L
   S3:  1.(h+1) / 0.L == (h+1).L
   S4:  0.(h+1) / 0.L == (h+1).L
   If the LSB of the higher-order half is set, readers will ignore the
   overflow bit in the lower-order half.

   To get an atomic snapshot in load operations, we exploit that the
   higher-order half is monotonically increasing; if we load a value V from
   it, then read the lower-order half, and then read the higher-order half
   again and see the same value V, we know that both halves have existed in
   the sequence of values the full counter had.  This is similar to the
   validated reads in the time-based STMs in GCC's libitm (e.g.,
   method_ml_wt).

   The xor operation needs to be an atomic read-modify-write.  The write
   itself is not an issue as it affects just the lower-order half but not bits
   used in the add operation.  To make the full fetch-and-xor atomic, we
   exploit that concurrently, the value can increase by at most 1<<31 (*): The
   xor operation is only called while having acquired the lock, so not more
   than __PTHREAD_COND_MAX_GROUP_SIZE waiters can enter concurrently and thus
   increment __wseq.  Therefore, if the xor operation observes a value of
   __wseq, then the value it applies the modification to later on can be
   derived (see below).

   One benefit of this scheme is that this makes load operations
   obstruction-free because unlike if we would just lock the counter, readers
   can almost always interpret a snapshot of each halves.  Readers can be
   forced to read a new snapshot when the read is concurrent with an overflow.
   However, overflows will happen infrequently, so load operations are
   practically lock-free.

   (*) The highest value we add is __PTHREAD_COND_MAX_GROUP_SIZE << 2 to
   __g1_start (the two extra bits are for the lock in the two LSBs of
   __g1_start).  */

typedef struct
{
  unsigned int low;
  unsigned int high;
} _condvar_lohi;

static uint64_t
__condvar_fetch_add_64_relaxed (_condvar_lohi *lh, unsigned int op)
{
  /* S1. Note that this is an atomic read-modify-write so it extends the
     release sequence of release MO store at S3.  */
  unsigned int l = atomic_fetch_add_relaxed (&lh->low, op);
  unsigned int h = atomic_load_relaxed (&lh->high);
  uint64_t result = ((uint64_t) h << 31) | l;
  l += op;
  if ((l >> 31) > 0)
    {
      /* Overflow.  Need to increment higher-order half.  Note that all
	 add operations are ordered in happens-before.  */
      h++;
      /* S2. Release MO to synchronize with the loads of the higher-order half
	 in the load operation.  See __condvar_load_64_relaxed.  */
      atomic_store_release (&lh->high, h | ((unsigned int) 1 << 31));
      l ^= (unsigned int) 1 << 31;
      /* S3.  See __condvar_load_64_relaxed.  */
      atomic_store_release (&lh->low, l);
      /* S4.  Likewise.  */
      atomic_store_release (&lh->high, h);
    }
  return result;
}

static uint64_t
__condvar_load_64_relaxed (_condvar_lohi *lh)
{
  unsigned int h, l, h2;
  do
    {
      /* This load and the second one below to the same location read from the
	 stores in the overflow handling of the add operation or the
	 initializing stores (which is a simple special case because
	 initialization always completely happens before further use).
	 Because no two stores to the higher-order half write the same value,
	 the loop ensures that if we continue to use the snapshot, this load
	 and the second one read from the same store operation.  All candidate
	 store operations have release MO.
	 If we read from S2 in the first load, then we will see the value of
	 S1 on the next load (because we synchronize with S2), or a value
	 later in modification order.  We correctly ignore the lower-half's
	 overflow bit in this case.  If we read from S4, then we will see the
	 value of S3 in the next load (or a later value), which does not have
	 the overflow bit set anymore.
	  */
      h = atomic_load_acquire (&lh->high);
      /* This will read from the release sequence of S3 (i.e, either the S3
	 store or the read-modify-writes at S1 following S3 in modification
	 order).  Thus, the read synchronizes with S3, and the following load
	 of the higher-order half will read from the matching S2 (or a later
	 value).
	 Thus, if we read a lower-half value here that already overflowed and
	 belongs to an increased higher-order half value, we will see the
	 latter and h and h2 will not be equal.  */
      l = atomic_load_acquire (&lh->low);
      /* See above.  */
      h2 = atomic_load_relaxed (&lh->high);
    }
  while (h != h2);
  if (((l >> 31) > 0) && ((h >> 31) > 0))
    l ^= (unsigned int) 1 << 31;
  return ((uint64_t) (h & ~((unsigned int) 1 << 31)) << 31) + l;
}

static uint64_t __attribute__ ((unused))
__condvar_load_wseq_relaxed (pthread_cond_t *cond)
{
  return __condvar_load_64_relaxed ((_condvar_lohi *) &cond->__data.__wseq32);
}

static uint64_t __attribute__ ((unused))
__condvar_fetch_add_wseq_acquire (pthread_cond_t *cond, unsigned int val)
{
  uint64_t r = __condvar_fetch_add_64_relaxed
      ((_condvar_lohi *) &cond->__data.__wseq32, val);
  atomic_thread_fence_acquire ();
  return r;
}

static uint64_t __attribute__ ((unused))
__condvar_fetch_xor_wseq_release (pthread_cond_t *cond, unsigned int val)
{
  _condvar_lohi *lh = (_condvar_lohi *) &cond->__data.__wseq32;
  /* First, get the current value.  See __condvar_load_64_relaxed.  */
  unsigned int h, l, h2;
  do
    {
      h = atomic_load_acquire (&lh->high);
      l = atomic_load_acquire (&lh->low);
      h2 = atomic_load_relaxed (&lh->high);
    }
  while (h != h2);
  if (((l >> 31) > 0) && ((h >> 31) == 0))
    h++;
  h &= ~((unsigned int) 1 << 31);
  l &= ~((unsigned int) 1 << 31);

  /* Now modify.  Due to the coherence rules, the prior load will read a value
     earlier in modification order than the following fetch-xor.
     This uses release MO to make the full operation have release semantics
     (all other operations access the lower-order half).  */
  unsigned int l2 = atomic_fetch_xor_release (&lh->low, val)
      & ~((unsigned int) 1 << 31);
  if (l2 < l)
    /* The lower-order half overflowed in the meantime.  This happened exactly
       once due to the limit on concurrent waiters (see above).  */
    h++;
  return ((uint64_t) h << 31) + l2;
}

static uint64_t __attribute__ ((unused))
__condvar_load_g1_start_relaxed (pthread_cond_t *cond)
{
  return __condvar_load_64_relaxed
      ((_condvar_lohi *) &cond->__data.__g1_start32);
}

static void __attribute__ ((unused))
__condvar_add_g1_start_relaxed (pthread_cond_t *cond, unsigned int val)
{
  ignore_value (__condvar_fetch_add_64_relaxed
      ((_condvar_lohi *) &cond->__data.__g1_start32, val));
}

#endif  /* !__HAVE_64B_ATOMICS  */


/* The lock that signalers use.  See pthread_cond_wait_common for uses.
   The lock is our normal three-state lock: not acquired (0) / acquired (1) /
   acquired-with-futex_wake-request (2).  However, we need to preserve the
   other bits in the unsigned int used for the lock, and therefore it is a
   little more complex.  */
static void __attribute__ ((unused))
__condvar_acquire_lock (pthread_cond_t *cond, int private)
{
  unsigned int s = atomic_load_relaxed (&cond->__data.__g1_orig_size);
  while ((s & 3) == 0)
    {
      if (atomic_compare_exchange_weak_acquire (&cond->__data.__g1_orig_size,
	  &s, s | 1))
	return;
      /* TODO Spinning and back-off.  */
    }
  /* We can't change from not acquired to acquired, so try to change to
     acquired-with-futex-wake-request and do a futex wait if we cannot change
     from not acquired.  */
  while (1)
    {
      while ((s & 3) != 2)
	{
	  if (atomic_compare_exchange_weak_acquire
	      (&cond->__data.__g1_orig_size, &s, (s & ~(unsigned int) 3) | 2))
	    {
	      if ((s & 3) == 0)
		return;
	      break;
	    }
	  /* TODO Back off.  */
	}
      futex_wait_simple (&cond->__data.__g1_orig_size,
	  (s & ~(unsigned int) 3) | 2, private);
      /* Reload so we see a recent value.  */
      s = atomic_load_relaxed (&cond->__data.__g1_orig_size);
    }
}

/* See __condvar_acquire_lock.  */
static void __attribute__ ((unused))
__condvar_release_lock (pthread_cond_t *cond, int private)
{
  if ((atomic_fetch_and_release (&cond->__data.__g1_orig_size,
				 ~(unsigned int) 3) & 3)
      == 2)
    futex_wake (&cond->__data.__g1_orig_size, 1, private);
}

/* Only use this when having acquired the lock.  */
static unsigned int __attribute__ ((unused))
__condvar_get_orig_size (pthread_cond_t *cond)
{
  return atomic_load_relaxed (&cond->__data.__g1_orig_size) >> 2;
}

/* Only use this when having acquired the lock.  */
static void __attribute__ ((unused))
__condvar_set_orig_size (pthread_cond_t *cond, unsigned int size)
{
  /* We have acquired the lock, but might get one concurrent update due to a
     lock state change from acquired to acquired-with-futex_wake-request.
     The store with relaxed MO is fine because there will be no further
     changes to the lock bits nor the size, and we will subsequently release
     the lock with release MO.  */
  unsigned int s;
  s = (atomic_load_relaxed (&cond->__data.__g1_orig_size) & 3)
      | (size << 2);
  if ((atomic_exchange_relaxed (&cond->__data.__g1_orig_size, s) & 3)
      != (s & 3))
    atomic_store_relaxed (&cond->__data.__g1_orig_size, (size << 2) | 2);
}

/* Returns FUTEX_SHARED or FUTEX_PRIVATE based on the provided __wrefs
   value.  */
static int __attribute__ ((unused))
__condvar_get_private (int flags)
{
  if ((flags & __PTHREAD_COND_SHARED_MASK) == 0)
    return FUTEX_PRIVATE;
  else
    return FUTEX_SHARED;
}

// Called by __pthread_cond_signal only
/*
  1. close G1 and wait for all futex waiters to leave G1.
  2. convert G1 into a new G2 and switch group roles(the former G2 becomes new G1 
     ending at the current __wseq)
*/
static bool __attribute__ ((unused))
__condvar_quiesce_and_switch_g1 (pthread_cond_t *cond, uint64_t wseq,
    unsigned int *g1index, int private)
{
  const unsigned int maxspin = 0;
  unsigned int g1 = *g1index;

  // __g_size may hold a negative value
  // so put the expression this way avoids relying on implementation-defined behavior.
  // this works correctly for a zero-initialized condvar too
  unsigned int old_orig_size = __condvar_get_orig_size (cond);
  uint64_t old_g1_start = __condvar_load_g1_start_relaxed (cond) >> 1;
  if (((unsigned) (wseq - old_g1_start - old_orig_size) + cond->__data.__g_size[g1 ^ 1]) == 0)
  {
    // Retufn false if G2 is empty, do not switch.
    // An new empty G1(from G2) would cause a next switch again on the next signal
    return false;
  }
  
  // Now try to close and quiesce G1

  /*  keep in mind:
      1. Waiters from less recent groups than G1 are not affected, 
         because nothing will change for them apart from __g1_start getting larger.
      2. waiters (New) arriving concurrently with this switching will all go into G2 until we atomically make the switch.  
      3. Waiters existing in G2 are not affected.
      4. Waiters existing in G1 will be closed out immediately by setting "the group closed flag" in __g_signals(LSB).
         The group closed flag will wake up waiters blocking on a futex word(__g_signals) 
         and also notifies them that the group is closed. 
         As a result, these waiters will remove their group reference, allowing us to complete switching group roles.
  */

  /*   
     waiters interpret it as if __g1_start was larger than their waiter
     sequence position.  This allows us to change __g1_start after waiting
     for all existing waiters with group references to leave, which in turn
     makes recovery after stealing a signal simpler because it then can be
     skipped if __g1_start indicates that the group is closed (otherwise,
     we would have to recover always because waiters don't know how big their
     groups are).  Relaxed MO is fine.  */
  
  // First, set LSB = true( set the group closed flag): this group would be closed.
  // This tells waiters which are about to wait that they shouldn't do that anymore.
  /*
    1.This basically serves as an advance notificaton of the upcoming change to __g1_start;
      waiters interpret it as if __g1_start was larger than their waiter sequence position. 
    Note the words "upcoming" and "as if": __g1_start is not changed now.
    Why say "as if" ? See __pthread_cond_wait_common, a waiter does the same(call __condvar_dec_grefs())
    if the group closed flag was set or __g1_start is larger than its seq.

    2. That(a waiter cancel blocking by calling __condvar_dec_grefs()) 
       allows us to change __g1_start after waiting
     for all existing waiters with group references to leave, which in turn
     makes recovery after stealing a signal simpler because it then can be
     skipped if __g1_start indicates that the group is closed (otherwise,
     we would have to recover always because waiters don't know how big their
     groups are).  Relaxed MO is fine. 
  */
  atomic_fetch_or_relaxed (cond->__data.__g_signals + g1, 1);

  /* Wait until there are no group references anymore.  The fetch-or operation
     injects us into the modification order of __g_refs; 
     
     release MO ensures that waiters incrementing __g_refs after our fetch-or see the previous
     changes to __g_signals and to __g1_start that had to happen before we can
     switch this G1 and alias with an older group (we have two groups, so
     aliasing requires switching group roles twice).  Note that nobody else
     can have set the wake-request flag, so we do not have to act upon it.

     Also note that it is harmless if older waiters or waiters from this G1
     get a group reference after we have quiesced the group because it will
     remain closed for them either because of the closed flag in __g_signals
     or the later update to __g1_start.  New waiters will never arrive here
     but instead continue to go into the still current G2.  */

  // This fetch-or operation set __g_refs[g1] to 0 injecting us into the modification order of __g_refs; 
  /*"release MO" ensures:
    1. Waiters incrementing __g_refs(calling __pthread_cond_wait_common()) 
       after our fetch-or see the previous changes to __g_signals(the group closed flag was set).
    2. 
  */
  unsigned r = atomic_fetch_or_release (cond->__data.__g_refs + g1, 0);

  //spin and wait until there are no group references anymore.
  while ((r >> 1) > 0)
  {
    for (unsigned int spin = maxspin; ((r >> 1) > 0) && (spin > 0); spin--)
    {
      /* TODO Back off.  */
      r = atomic_load_relaxed (cond->__data.__g_refs + g1);
    }
    
    if ((r >> 1) > 0)
    {
      /* There is still a waiter after spinning.  Set the wake-request
        flag and block.  Relaxed MO is fine because this is just about
        this futex word.  */
      r = atomic_fetch_or_relaxed (cond->__data.__g_refs + g1, 1);

      if ((r >> 1) > 0) {
        futex_wait_simple (cond->__data.__g_refs + g1, r, private);
      }
        
      /* Reload here so we eventually see the most recent value even if we
        do not spin.   */
      r = atomic_load_relaxed (cond->__data.__g_refs + g1);
    }
  }
  /* Acquire MO so that we synchronize with the release operation that waiters
     use to decrement __g_refs and thus happen after the waiters we waited
     for.  */
  atomic_thread_fence_acquire ();

  // Update __g1_start, finish closing this group.
  /*
    keep in mind:
       old_orig_size can only be zero when we switch groups the first time 
       after a condvar was initialized(filled with 0, G2 was set to 0), 
       in which case G1 will be at index 1 and we will add a value of 1.
      
       So the value we add will never be negative!

       See above for why this takes place after waiting for quiescence of the group.

       Why?
  */

  // Relaxed MO is fine because the change comes with no additional constraints 
  // that others would have to observe.
  __condvar_add_g1_start_relaxed (cond, (old_orig_size << 1) + (g1 == 1 ? 1 : - 1));


  // Now reopen the group

  /* keep in mind:
    1. __g_signals[g1] is set to 0 which means that:
       (1)the group closed flag is cleared(thus this group is reopend now).
       (2)the number of signals that can be consumed is set to 0.

    1. The reopening of the group enables waiters to block again on the futex controlled by __g_signals.
    2. "Release MO" ensures that the updating of __g1_start(above line) is visible to
       observer threads that see no signals(and thus can block). Thus this is a new group now.

    see __pthread_cond_wait_common() for the matching acquire MO loads.
  */
  atomic_store_release (cond->__data.__g_signals + g1, 0);

  /* At this point, the old G1 is now a valid new G2 (but not in use yet).
     No old waiter can neither grab a signal nor acquire a reference without
     noticing that __g1_start is larger.
     We can now publish the group switch by flipping the G2 index in __wseq.
     Release MO so that this synchronizes with the acquire MO operation
     waiters use to obtain a position in the waiter sequence.  */
  wseq = __condvar_fetch_xor_wseq_release (cond, 1) >> 1;
  g1 ^= 1;
  *g1index ^= 1;

  // __g1_orig_size is set nowhere else

  /* These values are just observed by signalers, and thus protected by the
     lock.  */
  unsigned int orig_size = wseq - (old_g1_start + old_orig_size);
  __condvar_set_orig_size (cond, orig_size);
  /* Use and addition to not loose track of cancellations in what was
     previously G2.  */
  cond->__data.__g_size[g1] += orig_size;

  /* The new G1's size may be zero because of cancellations during its time
     as G2.  If this happens, there are no waiters that have to receive a
     signal, so we do not need to add any and return false.  */
  if (cond->__data.__g_size[g1] == 0)
    return false;

  return true;
}
