#ifndef MYTINYRPC_EVENTLOOP_H
#define MYTINYRPC_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <queue>
#include <functional>
#include "tinyrpc/common/locker.h"
#include "tinyrpc/net/fd_event.h"
#include "tinyrpc/net/wakeup_fd_event.h"
#include "tinyrpc/net/timer.h"

namespace MyTinyRPC {

class EventLoop
{
public:
    typedef std::shared_ptr<EventLoop> s_ptr;
    EventLoop();
    ~EventLoop();

public:
    void loop();
    void wakeup();
    void stop();
    void addEpollEvent(FdEvent* event);
    void addTimerEvent(TimeEvent::s_ptr event);
    void deleteEpollEvent(FdEvent* event);

public:
    static EventLoop* GetCurrentEventLoop();

private:
    void dealWakeup();
    bool isInLoopThread();
    void addTask(std::function<void()> callback, bool is_wake_up = false);
    void initWakeUpFdEevent();
    void initTimer();

private:
    pid_t m_thread_id{ 0 }; // 线程id
    int m_epoll_fd{ 0 }; // epoll fd
    bool m_stop_flag{ false }; // 循环停止标志
    std::set<int> m_listen_fds; // 监听事件fd集合
    std::queue<std::function<void()>> m_pending_tasks; // 任务队列
    Mutex m_mutex; // 互斥锁
    WakeUpFdEvent* m_wakeup_fd_event{ NULL }; // 唤醒通知事件
    int m_wakeup_fd{ 0 };
    Timer* m_timer;
};

}

#endif