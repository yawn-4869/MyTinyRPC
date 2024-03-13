#include <unistd.h>
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {

TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr) 
    : m_event_loop(event_loop), m_fd(fd), m_state(NotConnected), m_peer_addr(peer_addr), m_local_addr(local_addr) {
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event = FdEventPool::GetFdEventPool()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    // 服务端一直监听读事件
    if(m_connection_type == TcpConnectionByServer) {
        listenRead();
    }
}

TcpConnection::~TcpConnection() {
    
}

void TcpConnection::onRead() {
    if(m_state != Connected) {
        // 不是连接状态, 无法读取数据
        ERRORLOG("onRead error: client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    // 将数据从fd中读取到in_buffer
    bool is_read_all = false;
    bool is_close = false;
    while(true) {
        if(!m_in_buffer->writeAble()) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->getBufferSize());
        }

        int read_count = m_in_buffer->writeAble();
        int write_index = m_in_buffer->getWriteIndex();

        int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
        if(rt > 0) {
            m_in_buffer->moveWriteIndex(rt);
            if(rt == read_count) {
                continue;
            } else {
                is_read_all = true;
                break;
            }
        } else if(rt == 0) {
            // read返回0意味着对端close
            is_close = true;
            break;
        } else if(rt == -1 && errno == EAGAIN) {
            is_read_all = true;
            break;
        }
    }

    if(is_close) {
        INFOLOG("peer closed, peer addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
        // TODO: 处理连接关闭事件
        clear();
        return;
    }

    if(!is_read_all) {
        ERRORLOG("not read all data from peer addr [%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
    }

    // TODO: 简单的echo回调
    excute();
}

void TcpConnection::excute() {
    // 读取in_buffer中的数据
    std::vector<char> result;
    int read_count = m_in_buffer->readAble();
    m_in_buffer->readFromBuffer(result, read_count);

    // TODO: 将读取到的数据解析成rpc报文, 执行处理逻辑, 生成响应报文
    // 目前只是简单的读取和写入
    std::string msg;
    for(int i = 0; i < read_count; ++i) {
        msg += result[i];
    }
    m_out_buffer->writeToBuffer(msg.c_str(), msg.length());
    INFOLOG("success get request [%s] from client [%s], clientfd [%d]", msg.c_str(), m_peer_addr->toString().c_str(), m_fd);

    listenWrite(); // 监听可读事件, 执行onWrite将响应报文写入out_buffer
}

void TcpConnection::onWrite() {
    if(m_state != Connected) {
        // 不是连接状态, 无法发送数据
        ERRORLOG("onWrite error: client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_write_all = false;
    while(true) {
        if(!m_out_buffer->readAble()) {
            // 写缓冲区为空, 没有数据需要发送
            is_write_all = true;
            break;
        }

        int write_count = m_out_buffer->readAble();
        int write_index = m_out_buffer->getReadIndex();
        int rt = write(m_fd, &m_out_buffer->m_buffer[write_index], write_count);

        if(rt >= write_count) {
            // write返回大于等于write_count, 写入完毕
            DEBUGLOG("no data need to send to client [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
            is_write_all = true;
            break;
        }

        if(rt == -1 && errno == EAGAIN) {
            // fd已满，不能再发送了。
            // 等下次 fd 可写的时候再次发送数据即可
            ERRORLOG("write data error, errno==EAGIN and rt == -1");
            break;
        }

    }

    if(is_write_all) {
        m_fd_event->cancel(FdEvent::OUT_EVENT);
        m_event_loop->addEpollEvent(m_fd_event);
    }
}

void TcpConnection::shutDown() {
    if(m_state == NotConnected || m_state == Closed) {
        return;
    }

    // 服务器主动关闭: 半关闭状态, 服务器不再向客户端发送数据, 但仍然会接收客户端发来的数据
    m_state = HalfClosed;

    // 调用 shutdown 关闭读写，服务器不会再对这个 fd 进行读写操作了
    shutdown(m_fd, SHUT_RDWR);
}

void TcpConnection::listenRead() {
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::listenWrite() {
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::clear() {
    if(m_state == Closed) {
        return;
    }
    // 关闭连接后的处理
    // 取消监听
    m_fd_event->cancel(FdEvent::IN_EVENT);
    m_fd_event->cancel(FdEvent::OUT_EVENT);

    // 移除fd事件
    m_event_loop->deleteEpollEvent(m_fd_event);

    // 设置state为Closed, 因为是客户端关闭了, 所以状态为Closed
    m_state = Closed;
}

}