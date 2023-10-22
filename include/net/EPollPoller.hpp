#pragma once

#include "net/Poller.hpp"
#include "base/Log.hpp"
#include <sys/epoll.h>
#include <csignal>

namespace cm::net {
	class EPollPoller : public Poller {
	public:
		explicit EPollPoller(EventLoop *loop) : Poller(loop),
		                                        epollFd_(epoll_create1(EPOLL_CLOEXEC)),
		                                        events_(kInitEventListSize) {
			if (epollFd_ < 0) {
				LOG_FATAL("EPollPoller::EPollPoller");
			}
		}

		~EPollPoller() override {
			::close(epollFd_);
		}

		cm::Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;

		void updateChannel(Channel *channel) override;

		void removeChannel(Channel *channel) override;

	private:
		void fillActiveChannels(int numEvents, ChannelList *activeChannels) ;

		void update(int operation, Channel *channel) const;

	private:
		static const int kInitEventListSize = 16;
		using EventList = std::vector<epoll_event>;
		int epollFd_;
		EventList events_;
	};
}