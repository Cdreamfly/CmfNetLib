#pragma once

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include "base/Log.hpp"

namespace cm {
	namespace net {
		void close(const int fd) {
			if (::close(fd) < 0) {
				LOG_FATAL("sockets::close!");
			}
		}

		void bindOrDie(const int fd, const sockaddr *addr) {
			if (::bind(fd, addr, static_cast<socklen_t>(sizeof(sockaddr_in6))) < 0) {
				LOG_FATAL("sockets::bindOrDie");
			}
		}
	}
}