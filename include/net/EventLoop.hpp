#pragma once

#include <functional>
#include <atomic>
#include <memory>
#include <mutex>

#include "base/NonCopyable.hpp"
#include "base/Timestamp.hpp"
#include "base/CurrentThread.hpp"
/**
 * 时间循环类，主要包含两大模块，Channel Poller(Epoll抽象)
 */
namespace cm::net {
	class Channel;

	class Poller;

	class EventLoop : private NonCopyable {
	public:
		using Functor = std::function<void()>;

		EventLoop();

		virtual ~ EventLoop();

		void loop();

		void quit();

		[[nodiscard]] Timestamp pollReturnTime() const { return pollReturnTime_; }

		//在当前loop中执行回调
		void runInLoop(const Functor &cb);

		//把cb放入队列中，唤醒loop所在的线程，执行回调
		void queueInLoop(const Functor &cb);

		//唤醒loop所在的线程
		void wakeup();

		void updateChannel(Channel *channel);

		void removeChannel(Channel *channel);

		bool hasChannel(Channel *channel);

		//判断EventLoop对象是否在自己的线程里
		[[nodiscard]] bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }


	private:
		using ChannelList = std::vector<Channel *>;

		//执行回调
		void doPendingFunctors();

	private:
		std::atomic_bool looping_{};
		std::atomic_bool quit_{};
		std::atomic_bool callingPendingFunctors_{};   //标识当前loop是否有需要执行的回调操作
		const pid_t threadId_;
		Timestamp pollReturnTime_;
		std::unique_ptr<Poller> poller_;
		int wakeupFd_{};                              //用于eventfd，线程间通信
		std::unique_ptr<Channel> wakeupChannel_;
		ChannelList activeChannels_;
		Channel *currentActiveChannel_{};
		std::mutex mutex_;                          //保持下面容器线程安全
		std::vector<Functor> pendingFunctors_;      //存储loop需要执行的所有回调操作
	};
}