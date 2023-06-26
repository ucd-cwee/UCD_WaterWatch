
//#pragma hdrstop
#include "../cweelib/precompiled.h"
#pragma hdrstop

/*
========================
Sys_Createthread
========================
*/
uintptr_t Sys_CreateThread(xthread_t function, void* parms, xthreadPriority priority, const char* name, core_t core, int stackSize, bool suspended) {
	DWORD flags = (suspended ? CREATE_SUSPENDED : 0);
	flags |= STACK_SIZE_PARAM_IS_A_RESERVATION;
	DWORD threadId;
	HANDLE handle = CreateThread(NULL, stackSize, (LPTHREAD_START_ROUTINE)function, parms, flags, &threadId);
	return (uintptr_t)handle;
}
void Sys_WaitForThread(uintptr_t threadHandle) {
	WaitForSingleObject((HANDLE)threadHandle, INFINITE);
}
void Sys_ForgetThread(uintptr_t threadHandle) {
	if (threadHandle == 0) {
		return;
	}
	CloseHandle((HANDLE)threadHandle);
}
void Sys_DestroyThread(uintptr_t threadHandle) {
	if (threadHandle == 0) {
		return;
	}
	WaitForSingleObject((HANDLE)threadHandle, INFINITE);
	CloseHandle((HANDLE)threadHandle);
}
void Sys_Yield() {
	SwitchToThread();
}
bool Sys_TryYield() {
	return SwitchToThread();
}
uintptr_t Sys_GetCurrentThreadID() {
	return GetCurrentThreadId();
}

/*
================================================================================================
	Interlocked Integer
================================================================================================
*/
interlockedInt_t Sys_InterlockedIncrement(interlockedInt_t& value) {
	return InterlockedIncrementAcquire(&value);
}
interlockedInt_t Sys_InterlockedDecrement(interlockedInt_t& value) {
	return InterlockedDecrementRelease(&value);
}
interlockedInt_t Sys_InterlockedAdd(interlockedInt_t& value, interlockedInt_t i) {
	return InterlockedExchangeAdd(&value, i) + i;
}
interlockedInt_t Sys_InterlockedSub(interlockedInt_t& value, interlockedInt_t i) {
	return InterlockedExchangeAdd(&value, -i) - i;
}
interlockedInt_t Sys_InterlockedExchange(interlockedInt_t& value, interlockedInt_t exchange) {
	return InterlockedExchange(&value, exchange);
}
interlockedInt_t Sys_InterlockedCompareExchange(interlockedInt_t& value, interlockedInt_t comparand, interlockedInt_t exchange) {
	return InterlockedCompareExchange(&value, exchange, comparand);
}

template <typename type>
type Sys_InterlockedIncrementV(type& value) {
	return InterlockedIncrementAcquire(&value);
}
template <typename type>
type Sys_InterlockedDecrementV(type& value) {
	return InterlockedDecrementRelease(&value);
}
template <typename type>
type Sys_InterlockedAddV(type& value, const type& i) {
	return InterlockedExchangeAdd(&value, i) + i;
}
template <typename type>
type Sys_InterlockedSubV(type& value, const type& i) {
	return InterlockedExchangeAdd(&value, -i) - i;
}
template <typename type>
type Sys_InterlockedExchangeV(type& value, const type& exchange) {
	return InterlockedExchange(&value, exchange);
}
template <typename type>
type Sys_InterlockedCompareExchangeV(type& value, const type& comparand, const type& exchange) {
	return InterlockedCompareExchange(&value, exchange, comparand);
}

/*
================================================================================================
	Interlocked Pointer
================================================================================================
*/

void* Sys_InterlockedExchangePointer(void*& ptr, void* exchange) {
	return InterlockedExchangePointer(&ptr, exchange);
}
void* Sys_InterlockedCompareExchangePointer(void*& ptr, void* comparand, void* exchange) {
	return InterlockedCompareExchangePointer(&ptr, exchange, comparand);
}


struct cpuInfo_t {
	int processorPackageCount;
	int processorCoreCount;
	int logicalProcessorCount;
	int numaNodeCount;
	struct cacheInfo_t {
		int count;
		int associativity;
		int lineSize;
		int size;
	} cacheLevel[3];
};

DWORD CountSetBits(ULONG_PTR bitMask) {
	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

	for (DWORD i = 0; i <= LSHIFT; i++) {
		bitSetCount += ((bitMask & bitTest) ? 1 : 0);
		bitTest /= 2;
	}

	return bitSetCount;
}

typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

enum LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL {
	localRelationProcessorCore,
	localRelationNumaNode,
	localRelationCache,
	localRelationProcessorPackage
};

/*
========================
GetCPUInfo
========================
*/
bool GetCPUInfo(cpuInfo_t& cpuInfo) {
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	PCACHE_DESCRIPTOR Cache;
	LPFN_GLPI	glpi;
	BOOL		done = FALSE;
	DWORD		returnLength = 0;
	DWORD		byteOffset = 0;

	memset(&cpuInfo, 0, sizeof(cpuInfo));

	glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
	if (NULL == glpi) {
		return 0;
	}

	while (!done) {
		DWORD rc = glpi(buffer, &returnLength);

		if (FALSE == rc) {
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				if (buffer) {
					free(buffer);
				}

				buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
			}
			else {
				return false;
			}
		}
		else {
			done = TRUE;
		}
	}

	ptr = buffer;

	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
		switch ((LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL)ptr->Relationship) {
		case localRelationProcessorCore:
			cpuInfo.processorCoreCount++;

			// A hyperthreaded core supplies more than one logical processor.
			cpuInfo.logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
			break;

		case localRelationNumaNode:
			// Non-NUMA systems report a single record of this type.
			cpuInfo.numaNodeCount++;
			break;

		case localRelationCache:
			// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
			Cache = &ptr->Cache;
			if (Cache->Level >= 1 && Cache->Level <= 3) {
				int level = Cache->Level - 1;
				if (cpuInfo.cacheLevel[level].count > 0) {
					cpuInfo.cacheLevel[level].count++;
				}
				else {
					cpuInfo.cacheLevel[level].associativity = Cache->Associativity;
					cpuInfo.cacheLevel[level].lineSize = Cache->LineSize;
					cpuInfo.cacheLevel[level].size = Cache->Size;
				}
			}
			break;

		case localRelationProcessorPackage:
			// Logical processors share a physical package.
			cpuInfo.processorPackageCount++;
			break;

		default:
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}

	free(buffer);

	return true;
}

/*
========================
Sys_CPUCount

numLogicalCPUCores	- the number of logical CPU per core
numPhysicalCPUCores	- the total number of cores per package
numCPUPackages		- the total number of packages (physical processors)
========================
*/
void Sys_CPUCount(int& numLogicalCPUCores, int& numPhysicalCPUCores, int& numCPUPackages) {
	cpuInfo_t cpuInfo;
	GetCPUInfo(cpuInfo);

	numPhysicalCPUCores = cpuInfo.processorCoreCount;
	numLogicalCPUCores = cpuInfo.logicalProcessorCount;
	numCPUPackages = cpuInfo.processorPackageCount;
}

