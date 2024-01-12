#include "log.h"
#include "util.h"
#include <sys/time.h>
#include <sstream>

namespace MyTinyRPC {

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

void Logger::log(LogEvent event) {
    if(event.getLogLevel() < m_set_level) {
        return;
    }
    
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

    // TODO: 文件名与行号字符串
    std::stringstream ss;
    ss << "[" << LogLevelToString(m_level) << "]\t" 
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_tid << "]\t";

    // TODO: 请求信息的id
    
    return ss.str();
}

}