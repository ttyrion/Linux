#ifndef _TIP4_SERVER_H
#define _TIP4_SERVER_H

#include <string>

class Server {
public:
    Server(const std::string& host);
    ~Server();
    
    void Init();
    void Run(const std::string& service);

private:
    std::string host_;
};

#endif //_TIP4_SERVER_H