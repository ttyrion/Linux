#include <dirent.h>
#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h> 
#include "utility/utility.h"

int main(int argc, char *argv[]) {
	DIR *dp;
	struct dirent *dirp;
	if (argc < 2) {
		utility::exit_with_msg("ls needs an parameter.", EXIT_FAILURE);
	}
	
	if( (dp = opendir(argv[1])) == NULL) {
		std::stringstream buffer;
		buffer << "ls cannot open " << argv[1];
		utility::exit_with_msg(buffer.str(), EXIT_FAILURE);
	}

	while((dirp = readdir(dp)) != NULL) {
		std::cout << dirp->d_name << std::endl;
	}
	closedir(dp);

	utility::exit_with_msg("", EXIT_SUCCESS);
}
