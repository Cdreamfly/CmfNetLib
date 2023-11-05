#include "net/TcpServer.hpp"
#include "net/EventLoop.hpp"
#include "base/Log.hpp"

class EchoServer {
public:
	EchoServer(cm::net::EventLoop *loop,
	           const cm::net::InetAddress &addr,
	           const std::string &name)
			: server_(loop, addr, name), loop_(loop) {
		// 注册回调函数
		server_.setConnectionCallback(
				std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
		);

		server_.setMessageCallback(
				std::bind(&EchoServer::onMessage, this,
				          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		);

		server_.setThreadInitCallback([](cm::net::EventLoop *loop){
			std::cout<<"---------------setThreadInitCallback"<<std::endl;
		});

		server_.setWriteCompleteCallback([](const cm::net::TcpConnectionPtr &conn){
			std::cout<<"---------------setWriteCompleteCallback"<<std::endl;
		});
		// 设置合适的loop线程数量 loopthread
		server_.setThreadNum(3);
	}

	void start() {
		server_.start();
	}

private:
	// 连接建立或者断开的回调
	void onConnection(const cm::net::TcpConnectionPtr &conn) {
		if (conn->connected()) {
			LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
		} else {
			LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
		}
	}

	// 可读写事件回调
	void onMessage(const cm::net::TcpConnectionPtr &conn,
	               cm::net::Buffer *buf,
	               cm::Timestamp time) {
		std::string msg = buf->retrieveAllAsString();
		std::cout << "------------" << msg << std::endl;
		conn->send(msg);
		conn->shutdown(); // 写端   EPOLLHUP =》 closeCallback_
	}

	cm::net::EventLoop *loop_;
	cm::net::TcpServer server_;
};

int main() {
	cm::net::EventLoop loop;
	cm::net::InetAddress addr(9199);
	EchoServer server(&loop, addr, "EchoServer-01"); // Acceptor non-blocking listenfd  create bind
	server.start(); // listen  loopthread  listenfd => acceptChannel => mainLoop =>
	loop.loop(); // 启动mainLoop的底层Poller

	return 0;
}