//
// Created by Cmf on 2022/6/4.
//

#include "net/Poller.hpp"
#include "net/Channel.hpp"

Poller::Poller(EventLoop *loop)
        : _ownerLoop(loop) {
}

bool Poller::HasChannel(Channel *channel) const {
    auto it = _channels.find(channel->Fd());
    return it != _channels.end() && it->second == channel;
}
