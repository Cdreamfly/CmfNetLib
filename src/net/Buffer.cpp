#include "net/Buffer.hpp"
#include "net/SocketOps.hpp"

ssize_t cm::net::Buffer::readFd(const int fd, int &savedErrno) {
	char extraBuf[65536] = {0};
	iovec vec[2];
	const std::size_t writable = writableBytes();

	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extraBuf;
	vec[1].iov_len = sizeof(extraBuf);
	//当前buffer writable比extraBuf还大就不用extraBuf这块缓冲了
	const ssize_t n = sockets::readv(fd, vec, (writable < sizeof(extraBuf)) ? 2 : 1);
	if (n < 0) {
		savedErrno = errno;
	} else if (n <= writable) {
		writerIndex_ += n;      //如果读取的数据相等或比可写入空间还小
	} else {                 //如果读取数据大于可写入空间
		writerIndex_ = buffer_.size();  //第一块缓冲区空间写满了
		//就把第二块缓冲区的数据追加到第一块缓冲区中，数据大小为：总读取数据大小n - 第一块缓冲区可写入大小
		append(extraBuf, n - writable);
	}
	return 0;
}

ssize_t cm::net::Buffer::writeFd(const int fd, int &savedErrno) const {
	ssize_t len = sockets::write(fd, peek(), readableBytes());
	if (len < 0) {
		savedErrno = errno;
	}
	return len;
}
