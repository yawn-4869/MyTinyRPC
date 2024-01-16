#ifndef MYTINYRPC_COMMON_LOCKER_H
#define MYTINYRPC_COMMON_LOCKER_H

#include <pthread.h>

namespace MyTinyRPC {

template <class T>
class ScopeLocker {
public:
    ScopeLocker(T& mutex) : m_mutex(mutex){
        m_mutex.lock();
        m_is_lock = true;
    }

    ~ScopeLocker() {
        m_mutex.unlock();
        m_is_lock = false;
    }

    void lock() {
        if(!m_is_lock) {
            m_mutex.lock();
        }
    }

    void unlock() {
        if(m_is_lock) {
            m_mutex.unlock();
            m_is_lock = false;
        }
    }
    
private:
    T m_mutex;
    bool m_is_lock {false};
};

class Mutex {
public:
    Mutex() {
        pthread_mutex_init(&m_mutex, NULL);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
    pthread_mutex_t* getMutex() {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

}

#endif