#include "net/InetAddress.h"

#include <cstring>

cm::net::InetAddress::InetAddress(const sockaddr_in &addr) : _addr(addr) {}

cm::net::InetAddress::InetAddress(const std::string &ip, uint16_t port) {
    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);
    _addr.sin_addr.s_addr = ::inet_addr(ip.c_str());
}

std::string cm::net::InetAddress::ip() const {
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &_addr.sin_addr, buf, static_cast<socklen_t>(sizeof(_addr)));
    return buf;
}

uint16_t cm::net::InetAddress::port() const {
    return be16toh(_addr.sin_port);
}

std::string cm::net::InetAddress::toIpPort() const {
    return this->ip() + ":" + std::to_string(this->port());
}

sa_family_t cm::net::InetAddress::family() const {
    return _addr.sin_family;
}

const sockaddr *cm::net::InetAddress::getSocketAddr() const {
    return (const sockaddr *) &_addr;
}
