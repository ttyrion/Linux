#ifndef _TIP4_UTILITY_H
#define _TIP4_UTILITY_H

#include <netinet/in.h>
#include <string>

namespace utility {
    enum ProtocolType {
        Protocol_TCP = 0,
        Protocol_UDP
    };

    bool set_addr(const std::string& host_name, const std::string& service_name, struct sockaddr_in* sap, ProtocolType protocol);
}

#endif //_TIP4_UTILITY_H