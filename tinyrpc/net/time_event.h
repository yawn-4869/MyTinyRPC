#ifndef MYTINYRPC_NET_TIME_EVENT_H
#define MYTINYRPC_NET_TIME_EVENT_H

#include <functional>
#include <memory>

namespace MyTinyRPC {
class TimeEvent {
public:
    typedef std::shared_ptr<TimeEvent> s_ptr;
    TimeEvent(int interval, int is_repeated, std::function<void()> cb);
    int64_t getArriveTime() {
        return m_arrive_time;
    }

    void setCancel(bool is_cancel) {
        m_is_cancel = true;
    }

    bool isCancel() {
        return m_is_cancel;
    }

    bool isRepeated() {
        return m_is_repeated;
    }

    std::function<void()> getCallback() {
        return m_task;
    }

    void resetArriveTime();

private:
    int64_t m_arrive_time; // 到期时间, ms
    int64_t m_interval; // 间隔时间, ms
    bool m_is_repeated; // 是否为周期性任务，需要重复执行
    bool m_is_cancel; // 是否取消定时任务
    std::function<void()> m_task; // 回调函数
};

}

#endif