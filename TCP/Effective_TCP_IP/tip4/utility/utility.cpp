#include "utility.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>



#include <string.h>

namespace utility {
    static const char* protocol_string[] = {
        "tcp",
        "udp"
    };

    bool set_addr(const std::string& host_name, const std::string& service_name, struct sockaddr_in* saddr, ProtocolType protocol) {
        saddr->sin_family = AF_INET;
        if(!host_name.empty()) {
            struct hostent* host_info = gethostbyname(host_name.c_str());
            if(!host_info) {
                return false;
            }

            saddr->sin_addr = *((struct in_addr*)host_info->h_addr); //h_addr_list[0]
        }
        else {
            saddr->sin_addr.s_addr = htonl(INADDR_ANY);
        }

        char* endptr = nullptr;
        short port = strtol(service_name.c_str(), &endptr, 0);
        if(*endptr == '\0') {
            saddr->sin_port = htons(port);
        }
        else {
            struct servent* service_info = getservbyname(service_name.c_str(), protocol_string[protocol]);
            if(!service_info) {
                return false;
            }

            //service_info->s_port is aready in network byte order.
            saddr->sin_port = service_info->s_port;
        }

        return true;    
    }
}
