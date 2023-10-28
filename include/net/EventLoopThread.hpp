#pragma once

#include "base/NonCopyable.hpp"
#include "base/Thread.hpp"
#include <mutex>
#include <string>
#include <functional>
#include <condition_variable>

namespace cm::net {
	class EventLoop;

	class EventLoopThread : private NonCopyable {
	public:
		using ThreadInitCallback = std::function<void(EventLoop *)>;

		explicit EventLoopThread(ThreadInitCallback, const std::string &name = std::string());

		virtual ~EventLoopThread();

		EventLoop *startLoop();

	private:
		EventLoop *loop_;
		bool exiting_;
		std::mutex mutex_;
		Thread thread_;
		std::condition_variable cond_;
		ThreadInitCallback callback_;
	};
}