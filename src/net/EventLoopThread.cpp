//
// Created by Cmf on 2022/6/8.
//

#include "net/EventLoopThread.h"
#include "net/EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name) :
        _loop(nullptr),
        _exiting(false),
        _thread(std::bind(&EventLoopThread::ThreadFunc, this), name),
        _mtx(),
        _cond(),
        _callback(cb) {

}

EventLoopThread::~EventLoopThread() {
    _exiting = true;
    if (_loop != nullptr) {
        _loop->Quit();
        _thread.Join();
    }
}

EventLoop *EventLoopThread::StartLoop() {
    _thread.Start();//启动底层的新线程,启动后执行的是EventLoopThread::threadFunc
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lk(_mtx);
        while (_loop == nullptr) {
            _cond.wait(lk);// 成员变量loop_没有被新线程初始化的时候，一直wait在lock上
        }
        loop = this->_loop;
    }
    return loop;
}

void EventLoopThread::ThreadFunc() {
    EventLoop loop;//创建一个独立的eventloop，和上面的线程是一一对应的，one loop per base
    if (_callback) {
        // 如果我们实现传递了callback_，ThreadInitCallback就是在底层启一个新线程绑定EventLoop时调用的，进行一些init相关操作
        _callback(&loop);
    }
    {
        std::unique_lock<std::mutex> lk(_mtx);
        this->_loop = &loop;
        _cond.notify_one();
    }
    loop.Loop();   // EventLoop loop  => Poller.poll，开启事件循环，监听新用户的连接或者已连接用户的读写事件
    //一般来说，loop是一直执行的，能执行到下面的语句，说明程序要退出了，要关闭事件循环
    std::unique_lock<std::mutex> lk(_mtx);
    _loop = nullptr;
}