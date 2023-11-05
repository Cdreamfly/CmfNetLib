#include "cm/net/Poller.hpp"
#include "cm/net/Channel.hpp"

cm::net::Poller::Poller(EventLoop *loop)
		: ownerLoop_(loop) {}

bool cm::net::Poller::hasChannel(Channel *channel) const {
	auto it = channels_.find(channel->fd());
	return it != channels_.end() && it->second == channel;
}
