#include "cm/net/Poller.hpp"
#include "cm/net/EPollPoller.hpp"

cm::net::Poller *cm::net::Poller::newDefaultPoller(EventLoop *loop) {
	return new EPollPoller(loop);
}