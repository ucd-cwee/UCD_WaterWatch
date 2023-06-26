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
	static std::shared_ptr< cweeAlloc< Type_t, BlockSize > > allocator;

};
template<typename Type_t, size_t BlockSize> std::shared_ptr< cweeAlloc< Type_t, BlockSize > > cweeAllocator<Type_t, BlockSize>::allocator = std::make_shared< cweeAlloc< Type_t, BlockSize >>();