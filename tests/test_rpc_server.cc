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
    // APPDEBUGLOG("now start sleep")
    // sleep(5);
    // APPDEBUGLOG("sleep end")
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

int main(int argc, char* argv[]) {
    if(argc != 2) {
      printf("Start rpc server error, argc != 2!\n");
      printf("Start like this:\n");
      printf("./test_rpc_server ../conf/tinyrpc.xml\n");
      return 0;
    }
    MyTinyRPC::Config::SetGlobalConfig(argv[1]);
    MyTinyRPC::Logger::InitGlobalLogger();
    MyTinyRPC::RpcDisPatcher::GetRpcDispatcher()->registerService(std::make_shared<OrderImpl>());

    MyTinyRPC::IPNetAddr::s_ptr local_addr = std::make_shared<MyTinyRPC::IPNetAddr>("127.0.0.1", MyTinyRPC::Config::GetGloabalConfig()->m_port);
    MyTinyRPC::TcpServer tcp_server(local_addr);
    
    tcp_server.start();
    
    return 0;
}