#pragma once

#include "base/NonCopyable.hpp"
#include "net/SocketOps.hpp"
#include "net/InetAddress.hpp"

namespace cm {
	namespace net {
		class Socket : private NonCopyable {
		public:
			explicit Socket(const int fd) : sockFd_(fd) {}

			virtual ~Socket() { close(sockFd_); }

			int fd() const { return sockFd_; }

			void bindAddress(const InetAddress &addr) { bindOrDie(sockFd_, addr.getSocketAddr()); }

		private:
			const int sockFd_;
		};
	}
}