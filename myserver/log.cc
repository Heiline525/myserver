#include <iostream>
#include "log.h"

namespace myserver {

Logger::Logger(const std::string& name)
    :m_name(name), m_level(LogLevel::Level::INFO){
}
Logger::~Logger(){ 
    log();
}

void Logger::log(){
    std::cout << "[" << m_name << "]    "
              << "[" << m_level << "]    " 
              << m_ss.str()
              << std::endl;
}

}
