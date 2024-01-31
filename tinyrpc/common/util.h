#ifndef MYTINYRPC_COMMON_UTIL_H
#define MYTINYRPC_COMMON_UTIL_H

#include <unistd.h>
#include <sys/types.h>

namespace MyTinyRPC {

pid_t getPid();

pid_t getThreadId();

int64_t getNowMs();

}

#endif