#ifndef MYTINYRPC_NET_CODER_ABSTRACT_CODER_H
#define MYTINYRPC_NET_CODER_ABSTRACT_CODER_H

#include "tinyrpc/net/coder/abstract_protocol.h"
#include "tinyrpc/net/tcp/tcp_buffer.h"

namespace MyTinyRPC {

class AbstractCoder {
public:
    // 将message对象转化为字节流，写入到buffer
    virtual void encode(const std::vector<AbstractProtocol*>& message, TcpBuffer::s_ptr out_buffer) = 0; 
    // 将buffer中的字节流转化为message对象 
    virtual void decode(std::vector<AbstractProtocol*>& message, const TcpBuffer::s_ptr in_buffer) = 0; 
    virtual ~AbstractCoder() {}

};

}

#endif