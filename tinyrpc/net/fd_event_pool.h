#ifndef MYTINYRPC_NET_FD_EVENT_POOL_H
#define MYTINYRPC_NET_FD_EVENT_POOL_H

#include <vector>
#include "tinyrpc/net/fd_event.h"

namespace MyTinyRPC {
class FdEventPool {
public:
    FdEventPool(int size);
    ~FdEventPool();
    FdEvent* getFdEvent(int fd);

public:
static FdEventPool* GetFdEventPool();

private:
    int m_size{ 0 };
    std::vector<FdEvent*> m_fd_event_pool;
};

}

#endif