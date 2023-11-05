#include "cm/base/CurrentThread.hpp"
#include <sys/syscall.h>
#include <unistd.h>

__thread int cm::CurrentThread::t_cachedTid = 0;

void cm::CurrentThread::cacheTid() {
	if (cm::CurrentThread::t_cachedTid == 0) {
		// 通过linux系统调用，获取当前线程的tid值
		cm::CurrentThread::t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
	}
}