#include "log.h"
#include "util.h"
#include <sys/time.h>
#include <sstream>
#include "config.h"

namespace MyTinyRPC {

static Logger* g_logger = NULL;

Logger* Logger::getGlobalLogger() {
    return g_logger;
}

void Logger::InitGlobalLogger() {
    std::string global_log_level = Config::GetGloabalConfig()->getGlobalLogLevel();
    printf("Init log level [%s]\n", global_log_level.c_str());
    g_logger = new Logger(StringToLogLevel(global_log_level));
}

void Logger::pushLog(const std::string msg) {
    // m_buffer.push(msg);
    printf(msg.c_str());
}

void Logger::log() {
    // std::queue<std::string> tmp;
    // ScopeLocker<Mutex> locker(m_mutex);
    // m_buffer.swap(tmp);
    // locker.unlock();

    // while(!tmp.empty()) {
    //     std::string msg = tmp.front();
    //     tmp.pop();

    //     // TODO: 输出到终端要改为输出到日志文件
    //     printf(msg.c_str());
    // }
}

std::string LogLevelToString(LogLevel level) {
    switch (level)
    {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

LogLevel StringToLogLevel(std::string& log_level) {
    if(log_level == "DEBUG") {
        return LogLevel::DEBUG;
    }else if(log_level == "INFO") {
        return LogLevel::INFO;
    }else if(log_level == "ERROR") {
        return LogLevel::ERROR;
    }
    return LogLevel::UNKNOWN;
}

std::string LogEvent::toString() {
    // 时间戳字符串
    struct timeval now_time;
    gettimeofday(&now_time, nullptr);
    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);
    char buf[128];
    strftime(&buf[0], sizeof(buf), "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf);
    int ms = now_time.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);

    // 进程与线程字符串
    m_pid = getPid();
    m_tid = getThreadId();

    std::stringstream ss;
    ss << "[" << LogLevelToString(m_level) << "]\t" 
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_tid << "]\t";
    // TODO: 请求信息的id
    
    return ss.str();
}

}