#pragma once

#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>

namespace cm::CurrentThread {
	extern __thread int t_cachedTid;
	extern __thread char t_tidString[32];
	extern __thread int t_tidStringLength;

	void cacheTid() {
		if (t_cachedTid == 0) {
			t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
			t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
		}
	}

	inline int tid() {
		if (__builtin_expect(t_cachedTid == 0, 0)) {
			cacheTid();
		}
		return t_cachedTid;
	}
}