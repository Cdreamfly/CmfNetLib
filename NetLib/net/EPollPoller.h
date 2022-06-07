//
// Created by Cmf on 2022/6/4.
//

#ifndef CMFNETLIB_EPOLLPOLLER_H
#define CMFNETLIB_EPOLLPOLLER_H

#include "Poller.h"

struct epoll_event;

class EPollPoller : public Poller {
public:
    using ptr = std::shared_ptr<EPollPoller>;

    EPollPoller(EventLoop *loop);

    ~EPollPoller() override;

    Timestamp::ptr Poll(int timeoutMs, ChannelList *activeChannels) override;

    void UpdateChannel(Channel *channel) override;

    void RemoveChannel(Channel *channel) override;

private:
    static const int InitEventListSize = 16;

    void FillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    void Update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

private:
    int _epollFd;
    EventList _events;
};


#endif //CMFNETLIB_EPOLLPOLLER_H
