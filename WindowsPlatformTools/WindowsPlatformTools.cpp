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
#include "WindowsPlatformTools.h"

#include <minwinbase.h>
#include <cstdint>
#include <synchapi.h>
#include <processthreadsapi.h>
#include <handleapi.h>
#include <libloaderapi.h>
#include <errhandlingapi.h>
#include <winerror.h>
// #include <corecrt_malloc.h>
#include <Windows.h>
#include <winnt.h>

typedef CRITICAL_SECTION		mutexHandle_t;
typedef HANDLE					signalHandle_t;
typedef LONG					interlockedInt_t;

// typedef unsigned int (*xthread_t)(void*);

enum e_LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL {
	e_localRelationProcessorCore,
	e_localRelationNumaNode,
	e_localRelationCache,
	e_localRelationProcessorPackage
};

__forceinline DWORD CountSetBits(ULONG_PTR bitMask) {
	DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

	for (DWORD i = 0; i <= LSHIFT; i++) {
		bitSetCount += ((bitMask & bitTest) ? 1 : 0);
		bitTest /= 2;
	}

	return bitSetCount;
};
WindowsPlatform::cweeCpuInfo_t WindowsPlatform::GetCPUInfo() {
	typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr; // NULL
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = nullptr; // NULL
	PCACHE_DESCRIPTOR Cache;
	LPFN_GLPI	glpi;
	BOOL		done = FALSE;
	DWORD		returnLength = 0;
	DWORD		byteOffset = 0;

	WindowsPlatform::cweeCpuInfo_t cpuInfo;
	cpuInfo = WindowsPlatform::cweeCpuInfo_t();

	// memset(&cpuInfo, 0, sizeof(WindowsPlatform::cweeCpuInfo_t));

	glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetLogicalProcessorInformation");
	if (NULL == glpi) {
		return cpuInfo;
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
				return cpuInfo;
			}
		}
		else {
			done = TRUE;
		}
	}

	ptr = buffer;

	while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
		switch ((e_LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL)ptr->Relationship) {
		case e_localRelationProcessorCore: // A hyperthreaded core supplies more than one logical processor.
			cpuInfo.processorCoreCount++;
			cpuInfo.logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
			break;

		case e_localRelationNumaNode: // Non-NUMA systems report a single record of this type.
			cpuInfo.numaNodeCount++;
			break;

		case e_localRelationCache: // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
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

		case e_localRelationProcessorPackage: // Logical processors share a physical package.
			cpuInfo.processorPackageCount++;
			break;

		default:
			break;
		}
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		ptr++;
	}

	free(buffer);

	if (cpuInfo.logicalProcessorCount > 32) cpuInfo.logicalProcessorCount = 32;
	if (cpuInfo.logicalProcessorCount <= 0) cpuInfo.logicalProcessorCount = 1;

	return cpuInfo;
};

INLINE static float CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks)
{
	static unsigned long long _previousTotalTicks = 0;
	static unsigned long long _previousIdleTicks = 0;

	unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
	unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

	float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

	_previousTotalTicks = totalTicks;
	_previousIdleTicks = idleTicks;
	return ret;
}
INLINE static unsigned long long FileTimeToInt64(const FILETIME& ft)
{
	return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}
float WindowsPlatform::GetCPULoad() {
	FILETIME idleTime, kernelTime, userTime;
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? 100.0f * CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
};