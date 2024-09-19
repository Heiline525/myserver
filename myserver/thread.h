#ifndef __MYSERVER_THREAD_H__
#define __MYSERVER_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include "mutex.h"

// pthread_t 线程句柄
// C++11中 std::thread，基于pthread实现，未区分读写锁；兼顾写少读多的场景
// 诉求：1. 线程利用名字区分 2. 简化pthread_create方法 3. 自动控制join，detach自动执行
// 线程作为协程的容器
namespace myserver{

class Thread {
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    pid_t getId() const { return m_id;}
    const std::string& getName() const { return m_name;}

    void join();

public:
    static Thread* GetThis();                       // 获取当前线程
    static const std::string& GetName();            // 获取当前的线程名称
    static void SetName(const std::string& name);   // 设置当前的线程名称

private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;

    static void* run(void* arg);
    
private:
    pid_t m_id = -1;                // 线程id
    pthread_t m_thread = 0;         // 线程结构
    std::function<void()> m_cb;     // 线程执行函数
    std::string m_name;             // 线程名称
    Semaphore m_semaphore;          // 信号量
};

} // end of namespac e myserver

#endif