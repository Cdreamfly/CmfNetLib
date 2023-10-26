#include "net/Channel.hpp"

void cm::net::Channel::update() {

}

void cm::net::Channel::remove() {

}

void cm::net::Channel::handleEvent(const cm::Timestamp &receiveTime) {
	if (tied_) {
		//如果当前 weak_ptr 已经过期，则该函数会返回一个空的 shared_ptr 指针；反之，该函数返回一个和当前 weak_ptr 指向相同的 shared_ptr 指针。
		std::shared_ptr<void> guard = tie_.lock();
		if (guard) {
			handleEventWithGuard(receiveTime);
		}
	} else {
		handleEventWithGuard(receiveTime);
	}
}
