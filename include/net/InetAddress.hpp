#pragma once

#include "base/Copyable.hpp"

#include <arpa/inet.h>
#include <string>

namespace cm::net {
	class InetAddress : public Copyable {
	public:
		explicit InetAddress(uint16_t port = 0, bool loopBackOnly = false);

		explicit InetAddress(const std::string &ip, uint16_t port);

		explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}

		[[nodiscard]] std::string toIp() const;

		[[nodiscard]] uint16_t toPort() const { return be16toh(addr_.sin_port); }

		[[nodiscard]] std::string toIpPort() const { return this->toIp() + ":" + std::to_string(this->toPort()); }

		[[nodiscard]] sa_family_t family() const { return addr_.sin_family; }

		[[nodiscard]] const sockaddr *getSocketAddr() const {
			return (const sockaddr *) &addr_;
		}

		void setSocketAddr(const sockaddr_in &addr) { addr_ = addr; }

	private:
		sockaddr_in addr_{};
	};
}