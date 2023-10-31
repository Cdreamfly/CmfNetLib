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

		cm::Timestamp poll(int, ChannelList *) override;

		void updateChannel(Channel *) override;

		void removeChannel(Channel *) override;

	private:
		void fillActiveChannels(int, ChannelList *);

		void update(int, Channel *) const;

		using EventList = std::vector<epoll_event>;
		static const int kInitEventListSize = 16;
	private:


		int epollFd_;
		EventList events_;
	};
}