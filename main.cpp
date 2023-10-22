#include <iostream>
#include "base/Log.hpp"
#include "base/Timestamp.hpp"
#include "net/InetAddress.hpp"
#include "net/Socket.hpp"
#include "net/Channel.hpp"
#include <chrono>
#include <netinet/in.h>
#include <cstring>
#include <netinet/tcp.h>

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
	if (std::is_same<typename std::decay<T>::type, sockaddr_in>::value) {
		std::cout << "true" << std::endl;

	} else {
		std::cout << "false" << std::endl;
	}
}

bool getTcpInfo(tcp_info &tcpi) {
	socklen_t len = sizeof(tcpi);
	memset(&tcpi, 0, len);
	return ::getsockopt(1, SOL_TCP, TCP_INFO, &tcpi, &len) == 0;
}

bool getTcpInfoString(char *buf, int len) {
	struct tcp_info tcpi{};
	bool ok = getTcpInfo(tcpi);
	if (ok) {
		snprintf(buf, len, "unrecovered=%u "
		                   "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
		                   "lost=%u retrans=%u rtt=%u rttvar=%u "
		                   "sshthresh=%u cwnd=%u total_retrans=%u",
		         tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
		         tcpi.tcpi_rto,          // Retransmit timeout in usec
		         tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
		         tcpi.tcpi_snd_mss,
		         tcpi.tcpi_rcv_mss,
		         tcpi.tcpi_lost,         // Lost packets
		         tcpi.tcpi_retrans,      // Retransmitted packets out
		         tcpi.tcpi_rtt,          // Smoothed round trip time in usec
		         tcpi.tcpi_rttvar,       // Medium deviation
		         tcpi.tcpi_snd_ssthresh,
		         tcpi.tcpi_snd_cwnd,
		         tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
	}
	return ok;
}

int main() {
	cm::net::InetAddress address(9190, true, true);
	std::cout << address.toIpPort() << std::endl;
	return 0;
}