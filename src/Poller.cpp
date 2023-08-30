#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop* loop) : ownerLoop_(loop){}
Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const
{
    auto channel_iterator = Channels_.find(channel->fd());
    return channel_iterator != Channels_.end() && channel_iterator->second == channel;
}