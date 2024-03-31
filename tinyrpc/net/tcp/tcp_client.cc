#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/error_code.h"

namespace MyTinyRPC {

TcpClient::TcpClient(NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr) : m_peer_addr(peer_addr), m_local_addr(local_addr) {
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
    if(m_fd < 0) {
        ERRORLOG("TcpClient error: failed to create fd, errno: %d", errno);
        return;
    }

    m_fd_event = FdEventPool::GetFdEventPool()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();

    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, m_peer_addr, m_local_addr, TcpConnectionByClient);
    // m_connection->setConnectionType(TcpConnectionByClient);
}

TcpClient::~TcpClient() {
    if(m_fd) {
        close(m_fd);
    }

    if(m_event_loop) {
        if(m_event_loop->isLooping()) {
            m_event_loop->stop();
        }
        delete m_event_loop;
        m_event_loop = NULL;
    }
}

void TcpClient::initLocalAddr() {
    sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    int rt = getsockname(m_fd, (sockaddr*)&local_addr, &local_addr_len);
    if(rt != 0) {
        ERRORLOG("initLocalAddr error, errno[%d], error: %s", errno, strerror(errno));
        return;
    }

    m_local_addr = std::make_shared<IPNetAddr>(local_addr);
}

void TcpClient::onConnect(std::function<void()> done) {
    int rt = connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if(rt == 0) {
        DEBUGLOG("connect success to %s", m_peer_addr->toString().c_str());
        m_connection->setState(Connected);
        initLocalAddr();
        if(done) {
            done();
        }
    } else if(rt == -1) {
        if(errno == EINPROGRESS) {
            // 添加到epoll, 监听可写事件
            m_fd_event->listen(FdEvent::OUT_EVENT, 
                [this, done](){
                    int rt = connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
                    if((rt < 0 && errno == EISCONN) || rt == 0) {
                        DEBUGLOG("connect success to %s", m_peer_addr->toString().c_str());
                        m_connection->setState(Connected);
                        initLocalAddr();
                    } else {
                        if(errno == ECONNREFUSED) {
                            m_error_code = ERROR_PEER_CLOSE;
                            m_error_info = "connect refused, sys error = " + std::string(strerror(errno));
                        } else {
                            m_error_code = ERROR_FAILED_CONNECT;
                            m_error_info = "connect unkonwn error, sys error = " + std::string(strerror(errno));
                        }
                        ERRORLOG("connect error, errno[%d], error: %s", errno, strerror(errno));
                        close(m_fd);
                        m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
                    }
                    // 连接完后需要去掉可写事件的监听，不然会一直触发
                    m_event_loop->deleteEpollEvent(m_fd_event);
                    DEBUGLOG("now begin to done");
                    // 如果连接完成，才会执行回调函数
                    if (done) {
                        done();
                    }
                }
            );

            m_event_loop->addEpollEvent(m_fd_event);
            if(!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }

        } else {
            m_error_code = ERROR_FAILED_CONNECT;
            m_error_info = "connect error, sys error: " + std::string(strerror(errno));
            ERRORLOG("conncet error, errno: %d, error: %s", errno, strerror(errno));
            if(done) {
                done();
            }
        }
    }
}

void TcpClient::writeMessage(AbstractProtocol::s_ptr request, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_connection->pushSendMessage(request, done);
    m_connection->listenWrite();
}

void TcpClient::readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    // 1. 监听可读事件
    // 2. 从buffer中decode得到message对象, 判断msg_id是否相等, 相等则成功
    m_connection->pushReadMessage(msg_id, done);
    m_connection->listenRead();
}

void TcpClient::stop() {
    if(m_event_loop->isLooping()) {
        m_event_loop->stop();
    }
}

void TcpClient::addTimerEvent(TimeEvent::s_ptr timer_event) {
    m_event_loop->addTimerEvent(timer_event);
}

int TcpClient::getErrorCode() {
    return m_error_code;
}

std::string TcpClient::getErrorInfo() {
    return m_error_info;
}

NetAddr::s_ptr TcpClient::getPeerAddr() {
    return m_peer_addr;
}

NetAddr::s_ptr TcpClient::getLocalAddr() {
    return m_local_addr;
}


}