//
// Created by Cmf on 2022/6/4.
//

#include "EPollPoller.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>

const int New = -1;     //未添加
const int Added = 1;    //已添加
const int Deleted = 2;  //已删除

EPollPoller::EPollPoller(EventLoop *loop) : Poller(loop),
                                            _epollFd(epoll_create1(EPOLL_CLOEXEC)),
                                            _events(InitEventListSize) {
    if (_epollFd < 0) {
        LOG_FATAL("EPollPoller::EPollPoller: %d", errno);
    }
}

EPollPoller::~EPollPoller() noexcept {
    close(this->_epollFd);
}

void EPollPoller::RemoveChannel(Channel *channel) {
    int fd = channel->Fd();
    _channels.erase(fd);//从map中删除
    LOG_INFO("func:%s fd:%d", __FUNCTION__, fd);
    if (channel->Index() == Added) {    //如果已经注册过了
        this->Update(EPOLL_CTL_DEL, channel);//删除
    }
    channel->SetIndex(New);//设置为为添加状态
}

void EPollPoller::UpdateChannel(Channel *channel) {
    const int index = channel->Index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d", __FUNCTION__, channel->Fd(), channel->Events(), index);
    if (index == New || index == Deleted) { //未添加或已删除
        if (index == New) { //如果未添加，键值对写入map中
            int fd = channel->Fd();
            _channels[fd] = channel;
        }
        this->Update(EPOLL_CTL_ADD, channel);   //添加一个channel到epoll
        channel->SetIndex(Added);
    } else {    //如果已经添加
        int fd = channel->Fd();
        if (channel->IsNoneEvent()) {   //如果对任何事件都不感兴趣
            this->Update(EPOLL_CTL_DEL, channel);   //删除已注册的channel的感兴趣的事件
            channel->SetIndex(Deleted);
        } else {
            this->Update(EPOLL_CTL_MOD, channel);
        }
    }
}

Timestamp EPollPoller::Poll(int timeoutMs, ChannelList *activeChannels) {
    LOG_FATAL("fd total count %d", _channels.size());
    int numEvents = epoll_wait(_epollFd, &*_events.begin(), static_cast<int>(_events.size()), timeoutMs);
    int saveErrno = errno;// 全局变量errno，poll可能在多个线程中的eventloop被调用，被读写，所以先用局部变量存起来
    Timestamp now(Timestamp::Now());
    if (saveErrno > 0) {//表示有已经发生相应事件的个数
        LOG_INFO("%d events happened", numEvents);
        FillActiveChannels(numEvents, activeChannels);
        if (numEvents == _events.size()) {//所有的监听的event都发生事件了,得扩容了
            _events.resize(_events.size() * 2);
        }
    } else if (numEvents == 0) {//epoll_wait这一轮监听没有事件发生，timeout超时了
        LOG_DEBUG("%s timeout!", __FUNCTION__);
    } else {
        if (saveErrno != EINTR) {//不等于外部的中断,是由其他错误类型引起的
            errno = saveErrno;//适配,把errno重置成当前loop之前发生的错误的值
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;

}

void EPollPoller::FillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(_events[i].data.ptr);
        channel->SetRevents(_events[i].events);
        activeChannels->emplace_back(channel);//EventLoop就拿到了它的poller给它返回的所有发生事件的channel列表了
    }
}

void EPollPoller::Update(int operation, Channel *channel) {
    epoll_event event;
    int fd = channel->Fd();
    memset(&event, 0, sizeof(event));
    event.events = channel->Events();
    event.data.fd = fd;
    event.data.ptr = channel;
    if (epoll_ctl(_epollFd, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("epoll_ctl del error:%d", errno);
        } else {
            LOG_FATAL("epoll_ctl add/mod error:%d", errno);
        }
    }

}
