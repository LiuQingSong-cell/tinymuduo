#ifndef CURRENTTHREAD_H__
#define CURRENTTHREAD_H__

#include <sys/syscall.h>
#include <unistd.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid;
    
    void cachedTid();

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0)) // more likely t_cachedTid == 0
        {
            cachedTid();
        }
        return t_cachedTid;
    }
}


#endif