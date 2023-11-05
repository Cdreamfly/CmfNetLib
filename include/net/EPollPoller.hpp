#pragma once

#include "net/Poller.hpp"
#include "base/Log.hpp"
#include <sys/epoll.h>
#include <csignal>

namespace cm::net {
	class EPollPoller : public Poller {
	public:
		explicit EPollPoller(EventLoop *);

		~EPollPoller() override;

		Timestamp poll(int, ChannelList *) override;

		void updateChannel(Channel *) override;

		void removeChannel(Channel *) override;

	private:
		static const int kInitEventListSize = 16;

		void fillActiveChannels(int, ChannelList *) const;

		void update(int, Channel *) const;

		typedef std::vector<struct epoll_event> EventList;
	private:
		int epollFd_;
		EventList events_;
	};
}