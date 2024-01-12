#include "util.h"
#include <sys/syscall.h>


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

}