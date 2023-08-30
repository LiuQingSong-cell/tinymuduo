#include "EpollPoller.h"
#include "logger.h"
#include "Channel.h"

#include <cerrno>
#include <cassert>
#include <cstring>
#include <cerrno>

const int kNew = -1; // channel.index = -1 not add to poller
const int kAdded = 1; // channel.index = 1 add to poller
const int kDeleted = 2; // channel.index = 2  delete from poller

EpollPoller::EpollPoller(EventLoop* loop) : 
    Poller(loop), 
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

/*
    channel update/remove ---> EventLoop updatechannel/removechannel
    ---> poller updatechannel/removechannel
*/

void EpollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();
    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            int fd = channel->fd();
            // assert(Channels_.find(fd) == Channels_.end());
            Channels_[fd] = channel;
        }
        else
        {
            // assert(Channels_.find(fd) != Channels_.end());
            // assert(Channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        // assert(Channels_.find(fd) != Channels_.end());
        // assert(Channels_[fd] == channel);
        // assert(index == kAdded);
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    int index = channel->index();
    Channels_.erase(fd);

    if (index == kAdded) update(EPOLL_CTL_DEL, channel);
    channel->set_index(kDeleted);
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (int i = 0; i < numEvents; i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // evnetloop obtain channels that has events
    }
}

void EpollPoller::update(int operation, Channel* channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof event);

    event.events = channel->events();
    int fd = channel->fd();
    
    event.data.ptr = channel; // 注意这里的data是union 不要在后面给fd赋值
    // event.data.fd = fd;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl error: %d", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl error: %d", errno);
        }       
    }
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) 
    {
        LOG_INFO("%d events happend \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);

        if (numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0) LOG_DEBUG("nothing happend \n");
    else
    {
        if (saveErrno == EINTR)
        {
            LOG_ERROR("Epoller poll error! \n");
        }
    }
    return now;
}
