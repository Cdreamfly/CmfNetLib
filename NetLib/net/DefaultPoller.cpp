//
// Created by Cmf on 2022/6/4.
//
#include "Poller.h"
#include "EPollPoller.h"

Poller *Poller::NewDefaultPoller(EventLoop::ptr loop) {
    // 通过此环境变量来决定使用poll还是epoll
    if (getenv("MUDUO_USE_POLL")) {
        return nullptr;
    } else return new EPollPoller(loop);
}