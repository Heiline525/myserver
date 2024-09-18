#ifndef __MYSERVER_LOG_H__
#define __MYSERVER_LOG_H__

#include <string>
#include <stdint.h>
#include <sstream>
#include <fstream>
#include <memory>
#include <list> 
#include <vector>
#include <map>
#include "singleton.h"
#include <stdarg.h>
#include "util.h"

/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 */
#define LOG_LEVEL(logger, level)                                        \
    if (logger->getLevel() <= level)                                    \
        myserver::LogEventWrap(logger, myserver::LogEvent::ptr(         \
            new myserver::LogEvent(level, __FILE__, __LINE__, 0,        \
            myserver::GetThreadId(), myserver::GetFiberId(), time(0),   \
            "thread_i"))).getSS()    
 
#define LOG_DEBUG(logger) LOG_LEVEL(logger, myserver::LogLevel::DEBUG)
#define LOG_INFO(logger) LOG_LEVEL(logger, myserver::LogLevel::INFO)
#define LOG_WARN(logger) LOG_LEVEL(logger, myserver::LogLevel::WARN)
#define LOG_ERROR(logger) LOG_LEVEL(logger, myserver::LogLevel::ERROR)
#define LOG_FATAL(logger) LOG_LEVEL(logger, myserver::LogLevel::FATAL)

#define LOG_FMT_LEVEL(logger, level, fmt, ...)                          \
    if(logger->getLevel() <= level)                                     \
        myserver::LogEventWrap(logger, myserver::LogEvent::ptr(         \
            new myserver::LogEvent(level, __FILE__, __LINE__, 0,        \
            myserver::GetThreadId(), myserver::GetFiberId(), time(0),   \
            "thread_i"))).getEvent()->format(fmt, __VA_ARGS__)                                              
    
#define LOG_FMT_DEBUG(logger, fmt, ...) LOG_FMT_LEVEL(logger, myserver::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define LOG_FMT_INFO(logger, fmt, ...) LOG_FMT_LEVEL(logger, myserver::LogLevel::INFO, fmt, __VA_ARGS__)
#define LOG_FMT_WARN(logger, fmt, ...) LOG_FMT_LEVEL(logger, myserver::LogLevel::WARN, fmt, __VA_ARGS__)
#define LOG_FMT_ERROR(logger, fmt, ...) LOG_FMT_LEVEL(logger, myserver::LogLevel::ERROR, fmt, __VA_ARGS__)
#define LOG_FMT_FATAL(logger, fmt, ...) LOG_FMT_LEVEL(logger, myserver::LogLevel::FATAL, fmt, __VA_ARGS__)

#define ROOT_LOGGER() myserver::LoggerMgr::GetInstance()->getRoot()
#define LOGGER_NAME(name) myserver::LoggerMgr::GetInstance()->getLogger(name)

namespace myserver {

class Logger;
class LoggerManager;

// 日志级别
class LogLevel{
public:
    // 日志级别枚举
    enum Level {
            UNKNOWN = 0,// 未知级别
            DEBUG = 1,  // DEBUG 级别
            INFO = 2,   // INFO 级别
            WARN = 3,   // WARN 级别
            ERROR = 4,  // ERROR 级别
            FATAL = 5,  // FATAL 级别
        };

    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string str);
};


// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(LogLevel::Level level, const char* file, int32_t line, 
            uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, 
            uint64_t time, std::string threadName);

    LogLevel::Level getLevel() const { return m_level; }
    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elaspe; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_ss.str(); }
    std::stringstream& getSS() { return m_ss; }
    const std::string& getThreadName() const { return m_threadName; }

    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

private:
    LogLevel::Level m_level;        // 日志级别
    const char* m_file = nullptr;   // 文件名
    int32_t m_line = 0;             // 行号
    uint32_t m_elaspe = 0;          // 程序启动至现在的毫秒数
    uint32_t m_threadId = 0;        // 线程ID
    uint32_t m_fiberId = 0;         // 协程ID
    uint64_t m_time;                // 时间戳
    std::stringstream m_ss;     // 日志内容流
    std::string m_threadName;   // 线程名称
};

// 日志事件包装器
class LogEventWrap {
public:
  LogEventWrap(std::shared_ptr<Logger> logger, LogEvent::ptr e);
  ~LogEventWrap();
  LogEvent::ptr getEvent() const { return m_event; }
  std::stringstream &getSS() { return m_event->getSS(); }

private:
  std::shared_ptr<Logger> m_logger;
  LogEvent::ptr m_event;
};


// 日志格式化
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);

    // 初始化，根据m_pattern解析日志模板
    void init();
    std::string format(std::shared_ptr<Logger> logger, LogEvent::ptr event);   
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogEvent::ptr event);
    
    const std::string getPattern() const { return m_pattern; }
    bool isError() const { return m_error; }

    class FormatItem{
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() { }
        virtual void format(std::ostream& os, std::shared_ptr<Logger>, LogEvent::ptr event) = 0;
    };

private:
    std::string m_pattern;  // 日志格式模板
    std::vector<FormatItem::ptr> m_items;   // 日志格式解析后格式
    bool m_error = false;   // 日志格式错误
};


// 日志输出地
class LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;

    virtual ~LogAppender() { }

    virtual void log(std::shared_ptr<Logger>, LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;

    LogFormatter::ptr getFormatter() const { return m_formatter; }
    void setFormatter(LogFormatter::ptr val);

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

    
protected:
    LogFormatter::ptr m_formatter;
    LogLevel::Level m_level = LogLevel::DEBUG;
    bool m_hasFormatter = false;
};


// 日志器
class Logger :  public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    
    Logger(const std::string& name="root");

    void log(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
    const std::string &getName() const { return m_name; }

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();

    std::string toYamlString(); 
private:
    std::string m_name;         // 日志名称
    LogLevel::Level m_level;    // 日志级别
    std::list<LogAppender::ptr> m_appenders;    // Appenders集合
    LogFormatter::ptr m_formatter;  // 日志格式
    Logger::ptr m_root;         // 主日志器
};


// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogEvent::ptr event) override;
    std::string toYamlString() override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename, LogLevel::Level level = LogLevel::DEBUG);
    void log(Logger::ptr logger, LogEvent::ptr event) override;
    std::string toYamlString() override;
    bool reopen();
private:
    std::string m_filename;     // 输出文件名
    std::ofstream m_filestream; // 输出文件流
};

// 日志器管理类
class LoggerManager {
public:
    LoggerManager();

    void init();
    Logger::ptr getLogger(const std::string& name);
    Logger::ptr getRoot() const { return m_root; }

    std::string toYamlString();
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

// 日志器管理类单例模式
typedef Singleton<LoggerManager> LoggerMgr;

}


#endif