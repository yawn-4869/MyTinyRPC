#ifndef MYTINYRPC_NET_TCP_TCP_BUFFER_H
#define MYTINYRPC_NET_TCP_TCP_BUFFER_H

#include <vector>

namespace MyTinyRPC {

class TcpBuffer {
public:
    TcpBuffer(int size);
    ~TcpBuffer();

    int readAble(); // 返回可读字节数
    int writeAble(); // 返回可写字节数
    int getReadIndex() {
        return m_read_idx;
    }
    int getWriteIndex() {
        return m_write_idx;
    }
    void writeToBuffer(const char* buf, int size); // 写入数据
    void readFromBuffer(std::vector<char>& res, int size); // 读取数据
    void resizeBuffer(int new_size); // 调整Buffer大小
    void moveReadIndex(int size); // 手动调整读指针
    void moveWriteIndex(int size); // 手动调整写指针

private:
    void adjustBuffer(); // 调节左右指针，防止内存泄漏

private:
    int m_size{ 0 };
    int m_read_idx{ 0 };
    int m_write_idx{ 0 };
    std::vector<char> m_buffer;
};
}

#endif