#pragma once

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include "base/Log.hpp"

namespace cm {
    namespace net {
        int createNonblockingOrDie(const sa_family_t family) {
            int fd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TP);
            if (fd < 0) {
                LOG_FATAL("sockets::createNonblockingOrDie");
            }
            return fd;
        }

        void close(const int fd) {
            if (::close(fd) < 0) {
                LOG_FATAL("sockets::close");
            }
        }

        void bindOrDie(const int fd, const sockaddr *addr) {
            if (::bind(fd, addr, static_cast<socklen_t>(sizeof(sockaddr_in6))) < 0) {
                LOG_FATAL("sockets::bindOrDie");
            }
        }

        void listenOrDie(const int fd) {
            if (::listen(fd, SOMAXCONN) < 0) {
                LOG_FATAL("sockets::listen");
            }
        }

        int accept(const int fd, sockaddr_in6 *addr) {
            auto len = static_cast<socklen_t>(sizeof(*addr));
            int connFd = ::accept4(fd, (sockaddr *) addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
            if (fd < 0) {
                int savedErrno = errno;
                switch (savedErrno) {
                    case EAGAIN:
                    case ECONNABORTED:
                    case EINTR:
                    case EPROTO:
                    case EPERM:
                    case EMFILE:
                        errno = savedErrno;
                        break;
                    case EBADF:
                    case EFAULT:
                    case EINVAL:
                    case ENFILE:
                    case ENOBUFS:
                    case ENOMEM:
                    case ENOTSOCK:
                    case EOPNOTSUPP:
                    LOG_FATAL("unexpected error of ::accept %d", savedErrno);
                        break;
                    default:
                    LOG_FATAL("unknown error of ::accept %d", savedErrno);
                        break;
                }
            }
            return connFd;
        }

        int connect(const int fd, const sockaddr *addr) {
            return ::connect(fd, addr, static_cast<socklen_t>(sizeof(sockaddr_in6)));
        }

        void shutdownWrite(const int fd) {
            if (::shutdown(fd, SHUT_WR) < 0) {
                LOG_FATAL("sockets::shutdownWrite");
            }
        }

        ssize_t read(const int fd, void *buf, size_t count) {
            return ::read(fd, buf, count);
        }

        ssize_t write(const int fd, void *buf, size_t count) {
            return ::write(fd, buf, count);
        }
    }
}