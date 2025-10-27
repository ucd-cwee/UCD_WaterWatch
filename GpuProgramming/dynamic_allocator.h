#pragma once
#include <memory>
#include <set>
// #include <corecrt_malloc.h>

static void* Mem_Alloc16(const size_t& size) {
	if (!size) return nullptr; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16);
};
static void  Mem_Free16(void* ptr) {
	if (ptr) ::_aligned_free(ptr);
};
static void* Mem_ClearedAlloc(const size_t& size) {
	void* mem = Mem_Alloc16(size);
	::memset(mem, 0, size);
	return mem;
};
static void  Mem_Free(void* ptr) { Mem_Free16(ptr); };
static void* Mem_Alloc(const size_t& size) { return Mem_Alloc16(size); };

#define CONST_MAX( x, y ) ( (x) > (y) ? (x) : (y) )
#include "BTree.h"

