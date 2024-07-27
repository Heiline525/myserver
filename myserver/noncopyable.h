#ifndef __MYSERVER_NONCOPYABLE_H__
#define __MYSERVER_NONCOPYABLE_H__

namespace myserver {

class Noncopyable {
public:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};

}

#endif
