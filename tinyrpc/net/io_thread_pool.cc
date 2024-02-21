#include "io_thread_pool.h"

namespace MyTinyRPC {
IOThreadPool::IOThreadPool(int size) : m_size(size) {
    m_io_thread_pool.resize(size);
    for(int i = 0; i < size; ++i) {
        m_io_thread_pool[i] = new IOThread();
    }
}

IOThreadPool::~IOThreadPool() {

}

void IOThreadPool::start() {
    for(int i = 0; i < m_size; ++i) {
        m_io_thread_pool[i]->start();
    }
}
void IOThreadPool::join() {
    for(int i = 0; i < m_size; ++i) {
        m_io_thread_pool[i]->join();
    }
}
}