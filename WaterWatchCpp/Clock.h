/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#pragma hdrstop
#include <stdint.h>
#include "Precompiled.h"

class Clock {
public:
	static float Seconds() {
		std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::seconds>(m_clock.now().time_since_epoch()).count();
	};
	static long long Milliseconds() {
		std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now().time_since_epoch()).count();
	};
	static long long Microseconds() {
		std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::microseconds>(m_clock.now().time_since_epoch()).count();
	};
	static long long Nanoseconds() {
		std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::nanoseconds>(m_clock.now().time_since_epoch()).count();
	};
};

INLINE static float clock_s() { return Clock::Seconds(); };
INLINE static long long clock_ms() { return Clock::Milliseconds(); };
INLINE static long long clock_us() { return Clock::Microseconds(); };
INLINE static long long clock_ns() { return Clock::Nanoseconds(); };
INLINE static long long Sys_Milliseconds() { return clock_ms(); };
INLINE static long long Sys_Microseconds(void) { return clock_us(); };

class Stopwatch {
public:
	Stopwatch() : t0(clock_ns()), t1(t0) {};
	void Start() { t0 = clock_ns(); };
	u64 Stop() { t1 = clock_ns(); return t1 - t0; };
	u64 Seconds_Passed() const { return (t1-t0) / 1000000000.0; };
	u64 Nanoseconds_Passed() const { return (t1 - t0); };
private:
	long long t0;
	long long t1;
};