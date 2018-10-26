#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <poll.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
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

    char buf[4096] = { 0 };
    std::map<std::string, int> clients;

#if 0
    fd_set read_set;
    FD_ZERO(&read_set);
    // FD_SET(STDIN_FILENO, &read_set[0]);
    // FD_SET(fifo, &read_set[0]);

    // fd_set write_set;
    // FD_ZERO(&write_set);
    // FD_SET(fifo, &write_set);

    int max_fd = fifo;

    while(true) {
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(fifo, &read_set);

        int fds = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (fds == -1) {
            std::cout << "select failed." << std::endl;
            return 1;
        }
        else if(fds > 0) {
            // fd_sets are updated by select
            if (FD_ISSET(fifo, &read_set)) {
                memset(buf,0,sizeof(buf));
                int n = read(fifo, buf, sizeof(buf));
                if (n == 0) {
                    // The other end point of this fifo was closed
                    std::cout << "select failed." << std::endl;
                    return 0;
                }
                std::cout << "recv: " << buf << std::endl;

                std::string data(buf, strlen(buf));
                //client data format, "pid=xx"
                if (data.find("connect") == 0) {
                    // data has a '\n' at the end.
                    std::string client_pid = data.substr(8, data.length() - 8).c_str();
                    int client_fifo = open(client_pid.c_str(),O_WRONLY);
                    if (client_fifo == -1) {
                        std::cout << "open fifo of " << client_pid << "  failed. errno=" << errno << std::endl;
                    }
                    else {
                        clients[client_pid] = client_fifo;
                        max_fd = MAX(max_fd, client_fifo);
                        // max_fd = MAX(max_fd, client_fifo);
                        // FD_SET(client_fifo, &read_set[index_of_set ^ 1]);
                    }
                }
                else if (data.find("echo ") == 0) {
                    std::string client_pid = data.substr(data.find_last_of(' ') + 1);
                    std::string message = data.substr(5, data.find_last_of(' ') - 4).c_str();
                    write(clients[client_pid], message.c_str(), message.length());
                }
            }

            if (FD_ISSET(STDIN_FILENO, &read_set)) {
                memset(buf,0,sizeof(buf));
                int n = read(STDIN_FILENO, buf, sizeof(buf));
                if (n > 0 ) {
                    if(strcmp(buf, "q\n") == 0 || strcmp(buf, "quit\n") == 0) {
                        break;
                    }
                    else {
                        //std::cout << buf << std::endl;
                    }
                }
            }          
        }
    }
#endif

    pollfd pfds[2];
    
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;

    pfds[1].fd = fifo;
    pfds[1].events = POLLIN;

    while(true) {
        int readable_fds = poll(pfds, sizeof(pfds)/sizeof(pollfd), -1);
        if(readable_fds == -1) {
            std::cout << "poll failed." << std::endl;
            return 1;
        }
        else if (readable_fds > 0) {
            if(pfds[0].revents & POLLIN) {
                memset(buf,0,sizeof(buf));
                int n = read(pfds[0].fd, buf, sizeof(buf));
                if (n > 0 ) {
                    if(strcmp(buf, "q\n") == 0 || strcmp(buf, "quit\n") == 0) {
                        break;
                    }
                    else {
                        //std::cout << buf << std::endl;
                    }
                }
            }
            
            if(pfds[1].revents & POLLIN) {
                memset(buf,0,sizeof(buf));
                int n = read(fifo, buf, sizeof(buf));
                if (n == 0) {
                    // The other end point of this fifo was closed
                    std::cout << "fifo was closed by client." << std::endl;
                    continue;
                }
                std::cout << "recv: " << buf << std::endl;

                std::string data(buf, strlen(buf));
                //client data format, "pid=xx"
                if (data.find("connect") == 0) {
                    // data has a '\n' at the end.
                    std::string client_pid = data.substr(8, data.length() - 8).c_str();
                    int client_fifo = open(client_pid.c_str(),O_WRONLY);
                    if (client_fifo == -1) {
                        std::cout << "open fifo of " << client_pid << "  failed. errno=" << errno << std::endl;
                    }
                    else {
                        clients[client_pid] = client_fifo;
                    }
                }
                else if (data.find("echo ") == 0) {
                    std::string client_pid = data.substr(data.find_last_of(' ') + 1);                    
                    if(clients.find(client_pid) == clients.end()) {
                        std::cout << "Please connect to the server first. " << std::endl;                        
                        continue;
                    }

                    std::string message = data.substr(5, data.find_last_of(' ') - 4).c_str();
                    write(clients[client_pid], message.c_str(), message.length());
                }
            }
        }
    }

    close(fifo);
    unlink(serverfifo.c_str());

    for (auto var : clients) {
        close(var.second);
    }

    return 0;
}