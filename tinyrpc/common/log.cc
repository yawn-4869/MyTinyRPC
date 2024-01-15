#include "log.h"
#include "util.h"
#include <sys/time.h>
#include <sstream>

namespace MyTinyRPC {

static Logger* g_logger = NULL;

Logger* Logger::getGlobalLogger() {
    if(g_logger) {
        return g_logger;
    }
    g_logger = new Logger();
    return g_logger;
}

void Logger::pushLog(const std::string msg) {
    m_buffer.push(msg);
}

void Logger::log() {
    while(!m_buffer.empty()) {
        std::string msg = m_buffer.front();
        m_buffer.pop();
        printf(msg.c_str());
    }
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
        << "[" << m_pid << ":" << m_tid << "]\t"
        << "[" << std::string(__FILE__) << ":" << __LINE__ << "]\t";

    // TODO: 请求信息的id
    
    return ss.str();
}

}