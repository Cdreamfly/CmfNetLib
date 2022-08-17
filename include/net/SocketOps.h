//
// Created by Cmf on 2022/5/31.
//

#pragma once

#include "base/Log.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/uio.h>


class Sockets {
public:
    static int CreateNonblockingSocket() {
        int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (fd < 0) {
            LOG_FATAL("Sockets::CreateNonblockingSocket");
        }
        return fd;
    }

    static void Bind(int fd, const sockaddr *addr) {
        if (bind(fd, addr, sizeof(addr)) < 0) {
            LOG_FATAL("Sockets::Bind");
        }
    }

    static void Listen(int fd) {
        if (listen(fd, SOMAXCONN) < 0) {
            LOG_FATAL("Sockets::Listen");
        }
    }

    static int Accept(int fd, sockaddr_in *addr) {
        socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
        int connfd = accept4(fd, (sockaddr *) addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
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

    static void Close(int fd) {
        if (close(fd) < 0) {
            LOG_FATAL("Sockets::Close");
        }
    }

    static void ShutdownWrite(int fd) {
        if (shutdown(fd, SHUT_WR) < 0) {
            LOG_FATAL("sockets::shutdownWrite");
        }
    }

    static ssize_t Readv(int fd, const iovec *iov, int iovcnt) {
        return readv(fd, iov, iovcnt);
    }

    static ssize_t Write(int fd, const void *buf, size_t size) {
        return write(fd, buf, size);
    }
};