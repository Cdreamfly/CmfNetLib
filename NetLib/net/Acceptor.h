//
// Created by Cmf on 2022/6/9.
//

#ifndef CMFNETLIB_ACCEPTOR_H
#define CMFNETLIB_ACCEPTOR_H

#include "NetLib/base/noncopyable.h"
#include "Socket.hpp"
#include "Channel.h"
#include <functional>

class InetAddress;

class EventLoop;

class Acceptor : private noncopyable {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress &)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);

    ~Acceptor();

    void SetNewConnectionCallback(const NewConnectionCallback &cb) {
        _newConnectionCallback = cb;
    }

    bool Listenning() const {
        return _listenning;
    }

    void Listen();

private:
    void HandleRead();

    EventLoop *_loop;//Acceptor用的就是用户定义的那个baseLoop，也称作mainLoop
    Socket _acceptSocket;
    Channel _acceptChannel;
    NewConnectionCallback _newConnectionCallback;
    bool _listenning;
};


#endif //CMFNETLIB_ACCEPTOR_H
