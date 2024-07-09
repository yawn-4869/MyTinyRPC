#ifndef MYTINYRPC_NET_RPC_RPC_INTERFACE_H
#define MYTINYRPC_NET_RPC_RPC_INTERFACE_H

#include <memory>
#include <google/protobuf/message.h>
#include <functional>
#include "tinyrpc/net/rpc/rpc_controller.h"

namespace MyTinyRPC {

class RpcClosure;

class RpcInterface : public std::enable_shared_from_this<RpcInterface> {
public:
    RpcInterface(const google::protobuf::Message* req, google::protobuf::Message* rsp, RpcClosure* done, RpcController* controller);

    virtual ~RpcInterface();

    void reply();

    void destroy();

    std::shared_ptr<RpcClosure> newRpcClosure(std::function<void()>& cb);

    virtual void run() = 0;

    virtual void setError(int code, const std::string& err_info) = 0;

protected:
    const google::protobuf::Message* m_req_base {NULL};

    google::protobuf::Message* m_rsp_base {NULL};

    RpcClosure* m_done {NULL};
    
    RpcController* m_controller {NULL};

};

}

#endif