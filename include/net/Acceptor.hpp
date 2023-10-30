#pragma once

#include "base/NonCopyable.hpp"
#include "net/Channel.hpp"
#include "net/Socket.hpp"

#include <functional>

namespace cm::net {
	class EventLoop;

	class InetAddress;

	class Acceptor : private NonCopyable {
	public:
		using NewConnectionCallback = std::function<void(const int, const InetAddress &)>;

		explicit Acceptor(EventLoop *, const InetAddress &, bool);

		virtual ~Acceptor();

		void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }

		[[nodiscard]] bool listening() const { return listening_; }

		void listen();

	private:
		EventLoop *loop_;
		Socket acceptSocket_;
		Channel acceptChannel_;
		NewConnectionCallback newConnectionCallback_;
		bool listening_;
	};
}