#include "ts.h"

#include <cstdarg>
#include <mutex>
#include <queue>

namespace ts {
	std::mutex printf_lock;
	void printf (const char* format, ...) {
		va_list vaptr;
		va_start (vaptr, format);
		std::lock_guard<std::mutex> lk (printf_lock);
		vprintf (format, vaptr);
	}
}
