#include "tinyrpc/net/time_event.h"
#include "tinyrpc/common/util.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {
TimeEvent::TimeEvent(int interval, int is_repeated, std::function<void()> cb)
    :m_interval(interval), m_is_repeated(is_repeated), m_task(cb) {
        resetArriveTime();
}

void TimeEvent::resetArriveTime() {
    m_arrive_time = getNowMs() + m_interval;
    // DEBUGLOG("success create timer event, will excute at [%lld]", m_arrive_time);
}

}