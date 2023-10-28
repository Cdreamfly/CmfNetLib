#include "net/EventLoopThreadPool.hpp"
#include "net/EventLoopThread.hpp"
#include "net/EventLoop.hpp"

cm::net::EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, std::string nameArg) : baseLoop_(baseLoop),
                                                                                              name_(std::move(nameArg)),
                                                                                              started_(false),
                                                                                              numThreads_(0),
                                                                                              next_(0) {
}

cm::net::EventLoopThreadPool::~EventLoopThreadPool() = default;

void cm::net::EventLoopThreadPool::start(const cm::net::EventLoopThreadPool::ThreadInitCallback &cb) {
	started_ = true;
	for (int i = 0; i < numThreads_; ++i) {
		char buf[name_.size() + 32];
		snprintf(buf, sizeof(buf), "%s-%d", name_.c_str(), i);
		auto *t = new EventLoopThread(cb, buf);
		threads_.push_back(std::unique_ptr<EventLoopThread>(t));
		loops_.push_back(t->startLoop());
	}
	if (numThreads_ == 0 && cb) {
		cb(baseLoop_);
	}
}

cm::net::EventLoop *cm::net::EventLoopThreadPool::getNextLoop() {
	EventLoop *loop = baseLoop_;
	if (!loops_.empty()) {
		loop = loops_[next_];
		if (++next_ >= loops_.size()) {
			next_ = 0;
		}
	}
	return loop;
}

std::vector<cm::net::EventLoop *> cm::net::EventLoopThreadPool::getAllLoops() {
	if (loops_.empty()) {
		return std::vector<EventLoop *>{1, baseLoop_};
	} else {
		return loops_;
	}
}
