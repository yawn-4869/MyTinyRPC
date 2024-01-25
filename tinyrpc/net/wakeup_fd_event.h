#ifndef MYTINYRPC_WAKEUP_FD_EVENT_H
#define MYTINYRPC_WAKEUP_FD_EVENT_H

#include "tinyrpc/net/fd_event.h"

namespace MyTinyRPC {
class WakeUpFdEvent : public FdEvent {
public:
    WakeUpFdEvent(int fd);
    ~WakeUpFdEvent();
    void wakeup();
private:
};

}

#endif