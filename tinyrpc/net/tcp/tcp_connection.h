#ifndef MYTINYRPC_NET_TCP_TCP_CONNECTION_H
#define MYTINYRPC_NET_TCP_TCP_CONNECTION_H

#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"
#include "tinyrpc/net/fd_event_pool.h"
#include "tinyrpc/net/coder/abstract_protocol.h"
#include "tinyrpc/net/coder/string_coder.h"
#include "tinyrpc/net/rpc/rpc_dispatcher.h"

namespace MyTinyRPC {

enum TcpState {
    NotConnected = 1,
    Connected = 2,
    HalfClosed = 3,
    Closed = 4
};

enum TcpConnectionType {
    TcpConnectionByServer = 1, // 作为服务端使用, 代表对客户端的连接
    TcpConnectionByClient = 2  // 作为客户端使用, 代表对服务端的连接
};

class TcpConnection {
public:

    typedef std::shared_ptr<TcpConnection> s_ptr;
    TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, 
    TcpConnectionType connection_type);
    ~TcpConnection();

    void onRead();
    void excute();
    void onWrite();
    void shutDown(); // 服务器主动关闭
    void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);
    void pushReadMessage(const std::string& message, std::function<void(AbstractProtocol::s_ptr)> done);

    void setState(const TcpState& state) { // 设置连接状态
        m_state = state;
    }
    const TcpState getState() {
        return m_state;
    }
    int getFd() {
        return m_fd;
    }
    void setConnectionType(const TcpConnectionType& type) {
        // 设置连接类别
        m_connection_type = type;
    }

    NetAddr::s_ptr getLocalAddr();
    NetAddr::s_ptr getPeerAddr();

    void listenWrite(); // 监听写事件
    void listenRead(); // 监听读事件

private:
    void clear(); // 清理连接断开后的事件

private:
    EventLoop* m_event_loop{ NULL }; // 所属io线程的eventloop
    int m_fd;
    TcpState m_state; // 连接状态

    NetAddr::s_ptr m_peer_addr; // 监听地址
    NetAddr::s_ptr m_local_addr; // 本地地址
    FdEvent* m_fd_event; // 绑定的监听事件

    TcpBuffer::s_ptr m_in_buffer; // 写入缓冲区
    TcpBuffer::s_ptr m_out_buffer; // 写出缓冲区

    TcpConnectionType m_connection_type{ TcpConnectionByServer };

    AbstractCoder* m_coder;

    // 存储请求和对应的回调函数
    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;
    // 
    std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;
};

}


#endif