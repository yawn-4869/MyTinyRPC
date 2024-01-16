#include "tinyrpc/common/log.h"
#include "tinyrpc/common/config.h"
#include <pthread.h>

void* fun(void*) {
    int i = 20;
    while(i--) {
        DEBUGLOG("debug: this is thread in %s", "fun");
        INFOLOG("info: this is thread in %s", "fun");
    }
    return NULL;
}

int main() {

    MyTinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    // 一开始就创建Logger，避免多个线程访问到g_logger为NULL时，创建多个日志实例
    MyTinyRPC::Logger::InitGlobalLogger();

    pthread_t thread;
    pthread_create(&thread, NULL, &fun, NULL);
    int i = 20;
    while(i--) {
        DEBUGLOG("test log %s", "debug");
        INFOLOG("test log %s", "info");
    }

    pthread_join(thread, NULL);
    return 0;
}