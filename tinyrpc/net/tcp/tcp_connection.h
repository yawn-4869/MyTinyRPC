#ifndef MYTINYRPC_NET_TCP_TCP_CONNECTION
#define MYTINYRPC_NET_TCP_TCP_CONNECTION

#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"

namespace MyTinyRPC {

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosed = 3,
    Closed = 4
};

class TcpConnection {
public:
    TcpConnection();
    ~TcpConnection();

private:
    EventLoop* m_event_loop{ NULL }; // 所属io线程的eventloop

    NetAddr::s_ptr m_local_addr; // 本地地址
    NetAddr::s_ptr m_peer_addr; // 监听地址

    TcpBuffer::s_ptr m_in_buffer; // 写入缓冲区
    TcpBuffer::s_ptr m_out_buffer; // 写出缓冲区

    TcpState m_state; // 连接状态


};

}


#endif