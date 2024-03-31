#ifndef MYTINYRPC_NET_RPC_RPC_CHANNEL_H
#define MYTINYRPC_NET_RPC_RPC_CHANNEL_H

#include <memory>
#include <google/protobuf/service.h>
#include "tinyrpc/net/tcp/net_addr.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/time_event.h"

namespace MyTinyRPC {

#define NEWMESSAGE(type, var_name) \
    std::shared_ptr<type> var_name = std::make_shared<type>();\

#define NEWRPCCHANNEL(var_name, addr) \
    std::shared_ptr<MyTinyRPC::RpcChannel> var_name = std::make_shared<MyTinyRPC::RpcChannel>(std::make_shared<MyTinyRPC::IPNetAddr>(addr)); \

#define NEWRPCCONTROLLER(var_name) \
    std::shared_ptr<MyTinyRPC::RpcController> var_name = std::make_shared<MyTinyRPC::RpcController>(); \

#define CALLRPC(addr, stub_name, method_name, controller, request, response, closure) \
    { \
    NEWRPCCHANNEL(channel, addr); \
    channel->Init(controller, request, response, closure); \
    stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
    } \

class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel> {
public:
    typedef std::shared_ptr<RpcChannel> s_ptr;
    typedef std::shared_ptr<google::protobuf::RpcController> controller_s_ptr;
    typedef std::shared_ptr<google::protobuf::Message> message_s_ptr;
    typedef std::shared_ptr<google::protobuf::Closure> closure_s_ptr;

    RpcChannel(NetAddr::s_ptr peer_addr);

    ~RpcChannel();

    void Init(controller_s_ptr controller, message_s_ptr request, message_s_ptr response, closure_s_ptr done);

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                            google::protobuf::Message* response, google::protobuf::Closure* done);

    google::protobuf::RpcController* getController();

    google::protobuf::Message* getRequest();

    google::protobuf::Message* getResponse();

    google::protobuf::Closure* getClosure();

    TcpClient* getTcpClient();

    TimeEvent::s_ptr getTimerEvent();
private:
    NetAddr::s_ptr m_local_addr{ nullptr };
    NetAddr::s_ptr m_peer_addr{ nullptr };
    
    controller_s_ptr m_controller{ nullptr };
    message_s_ptr m_request{ nullptr };
    message_s_ptr m_response{ nullptr };
    closure_s_ptr m_closure{ nullptr };
    TcpClient::s_ptr m_client{ nullptr };
    std::shared_ptr<TimeEvent> m_timer_event;

    bool m_is_init{ false };
};

}

#endif