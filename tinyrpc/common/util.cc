#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "util.h"


namespace MyTinyRPC {

// TODO: 不太懂静态成员变量的设置 减少系统函数调用？
static int g_pid = 0;

static thread_local int t_thread_id = 0;

pid_t getPid() {
    if(g_pid != 0) {
        return g_pid;
    }
    return getpid();
}

pid_t getThreadId() {
    if(t_thread_id != 0) {
        return t_thread_id;
    }
    return syscall(SYS_gettid);
}

int64_t getNowMs() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int32_t getInt32FromNetByte(const char* buf) {
    int32_t tmp;
    memcpy(&tmp, buf, sizeof(tmp));
    return ntohl(tmp);
}

}