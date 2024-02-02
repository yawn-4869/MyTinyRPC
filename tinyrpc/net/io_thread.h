#ifndef MYTINYRPC_NET_IO_THREAD_H
#define MYTINYRPC_NET_IO_THREAD_H

#include <semaphore.h>
#include "tinyrpc/net/eventloop.h"

namespace MyTinyRPC {

class IOThread {
public:
    IOThread();
    ~IOThread();
    EventLoop* getEventLoop() {
        return m_event_loop;
    }
    void start(); // 启动loop循环
    void join(); // 等待线程结束

public:
    static void* Run(void* arg);

private:
    pid_t m_thread_id{ -1 }; // 线程号
    pthread_t m_thread{ 0 }; // 线程句柄
    EventLoop* m_event_loop{ NULL }; // 当前io线程的loop对象
    sem_t m_init_sem; // 初始化信号量
    sem_t m_start_sem; // loop启动信号量
};

}

#endif