/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#ifndef __SYS_PUBLIC__
#define __SYS_PUBLIC__

#include "precompiled.h"
#pragma hdrstop


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
#include "sys_local.h"
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

#define id_attribute(x)  

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

#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#pragma comment(lib, "User32.lib")
#include <locale> 
#include <codecvt>

std::vector<std::string>	Sys_ListFiles(const char* directory);
INLINE std::vector<std::string>	 Sys_ListFiles(const char* directory) {
	std::vector<std::string> list;
	
	TCHAR argv;
	WIN32_FIND_DATAW ffd;
	LARGE_INTEGER filesize;
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
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
	StringCchCopyW(szDir, MAX_PATH, thisStr.c_str());

	// Find the first file in the directory.
	hFind = FindFirstFileW(szDir, &ffd);
	do
	{
		std::wstring thisString(ffd.cFileName);
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;
		std::string converted_str = converter.to_bytes(thisString);

		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))	
			list.push_back(converted_str);
	} while (FindNextFileW(hFind, &ffd) != 0);
	FindClose(hFind);
	return list;
};

sysMemoryStats_t Sys_GetMemoryStats(void);
INLINE sysMemoryStats_t Sys_GetMemoryStats(void) {

	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	sysMemoryStats_t out;
	out.percentMemoryUsed = memInfo.dwMemoryLoad;
	out.totalPhysMemory = memInfo.ullTotalPhys;
	out.CurrentPhysMemoryUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
	out.totalPageMemory = memInfo.ullTotalPageFile;
	out.CurrentPageMemoryUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
	out.totalVirtualMemory = memInfo.ullTotalVirtual;
	out.CurrentVirtualMemoryUsed = memInfo.ullTotalVirtual - memInfo.ullAvailVirtual;

	return out;
}



#endif