#pragma once

#include "base/NonCopyable.hpp"
#include "base/Timestamp.hpp"
#include <vector>
#include <unordered_map>

namespace cm::net {
	class Channel;

	class EventLoop;

	class Poller : private NonCopyable {
	public:
		using ChannelList = std::vector<Channel *>;

		explicit Poller(EventLoop *loop) : ownerLoop_(loop) {}

		virtual ~Poller() = default;

		virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

		virtual void updateChannel(Channel *channel) = 0;

		virtual void removeChannel(Channel *channel) = 0;

		virtual bool hasChannel(Channel *channel);

		static Poller *newDefaultPoller(EventLoop *loop);

	protected:
		using ChannelMap = std::unordered_map<int, Channel *>;
		ChannelMap channels_;
	private:
		EventLoop *ownerLoop_;
	};
}