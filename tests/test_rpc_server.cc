#include <unistd.h>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/net/tcp/tcp_server.h"
#include "tinyrpc/net/rpc/rpc_dispatcher.h"

#include "order.pb.h"

class OrderImpl : public Order {
 public:
  void makeOrder(google::protobuf::RpcController* controller,
                      const ::makeOrderRequest* request,
                      ::makeOrderResponse* response,
                      ::google::protobuf::Closure* done) {
    APPDEBUGLOG("now start sleep")
    sleep(5);
    APPDEBUGLOG("sleep end")
    if (request->price() < 10) {
      response->set_ret_code(-1);
      response->set_res_info("short balance");
      return;
    }
    response->set_order_id("20230514");

    APPDEBUGLOG("call makeOrder success");
    if (done) {
      done->Run();
      delete done;
      done = NULL;
    }
  }
};

void test_tcp_server() {
    MyTinyRPC::IPNetAddr::s_ptr local_addr = std::make_shared<MyTinyRPC::IPNetAddr>("127.0.0.1", 12345);
    MyTinyRPC::TcpServer tcp_server(local_addr);
    tcp_server.start();
}

int main() {
    MyTinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    MyTinyRPC::Logger::InitGlobalLogger();
    MyTinyRPC::RpcDisPatcher::GetRpcDispatcher()->registerService(std::make_shared<OrderImpl>());

    test_tcp_server();
    
    return 0;
}