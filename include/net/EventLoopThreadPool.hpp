#pragma once

#include "base/NonCopyable.hpp"

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace cm::net {

	class EventLoop;

	class EventLoopThread;

	class EventLoopThreadPool : private NonCopyable {
	public:
		using ThreadInitCallback = std::function<void(EventLoop *)>;

		explicit EventLoopThreadPool(EventLoop *, std::string);

		virtual ~EventLoopThreadPool();

		void setThreadNum(const int num) { numThreads_ = num; }

		void start(const ThreadInitCallback &cb = ThreadInitCallback());

		EventLoop *getNextLoop();

		std::vector<EventLoop *> getAllLoops();

		[[nodiscard]] bool started() const { return started_; }

		[[nodiscard]] const std::string &name() const { return name_; }

	private:
		EventLoop *baseLoop_;
		std::string name_;
		bool started_;
		int numThreads_;
		int next_;
		std::vector<std::unique_ptr<EventLoopThread>> threads_;
		std::vector<EventLoop *> loops_;
	};
}