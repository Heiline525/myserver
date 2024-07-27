
#include <unistd.h>
#include <sys/syscall.h>

#include "util.h"

namespace myserver {

pid_t GetThreadId(){
    return syscall(SYS_gettid);
}


}