#include "net/TcpConnection.hpp"

#include <utility>
#include "net/Socket.hpp"
#include "net/Channel.hpp"
#include "net/EventLoop.hpp"
#include "net/SocketOps.hpp"
#include "base/Log.hpp"

static cm::net::EventLoop *CheckLoopNotNull(cm::net::EventLoop *loop) {
	if (loop == nullptr) {
		LOG_FATAL("%s:%s:%d TcpConnection Loop is null! \n", __FILE__, __FUNCTION__, __LINE__);
	}
	return loop;
}

cm::net::TcpConnection::TcpConnection(EventLoop *loop,
                                      std::string nameArg,
                                      int sockFd,
                                      const InetAddress &localAddr,
                                      const InetAddress &peerAddr)
		: loop_(CheckLoopNotNull(loop)),
		  name_(std::move(nameArg)),
		  state_(StateE::kConnecting),
		  reading_(true),
		  socket_(std::make_unique<Socket>(sockFd)),
		  channel_(std::make_unique<Channel>(loop, sockFd)),
		  localAddr_(localAddr), peerAddr_(peerAddr),
		  highWaterMark_(64 * 1024 * 1024) // 64M
{
	// 下面给channel设置相应的回调函数，poller给channel通知感兴趣的事件发生了，channel会回调相应的操作函数
	channel_->setReadCallback([this](Timestamp receiveTime) { handleRead(receiveTime); });
	channel_->setWriteCallback([this] { handleWrite(); });
	channel_->setCloseCallback([this] { handleClose(); });
	channel_->setErrorCallback([this] { handleError(); });
	LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockFd);
	socket_->setKeepAlive(true);
}


cm::net::TcpConnection::~TcpConnection() = default;

void cm::net::TcpConnection::send(const std::string_view &buf) {
	if (state_ == StateE::kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop(buf.data(), buf.size());
		} else {
			loop_->runInLoop([&] { sendInLoop(buf.data(), buf.size()); });
		}
	}
}

/**
 * 发送数据  应用写的快， 而内核发送数据慢， 需要把待发送数据写入缓冲区， 而且设置了水位回调
 */
void cm::net::TcpConnection::sendInLoop(const void *data, const size_t len) {
	auto self(shared_from_this());
	ssize_t nWrote = 0;
	size_t remaining = len;
	bool faultError = false;
	// 之前调用过该connection的shutdown，不能再进行发送了
	if (state_ == StateE::kDisconnected) {
		LOG_ERROR("disconnected, give up writing!");
		return;
	}
	// 表示channel_第一次开始写数据，而且缓冲区没有待发送数据
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
		nWrote = ::write(channel_->fd(), data, len);
		if (nWrote >= 0) {
			remaining = len - nWrote;
			if (remaining == 0 && writeCompleteCallback_) {
				// 既然在这里数据全部发送完成，就不用再给channel设置epoll out事件了
				loop_->queueInLoop([this, self] { writeCompleteCallback_(self); });
			}
		} else // nWrote < 0
		{
			nWrote = 0;
			if (errno != EWOULDBLOCK) {
				LOG_ERROR("TcpConnection::sendInLoop");
				if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE  RESET
				{
					faultError = true;
				}
			}
		}
	}
	// 说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存到缓冲区当中，然后给channel
	// 注册epoll out事件，poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
	// 也就是调用TcpConnection::handleWrite方法，把发送缓冲区中的数据全部发送完成
	if (!faultError && remaining > 0) {
		// 目前发送缓冲区剩余的待发送数据的长度
		size_t oldLen = outputBuffer_.readableBytes();
		if (oldLen + remaining >= highWaterMark_
		    && oldLen < highWaterMark_
		    && highWaterMarkCallback_) {
			loop_->queueInLoop([this, self, oldLen, remaining] { highWaterMarkCallback_(self, oldLen + remaining); });
		}
		outputBuffer_.append((char *) data + nWrote, remaining);
		if (!channel_->isWriting()) {
			channel_->enableWriting(); // 这里一定要注册channel的写事件，否则poller不会给channel通知epoll out
		}
	}
}

// 关闭连接
void cm::net::TcpConnection::shutdown() {
	if (state_ == StateE::kConnected) {
		setState(StateE::kDisconnecting);
		loop_->runInLoop([this] { shutdownInLoop(); });
	}
}

void cm::net::TcpConnection::shutdownInLoop() {
	if (!channel_->isWriting()) // 说明outputBuffer中的数据已经全部发送完成
	{
		socket_->shutdownWrite(); // 关闭写端
	}
}

// 连接建立
void cm::net::TcpConnection::connectEstablished() {
	setState(StateE::kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading(); // 向poller注册channel的epollin事件
	// 新连接建立，执行回调
	connectionCallback_(shared_from_this());
}

// 连接销毁
void cm::net::TcpConnection::connectDestroyed() {
	if (state_ == StateE::kConnected) {
		setState(StateE::kDisconnected);
		channel_->disableAll(); // 把channel的所有感兴趣的事件，从poller中del掉
		connectionCallback_(shared_from_this());
	}
	channel_->remove(); // 把channel从poller中删除掉
}

void cm::net::TcpConnection::handleRead(Timestamp receiveTime) {
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), savedErrno);
	if (n > 0) {
		// 已建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
		messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
	} else if (n == 0) {
		handleClose();
	} else {
		errno = savedErrno;
		LOG_ERROR("TcpConnection::handleRead");
		handleError();
	}
}

void cm::net::TcpConnection::handleWrite() {
	if (channel_->isWriting()) {
		int savedErrno = 0;
		ssize_t n = outputBuffer_.writeFd(channel_->fd(), savedErrno);
		if (n > 0) {
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0) {
				channel_->disableWriting();
				if (writeCompleteCallback_) {
					// 唤醒loop_对应的thread线程，执行回调
					//loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
					loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
				}
				if (state_ == StateE::kDisconnecting) {
					shutdownInLoop();
				}
			}
		} else {
			LOG_ERROR("TcpConnection::handleWrite");
		}
	} else {
		LOG_ERROR("TcpConnection fd=%d is down, no more writing", channel_->fd());
	}
}

// poller => channel::closeCallback => TcpConnection::handleClose
void cm::net::TcpConnection::handleClose() {
	setState(StateE::kDisconnected);
	channel_->disableAll();
	TcpConnectionPtr connPtr(shared_from_this());
	connectionCallback_(connPtr); // 执行连接关闭的回调
	closeCallback_(connPtr); // 关闭连接的回调  执行的是TcpServer::removeConnection回调方法
}

void cm::net::TcpConnection::handleError() {
	int optVal;
	socklen_t optLen = sizeof optVal;
	int err = 0;
	if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optVal, &optLen) < 0) {
		err = errno;
	} else {
		err = optVal;
	}
	LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d", name_.c_str(), err);
}


