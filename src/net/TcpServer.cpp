#include "net/TcpServer.hpp"
#include "net/Acceptor.hpp"
#include "net/EventLoop.hpp"
#include "net/SocketOps.hpp"
#include "net/EventLoopThreadPool.hpp"

void defaultMessageCallback(const cm::TcpConnectionPtr &, cm::net::Buffer *buf, cm::Timestamp) {
	buf->retrieveAll();
}

cm::net::TcpServer::TcpServer(cm::net::EventLoop *loop, const cm::net::InetAddress &listenAddr,
                              std::string nameArg, const Option &option) :
		loop_(loop), ipPort_(listenAddr.toIpPort()), name_(std::move(nameArg)), nextConnId_(1),
		acceptor_(std::make_unique<Acceptor>(loop, listenAddr, option == Option::kReusePort)),
		threadPool_(std::make_shared<EventLoopThreadPool>(loop, name_)),
		connectionCallback_(),
		messageCallback_() {
	acceptor_->setNewConnectionCallback([&](const int fd, const InetAddress &peerAddr) {
		EventLoop *ioLoop = threadPool_->getNextLoop();
		char buf[64] = {0};
		snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_++);
		std::string connName = name_ + buf;
		InetAddress localAddr(sockets::getLocalAddr(fd));

	});
}

cm::net::TcpServer::~TcpServer() = default;

void cm::net::TcpServer::setThreadNum(const int num) {
	threadPool_->setThreadNum(num);
}

void cm::net::TcpServer::start() {
	if (started_++ == 0) {
		threadPool_->start(threadInitCallback_);
		loop_->runInLoop([accept = acceptor_.get()] { accept->listen(); });
	}
}
