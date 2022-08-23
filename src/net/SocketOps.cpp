//
// Created by Cmf on 2022/8/22.
//

#include "net/SocketOps.hpp"


int SocketOps::CreateNonblockingSocket(sa_family_t family) {
    int fd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (fd < 0) {
        LOG_FATAL("Sockets::CreateNonblockingSocket");
    }
    return fd;
}

void SocketOps::Bind(int fd, const sockaddr *addr) {
    if (::bind(fd, addr, sizeof(sockaddr_in)) < 0) {
        LOG_FATAL("Sockets::Bind");
    }
}

void SocketOps::Listen(int fd) {
    if (::listen(fd, SOMAXCONN) < 0) {
        LOG_FATAL("Sockets::Listen");
    }
}

int SocketOps::Accept(int fd, sockaddr_in *addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = ::accept4(fd, (sockaddr *) addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        LOG_FATAL("Sockets::Accept");
        int no = errno;
        switch (no) {
            case EMFILE:
                errno = no;
                break;
            case EOPNOTSUPP:
                LOG_FATAL("unexpected error of ::accept %d", no);
                break;
        }
    }
    return connfd;
}

void SocketOps::Close(int fd) {
    if (::close(fd) < 0) {
        LOG_FATAL("Sockets::Close");
    }
}

void SocketOps::ShutdownWrite(int fd) {
    if (::shutdown(fd, SHUT_WR) < 0) {
        LOG_FATAL("sockets::shutdownWrite");
    }
}

ssize_t SocketOps::Readv(int fd, const iovec *iov, int iovcnt) {
    return ::readv(fd, iov, iovcnt);
}

ssize_t SocketOps::Write(int fd, const void *buf, size_t size) {
    return ::write(fd, buf, size);
}
