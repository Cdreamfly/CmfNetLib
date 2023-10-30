#pragma once

#include <netinet/tcp.h>
#include <arpa/inet.h>

namespace cm::net::sockets {

	int createNonblockingOrDie(sa_family_t);

	void close(int);

	void bindOrDie(int, const sockaddr *);

	void listenOrDie(int);

	int accept(int, sockaddr_in6 *);

	int connect(int, const sockaddr *);

	void shutdownWrite(int);

	ssize_t read(int, void *, size_t);

	ssize_t readv(int, const iovec *, int);

	ssize_t write(int, const void *, size_t);

	void setTcpNoDelay(int, bool);

	void setReuseAddr(int, bool);

	void setReusePort(int, bool);

	void setKeepAlive(int, bool);

	sockaddr_in6 getLocalAddr(int);

	int getSocketError(int);
}