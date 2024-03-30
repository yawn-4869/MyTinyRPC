#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "tinyrpc/net/rpc/rpc_channel.h"
#include "tinyrpc/net/coder/tinypb_protocol.h"
#include "tinyrpc/net/rpc/rpc_controller.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/msg_util.h"
#include "tinyrpc/common/error_code.h"
#include "tinyrpc/net/tcp/tcp_client.h"

namespace MyTinyRPC {

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
    m_client = std::make_shared<TcpClient>(m_peer_addr);
}

RpcChannel::~RpcChannel() {
    INFOLOG("~RpcChannel()");
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                        google::protobuf::Message* response, google::protobuf::Closure* done) {
    std::shared_ptr<TinyPBProtocol> req_protocol = std::make_shared<TinyPBProtocol>();

    RpcController* my_controller = dynamic_cast<RpcController*>(controller);
    if(my_controller == NULL || request == NULL || response == NULL) {
        ERRORLOG("initialize rpc controller failed");
        my_controller->SetError(ERROR_INIT_RPC_CHANNEL, "controller or request or response NULL");
        return;
    }

    if(!m_is_init) {
        ERRORLOG("failed callmethod, RpcChannel not init");
        my_controller->SetError(ERROR_INIT_RPC_CHANNEL, "failed callmethod, RpcChannel not init");
        return;
    }

    if(my_controller->GetMsgId().empty()) {
        req_protocol->m_msg_id = MsgUtil::GenMsgID();
        my_controller->SetMsgId(req_protocol->m_msg_id);
    } else {
        req_protocol->m_msg_id = my_controller->GetMsgId();
    }

    // 方法名
    req_protocol->m_method_name = method->full_name();

    // request序列化
    if(!request->SerializeToString(&(req_protocol->m_pb_data))) {
        ERRORLOG("failed to serialize");
        my_controller->SetError(ERROR_FAILED_SERIALIZE, "failed to serialize");
        return;
    }

    s_ptr channel = shared_from_this();

    // TcpClient client(m_peer_addr);
    m_client->onConnect([req_protocol, request, channel]() mutable {
        // 由于是回调函数, 因此在执行回调函数时, 传入的参数可能已经析构
        // 需要使用成员函数存储

        RpcController* my_controller = dynamic_cast<RpcController*>(channel->getController());

        if(channel->getTcpClient()->getErrorCode() != 0) {
            my_controller->SetError(channel->getTcpClient()->getErrorCode(), channel->getTcpClient()->getErrorInfo());
            ERRORLOG("%s | connect error, peer addr[%s], error code[%d], error info[%s]", req_protocol->m_msg_id.c_str(), 
            channel->getTcpClient()->getPeerAddr()->toString().c_str(), 
            channel->getTcpClient()->getErrorCode(), channel->getTcpClient()->getErrorInfo().c_str());
            return;
        }

        channel->getTcpClient()->writeMessage(req_protocol, [req_protocol, request, channel, my_controller](AbstractProtocol::s_ptr) mutable {
            // 发送成功的回调函数
            INFOLOG("%s |, send request success, call method name[%s], peer addr[%s], local addr[%s]", 
            req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str(),
            channel->getTcpClient()->getPeerAddr()->toString().c_str(), channel->getTcpClient()->getLocalAddr()->toString().c_str());
            
            // 读取回包
            channel->getTcpClient()->readMessage(req_protocol->m_msg_id, [channel, my_controller](AbstractProtocol::s_ptr msg_ptr) mutable {
                std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(msg_ptr);
                INFOLOG("%s| get response %s, call method name[%s]", rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_msg_id.c_str(), 
                rsp_protocol->m_method_name.c_str());

                if(!(channel->getResponse()->ParseFromString(rsp_protocol->m_pb_data))) {
                    my_controller->SetError(ERROR_FAILED_DESERIALIZE, "deserialized error");
                    ERRORLOG("%S | deserialized error", rsp_protocol->m_msg_id.c_str());
                    return;
                }

                if(rsp_protocol->m_error_code != 0) {
                    my_controller->SetError(rsp_protocol->m_error_code, rsp_protocol->m_error_msg);
                    ERRORLOG("%S | call rpc method failed, error code[%d], error info: %s", rsp_protocol->m_msg_id.c_str(), 
                    rsp_protocol->m_error_code, rsp_protocol->m_error_msg.c_str());
                    return;
                }

                INFOLOG("%s | call rpc success, peer addr[%s], local addr[%s]", rsp_protocol->m_msg_id.c_str(), 
                channel->getTcpClient()->getPeerAddr()->toString().c_str(), channel->getTcpClient()->getLocalAddr()->toString().c_str());

                if(channel->getClosure()) {
                    channel->getClosure()->Run();
                }

                channel.reset();
            });
        });
    });
    
}

void RpcChannel::Init(controller_s_ptr controller, message_s_ptr request, message_s_ptr response, closure_s_ptr done) {
    if(m_is_init) {
        return;
    }

    m_controller = controller;
    m_request = request;
    m_response = response;
    m_closure = done;
    m_is_init = true;
}

google::protobuf::RpcController* RpcChannel::getController() {
    return m_controller.get();
}

google::protobuf::Message* RpcChannel::getRequest() {
    return m_request.get();
}

google::protobuf::Message* RpcChannel::getResponse() {
    return m_response.get();
}

google::protobuf::Closure* RpcChannel::getClosure() {
    return m_closure.get();
}

TcpClient* RpcChannel::getTcpClient() {
    return m_client.get();
}

}