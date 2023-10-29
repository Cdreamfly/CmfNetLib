#include "net/TcpConnection.hpp"
#include "net/Socket.hpp"
#include "net/Channel.hpp"
#include "net/EventLoop.hpp"
#include "base/Log.hpp"

cm::net::TcpConnection::TcpConnection(cm::net::EventLoop *loop, std::string name, const int fd,
                                      const cm::net::InetAddress &localAddr, const cm::net::InetAddress &peerAddr) :
		loop_(loop), name_(std::move(name)), reading_(true), state_(StateE::kConnecting),
		socket_(std::make_unique<Socket>(fd)), channel_(std::make_unique<Channel>(loop, fd)),
		localAddr_(localAddr), peerAddr_(peerAddr), highWaterMark_(64 * 1024 * 1024) {
	channel_->setReadCallback([&](const Timestamp &) {});
	channel_->setWriteCallback([&] {});
	channel_->setCloseCallback([&] {});
	channel_->setErrorCallback([&] {});
	LOG_INFO("TcpConnection::ctor[%s] at fd=%d", name_.c_str(), fd);
	socket_->setKeepAlive(true);
}

cm::net::TcpConnection::~TcpConnection() {
	LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d", name_.c_str(), channel_->fd(), static_cast<StateE>(state_));
}

void cm::net::TcpConnection::send(const std::string &buf) {

}

void cm::net::TcpConnection::shutdown() {

}

void cm::net::TcpConnection::connectEstablished() {

}

void cm::net::TcpConnection::connectDestroyed() {

}

void cm::net::TcpConnection::sendInLoop(const std::string &msg) {

}

void cm::net::TcpConnection::shutdownInLoop() {

}
