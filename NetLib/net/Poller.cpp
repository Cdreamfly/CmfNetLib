//
// Created by Cmf on 2022/6/4.
//

#include "Poller.h"
#include "Channel.h"

bool Poller::HasChannel(Channel *channel) const {
    auto it = _channels.find(channel->Fd());
    return it != _channels.end() && it->second == channel;
}
