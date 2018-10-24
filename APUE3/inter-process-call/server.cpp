#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <string.h>

#define MAX(a,b) \
    ((a) >= (b) ? (a) : (b))

int main(int argc, char* arg[]) {
    std::string serverfifo = "serverfifo";
    int not_exist = access(serverfifo.c_str(), F_OK);
    if (not_exist != 0) {
        int err = mkfifoat(AT_FDCWD, serverfifo.c_str(), S_IRUSR |S_IWUSR);
        if (err == -1) {
            std::cout << "mkfifoat " << serverfifo << "  failed." << std::endl;
            return 1;
        }
    }
    
    int fifo = openat(AT_FDCWD, serverfifo.c_str(),O_RDWR);
    if (fifo == -1) {
        std::cout << "openat " << serverfifo << "  failed." << std::endl;
        return 1;
    }

    fd_set read_set[2];
    FD_ZERO(&read_set[0]);
    // FD_SET(STDIN_FILENO, &read_set[0]);
    // FD_SET(fifo, &read_set[0]);

    // fd_set write_set;
    // FD_ZERO(&write_set);
    // FD_SET(fifo, &write_set);

    char buf[4096] = { 0 };
    int index_of_set = 1;
    int max_fd = fifo;
    while(true) {
        index_of_set = index_of_set ^ 1;
        FD_ZERO(&read_set[index_of_set]);
        FD_SET(STDIN_FILENO, &read_set[index_of_set]);
        FD_SET(fifo, &read_set[index_of_set]);

        memset(buf,0,sizeof(buf));

        int fds = select(max_fd + 1, &read_set[index_of_set], NULL, NULL, NULL);
        if (fds == -1) {
            std::cout << "select failed." << std::endl;
            return 1;
        }
        else if(fds > 0) {
            // fd_sets are updated by select
            if (FD_ISSET(fifo, &read_set[index_of_set])) {
                int n = read(fifo, buf, sizeof(buf));
                if (n == 0) {
                    // The other end point of this fifo was closed
                    std::cout << "select failed." << std::endl;
                    return 0;
                }
                std::cout << "recv: " << buf << std::endl;

                std::string data(buf, strlen(buf));
                //client data format, "pid=xx"
                if (data.find("pid") == 0) {
                    // data has a '\n' at the end.
                    std::string client_pid = data.substr(4, data.length() - 4 -1).c_str();
                    int client_fifo = open(client_pid.c_str(),O_WRONLY);
                    if (client_fifo == -1) {
                        std::cout << "open fifo of " << client_pid << "  failed. errno=" << errno << std::endl;
                    }
                    else {
                        max_fd = MAX(max_fd, client_fifo);
                        FD_SET(client_fifo, &read_set[index_of_set ^ 1]);
                    }
                }
            }

            if (FD_ISSET(STDIN_FILENO, &read_set[index_of_set])) {
                
                int n = read(STDIN_FILENO, buf, sizeof(buf));
                if (n > 0 ) {
                    if(strcmp(buf, "q") == 0 || strcmp(buf, "quit") == 0) {
                        break;
                    }
                    else {
                        std::cout << buf << std::endl;
                    }
                }
            }

            for(int fd = 0; fd <= max_fd; ++fd) {
                if (fd != STDIN_FILENO && fd != fifo 
                    && FD_ISSET(fd, &read_set[index_of_set])) {
                    memset(buf,0,sizeof(buf));
                    int n = read(fd, buf, sizeof(buf));
                    if (n > 0 ) {
                        write(fd, buf, strlen(buf));
                    }
                }
            }            
        }
    }

    close(fifo);
    unlink(serverfifo.c_str());

    return 0;
}