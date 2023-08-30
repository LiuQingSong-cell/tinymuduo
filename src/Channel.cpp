#include "Channel.h"
#include <sys/epoll.h>
#include <cassert>
#include "EventLoop.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; // OOB
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd_):
    loop_(loop),
    fd_(fd_),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
    {}

Channel::~Channel()
{

}

void Channel::tie(const std::shared_ptr<void>& obj) // 右值 或者 const std::shared_ptr<void>& obj
{
    tie_ = obj;
    tied_ = true;
}

// push update to poller
void Channel::update()
{
    addedToLoop_ = true;
    // add code
    loop_->updateChannel(this);
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedToLoop_ = false;
    // add code
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveStamp)
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock(); // promote
        if (guard)
        {
            handleEventWithGuard(receiveStamp);
        }
    }
    else
    {
        handleEventWithGuard(receiveStamp);
    }
}

void Channel::handleEventWithGuard(Timestamp receivestamp)
{
    eventHandling_ = true;

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_) closeCallback_();
    }

    if (revents_ & EPOLLERR)
    {
        if (errorCallback_) errorCallback_();
    }

    if (revents_ & (EPOLLIN | EPOLLPRI))
    {   
        if (readCallback_) readCallback_(receivestamp);
    }

    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_) writeCallback_();
    }

    eventHandling_ = false;
}