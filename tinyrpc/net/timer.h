#ifndef MYTINYRPC_NET_TIMER_H
#define MYTINYRPC_NET_TIMER_H

#include <map>
#include "fd_event.h"
#include "time_event.h"
#include "tinyrpc/common/locker.h"

namespace MyTinyRPC {

class Timer : public FdEvent {
public:
    Timer();
    ~Timer();

    void addTimeEvent(TimeEvent::s_ptr event);
    void deleteTimeEvent(TimeEvent::s_ptr event);
    void onTimer(); // 发生IO事件后, eventloop执行的回调函数

private:
    void resetArriveTime();
private:
    std::multimap<int64_t, TimeEvent::s_ptr> m_pending_events;
    Mutex m_mutex;
};

}

#endif