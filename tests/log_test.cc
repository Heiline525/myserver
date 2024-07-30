#include "myserver/log.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

void TEST_macroDefaultLogger(){
    auto logger = GET_ROOT_LOGGER();
    myserver::StdoutLogAppender::ptr stdApd(new myserver::StdoutLogAppender());
    logger->addAppender(stdApd);
    
    myserver::FileLogAppender::ptr FileApd(
        new myserver::FileLogAppender("./log.txt"));
    logger->addAppender(FileApd);

    std::cout << ">>>>>>>>>>>>>>>>>>>>>>> START <<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
    LOG_DEBUG(logger) << "debug message";
    LOG_INFO(logger) << "info message";
    LOG_WARN(logger) << "warn message";
    LOG_ERROR(logger) << "error message";
    LOG_FATAL(logger) << "fatal message";

    LOG_FMT_DEBUG(logger, "%s", "this is a DEBUG message");
    LOG_FMT_INFO(logger, "%s", "this is a INFO message");
    LOG_FMT_WARN(logger, "%s", "this is a WARN message");
    LOG_FMT_ERROR(logger, "%s", "this is a ERROR message");
    LOG_FMT_FATAL(logger, "%s", "this is a FATAL message");
    std::cout << ">>>>>>>>>>>>>>>>>>>>>>>> END <<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
}

int main(int argc, char** argv){    
    TEST_macroDefaultLogger();
    return 0;
}