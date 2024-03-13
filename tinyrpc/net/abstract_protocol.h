#ifndef MYTINYRPC_NET_ABSTRACT_PROTOCOL_H
#define MYTINYRPC_NET_ABSTRACT_PROTOCOL_H

#include <memory>


namespace MyTinyRPC {
class AbstractProtocol {
public: 
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

    std::string getReqId() {
        return m_req_id;
    }

    void setReqId(const std::string req_id) {
        m_req_id = req_id;
    }

protected:
    std::string m_req_id; // 请求号, 唯一的标识一个请求或响应
};

}

#endif