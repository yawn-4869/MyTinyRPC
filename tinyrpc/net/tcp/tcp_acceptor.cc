#include <sys/socket.h>
#include <string.h>
#include "tinyrpc/net/tcp/tcp_acceptor.h"
#include "tinyrpc/common/log.h"

namespace MyTinyRPC {

TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {
    if(!local_addr->checkValid()) {
        ERRORLOG("invalid local addr: %s", local_addr->toString().c_str());
        exit(0);
    }
    m_family = m_local_addr->getFamily();

    // 创建监听套接字
    m_listenfd = socket(m_family, SOCK_STREAM, 0);
    if(m_listenfd < 0) {
        ERRORLOG("create listenfd[%d] failed", m_listenfd);
        exit(0);
    }

    // 设置端口复用和非阻塞
    int val = 1;
    if(setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        ERRORLOG("setsockopt REUSEADDR error, errno=%d, error=%s", errno, strerror(errno));
    }

    // 绑定
    socklen_t len = m_local_addr->getSockLen();
    if(bind(m_listenfd, m_local_addr->getSockAddr(), len) != 0) {
        ERRORLOG("bind error, errno=%d, error=%s", errno, strerror(errno));
        exit(0);
    }

    // 监听, 设置最大连接数为1000
    if(listen(m_listenfd, 1000) != 0) {
        ERRORLOG("listen error, errno=%d, error=%s", errno, strerror(errno));
        exit(0);
    }
}
TcpAcceptor::~TcpAcceptor() {

}

std::pair<int, NetAddr::s_ptr> TcpAcceptor::accept() {
    if(m_family == AF_INET) {
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(sockaddr_in));
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = ::accept(m_listenfd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);

        if(client_fd < 0) {
            ERRORLOG("accept error, errno=%d, error=%s", errno, strerror(errno));
        }
        INFOLOG("A peer client have accepted, client_fd:%d", client_fd);

        IPNetAddr::s_ptr peer_addr = std::make_shared<IPNetAddr>(client_addr);
        return std::make_pair(client_fd, peer_addr);
    } else {
        // TODO: 支持其他协议的方法...
        return {-1, nullptr};
    }
}

}