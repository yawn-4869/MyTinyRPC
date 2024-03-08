#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"

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

int main() {
    MyTinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    MyTinyRPC::Logger::InitGlobalLogger();

    test_connect();

    return 0;
}