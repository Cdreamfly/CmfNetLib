#pragma once

#include "base/NonCopyable.hpp"

namespace cm::net {
	class InetAddress;

	class Socket : private NonCopyable {
	public:
		explicit Socket(const int fd) : sockFd_(fd) {}

		virtual ~Socket();

		[[nodiscard]] int fd() const { return sockFd_; }

		void bindAddress(const InetAddress &) const;

		void listen() const;

		int accept(InetAddress &) const;

		void shutdownWrite() const;

		void setTcpNoDelay(bool) const;

		void setReuseAddr(bool) const;

		void setReusePort(bool) const;

		void setKeepAlive(bool) const;

	private:
		const int sockFd_;
	};
}