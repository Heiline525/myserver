#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include "thread.h"
#include "mutex.h"

myserver::Semaphore semaphore;
int count = 0;

typedef myserver::Mutex LockType;
// typedef myserver::Spinlock LockType;
// typedef myserver::RWMutex LockType;

LockType gLock;
void* addFun(void* arg) {
    for (int i = 0; i < 100000; ++i) {
        LockType::Lock lock(gLock);
        // LockType::ReadLock lock(gLock);
        // LockType::WriteLock lock(gLock);
        count++;
    }
    return nullptr;
}

void testMutex() {
    pthread_t tid1;
    pthread_t tid2;

    pthread_create(&tid1, nullptr, addFun, nullptr);
    pthread_create(&tid2, nullptr, addFun, nullptr);

    pthread_join(tid1, nullptr);
    pthread_join(tid2, nullptr);

    std::cout << count << std::endl;
}

void testThreadAddFun() { 
    std::function<void()> cb = []() {
        for (int i = 0; i < 100000; ++i) {
            LockType::Lock lock(gLock);
            count++;
        }
    };

    myserver::Thread::ptr thr1(new myserver::Thread(cb, "THREAD_1"));
    myserver::Thread::ptr thr2(new myserver::Thread(cb, "THREAD_2"));

    thr1->join();
    thr2->join();
    std::cout << count << std::endl;
}

void testThreadName() {
    std::function<void()> cb = []() {
        {
            LockType::Lock lock(gLock);
            std::cout << myserver::Thread::GetName() << std::endl;
        }
        
    };
    myserver::Thread::ptr thr1(new myserver::Thread(cb, "THREAD_1"));
    myserver::Thread::ptr thr2(new myserver::Thread(cb, "THREAD_2"));

    {
        LockType::Lock lock(gLock);
        std::cout << "线程创建完成" << std::endl;
    }
   
    thr1->join();
    thr2->join();
}

int main(int argc, char** argv) {
    // testMutex();
    // testThreadAddFun();
    testThreadName();
    return 0;
}
