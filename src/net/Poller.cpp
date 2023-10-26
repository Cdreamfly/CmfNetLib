#include "net/Poller.hpp"
#include "net/Channel.hpp"

bool cm::net::Poller::hasChannel(Channel *channel) {
	auto it = channels_.find(channel->fd());
	return it != channels_.end() && it->second == channel;
}
