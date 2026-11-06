#include "stopwatch.h"

#ifdef _WIN32
#include <windows.h>
static long double PCFreq = 0.0;
static long double invPCFreqNS = 0.0;
static long double invPCFreqUS = 0.0;
static long double invPCFreqMS = 0.0;
static long double invPCFreqS = 0.0;
static __int64 CounterStart = 0;
auto StartCounter = [](){
	LARGE_INTEGER li; std::memset(&li, 0, sizeof(li));
	while (!QueryPerformanceFrequency(&li)) {}
	PCFreq = long double(li.QuadPart) / 1'000'000'000.0l;
	invPCFreqNS = 1.0 / PCFreq;
	invPCFreqUS = invPCFreqNS / 1'000.0l;
	invPCFreqMS = invPCFreqNS / 1'000'000.0l;
	invPCFreqS =  invPCFreqNS / 1'000'000'000.0l;
	//QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
	return true;
}();
#endif

namespace GL {
	namespace clock {
		// seconds since boot
		long long s() {
#ifdef _WIN32
			LARGE_INTEGER li; std::memset(&li, 0, sizeof(li));
			while (!QueryPerformanceCounter(&li)) {}
			return (long long)(long double(li.QuadPart - CounterStart) * invPCFreqS);
#else
			return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
		// milliseconds since boot
		long long ms() {
#ifdef _WIN32
			LARGE_INTEGER li; std::memset(&li, 0, sizeof(li));
			while (!QueryPerformanceCounter(&li)) {}
			return (long long)(long double(li.QuadPart - CounterStart) * invPCFreqMS);
#else
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
		// microseconds since boot
		long long us() {
#ifdef _WIN32
			LARGE_INTEGER li; std::memset(&li, 0, sizeof(li));
			while (!QueryPerformanceCounter(&li)) {}
			return (long long)(long double(li.QuadPart - CounterStart) * invPCFreqUS);
#else
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
		// nanoseconds since boot
		long long ns() {
#ifdef _WIN32
			LARGE_INTEGER li; std::memset(&li, 0, sizeof(li));
			while (!QueryPerformanceCounter(&li)) {}
			auto len = (long long)(long double(li.QuadPart - CounterStart) * invPCFreqNS);
/*			if (((((len / 1000000000.0l) / 3600.0l) / 24.0l) / 365.0l) > 70) {
				std::cout << "BAD DATA!\n";
			}*/			
			return len;
#else
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#endif
		};
	};
}