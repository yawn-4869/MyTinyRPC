#ifndef MYTINYRPC_COMMON_LOG_H
#define MYTINYRPC_COMMON_LOG_H

#include <string>
#include <queue>
#include <semaphore.h>
#include <memory>
#include "tinyrpc/common/locker.h"
#include "tinyrpc/net/time_event.h"
#include "tinyrpc/net/eventloop.h"

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
    } \

#define INFOLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::INFO) { \
    MyTinyRPC::Logger::getGlobalLogger()->pushLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::INFO))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    } \

#define ERRORLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::ERROR) { \
    MyTinyRPC::Logger::getGlobalLogger()->pushLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::ERROR))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    } \

#define APPDEBUGLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::DEBUG){ \
    MyTinyRPC::Logger::getGlobalLogger()->pushAppLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::DEBUG))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    } \

#define APPINFOLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::INFO) { \
    MyTinyRPC::Logger::getGlobalLogger()->pushAppLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::INFO))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    } \

#define APPERRORLOG(str, ...) \
    if(MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() && MyTinyRPC::Logger::getGlobalLogger()->getLogLevel() <= MyTinyRPC::ERROR) { \
    MyTinyRPC::Logger::getGlobalLogger()->pushAppLog((new MyTinyRPC::LogEvent(MyTinyRPC::LogLevel::ERROR))->toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" \
    + MyTinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
    } \


enum LogLevel {
    UNKNOWN = 0,
    DEBUG = 1,
    INFO = 2,
    ERROR = 3
};
std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(std::string& log_level);

class AsyncLogger {
public:
    typedef std::shared_ptr<AsyncLogger> s_ptr;
    AsyncLogger(std::string file_name, std::string file_path, int max_size);
    ~AsyncLogger();
    static void* Loop(void*); // 核心循环函数
    void pushLogBuffer(std::vector<std::string> &vec);
    void stop();
    void flush();
    
private: 
    std::queue<std::vector<std::string>> m_buffer; // 缓冲区
    // m_file_path/m_file_name_yyyymmdd_0_log
    std::string m_file_name; // 日志输出文件名
    std::string m_file_path; // 日志输出路径
    int m_max_file_size{ 0 }; // 日志单个文件最大大小

    sem_t m_semaphore; // 信号量, 用于实现buffer区的写入
    pthread_cond_t m_conditon; // 条件变量, 用于实现buffer区的写入
    Mutex m_mutex; // 互斥锁, 与条件变量结合使用
    pthread_t m_thread; // 写日志线程

    std::string m_date; // 当前打开的日志文件的日期
    FILE* m_file_handler{ NULL }; // 当前打开的日志文件句柄
    bool m_reopen_flag{ false }; // 是否需要重新打开日志文件：文件打开失败？超出日志最大大小？文件名修改（天数改变）？
    int m_no{ 0 }; // 日志文件序号, 从0开始
    bool m_stop_flag{ false };
};

class Logger {
public: 
    Logger(LogLevel level, int type = 1);
    LogLevel getLogLevel() {
        return m_set_level;
    }
    void init();
    void pushLog(const std::string msg);
    void pushAppLog(const std::string msg);
    void log(); // 实现日志打印的方法
    void asyncLoop();

public:
    static Logger* getGlobalLogger();
    static void InitGlobalLogger(int type = 1);

private:
    LogLevel m_set_level; // 设置的日志级别，高于日志级别才打印
    std::vector<std::string> m_buffer; // 系統日志缓冲队列
    std::vector<std::string> m_app_buffer; // 業務日志缓冲队列
    Mutex m_mutex; // 系統日志缓冲队列的互斥锁
    Mutex m_app_mutex; // 業務日志缓冲队列的互斥锁
    AsyncLogger::s_ptr m_async_logger;
    AsyncLogger::s_ptr m_async_app_logger;
    TimeEvent::s_ptr m_time_event;
    EventLoop* m_event_loop;
    int m_type{ 0 }; // 0 -- 同步日志 1 -- 异步日志
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
    int32_t m_pid; // 进程号
    int32_t m_tid; // 线程号

    LogLevel m_level; // 日志级别
};

}

#endif