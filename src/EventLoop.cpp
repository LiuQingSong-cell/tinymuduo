#include "EventLoop.h"
#include <sys/eventfd.h>
#include "logger.h"
#include "Poller.h"
#include "Channel.h"

#include <unistd.h>
#include <fcntl.h>
#include <cassert>



// avoid a thread create multiple EventLoop
__thread EventLoop* t_loopIniThisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventfd()
{
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventfd < 0)
    {
        LOG_FATAL("eventfd error: %d \n", errno);
    }
    return eventfd;
}

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_))
    {
        LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
        if (t_loopIniThisThread)
        {
            LOG_FATAL("This thread exists another EventLoop %p \n", t_loopIniThisThread);
        }
        else t_loopIniThisThread = this;

        // setting wakeupfd events and callbacks
        wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
        wakeupChannel_->enableReading();
    }


EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopIniThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
}

void EventLoop::loop()
{
    assert(!looping_);
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start loop \n", this);

    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel* channel : activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }

        /*
            mainloop 事先注册一个回调，注册在pendingFunctors_里（需要subloop执行) 
        */
        doPendingFunctor(); 
    }

    LOG_INFO("EvnentLoop %p quit looping \n", this);
}

void EventLoop::quit()
{
    quit_ = true;
    
    if (!isInLoopThread()) // 在非loop的线程中调用quit 
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else // 非当前loop线程中执行cb 先唤醒在执行
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}


void EventLoop::wakeup()
{
    // 实际上是自己给自己监听的wakeupfd写信息
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctor()
{
    std::vector<Functor> functors; // 使加锁时间更短
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> guard(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (auto functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}
