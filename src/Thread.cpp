#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>
#include <cassert>

std::atomic_int Thread::numCreated_{0};

Thread::Thread(ThreadFunc func, const std::string& name) :
    started_(false),
    joined_(false),
    tid_(0),
    func_(std::move(func)),
    name_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;

    // 使用信号量
    sem_t sem;
    sem_init(&sem, 0, 0);

    thread_ = std::make_shared<std::thread>([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    });
    //确保正确的创建的线程
    sem_wait(&sem);
    assert(tid_ > 0);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}
    
void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}