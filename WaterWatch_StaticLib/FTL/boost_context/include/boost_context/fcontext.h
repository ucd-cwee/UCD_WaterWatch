//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Modified for use in FiberTaskingLib
//
// FiberTaskingLib is the legal property of Adrian Astley
// Copyright Adrian Astley 2015 - 2018

#pragma once

#include <stddef.h> // size_t

extern "C" void jump_fcontext(void** from, void* to, void* arg);
extern "C" void* make_fcontext(void* sp, size_t size, void (*func)(void*));

namespace boost_context {

	using fcontext_t = void *;
	//void jump_fcontext(fcontext_t* from, fcontext_t to, void* arg) {
	//	::jump_fcontext(from, to, arg);
	//};
	//fcontext_t make_fcontext(void* sp, size_t size, void (*func)(void*)) {
	//	::make_fcontext(sp, size, func);
	//};
	// sp is the pointer to the _top_ of the stack (ie &stack_buffer[size]).

} // End of namespace boost_context
