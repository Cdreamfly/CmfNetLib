#include "cm/net/Buffer.hpp"
#include "cm/net/SocketOps.hpp"

ssize_t cm::net::Buffer::readFd(const int fd, int &saveErrno) {
	char extraBuf[65536] = {0}; // 栈上的内存空间  64K
	iovec vec[2];
	const size_t writable = writableBytes(); // 这是Buffer底层缓冲区剩余的可写空间大小
	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extraBuf;
	vec[1].iov_len = sizeof extraBuf;
	//当前buffer writable比extraBuf还大就不用extraBuf这块缓冲了
	const ssize_t n = sockets::readv(fd, vec, (writable < sizeof extraBuf) ? 2 : 1);
	if (n < 0) {
		saveErrno = errno;
	} else if (n <= writable) // Buffer的可写缓冲区已经够存储读出来的数据了
	{
		writerIndex_ += n;   //如果读取的数据相等或比可写入空间还小
	} else // extraBuf里面也写入了数据  //如果读取数据大于可写入空间
	{
		writerIndex_ = buffer_.size();  //第一块缓冲区空间写满了
		//就把第二块缓冲区的数据追加到第一块缓冲区中，数据大小为：总读取数据大小n - 第一块缓冲区可写入大小
		append(extraBuf, n - writable);  // writerIndex_开始写 n - writable大小的数据
	}
	return n;
}

ssize_t cm::net::Buffer::writeFd(const int fd, int &saveErrno) const {
	ssize_t n = sockets::write(fd, peek(), readableBytes());
	if (n < 0) {
		saveErrno = errno;
	}
	return n;
}
