#include "system.h"
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
	if(argc < 2) {
		std::cout << "sys need a prameter." << std::endl;
		return 1;
	}
	
	std::stringstream cmd_param;
	for(int i = 1; i < argc; ++i) {
		cmd_param << argv[i];
		cmd_param << " ";
	}
	
	return apue::system(cmd_param.str().c_str());
}