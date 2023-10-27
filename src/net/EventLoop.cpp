#include "net/EventLoop.hpp"
#include "net/Poller.hpp"
#include "net/Channel.hpp"
#include "net/SocketOps.hpp"
#include "base/Log.hpp"
#include <sys/eventfd.h>

namespace {
//防止一个线程创建多个EventLoop   __thread：thread_local
//当一个event loop创建起来它就指向那个对象，在一个线程里再去创建一个对象，由于这个指针为空，就不创建
	__thread cm::net::EventLoop *t_loopInThisThread = nullptr;
//定义默认的Poller IO复用接口的超时时间
	const int kPollTimeMs = 10000;//10秒钟
}

cm::net::EventLoop::EventLoop() : looping_(false), quit_(false), callingPendingFunctors_(false),
                                  threadId_(CurrentThread::tid()), poller_(Poller::newDefaultPoller(this)),
                                  wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
                                  currentActiveChannel_(nullptr) {
	wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (wakeupFd_ < 0) {
		LOG_FATAL("Failed in eventfd");
		abort();
	}
	if (t_loopInThisThread) {
		LOG_FATAL("Another EventLoop %p exists in this base %d", t_loopInThisThread, t_loopInThisThread);
	} else {
		t_loopInThisThread = this;
	}
	//设置wakeup fd的事件类型以及发生事件后的回调操作
	wakeupChannel_->setReadCallback([&](const Timestamp &) {
		uint64_t one = 1;
		ssize_t n = sockets::read(wakeupFd_, &one, sizeof(one));
		if (n != sizeof(one)) {
			LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
		}
	});
	//每一个event loop都将监听wakeup channel的EPOLLIN读事件了
	wakeupChannel_->enableReading();
}

//开启事件循环 驱动底层的poller执行poll
void cm::net::EventLoop::loop() {
	looping_ = true;
	quit_ = false;
	while (!quit_) {
		activeChannels_.clear();
		//监听两类fd， client fd wake fd
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
		for (Channel *channel: activeChannels_) {
			//Poller监听哪些channel发生事件了，然后上报给EventLoop，通知channel处理相应的事件
			channel->handleEvent(pollReturnTime_);
		}
		/**
		 * 执行当前EventLoop事件循环需要处理的回调操作
		 * IO线程mainLoop accept fd <= channel subloop
		 * mainLoop事先注册一个回调，需要subLoop来执行，wakeup subLoop后，执行下面的方法，执行执行之前mainLoop注册的回调
		 */
		doPendingFunctors();
	}
}

void cm::net::EventLoop::quit() {
	quit_ = true;
	//如果在其他线程中调用，在一个subLoop中调用mainLoop quit，需要先让其工作起来回到while，然后判断quit退出loop
	if (!isInLoopThread()) {
		wakeup();
	}
}

void cm::net::EventLoop::runInLoop(const cm::net::EventLoop::Functor &cb) {
	if (isInLoopThread()) {
		cb();
	} else {
		queueInLoop(cb);
	}
}

void cm::net::EventLoop::queueInLoop(const cm::net::EventLoop::Functor &cb) {
	{
		std::unique_lock<std::mutex> lock{mutex_};
		pendingFunctors_.emplace_back(cb);
	}
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup();
	}
}

void cm::net::EventLoop::wakeup() const {
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof(one));
	if (n != sizeof(one)) {
		LOG_ERROR("EventLoop::wakeup() writes %d bytes instead of 8", n);
	}
}

void cm::net::EventLoop::updateChannel(cm::net::Channel *channel) {
	poller_->updateChannel(channel);
}

void cm::net::EventLoop::removeChannel(cm::net::Channel *channel) {
	poller_->removeChannel(channel);
}

bool cm::net::EventLoop::hasChannel(cm::net::Channel *channel) {
	return poller_->hasChannel(channel);
}

void cm::net::EventLoop::doPendingFunctors() {
	std::vector<Functor> functions;
	callingPendingFunctors_ = true;
	{
		std::unique_lock<std::mutex> lock{mutex_};
		functions.swap(pendingFunctors_);
	}
	for (const Functor &functor: functions) {
		functor();
	}
	callingPendingFunctors_ = false;
}

cm::net::EventLoop::~EventLoop() {
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = nullptr;
}
