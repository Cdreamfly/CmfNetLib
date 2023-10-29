#pragma once

#include "base/Copyable.hpp"

#include <vector>
#include <string>
#include <string_view>

namespace cm::net {
	/**premeditate
	 * +-------------------+------------------+------------------+
	 * | prependable bytes |  readable bytes  |  writable bytes  |
	 * |                   |     (CONTENT)    |                  |
	 * +-------------------+------------------+------------------+
	 * |                   |                  |                  |
	 * 0      <=      readerIndex   <=   writerIndex    <=     size
	 */
	class Buffer : public Copyable {
	public:
		static const std::size_t kCheapPrepend = 8;
		static const std::size_t kInitialSize = 1024;

		explicit Buffer(const std::size_t initialSize = kInitialSize) : buffer_(kCheapPrepend + kInitialSize),
		                                                                readerIndex_(kCheapPrepend),
		                                                                writerIndex_(kCheapPrepend) {}

		//可读出字节数
		[[nodiscard]] std::size_t readableBytes() const { return writerIndex_ - readerIndex_; }

		//可写入字节数
		[[nodiscard]] std::size_t writableBytes() const { return buffer_.size() - writerIndex_; }

		//头部字节数
		[[nodiscard]] std::size_t prependableBytes() const { return readerIndex_; }

		//返回缓冲区可读数据起始地址
		[[nodiscard]] const char *peek() const { return begin() + readerIndex_; }

		char *beginWrite() { return begin() + writerIndex_; }

		[[nodiscard]] const char *beginWrite() const { return begin() + writerIndex_; }

		void retrieve(const std::size_t len) {
			if (len < readableBytes()) {
				readerIndex_ += len;
			} else {
				retrieveAll();
			}
		}

		void retrieveAll() { readerIndex_ = writerIndex_ = kCheapPrepend; }

		std::string retrieveAsString(const std::size_t len) {
			std::string result(peek(), len);
			retrieve(len);
			return result;
		}

		std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }

		void ensureWritableBytes(const std::size_t len) {
			if (writableBytes() < len) {
				makeSpace(len);
			}
		}

		[[nodiscard]] std::string_view toStringPiece() const { return {peek(), readableBytes()}; }

		void append(const std::string_view &str) {
			append(str.data(), str.size());
		}

		void append(const char *data, const std::size_t len) {
			ensureWritableBytes(len);
			std::copy(data, data + len, beginWrite());
			writerIndex_ += len;
		}

		void append(const void *data, const std::size_t len) {
			append(static_cast<const char *>(data), len);
		}

		ssize_t readFd(int fd, int &savedErrno);

		ssize_t writeFd(int fd, int &savedErrno) const;

	private:
		char *begin() { return &*buffer_.begin(); }

		[[nodiscard]] const char *begin() const { return &*buffer_.begin(); }

		void makeSpace(const std::size_t len) {
			//如果readerIndex之前的空间＋writerIndex之后的空间都不够存储数据就扩充
			if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
				buffer_.resize(writerIndex_ + len);
			} else {    //否则buffer够用，就把当前未读出和已写入的数据向前挪挪
				const size_t readable = readableBytes();
				std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
				readerIndex_ = kCheapPrepend;
				writerIndex_ = readerIndex_ + readable;
			}
		}

	private:
		std::vector<char> buffer_;
		std::size_t readerIndex_;
		std::size_t writerIndex_;
	};
}