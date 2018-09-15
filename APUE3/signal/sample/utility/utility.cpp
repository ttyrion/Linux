#include <signal.h>
#include <iostream>
#include <stdlib.h>
#include "utility.h"

namespace utility {
	void exit_with_msg(const std::string& msg, const int exit_code) {
		std::cout << msg << std::endl;
		exit(exit_code);
	}

	namespace signal {
		sighandler signal(int signo, sighandler handler, bool auto_restart_syscall) {
			if(!handler) {  // query
				struct sigaction oact;
				if(sigaction(signo, nullptr, &oact) < 0) {
					return SIG_ERR;
				}

				return oact.sa_handler;
			}
			struct sigaction act, oact;
			act.sa_handler = handler;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;
			if(signo == SIGALRM) {
				#ifdef SA_INTERRUPT
					act.sa_flags |= SA_INTERRUPT;
			    #endif
			}
			else if(auto_restart_syscall) {
				act.sa_flags |= SA_RESTART;
			}

			if(sigaction(signo, &act, &oact) < 0) {
				return SIG_ERR;
			}

			return oact.sa_handler;
		}
	}
}
