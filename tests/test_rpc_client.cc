#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/coder/tinypb_coder.h"
#include "tinyrpc/net/rpc/rpc_controller.h"
#include "tinyrpc/net/rpc/rpc_channel.h"
#include "tinyrpc/net/rpc/rpc_closure.h"
#include "order.pb.h"

void test_client() {
    MyTinyRPC::IPNetAddr::s_ptr peer_addr = std::make_shared<MyTinyRPC::IPNetAddr>("127.0.0.1", 12345);
    MyTinyRPC::TcpClient::s_ptr client = std::make_shared<MyTinyRPC::TcpClient>(peer_addr);
    client->onConnect([peer_addr, client]() {
        DEBUGLOG("connect to [%s] success", peer_addr->toString().c_str());
        std::shared_ptr<MyTinyRPC::TinyPBProtocol> message = std::make_shared<MyTinyRPC::TinyPBProtocol>();
        message->setReqId("99998888");
        // message->m_pb_data = "test pb data";

        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");

        if(!request.SerializeToString(&(message->m_pb_data))) {
            ERRORLOG("request serialize error");
            return;
        }

        message->m_method_name = "Order.makeOrder";

        client->writeMessage(message, [request](MyTinyRPC::AbstractProtocol::s_ptr message_ptr){
            // DEBUGLOG("message send success, request: %s", request.ShortDebugString().c_str());
            DEBUGLOG("message send success, request: %s", request.ShortDebugString().c_str());
        });
        client->readMessage("99998888", [](MyTinyRPC::AbstractProtocol::s_ptr msg_ptr){
            std::shared_ptr<MyTinyRPC::TinyPBProtocol> message = std::dynamic_pointer_cast<MyTinyRPC::TinyPBProtocol>(msg_ptr);

            makeOrderResponse response;
            if(!response.ParseFromString(message->m_pb_data)) {
                ERRORLOG("request deserialize error");
                return;
            }

            DEBUGLOG("msg_id[%s], get response: %s", message->getReqId().c_str(), response.ShortDebugString().c_str());


        });
    });
}

// void test_rpc_channel() {
//     NEWRPCCHANNEL(channel, "127.0.0.1:12345");

//     NEWMESSAGE(makeOrderRequest, request);
//     NEWMESSAGE(makeOrderResponse, response);

//     // std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
//     request->set_price(100);
//     request->set_goods("apple");

//     NEWRPCCONTROLLER(controller);
//     controller->SetMsgId("99998888");
//     controller->SetTimeout(10000);

//     // std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();

//     std::shared_ptr<MyTinyRPC::RpcClosure> closure = std::make_shared<MyTinyRPC::RpcClosure>([request, response, channel, controller]() mutable {
//         if(controller->GetErrorCode() == 0) {
//             INFOLOG("call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), 
//             response->ShortDebugString().c_str());
//             if(response->order_id() == "xxx") {

//             }
//         } else {
//             ERRORLOG("call rpc failed, request[%s], error code[%d], error info: %s", 
//             request->ShortDebugString().c_str(), 
//             controller->GetErrorCode(),
//             controller->GetErrorInfo().c_str());
//         }
//         // channel->getTcpClient()->stop();
//         channel.reset();
//     });

//     // channel->Init(controller, request, response, closure);

//     // Order_Stub stub(channel.get());
//     // stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
//     CALLRPC("127.0.0.1:12345", Order_Stub, makeOrder, controller, request, response, closure);
// }

int main() {
    MyTinyRPC::Config::SetGlobalConfig(NULL);
    MyTinyRPC::Logger::InitGlobalLogger(0);

    // test_client();
    // test_rpc_channel();

    return 0;
}