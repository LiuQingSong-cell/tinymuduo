#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name):
    loop_(nullptr),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    mutex_(),
    cond_(),
    callback_(cb)
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cond_.wait(lk, [&](){return loop_ != nullptr;});
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> guard(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    // loop 退出
    std::lock_guard<std::mutex> guard(mutex_);
    loop_ = nullptr;
}