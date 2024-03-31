#ifndef MYTINYRPC_NET_TCP_TCP_CLIENT_H
#define MYTINYRPC_NET_TCP_TCP_CLIENT_H

#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/coder/abstract_protocol.h"

namespace MyTinyRPC{

class TcpClient {
public:
    typedef std::shared_ptr<TcpClient> s_ptr;
    TcpClient(NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr = nullptr);
    ~TcpClient();
    void initLocalAddr();

    // 异步connect
    // connect成功, 调用done函数
    void onConnect(std::function<void()> done);

    // 异步发送
    // 成功调用done函数, 传入AbstractProtocol::s_ptr的参数
    void writeMessage(AbstractProtocol::s_ptr request, std::function<void(AbstractProtocol::s_ptr)> done);
    
    // 异步读取
    // 成功调用done函数, 传入AbstractProtocol::s_ptr的参数
    void readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);

    // 结束eventloop
    void stop();

    void addTimerEvent(TimeEvent::s_ptr timer_event);

    int getErrorCode();

    std::string getErrorInfo();

    NetAddr::s_ptr getPeerAddr();

    NetAddr::s_ptr getLocalAddr();

private:
    NetAddr::s_ptr m_peer_addr;
    NetAddr::s_ptr m_local_addr;
    EventLoop* m_event_loop{ NULL };
    int m_fd{ -1 };
    FdEvent* m_fd_event;
    TcpConnection::s_ptr m_connection;

    int m_error_code{ 0 };
    std::string m_error_info;
};

}

#endif