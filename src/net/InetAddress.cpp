#include "net/InetAddress.hpp"

#include <cstring>

cm::net::InetAddress::InetAddress(uint16_t port, bool loopBackOnly, bool ipv6) : ipv6_(ipv6) {
	if (ipv6_) {
		memset(&addr6_, 0, sizeof(addr6_));
		addr6_.sin6_family = AF_INET6;
		addr6_.sin6_port = htobe16(port);
		addr6_.sin6_addr = loopBackOnly ? in6addr_loopback : in6addr_any;
	} else {
		memset(&addr_, 0, sizeof(addr_));
		addr_.sin_family = AF_INET;
		addr_.sin_port = htobe16(port);
		addr_.sin_addr.s_addr = htobe32(loopBackOnly ? INADDR_LOOPBACK : INADDR_ANY);
	}
}

cm::net::InetAddress::InetAddress(const std::string &ip, uint16_t port, bool ipv6) : ipv6_(ipv6) {
	if (ipv6_) {
		memset(&addr6_, 0, sizeof(addr6_));
		addr6_.sin6_family = AF_INET6;
		addr6_.sin6_port = htobe16(port);
		::inet_pton(AF_INET6, ip.c_str(), &addr6_.sin6_addr);
	} else {
		memset(&addr_, 0, sizeof(addr_));
		addr_.sin_family = AF_INET;
		addr_.sin_port = htons(port);
		::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
	}
}

std::string cm::net::InetAddress::toIp() const {
	char buf[64] = {0};
	if (ipv6_) {
		::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<socklen_t>(sizeof(addr6_)));
	} else {
		::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof(addr_)));
	}
	return buf;
}