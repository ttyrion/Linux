#include "Server.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "utility/utility.h"

Server::Server(const std::string& host)
:host_(host) {
    
}

Server::~Server() {

}

void Server::Init() {
    
}

void Server::Run(const std::string& service) {
    struct sockaddr_in local;
    if(!utility::set_addr(host_, service, &local, utility::Protocol_TCP)) {
        std::cout << "[Server] set address with " << host_ << ":" << service << " failed." << std::endl;
        return;
    }

    int listening_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(listening_sock <= 0) {
        std::cout << "[Server] create socket failed." << std::endl;
        return;
    }

    const int on = 1;
    if(setsockopt(listening_sock, SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) != 0) {
        std::cout << "[Server] set socket option failed." << std::endl;
        return;
    }

    if(bind(listening_sock, (sockaddr*)&local, sizeof(local)) != 0) {
        std::cout << "[Server] bind failed." << std::endl;
        return;
    }

    if(listen(listening_sock, 10) != 0) {
        std::cout << "[Server] listen failed." << std::endl;
        return;
    }

    for(;;) {
        sockaddr_in peer;
        socklen_t peer_addr_length = sizeof(peer);
        int peer_sock = accept(listening_sock, (sockaddr*)&peer, &peer_addr_length);
        std::string buf("Hello, welcome here.");
        send(peer_sock, buf.c_str(), buf.length(), 0);
        close(peer_sock);
    }
}