#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/coder/tinypb_coder.h"
#include "order.pb.h"

void test_client() {
    MyTinyRPC::IPNetAddr::s_ptr peer_addr = std::make_shared<MyTinyRPC::IPNetAddr>("127.0.0.1", 12345);
    MyTinyRPC::TcpClient::s_ptr client = std::make_shared<MyTinyRPC::TcpClient>(nullptr, peer_addr);
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

            DEBUGLOG("req_id[%s], get response: %s", message->getReqId().c_str(), response.ShortDebugString().c_str());


        });
    });
}

int main() {
    MyTinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    MyTinyRPC::Logger::InitGlobalLogger();

    // test_connect();
    test_client();

    return 0;
}