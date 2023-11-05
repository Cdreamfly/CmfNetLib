#include "cm/net/EventLoop.hpp"
#include "cm/net/Poller.hpp"
#include "cm/net/Channel.hpp"
#include "cm/net/SocketOps.hpp"
#include "cm/base/Log.hpp"
#include <sys/eventfd.h>
#include <csignal>

namespace {
	//防止一个线程创建多个EventLoop __thread：thread_local
	//当一个event loop创建起来它就指向那个对象，在一个线程里再去创建一个对象，由于这个指针为空，就不创建
	__thread cm::net::EventLoop *t_loopInThisThread = nullptr;
	//定义默认的Poller IO复用接口的超时时间
	const int kPollTimeMs = 10000;//10秒钟

	int createEventFd() {
		int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (evtfd < 0) {
			LOG_FATAL("eventFd error:%d", errno);
			abort();
		}
		return evtfd;
	}
}

cm::net::EventLoop::EventLoop()
		: looping_(false),
		  quit_(false),
		  threadId_(CurrentThread::tid()),
		  poller_(Poller::newDefaultPoller(this)),
		  wakeupFd_(createEventFd()),
		  wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
		  callingPendingFunctors_(false) {
	if (t_loopInThisThread) {
		LOG_FATAL("Another EventLoop %p exists in this thread %d", t_loopInThisThread, threadId_);
	} else {
		t_loopInThisThread = this;
	}
	// 设置wakeup fd的事件类型以及发生事件后的回调操作
	wakeupChannel_->setReadCallback([this](Timestamp) {
		uint64_t one = 1;
		ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
		if (n != sizeof one) {
			LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
		}
	});
	// 每一个event loop都将监听wakeup channel的EPOLLIN读事件了
	wakeupChannel_->enableReading();
}

cm::net::EventLoop::~EventLoop() {
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = nullptr;
}

// 开启事件循环
void cm::net::EventLoop::loop() {
	looping_ = true;
	quit_ = false;

	LOG_INFO("EventLoop %p start looping", this);

	while (!quit_) {
		activeChannels_.clear();
		// 监听两类fd   一种是client的fd，一种wakeup fd
		pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
		for (Channel *channel: activeChannels_) {
			// Poller监听哪些channel发生事件了，然后上报给EventLoop，通知channel处理相应的事件
			channel->handleEvent(pollReturnTime_);
		}
		// 执行当前EventLoop事件循环需要处理的回调操作
		/**
		 * IO线程 mainLoop accept fd《=channel sub loop
		 * mainLoop 事先注册一个回调cb（需要sub loop来执行）    wakeup sub loop后，执行下面的方法，执行之前mainloop注册的cb操作
		 */
		doPendingFunctors();
	}

	LOG_INFO("EventLoop %p stop looping. \n", this);
	looping_ = false;
}

void cm::net::EventLoop::quit() {
	quit_ = true;
	// 如果是在其它线程中，调用的quit   在一个sub loop(worker)中，调用了mainLoop(IO)的quit
	if (!isInLoopThread()) {
		wakeup();
	}
}

// 在当前loop中执行cb
void cm::net::EventLoop::runInLoop(const Functor &cb) {
	if (isInLoopThread()) // 在当前的loop线程中，执行cb
	{
		cb();
	} else // 在非当前loop线程中执行cb , 就需要唤醒loop所在线程，执行cb
	{
		queueInLoop(cb);
	}
}

// 把cb放入队列中，唤醒loop所在的线程，执行cb
void cm::net::EventLoop::queueInLoop(const Functor &cb) {
	{
		std::unique_lock<std::mutex> lock{mutex_};
		pendingFunctors_.emplace_back(cb);
	}
	// 唤醒相应的，需要执行上面回调操作的loop的线程了
	// || callingPendingFunctors_的意思是：当前loop正在执行回调，但是loop又有了新的回调
	if (!isInLoopThread() || callingPendingFunctors_) {
		wakeup(); // 唤醒loop所在线程
	}
}

// 用来唤醒loop所在的线程的  向wakeup fd_写一个数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
void cm::net::EventLoop::wakeup() const {
	const uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one) {
		LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8", n);
	}
}

// EventLoop的方法 =》 Poller的方法
void cm::net::EventLoop::updateChannel(Channel *channel) {
	poller_->updateChannel(channel);
}

void cm::net::EventLoop::removeChannel(Channel *channel) {
	poller_->removeChannel(channel);
}

bool cm::net::EventLoop::hasChannel(Channel *channel) {
	return poller_->hasChannel(channel);
}

void cm::net::EventLoop::doPendingFunctors() // 执行回调
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	{
		std::unique_lock<std::mutex> lock{mutex_};
		functors.swap(pendingFunctors_);
	}

	for (const Functor &functor: functors) {
		functor(); // 执行当前loop需要执行的回调操作
	}
	callingPendingFunctors_ = false;
}
