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
#include <minwinbase.h>
// #include <cstdint>
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

typedef unsigned int (*xthread_t)(void*);

class cweeSysThreadTools {
public:
	static uintptr_t		Sys_GetCurrentThreadID() { return GetCurrentThreadId(); };
	static uintptr_t		Sys_CreateThread(xthread_t function, void* parms, int stackSize = 256 * 1024) {
		DWORD flags = (0);
		flags |= STACK_SIZE_PARAM_IS_A_RESERVATION;
		DWORD threadId;
		HANDLE handle = CreateThread(NULL, stackSize, (LPTHREAD_START_ROUTINE)function, parms, flags, &threadId);
		return (uintptr_t)handle;
	};
	static void				Sys_WaitForThread(uintptr_t threadHandle) { WaitForSingleObject((HANDLE)threadHandle, INFINITE); };
	static void				Sys_ForgetThread(uintptr_t threadHandle) { if (threadHandle == 0) { return; } CloseHandle((HANDLE)threadHandle); };
	static void				Sys_DestroyThread(uintptr_t threadHandle) { if (threadHandle == 0) { return; } WaitForSingleObject((HANDLE)threadHandle, INFINITE); CloseHandle((HANDLE)threadHandle); };
	static void				Sys_Yield() { SwitchToThread(); };
	static bool				Sys_TryYield() { return SwitchToThread(); };

	static void				Sys_SignalCreate(signalHandle_t& handle, bool manualReset) { handle = CreateEvent(NULL, manualReset, FALSE, NULL); };
	static void				Sys_SignalDestroy(signalHandle_t& handle) { CloseHandle(handle); };
	static void				Sys_SignalRaise(signalHandle_t& handle) { SetEvent(handle); };
	static void				Sys_SignalClear(signalHandle_t& handle) { ResetEvent(handle); };
	static bool				Sys_SignalWait(signalHandle_t& handle, int timeout = -1) { return WaitForSingleObject(handle, timeout == -1 ? INFINITE : timeout) == WAIT_OBJECT_0; };

	static void				Sys_MutexCreate(mutexHandle_t& handle) { InitializeCriticalSection(&handle); };
	static void				Sys_MutexDestroy(mutexHandle_t& handle) { DeleteCriticalSection(&handle); };
	static void				Sys_MutexLock(mutexHandle_t& handle) { EnterCriticalSection(&handle); };
	static bool				Sys_MutexTryLock(mutexHandle_t& handle) { return TryEnterCriticalSection(&handle) != 0; };
	static bool				Sys_MutexLock(mutexHandle_t& handle, bool blocking) { if (blocking) { Sys_MutexLock(handle); return true; } else { return Sys_MutexTryLock(handle); } };
	static void				Sys_MutexUnlock(mutexHandle_t& handle) { LeaveCriticalSection(&handle); };

	static interlockedInt_t Sys_InterlockedIncrement(interlockedInt_t& value) { return InterlockedIncrementAcquire(&value); };
	static interlockedInt_t Sys_InterlockedDecrement(interlockedInt_t& value) { return InterlockedDecrementRelease(&value); };
	static interlockedInt_t Sys_InterlockedAdd(interlockedInt_t& value, interlockedInt_t i) { return InterlockedExchangeAdd(&value, i) + i; };
	static interlockedInt_t Sys_InterlockedSub(interlockedInt_t& value, interlockedInt_t i) { return InterlockedExchangeAdd(&value, -i) - i; };
	static interlockedInt_t Sys_InterlockedExchange(interlockedInt_t& value, interlockedInt_t exchange) { return InterlockedExchange(&value, exchange); };
	static interlockedInt_t Sys_InterlockedCompareExchange(interlockedInt_t& value, interlockedInt_t comparand, interlockedInt_t exchange) { return InterlockedCompareExchange(&value, exchange, comparand); };

	template <typename type> static type Sys_InterlockedIncrementV(type& value) { return InterlockedIncrementAcquire(&value); }
	template <typename type> static type Sys_InterlockedDecrementV(type& value) { return InterlockedDecrementRelease(&value); }
	template <typename type> static type Sys_InterlockedAddV(type& value, const type& i) { return InterlockedExchangeAdd(&value, i) + i; }
	template <typename type> static type Sys_InterlockedSubV(type& value, const type& i) { return InterlockedExchangeAdd(&value, -i) - i; }
	template <typename type> static type Sys_InterlockedExchangeV(type& value, const type& exchange) { return InterlockedExchange(&value, exchange); }
	template <typename type> static type Sys_InterlockedCompareExchangeV(type& value, const type& comparand, const type& exchange) { return InterlockedCompareExchange(&value, exchange, comparand); }

	static void* Sys_InterlockedExchangePointer(void*& ptr, void* exchange) { return InterlockedExchangePointer(&ptr, exchange); };
	static void* Sys_InterlockedCompareExchangePointer(void*& ptr, void* comparand, void* exchange) { return InterlockedCompareExchangePointer(&ptr, exchange, comparand); };
};

class Computer_Usage {
public:
	Computer_Usage() : 
		percentMemoryUsed(0)
		, totalPhysMemory(0)
		, currentPhysMemoryUsed(0)
		, totalPageMemory(0)
		, currentPageMemoryUsed(0)
		, totalVirtualMemory(0)
		, currentVirtualMemoryUsed(0)
	{ Init(); };

public:
	float const& PercentMemoryUsed() const { return percentMemoryUsed; }
	float const& TotalPhysMemory() const { return totalPhysMemory; }
	float const& Ram_MB() const { return system_ram_MB; }
	float const& CurrentPhysMemoryUsed() const { return currentPhysMemoryUsed; }
	float const& TotalPageMemory() const { return totalPageMemory; }
	float const& CurrentPageMemoryUsed() const { return currentPageMemoryUsed; }
	float const& TotalVirtualMemory() const { return totalVirtualMemory; }
	float const& CurrentVirtualMemoryUsed() const { return currentVirtualMemoryUsed; }

private:
	typedef enum {
		CPUID_NONE = 0x00000,
		CPUID_UNSUPPORTED = 0x00001,	// unsupported (386/486)
		CPUID_GENERIC = 0x00002,		// unrecognized processor
		CPUID_INTEL = 0x00004,			// Intel
		CPUID_AMD = 0x00008,			// AMD
		CPUID_MMX = 0x00010,			// Multi Media Extensions
		CPUID_3DNOW = 0x00020,			// 3DNow!
		CPUID_SSE = 0x00040,			// Streaming SIMD Extensions
		CPUID_SSE2 = 0x00080,			// Streaming SIMD Extensions 2
		CPUID_SSE3 = 0x00100,			// Streaming SIMD Extentions 3 aka Prescott's New Instructions
		CPUID_ALTIVEC = 0x00200,		// AltiVec
		CPUID_HTT = 0x01000,			// Hyper-Threading Technology
		CPUID_CMOV = 0x02000,			// Conditional Move (CMOV) and fast floating point comparison (FCOMI) instructions
		CPUID_FTZ = 0x04000,			// Flush-To-Zero mode (denormal results are flushed to zero)
		CPUID_DAZ = 0x08000				// Denormals-Are-Zero mode (denormal source operands are set to zero)
	} cpuid_t;
	struct sysMemoryStats_t {
		DWORDLONG percentMemoryUsed;
		DWORDLONG totalPhysMemory;
		DWORDLONG CurrentPhysMemoryUsed;
		DWORDLONG totalPageMemory;
		DWORDLONG CurrentPageMemoryUsed;
		DWORDLONG totalVirtualMemory;
		DWORDLONG CurrentVirtualMemoryUsed;
	};
	void Init() {
		MEMORYSTATUSEX memInfo;
		memInfo.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&memInfo);

		system_ram_MB = (memInfo.ullTotalPhys / (1024 * 1024) + 8) & ~15; 
		percentMemoryUsed = memInfo.dwMemoryLoad;
		totalPhysMemory = memInfo.ullTotalPhys;
		currentPhysMemoryUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
		totalPageMemory = memInfo.ullTotalPageFile;
		currentPageMemoryUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
		totalVirtualMemory = memInfo.ullTotalVirtual;
		currentVirtualMemoryUsed = memInfo.ullTotalVirtual - memInfo.ullAvailVirtual;
	};
	float percentMemoryUsed;
	float totalPhysMemory;
	float system_ram_MB;
	float currentPhysMemoryUsed;
	float totalPageMemory;
	float currentPageMemoryUsed;
	float totalVirtualMemory;
	float currentVirtualMemoryUsed;

};

#include <windows.h>
#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <MAPI.h>
#include <shellapi.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

#include <Psapi.h>

#include <TCHAR.h>
#include <pdh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Win32
#if defined(WIN32) || defined(_WIN32)
#define	BUILD_STRING					"win-x86"
#define BUILD_OS_ID						0
#define	CPUSTRING						"x86"
#define CPU_EASYARGS					1
#define ALIGN16( x )					__declspec(align(16)) x
#define PACKED
#define _alloca16( x )					((void *)((((int)_alloca( (x)+15 )) + 15) & ~15))
#define PATHSEPERATOR_STR				"\\"
#define PATHSEPERATOR_CHAR				'\\'
#define INLINE						__forceinline
#define STATIC_TEMPLATE				static
#define assertmem( x, y )				assert( _CrtIsValidPointer( x, y, true ) )
#endif

#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
// #include <strsafe.h>
#pragma comment(lib, "User32.lib")
#include <locale> 
#include <codecvt>
#include <iostream>
#include <filesystem>

class DirectoryFiles {
public:
	DirectoryFiles() : files() {};
	DirectoryFiles(const char* directory) : files() { Init(directory); };
	std::vector<std::string> const& Files() const { return files; };

private:
	std::vector<std::string> files;
	void Init(const char* directory) {
		WIN32_FIND_DATAW ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WCHAR szDir[MAX_PATH];

		std::string s = directory;
		s += "\\*";
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		std::wstring thisStr = r;
		//StringCchCopyW(szDir, MAX_PATH, thisStr.c_str());

		// Find the first file in the directory.
		hFind = FindFirstFileW(thisStr.c_str()/*szDir*/, &ffd);
		do
		{
			std::wstring thisString(ffd.cFileName);
			using convert_type = std::codecvt_utf8<wchar_t>;
			std::wstring_convert<convert_type, wchar_t> converter;
			std::string converted_str = converter.to_bytes(thisString);

			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(converted_str);
		} while (FindNextFileW(hFind, &ffd) != 0);
		FindClose(hFind);
	};
};

class Directories {
public:
	Directories() : directories() {};
	Directories(const char* directory, bool includeFileNames) : directories() { Init(directory, includeFileNames); };
	std::vector<std::string> const& GetDirectories() const { return directories; };

private:
	std::vector<std::string> directories;
	void Init(const char* directory, bool includeFileNames) {
		std::map<std::string, int> get;

		using namespace std;
		using namespace std::filesystem;

		for (recursive_directory_iterator i(directory), end; i != end; ++i) {
			if (!is_directory(i->path())) {
				using convert_type = std::codecvt_utf8<wchar_t>;
				std::wstring_convert<convert_type, wchar_t> converter;
				std::string converted_str = converter.to_bytes(i->path().native());
				if (includeFileNames) {
					get[converted_str] = 0;
				}
				else {
					get[converted_str.substr(0, converted_str.rfind("\\"))] = 0;
				}				
			}
		}

		for (auto& x : get) directories.push_back(x.first);		
	};
};