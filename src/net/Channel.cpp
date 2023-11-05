#include "net/Channel.hpp"
#include "net/EventLoop.hpp"

const int cm::net::Channel::kNoneEvent = 0;
const int cm::net::Channel::kReadEvent = POLLIN | POLLPRI;
const int cm::net::Channel::kWriteEvent = POLLOUT;

// EventLoop: ChannelList Poller
cm::net::Channel::Channel(EventLoop *loop, int fd)
		: loop_(loop),
		  fd_(fd),
		  events_(0),
		  receivedEvents_(0),
		  index_(-1),
		  tied_(false) {}

cm::net::Channel::~Channel() = default;

// channel的tie方法什么时候调用过？一个TcpConnection新连接创建的时候 TcpConnection => Channel
void cm::net::Channel::tie(const std::shared_ptr<void> &obj) {
	tie_ = obj;
	tied_ = true;
}

/**
 * 当改变channel所表示fd的events事件后，update负责在poller里面更改fd相应的事件epoll_ctl
 * EventLoop => ChannelList   Poller
 */
void cm::net::Channel::update() {
	// 通过channel所属的EventLoop，调用poller的相应方法，注册fd的events事件
	loop_->updateChannel(this);
}

// 在channel所属的EventLoop中， 把当前的channel删除掉
void cm::net::Channel::remove() {
	loop_->removeChannel(this);
}

// fd得到poller通知以后，处理事件的
void cm::net::Channel::handleEvent(Timestamp receiveTime) {
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

// 根据poller通知的channel发生的具体事件， 由channel负责调用具体的回调操作
void cm::net::Channel::handleEventWithGuard(Timestamp receiveTime) {
	if ((receivedEvents_ & POLLHUP) && !(receivedEvents_ & POLLIN)) {
		if (closeCallback_) closeCallback_();
	}
	if (receivedEvents_ & (POLLERR | POLLNVAL)) {
		if (errorCallback_) errorCallback_();
	}
	if (receivedEvents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
		if (readCallback_) readCallback_(receiveTime);
	}
	if (receivedEvents_ & POLLOUT) {
		if (writeCallback_) writeCallback_();
	}
}
