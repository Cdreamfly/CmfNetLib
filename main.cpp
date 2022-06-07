#include <iostream>
#include "NetLib/log/Log.hpp"
#include "NetLib/thread/ThreadPool.hpp"
#include "NetLib/net/Buffer.hpp"
#include "NetLib/net/InetAddress.hpp"
#include "NetLib/net/Socket.hpp"
#include "NetLib/net/EventLoop.h"
#include "NetLib/net/Channel.h"
#include "NetLib/net/EPollPoller.h"

int main() {

    Buffer buffer;
    buffer.Append("hello");
    std::cout << buffer.WritableBytes() << " ";
    std::cout << buffer.ReadableBytes() << std::endl;

    InetAddress address("127.0.0.1", 9190);
    std::cout << address.GetIP() << " " << address.GetPort() << std::endl;

    Socket sock(100);
    std::cout << sock.GetFd() << std::endl;
    EventLoop*loop;
    Channel channel(loop, 1);
    LOG_DEBUG("HELLO");
    LOG_INFO("HELLO");

    return 0;
}