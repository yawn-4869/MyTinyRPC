#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/net/tcp/tcp_server.h"

void test_tcp_server() {
    MyTinyRPC::IPNetAddr::s_ptr local_addr = std::make_shared<MyTinyRPC::IPNetAddr>("127.0.0.1", 12345);
    MyTinyRPC::TcpServer tcp_server(local_addr);
    tcp_server.start();
}

int main() {
    MyTinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    MyTinyRPC::Logger::InitGlobalLogger();

    test_tcp_server();
    
    return 0;
}