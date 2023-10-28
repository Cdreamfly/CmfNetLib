#include "net/EventLoopThread.hpp"
#include "net/EventLoop.hpp"

cm::net::EventLoopThread::EventLoopThread(ThreadInitCallback cb,
                                          const std::string &name) : loop_(nullptr),
                                                                     exiting_(false),
                                                                     mutex_(),
                                                                     thread_([&] {
	                                                                     EventLoop loop; //创建一个独立的EventLoop，和上面的线程一一对应， one loop per thread
	                                                                     if (callback_) {
		                                                                     callback_(&loop);
	                                                                     }
	                                                                     {
		                                                                     std::unique_lock<std::mutex> lock{mutex_};
		                                                                     loop_ = &loop;
		                                                                     cond_.notify_one();
	                                                                     }
	                                                                     loop.loop();
	                                                                     std::unique_lock<std::mutex> lock{mutex_};
	                                                                     loop_ = nullptr;
                                                                     }, name),
                                                                     cond_(),
                                                                     callback_(std::move(cb)) {
}

cm::net::EventLoopThread::~EventLoopThread() {
	exiting_ = true;
	if (loop_ != nullptr) {
		loop_->quit();
		thread_.join();
	}
}

cm::net::EventLoop *cm::net::EventLoopThread::startLoop() {
	thread_.start();
	EventLoop *loop = nullptr;
	{
		std::unique_lock<std::mutex> lock{mutex_};
		while (loop_ == nullptr) {
			cond_.wait(lock);
		}
		loop = loop_;
	}
	return loop;
}