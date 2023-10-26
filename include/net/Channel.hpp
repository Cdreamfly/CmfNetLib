#pragma once

#include <functional>
#include <memory>
#include <sys/poll.h>

#include "base/Timestamp.hpp"
#include "base/NonCopyable.hpp"

/**
 * Channel理解为通道，封装了sockFd和其感兴趣的event，如EPOLLIN,EPOLLOUT事件，还绑定了Poller返回的具体事件
 */

namespace cm::net {
	class EventLoop;

	class Channel : private NonCopyable {
	public:
		using EventCallback = std::function<void()>;
		using ReadEventCallback = std::function<void(const Timestamp &)>;

		Channel(EventLoop *loop, const int fd) : loop_(loop), fd_(fd),
		                                         events_(0), receivedEvents(0), index_(-1), tied_(false) {}

		/**
		 * 当对方断开TCP连接，这个IO事件会触发Channel::handleEvent()调用，后者会调用用户提供的CloseCallback，而用户代码在onClose()中有可能析构Channel对象，这就造成了灾难。
		 * 等于说Channel::handleEvent()执行到一半的时候，其所属的Channel对象本身被销毁了。这时程序立刻core dump就是最好的结果了。
		 * 解决办法是提供Channel::tie(const boost::shared_ptr<void>&)这个函数，用于延长某些对象（可以是Channel对象，也可以是其owner对象）的生命期，使之长过Channel::handleEvent()函数。
		 * @param receiveTime
		 */
		void handleEvent(const Timestamp &receiveTime);

		void setReadCallback(const ReadEventCallback &cb) { readCallback_ = cb; }

		void setWriteCallback(const EventCallback &cb) { writeCallback_ = cb; }

		void setCloseCallback(const EventCallback &cb) { closeCallback_ = cb; }

		void setErrorCallback(const EventCallback &cb) { errorCallback_ = cb; }

		[[nodiscard]] bool isNoneEvent() const { return events_ == kNoneEvent; }

		[[nodiscard]] bool isWriting() const { return events_ == kWriteEvent; }

		[[nodiscard]] bool isReading() const { return events_ == kReadEvent; }

		void enableReading() {
			events_ |= kReadEvent;
			update();
		}

		void disableReading() {
			events_ &= ~kReadEvent;
			update();
		}

		void enableWriting() {
			events_ |= kWriteEvent;
			update();
		}

		void disableWriting() {
			events_ &= ~kWriteEvent;
			update();
		}

		void disableAll() {
			events_ |= kNoneEvent;
			update();
		}

		/**
		 * 防止当Channel被手动remove掉，Channel还在执行回调函数
		 * @param obj
		 */
		void tie(const std::shared_ptr<void> &obj) {
			tie_ = obj;
			tied_ = true;
		}

		[[nodiscard]] int fd() const { return fd_; }

		[[nodiscard]] int events() const { return events_; }

		void setReceivedEvents(const int receivedEvent) { receivedEvents = receivedEvent; }

		[[nodiscard]] int index() const { return index_; }

		void setIndex(const int index) { index_ = index; }

		[[nodiscard]] EventLoop *ownerLoop() const { return loop_; }

		void remove();

	private:
		/**
		 * 当改变Channel所表示fd的events事件后，update负责在poller里面改变fd响应的事件epoll_ctl
		 *
		 */
		void update();

		void handleEventWithGuard(const Timestamp &receiveTime) {
			if ((events_ & POLLHUP) && !(events_ & POLLIN)) {
				if (closeCallback_) closeCallback_();
			}
			if (events_ & (POLLERR | POLLNVAL)) {
				if (errorCallback_)errorCallback_();
			}
			if (events_ & (POLLIN | POLLPRI | POLLRDHUP)) {
				if (readCallback_) readCallback_(receiveTime);
			}
			if (events_ & POLLOUT) {
				if (writeCallback_) writeCallback_();
			}
		}

		static const int kNoneEvent = 0;
		static const int kReadEvent = POLLIN | POLLPRI;
		static const int kWriteEvent = POLLOUT;

	private:
		EventLoop *loop_{};     //事件循环
		const int fd_;          //fd，Poller监听的对象
		int events_{};          //注册fd感兴趣的事件
		int receivedEvents{};   //Poller返回的具体发生的事件
		int index_{};

		std::weak_ptr<void> tie_;
		bool tied_{};

		ReadEventCallback readCallback_;
		EventCallback writeCallback_;
		EventCallback closeCallback_;
		EventCallback errorCallback_;
	};
}