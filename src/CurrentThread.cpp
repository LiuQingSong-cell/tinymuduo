#include "CurrentThread.h"


namespace CurrentThread
{
    thread_local int t_cachedTid = 0;

    void cachedTid()
    {
        if (t_cachedTid == 0)
        {
            t_cachedTid = static_cast<int>(::syscall(SYS_gettid));
        }
    }

}