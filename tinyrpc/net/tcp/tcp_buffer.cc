#include "string.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {
TcpBuffer::TcpBuffer(int size) : m_size(size) {
    m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer() {

}

int TcpBuffer::readAble() {
    return m_write_idx - m_read_idx;
}

int TcpBuffer::writeAble() {
    return m_size - m_write_idx;
}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
    if(size > writeAble()) {
        // 调整buffer大小，扩容
        int new_size = (int)(1.5 * (m_write_idx + size));
        resizeBuffer(new_size);
    }
    memcpy(&m_buffer[m_write_idx], buf, size);
}

void TcpBuffer::readFromBuffer(std::vector<char>& res, int size) {
    if(readAble() == 0) return;
    int read_size = readAble() > size ? size : readAble();
    std::vector<char> tmp(read_size);
    memcpy(&tmp[0], &m_buffer[m_read_idx], read_size);
    res.swap(tmp);
    m_read_idx += read_size;
    adjustBuffer();
}

void TcpBuffer::resizeBuffer(int new_size) {
    std::vector<char> tmp(new_size);
    int count = std::min(new_size, readAble());
    memcpy(&tmp[0], &m_buffer[m_read_idx], count);
    m_buffer.swap(tmp);

    m_read_idx = 0;
    m_write_idx = count;
    m_size = new_size;
}

void TcpBuffer::adjustBuffer() {
    // 读取超过buffer的1/3后，开始调整指针
    if(m_read_idx < m_size / 3) {
        return;
    }

    int count = readAble();

    std::vector<char> tmp(m_size);
    memcpy(&tmp[0], &m_buffer[m_read_idx], count);
    m_buffer.swap(tmp); 

    m_read_idx = 0;
    m_write_idx = count;
}

void TcpBuffer::moveReadIndex(int size) {
    if(size < 0) {
        ERRORLOG("moveReadIndex error, invalid size: %d, it can't less than 0", size);
    }
    int new_read_idx = m_read_idx + size;
    if(new_read_idx >= m_size) {
        ERRORLOG("moveReadIndex error, invalid size: %d, old_read_idx: %d, buffer size: %d", size, m_read_idx, m_size);
        return;
    }
    m_read_idx = new_read_idx;
    adjustBuffer();
}
void TcpBuffer::moveWriteIndex(int size) {
    if(size < 0) {
        ERRORLOG("moveWriteIndex error, invalid size: %d, it can't less than 0", size);
    }
    int new_write_idx = m_write_idx + size;
    if(new_write_idx >= m_size) {
        ERRORLOG("moveWriteIndex error, invalid size: %d, old_read_idx: %d, buffer size: %d", size, m_read_idx, m_size);
        return;
    }
    m_write_idx = new_write_idx;
}

}