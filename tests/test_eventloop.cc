#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "tinyrpc/common/config.h"
#include "tinyrpc/common/log.h"
#include "tinyrpc/net/eventloop.h"
#include "tinyrpc/net/fd_event.h"

int main() {

    MyTinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    // 一开始就创建Logger，避免多个线程访问到g_logger为NULL时，创建多个日志实例
    MyTinyRPC::Logger::InitGlobalLogger();
    MyTinyRPC::EventLoop* eventloop = new MyTinyRPC::EventLoop();
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12310);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);

    int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rt != 0) {
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(listenfd, 100);
    if (rt != 0) {
        ERRORLOG("listen error");
        exit(1);
    }

    MyTinyRPC::FdEvent event(listenfd);
    event.listen(MyTinyRPC::FdEvent::IN_EVENT, [listenfd](){
      sockaddr_in peer_addr;
      socklen_t addr_len = sizeof(peer_addr);
      memset(&peer_addr, 0, sizeof(peer_addr));
      int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

      DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    });
    eventloop->addEpollEvent(&event);
    eventloop->loop();

    return 0;
}