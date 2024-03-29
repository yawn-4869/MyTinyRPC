#ifndef MYTINYRPC_NET_CODER_ABSTRACT_PROTOCOL_H
#define MYTINYRPC_NET_CODER_ABSTRACT_PROTOCOL_H

#include <memory>


namespace MyTinyRPC {

struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {
public: 
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

    std::string getReqId() {
        return m_msg_id;
    }

    void setReqId(std::string id) {
        m_msg_id = id;
    }

    virtual ~AbstractProtocol() {};

public:
    std::string m_msg_id; // 请求号, 唯一的标识一个请求或响应
};

}

#endif