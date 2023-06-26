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
#include "Precompiled.h"

template<typename Type_t, size_t BlockSize = 128> class cweeAllocator {
public:
	cweeAllocator() = delete;

	using Type_p = Type_t*;
	static Type_p Alloc() {
		Type_p out;
		out = allocator->Alloc();
		return out;
	};
	template <typename T, typename... Fargs>
	static Type_p Alloc(T arg1, Fargs... Args) {
		Type_p out;
		out = allocator->Alloc(arg1, Args...);
		return out;
	};
	static void Free(Type_p p) {
		allocator->Free(p);
	};
	static void Clean() {
		allocator->Clean();
	};
	static size_t GetTotalCount() {
		size_t out;
		out = allocator->GetTotalCount();
		return out;
	};
	static size_t GetAllocCount() {
		size_t out;
		out = allocator->GetAllocCount();
		return out;
	};
	static void Reserve(size_t n) {
		allocator->Reserve(n);
	};

private:
	static std::shared_ptr< cweeAlloc< Type_t, BlockSize > > allocator; // should be extern? 

};
template<typename Type_t, size_t BlockSize> std::shared_ptr< cweeAlloc< Type_t, BlockSize > > cweeAllocator<Type_t, BlockSize>::allocator = std::make_shared< cweeAlloc< Type_t, BlockSize >>();