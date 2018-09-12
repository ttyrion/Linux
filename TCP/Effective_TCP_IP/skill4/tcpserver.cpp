#include <iostream>
#include <unistd.h>
#include "Server.h"

//argv[1]: host name or ip address (optional)
//argv[2]: service name or port
int main(int argc, char **argv) {
    std::string host_name;
    std::string service_name;
    if(argc < 2) {
        std::cout << "Some args are missing." << std::endl;
        return 1;
    }
    else if(argc == 2) {
        service_name = argv[1];
    }
    else {
        host_name = argv[1];
        service_name = argv[2];
    }

    Server server(host_name);
    server.Init();
    server.Run(service_name);

    return 0;
}