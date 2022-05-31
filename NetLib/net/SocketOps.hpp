//
// Created by Cmf on 2022/5/31.
//

#ifndef CMFNETLIB_SOCKETOPS_HPP
#define CMFNETLIB_SOCKETOPS_HPP

#include "NetLib/log/Log.hpp"
#include <arpa/inet.h>
#include <unistd.h>


namespace Sockets {
    int CreateNonblockingSocket() {
        int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (fd < 0) {
            LOG_FATAL("Sockets::CreateNonblockingSocket");
        }
        return fd;
    }

    void Bind(int fd, const sockaddr *addr) {
        if (bind(fd, addr, sizeof(addr)) < 0) {
            LOG_FATAL("Sockets::Bind");
        }
    }

    void Listen(int fd) {
        if (listen(fd, SOMAXCONN) < 0) {
            LOG_FATAL("Sockets::Listen");
        }
    }

    int Accept(int fd, sockaddr_in *addr) {
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

    void Close(int fd) {
        if (close(fd) < 0) {
            LOG_FATAL("Sockets::Close");
        }
    }
}

#endif //CMFNETLIB_SOCKETOPS_HPP
