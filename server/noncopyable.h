#ifndef _NONCOPY_H_
#define _NONCOPY_H_

class Noncopyable
{
public:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable& ) = delete;
    Noncopyable& operator=(const Noncopyable& ) = delete;

};

#endif


