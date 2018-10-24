#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <string.h>
#include <stdio.h>

#define MAX(a,b) \
    ((a) >= (b) ? (a) : (b))

int main(int argc, char* arg[]) {
    pid_t pid = getpid();
    std::cout << "pid=" << pid << std::endl;
    std::string client_fifo = std::to_string(pid);
    int err = mkfifoat(AT_FDCWD, client_fifo.c_str(), S_IRUSR |S_IWUSR);
    if (err == -1) {
        std::cout << "mkfifo " << client_fifo << "  failed." << std::endl;
        return 1;
    }

    int server_fifo = openat(AT_FDCWD, "serverfifo",O_RDWR);
    if (server_fifo == -1) {
        std::cout << "openat " << "serverfifo" << "  failed." << std::endl;
        return 1;
    }

    int fifo = openat(AT_FDCWD, client_fifo.c_str(),O_RDWR);
    if (fifo == -1) {
        std::cout << "openat " << client_fifo << "  failed." << std::endl;
        return 1;
    }

    std::string data = "pid=" + std::to_string(pid) + "\n";
    write(server_fifo, data.c_str(), data.length());

    fd_set read_set;
    FD_ZERO(&read_set);
    // FD_SET(STDIN_FILENO, &read_set[0]);
    // FD_SET(fifo, &read_set[0]);

    // fd_set write_set;
    // FD_ZERO(&write_set);
    // FD_SET(fifo, &write_set);

    // fd_set write_set;
    // FD_ZERO(&write_set);

    char buf[4096] = { 0 };
    int max_fd = fifo;
    while(true) {
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(fifo, &read_set);
        //FD_SET(server_fifo, &write_set);

        int fds = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (fds == -1) {
            std::cout << "select failed." << std::endl;
            return 1;
        }
        else if(fds > 0) {
            // fd_sets are updated by select
            if (FD_ISSET(fifo, &read_set)) {
                int n = read(fifo, buf, sizeof(buf));
                if (n == 0) {
                    // The other end point of this fifo was closed
                    std::cout << "select failed." << std::endl;
                    return 0;
                }
                std::cout << "recv: ";
                std::cout << buf << std::endl;
            }

            if (FD_ISSET(STDIN_FILENO, &read_set)) {
                memset(buf,0,sizeof(buf));
                int n = read(STDIN_FILENO, buf, sizeof(buf));
                if (n > 0 ) {
                    if(strcmp(buf, "q") == 0 || strcmp(buf, "quit") == 0) {
                        break;
                    }
                    else {
                        write(fifo, buf, strlen(buf));
                    }
                }
            }          
        }
    }

    close(fifo);
    unlink(client_fifo.c_str());

    close(server_fifo);
    unlink("serverfifo");

    return 0;
}