#include "net/Poller.hpp"
#include "net/EPollPoller.hpp"

cm::net::Poller *cm::net::Poller::newDefaultPoller(EventLoop *loop) {
	return new EPollPoller(loop);
}