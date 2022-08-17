//
// Created by Cmf on 2022/6/3.
//

#include "net/Channel.h"
#include "net/EventLoop.h"

void Channel::EnableReading() {
    _events |= ReadEvent;
    _loop->UpdateChannel(this);
}

void Channel::DisableReading() {
    _events &= ~ReadEvent;
    _loop->UpdateChannel(this);
}

void Channel::EnableWriting() {
    _events |= WriteEvent;
    _loop->UpdateChannel(this);
}

void Channel::DisableWriting() {
    _events &= ~WriteEvent;
    _loop->UpdateChannel(this);
}

void Channel::DisableAll() {
    _events = NoneEvent;
    _loop->UpdateChannel(this);
}

void Channel::Remove() {
    _loop->RemoveChannel(this);
}