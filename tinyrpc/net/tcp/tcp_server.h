#ifndef MYTINYRPC_NET_TCP_TCP_SERVER_H
#define MYTINYRPC_NET_TCP_TCP_SERVER_H

#include <set>
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/net/io_thread_pool.h"
#include "tinyrpc/net/tcp/tcp_acceptor.h"

namespace MyTinyRPC {
class TcpServer {
public: 
    TcpServer(NetAddr::s_ptr local_addr);
    ~TcpServer();
    void start();

private:
    void init();
    void onAccept();

private:
    EventLoop* m_main_event_loop; // mainReactor, 只处理连接
    IOThreadPool* m_io_thread_pool; // subReactors, mainReactor收到连接请求时将业务分发给线程池, 由线程池来处理对应的业务逻辑

    NetAddr::s_ptr m_local_addr; // 本地监听地址
    TcpAcceptor::s_ptr m_acceptor; // 
    FdEvent* m_listen_fd_event; // 监听事件

    std::set<TcpConnection::s_ptr> m_clients; // 客户端连接, 每一个客户端绑定一个线程池中的线程
};

}

#endif