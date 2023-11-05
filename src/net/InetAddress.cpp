#include "net/InetAddress.hpp"
#include "net/SocketOps.hpp"
#include <cstring>

cm::net::InetAddress::InetAddress(uint16_t port, const std::string& ip)
{
	bzero(&addr_, sizeof addr_);
	addr_.sin_family = AF_INET;
	addr_.sin_port = htons(port);
	addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string cm::net::InetAddress::toIp() const
{
	// addr_
	char buf[64] = {0};
	::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
	return buf;
}

std::string cm::net::InetAddress::toIpPort() const
{
	// ip:port
	char buf[64] = {0};
	::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
	size_t end = strlen(buf);
	uint16_t port = ntohs(addr_.sin_port);
	sprintf(buf+end, ":%u", port);
	return buf;
}

uint16_t cm::net::InetAddress::toPort() const
{
	return ntohs(addr_.sin_port);
}