#pragma once

#include "base/Timestamp.hpp"
#include <functional>
#include <memory>

namespace cm {
	class Buffer;

	class TcpConnection;

	using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
	using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
	using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
	using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
	using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;
	using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;
}