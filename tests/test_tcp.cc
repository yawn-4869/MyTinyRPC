#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include "tinyrpc/net/tcp/net_addr.h"

int main() {
    MyTinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    MyTinyRPC::Logger::InitGlobalLogger();

    MyTinyRPC::IPNetAddr addr("127.0.0.1", 12346);

    DEBUGLOG("create addr %s", addr.toString().c_str());

    return 0;
}