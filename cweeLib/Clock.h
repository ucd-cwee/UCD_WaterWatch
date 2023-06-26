

#ifndef __CLOCK_H__
#define __CLOCK_H__

#pragma hdrstop
#include <stdint.h>

// Return the number of seconds since boot.
float clock_s();
INLINE float clock_s() { std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::seconds>(m_clock.now().time_since_epoch()).count(); }

// Return the number of milliseconds since boot.
uint64 clock_ms();
INLINE uint64 clock_ms() { std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::milliseconds>(m_clock.now().time_since_epoch()).count(); }

// Return the number of microseconds since boot.
uint64 clock_us();
INLINE uint64 clock_us() { std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::microseconds>(m_clock.now().time_since_epoch()).count(); }

// Return the number of nanoseconds since boot.
uint64 clock_ns();
INLINE uint64 clock_ns() { std::chrono::high_resolution_clock m_clock; return std::chrono::duration_cast<std::chrono::nanoseconds>(m_clock.now().time_since_epoch()).count(); }

#endif