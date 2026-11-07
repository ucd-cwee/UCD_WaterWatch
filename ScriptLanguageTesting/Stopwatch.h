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
		long long s();
		// milliseconds since boot
		long long ms();
		// microseconds since boot
		long long us();
		// nanoseconds since boot
		long long ns();
	};
	class stopwatch {
	public:
		stopwatch() : t0{ clock::ns() }, t1{ 0 } {};
		stopwatch(stopwatch const& rhs) : t0{ rhs.t0 }, t1{ rhs.t1 } {};
		stopwatch(stopwatch && rhs) noexcept : t0{ rhs.t0 }, t1{ rhs.t1 } {};
		stopwatch& operator=(stopwatch const& rhs) {
			t0 = rhs.t0;
			t1 = rhs.t1;
			return *this;
		};
		stopwatch& operator=(stopwatch && rhs) noexcept {
			t0 = rhs.t0;
			t1 = rhs.t1;
			return *this;
		};
		~stopwatch() {
			t0 = 0;
			t1 = 0;
		};

		// resets the timer to start "now"
		__forceinline long long reset() { 
			GL::interlocked::exchange(t0, clock::ns());
			return t0;
		};
		// stops the timer and returns the time passed since in seconds
		long double stop() { 
			GL::interlocked::exchange(t1, clock::ns());
			if (t0 >= t1) 
				return 0.0l;
			else 
				return static_cast<long double>(t1 - t0) / 1000000000.0; 
		};
		// does not stop the timer, but does return the time passed since in seconds
		long double check() const { 
			// if (t1 < t0) InterlockedExchange64(reinterpret_cast<volatile long long*>(&const_cast<stopwatch*>(this)->t1), clock::ns()); // const_cast<stopwatch*>(this)->t1 = clock::ns();
			long long c = clock::ns();
			if (t0 >= c) return 0.0l;
			else return static_cast<long double>(c - t0) / 1000000000.0;
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
					startTime -= clock::ns();
					startTime *= -1ll;
					if (startTime >= 3000) {
						auto stopTime = static_cast<long double>(startTime) / 1'000'000'000.0;
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
					startTime -= clock::ns();
					startTime *= -1ll;
					if (startTime >= 3000) {
						auto stopTime = static_cast<long double>(startTime) / 1'000'000'000.0;
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
					startTime -= clock::ns();
					startTime *= -1ll;
					if (startTime >= 3000) {
						auto stopTime = static_cast<long double>(startTime) / 1'000'000'000.0;
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

	// private:
		long long t0;
		long long t1;
	};

};