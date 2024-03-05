/**
 * FiberTaskingLib - A tasking library that uses fibers for efficient task switching
 *
 * This library was created as a proof of concept of the ideas presented by
 * Christian Gyrling in his 2015 GDC Talk 'Parallelizing the Naughty Dog Engine Using Fibers'
 *
 * http://gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
 *
 * FiberTaskingLib is the legal property of Adrian Astley
 * Copyright Adrian Astley 2015 - 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Fiber.h"
#include "boost_context/fcontext.h"
#include "Alloc.h"

#pragma region assert
#include <cstdio>
#include <iostream>

#define FTL_EXPAND(s) s
#define FTL_STR_IMPL(s) #s
#define FTL_STR(s) FTL_EXPAND(FTL_STR_IMPL(s))
#ifndef NDEBUG
#	define FTL_ASSERT(msg, expr)                                                                     \
		if (!(expr)) {                                                                                \
			std::fputs("FTL Assertion Failure: " FTL_STR(msg) ". Expr: " FTL_STR(expr) "\n", stderr); \
			std::fputs("Source File: " __FILE__ ":" FTL_STR(__LINE__) "\n", stderr);                  \
			std::abort();                                                                             \
		}
#else
#	define FTL_ASSERT(msg, expr)
#endif
#pragma endregion

#include "Config.h"

#include <algorithm>

#if 1
namespace fibers {

	namespace {
		__forceinline size_t RoundUp(size_t numToRound, size_t multiple) {
			if (multiple == 0) {
				return numToRound;
			}

			size_t const remainder = numToRound % multiple;
			if (remainder == 0) {
				return numToRound;
			}

			return numToRound + multiple - remainder;
		};
	};

	Fiber::Fiber(size_t stackSize, FiberStartRoutine startRoutine, void* arg) : m_arg(arg) {
#if defined(FTL_FIBER_STACK_GUARD_PAGES)
		m_systemPageSize = SystemPageSize();
		const size_t alignment = SystemPageSize();
#else
		m_systemPageSize = 0;
		// All systems require stacks that are at least 16 byte aligned
		const size_t alignment = 16;
#endif

		m_stackSize = RoundUp(stackSize, m_systemPageSize);
		// We add a guard page both the top and the bottom of the stack
		m_stack = AlignedAlloc(m_systemPageSize + m_stackSize + m_systemPageSize, alignment);
		m_context = boost_context::make_fcontext(static_cast<char*>(m_stack) + m_systemPageSize + stackSize, stackSize, startRoutine);

		FTL_VALGRIND_REGISTER(static_cast<char*>(m_stack) + m_systemPageSize, static_cast<char*>(m_stack) + m_systemPageSize + stackSize);
#if defined(FTL_FIBER_STACK_GUARD_PAGES)
		MemoryGuard(static_cast<char*>(m_stack), m_systemPageSize);
		MemoryGuard(static_cast<char*>(m_stack) + m_systemPageSize + stackSize, m_systemPageSize);
#endif
	}

	Fiber::~Fiber() {
		if (m_stack != nullptr) {
			if (m_systemPageSize != 0) {
				MemoryGuardRelease(static_cast<char*>(m_stack), m_systemPageSize);
				MemoryGuardRelease(static_cast<char*>(m_stack) + m_systemPageSize + m_stackSize, m_systemPageSize);
			}
			FTL_VALGRIND_DEREGISTER();

			AlignedFree(m_stack);
		}
	}

	void Fiber::SwitchToFiber(Fiber* const fiber) {
		boost_context::jump_fcontext(&m_context, fiber->m_context, fiber->m_arg);
	};
	void Fiber::Reset(FiberStartRoutine const startRoutine, void* const arg) {
		m_context = boost_context::make_fcontext(static_cast<char*>(m_stack) + m_stackSize, m_stackSize, startRoutine);
		m_arg = arg;
	};

	void Fiber::Swap(Fiber& first, Fiber& second) noexcept {
		std::swap(first.m_stack, second.m_stack);
		std::swap(first.m_systemPageSize, second.m_systemPageSize);
		std::swap(first.m_stackSize, second.m_stackSize);
		std::swap(first.m_context, second.m_context);
		std::swap(first.m_arg, second.m_arg);
	};
} // End of namespace ftl
#endif