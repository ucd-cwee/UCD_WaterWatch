#pragma once
#include "Heap.h"

void* Mem_Alloc16(const size_t& size, const memTag_t& tag) { if (!size) return NULL; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16); };
void  Mem_Free16(void* ptr) { if (ptr) ::_aligned_free(ptr); };
void* Mem_ClearedAlloc(const size_t& size, const memTag_t& tag) { void* mem = Mem_Alloc16(size, tag); ::memset(mem, 0, size); return mem; };
void  Mem_Free(void* ptr) { Mem_Free16(ptr); }
#ifndef FORCE_INIT_LOCAL_MEM
void* Mem_Alloc(const size_t& size, const memTag_t& tag) { return Mem_Alloc16(size, tag); }
#else
void* Mem_Alloc(const size_t size, const memTag_t tag) { return Mem_ClearedAlloc(size, tag); }
#endif
char* Mem_CopyString(const char* in) {
	size_t L{ strlen(in) + 1 };
	char* out = (char*)Mem_Alloc(L, TAG_STRING);
	::strncpy(out, in, L - 1); // ::strcpy_s(out, L, in);
	return out;
};
