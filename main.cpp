#include <iostream>
#include "base/Log.hpp"
#include "base/Timestamp.hpp"
#include "net/InetAddress.hpp"
#include <chrono>
#include <netinet/in.h>
#include <cstring>

class Timer {
public:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration{};

	Timer() {
		start = std::chrono::steady_clock::now();
	}

	~Timer() {
		end = std::chrono::steady_clock::now();
		duration = end - start;
		float ms = duration.count() * 1000.0f;
		std::cout << "Timer took " << ms << " ms" << std::endl;
	}
};
template<typename T>
void setSockAddrInet(const T &addr) {
	if(std::is_same<typename std::decay<T>::type,sockaddr_in>::value){
		std::cout<<"true"<<std::endl;

	}else {
		std::cout<<"false"<<std::endl;
	}
}

int main() {
	cm::net::InetAddress address( 9190, true, true);
	std::cout<<address.toIpPort()<<std::endl;
	return 0;
}