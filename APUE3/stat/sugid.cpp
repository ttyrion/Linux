#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "ERROR: sugid nees a parameter." << std::endl;
		return 1;
	}
	int fd = open(argv[1], O_WRONLY | O_APPEND | O_CLOEXEC);
	if (fd == -1) {
		std::cout << "ERROR: open " << argv[1] << " failed." << std::endl;
		return 1;
	}
	std::string buf = "sugid test.";
	int bytes = write(fd, buf.c_str(), buf.length());
	if (bytes < buf.length()) {
		std::cout << "ERROR: write " << argv[1] << " failed." << std::endl;
		close(fd);		
		return 1;
	}
	close(fd);
	
	return 0;
}
