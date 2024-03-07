#ifndef MYTINYRPC_NET_TCP_TCP_CONNECTION_H
#define MYTINYRPC_NET_TCP_TCP_CONNECTION_H

#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"
#include "tinyrpc/net/fd_event_pool.h"

namespace MyTinyRPC {

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosed = 3,
    Closed = 4
};

class TcpConnection {
public:

    typedef std::shared_ptr<TcpConnection> s_ptr;
    TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr);
    ~TcpConnection();

    void onRead();
    void excute();
    void onWrite();
    void shutDown(); // 服务器主动关闭

    void setState(const TcpState& state) { // 设置连接状态
        m_state = state;
    }

    const TcpState getState() {
        return m_state;
    }

private:
    void listenRead(); // 监听读事件
    void listenWrite(); // 监听写事件
    void clear(); // 清理连接断开后的事件

private:
    EventLoop* m_event_loop{ NULL }; // 所属io线程的eventloop

    NetAddr::s_ptr m_local_addr; // 本地地址
    NetAddr::s_ptr m_peer_addr; // 监听地址
    FdEvent* m_fd_event; // 绑定的监听事件
    int m_fd;

    TcpBuffer::s_ptr m_in_buffer; // 写入缓冲区
    TcpBuffer::s_ptr m_out_buffer; // 写出缓冲区

    TcpState m_state; // 连接状态
};

}


#endif