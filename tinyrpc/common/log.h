#ifndef MYTINYRPC_COMMON_LOG_H
#define MYTINYRPC_COMMON_LOG_H

#include <string>
#include <queue>
#include "locker.h"

namespace MyTinyRPC {

template <typename... Args>
std::string formatString(const char* str, Args&&... args) {
    // 获取格式化字符串的长度
    int size = snprintf(nullptr, 0, str, args...);
    std::string result;
    if(size > 0) {
        // 手动调整缓冲区的大小
        result.resize(size);
        snprintf(&result[0], size+1, str, args...);
    }
    return result;
}

#define DEBUGLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::DEBUG){ \
    MyTinyRPC::Logger::getGlobalLogger()->pushLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::DEBUG))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    MyTinyRPC::Logger::getGlobalLogger()->log(); \
    } \

#define INFOLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::INFO) { \
    MyTinyRPC::Logger::getGlobalLogger()->pushLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::INFO))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    MyTinyRPC::Logger::getGlobalLogger()->log(); \
    } \

#define ERRORLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::ERROR) { \
    MyTinyRPC::Logger::getGlobalLogger()->pushLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::ERROR))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    MyTinyRPC::Logger::getGlobalLogger()->log(); \
    } \


enum LogLevel {
    UNKNOWN = 0,
    DEBUG = 1,
    INFO = 2,
    ERROR = 3
};
std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(std::string& log_level);

class Logger {
public: 
    Logger(LogLevel level) : m_set_level(level){}
    LogLevel getLogLevel() {
        return m_set_level;
    }
    void pushLog(const std::string msg);
    void log(); // 实现日志打印的方法

public:
    static Logger* getGlobalLogger();
    static void InitGlobalLogger();

private:
    LogLevel m_set_level; // 设置的日志级别，高于日志级别才打印
    std::queue<std::string> m_buffer; // 日志缓冲队列
    Mutex m_mutex; // 日志缓冲队列的互斥锁
};

class LogEvent {
public:
    LogEvent(LogLevel level) : m_level(level){}
    std::string getFileName() const {
        return m_file_name;
    }

    LogLevel getLogLevel() const {
        return m_level;
    }

    std::string toString();
private:
    std::string m_file_name; // 输出文件
    int32_t m_file_line; // 行号
    int32_t m_pid; // 进程号
    int32_t m_tid; // 线程号
    std::string m_timestamp; // 时间

    LogLevel m_level; // 日志级别
};

}

#endif