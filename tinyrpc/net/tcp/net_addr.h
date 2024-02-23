#ifndef MYTINYRPC_NET_TCP_NET_ADDR_H
#define MYTINYRPC_NET_TCP_NET_ADDR_H

#include <arpa/inet.h>
#include <string>
#include <memory>

namespace MyTinyRPC {

class NetAddr {

public: 
    typedef std::shared_ptr<NetAddr> s_ptr;

    virtual sockaddr* getSockAddr() = 0;

    virtual socklen_t getSockLen() = 0;

    virtual int getFamily() = 0;

    virtual std::string toString() = 0;

    virtual bool checkValid() = 0;
};

class IPNetAddr : public NetAddr {

public:
    typedef std::shared_ptr<IPNetAddr> s_ptr;

    IPNetAddr(const std::string &ip, int16_t port);
    
    IPNetAddr(const std::string &addr);

    IPNetAddr(sockaddr_in addr);

    sockaddr* getSockAddr();

    socklen_t getSockLen();

    int getFamily();

    std::string toString();

    bool checkValid();

private:
    std::string m_ip;

    int16_t m_port;

    sockaddr_in m_addr;

};

}

#endif