#include "log.h"
#include <iostream>
#include <map>
#include <functional>
#include "config.h"

namespace myserver {

const char* LogLevel::ToString(LogLevel::Level level){
    switch(level){
#define XX(name)        \
    case name :         \
        return #name;   \
        break;

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

LogLevel::Level LogLevel::FromString(const std::string str) {
#define XX(level, v)            \
    if(str == #v) {             \
        return LogLevel::level; \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOWN;
#undef XX

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
    :m_name(name)
    ,m_level(LogLevel::DEBUG) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()){
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it){
        if (*it == appender){
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders() {
    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr val) {
    m_formatter = val;

    for(auto& i : m_appenders) {
        if(!i->m_hasFormatter) {
            i->m_formatter = m_formatter;
        }
    }
}

void Logger::setFormatter(const std::string& val) {
    myserver::LogFormatter::ptr new_fmt(new myserver::LogFormatter(val));
    if (new_fmt->isError()) {
        std::cout << "Logger setFormatter name=" << m_name
                  << " value=" << val << " invalid formatter"
                  << std::endl;
        return;
    }
    setFormatter(new_fmt);
}   

LogFormatter::ptr Logger::getFormatter() {
    return m_formatter;
}

std::string Logger::toYamlString() {
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }

    for(auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void Logger::log(LogEvent::ptr event){
    if (event->getLevel() >= m_level){
        auto self = shared_from_this();
        if (!m_appenders.empty()) {
            for (auto& appender : m_appenders){
                appender->log(self, event);
            }
        } else if (m_root) {
            m_root->log(event); // appenders为空，则写在root中
        }
        
    }
}

void LogAppender::setFormatter(LogFormatter::ptr val) {
    m_formatter = val;
    if(m_formatter) {
        m_hasFormatter = true;
    } else {
        m_hasFormatter = false;
    }
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogEvent::ptr event) {
    if (event->getLevel() >= m_level){
        m_formatter->format(std::cout, logger, event);
    }
}

std::string StdoutLogAppender::toYamlString() {
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string& filename, LogLevel::Level level)
    :m_filename(filename) {
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

std::string FileLogAppender::toYamlString() {
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::UNKNOWN) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
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
            m_error = true;
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
                m_error = true;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }

}

LoggerManager::LoggerManager(){
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

    m_loggers[m_root->m_name] = m_root;
    init();
}

Logger::ptr LoggerManager::getLogger(const std::string& name){
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    } 

    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

struct LogAppenderDefine {
    int type = 0;   // 1: File, 2: Stdout     
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& rhs) const {   // 定义==在Config管理时使用
        return type == rhs.type
            && level == rhs.level
            && formatter == rhs.formatter
            && file == rhs.file;
    }
};

struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& rhs) const {
        return name == rhs.name
            && level == rhs.level
            && formatter == rhs.formatter
            && appenders == rhs.appenders;
    }

    bool operator<(const LogDefine& rhs) const {
        return name < rhs.name; // 重定义<，为config中std::set<LogDefine>排序使用
    }
};

// 类型转换偏特化： string --> class LogDefine
template<>
class LexicalCast<std::string, std::set<LogDefine> > {
public:
    std::set<LogDefine> operator()(const std::string& v) { 
        YAML::Node node = YAML::Load(v);
        typename std::set<LogDefine> vec;
        for (size_t i = 0; i < node.size() ; ++i) {
            auto n = node[i];
            if (!n["name"].IsDefined()) {
                std::cout << "log config error: name is null, " << n << std::endl;
                continue;         
            }
             
            LogDefine logDef;
            logDef.name = n["name"].as<std::string>();
            logDef.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
            if(n["formatter"].IsDefined()) {
                logDef.formatter = n["formatter"].as<std::string>();
            }

            if(n["appenders"].IsDefined()) {
                for(size_t x = 0; x < n["appenders"].size(); ++x) {
                    auto apdr = n["appenders"][x];
                    if(!apdr["type"].IsDefined()) {
                        std::cout << "log config error: appender type is null, " << apdr << std::endl;
                        continue;
                    }
                    std::string type = apdr["type"].as<std::string>();
                    LogAppenderDefine logApdrDef;
                    if(type == "FileLogAppender") {
                        logApdrDef.type = 1;
                        if(!apdr["file"].IsDefined()) {
                            std::cout << "log config error: fileappender file is null, " << apdr << std::endl;
                            continue;
                        }
                        logApdrDef.file = apdr["file"].as<std::string>();
                        if(apdr["formatter"].IsDefined()) {
                            logApdrDef.formatter = apdr["formatter"].as<std::string>();
                        } 
                    } else if(type == "StdoutLogAppender") {
                        logApdrDef.type = 2;
                        if(apdr["formatter"].IsDefined()) {
                            logApdrDef.formatter = apdr["formatter"].as<std::string>();
                        }
                    } else {
                        std::cout << "log config error: appender type is invalid, " << apdr << std::endl;
                        continue;
                    }
    
                    logDef.appenders.push_back(logApdrDef);
                }
            }
            vec.insert(logDef);
        }
        return vec;
    }

};

// 类型转换偏特化：class LogDefine --> string
template<>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator()(const LogDefine& logDef) {
        YAML::Node n;
        n["name"] = logDef.name;
        if(logDef.level != LogLevel::UNKNOWN) {
            n["level"] = LogLevel::ToString(logDef.level);
        }
        if(!logDef.formatter.empty()) {
            n["formatter"] = logDef.formatter;
        }

        for(auto& appender : logDef.appenders) {
            YAML::Node na;
            if(appender.type == 1) {
                na["type"] = "FileLogAppender";
                na["file"] = appender.file;
            } else if(appender.type ==    2) {
                na["type"] = "StdoutLogAppender";
            }
            if(appender.level != LogLevel::UNKNOWN) {
                na["level"] = LogLevel::ToString(appender.level);
            }

            if(!appender.formatter.empty()) {
                na["formatter"] = appender.formatter;
            }

            n["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << n;
        return ss.str();
    }
};

myserver::ConfigVar<std::set<LogDefine>>::ptr g_log_defines = 
    myserver::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
    LogIniter() {
        g_log_defines->addListener(0xF1E231, 
            [](const std::set<LogDefine> &old_value, const std::set<LogDefine>& new_value){
            LOG_INFO(ROOT_LOGGER()) << "on_logger_config_changed";
            for (auto& i : new_value) {
                auto it = old_value.find(i);
                myserver::Logger::ptr logger;
                if (it == old_value.end()) {
                    // 新增日志 logger
                    logger = LOGGER_NAME(i.name);
                } else {
                    if (!(i == *it)) {
                        // 修改日志 logger
                        logger = LOGGER_NAME(i.name);
                    } else {
                        continue;
                    }
                }
                logger->setLevel(i.level);
                if (!i.formatter.empty()) {
                    logger->setFormatter(i.formatter);
                }

                logger->clearAppenders();
                for (auto& a : i.appenders) {
                    myserver::LogAppender::ptr ap;
                    if(a.type == 1) {
                        ap.reset(new FileLogAppender(a.file));
                    } else if(a.type == 2) {
                        ap.reset(new StdoutLogAppender);
                    }
                    ap->setLevel(a.level);
                    if(!a.formatter.empty()) {
                        LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                        if(!fmt->isError()) {
                            ap->setFormatter(fmt);
                        } else {
                            std::cout << "log.name=" << i.name << " appender type=" << a.type
                                      << " formatter=" << a.formatter << " is invalid" << std::endl;
                        }
                    }
                    logger->addAppender(ap);
                }
            }

            for (auto& i : old_value) {
                auto it = new_value.find(i);
                if (it == new_value.end()){
                    // 删除日志 logger
                    auto logger = LOGGER_NAME(i.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppenders();
                }
            }
                                             
        });
    }
};

static LogIniter __log_init;

std::string LoggerManager::toYamlString() {
    YAML::Node node;
    for(auto& i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void LoggerManager::init(){
}

}

