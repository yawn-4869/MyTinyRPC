#ifndef MYTINYRPC_NET_CODER_TINYPB_CODER_H
#define MYTINYRPC_NET_CODER_TINYPB_CODER_H

#include "tinyrpc/net/coder/abstract_coder.h"
#include "tinyrpc/net/coder/tinypb_protocol.h"

namespace MyTinyRPC {
class TinyPBCoder : public AbstractCoder {
public:
    TinyPBCoder() {}
    ~TinyPBCoder() {}
    // 将message对象转化为字节流，写入到buffer
    void encode(const std::vector<AbstractProtocol::s_ptr>& in_message, TcpBuffer::s_ptr out_buffer); 
    // 将buffer中的字节流转化为message对象 
    void decode(std::vector<AbstractProtocol::s_ptr>& out_message, const TcpBuffer::s_ptr in_buffer); 

private:
    const char* encodeTinyPB(TinyPBProtocol::s_ptr message, int& len);
};
}

#endif