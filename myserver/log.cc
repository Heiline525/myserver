#include "log.h"
#include <iostream>
#include <map>
#include <functional>

namespace myserver {

const char* LogLevel::ToString(LogLevel::Level level){
    switch(level){
#define XX(name) case name : return #name;
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

// 各种FormatItem
class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << LogLevel::ToString(event->getLevel());
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FileNameFormatItem : public LogFormatter::FormatItem {
public:
    FileNameFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :m_string(str){    
    }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") { }
    void format(std::ostream& os, Logger::ptr logger, LogEvent::ptr event) override {
        os << "\t";
    }
};

LogEvent::LogEvent(LogLevel::Level level, const char* file, int32_t line, 
            uint32_t elapse, uint32_t thread_id, uint32_t fiber_id, 
            uint64_t time, std::string threadName)
            :m_level(level), m_file(file), m_line(line),
             m_elaspe(elapse), m_threadId(thread_id), m_fiberId(fiber_id), 
             m_time(time), m_threadName(threadName){
}

void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

LogEventWrap::LogEventWrap(std::shared_ptr<Logger> logger, LogEvent::ptr event)
    :m_logger(logger), m_event(event){
}

LogEventWrap::~LogEventWrap(){
    m_logger->log(m_event); 
}


Logger::Logger(const std::string& name)
    :m_name(name){
    m_level = LogLevel::DEBUG;
}

void Logger::addAppender(LogAppender::ptr appender){
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it){
        if (*it == appender){
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogEvent::ptr event){
    auto self = shared_from_this();
    for (auto& appender : m_appenders){
        appender->log(self, event);
    }
}

LogAppender::LogAppender(LogLevel::Level level)
    :m_level(level) {
    m_formatter = std::make_shared<LogFormatter>();
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogEvent::ptr event) {
    if (event->getLevel() >= m_level){
        std::cout << m_formatter->format(logger, event);
    }
}

FileLogAppender::FileLogAppender(const std::string& filename, LogLevel::Level level)
    :LogAppender(level), m_filename(filename) {
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogEvent::ptr event) {
    if (event->getLevel() >= m_level){
        if(!m_formatter->format(m_filestream, logger, event)) {
            std::cout << "error" << std::endl;
        }
    }
}

bool FileLogAppender::reopen(){
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !m_filestream;
}

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern) {
    init();     // 构造时初始化解析pattern
}

std::string LogFormatter::format(Logger::ptr logger, LogEvent::ptr event){
    std::stringstream ss;
    for (auto &m_item : m_items){
        m_item->format(ss, logger, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogEvent::ptr event) {
    for(auto& i : m_items) {
        i->format(ofs, logger, event);
    }
    return ofs;
}

void LogFormatter::init(){
    // 解析形如：%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
    std::vector<std::tuple<std::string, std::string, int>> vec; // str, format, type
    std::string nstr;   // 解析后的string
    for (size_t i = 0; i < m_pattern.size(); ++i){
        // 若m_pattern[i] != %,nstr增加m_pattern[i]
        if (m_pattern[i] != '%'){
            nstr.append(1, m_pattern[i]);
            continue;
        }
        // 若m_pattern[i] == m_pattern[i+1] == %，nstr增加%
        if (i + 1 < m_pattern.size()){
            if (m_pattern[i + 1] == '%'){
                nstr.append(i, '%');
                continue;
            }
        }
        // 若m_pattern[i] == %, m_pattern[i+1] != %，则从i+1开始解析
        size_t n = i + 1;
        size_t fmt_begin = 0;
        int fmt_status = 0;     // 是否解析{}内的内容：若遇到{，未遇到}，则值为1
        std::string str;
        std::string fmt;    // {}中的字符
        while (n < m_pattern.size()){
            // 未进入{}，不是字母，不是'{'与'}'；则为'['、']'、':'字符时
            if (!fmt_status && !isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'){
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0){
                if (m_pattern[n] == '{'){
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1; // 进入 {
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1){
                if (m_pattern[n] == '}'){
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0; // 离开 {
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size()){
                if (str.empty()){
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0){
            if (!nstr.empty()){
                // 保存其他字符：'['  ']'  ':'
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1){
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty()){
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}

        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FileNameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX

    };

    for (auto& i : vec){
        if (std::get<2>(i) == 0){
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()){
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }

}

_LoggerManager::_LoggerManager(){
    init();
}

void _LoggerManager::init(){
    Logger::ptr logger = std::make_shared<Logger>("root");
    m_loggers.insert(std::make_pair("root", logger));
}

Logger::ptr _LoggerManager::getLogger(const std::string& name){
    auto iter = m_loggers.find(name);
    if (iter == m_loggers.end()){
        // 日志器不存在就返回全局默认日志器
        return m_loggers.find("root")->second;
    }
    return iter->second;
}

Logger::ptr _LoggerManager::getRoot(){
    return m_loggers.find("root")->second;
}

}

