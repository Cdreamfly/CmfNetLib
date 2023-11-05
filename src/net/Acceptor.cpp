#include "cm/net/Acceptor.hpp"
#include "cm/net/InetAddress.hpp"
#include "cm/net/SocketOps.hpp"
#include "cm/base/Log.hpp"

cm::net::Acceptor::Acceptor(cm::net::EventLoop *loop, const cm::net::InetAddress &listenAddr, bool reUsePort)
		: loop_(loop),
		  acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
		  acceptChannel_(loop, acceptSocket_.fd()),
		  listening_(false) {
	acceptSocket_.setReuseAddr(true);
	acceptSocket_.setReusePort(reUsePort);
	acceptSocket_.bindAddress(listenAddr);
	acceptChannel_.setReadCallback([this](Timestamp) {
		InetAddress peerAddr;
		int connFd = acceptSocket_.accept(peerAddr);
		if (connFd >= 0) {
			if (newConnectionCallback_) {
				newConnectionCallback_(connFd, peerAddr);
			} else {
				sockets::close(connFd);
			}
		} else {
			LOG_ERROR("accept err:%d", errno);
			if (errno == EMFILE) {
				LOG_ERROR("sock fd reached limit!");
			}
		}
	});
}

cm::net::Acceptor::~Acceptor() {
	acceptChannel_.disableAll();
	acceptChannel_.remove();

}

void cm::net::Acceptor::listen() {
	listening_ = true;
	acceptSocket_.listen();
	acceptChannel_.enableReading();
}