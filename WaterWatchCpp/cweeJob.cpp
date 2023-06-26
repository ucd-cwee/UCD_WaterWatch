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

#include "cweeJob.h"

#include "../WindowsPlatformTools/WindowsPlatformTools.h"
#include "cwee_math.h"

#if 1
class CPU_DATA {
public:
	CPU_DATA() : m_numPhysicalCpuCores(0), m_numLogicalCpuCores(0), m_numCpuPackages(0) { InitCpuData(); };

private:
	struct cweeCpuInfo_t {
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
	static DWORD CountSetBits(ULONG_PTR bitMask) {
		DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

		for (DWORD i = 0; i <= LSHIFT; i++) {
			bitSetCount += ((bitMask & bitTest) ? 1 : 0);
			bitTest /= 2;
		}

		return bitSetCount;
	};
	enum e_LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL {
		e_localRelationProcessorCore,
		e_localRelationNumaNode,
		e_localRelationCache,
		e_localRelationProcessorPackage
	};
	static bool GetCPUInfo(cweeCpuInfo_t& cpuInfo) {
#if 0

		typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr; // NULL
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = nullptr; // NULL
		PCACHE_DESCRIPTOR Cache;
		LPFN_GLPI	glpi;
		BOOL		done = FALSE;
		DWORD		returnLength = 0;
		DWORD		byteOffset = 0;

		::memset(&cpuInfo, 0, sizeof(cpuInfo));
		//#ifdef _UWPTARGET
				//return 0;
		//#else
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

		return true;
		//#endif
#else
		::memset(&cpuInfo, 0, sizeof(cpuInfo));

		WindowsPlatform::cweeCpuInfo_t cpuInfoT = WindowsPlatform::GetCPUInfo();
		for (int i = 0; i < 3; i++) {
			cpuInfo.cacheLevel[i].associativity = cpuInfoT.cacheLevel[i].associativity;
			cpuInfo.cacheLevel[i].count = cpuInfoT.cacheLevel[i].count;
			cpuInfo.cacheLevel[i].lineSize = cpuInfoT.cacheLevel[i].lineSize;
			cpuInfo.cacheLevel[i].size = cpuInfoT.cacheLevel[i].size;
		}
		cpuInfo.logicalProcessorCount = cpuInfoT.logicalProcessorCount;
		cpuInfo.numaNodeCount = cpuInfoT.numaNodeCount;
		cpuInfo.processorCoreCount = cpuInfoT.processorCoreCount;
		cpuInfo.processorPackageCount = cpuInfoT.processorPackageCount;

		return true;
#endif
	};
	void InitCpuData() {
		cweeCpuInfo_t cpuInfo;
		GetCPUInfo(cpuInfo);

		const_cast<int&>(this->m_numPhysicalCpuCores) = cpuInfo.processorCoreCount;
		const_cast<int&>(this->m_numLogicalCpuCores) = cpuInfo.logicalProcessorCount;
		const_cast<int&>(this->m_numCpuPackages) = cpuInfo.processorPackageCount;
	};

public:
	const int								m_numPhysicalCpuCores;
	const int								m_numLogicalCpuCores;
	const int								m_numCpuPackages;
};
#endif

cweeSharedPtr<cweeJobThreads> cweeSharedJobThreads = make_cwee_shared<cweeJobThreads>(new cweeJobThreads(cweeMath::max(cweeMath::min(CPU_DATA().m_numLogicalCpuCores, 32),1)));