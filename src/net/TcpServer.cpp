#include "net/TcpServer.hpp"
#include "net/Acceptor.hpp"
#include "net/EventLoop.hpp"
#include "net/SocketOps.hpp"
#include "net/EventLoopThreadPool.hpp"
#include "base/Log.hpp"

void defaultMessageCallback(const cm::net::TcpConnectionPtr &, cm::net::Buffer *buf, cm::Timestamp) {
	buf->retrieveAll();
}

cm::net::TcpServer::TcpServer(cm::net::EventLoop *loop, const cm::net::InetAddress &listenAddr,
                              std::string nameArg, const Option &option) :
		loop_(loop), ipPort_(listenAddr.toIpPort()), name_(std::move(nameArg)), nextConnId_(1),
		acceptor_(std::make_unique<Acceptor>(loop, listenAddr, option == Option::kReusePort)),
		threadPool_(std::make_shared<EventLoopThreadPool>(loop, name_)), connectionCallback_(),
		messageCallback_(), started_(0) {
	acceptor_->setNewConnectionCallback([&](const int fd, const InetAddress &peerAddr) {
		EventLoop *ioLoop = threadPool_->getNextLoop();
		char buf[64] = {0};
		snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_++);
		std::string connName = name_ + buf;
		LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s", name_.c_str(), connName.c_str(),
		         peerAddr.toIpPort().c_str());
		InetAddress localAddr(sockets::getLocalAddr(fd));
		TcpConnectionPtr tcpConnectionPtr = std::make_shared<TcpConnection>(ioLoop, connName, fd, localAddr, peerAddr);
		connections_[connName] = tcpConnectionPtr;
		tcpConnectionPtr->setConnectionCallback(connectionCallback_);
		tcpConnectionPtr->setMessageCallback(messageCallback_);
		tcpConnectionPtr->setWriteCompleteCallback(writeCompleteCallback_);
		tcpConnectionPtr->setCloseCallback([this](const TcpConnectionPtr &conn) {
			loop_->runInLoop([this, conn]() {
				LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s", name_.c_str(), conn->name().c_str());
				connections_.erase(conn->name());
				EventLoop *ioLoop = conn->getLoop();
				ioLoop->queueInLoop([&] { conn->connectDestroyed(); });
			});
		});
		// 直接调用TcpConnection::connectEstablished
		ioLoop->runInLoop([tcpConnectionPtr] { tcpConnectionPtr->connectEstablished(); });
	});
}

cm::net::TcpServer::~TcpServer() {
	for (auto &item: connections_) {
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		conn->getLoop()->runInLoop([&] { conn->connectDestroyed(); });
	}
}

void cm::net::TcpServer::setThreadNum(const int num) {
	threadPool_->setThreadNum(num);
}

void cm::net::TcpServer::start() {
	if (started_++ == 0) {
		threadPool_->start(threadInitCallback_);
		loop_->runInLoop([accept = acceptor_.get()] { accept->listen(); });
	}
}

void cm::net::TcpServer::newConnection(const int sockFd, const cm::net::InetAddress &peerAddr) {

}

void cm::net::TcpServer::removeConnection(const cm::net::TcpConnectionPtr &conn) {

}

void cm::net::TcpServer::removeConnectionInLoop(const cm::net::TcpConnectionPtr &conn) {

}
