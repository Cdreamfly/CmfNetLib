#include "base/Thread.hpp"
#include "base/CurrentThread.hpp"

#include <semaphore.h>

#include <memory>
#include <utility>

std::atomic_int cm::Thread::numCreated_(0);

cm::Thread::Thread(ThreadFunc func, std::string name)
		: started_(false), joined_(false), tid_(0), func_(std::move(func)), name_(std::move(name)) {
	setDefaultName();
}

cm::Thread::~Thread() {
	if (started_ && !joined_) {
		thread_->detach(); // thread类提供的设置分离线程的方法
	}
}

void cm::Thread::start()  // 一个Thread对象，记录的就是一个新线程的详细信息
{
	started_ = true;
	sem_t sem;
	sem_init(&sem, false, 0);

	// 开启线程
	thread_ = std::make_shared<std::thread>([&]() {
		// 获取线程的tid值
		tid_ = CurrentThread::tid();
		sem_post(&sem);
		// 开启一个新线程，专门执行该线程函数
		func_();
	});

	// 这里必须等待获取上面新创建的线程的tid值
	sem_wait(&sem);
}

void cm::Thread::join() {
	joined_ = true;
	thread_->join();
}

void cm::Thread::setDefaultName() {
	int num = ++numCreated_;
	if (name_.empty()) {
		char buf[32] = {0};
		snprintf(buf, sizeof buf, "Thread%d", num);
		name_ = buf;
	}
}