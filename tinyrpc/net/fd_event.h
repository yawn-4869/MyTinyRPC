#ifndef MYTINYRPC_NET_FD_EVENT_H
#define MYTINYRPC_NET_FD_EVENT_H

#include <sys/epoll.h>
#include<functional>

namespace MyTinyRPC {
class FdEvent {
public:
    enum TriggerEvent {
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT
    };

    FdEvent(int fd);
    FdEvent();
    ~FdEvent();
    void setNonBlock(); // 设置未阻塞
    std::function<void()> handler(TriggerEvent event_type); // 返回事件绑定的函数
    void listen(TriggerEvent event_type, std::function<void()> callback); // 绑定函数
    void cancel(TriggerEvent event_type); // 取消事件绑定
    int getFd() {
        return m_fd;
    }
    epoll_event getEpollEvents() {
        return m_listen_events;
    }
protected:
    int m_fd{ -1 };
    epoll_event m_listen_events;
    std::function<void()> m_read_callback;
    std::function<void()> m_write_callback;
    std::function<void()> m_error_callback;
};

}

#endif