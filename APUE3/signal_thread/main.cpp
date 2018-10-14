#include <pthread.h>
#include <signal.h>
#include <iostream>

int quitflag = 0;
sigset_t mask;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* signal_thread(void* arg) {
    while(1) {
        // The sigwait() function suspends execution of the calling thread until
        // one of the signals specified in the signal set set becomes pending.
        // The function accepts the signal (removes it from the pending list of signals), and
        // returns the signal number in signo.

        int signo = 0;
        std::cout << "wait for signals." << std::endl;
        int err = sigwait(&mask, &signo);
        if (err) {
            std::cout << "sigwait error." << std::endl;
        }

        switch(signo) {
            case SIGINT: {
                std::cout << "SIGINT caught." << std::endl;
                break;
            }
            case SIGQUIT: {
                std::cout << "SIGQUIT caught." << std::endl;
                quitflag = 1;
                pthread_mutex_lock(&mutex);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
                break;
            }
            default: {
                std::cout << "unknown signal caught." << std::endl;
            }
        }
    }
}

int main(int argc, char* arg[]) {
    sigset_t oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    //The main thread would block SIGINT and SIGQUIT
    //i.e., SIGINT and SIGQUIT would be pending.
    int err = pthread_sigmask(SIG_BLOCK, &mask, &oldmask);
    if(err) {
        std::cout << "pthread_sigmask error." << std::endl;
        exit(1);
    }

    pthread_t tid;
    err = pthread_create(&tid, NULL, signal_thread, 0);
    if(err) {
        std::cout << "create thread error." << std::endl;
        exit(1);
    }

    pthread_mutex_lock(&mutex);
    while(quitflag == 0) {
        pthread_cond_wait(&cond, &mutex);
    }
    std::cout << "unblock from cond." << std::endl;
    pthread_mutex_unlock(&mutex);

    if(sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
        std::cout << "sigprocmask error." << std::endl;
    }
    std::cout << "signal mask is reset." << std::endl;

    while(1) {

    }

    return 0;
}