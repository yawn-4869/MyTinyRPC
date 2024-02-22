#ifndef MYTINYRPC_NET_TCP_TCP_ACCEPTOR_H
#define MYTINYRPC_NET_TCP_TCP_ACCEPTOR_H

#include "tinyrpc/net/tcp/net_addr.h"

namespace MyTinyRPC {

class TcpAcceptor {
public:
    TcpAcceptor();
    ~TcpAcceptor();

private: 
    NetAddr::s_ptr m_local_addr;
};

}

#endif