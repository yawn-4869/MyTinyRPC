#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/net/tcp/tcp_client.h"
#include "tinyrpc/net/coder/abstract_protocol.h"
#include "tinyrpc/net/coder/tinypb_coder.h"

void test_connect() {
    // 调用connect连接server
    // write一个字符串
    // 等待server读取后返回结果
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        printf("invalid fd [%d]", fd);
        exit(0);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int rt = connect(fd, (sockaddr*)&addr, sizeof(addr));
    if(rt < 0) {
        printf("connect to server failed!, errno: %d\n", errno);
        exit(0);
    }

    std::string msg = "hello, tinyrpc!";
    rt = write(fd, msg.c_str(), msg.length());
    printf("success write %d bytes to server, clientfd: %d, msg: %s\n", rt, fd, msg.c_str());

    char buf[100];
    rt = read(fd, buf, 100);
    printf("success read %d bytes from server, msg: %s\n", rt, std::string(buf).c_str());
}

void test_client() {
    // MyTinyRPC::IPNetAddr::s_ptr local_addr = std::make_shared<MyTinyRPC::IPNetAddr>("127.0.0.1", 12346);
    MyTinyRPC::IPNetAddr::s_ptr peer_addr = std::make_shared<MyTinyRPC::IPNetAddr>("127.0.0.1", 12345);
    MyTinyRPC::TcpClient::s_ptr client = std::make_shared<MyTinyRPC::TcpClient>(nullptr, peer_addr);
    client->onConnect([peer_addr, client]() {
        DEBUGLOG("connect to [%s] success", peer_addr->toString().c_str());
        std::shared_ptr<MyTinyRPC::TinyPBProtocol> message = std::make_shared<MyTinyRPC::TinyPBProtocol>();
        message->setReqId("123456789");
        message->m_pb_data = "test pb data";
        client->writeMessage(message, [](MyTinyRPC::AbstractProtocol::s_ptr message_ptr){
            DEBUGLOG("message send success\n");
        });
        client->readMessage("123456789", [](MyTinyRPC::AbstractProtocol::s_ptr msg_ptr){
            std::shared_ptr<MyTinyRPC::TinyPBProtocol> message = std::dynamic_pointer_cast<MyTinyRPC::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response: %s", message->getReqId().c_str(), message->m_pb_data.c_str());
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