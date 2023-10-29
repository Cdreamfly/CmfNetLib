#pragma once

#include "base/NonCopyable.hpp"
#include "net/Callbacks.hpp"
#include "net/InetAddress.hpp"
#include "net/Buffer.hpp"

#include <atomic>

namespace cm::net {
	class Channel;

	class EventLoop;

	class Socket;

	class TcpConnection : private NonCopyable, public std::enable_shared_from_this<cm::net::TcpConnection> {
	public:
		explicit TcpConnection(EventLoop *loop, std::string name, int fd, const InetAddress &localAddr,
		              const InetAddress &peerAddr);

		virtual ~TcpConnection();

		void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

		void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

		void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

		void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

		void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, const size_t highWaterMark) {
			highWaterMarkCallback_ = cb;
			highWaterMark_ = highWaterMark;
		}

		const InetAddress &localAddress() const { return localAddr_; }

		const InetAddress &peerAddress() const { return peerAddr_; }

		EventLoop *getLoop() const { return loop_; }

		const std::string &name() const { return name_; }

		bool connect() const { return state_ == StateE::kConnected; }

		void send(const std::string &buf);

		void shutdown();

		void connectEstablished();

		void connectDestroyed();

	private:
		enum class StateE {
			kDisconnected, kConnecting, kConnected, kDisconnecting
		};

		void sendInLoop(const std::string &msg);

		void shutdownInLoop();
	private:
		EventLoop *loop_;
		const std::string name_;
		bool reading_;
		std::atomic<StateE> state_;
		std::unique_ptr<Socket> socket_;
		std::unique_ptr<Channel> channel_;
		const InetAddress localAddr_;
		const InetAddress peerAddr_;
		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
		WriteCompleteCallback writeCompleteCallback_;
		HighWaterMarkCallback highWaterMarkCallback_;
		CloseCallback closeCallback_;

		size_t highWaterMark_{};
		Buffer inputBuffer_;//接收数据的缓冲区
		Buffer outputBuffer_;//发送数据的缓冲区
	};
}