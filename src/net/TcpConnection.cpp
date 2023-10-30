#include "net/TcpConnection.hpp"
#include "net/Socket.hpp"
#include "net/Channel.hpp"
#include "net/EventLoop.hpp"
#include "net/SocketOps.hpp"
#include "base/Log.hpp"

cm::net::TcpConnection::TcpConnection(cm::net::EventLoop *loop, std::string name, const int fd,
                                      const cm::net::InetAddress &localAddr, const cm::net::InetAddress &peerAddr) :
		loop_(loop), name_(std::move(name)), reading_(true), state_(StateE::kConnecting),
		socket_(std::make_unique<Socket>(fd)), channel_(std::make_unique<Channel>(loop, fd)),
		localAddr_(localAddr), peerAddr_(peerAddr), highWaterMark_(64 * 1024 * 1024) {
	channel_->setReadCallback([this](const Timestamp &receiveTime) {
		int saveErrno = 0;
		ssize_t n = inputBuffer_.readFd(channel_->fd(), saveErrno);
		if (n > 0) {
			//已建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
			messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
		} else if (n == 0) {
			handleClose();
		} else {
			errno = saveErrno;
			LOG_ERROR("TcpConnection::handleRead");
			handleError();
		}
	});
	channel_->setWriteCallback([&] {
		if (channel_->isWriting()) {
			int saveErrno = 0;
			ssize_t n = outputBuffer_.writeFd(channel_->fd(), saveErrno);
			if (n > 0) {
				outputBuffer_.retrieve(n);
				if (outputBuffer_.readableBytes() == 0) {
					channel_->disableWriting();
					//唤醒loop_对应的thread线程，执行回调
					loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
				}
				if (state_ == StateE::kDisconnecting) {
					shutdownInLoop();
				}
			} else {
				LOG_ERROR("TcpConnection::handleWrite");
			}
		} else {
			LOG_ERROR("TcpConnection fd=%d is down, no more writing", channel_->fd());
		}
	});
	channel_->setCloseCallback([&] { handleClose(); });
	channel_->setErrorCallback([&] { handleError(); });
	LOG_INFO("TcpConnection::ctor[%s] at fd=%d", name_.c_str(), fd);
	socket_->setKeepAlive(true);
}

void cm::net::TcpConnection::handleClose() {
	setState(StateE::kDisconnected);
	channel_->disableAll();
	TcpConnectionPtr guardThis(shared_from_this());
	connectionCallback_(guardThis);
	closeCallback_(guardThis);
}

void cm::net::TcpConnection::handleError() {
	LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d", name_.c_str(),
	          sockets::getSocketError(channel_->fd()));
}

cm::net::TcpConnection::~TcpConnection() {
	LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d", name_.c_str(), channel_->fd(), static_cast<StateE>(state_));
}

void cm::net::TcpConnection::send(const std::string_view &buf) {
	if (state_ == StateE::kConnected) {
		if (loop_->isInLoopThread()) {
			sendInLoop(buf);
		} else {
			loop_->runInLoop([&] { sendInLoop(buf); });
		}
	}
}

/**
 * 发送数据  应用写的快， 而内核发送数据慢， 需要把待发送数据写入缓冲区， 而且设置了水位回调
 */
void cm::net::TcpConnection::sendInLoop(const std::string_view &msg) {
	ssize_t n = 0;
	std::size_t remaining = msg.size();
	bool faultError = false;
	//之前调用过该connection的shutdown，不能再进行发送了
	if (state_ == StateE::kDisconnected) {
		LOG_ERROR("disconnected, give up writing!");
		return;
	}
	//表示channel_第一次开始写数据，而且缓冲区没有待发送数据
	if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
		n = sockets::write(channel_->fd(), msg.data(), msg.size());
		if (n >= 0) {
			remaining -= n;
			if (remaining == 0 && writeCompleteCallback_) {
				//既然在这里数据全部发送完成，就不用再给channel设置epoll out事件了
				loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
			} else {
				n = 0;
				if (errno != EWOULDBLOCK) {
					if (errno == EPIPE || errno == ECONNRESET) {
						faultError = true;
					}
				}
			}
		}
	}
	/**
	 * 说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存到缓冲区当中，然后给channel
	 * 注册epoll out事件，poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
	 * 也就是调用TcpConnection::handleWrite方法，把发送缓冲区中的数据全部发送完成
	 */
	if (!faultError && remaining > 0) {
		//目前发送缓冲区剩余的待发送数据的长度
		std::size_t oldLen = outputBuffer_.readableBytes();
		//剩余待发送量大于高水位线
		if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_) {
			loop_->queueInLoop([&] { highWaterMarkCallback_(shared_from_this(), oldLen + remaining); });
		}
		outputBuffer_.append(msg.data() + n, remaining);
		if (!channel_->isWriting()) {
			//这里一定要注册channel的写事件，否则poller不会给channel通知epoll out
			channel_->enableWriting();
		}
	}
}

void cm::net::TcpConnection::shutdown() {
	if (state_ == StateE::kConnected) {
		setState(StateE::kDisconnecting);
		loop_->runInLoop([this] { shutdownInLoop(); });
	}
}

void cm::net::TcpConnection::shutdownInLoop() {
	if (!channel_->isWriting()) {
		socket_->shutdownWrite();
	}
}

void cm::net::TcpConnection::connectEstablished() {
	channel_->tie(shared_from_this());
	channel_->enableReading();
}

void cm::net::TcpConnection::connectDestroyed() {
	if (state_ == StateE::kConnected) {
		setState(StateE::kDisconnected);
		//把channel的所有感兴趣的事件，从poller中del掉
		channel_->disableAll();
		connectionCallback_(shared_from_this());
	}
	//把channel从poller中删除掉
	channel_->remove();
}





