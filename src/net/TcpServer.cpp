//
// Created by Cmf on 2022/6/9.
//
#include "net/TcpServer.h"
#include "net/EventLoopThreadPool.h"

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option) :
        _loop(loop),
        _ipPort(listenAddr.ToIpPort()),
        _name(nameArg),
        _acceptor(new Acceptor(loop, listenAddr, option == ReusePort)),
        _threadPool(new EventLoopThreadPool(loop, _name)),
        _connectionCallback(),
        _messageCallback(),
        _writeCompleteCallback(),
        _nextConnId(1),
        _started(0) {
    _acceptor->SetNewConnectionCallback(
            std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto &item: _connections) {
        //这个局部的shared_ptr智能指针对象，出右括号，可以自动释放new出来的TcpConnection对象资源了
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->GetLoop()->RunInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
    }
}

void TcpServer::SetThreadNum(int num) {
    _threadPool->SetThreadNum(num);
}

void TcpServer::Start() {
    if (_started++ == 0) {
        _threadPool->Start(_threadInitCallback);
        _loop->RunInLoop(std::bind(&Acceptor::Listen, _acceptor.get()));
    }
}

/**
 * 有一个新的客户端的连接，acceptor会执行这个回调操作
 * @param sockfd
 * @param peerAddr
 */
void TcpServer::NewConnection(int sockfd, const InetAddress &peerAddr) {
    EventLoop *ioLoop = _threadPool->GetNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "-%s#%d", _ipPort.c_str(), _nextConnId);
    ++_nextConnId;
    std::string connName = _name + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s", _name.c_str(), connName.c_str(),
             peerAddr.ToIpPort().c_str());
    //通过sockfd获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    memset(&local, 0, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (getsockname(sockfd, (sockaddr *) &local, &addrlen) < 0) {
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);
    //根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    _connections[connName] = conn;
    //下面的回调都是用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
    conn->SetConnectionCallback(_connectionCallback);
    conn->SetMessageCallback(_messageCallback);
    conn->SetWriteCompleteCallback(_writeCompleteCallback);
    //设置了如何关闭连接的回调   conn->shutDown()
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
    //直接调用TcpConnection::connectEstablished
    ioLoop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn) {
    _loop->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s", _name.c_str(), conn->Name().c_str());
    _connections.erase(conn->Name());
    EventLoop *ioLoop = conn->GetLoop();
    ioLoop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}