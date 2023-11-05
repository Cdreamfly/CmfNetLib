#include "net/EventLoopThread.hpp"

#include <utility>
#include "net/EventLoop.hpp"


cm::net::EventLoopThread::EventLoopThread(ThreadInitCallback cb, const std::string &name)
		: loop_(nullptr), exiting_(false),
		  thread_([this] { threadFunc(); }, name),
		  mutex_(), cond_(), callback_(std::move(cb)) {

}

cm::net::EventLoopThread::~EventLoopThread() {
	exiting_ = true;
	if (loop_ != nullptr) {
		loop_->quit();
		thread_.join();
	}
}

cm::net::EventLoop *cm::net::EventLoopThread::startLoop() {
	thread_.start(); // 启动底层的新线程
	EventLoop *loop = nullptr;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (loop_ == nullptr) {
			cond_.wait(lock);
		}
		loop = loop_;
	}
	return loop;
}

// 下面这个方法，实在单独的新线程里面运行的
void cm::net::EventLoopThread::threadFunc() {
	EventLoop loop; // 创建一个独立的event loop，和上面的线程是一一对应的，one loop per thread
	if (callback_) callback_(&loop);

	{
		std::unique_lock<std::mutex> lock{mutex_};
		loop_ = &loop;
		cond_.notify_one();
	}

	loop.loop(); // EventLoop loop  => Poller.poll
	std::unique_lock<std::mutex> lock{mutex_};
	loop_ = nullptr;
}