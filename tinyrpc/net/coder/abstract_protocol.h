#ifndef MYTINYRPC_NET_CODER_ABSTRACT_PROTOCOL_H
#define MYTINYRPC_NET_CODER_ABSTRACT_PROTOCOL_H

#include <memory>


namespace MyTinyRPC {

struct AbstractProtocol {
public: 
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

    virtual ~AbstractProtocol() {};

protected:
    std::string m_req_id; // 请求号, 唯一的标识一个请求或响应
};

}

#endif