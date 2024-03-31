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
    g_logger = new Logger(StringToLogLevel(global_log_level));
    g_logger->init();
}

Logger::Logger(LogLevel level) : m_set_level(level){
                      
}

void Logger::init() {
    m_async_logger = std::make_shared<AsyncLogger>(Config::GetGloabalConfig()->m_log_file_name, 
                                                    Config::GetGloabalConfig()->m_log_file_path,
                                                    Config::GetGloabalConfig()->m_log_max_file_size);
    m_time_event = std::make_shared<TimeEvent>(Config::GetGloabalConfig()->m_async_log_interval, true, 
                                                std::bind(&Logger::asyncLoop, this));   
    EventLoop::GetCurrentEventLoop()->addTimerEvent(m_time_event);
}

void Logger::pushLog(const std::string msg) {
    // m_buffer.push(msg);
    // printf(msg.c_str());
    ScopeLocker<Mutex> lck(m_mutex);
    m_buffer.push_back(msg);
    lck.unlock();
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

void Logger::asyncLoop() {
    // 同步m_buffer到async_logger的buffer队尾
    std::vector<std::string> tmp;
    ScopeLocker<Mutex> lck(m_mutex);
    tmp.swap(m_buffer);
    lck.unlock();

    if(tmp.empty()) {
        return;
    }
    // printf("logger asyncLoop: push tmp.size():%d, msg: %s\n", tmp.size(), tmp[0].c_str());
    m_async_logger->pushLogBuffer(tmp);
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

AsyncLogger::AsyncLogger(std::string file_name, std::string file_path, int max_size)
    : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size) {
    sem_init(&m_semaphore, 0, 0);
    pthread_create(&m_thread, NULL, &AsyncLogger::Loop, this);
    // assert(pthread_cond_init(&m_conditon, NULL) == 0);
    sem_wait(&m_semaphore);
}

AsyncLogger::~AsyncLogger() {

}

void* AsyncLogger::Loop(void* arg) {
    // 定时将buffer中的全部数据打印到文件中, 然后线程睡眠, 直到定时任务再次唤醒
    AsyncLogger* async_logger = reinterpret_cast<AsyncLogger*>(arg);
    assert(pthread_cond_init(&async_logger->m_conditon, NULL) == 0);
    sem_post(&async_logger->m_semaphore);

    while(true) {
        ScopeLocker<Mutex> lck(async_logger->m_mutex);
        while(async_logger->m_buffer.empty()) {
            // 等待缓冲区内有数据
            pthread_cond_wait(&(async_logger->m_conditon), async_logger->m_mutex.getMutex());
        }

        // printf("AsyncLogger::Loop: pthread_cond_wait end, start loop thread\n");
        // 为解决缓冲区读出与写入速度不匹配的问题
        // 即有可能在缓冲区内的内容还未全部写入到日志文件时, 又有新的数据要写入到缓冲区
        // 因此使用队列作为缓冲区的数据结构, 每次只读取队首的数据
        std::vector<std::string> tmp;
        tmp.swap(async_logger->m_buffer.front());
        async_logger->m_buffer.pop();
        lck.unlock();

        // 获取当前日期
        time_t now;
        time(&now);
        struct tm now_time;
        localtime_r(&now, &now_time);
        char date[32];
        strftime(date, 32, "%Y%m%d", &now_time);

        // 判断日志文件是否需要重新被打开: 更换日期?超出大小?文件打开失败?
        if(std::string(date) != async_logger->m_date) {
            async_logger->m_reopen_flag = true;
            async_logger->m_date = std::string(date);
            async_logger->m_no = 0;
        }

        if (async_logger->m_file_handler == NULL) {
            async_logger->m_reopen_flag = true;
        }

        std::stringstream ss;
        ss << async_logger->m_file_path << async_logger->m_file_name << "_"
        << async_logger->m_date << "_";
        
        std::string log_file_name = ss.str() + std::to_string(async_logger->m_no) + ".log";
        if(async_logger->m_reopen_flag) {
            if(async_logger->m_file_handler) {
                fclose(async_logger->m_file_handler);
            }
            async_logger->m_file_handler = fopen(log_file_name.c_str(), "a");
            async_logger->m_reopen_flag = false;
        }
        
        if(ftell(async_logger->m_file_handler) >= async_logger->m_max_file_size) {
            fclose(async_logger->m_file_handler);
            log_file_name = ss.str() + std::to_string(async_logger->m_no++) + ".log";
            async_logger->m_file_handler = fopen(log_file_name.c_str(), "a");

            async_logger->m_reopen_flag = false;
        }

        // 写入日志文件
        for(auto &msg : tmp) {
            if(!msg.empty()) {
                fwrite(msg.c_str(), 1, msg.length(), async_logger->m_file_handler);
            }
        }
        fflush(async_logger->m_file_handler);

        // 输出完毕后判断是否需要停止
        if(async_logger->m_stop_flag) {
            return NULL;
        }
    }
    return NULL;
}

void AsyncLogger::pushLogBuffer(std::vector<std::string> &vec) {
    ScopeLocker<Mutex> lck(m_mutex);
    m_buffer.push(vec);
    pthread_cond_signal(&m_conditon);
    lck.unlock();

    // 唤醒线程
    // printf("pushLogBuffer success, pthread_con_signal...\n");
}

void AsyncLogger::stop() {
    m_stop_flag = true;
}

void AsyncLogger::flush() {
    if(m_file_handler) {
        fflush(m_file_handler);
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

    std::stringstream ss;
    ss << "[" << LogLevelToString(m_level) << "]\t" 
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_tid << "]\t";
    // TODO: 请求信息的id
    
    return ss.str();
}

}