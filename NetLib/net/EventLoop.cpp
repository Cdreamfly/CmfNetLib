//
// Created by Cmf on 2022/6/7.
//

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "NetLib/log/Log.hpp"
#include <sys/eventfd.h>
#include <unistd.h>

//防止一个线程创建多个EventLoop   __thread：thread_local
//当一个eventloop创建起来它就指向那个对象，在一个线程里再去创建一个对象，由于这个指针为空，就不创建
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller IO复用接口的超时时间
const int PollTimeMs = 10000;//10秒钟

EventLoop::EventLoop() : _looping(false),
                         _quit(false),
                         _callingPendingFunctors(false),
                         _poller(Poller::NewDefaultPoller(this)),
                         _threadId(std::this_thread::get_id()) {
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    this->_wakeupFd = evtfd;
    this->_wakeupChannel = std::make_unique<Channel>(this, _wakeupFd);

    LOG_DEBUG("EventLoop Created %p in thread %d", this, _threadId);
    if (t_loopInThisThread) {//这个线程已经有loop了，就不创建了
        LOG_FATAL("Another EventLoop %p exists in this thread %d", t_loopInThisThread, _threadId);
    } else {
        t_loopInThisThread = this;
    }
    //设置wakeupfd的事件类型以及发生事件后的回调操作
    _wakeupChannel->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
    //每一个eventloop都将监听wakeupchannel的EPOLLIN读事件了
    _wakeupChannel->EnableReading();
}

EventLoop::~EventLoop() {
    _wakeupChannel->DisableAll();
    _wakeupChannel->Remove();
    close(_wakeupFd);
    t_loopInThisThread = nullptr;
}

//开启事件循环 驱动底层的poller执行poll
void EventLoop::Loop() {
    _looping = true;
    _quit = false;
    LOG_INFO("EventLoop %p Start Looping!", this);
    while (!_quit) {
        _activeChannels.clear();
        _pollReturnTime = _poller->Poll(PollTimeMs, _activeChannels);
        for (Channel *channel: _activeChannels) {
            //Poller监听哪些channel发生事件了，然后上报给EventLoop，通知channel处理相应的事件
            channel->HandleEvent((_pollReturnTime));
        }
        this->DoPendingFunctors();
    }
    LOG_INFO("EventLoop %p Stop Looping!", this);
    _looping = false;
}

void EventLoop::Quit() {
    _quit = true;
    if (!this->IsInLoopThread()) {
        this->WakeUp();
    }
}

void EventLoop::RunInLoop(Functor cb) {
    if (this->IsInLoopThread()) {
        cb();
    } else {
        this->QueueInLoop(cb);
    }
}

bool EventLoop::IsInLoopThread() const {
    return this->_threadId == std::this_thread::get_id();
}

void EventLoop::QueueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex>(_mtx);
        _pendingFunctors.emplace_back(cb);
    }
    if (!this->IsInLoopThread() || this->_callingPendingFunctors) {
        this->WakeUp();
    }
}

void EventLoop::HandleRead() {
    uint64_t one = 1;
    ssize_t n = read(_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::handleRead() reads % lu bytes instead of 8", n);
    }
}

void EventLoop::WakeUp() {
    uint64_t one;
    ssize_t n = write(_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes % lu bytes instead of 8", n);
    }
}

void EventLoop::UpdateChannel(Channel *channel) {
    this->_poller->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel *channel) {
    this->_poller->RemoveChannel(channel);
}

void EventLoop::HasChannel(Channel *channel) {
    this->_poller->HasChannel(channel);
}

void EventLoop::DoPendingFunctors() {
    std::vector<Functor> functors;
    _callingPendingFunctors = true;
    {
        std::unique_lock<std::mutex> lk(_mtx);
        functors.swap(_pendingFunctors);
    }
    for (const Functor &functor: functors) {
        functor();
    }
    _callingPendingFunctors = false;
}