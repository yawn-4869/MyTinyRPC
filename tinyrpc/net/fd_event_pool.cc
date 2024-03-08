#include "tinyrpc/net/fd_event_pool.h"

namespace MyTinyRPC {

FdEventPool::FdEventPool(int size) : m_size(size) {
    for(int i = 0; i < m_size; ++i) {
        m_fd_event_pool.push_back(new FdEvent(i));
    }
}

FdEventPool::~FdEventPool() {
    for(int i = 0; i < m_size; ++i) {
        delete m_fd_event_pool[i];
        m_fd_event_pool[i] = NULL;
    }
}

static FdEventPool* g_fd_event_pool = NULL;
FdEventPool* FdEventPool::GetFdEventPool() {
    if(g_fd_event_pool) {
        return g_fd_event_pool;
    }

    g_fd_event_pool = new FdEventPool(128);
    return g_fd_event_pool;
}

FdEvent* FdEventPool::getFdEvent(int fd){
    if(fd < m_size) {
        return m_fd_event_pool[fd];
    }

    int new_size = fd * 1.5;
    for(int i = m_size; i < new_size; ++i) {
        m_fd_event_pool.push_back(new FdEvent(i));
    }

    m_size = new_size;
    return m_fd_event_pool[fd];
}

}