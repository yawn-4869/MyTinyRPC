#ifndef MYTINYRPC_NET_TCP_TCP_ACCEPTOR_H
#define MYTINYRPC_NET_TCP_TCP_ACCEPTOR_H

#include "tinyrpc/net/tcp/net_addr.h"

namespace MyTinyRPC {

class TcpAcceptor {
public:
    typedef std::shared_ptr<TcpAcceptor> s_ptr;
    TcpAcceptor(NetAddr::s_ptr local_addr);
    ~TcpAcceptor();
    std::pair<int, NetAddr::s_ptr> accept();
    int getListenFd() {
        return m_listenfd;
    }

private: 
    NetAddr::s_ptr m_local_addr;

    int m_family{ -1 };

    int m_listenfd{ -1 };
};

}

#endif