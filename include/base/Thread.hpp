#pragma once

#include "base/NonCopyable.hpp"
#include <thread>
#include <memory>
#include <functional>
#include <atomic>
#include <string>

namespace cm {
	class Thread : private NonCopyable {
	public:
		using ThreadFunc = std::function<void()>;

		explicit Thread(ThreadFunc, std::string name = std::string());

		virtual ~Thread();

		void start();

		void join();

		[[nodiscard]] bool started() const { return started_; }

		[[nodiscard]] pid_t tid() const { return tid_; }

		[[nodiscard]] const std::string &name() const { return name_; }

		static int numCreated() { return numCreated_; }

	private:
		void setDefaultName();

	private:
		bool started_;
		bool joined_;
		pid_t tid_;
		std::string name_;
		std::shared_ptr<std::thread> thread_;
		ThreadFunc func_;
		static std::atomic_int numCreated_;
	};
}