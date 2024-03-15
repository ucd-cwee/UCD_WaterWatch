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

/* thread-safe and fiber-safe integer-style number values (Does not support doubles, floats, etc.) */
template <typename type> class cweeSysInterlockedValue {
public:
	cweeSysInterlockedValue() : value(static_cast<type>(0)) {};
	cweeSysInterlockedValue(const type& a) : value(a) {};
	cweeSysInterlockedValue(const cweeSysInterlockedValue& other) : value(other.GetValue()) {};
	cweeSysInterlockedValue& operator=(const cweeSysInterlockedValue& other) { SetValue(other.GetValue()); return *this; };
	cweeSysInterlockedValue& operator=(const type& newSource) { SetValue(newSource); return *this; };

	operator type() { return GetValue(); };
	operator type() const { return GetValue(); };

	friend cweeSysInterlockedValue operator+(const type& i, const cweeSysInterlockedValue& b) { cweeSysInterlockedValue out(b); out.Add(i); return out; };
	friend cweeSysInterlockedValue operator+(const cweeSysInterlockedValue& b, const type& i) { cweeSysInterlockedValue out(b); out.Add(i); return out; };
	friend cweeSysInterlockedValue operator-(const type& i, const cweeSysInterlockedValue& b) { cweeSysInterlockedValue out(i); out.Add(-b.GetValue()); return out; };
	friend cweeSysInterlockedValue operator-(const cweeSysInterlockedValue& b, const type& i) { cweeSysInterlockedValue out(b); out.Add(-i); return out; };
	friend cweeSysInterlockedValue operator/(const type& i, const cweeSysInterlockedValue& b) { return i / b.GetValue(); };
	friend cweeSysInterlockedValue operator/(const cweeSysInterlockedValue& b, const type& i) { return b.GetValue() / i; };

	cweeSysInterlockedValue& operator+=(int i) { Add(i); return *this; };
	cweeSysInterlockedValue& operator-=(int i) { Add(-i); return *this; };

	cweeSysInterlockedValue& operator+=(const cweeSysInterlockedValue& i) { Add(i.GetValue()); return *this; };
	cweeSysInterlockedValue& operator-=(const cweeSysInterlockedValue& i) { Add(-i.GetValue()); return *this; };

	bool operator==(const type& i) { return i == GetValue(); };
	bool operator!=(const type& i) { return i != GetValue(); };
	bool operator==(const cweeSysInterlockedValue& i) { return i.GetValue() == GetValue(); };
	bool operator!=(const cweeSysInterlockedValue& i) { return i.GetValue() != GetValue(); };

	friend bool operator<=(const type& i, const cweeSysInterlockedValue& b) { return i <= b.GetValue(); };
	friend bool operator<=(const cweeSysInterlockedValue& b, const type& i) { return i > b.GetValue(); };
	friend bool operator>=(const type& i, const cweeSysInterlockedValue& b) { return i >= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedValue& b, const type& i) { return i < b.GetValue(); };
	friend bool operator>(const type& i, const cweeSysInterlockedValue& b) { return i > b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedValue& b, const type& i) { return i <= b.GetValue(); };
	friend bool operator<(const type& i, const cweeSysInterlockedValue& b) { return i < b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedValue& b, const type& i) { return i >= b.GetValue(); };

	friend bool operator<=(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() <= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() >= b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() > b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() < b.GetValue(); };

	type					Increment() { return cweeSysThreadTools::Sys_InterlockedIncrementV(value); } // atomically increments the integer and returns the new value
	type					Decrement() { return cweeSysThreadTools::Sys_InterlockedDecrementV(value); } // atomically decrements the integer and returns the new value
	type					Add(const type& v) { return cweeSysThreadTools::Sys_InterlockedAddV(value, (type)v); } // atomically adds a value to the integer and returns the new value
	type					Sub(const type& v) { return cweeSysThreadTools::Sys_InterlockedSubV(value, (type)v); } // atomically subtracts a value from the integer and returns the new value
	type					GetValue() const { return value; } // returns the current value of the integer
	void				    SetValue(const type& v) { value = v; };

private:
	type	            value;
};

/* thread-safe and fiber-safe integer (atomic swapping of integers) */
class cweeSysInterlockedInteger {
public:
	constexpr cweeSysInterlockedInteger() noexcept : value(0) {};
	constexpr cweeSysInterlockedInteger(int a) noexcept : value(a) {};
	cweeSysInterlockedInteger(const cweeSysInterlockedInteger& other) : value(other.GetValue()) {};
	cweeSysInterlockedInteger& operator=(const cweeSysInterlockedInteger& other) { SetValue(other.GetValue()); return *this; };
	cweeSysInterlockedInteger& operator=(int newSource) { SetValue(newSource); return *this; };

	explicit operator int() { return GetValue(); };
	explicit operator int() const { return GetValue(); };

	explicit operator bool() { if (GetValue() == 0) return false; else return true; };
	explicit operator bool() const { if (GetValue() == 0) return false; else return true; };

	friend cweeSysInterlockedInteger operator+(int i, const cweeSysInterlockedInteger& b) { cweeSysInterlockedInteger out(b); out.Add(i); return out; };
	friend cweeSysInterlockedInteger operator+(const cweeSysInterlockedInteger& b, int i) { cweeSysInterlockedInteger out(b); out.Add(i); return out; };
	friend cweeSysInterlockedInteger operator-(int i, const cweeSysInterlockedInteger& b) { cweeSysInterlockedInteger out(i); out.Add(-b.GetValue()); return out; };
	friend cweeSysInterlockedInteger operator-(const cweeSysInterlockedInteger& b, int i) { cweeSysInterlockedInteger out(b); out.Add(-i); return out; };
	friend cweeSysInterlockedInteger operator/(int i, const cweeSysInterlockedInteger& b) { return i / b.GetValue(); };
	friend cweeSysInterlockedInteger operator/(const cweeSysInterlockedInteger& b, int i) { return b.GetValue() / i; };

	friend bool operator<=(int i, const cweeSysInterlockedInteger& b) { return i <= b.GetValue(); };
	friend bool operator<=(const cweeSysInterlockedInteger& b, int i) { return i > b.GetValue(); };
	friend bool operator>=(int i, const cweeSysInterlockedInteger& b) { return i >= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedInteger& b, int i) { return i < b.GetValue(); };
	friend bool operator>(int i, const cweeSysInterlockedInteger& b) { return i > b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedInteger& b, int i) { return i <= b.GetValue(); };
	friend bool operator<(int i, const cweeSysInterlockedInteger& b) { return i < b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedInteger& b, int i) { return i >= b.GetValue(); };

	friend bool operator<=(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() <= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() >= b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() > b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() < b.GetValue(); };

	cweeSysInterlockedInteger& operator+=(int i) { Add(i); return *this; };
	cweeSysInterlockedInteger& operator-=(int i) { Add(-i); return *this; };

	cweeSysInterlockedInteger& operator+=(const cweeSysInterlockedInteger& i) { Add(i.GetValue()); return *this; };
	cweeSysInterlockedInteger& operator-=(const cweeSysInterlockedInteger& i) { Add(-i.GetValue()); return *this; };

	bool operator==(int i) { return i == GetValue(); };
	bool operator!=(int i) { return i != GetValue(); };
	bool operator==(const cweeSysInterlockedInteger& i) { return i.GetValue() == GetValue(); };
	bool operator!=(const cweeSysInterlockedInteger& i) { return i.GetValue() != GetValue(); };

	int					Increment() { return cweeSysThreadTools::Sys_InterlockedIncrement(value); } // atomically increments the integer and returns the new value
	int					Decrement() { return cweeSysThreadTools::Sys_InterlockedDecrement(value); } // atomically decrements the integer and returns the new value
	int					Add(int v) { return cweeSysThreadTools::Sys_InterlockedAdd(value, (interlockedInt_t)v); } // atomically adds a value to the integer and returns the new value
	int					Sub(int v) { return cweeSysThreadTools::Sys_InterlockedSub(value, (interlockedInt_t)v); } // atomically subtracts a value from the integer and returns the new value
	int					GetValue() const { return cweeSysThreadTools::Sys_InterlockedAdd(const_cast<interlockedInt_t&>(value), 0); } // returns the current value of the integer
	void				SetValue(int v) { cweeSysThreadTools::Sys_InterlockedAdd(value, (interlockedInt_t)(v - GetValue())); };

	bool				TryIncrementTo(int n) {
		if (Increment() == n) {
			return true;
		}
		Decrement();
		return false;
	};

private:
	interlockedInt_t	value;
};

/* thread-safe and fiber-safe pointer (atomic swapping of pointers) */
template< typename T > class cweeSysInterlockedPointer {
public:
	constexpr cweeSysInterlockedPointer() noexcept : ptr(nullptr) {}
	constexpr cweeSysInterlockedPointer(std::nullptr_t) noexcept : ptr(nullptr) {}
	constexpr cweeSysInterlockedPointer(T* newSource) noexcept : ptr(newSource) {}
	constexpr cweeSysInterlockedPointer(const cweeSysInterlockedPointer& other) : ptr(other.Get()) {};
	cweeSysInterlockedPointer& operator=(const cweeSysInterlockedPointer& other) { Set(other.Get()); return *this; };
	cweeSysInterlockedPointer& operator=(T* newSource) { Set(newSource); return *this; };
	~cweeSysInterlockedPointer() { ptr = nullptr; };

	explicit operator bool() { return ptr; };
	explicit operator bool() const { return ptr; };

	constexpr operator T* () noexcept { return ptr; };
	constexpr operator const T* () const noexcept { return ptr; };

	/* atomically sets the pointerand returns the previous pointer value */
	T* Set(T* newPtr) { return (T*)cweeSysThreadTools::Sys_InterlockedExchangePointer((void*&)ptr, (void*)newPtr); };
	/* atomically sets the pointer to 'newPtr' only if the previous pointer is equal to 'comparePtr' */
	T* CompareExchange(T* comparePtr, T* newPtr) { return (T*)cweeSysThreadTools::Sys_InterlockedCompareExchangePointer((void*&)ptr, (void*)comparePtr, (void*)newPtr); };

	constexpr T* operator->() noexcept { return Get(); };
	constexpr const T* operator->() const noexcept { return Get(); };
	constexpr T* Get() noexcept { return ptr; };
	constexpr T* Get() const noexcept { return ptr; };

private:
	T* ptr;
};

class cweeConstexprLock {
public:
	class cweeConstexprLockLifetimeGuard {
	public:
		constexpr explicit operator bool() const noexcept { return true; };
		explicit cweeConstexprLockLifetimeGuard(cweeSysInterlockedInteger& mut) noexcept : owner(mut) { while (owner.Increment() != 1) { owner.Decrement(); } };
		~cweeConstexprLockLifetimeGuard() noexcept { owner.Decrement(); };
		explicit cweeConstexprLockLifetimeGuard() = delete;
		explicit cweeConstexprLockLifetimeGuard(const cweeSysInterlockedInteger& other) = delete;
		explicit cweeConstexprLockLifetimeGuard(cweeSysInterlockedInteger&& other) = delete;
		cweeConstexprLockLifetimeGuard& operator=(const cweeSysInterlockedInteger&) = delete;
		cweeConstexprLockLifetimeGuard& operator=(cweeSysInterlockedInteger&&) = delete;
	protected:
		cweeSysInterlockedInteger& owner;
	};

public:
	constexpr cweeConstexprLock() noexcept : Handle(0) {};
	constexpr cweeConstexprLock(const cweeConstexprLock& other) noexcept : Handle(0) {};
	constexpr cweeConstexprLock(cweeConstexprLock&& other) noexcept : Handle(0) {};
	constexpr cweeConstexprLock& operator=(const cweeConstexprLock& s) noexcept { return *this; };
	constexpr cweeConstexprLock& operator=(cweeConstexprLock&& s) noexcept { return *this; };
	~cweeConstexprLock() {};

	NODISCARD		cweeConstexprLockLifetimeGuard	Guard() const noexcept { return cweeConstexprLockLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const noexcept {
		if (blocking) {
			while (Handle.Increment() != 1) {
				Handle.Decrement();
				_mm_pause();
			}

			return true;
		}
		else {
			if (Handle.Increment() != 1) {
				Handle.Decrement();
				return false;
			}
			return true;
		}
	};
	void			Unlock() const noexcept {
		Handle.Decrement();
	};
	void            lock() const { Lock(); }
	void            unlock() const { Unlock(); }
protected:
	mutable cweeSysInterlockedInteger Handle;

};
class cweeSysMutexImpl {
public:
	cweeSysMutexImpl() noexcept { cweeSysThreadTools::Sys_MutexCreate(Handle); };
	~cweeSysMutexImpl()	noexcept { cweeSysThreadTools::Sys_MutexDestroy(Handle); };
	bool Lock(bool blocking = true) noexcept { return cweeSysThreadTools::Sys_MutexLock(Handle, blocking); };
	void Unlock() noexcept { cweeSysThreadTools::Sys_MutexUnlock(Handle); };

	cweeSysMutexImpl(const cweeSysMutexImpl&) = delete;
	cweeSysMutexImpl(cweeSysMutexImpl&&) = delete;
	cweeSysMutexImpl& operator=(cweeSysMutexImpl const&) = delete;
	cweeSysMutexImpl& operator=(cweeSysMutexImpl&&) = delete;

protected:
	mutexHandle_t Handle;
};
class cweeSysMutex {
public:
	using Handle_t = cweeSysMutexImpl;
	using Phandle = std::shared_ptr<Handle_t>;

	class cweeSysMutexLifetimeGuard {
	public:
		explicit operator bool() const { return (bool)owner; };
		explicit cweeSysMutexLifetimeGuard(const Phandle& mut) noexcept : owner(mut) { owner->Lock(); };
		~cweeSysMutexLifetimeGuard() noexcept { owner->Unlock(); };
		explicit cweeSysMutexLifetimeGuard() = delete;
		explicit cweeSysMutexLifetimeGuard(const cweeSysMutexLifetimeGuard& other) = delete;
		explicit cweeSysMutexLifetimeGuard(cweeSysMutexLifetimeGuard&& other) = delete;
		cweeSysMutexLifetimeGuard& operator=(const cweeSysMutexLifetimeGuard&) = delete;
		cweeSysMutexLifetimeGuard& operator=(cweeSysMutexLifetimeGuard&&) = delete;
	protected:
		Phandle owner;
	};

public:
	cweeSysMutex() : Handle(std::make_shared<Handle_t>()) {};
	cweeSysMutex(const cweeSysMutex& other) : Handle(std::make_shared<Handle_t>()) {};
	cweeSysMutex(cweeSysMutex&& other) : Handle(std::make_shared<Handle_t>()) {};
	cweeSysMutex& operator=(const cweeSysMutex& s) { return *this; };
	cweeSysMutex& operator=(cweeSysMutex&& s) { return *this; };
	~cweeSysMutex() {};

	NODISCARD cweeSysMutexLifetimeGuard	Guard() const { return cweeSysMutexLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const { return Handle->Lock(blocking); };
	void			Unlock() const { Handle->Unlock(); };

protected:
	Phandle Handle;

};
class cweeReadWriteMutex {
public:
	using Handle_t = std::shared_mutex;
	using Phandle = std::shared_ptr<Handle_t>;

	class cweeReadWriteMutexLifetime_ReadGuard {
	public:
		explicit operator bool() const { return (bool)owner; };
		explicit cweeReadWriteMutexLifetime_ReadGuard(const Phandle& mut) noexcept : owner(mut) { owner->lock_shared(); };
		~cweeReadWriteMutexLifetime_ReadGuard() noexcept { owner->unlock_shared(); };
		explicit cweeReadWriteMutexLifetime_ReadGuard() = delete;
		explicit cweeReadWriteMutexLifetime_ReadGuard(const cweeReadWriteMutexLifetime_ReadGuard& other) = delete;
		explicit cweeReadWriteMutexLifetime_ReadGuard(cweeReadWriteMutexLifetime_ReadGuard&& other) = delete;
		cweeReadWriteMutexLifetime_ReadGuard& operator=(const cweeReadWriteMutexLifetime_ReadGuard&) = delete;
		cweeReadWriteMutexLifetime_ReadGuard& operator=(cweeReadWriteMutexLifetime_ReadGuard&&) = delete;
	protected:
		Phandle owner;
	};

	class cweeReadWriteMutexLifetime_WriteGuard {
	public:
		explicit operator bool() const { return (bool)owner; };
		explicit cweeReadWriteMutexLifetime_WriteGuard(const Phandle& mut) noexcept : owner(mut) { owner->lock(); };
		~cweeReadWriteMutexLifetime_WriteGuard() noexcept { owner->unlock(); };
		explicit cweeReadWriteMutexLifetime_WriteGuard() = delete;
		explicit cweeReadWriteMutexLifetime_WriteGuard(const cweeReadWriteMutexLifetime_WriteGuard& other) = delete;
		explicit cweeReadWriteMutexLifetime_WriteGuard(cweeReadWriteMutexLifetime_WriteGuard&& other) = delete;
		cweeReadWriteMutexLifetime_WriteGuard& operator=(const cweeReadWriteMutexLifetime_WriteGuard&) = delete;
		cweeReadWriteMutexLifetime_WriteGuard& operator=(cweeReadWriteMutexLifetime_WriteGuard&&) = delete;
	protected:
		Phandle owner;
	};

public:
	cweeReadWriteMutex() : Handle(std::make_shared<Handle_t>()) {};
	cweeReadWriteMutex(const cweeReadWriteMutex& other) : Handle(std::make_shared<Handle_t>()) {};
	cweeReadWriteMutex(cweeReadWriteMutex&& other) : Handle(std::make_shared<Handle_t>()) {};
	cweeReadWriteMutex& operator=(const cweeReadWriteMutex& s) { return *this; };
	cweeReadWriteMutex& operator=(cweeReadWriteMutex&& s) { return *this; };
	~cweeReadWriteMutex() {};

	NODISCARD AUTO	Read_Guard() const { return cweeReadWriteMutexLifetime_ReadGuard(Handle); };
	NODISCARD AUTO	Write_Guard() const { return cweeReadWriteMutexLifetime_WriteGuard(Handle); };
	void			Read_Lock() const { Handle->lock_shared(); };
	void			Read_Unlock() const { Handle->unlock_shared(); };
	void			Write_Lock() const { Handle->lock(); };
	void			Write_Unlock() const { Handle->unlock(); };

	NODISCARD AUTO	Guard() const { return cweeReadWriteMutexLifetime_WriteGuard(Handle); };
	bool			Lock(bool blocking = true) const { Handle->lock(); return true; };
	void			Unlock() const { Handle->unlock(); };

protected:
	Phandle Handle;

};


class cweeSysSignalImpl {
public:
	static constexpr int WAIT_INFINITE = -1;
	cweeSysSignalImpl(bool manualReset = false) noexcept { cweeSysThreadTools::Sys_SignalCreate(Handle, manualReset); };
	~cweeSysSignalImpl()	noexcept { cweeSysThreadTools::Sys_SignalDestroy(Handle); };
	/* Raise an event. (Bool = true). Repeated raising is OK. */
	void	Raise() noexcept { cweeSysThreadTools::Sys_SignalRaise(Handle); }
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() noexcept { cweeSysThreadTools::Sys_SignalClear(Handle); }
	/* Waits till the event was called. (Bool == true ? return : continue). Waiting on a cweeSysSignal will put a thread to sleep until the thread is awoken by the Raise() from another thread.
	Wait returns true if the object is in a signalled state and returns false if the wait timed out. Wait also clears the signalled state when the signalled state is reached within the time out period.*/
	bool	Wait(int timeout = cweeSysSignalImpl::WAIT_INFINITE) noexcept { return cweeSysThreadTools::Sys_SignalWait(Handle, timeout); }

	cweeSysSignalImpl(const cweeSysSignalImpl&) = delete;
	cweeSysSignalImpl(cweeSysSignalImpl&&) = delete;
	cweeSysSignalImpl& operator=(cweeSysSignalImpl const&) = delete;
	cweeSysSignalImpl& operator=(cweeSysSignalImpl&&) = delete;

protected:
	signalHandle_t		Handle;
};
class cweeSysSignal {
public:
	static constexpr int	WAIT_INFINITE = -1;
	cweeSysSignal(bool manualReset = false) : Handle(new cweeSysSignalImpl(manualReset)) {};
	cweeSysSignal(const cweeSysSignal& other) : Handle(other.Handle) {};
	cweeSysSignal(cweeSysSignal&& other) : Handle(other.Handle) {};
	cweeSysSignal& operator=(const cweeSysSignal& s) { Handle = s.Handle; return *this; };
	cweeSysSignal& operator=(cweeSysSignal&& s) { Handle = s.Handle; return *this; };
	~cweeSysSignal() {};
	/* Raise an event. (Bool = true). Repeated raising is OK. */
	void	Raise() noexcept {
		std::shared_ptr<cweeSysSignalImpl> h = Handle;
		h->Raise();
	};
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() noexcept {
		std::shared_ptr<cweeSysSignalImpl> h = Handle;
		h->Clear();
	};
	/* Waits till the event was called. (Bool == true ? return : continue). Waiting on a cweeSysSignal will put a thread to sleep until the thread is awoken by the Raise() from another thread.
	Wait returns true if the object is in a signalled state and returns false if the wait timed out. Wait also clears the signalled state when the signalled state is reached within the time out period.*/
	bool	Wait(int timeout = cweeSysSignal::WAIT_INFINITE) noexcept {
		std::shared_ptr<cweeSysSignalImpl> h = Handle;
		return h->Wait(timeout);
	};
protected:
	mutable std::shared_ptr<cweeSysSignalImpl> Handle;
};


template< typename type>
class Interlocked {
public:
	class ExclusiveObject {
	public:
		constexpr ExclusiveObject(const Interlocked<type>& mut) : owner(const_cast<Interlocked<type>&>(mut)) { this->owner.Lock(); };
		~ExclusiveObject() { this->owner.Unlock(); };

		ExclusiveObject() = delete;
		ExclusiveObject(const ExclusiveObject& other) = delete; 
		ExclusiveObject(ExclusiveObject&& other) = delete; 
		ExclusiveObject& operator=(const ExclusiveObject& other) = delete;
		ExclusiveObject& operator=(ExclusiveObject&& other) = delete;

		type& operator=(const type& a) { data() = a; return data(); };
		type& operator=(type&& a) { data() = a; return data(); };
		type& operator*() const { return data(); };
		type* operator->() const { return &data(); };

	protected:
		type& data() const { return owner.UnsafeRead(); };
		Interlocked<type>& owner;
	};

public: // construction and destruction
	typedef type Type;

	Interlocked() : data() {};
	Interlocked(const type& other) : data(other) {};
	Interlocked(type&& other) : data(std::forward<type>(other)) {};
	Interlocked(const Interlocked& other) : data() { this->Copy(other); };
	Interlocked(Interlocked&& other) : data() { this->Copy(std::forward<Interlocked>(other)); };
	~Interlocked() {};

public: // copy and clear
	Interlocked<type>& operator=(const Interlocked<type>& other) {
		this->Copy(other);
		return *this;
	};
	Interlocked<type>& operator=(Interlocked<type>&& other) {
		this->Copy(std::forward<Interlocked<type>>(other));
		return *this;
	};
	void Copy(const Interlocked<type>& copy) {
		if (&copy == this) return;
		lock.Lock();
		copy.Lock();
		data = copy.data;
		copy.Unlock(); 
		lock.Unlock();
	};
	void Copy(Interlocked<type>&& copy) {
		if (&copy == this) return;
		lock.Lock();
		copy.Lock();
		data = copy.data;
		copy.Unlock();
		lock.Unlock();
	};
	void Clear() {
		lock.Lock();
		data = type();
		lock.Unlock();
	};

public: // read and swap
	type Read() const {
		AUTO g = Guard();
		return data;
	};
	void Swap(const type& replacement) {
		AUTO g = Guard();
		data = replacement;
	};
	Interlocked<type>& operator=(const type& other) {
		Swap(other);
		return *this;
	};

	operator type() const { return Read(); };
	type* operator->() const { return &data; };

public: // lock, unlock, and direct edit
	ExclusiveObject GetExclusive() const { return ExclusiveObject(*this); };

	NODISCARD AUTO Guard() const { return lock.Guard(); };
	void Lock() const { lock.Lock(); };
	void Unlock() const { lock.Unlock(); };
	type& UnsafeRead() const { return data; };

private:
	mutable type data;
	cweeConstexprLock lock;

};

#if 1
/* A standard thread timer that self-cleans after going out-of-scope, including when an app closes. Note that the last timer "sleep" must wake-up before the timer can close-out, which can delay closing of apps. */
class Timer {
private:
	class TimerImpl {
	public:
		TimerImpl(double secondsBetweenDispatch, Action const& queuedActivity) : handle(), stop(new cweeSysInterlockedInteger(0)) {
			std::tuple<double, Action, std::shared_ptr<cweeSysInterlockedInteger>>* data = new std::tuple<double, Action, std::shared_ptr<cweeSysInterlockedInteger>>(secondsBetweenDispatch, queuedActivity, stop);
			handle = cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
				if (_anon_ptr != nullptr) {
					std::tuple<double, Action, std::shared_ptr<cweeSysInterlockedInteger>>* T = static_cast<std::tuple< double, Action, std::shared_ptr<cweeSysInterlockedInteger>>*>(_anon_ptr);
					while (1) {
						if (std::get<2>(*T)->GetValue() >= 1) { break; }
#if 1
						::Sleep(std::get<0>(*T) * 1000.0);
#else
						Stopwatch sw; sw.Start();
						while (cweeUnitValues::nanosecond(sw.Stop()) < cweeUnitValues::millisecond(T->get<0>()) && T->get<2>()->GetValue() < 1) {
							ftl::YieldThread();
						}
#endif
						if (std::get<2>(*T)->GetValue() >= 1) { break; }

						std::get<1>(*T).ForceInvoke();
					}
					delete T;
				}
				return 0;
				}), (void*)data, 1024);
		};
		~TimerImpl() {
			stop->Increment();
			cweeSysThreadTools::Sys_DestroyThread(handle);
		};

	private:
		std::shared_ptr<cweeSysInterlockedInteger> stop;
		uintptr_t handle;
	};

public:
	Timer() : data(nullptr) {};
	Timer(Timer const&) = default;
	Timer(Timer&&) = default;
	Timer& operator=(Timer const&) = default;
	Timer& operator=(Timer&&) = default;
	Timer(double secondsBetweenDispatch, Action const& queuedActivity) : data(std::static_pointer_cast<void>(std::make_shared<TimerImpl>(secondsBetweenDispatch, queuedActivity))) {};
	~Timer() {};

private:
	std::shared_ptr<void> data;

};
#endif
