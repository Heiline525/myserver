#ifndef __MYSERVER_LOG_H__
#define __MYSERVER_LOG_H__

#include <string>
#include <stdint.h>
#include <sstream>
#include <memory>

namespace myserver {

// 日志级别
class LogLevel{
public:
    enum Level {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5,
        };
};


// 日志事件
class LogEvent {
public:
    LogEvent();

private:
    const char* m_file = nullptr;   // 文件名
    int32_t m_line = 0;         // 行号
    uint32_t m_threadId = 0;    // 线程ID
    uint32_t m_fiberId = 0;     // 协程ID
    uint64_t m_time;            // 时间戳
    std::string m_content;      
};

// 日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
    
    Logger(const std::string& name="UNKNOWN");
    ~Logger();

    const std::string getName() const { return m_name; }
    void setLevel(LogLevel::Level val) { m_level = val; }
    std::stringstream& getSS() { return m_ss; }
    void log();

private:
    std::string m_name;
    LogLevel::Level m_level;
    std::stringstream m_ss;
};

// 日志输出地
class LogAppender {
public:
    virtual ~LogAppender();
private:
};

// 日志格式化
class LogFormatter {
};

}


#endif