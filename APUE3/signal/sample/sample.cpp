#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h> 
#include "Handler.h"
#include "utility/utility.h"

/*
	char *fgets(char *s, int size, FILE *stream);
	Note:
	fgets read into the buffer s, and stop reading when char '\n' or EOF is encountered, or (size-1) num of chars
	have been read into s.
	And at the end, fgets add a null character as a terminator(so the buf is not initialized in the uppon sample code).
	
	RETURN:
	fgets return s or NULL if error.
*/

int main(int argc, char** argv) {
	const int MAXLINE = 80;
	//fgets would add null as terminator, so buf needs not to be initialized.	
	char buf[MAXLINE];
	
	if(signal(SIGINT, &Handler::OnSignal) == SIG_ERR) {
		std::cout << "signal error!" << std::endl;
	}
	
	std::cout << "> ";
	//read from file stream "stdin"
	while (fgets(buf, MAXLINE, stdin) != NULL) {
		if (buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = 0;	
		}
		
		pid_t pid;
		if ((pid = fork()) < 0) {
			utility::exit_with_msg("fork error! Exiting now...", EXIT_FAILURE);
		}
		else if (pid == 0) {  //child process
			execlp(buf, buf, (char*)0);
			std::string msg(buf);
			msg += " cannot be executed.";
			utility::exit_with_msg(msg, EXIT_FAILURE);
		}

		//parent process
		int status = 0;
		if ((pid = waitpid(pid, &status, 0)) < 0) {
			utility::exit_with_msg("waitpid error! Exiting now...", EXIT_FAILURE);	
		}
		
		std::cout << std::endl << "> ";
	}

	utility::exit_with_msg("Exit.", EXIT_SUCCESS);
}



