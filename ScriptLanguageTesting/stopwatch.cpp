#include "stopwatch.h"

#ifdef _WIN32
#include <windows.h>

double PCFreq = 0.0;
double invPCFreqNS = 0.0;
double invPCFreqUS = 0.0;
double invPCFreqMS = 0.0;
double invPCFreqS = 0.0;
__int64 CounterStart = 0;
thread_local LARGE_INTEGER li;
auto StartCounter = []() -> bool {	
	if (QueryPerformanceFrequency(&li)) {
		PCFreq = double(li.QuadPart) / double(1'000.0 * 1'000'000.0);
		invPCFreqNS = 1.0 / PCFreq;
		invPCFreqUS = invPCFreqNS / 1'000.0;
		invPCFreqMS = invPCFreqNS / 1'000'000.0;
		invPCFreqS =  invPCFreqNS / 1'000'000'000.0;
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
	}
	return false;
}();
#endif

namespace GL {
	namespace clock {
		// seconds since boot
		long long s() {
#ifdef _WIN32
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqS);
#else
			return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
		// milliseconds since boot
		long long ms() {
#ifdef _WIN32
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqMS);
#else
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
		// microseconds since boot
		long long us() {
#ifdef _WIN32
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqUS);
#else
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
		// nanoseconds since boot
		long long ns() {
#ifdef _WIN32
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqNS);
#else
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
	};
}