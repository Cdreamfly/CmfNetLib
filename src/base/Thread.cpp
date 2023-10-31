#include "base/Thread.hpp"
#include "base/CurrentThread.hpp"

#include <semaphore.h>

std::atomic_int cm::Thread::numCreated_ = 0;

cm::Thread::Thread(cm::Thread::ThreadFunc func, std::string name) : started_(false), joined_(false), tid_(0),
                                                                    name_(std::move(name)), func_(std::move(func)) {
	setDefaultName();
}

cm::Thread::~Thread() {
	if (started_ && !joined_) {
		thread_->detach();
	}
}

void cm::Thread::start() {
	started_ = true;
	sem_t sem;
	sem_init(&sem, false, 0);
	thread_ = std::make_shared<std::thread>([&] {
		tid_ = CurrentThread::tid();
		sem_post(&sem);
		func_();
	});
	//这里必须等待获取上面新创建的线程的tid值
	sem_wait(&sem);
}

void cm::Thread::join() {
	joined_ = true;
	thread_->join();
}

void cm::Thread::setDefaultName() {
	if (name_.empty()) {
		char buf[32] = {0};
		snprintf(buf, sizeof(buf), "Thread:%d", ++numCreated_);
		name_ = buf;
	}
}
