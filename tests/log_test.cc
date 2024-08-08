#include "myserver/log.h"
#include <iostream>

void TEST_macroLogger(){
    myserver::Logger::ptr logger(new myserver::Logger);
    logger->addAppender(myserver::LogAppender::ptr(new myserver::StdoutLogAppender));
    
    myserver::FileLogAppender::ptr FileApd(new myserver::FileLogAppender("./log.txt"));
    logger->addAppender(FileApd);

    myserver::LogFormatter::ptr fmt(new myserver::LogFormatter("%d%T%p%T%m%n"));
    FileApd->setFormatter(fmt);
    FileApd->setLevel(myserver::LogLevel::ERROR);

    std::cout << ">>>>>>>>>>>>>>>>>>>>>>> START <<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
    LOG_DEBUG(logger) << "Test DEBUG macro";
    LOG_INFO(logger) << "Test INFO macro";
    LOG_WARN(logger) << "Test WARN macro";
    LOG_ERROR(logger) << "Test ERROR macro";
    LOG_FATAL(logger) << "Test FATAL macro";

    LOG_FMT_DEBUG(logger, "Test macro fmt %s", "DEBUG");
    LOG_FMT_INFO(logger, "Test macro fmt %s", "INFO");
    LOG_FMT_WARN(logger, "Test macro fmt %s", "WARN");
    LOG_FMT_ERROR(logger, "Test macro fmt %s", "ERROR");
    LOG_FMT_FATAL(logger, "Test macro fmt %s", "FATAL");

    LOG_DEBUG(ROOT_LOGGER()) << "Test macro Root_Logger";
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>> END <<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
}

int main(int argc, char** argv){    
    TEST_macroLogger();
    return 0;
}