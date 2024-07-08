#pragma once

#include "MultiWord_CompareAndSwap.h"

namespace fibers {
	namespace utilities {
		int Hardware::GetNumCpuCores() {

			typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

			PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = nullptr; // NULL
			PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = nullptr; // NULL
			PCACHE_DESCRIPTOR Cache;
			LPFN_GLPI	glpi;
			BOOL		done = FALSE;
			DWORD		returnLength = 0;
			DWORD		byteOffset = 0;

			CpuInfo_t cpuInfo;
			cpuInfo = CpuInfo_t();

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
						return 0;
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

			return cpuInfo.logicalProcessorCount;

			// return static_cast<int>(WindowsPlatform::GetCPUInfo().logicalProcessorCount); // std::thread::hardware_concurrency());
		};
		float Hardware::GetPercentCpuLoad() {
			auto CalculateCPULoad = [](unsigned long long idleTicks, unsigned long long totalTicks)->float
			{
				static unsigned long long _previousTotalTicks = 0;
				static unsigned long long _previousIdleTicks = 0;

				unsigned long long totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
				unsigned long long idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

				float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

				_previousTotalTicks = totalTicks;
				_previousIdleTicks = idleTicks;
				return ret;
			};
			auto FileTimeToInt64 = [](const FILETIME& ft)->unsigned long long
			{
				return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
			};

			FILETIME idleTime, kernelTime, userTime;
			return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? 100.0f * CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
		};
	};
};

namespace fibers {
	namespace utilities {
		namespace dbgroup::thread {
			std::shared_ptr<std::atomic_bool[]> id_vec = std::shared_ptr<std::atomic_bool[]>(new std::atomic_bool[kMaxThreadNum]);
			// std::shared_ptr<concurrency::concurrent_unordered_map<unsigned long long, std::atomic_bool>>  id_vec = std::shared_ptr<concurrency::concurrent_unordered_map<unsigned long long, std::atomic_bool>>(new concurrency::concurrent_unordered_map<unsigned long long, std::atomic_bool>());
		}
	};
};