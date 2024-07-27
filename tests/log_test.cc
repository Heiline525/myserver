#include "myserver/log.h"

int main(int argc, char** argv){

    myserver::Logger::ptr logger(new myserver::Logger());
    
    return 0;
}