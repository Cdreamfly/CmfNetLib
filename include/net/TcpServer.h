//
// Created by Cmf on 2022/6/2.
//

#ifndef CMFNETLIB_TCPSERVER_H
#define CMFNETLIB_TCPSERVER_H

#include "base/noncopyable.h"
#include "base/Timestamp.hpp"
#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "Buffer.hpp"
#include "InetAddress.hpp"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

#include <functional>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer : private noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    enum Option {
        NoReusePort,
        ReusePort,
    };

    TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option = NoReusePort);

    ~TcpServer();

    void SetThreadInitcallback(const ThreadInitCallback &cb) {_threadInitCallback = cb;}

    void SetConnectionCallback(const ConnectionCallback &cb) { _connectionCallback = cb; }

    void SetMessageCallback(const MessageCallback &cb) { _messageCallback = cb; }

    void SetWriteCompleteCallback(const WriteCompleteCallback &cb) { _writeCompleteCallback = cb; }

    /**
     * 置底层subloop的个数
     * @param num
     */
    void SetThreadNum(int num);

    /**
     * 开启服务器监听 实际上就是开启mainloop的accptor的listen
     */
    void Start();

private:
    void NewConnection(int sockfd, const InetAddress &peerAddr);//有新连接来了
    void RemoveConnection(const TcpConnectionPtr &conn);//有连接断开了，不要这条连接了
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop *_loop;//baseLoop 用户定义的loop 一个线程一个loop循环
    const std::string _ipPort;
    const std::string _name;
    std::unique_ptr<Acceptor> _acceptor;//运行在mainLoop，任务就是监听新连接事件
    std::shared_ptr<EventLoopThreadPool> _threadPool;//线程池 one loop per base
    ConnectionCallback _connectionCallback; //有新连接时的回调
    MessageCallback _messageCallback;//已连接用户有读写消息时的回调 reactor调用
    WriteCompleteCallback _writeCompleteCallback;//消息发送完成以后的回调
    ThreadInitCallback _threadInitCallback;//loop线程初始化的回调
    std::atomic_int _started;
    int _nextConnId;
    ConnectionMap _connections;//保存所有的连接
};

#endif //CMFNETLIB_TCPSERVER_H
