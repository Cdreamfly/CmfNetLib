//
// Created by Cmf on 2022/6/8.
//

#include "Thread.h"
#include <semaphore.h>

std::atomic_int Thread::_numCreated(0);

Thread::Thread(ThreadFunc func, const std::string &name) : _func(std::move(func)),
                                                           _name(name),
                                                           _tid(0),
                                                           _started(false),
                                                           _joined(false) {
    int num = ++_numCreated;
    if (_name.empty()) {//线程还没有名字
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "Thread %d", num);
        _name = buf;
    }
}

Thread::~Thread() {
    if (_started && !_joined) {//线程已经运行起来并且不是工作线程join
        _thread->detach(); // std::thread类提供的设置分离线程的方法，detach后成为守护线程，守护线程结束后，内核自动回收，不会出现孤儿线程
    }
}

void Thread::Start() {
    _started = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    _thread = std::make_shared<std::thread>([&] {
        _tid = std::this_thread::get_id();
        sem_post(&sem);
        _func();
    });
    sem_wait(&sem);
}

void Thread::Join() {
    _joined = true;
    _thread->join();
}