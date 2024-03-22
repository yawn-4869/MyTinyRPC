#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {

TcpClient::TcpClient(NetAddr::s_ptr local_addr, NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr), m_local_addr(local_addr) {
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

void TcpClient::onConnect(std::function<void()> done) {
    int rt = connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if(rt == 0) {
        DEBUGLOG("connect success to %s", m_peer_addr->toString().c_str());
        m_connection->setState(Connected);
        if(done) {
            done();
        }
    } else if(rt == -1) {
        if(errno == EINPROGRESS) {
            // 添加到epoll, 监听可写事件
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){
                int val = 0;
                socklen_t val_len = sizeof(val);
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &val, &val_len);
                if(val == 0) {
                    DEBUGLOG("connect success to %s", m_peer_addr->toString().c_str());
                    m_connection->setState(Connected);

                    // 连接成功后去掉对可写事件的监听，否则会一直触发
                    m_fd_event->cancel(FdEvent::OUT_EVENT);
                    m_event_loop->deleteEpollEvent(m_fd_event);

                    // 执行回调函数
                    if(done) {
                        done();
                    }
                } else {
                    ERRORLOG("conncet error, errno: %d, error: %s", errno, strerror(errno));
                }
            });

            m_event_loop->addEpollEvent(m_fd_event);
            if(!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }

        } else {
            ERRORLOG("conncet error, errno: %d, error: %s", errno, strerror(errno));
        }
    }
}

void TcpClient::writeMessage(AbstractProtocol::s_ptr request, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_connection->pushSendMessage(request, done);
    m_connection->listenWrite();
}

void TcpClient::readMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    // 1. 监听可读事件
    // 2. 从buffer中decode得到message对象, 判断req_id是否相等, 相等则成功
    m_connection->pushReadMessage(req_id, done);
    m_connection->listenRead();
}


}