//
// Created by Cmf on 2022/6/8.
//

#include "net/EventLoopThreadPool.hpp"
#include "net/EventLoop.hpp"
#include "net/EventLoopThread.hpp"


EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg) :
        _baseLoop(baseLoop),
        _started(false),
        _name(nameArg),
        _numThreads(0),
        _next(0) {

}

EventLoopThreadPool::~EventLoopThreadPool() {}

/**
 * 根据指定的numThreads_创建线程，开启事件循环
 * @param cb
 */
void EventLoopThreadPool::Start(const ThreadInitCallback &cb) {
    _started = true;
    //整个服务端只有一个线程，运行着baseloop，就是用户创建的mainloop
    if (_numThreads == 0 && cb) {
        cb(_baseLoop);
    } else {
        for (int i = 0; i < _numThreads; ++i) {
            char buf[this->_name.size() + 32];
            snprintf(buf, sizeof(buf), "%s %d", _name.c_str()), i;
            EventLoopThread *t = new EventLoopThread(cb, buf);
            _threads.emplace_back(std::unique_ptr<EventLoopThread>(t));
            _loops.emplace_back(t->StartLoop());//底层创建线程，绑定一个新的EventLoop，并返回该loop的地址
        }
    }
}

EventLoop *EventLoopThreadPool::GetNextLoop() {
    EventLoop *loop = _baseLoop;
    if (!_loops.empty()) {
        loop = _loops[_next++];
        if (_next >= _loops.size()) {
            _next = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::GetAllLoops() {
    if (_loops.empty()) {
        return std::vector<EventLoop *>(1, _baseLoop);
    } else {
        return _loops;
    }
}