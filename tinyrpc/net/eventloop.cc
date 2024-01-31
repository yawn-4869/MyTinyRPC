#include "eventloop.h"
#include <sys/epoll.h>
#include <algorithm>
#include <string.h>
#include <sys/eventfd.h>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/util.h"

#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if(it != m_listen_fds.end()) { \
        op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvents(); \
    INFOLOG("epoll_event.events=%d", (int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if(rt < 0) { \
        ERRORLOG("failed epoll_ctl when add fd %d, errno=%d, error=%s", event->getFd(), errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    DEBUGLOG("add event success, fd[%d]", event->getFd()); \

#define DEL_FROM_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    if(it == m_listen_fds.end()) { \
        return; \
    } \
    int op = EPOLL_CTL_DEL; \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), NULL); \
    if(rt < 0) { \
        ERRORLOG("failed epoll_ctl when delete fd %d, errno=%d, error=%s", event->getFd(), errno, strerror(errno)); \
    } \
    m_listen_fds.erase(event->getFd()); \
    DEBUGLOG("delete event success, fd[%d]", event->getFd()); \

namespace MyTinyRPC {

static thread_local EventLoop* t_current_event_loop = NULL;
static int g_epoll_max_timeout = 10000;
static int g_epoll_max_events = 10;
    
EventLoop::EventLoop() {
    if(t_current_event_loop != NULL) {
        ERRORLOG("failed to create event loop, this thread has created event loop");
        exit(0);
    }
    m_thread_id = getThreadId();
    m_epoll_fd = epoll_create(10);
    if(m_epoll_fd == -1) {
        ERRORLOG("failed to create event loop, epoll_create error, errno=%d", errno);
        exit(0);
    }
    initWakeUpFdEevent();
    initTimer();

    INFOLOG("success created event loop in thread %d", m_thread_id);
    t_current_event_loop = this;
}

EventLoop::~EventLoop() {
    close(m_epoll_fd);
    if(m_wakeup_fd_event) {
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = NULL;
    }
}

void EventLoop::loop() {
    while(!m_stop_flag) {
        // 处理任务队列
        ScopeLocker<Mutex> lock(m_mutex);
        std::queue<std::function<void()>> tmp_tasks;
        tmp_tasks.swap(m_pending_tasks);
        lock.unlock();
        while(!tmp_tasks.empty()) {
            std::function<void()> cb = tmp_tasks.front();
            tmp_tasks.pop();
            if(cb) {
                cb();
            }
        }

        // 开始epoll调用
        // TODO: time_out应该是设定值与下次定时任务事件之中的最大值，目前定时任务类还没写，故先初始化为设定值
        // 如果有定时任务，那么执行
        // 1. 如何判断定时任务需要执行：(now() > TimerEvent.arrive_time)
        // 2. arrive_time 如何让eventloop监听
        int time_out = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];
        int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, time_out);
        if(rt < 0) {
            ERRORLOG("epoll wait error, errno: %d", errno);
        } else {
            for(int i = 0; i < rt; ++i) {
                epoll_event trigger_event = result_events[i];
                FdEvent* fd_event = static_cast<FdEvent*> (trigger_event.data.ptr);
                if(fd_event == NULL) {
                    continue;
                }

                if(trigger_event.events & EPOLLIN) {
                    DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd())
                    addTask(fd_event->handler(FdEvent::IN_EVENT));
                }

                if(trigger_event.events & EPOLLOUT) {
                    DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd())
                    addTask(fd_event->handler(FdEvent::OUT_EVENT));
                }
            }
        }
    }
}

void EventLoop::wakeup() {
    m_wakeup_fd_event->wakeup();
}

void EventLoop::stop() {
    m_stop_flag = true;
}

void EventLoop::addEpollEvent(FdEvent* event) {
    if(isInLoopThread()) {
        ADD_TO_EPOLL();
    } else {
        auto cb = [this, event]() {
            ADD_TO_EPOLL()
        };
        addTask(cb, true);
    }
}

void EventLoop::addTimerEvent(TimeEvent::s_ptr event) {
    m_timer->addTimeEvent(event);
}

void EventLoop::deleteEpollEvent(FdEvent* event) {
    if(isInLoopThread()) {
        DEL_FROM_EPOLL();
    } else {
        auto cb = [this, event]() {
            DEL_FROM_EPOLL();
        };
        addTask(cb, true);
    }
}
void EventLoop::dealWakeup() {

}

bool EventLoop::isInLoopThread() {
    return getThreadId() == m_thread_id;
}

void EventLoop::addTask(std::function<void()> callback, bool is_wake_up) {
    ScopeLocker<Mutex> lock(m_mutex);
    m_pending_tasks.push(callback);
    lock.unlock();
    if(is_wake_up) {
        // 对于紧急任务，希望尽早完成，添加了唤醒操作
        wakeup();
    }
}

void EventLoop::initWakeUpFdEevent() {
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
    if(m_wakeup_fd < 0) {
        ERRORLOG("failed to create event loop, eventfd create error, errno=%d", errno);
        exit(0);
    }

    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]() {
        char buf[8];
        while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {
        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    });
    addEpollEvent(m_wakeup_fd_event);
}

void EventLoop::initTimer() {
    m_timer = new Timer();
    addEpollEvent(m_timer);
}

}