#include <iostream>
#include <stdlib.h>

namespace utility {
	void exit_with_msg(const std::string& msg, const int exit_code) {
		std::cout << msg << std::endl;
		exit(exit_code);
	}
}
