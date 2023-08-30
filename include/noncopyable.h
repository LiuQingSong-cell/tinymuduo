#ifndef NOCOPYABLE__H__
#define NOCOPYABLE__H__

class noncopyable
{
    public:
        noncopyable(noncopyable&) = delete;
        noncopyable& operator=(noncopyable&) = delete;
    protected:
        noncopyable() = default;
        ~noncopyable() = default;
};


#endif