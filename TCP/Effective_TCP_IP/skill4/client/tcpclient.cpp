#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include "../utility/utility.h"

//argv[1]: host name or ip address for service
//argv[2]: service name or port for service
int main(int argc, char **argv) {
    std::string host_name;
    std::string service_name;
    if(argc < 3) {
        std::cout << "Some args are missing." << std::endl;
        return 1;
    }
    else {
        host_name = argv[1];
        service_name = argv[2];
    }

    sockaddr_in server;
    utility::set_addr(host_name,service_name, &server, utility::Protocol_TCP);
    int sock_client = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(sock_client, (sockaddr*)&server,sizeof(server)) != 0) {
        std::cout << "faild to connect to server." << std::endl;
        return 1;
    }

    char buf[256] = { 0 };
    recv(sock_client, buf, sizeof(buf), 0);
    std::cout << "received: " << buf << std::endl;
    close(sock_client);

    return 0;
}