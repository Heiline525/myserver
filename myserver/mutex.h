#ifndef __MYSERVER_MUTEX_H__
#define __MYSERVER_MUTEX_H__

#include "noncopyable.h"
#include <semaphore.h>
#include <stdint.h>
#include <memory>
#include <atomic>

namespace myserver {

// 信号量 和 锁 类：在构造时进行初始化，在析构时进行销毁；隐藏初始化与销毁操作

// 信号量
class Semaphore : Noncopyable {
public:
     /**
     * @brief 构造函数
     * @param[in] count 信号量值的大小
     */
    Semaphore(uint32_t count=0);
    ~Semaphore();

    void wait();
    void notify();

private:
    sem_t m_semaphore;
};

// 1. 自主上锁解锁：遵循RAII准则，提供锁的局部控制类，在栈内创建局部控制类锁时上锁，超出作用域时解锁；
// 2. 统一上锁解锁：抽象类：单一锁、读锁、写锁的统一
// 局部锁 模板类
template<class T>
struct ScopedLockImpl {
public:
    // 构造函数，自动上锁
    ScopedLockImpl(T& mutex)
        : m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }

    // 析构函数, 自动释放锁
    ~ScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;     // 锁
    bool m_locked;  // 是否上锁
};

// 读锁 模板类
template<class T>
struct ReadScopedLockImpl {
public:
    // 构造函数，自动上锁
    ReadScopedLockImpl(T& mutex)
        : m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    // 析构函数, 自动释放锁
    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;     // mutex
    bool m_locked;  // 是否已上锁
};

// 局部写锁 模板类
template<class T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        : m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;     // Mutex
    bool m_locked;  // 是否已上锁
};

// 互斥锁
class Mutex : Noncopyable {
public: 
    typedef ScopedLockImpl<Mutex> Lock;     // 装配局部锁

    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;    // 互斥锁
};


// 空锁
class NullMutex : Noncopyable{
public:
    typedef ScopedLockImpl<NullMutex> Lock; // 装配局部锁

    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};


// 读写锁
class RWMutex : Noncopyable{
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;    // 装配局部读锁
    typedef WriteScopedLockImpl<RWMutex> WriteLock;  // 装配局部写锁

    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;    // 读写锁
};


// 空读写锁(用于调试)
class NullRWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<NullMutex> ReadLock;  // 装配局部读锁
    typedef WriteScopedLockImpl<NullMutex> WriteLock;    // 装配局部写锁

    NullRWMutex() {}
    ~NullRWMutex() {}
    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};


// 自旋锁
class Spinlock : Noncopyable {
public:
    typedef ScopedLockImpl<Spinlock> Lock;  // 装配局部锁
    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }

    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;     // 自旋锁
};

// 原子锁（自定义的自旋锁）
class CASLock : Noncopyable {
public:
    typedef ScopedLockImpl<CASLock> Lock;   // 局部锁

    CASLock() {
        m_mutex.clear();
    }

    ~CASLock() {
    }

    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }

    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    volatile std::atomic_flag m_mutex;  // 原子状态
};


}

#endif