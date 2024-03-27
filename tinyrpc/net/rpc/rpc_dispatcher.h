#ifndef MYTINYRPC_NET_RPC_RPC_DISPATCHER_H
#define MYTINYRPC_NET_RPC_RPC_DISPATCHER_H

#include <map>
#include <google/protobuf/service.h>
#include "tinyrpc/net/coder/tinypb_protocol.h"

namespace MyTinyRPC {

class TcpConnection;

class RpcDisPatcher {
public:
    static RpcDisPatcher* GetRpcDispatcher();
public:
    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;
    // 根据对应的请求生成对应的rpc方法, 并发送相应信息
    void dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection);
    // 注册服务对象
    void registerService(service_s_ptr service);
    void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t error_code, std::string error_info);

private:
    bool parseServiceFullName(const std::string full_name, std::string& service_name, std::string& method_name);

private:
    std::map<std::string, service_s_ptr> m_service_map;
};
}

#endif