#include "net/TcpServer.hpp"
#include "net/EventLoop.hpp"
#include "base/Log.hpp"

class EchoServer {
public:
	explicit EchoServer(cm::net::EventLoop *loop, const cm::net::InetAddress &addr, const std::string &name) :
			loop_(loop), tcpServer_(loop, addr, name) {
		tcpServer_.setConnectionCallback([&](const cm::net::TcpConnectionPtr &conn) {
			if (conn->connect()) {
				LOG_INFO("Connection UP : %s", conn->peerAddress().toIpPort().c_str());
			} else {
				LOG_INFO("Connection DOWN : %s", conn->peerAddress().toIpPort().c_str());
			}
		});
		tcpServer_.setMessageCallback(
				[&](const cm::net::TcpConnectionPtr &conn, cm::net::Buffer *buf, cm::Timestamp time) {
					std::string msg = buf->retrieveAllAsString();
					LOG_INFO("%s", msg.c_str());
					conn->send(msg);
					conn->shutdown();
				});
		tcpServer_.setThreadNum(3);
	}

	void start() {
		tcpServer_.start();
	}

private:
	cm::net::EventLoop *loop_;
	cm::net::TcpServer tcpServer_;
};


int main() {
	cm::net::EventLoop loop;
	cm::net::InetAddress addr(9190);
	EchoServer echoServer(&loop, addr, "EchoServer");
	echoServer.start();
	loop.loop();
	return 0;
}