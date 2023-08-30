#include "Poller.h"
#include "EpollPoller.h"

#include <cstdlib>

Poller* Poller::newDefaultPoller(EventLoop * loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        // return EpollPoller
        // add code
        return new EpollPoller(loop);
    }
}