#ifndef __APUE3_LS_UTILITY_UTILITY_H
#define __APUE3_LS_UTILITY_UTILITY_H

#include <iostream>
#include <unistd.h>

namespace utility {
	void exit_with_msg(const std::string& msg, const int exit_code);
}

#endif
