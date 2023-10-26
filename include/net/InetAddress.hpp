#pragma once

#include "base/Copyable.hpp"

#include <arpa/inet.h>
#include <string>

namespace cm::net {
	class InetAddress : public Copyable {
	public:
		explicit InetAddress(uint16_t port = 0, bool loopBackOnly = false, bool ipv6 = false);

		explicit InetAddress(const std::string &ip, uint16_t port, bool ipv6 = false);

		explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}

		explicit InetAddress(const sockaddr_in6 &addr) : addr6_(addr), ipv6_(true) {}

		[[nodiscard]] std::string toIp() const;

		[[nodiscard]] uint16_t toPort() const { return ipv6_ ? be16toh(addr6_.sin6_port) : be16toh(addr_.sin_port); }

		[[nodiscard]] std::string toIpPort() const { return this->toIp() + ":" + std::to_string(this->toPort()); }

		[[nodiscard]] sa_family_t family() const { return ipv6_ ? addr6_.sin6_family : addr_.sin_family; }

		[[nodiscard]] const sockaddr *getSocketAddr() const {
			return ipv6_ ? (const sockaddr *) &addr6_ : (const sockaddr *) &addr_;
		}

		void setSocketAddr(const sockaddr_in &addr) {
			addr_ = addr;
			ipv6_ = false;
		}

		void setSocketAddr6(const sockaddr_in6 &addr) {
			addr6_ = addr;
			ipv6_ = true;
		}

	private:
		sockaddr_in addr_{};
		sockaddr_in6 addr6_{};
		bool ipv6_ = false;
	};
}