#ifndef __APUE3_LS_UTILITY_UTILITY_H
#define __APUE3_LS_UTILITY_UTILITY_H

#include <iostream>
#include <unistd.h>

namespace utility {
	void exit_with_msg(const std::string& msg, const int exit_code);

	namespace signal {
		typedef __sighandler_t sighandler;
		sighandler signal(int signo, sighandler handler, bool auto_restart_syscall = false);
	}
}

#endif
