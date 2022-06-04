//
// Created by Cmf on 2022/6/2.
//

#ifndef CMFNETLIB_CHANNEL_H
#define CMFNETLIB_CHANNEL_H

#include <memory>
#include <functional>
#include <sys/epoll.h>
#include "NetLib/base/noncopyable.h"
#include "NetLib/base/Timestamp.hpp"
#include "NetLib/log/Log.hpp"

/**
 * Channel(事件分发器),只属于一个EventLoop,Channel类中保存着IO事件的类型以及对应的回调函数,每个channel只负责一个文件描述符
 */

class Channel : private noncopyable {
public:
    using ptr = std::shared_ptr<Channel>;
    using EventCallback = std::function<void()>;    //事件回调
    using ReadEventCallback = std::function<void(Timestamp::ptr)>;//读事件回调


    Channel(EventLoop::ptr loop, int fd) : _loop(loop), _fd(fd), _events(0), _revents(0), _index(-1), _tied(false) {
    }

    /**
     * 设置回调函数对象
     * @param cb
     */
    void SetReadCallback(ReadEventCallback cb) {
        _readCallback = std::move(cb);
    }

    void SetWriteCallback(EventCallback cb) {
        _writeCallback = std::move(cb);
    }

    void SetCloseCallback(EventCallback cb) {
        _closeCallback = std::move(cb);
    }

    void SetErrorCallback(EventCallback cb) {
        _errorCallback = std::move(cb);
    }

    /**
     * 返回fd当前的事件状态
     */
    bool IsNoneEvent() const {
        return _events == NoneEvent;
    }

    bool IsWriting() const {
        return _events & WriteEvent;
    }

    bool IsReading() const {
        return _events & ReadEvent;
    }

    /**
     * 设置fd相应的事件状态，要让fd对这个事件感兴趣,update就是调用epoll_ctrl，通知poller把fd感兴趣的事件添加到fd上
     */
    void EnableReading();

    void DisableReading();

    void EnableWriting();

    void DisableWriting();

    void DisableAll();

    uint32_t Revents() const {
        return _revents;
    }

    void SetRevents(uint32_t revt) {
        _revents = revt;
    }

    int Index() const {
        return _index;
    }

    void SetIndex(int index) {
        _index = index;
    }

    uint32_t Events() const {
        return _events;
    }

    int Fd() const {
        return _fd;
    }

    void Remove();

    /**
     * 防止当channel被手动remove掉，channel还在执行回调操作
     * @param obj
     */
    void Tie(std::shared_ptr<void> &obj) {
        _tie = obj;
        _tied = true;
    }

    /**
     * 处理事件
     * @param receiveTime
     */
    void HandleEvent(Timestamp::ptr receiveTime) {
        if (_tied) {
            std::shared_ptr<void> guard = _tie.lock();
            if (guard) {
                HandleEventWithGuard(receiveTime);
            }
        } else {
            HandleEventWithGuard(receiveTime);
        }
    }


private:
    /**
     * //根据poller通知的channel发生的具体事件， 由channel负责调用具体的回调操作
     * EPOLLIN:表示对应的文件描述符可以读；
     * EPOLLOUT:表示对应的文件描述符可以写；
     * EPOLLPRI:表示对应的文件描述符有紧急的数据可读
     * EPOLLERR:表示对应的文件描述符发生错误；
     * EPOLLHUP:表示对应的文件描述符被挂断；
     * EPOLLET:表示对应的文件描述符有事件发生；
     * @param receiveTime
     */
    void HandleEventWithGuard(Timestamp::ptr receiveTime) {
        LOG_INFO("channel handleEvent revents:%d", Revents());
        if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN)) {
            if (_closeCallback) {
                _closeCallback();
            }
        }
        if (_revents & EPOLLERR) {
            if (_errorCallback) {
                _errorCallback();
            }
        }
        if (_revents & (EPOLLIN | EPOLLPRI)) {
            if (_readCallback) {
                _readCallback(receiveTime);
            }
        }
        if (_revents & EPOLLOUT) {
            if (_writeCallback) {
                _writeCallback();
            }
        }
    }

private:


    static const int NoneEvent = 0; //无事件
    static const int ReadEvent = EPOLLIN | EPOLLET; //可读事件
    static const int WriteEvent = EPOLLOUT; //可写事件

    EventLoop::ptr _loop;   //channel所属的loop,一个channel只属于一个loop
    const int _fd;          //channel所属的文件描述符
    uint32_t _events;       //注册的事件
    uint32_t _revents;      //poller设置的就绪的事件
    int _index;             //被poller使用的下标
    std::weak_ptr<void> _tie;
    bool _tied;

    ReadEventCallback _readCallback;//读事件回调
    EventCallback _writeCallback;   //写事件回调
    EventCallback _closeCallback;   //关闭事件回调
    EventCallback _errorCallback;   //错误事件回调
};

#endif //CMFNETLIB_CHANNEL_H
