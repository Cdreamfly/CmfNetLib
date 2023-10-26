#include "net/Socket.hpp"
#include "net/InetAddress.hpp"
#include "net/SocketOps.hpp"

#include <cstring>

void cm::net::Socket::bindAddress(const cm::net::InetAddress &addr) const {
	sockets::bindOrDie(sockFd_, addr.getSocketAddr());
}

int cm::net::Socket::accept(cm::net::InetAddress &peerAddr) const {
	sockaddr_in6 addr{};
	memset(&addr, 0, sizeof(addr));
	int connFd = sockets::accept(sockFd_, &addr);
	if (connFd >= 0) {
		peerAddr.setSocketAddr6(addr);
	}
	return connFd;
}

void cm::net::Socket::listen() const { sockets::listenOrDie(sockFd_); }

void cm::net::Socket::shutdownWrite() const { sockets::shutdownWrite(sockFd_); }

void cm::net::Socket::setTcpNoDelay(const bool on) const {
	sockets::setTcpNoDelay(sockFd_, on);
}

void cm::net::Socket::setReuseAddr(const bool on) const {
	sockets::setReuseAddr(sockFd_, on);
}

void cm::net::Socket::setReusePort(const bool on) const {
	sockets::setReusePort(sockFd_, on);
}

void cm::net::Socket::setKeepAlive(const bool on) const {
	sockets::setKeepAlive(sockFd_, on);
}

cm::net::Socket::~Socket() { sockets::close(sockFd_); }