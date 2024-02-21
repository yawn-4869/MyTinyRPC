#ifndef MYTINYRPC_NET_IO_THREAD_POOL_H
#define MYTINYRPC_NET_IO_THREAD_POOL_H

#include "io_thread.h"

namespace MyTinyRPC {

class IOThreadPool {
public:
    IOThreadPool(int size);
    ~IOThreadPool();
    void start();
    void join();
    IOThread* getIOThread() {
        if (m_index == m_size || m_index == -1)  {
            m_index = 0;
        }
        return m_io_thread_pool[m_index++];
    }

public:

private:
    int m_size{ 0 };
    std::vector<IOThread*> m_io_thread_pool;
    int m_index{ -1 };

};

}

#endif