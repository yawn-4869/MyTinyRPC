#ifndef MYTINYRPC_COMMON_UTIL_H
#define MYTINYRPC_COMMON_UTIL_H

#include <unistd.h>
#include <sys/types.h>

namespace MyTinyRPC {

pid_t getPid(); // 获取进程id

pid_t getThreadId(); // 获取线程id

int64_t getNowMs(); // 获取当前时间

int32_t getInt32FromNetByte(const char* buf); // 转成网络字节序

}

#endif