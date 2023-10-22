#pragma once

#include "base/NonCopyable.hpp"
#include "net/SocketOps.hpp"

#include <netinet/tcp.h>
#include <cstring>

namespace cm::net {
	class InetAddress;

	class Socket : private NonCopyable {
	public:
		explicit Socket(const int fd) : sockFd_(fd) {}

		virtual ~Socket() { sockets::close(sockFd_); }

		int fd() const { return sockFd_; }

		void bindAddress(const InetAddress &addr) const;

		void listen() const { sockets::listenOrDie(sockFd_); }

		int accept(InetAddress &peerAddr) const;

		void shutdownWrite() const { sockets::shutdownWrite(sockFd_); }

		void setTcpNoDelay(const bool on) const {
			int opt = on ? 1 : 0;
			::setsockopt(sockFd_, IPPROTO_TCP, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof(opt)));
		}

		void setReuseAddr(const bool on) const {
			int opt = on ? 1 : 0;
			::setsockopt(sockFd_, IPPROTO_TCP, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof(opt)));
		}

		void setReusePort(const bool on) const {
			int opt = on ? 1 : 0;
			int ret = ::setsockopt(sockFd_, IPPROTO_TCP, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof(opt)));
			if (ret < 0 && on) {
				LOG_FATAL("SO_REUSEPORT failed.");
			}
		}

		void setKeepAlive(const bool on) const {
			int opt = on ? 1 : 0;
			::setsockopt(sockFd_, IPPROTO_TCP, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof(opt)));
		}

	private:
		const int sockFd_;
	};
}