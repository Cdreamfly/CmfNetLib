#include <cstring>
#include "net/EPollPoller.hpp"
#include "net/Channel.hpp"

namespace {
	const int kNew = -1;    //未添加
	const int kAdded = 1;   //已添加
	const int kDeleted = 2; //已删除
}

void cm::net::EPollPoller::updateChannel(cm::net::Channel *channel) {
	const int index = channel->index();
	LOG_INFO("fd = %d events = %d index = %d", channel->fd(), channel->events(), index);
	//如果是未添加或已删除说明调用当前接口是执行要添加操作
	if (index == kNew || index == kDeleted) {
		if (index == kNew) {
			this->channels_[channel->fd()] = channel;
		}
		channel->setIndex(kAdded);
		update(EPOLL_CTL_ADD, channel);
	} else {    //否则如果是已添加状态则调用接口是要执行修改或删除
		if (channel->isNoneEvent()) {
			update(EPOLL_CTL_DEL, channel);
		} else {
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

void cm::net::EPollPoller::removeChannel(cm::net::Channel *channel) {
	channels_.erase(channel->fd());
	if (channel->index() == kNew) {
		update(EPOLL_CTL_DEL, channel);
	}
	channel->setIndex(kDeleted);
}

void cm::net::EPollPoller::fillActiveChannels(const int numEvents, cm::net::Poller::ChannelList *activeChannels) {
	for (int i = 0; i < numEvents; ++i) {
		auto *channel = static_cast<Channel *>(events_[i].data.ptr);
		channel->setReceivedEvents(static_cast<int>(events_[i].events));
		activeChannels->push_back(channel);
	}
}

void cm::net::EPollPoller::update(const int operation, cm::net::Channel *channel) const {
	epoll_event event{};
	memset(&event, 0, sizeof(event));
	event.events = channel->events();
	event.data.ptr = channel;
	const int fd = channel->fd();
	if (::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
		if (operation == EPOLL_CTL_DEL) {
			LOG_ERROR("epoll_ctl op = EPOLL_CTL_DEL fd =%d", fd);
		} else {
			LOG_ERROR("epoll_ctl op = EPOLL_CTL_ADD/MOD fd =%d", fd);
		}
	}
}

cm::Timestamp cm::net::EPollPoller::poll(int timeoutMs, cm::net::Poller::ChannelList *activeChannels) {
	const int numEvents = ::epoll_wait(epollFd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
	const int savedErrno = errno;
	if (numEvents > 0) {
		LOG_INFO("events happened");
		fillActiveChannels(numEvents, activeChannels);
		if (numEvents == events_.size()) {
			events_.resize(events_.size() * 2);
		}
	} else if (numEvents == 0) {
		LOG_ERROR("nothing happened");
	} else {
		if (savedErrno != EINTR) {
			errno = savedErrno;
			LOG_ERROR("EPollPoller::poll()");
		}
	}
	return cm::Timestamp::now();
}

