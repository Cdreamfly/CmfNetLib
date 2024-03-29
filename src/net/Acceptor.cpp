//
// Created by Cmf on 2022/6/9.
//

#include "net/Acceptor.hpp"
#include "net/SocketOps.hpp"

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort) :
        _loop(loop),
        _listening(false),
        _acceptSocket(SocketOps::CreateNonblockingSocket(listenAddr.Family())),
        _acceptChannel(loop, _acceptSocket.GetFd()) {
    _acceptSocket.SetReuseAddr(true);//设置端口可重用
    _acceptSocket.SetReusePort(true); //设置地址可重用
    _acceptSocket.Bind(listenAddr);
    _acceptChannel.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
    _acceptChannel.DisableAll();//将其冲poller监听集合中移除，此时为kDeleted状态
    _acceptChannel.Remove();//将其从EventList events_中移除，此时为kNew状态
}

void Acceptor::Listen() {
    _listening = true;
    _acceptSocket.Listen();
    _acceptChannel.EnableReading();//注册可读事件
}

void Acceptor::HandleRead() {
    InetAddress peerAddr;
    int connfd = _acceptSocket.Accept(&peerAddr);
    if (connfd >= 0) {
        if (_newConnectionCallback) {
            _newConnectionCallback(connfd, peerAddr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_ERROR("%s:%s:%d accept err:%d", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE) {
            LOG_ERROR("%s:%s:%d sockfd reached limit!", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}