#pragma once

#include "base/copyable.hpp"

#include <arpa/inet.h>
#include <string>

namespace cm {
    namespace net {
        class InetAddress : public copyable {
        public:
            explicit InetAddress(const sockaddr_in &addr);

            explicit InetAddress(const std::string &ip = "127.0.0.1", uint16_t port = 80);

            std::string ip()const;

            uint16_t port()const;

            std::string toIpPort()const;
            sa_family_t  family()const;

            const sockaddr* getSocketAddr()const;
        private:
            sockaddr_in _addr{};
        };
    }
}