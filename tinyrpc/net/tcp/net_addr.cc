#include <string.h>
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {

IPNetAddr::IPNetAddr(const std::string &ip, int16_t port) : m_ip(ip), m_port(port) {
    memset(&m_addr, 0, sizeof(sockaddr_in));

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(m_port);
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
}

IPNetAddr::IPNetAddr(const std::string &addr) {
    size_t idx = addr.find_first_of(":");
    if(idx == addr.npos) {
        ERRORLOG("invalid ipv4 address %s", addr.c_str());
        return;
    }
    m_ip = addr.substr(0, idx);
    m_port = std::atoi(addr.substr(idx+1, addr.size()-idx-1).c_str());

    memset(&m_addr, 0, sizeof(sockaddr_in));

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(m_port);
    m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
}

IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr) {
    m_ip = std::string(inet_ntoa(m_addr.sin_addr));
    m_port = ntohs(m_addr.sin_port);
}

sockaddr* IPNetAddr::getSockAddr() {
    return reinterpret_cast<sockaddr*>(&m_addr);
}

socklen_t IPNetAddr::getSockLen() {
    return sizeof(m_addr);
}

int IPNetAddr::getFamily() {
    return AF_INET;
}

std::string IPNetAddr::toString(){
    std::string res;
    res = m_ip + ":" + std::to_string(m_port);
    return res;
}

bool IPNetAddr::checkValid() {
    if(m_ip.empty()) {
        return false;
    }

    if(m_port < 0 || m_port > 65536) {
        return false;
    }

    if(inet_addr(m_ip.c_str()) == INADDR_NONE) {
        return false;
    }

    return true;
}

}