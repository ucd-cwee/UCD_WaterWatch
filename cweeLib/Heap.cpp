/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#include "../cweelib/precompiled.h"
#pragma hdrstop

//===============================================================
//	cweeHeap
//===============================================================

//#undef new

/*
Nearly all system memory allocations go through this method. 
*/
void* Mem_Alloc16(const size_t& size, const memTag_t& tag) {
	//if (!size) return NULL; const int paddedSize = (size + 15) & ~15; return _aligned_malloc(paddedSize, 16);

	if (!size) return NULL;
	const size_t paddedSize = (size + 15) & ~15;
#ifdef			FORCE_MEM_TRACKING
	auto out = _aligned_malloc(paddedSize, 16);	

	std::fstream file("memoryMap.txt",std::fstream::app); // ifstream is read only, ofstream is write only, fstream is read/write.
	file << tag << "," << paddedSize << "," << (uintptr_t)out << "," << clock_ns() << "," << "1" << std::endl;

	return (void*)out; 
#else
	return _aligned_malloc(paddedSize, 16);
#endif
}

void Mem_Free16(void* ptr) {
	//if (ptr == NULL) return; _aligned_free(ptr);
	
#ifdef			FORCE_MEM_TRACKING
	if (!ptr) return;
	std::fstream file("memoryMap.txt", std::fstream::app); // ifstream is read only, ofstream is write only, fstream is read/write.
	file << TAG_UNSET << "," << sizeof(ptr) << "," << (uintptr_t)ptr << "," << clock_ns() << "," << "0" << std::endl;	
	_aligned_free(ptr);
#else
	if (ptr) _aligned_free(ptr);
#endif
}

void* Mem_ClearedAlloc(const size_t& size, const memTag_t& tag) {
	void* mem = Mem_Alloc16(size, tag); //Mem_Alloc(size, tag);
	SIMDProcessor->Memset(mem, 0, size);
	return mem;
}

char* Mem_CopyString(const char* in) {
	char* out = (char*)Mem_Alloc((size_t)(strlen(in) + 1), TAG_STRING);
	strcpy(out, in);
	return out;
}

