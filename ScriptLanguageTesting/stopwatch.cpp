#include "stopwatch.h"
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

namespace GL {
	namespace clock {
		// seconds since boot
		long long s() {
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqS);
			// return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
		// milliseconds since boot
		long long ms() {
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqMS);
			// return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
		// microseconds since boot
		long long us() {
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqUS);
			// return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
		// nanoseconds since boot
		long long ns() {
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return (long long)(double(li.QuadPart - CounterStart) * invPCFreqNS);
			// return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
	};
}