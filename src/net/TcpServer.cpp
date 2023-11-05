#include "cm/net/TcpServer.hpp"
#include "cm/net/Acceptor.hpp"
#include "cm/net/EventLoop.hpp"
#include "cm/net/SocketOps.hpp"
#include "cm/net/EventLoopThreadPool.hpp"
#include "cm/base/Log.hpp"

static cm::net::EventLoop *CheckLoopNotNull(cm::net::EventLoop *loop) {
	if (loop == nullptr) {
		LOG_FATAL("mainLoop is null!");
	}
	return loop;
}

cm::net::TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg, const Option &option)
		: loop_(CheckLoopNotNull(loop)),
		  ipPort_(listenAddr.toIpPort()),
		  name_(std::move(nameArg)),
		  acceptor_(std::make_unique<Acceptor>(loop, listenAddr, option == Option::kReusePort)),
		  threadPool_(std::make_shared<EventLoopThreadPool>(loop, name_)),
		  connectionCallback_(),
		  messageCallback_(),
		  started_(0),
		  nextConnId_(1) {
	// 当有先用户连接时，会执行TcpServer::newConnection回调
	acceptor_->setNewConnectionCallback(
			[&](const int fd, const InetAddress &peerAddr) { newConnection(fd, peerAddr); });
}

cm::net::TcpServer::~TcpServer() {
	for (auto &item: connections_) {
		// 这个局部的shared_ptr智能指针对象，出右括号，可以自动释放new出来的TcpConnection对象资源了
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		// 销毁连接
		conn->getLoop()->runInLoop([&] { conn->connectDestroyed(); });
	}
}

// 设置底层sub loop的个数
void cm::net::TcpServer::setThreadNum(const int numThreads) {
	threadPool_->setThreadNum(numThreads);
}

// 开启服务器监听   loop.loop()
void cm::net::TcpServer::start() {
	if (started_++ == 0) // 防止一个TcpServer对象被start多次
	{
		threadPool_->start(threadInitCallback_); // 启动底层的loop线程池
		loop_->runInLoop([accept = acceptor_.get()] { accept->listen(); });
	}
}

// 有一个新的客户端的连接，acceptor会执行这个回调操作
void cm::net::TcpServer::newConnection(const int sockFd, const InetAddress &peerAddr) {
	// 轮询算法，选择一个subLoop，来管理channel
	EventLoop *ioLoop = threadPool_->getNextLoop();
	char buf[64] = {0};
	snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
	++nextConnId_;
	std::string connName = name_ + buf;
	LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s",
	         name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());
	// 通过sock fd获取其绑定的本机的ip地址和端口信息
	InetAddress localAddr(sockets::getLocalAddr(sockFd));
	// 根据连接成功的sock fd，创建TcpConnection连接对象
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, connName, sockFd, localAddr, peerAddr);
	connections_[connName] = conn;
	// 下面的回调都是用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);

	// 设置了如何关闭连接的回调   conn->shutDown()
	conn->setCloseCallback([this](const TcpConnectionPtr &conn) {
		loop_->runInLoop([this, conn] {
			LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n", name_.c_str(), conn->name().c_str());
			connections_.erase(conn->name());
			EventLoop *ioLoop = conn->getLoop();
			ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
		});
	});

	// 直接调用TcpConnection::connectEstablished
	ioLoop->runInLoop([conn] { conn->connectEstablished(); });
}