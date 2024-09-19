#include "thread.h"
#include "log.h"
#include "util.h"

namespace myserver {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOWN";

static myserver::Logger::ptr g_logger = LOGGER_NAME("system");

Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if(name.empty()) {
        return;
    }
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string& name)
    : m_cb(cb), m_name(name) {
    if(name.empty()) {
        m_name = "UNKNOWN";
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);    // 传入this 当前线程
    if(rt) {
        LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                            << " name=" << name;
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();     // 信号量等待
}

Thread::~Thread() {
    if(m_thread) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if(m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if(rt) {
            LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                                << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;  // arg是this指针
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = myserver::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());   // 查看线程名称

    std::function<void()> cb;   
    cb.swap(thread->m_cb);      // 将回调函数转移至栈内，并将原回调置空 cb = m_cb; m_cb = nullptr;

    thread->m_semaphore.notify();   // 信号量抛出

    cb();
    return 0;
}


}