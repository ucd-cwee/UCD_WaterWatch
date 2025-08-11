#pragma once
#pragma hdrstop
#include <stdint.h>
#include <chrono>
#include <ShlDisp.h>
#include <winnt.h>
#include <string>

namespace GL {
	namespace clock {
		// seconds since boot
		__forceinline static long long s() {
			return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
		// milliseconds since boot
		__forceinline static long long ms() {
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
		// microseconds since boot
		__forceinline static long long us() {
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
		// nanoseconds since boot
		__forceinline static long long ns() {
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		};
	};
	class stopwatch {
	public:
		stopwatch() : t0(clock::ns()), t1(0) {};
		// resets the timer to start "now"
		long long reset() { 
			InterlockedExchange64(reinterpret_cast<volatile long long*>(&t0), clock::ns());
			return t0;
		};
		// stops the timer and returns the time passed since in seconds
		long double stop() { 
			InterlockedExchange64(reinterpret_cast<volatile long long*>(&t1), clock::ns());
			return static_cast<long double>(t1 - t0) / 1000000000.0; 
		};
		// does not stop the timer, but does return the time passed since in seconds
		long double check() const { 
			if (t1 < t0) InterlockedExchange64(reinterpret_cast<volatile long long*>(&const_cast<stopwatch*>(this)->t1), clock::ns()); // const_cast<stopwatch*>(this)->t1 = clock::ns();
			return static_cast<long double>(t1 - t0) / 1000000000.0; 
		};

		std::shared_ptr<void> debug_timer() {
			return std::static_pointer_cast<void>(std::shared_ptr<int>(new int(0), [startTime = this->reset(), this](int* p) -> void {
				auto stopTime = this->stop();
				std::string to_print = std::to_string(stopTime) + " s\n";
				std::cout << to_print;				
				delete p;
			}));
		};

		template<typename T>
		std::shared_ptr<void> debug_timer(T const& additional_message_content) {
			return std::static_pointer_cast<void>(std::shared_ptr<int>(new int(0), [startTime = this->reset(), this, additional_message = std::to_string(additional_message_content)](int* p) -> void {
				auto stopTime = this->stop();
				if (additional_message.empty()) {
					std::string to_print = std::to_string(stopTime) + " s\n";
					std::cout << to_print;
				}
				else {
					std::string to_print = additional_message + ": " + std::to_string(stopTime) + " s\n";
					std::cout << to_print;
				}
				delete p;
			}));
		};

	private:
		long long t0;
		long long t1;
	};
};