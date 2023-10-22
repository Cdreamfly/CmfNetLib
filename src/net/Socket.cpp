#include "net/Socket.hpp"
#include "net/InetAddress.hpp"

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
