//
// Created by Cmf on 2022/6/9.
//

#include "net/TcpConnection.hpp"
#include "net/EventLoop.hpp"
#include "net/Socket.hpp"
#include "net/Channel.hpp"

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr) :
        _loop(loop),
        _name(name),
        _state(Connecting),
        _reading(true),
        _socket(new Socket(sockfd)),
        _channel(new Channel(loop, sockfd)),
        _localAddr(localAddr),
        _peerAddr(peerAddr),
        _highWaterMark(64 * 1024 * 1024) /*超过64M就到水位线了，要停止发送*/ {
    _channel->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
    _channel->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
    _channel->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
    _channel->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d", _name.c_str(), sockfd);
    _socket->SetKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d", _name.c_str(), _channel->Fd(), static_cast<int>(_state));
}

void TcpConnection::Send(const std::string &buf) {
    if (_state == Connected) {
        if (_loop->IsInLoopThread()) {//当前loop是不是在对应的线程
            SendInLoop(buf.c_str(), buf.size());
        } else {
            _loop->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

/**
 * 发送数据  应用写的快， 而内核发送数据慢， 需要把待发送数据写入缓冲区， 而且设置了水位回调
 */
void TcpConnection::SendInLoop(const void *message, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    //之前调用过该connection的shutdown，不能再进行发送了
    if (_state == Disconnected) {
        LOG_ERROR("disconnected, give up writing!");
        return;
    }
    //表示channel_第一次开始写数据，而且缓冲区没有待发送数据
    if (!_channel->IsWriting() && _outputBuffer.ReadableBytes() == 0) {
        nwrote = write(_channel->Fd(), message, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && _writeCompleteCallback) {
                //既然在这里数据全部发送完成，就不用再给channel设置epollout事件了
                _loop->QueueInLoop(std::bind(_writeCompleteCallback, shared_from_this()));
            }
        } else//nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE  RESET
                {
                    faultError = true;
                }
            }
        }
    }
    //说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存到缓冲区当中，然后给channel
    //注册epollout事件，poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
    //也就是调用TcpConnection::handleWrite方法，把发送缓冲区中的数据全部发送完成
    if (!faultError && remaining > 0) {
        //目前发送缓冲区剩余的待发送数据的长度
        size_t oldLen = _outputBuffer.ReadableBytes();
        if (oldLen + remaining >= _highWaterMark
            && oldLen < _highWaterMark
            && _highWaterMarkCallback) {
            _loop->QueueInLoop(std::bind(_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
        }
        _outputBuffer.Append((char *) message + nwrote, remaining);
        if (!_channel->IsWriting()) {
            _channel->EnableWriting();//这里一定要注册channel的写事件，否则poller不会给channel通知epollout
        }
    }
}

void TcpConnection::Shutdown() {
    if (_state == Connected) {
        SetState(Disconnecting);
        _loop->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}

void TcpConnection::ShutdownInLoop() {
    if (!_channel->IsWriting()) {
        _socket->ShutdownWrite();//关闭写端
    }
}

void TcpConnection::ConnectEstablished() {
    SetState(Connected);
    _channel->Tie(shared_from_this());
    _channel->EnableReading();//向poller注册channel的epollin事件
}

void TcpConnection::ConnectDestroyed() {
    if (_state == Connected) {
        SetState(Disconnected);
        _channel->DisableAll(); // 把channel的所有感兴趣的事件，从poller中del掉
        _connectionCallback(shared_from_this());
    }
    _channel->Remove();//把channel从poller中删除掉
}

void TcpConnection::HandleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = _inputBuffer.ReadFd(_channel->Fd(), &savedErrno);
    //已建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
    if (n > 0) {
        _messageCallback(shared_from_this(), &_inputBuffer, receiveTime);
    } else if (n == 0) {
        this->HandleClose();
    } else {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        this->HandleError();
    }
}

void TcpConnection::HandleWrite() {
    if (_channel->IsWriting()) {
        int savedErrno = 0;
        ssize_t n = _outputBuffer.WriteFd(_channel->Fd(), &savedErrno);
        if (n > 0) {
            _outputBuffer.Retrieve(n);
            if (_outputBuffer.ReadableBytes() == 0) {
                _channel->DisableWriting();
                if (_writeCompleteCallback) {
                    //唤醒loop_对应的thread线程，执行回调
                    _loop->QueueInLoop(std::bind(_writeCompleteCallback, shared_from_this()));
                }
                if (_state == Disconnecting) {
                    ShutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    } else {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing", _channel->Fd());
    }
}

void TcpConnection::HandleClose() {
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d", _channel->Fd(), static_cast<int>(_state));
    SetState(Disconnected);
    _channel->DisableAll();
    TcpConnectionPtr connPtr(shared_from_this());
    _connectionCallback(connPtr);//执行连接关闭的回调
    _closeCallback(connPtr);//关闭连接的回调  执行的是TcpServer::removeConnection回调方法
}

void TcpConnection::HandleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (getsockopt(_channel->Fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d", _name.c_str(), err);
}