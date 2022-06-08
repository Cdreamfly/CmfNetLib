//
// Created by Cmf on 2022/6/8.
//

#ifndef CMFNETLIB_EVENTLOOPTHREAD_H
#define CMFNETLIB_EVENTLOOPTHREAD_H

#include "NetLib/base/noncopyable.h"
#include "NetLib/thread/Thread.h"
#include <functional>
#include <condition_variable>

class EventLoop;

class EventLoopThread : private noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());

    ~EventLoopThread();

    EventLoop *StartLoop();

private:
    void ThreadFunc();

    EventLoop *_loop;
    bool _exiting;
    Thread _thread;
    std::mutex _mtx;
    std::condition_variable _cond;
    ThreadInitCallback _callback;
};


#endif //CMFNETLIB_EVENTLOOPTHREAD_H
