#include "net/InetAddress.hpp"
#include "net/SocketOps.hpp"
#include <cstring>

cm::net::InetAddress::InetAddress(uint16_t port, bool loopBackOnly) {
	memset(&addr_, 0, sizeof(addr_));
	addr_.sin_family = AF_INET;
	addr_.sin_port = htobe16(port);
	addr_.sin_addr.s_addr = htobe32(loopBackOnly ? INADDR_LOOPBACK : INADDR_ANY);
}

cm::net::InetAddress::InetAddress(const std::string &ip, uint16_t port) {
	memset(&addr_, 0, sizeof(addr_));
	sockets::fromIpPort(ip.c_str(), port, &addr_);
}

std::string cm::net::InetAddress::toIp() const {
	char buf[64] = {0};
	::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof(addr_)));
	return buf;
}