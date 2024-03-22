#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {
TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {
    init();
    INFOLOG("TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
}

TcpServer::~TcpServer() {

}

void TcpServer::start() {
    m_io_thread_pool->start();
    m_main_event_loop->loop();
}

void TcpServer::init() {
    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    m_io_thread_pool = new IOThreadPool(Config::GetGloabalConfig()->m_io_threads);

    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);
    m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());
    // 监听写入事件
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    auto client = m_acceptor->accept();
    int client_fd = client.first;
    NetAddr::s_ptr client_addr = client.second;

    TcpConnection::s_ptr connection = std::make_shared<TcpConnection>(
        m_io_thread_pool->getIOThread()->getEventLoop(), client_fd, 128, client_addr, m_local_addr, TcpConnectionByServer);
    connection->setState(Connected);
    m_clients.insert(connection);

    INFOLOG("TcpServer get client[%s] success, clientfd[%d]", client_addr->toString().c_str(), client_fd);
}

}