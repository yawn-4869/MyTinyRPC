#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "tinyrpc/net/rpc/rpc_dispatcher.h"
#include "tinyrpc/net/coder/tinypb_protocol.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/error_code.h"
#include "tinyrpc/net/rpc/rpc_controller.h"
#include "tinyrpc/net/tcp/tcp_connection.h"

namespace MyTinyRPC {

static RpcDisPatcher* g_rpc_dispatcher = NULL;
RpcDisPatcher* RpcDisPatcher::GetRpcDispatcher() {
    if(g_rpc_dispatcher) {
        return g_rpc_dispatcher;
    }
    g_rpc_dispatcher = new RpcDisPatcher();
    return g_rpc_dispatcher;
}

void RpcDisPatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection) {
    std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
    // 根据请求体得到method_name
    std::string full_name = req_protocol->m_method_name;
    std::string service_name, method_name;
    if(!parseServiceFullName(full_name, service_name, method_name)) {
        setTinyPBError(req_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
        return;
    }

    // 根据method_name找到服务
    auto it = m_service_map.find(service_name);
    if(it == m_service_map.end()) {
        ERRORLOG("%s | service name[%s] not found", req_protocol->m_msg_id.c_str(), service_name.c_str());
        setTinyPBError(req_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
        return;
    }
    service_s_ptr service = it->second;
    const google::protobuf::MethodDescriptor* method = service->GetDescriptor()->FindMethodByName(method_name);
    if(method == NULL) {
        ERRORLOG("%s | method name[%s] not found", req_protocol->m_msg_id.c_str(), method_name.c_str());
        setTinyPBError(req_protocol, ERROR_METHOD_NOT_FOUNT, "method not found");
        return;
    }

    // 根据方法找到对应的request_type
    google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();
    
    // 将pb_data反序列化为对应的request_type对象
    if(!req_msg->ParseFromString(req_protocol->m_pb_data)) {
        ERRORLOG("%s | deserilize error, origin message [%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());
        setTinyPBError(req_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
        return;
    }

    INFOLOG("msg_id[%s], get rpc request[%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());

    // 根据方法找到对应的response对象
    google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

    RpcController rpc_controller;
    rpc_controller.SetLocalAddr(connection->getLocalAddr());
    rpc_controller.SetPeerAddr(connection->getPeerAddr());
    rpc_controller.SetMsgId(req_protocol->m_msg_id);

    service->CallMethod(method, &rpc_controller, req_msg, rsp_msg, NULL);
    
    // 生成对应的协议以发送
    rsp_protocol->m_msg_id = req_protocol->m_msg_id;
    rsp_protocol->m_method_name = req_protocol->m_method_name;

    // 将response对象序列化为pb_data
    if(!rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data))) {
        ERRORLOG("%s | serilize error, origin message", req_protocol->m_msg_id.c_str(), rsp_msg->ShortDebugString().c_str());
        setTinyPBError(req_protocol, ERROR_FAILED_SERIALIZE, "serilize error");
        return;
    }

    rsp_protocol->m_error_code = 0;
    INFOLOG("%s | dispatch success, request[%s], response[%s]", req_protocol->m_msg_id.c_str(),
    req_msg->ShortDebugString().c_str(), rsp_msg->ShortDebugString().c_str());

    delete req_msg;
    delete rsp_msg;
    req_msg = NULL;
    rsp_msg = NULL;
}

void RpcDisPatcher::registerService(service_s_ptr service) {
    std::string service_name = service->GetDescriptor()->full_name();
    m_service_map[service_name] = service;
}

void RpcDisPatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t error_code, std::string error_info) {
    msg->m_error_code = error_code;
    msg->m_error_msg = error_info;
    msg->m_error_msg_len = error_info.length();
}

bool RpcDisPatcher::parseServiceFullName(const std::string full_name, std::string& service_name, std::string& method_name) {
    if(full_name.empty()) {
        ERRORLOG("full name is empty");
        return false;
    }
    int split_pos = full_name.find_first_of(".");
    if(split_pos == full_name.npos) {
        ERRORLOG("parseServiceFullName[%s] error, method full name illeagal", full_name.c_str());
        return false;
    }

    service_name = full_name.substr(0, split_pos);
    method_name = full_name.substr(split_pos + 1, full_name.length() - split_pos - 1);
    INFOLOG("parse success from %s, service_name[%s], method_name[%s]", 
    full_name.c_str(), service_name.c_str(), method_name.c_str());
    return true;
}
}