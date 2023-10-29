#pragma once

#include "base/NonCopyable.hpp"
#include "net/TcpConnection.hpp"
/**
 * 对外服务器编程使用的类
 */
namespace cm::net {
	class Acceptor;

	class EventLoop;

	class EventLoopThreadPool;

	class TcpServer : private NonCopyable {
	public:
		using ThreadInitCallback = std::function<void(EventLoop *)>;
		enum class Option {
			kNoReusePort,
			kReusePort,
		};
	public:
		explicit TcpServer(EventLoop *, const InetAddress &, std::string , const Option &);

		virtual ~TcpServer();

		void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }

		void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

		void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

		void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

		void setThreadNum(int);

		void start();

	private:
		using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
	private:
		EventLoop *loop_{};   //baseLoop 用户定义的loop
		const std::string ipPort_;
		const std::string name_;
		int nextConnId_{};
		std::unique_ptr<Acceptor> acceptor_;    //运行在mainLoop，任务就是监听连接
		std::shared_ptr<EventLoopThreadPool> threadPool_;   //one loop per thread
		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
		WriteCompleteCallback writeCompleteCallback_;
		ThreadInitCallback threadInitCallback_;
		std::atomic_int started_{};
		ConnectionMap connections_; //保存所有连接
	};
}