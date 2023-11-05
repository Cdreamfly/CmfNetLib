#include <cstring>
#include "net/EPollPoller.hpp"
#include "net/Channel.hpp"

namespace {
	// channel未添加到poller中
	const int kNew = -1;  // channel的成员index_ = -1
	// channel已添加到poller中
	const int kAdded = 1;
	// channel从poller中删除
	const int kDeleted = 2;
}


cm::net::EPollPoller::EPollPoller(EventLoop *loop)
		: Poller(loop), epollFd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)  // vector<epoll_event>
{
	if (epollFd_ < 0) {
		LOG_FATAL("epoll_create error:%d", errno);
	}
}

cm::net::EPollPoller::~EPollPoller() {
	::close(epollFd_);
}

cm::Timestamp cm::net::EPollPoller::poll(const int timeoutMs, ChannelList *activeChannels) {
	// 实际上应该用LOG_DEBUG输出日志更为合理
	LOG_INFO("=> fd total count:%lu", channels_.size());
	int numEvents = ::epoll_wait(epollFd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
	int saveErrno = errno;
	Timestamp now(Timestamp::now());

	if (numEvents > 0) {
		LOG_INFO("%d events happened", numEvents);
		fillActiveChannels(numEvents, activeChannels);
		if (numEvents == events_.size()) {
			events_.resize(events_.size() * 2);
		}
	} else if (numEvents == 0) {
		LOG_DEBUG("timeout!");
	} else {
		if (saveErrno != EINTR) {
			errno = saveErrno;
			LOG_ERROR("EPollPoller::poll() err!");
		}
	}
	return now;
}

// channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel
/**
 *            EventLoop  =>   poller.poll
 *     ChannelList      Poller
 *                     ChannelMap  <fd, channel*>   epollfd
 */
void cm::net::EPollPoller::updateChannel(Channel *channel) {
	const int index = channel->index();
	LOG_INFO("=> fd=%d events=%d index=%d", channel->fd(), channel->events(), index);

	if (index == kNew || index == kDeleted) {
		if (index == kNew) {
			int fd = channel->fd();
			channels_[fd] = channel;
		}
		channel->setIndex(kAdded);
		update(EPOLL_CTL_ADD, channel);
	} else  // channel已经在poller上注册过了
	{
		if (channel->isNoneEvent()) {
			update(EPOLL_CTL_DEL, channel);
			channel->setIndex(kDeleted);
		} else {
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

// 从poller中删除channel
void cm::net::EPollPoller::removeChannel(Channel *channel) {
	channels_.erase(channel->fd());
	if (channel->index() == kAdded) {
		update(EPOLL_CTL_DEL, channel);
	}
	channel->setIndex(kNew);
}

// 填写活跃的连接
void cm::net::EPollPoller::fillActiveChannels(const int numEvents, ChannelList *activeChannels) const {
	for (int i = 0; i < numEvents; ++i) {
		auto *channel = static_cast<Channel *>(events_[i].data.ptr);
		channel->setReceivedEvents_(static_cast<int>(events_[i].events));
		activeChannels->push_back(channel); // EventLoop就拿到了它的poller给它返回的所有发生事件的channel列表了
	}
}

// 更新channel通道 epoll_ctl add/mod/del
void cm::net::EPollPoller::update(const int operation, Channel *channel) const {
	epoll_event event{};
	memset(&event, 0, sizeof(event));
	const int fd = channel->fd();
	event.events = channel->events();
	event.data.fd = fd;
	event.data.ptr = channel;

	if (::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
		if (operation == EPOLL_CTL_DEL) {
			LOG_ERROR("epoll_ctl del error:%d", errno);
		} else {
			LOG_FATAL("epoll_ctl add/mod error:%d", errno);
		}
	}
}

