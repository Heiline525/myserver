#ifndef __MYSERVER_UTIL_H__
#define __MYSERVER_UTIL_H__

#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>

namespace myserver {

pid_t GetThreadId();
uint32_t GetFiberId();

}



#endif
