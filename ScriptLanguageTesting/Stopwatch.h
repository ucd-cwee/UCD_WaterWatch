#pragma once
#pragma hdrstop
#include <stdint.h>
#include <chrono>
#include <ShlDisp.h>
#include <winnt.h>
#include <string>
#include <memory>
#include <iostream>
#include "strings.h"

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
			// if (t1 < t0) InterlockedExchange64(reinterpret_cast<volatile long long*>(&const_cast<stopwatch*>(this)->t1), clock::ns()); // const_cast<stopwatch*>(this)->t1 = clock::ns();
			return static_cast<long double>(clock::ns() - t0) / 1000000000.0;
		};

		static auto debug_timer() {
			class wrapper {
			private:
				long long startTime;

			public:
				wrapper() : startTime{ clock::ns() } {};
				wrapper(wrapper const&) = delete;
				wrapper(wrapper&&) = delete;
				wrapper& operator=(wrapper const&) = delete;
				wrapper& operator=(wrapper&&) = delete;
				~wrapper() {
					long long NS = clock::ns() - startTime;
					if (NS >= 3000) {
						auto stopTime = static_cast<long double>(NS) / 1000000000.0;
						std::string to_print = std::to_string(stopTime) + " s\n";
						std::cout << to_print;
					}
				};
				constexpr operator bool() const { return true; };
			};
			return wrapper();
		};

		template <size_t N>
		__forceinline static auto debug_timer(const char(&additional_message_content)[N]) {
			class wrapper {
			private:
				long long startTime;
				const char(&additional_message)[N];

			public:
				wrapper(const char(&additional_message_content)[N]) : startTime{ clock::ns() }, additional_message{ additional_message_content } {};
				wrapper(wrapper const&) = delete;
				wrapper(wrapper&&) = delete;
				wrapper& operator=(wrapper const&) = delete;
				wrapper& operator=(wrapper&&) = delete;
				~wrapper() {
					long long NS = clock::ns() - startTime;
					if (NS >= 3000) {
						auto stopTime = static_cast<long double>(NS) / 1000000000.0;
						if constexpr (N == 0) {
							std::string to_print = std::to_string(stopTime) + " s\n";
							std::cout << to_print;
						}
						else {
							std::string to_print = std::string(additional_message) + ": " + std::to_string(stopTime) + " s\n";
							std::cout << to_print;
						}
					}
				};
				constexpr operator bool() const { return true; };
			};
			return wrapper(additional_message_content);
		};

		template<typename T>
		__forceinline static auto debug_timer(T const& additional_message_content) {
			class wrapper {
			private:
				long long startTime;
				std::string additional_message;

			public:
				wrapper(T const& additional_message_content) : startTime{ clock::ns() }, additional_message{ std::to_string(additional_message_content) } {};
				wrapper(wrapper const&) = delete;
				wrapper(wrapper &&) = delete;
				wrapper& operator=(wrapper const&) = delete;
				wrapper& operator=(wrapper&&) = delete;
				~wrapper() {
					long long NS = clock::ns() - startTime;
					if (NS >= 3000) {
						auto stopTime = static_cast<long double>(NS) / 1000000000.0;
						if (additional_message.empty()) {
							std::string to_print = std::to_string(stopTime) + " s\n";
							std::cout << to_print;
						}
						else {
							std::string to_print = additional_message + ": " + std::to_string(stopTime) + " s\n";
							std::cout << to_print;
						}
					}
				};
				constexpr operator bool() const { return true; };
			};
			return wrapper(additional_message_content);
		};

	private:
		long long t0;
		long long t1;
	};

};