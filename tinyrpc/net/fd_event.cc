#include <string.h>
#include <fcntl.h>
#include "tinyrpc/net/fd_event.h"


namespace MyTinyRPC {
FdEvent::FdEvent(int fd) : m_fd(fd){
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::FdEvent() {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}


FdEvent::~FdEvent() {

}

void FdEvent::setNonBlock() {
    int flag = fcntl(m_fd, F_GETFL, 0);
    if(flag & O_NONBLOCK) {
        return;
    }
    fcntl(m_fd, F_SETFL, O_NONBLOCK);
}

std::function<void()> FdEvent::handler(TriggerEvent event_type) {
    if(event_type == TriggerEvent::IN_EVENT) {
        return m_read_callback;
    } else {
        return m_write_callback;
    }
}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback) {
    if(event_type == TriggerEvent::IN_EVENT) {
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
    } else {
        m_listen_events.events |= EPOLLOUT;
        m_write_callback = callback;
    }
    m_listen_events.data.ptr = this;
}

}