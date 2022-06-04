//
// Created by Cmf on 2022/5/31.
//

#ifndef CMFNETLIB_SOCKET_HPP
#define CMFNETLIB_SOCKET_HPP

#include "NetLib/base/noncopyable.h"
#include "NetLib/net/SocketOps.hpp"
#include "NetLib/net/InetAddress.hpp"

#include <netinet/tcp.h>
#include <fcntl.h>

class Socket : private noncopyable {
public:
    using ptr = std::shared_ptr<Socket>;

    explicit Socket(int fd) : _fd(fd) {

    }

    ~Socket() noexcept {
        Sockets::Close(_fd);
    }

    int GetFd() const {
        return _fd;
    }

    void Bind(const InetAddress::ptr addr) const {
        Sockets::Bind(_fd, addr->GetSockAddr());
    }

    void Listen() const {
        Sockets::Listen(_fd);
    }

    int Accept(InetAddress::ptr peeraddr) {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        int fd = Sockets::Accept(_fd, &addr);
        if (fd >= 0) {
            peeraddr->SetSockAddr(addr);
        }
        return fd;
    }

    void SetTcpNoDelay(bool on) {
        int optval = on ? 1 : 0;
        setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
    }

    void SetReuseAddr(bool on) {
        int optval = on ? 1 : 0;
        setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
    }

    void SetReusePort(bool on) {
        int optval = on ? 1 : 0;
        int ret = setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
        if (ret < 0 && on) {
            LOG_ERROR("setReusePort error!");
        }
    }

    void setKeepAlive(bool on) {
        int optval = on ? 1 : 0;
        setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
    }

    void setnonblocking() {
        fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | O_NONBLOCK);
    }

private:
    const int _fd;
};

#endif //CMFNETLIB_SOCKET_HPP
