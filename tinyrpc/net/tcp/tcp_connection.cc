#include <unistd.h>
#include "tinyrpc/net/tcp/tcp_connection.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/net/coder/tinypb_coder.h"

namespace MyTinyRPC {

TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, TcpConnectionType connection_type) 
    : m_event_loop(event_loop), m_fd(fd), m_state(NotConnected), m_peer_addr(peer_addr), m_local_addr(local_addr), m_connection_type(connection_type) {
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event = FdEventPool::GetFdEventPool()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    m_coder = new TinyPBCoder();

    // 服务端一直监听读事件
    if(m_connection_type == TcpConnectionByServer) {
        listenRead();
    }
}

TcpConnection::~TcpConnection() {
    INFOLOG("~TcpConnection()");
    if(m_coder) {
        delete m_coder;
    }
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
    if(m_connection_type == TcpConnectionByServer) {
        // server
        // 读取in_buffer中的数据
        // std::vector<char> result;
        // int read_count = m_in_buffer->readAble();
        // m_in_buffer->readFromBuffer(result, read_count);

        std::vector<AbstractProtocol::s_ptr> results;
        std::vector<AbstractProtocol::s_ptr> responses;
        m_coder->decode(results, m_in_buffer);

        for(size_t i = 0; i < results.size(); ++i) {
            // 对于每一个request, 调用rpc方法, 获取response
            // 将response放入到发送缓冲区, 监听可写事件
            INFOLOG("success get request [%s] from client [%s], clientfd [%d]", results[i]->m_msg_id.c_str(), 
            m_peer_addr->toString().c_str(), m_fd);
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            RpcDisPatcher::GetRpcDispatcher()->dispatch(results[i], message, this);
            // m_dispatcher->dispatch(results[i], message, this);

            // std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            // message->m_pb_data = "hello, this is rpc response test data";
            // message->m_msg_id = results[i]->m_msg_id;

            responses.emplace_back(message);
        }

        m_coder->encode(responses, m_out_buffer);

        // std::string msg;
        // for(int i = 0; i < read_count; ++i) {
        //     msg += result[i];
        // }
        // m_out_buffer->writeToBuffer(msg.c_str(), msg.length());

        listenWrite(); // 监听可读事件, 执行onWrite将响应报文写入out_buffer
    } else {
        // client
        // 从buffer中读取request对象, decode, 判断msg_id是否一致
        std::vector<AbstractProtocol::s_ptr> result;
        m_coder->decode(result, m_in_buffer);
        for(auto request : result) {
            std::string msg_id = request->getReqId();
            auto it = m_read_dones.find(msg_id);
            if(it != m_read_dones.end()) {
                it->second(request);
            }
        }
    }
}

void TcpConnection::onWrite() {
    if(m_state != Connected) {
        // 不是连接状态, 无法发送数据
        ERRORLOG("onWrite error: client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    std::vector<AbstractProtocol::s_ptr> messages;
    if(m_connection_type == TcpConnectionByClient) {
        // 如果是客户端连接, 将所有的message写入buffer
        // 1. message序列化(编码) 2. 保存到buffer 3. 读取buffer, 发送(已完成)
        for(size_t i = 0; i < m_write_dones.size(); ++i) {
            messages.push_back(m_write_dones[i].first);
        }

        m_coder->encode(messages, m_out_buffer);
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

    if(m_connection_type == TcpConnectionByClient) {
        // 执行回调函数
        for(size_t i = 0; i < m_write_dones.size(); ++i) {
            m_write_dones[i].second(m_write_dones[i].first);
        }

        m_write_dones.clear();
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

void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_write_dones.push_back(std::make_pair(message, done));
}

void TcpConnection::pushReadMessage(const std::string& message, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_read_dones.insert(std::make_pair(message, done));
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

NetAddr::s_ptr TcpConnection::getLocalAddr() {
    return m_local_addr;
}
NetAddr::s_ptr TcpConnection::getPeerAddr() {
    return m_peer_addr;
}

}