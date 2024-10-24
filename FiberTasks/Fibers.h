#pragma once

#pragma region Precompiled STL Headers
#pragma warning(disable : 4005)				// macro redefinition
#pragma warning(disable : 4010)				// single-line comment contains line-continuation character
#pragma warning(disable : 4018)				// singed / unsigned mismatch
#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4101)				// unreferenced local variable
#pragma warning(disable : 4127)				// conditional expression is constant
#pragma warning(disable : 4172)				// returning address of local variable or temporary
#pragma warning(disable : 4189)				// local variable is initialized but not referenced
#pragma warning(disable : 4200)				// nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable : 4238)				// nonstandard extension used: class rvalue used as lvalue
#pragma warning(disable : 4239)				// conversion from 'T' to 'T&'
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4251)				// needs to have dll-interface
#pragma warning(disable : 4267)				// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4273)				// inconsistent DLL linkage
#pragma warning(disable : 4293)				// '<<': shift count negative or too big, undefined behavior
#pragma warning(disable : 4297)				// function assumed not to throw but does
#pragma warning(disable : 4302)				// truncation from 'void *' to 'int'
#pragma warning(disable : 4305)				// truncating a literal from double to float
#pragma warning(disable : 4311)				// pointer truncation from 'void *' to 'int'
#pragma warning(disable : 4312)				// conversion from 'int' to 'void*' of greater size
#pragma warning(disable : 4351)             // Squelch warnings on initializing arrays; it is new (good) behavior in C++11.
#pragma warning(disable : 4390)				// ';' empty controlled statement
#pragma warning(disable : 4456)				// declaration hides previous local declaration
#pragma warning(disable : 4458)				// hides class member
#pragma warning(disable : 4459)				// hides global declaration
#pragma warning(disable : 4499)				// 'static': an explicit specialization cannot have a storage class
#pragma warning(disable : 4505)				// unreferenced local function has been removed
#pragma warning(disable : 4595)				// non-member operator new or delete functions may not be declared inline
#pragma warning(disable : 4701)				// potentially uninitialized local variable
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4715)				// not all control paths return a value
#pragma warning(disable : 4996)				// unsafe string operations
#pragma warning(disable : 6011)				// Dereferencing NULL ptr
#pragma warning(disable : 6385)				// Reading invalid data from buf
#pragma warning(disable : 26110)			// Caller failing to hold lock
#pragma warning(disable : 26439)			// This kind of function may not throw
#pragma warning(disable : 26450)			// Arithmetic overflow: using '<<'
#pragma warning(disable : 26451)			// Arithmetic overflow: using '*' on a 4 byte variable and casting to 8 bytes
#pragma warning(disable : 26495)			// uninitialized member variable type 6
#pragma warning(disable : 26498)			// Mark function constexpr if compile-time evaluation is desired
#pragma warning(disable : 26812)			// prefer enum class to enum
#pragma warning(disable : 28182)			// Dereferencing NULL pointer
#pragma warning(disable : 28251)			// Inconsistent annotation for 'new'
#define NOMINMAX
#define CONST_MAX( x, y )			( (x) > (y) ? (x) : (y) )
#define _CRT_FUNCTIONS_REQUIRED 1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include <functional>
#include <atomic>
#include <thread>
#include <emmintrin.h> // _mm_pause()
#include <chrono>
#include <string>
#include <memory>
#include <algorithm>
#include <deque>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <ShlDisp.h>
#include <mutex>
#include <shared_mutex>
#include <synchapi.h>
#include <handleapi.h>
#include <ppl.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_set.h>
#include <boost/any.hpp>
#include <cstdint>
#include <type_traits>
#include <execution>
#include <optional>
#include <future>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

#pragma endregion
#pragma region iterator_definition
#ifdef SETUP_STL_ITERATOR
#else
#define SETUP_STL_ITERATOR(ParentClass, IterType, StateType)  typedef std::ptrdiff_t difference_type;											 \
	typedef size_t size_type; typedef IterType value_type; typedef IterType* pointer; typedef const IterType* const_pointer;					 \
	typedef IterType& reference;																												 \
	typedef const IterType& const_reference;																									 \
	class iterator  : public std::iterator<std::random_access_iterator_tag, value_type> {					 \
	public: ParentClass* ref;	mutable StateType state;			 \
        using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;  \
		iterator() : ref(nullptr), state() {};																									 \
		iterator(ParentClass* parent) : ref(parent), state() {};																			 \
		inline iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };									 \
		inline iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };									 \
        inline value_type& operator*() { return state.get(ref); }  \
        inline value_type* operator->() { return &state.get(ref); }  \
        inline const value_type& operator*() const { return state.get(ref); }  \
        inline const value_type* operator->() const { return &state.get(ref); }  \
        inline value_type& operator[](difference_type rhs) const { return *(*this + rhs); }  \
        inline iterator& operator++() { state.next(ref); return *this; };  \
        inline iterator& operator--() { state.prev(ref); return *this; };  \
		inline iterator operator++(int) { iterator retval = *this; ++(*this); return retval; };  \
		inline iterator operator--(int) { iterator retval = *this; --(*this); return retval; };  \
        inline difference_type operator-(iterator const& other) const { return state.distance(other.state); };  \
		inline iterator operator+(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };	 \
        inline iterator operator-(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };	 \
		friend inline iterator operator+(difference_type lhs, const iterator& rhs) { return rhs + lhs; }  \
		friend inline iterator operator-(difference_type lhs_pos, const iterator& rhs) { iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++){ ++retval; } return retval; }  \
		inline bool operator==(const iterator& other) const { return !(operator!=(other)); }  \
		inline bool operator!=(const iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }  \
		inline bool operator>(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }  \
		inline bool operator<(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos;  }  \
		inline bool operator>=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos;  }  \
		inline bool operator<=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos;  }  \
		iterator& begin() { state.begin(ref); return *this; };  \
		iterator& end() { state.end(ref); return *this; };  \
	}; \
	iterator begin() { return iterator(this).begin(); };																						 \
	iterator end() { return iterator(this).end(); }; \
    \
	class const_iterator  : public std::iterator<std::random_access_iterator_tag, value_type> {					 \
	public: const ParentClass* ref;	mutable StateType state;			 \
        using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;  \
		const_iterator() : ref(nullptr), state() {};																									 \
		const_iterator(const ParentClass* parent) : ref(parent), state() {};																			 \
		inline const_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };									 \
		inline const_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };									 \
        inline const value_type& operator*() const { return state.get(ref); }  \
        inline const value_type* operator->() const { return &state.get(ref); }  \
        inline const value_type& operator[](difference_type rhs) const { return *(*this + rhs); }  \
        inline const_iterator& operator++() { state.next(ref); return *this; };  \
        inline const_iterator& operator--() { state.prev(ref); return *this; };  \
		inline const_iterator operator++(int) { const_iterator retval = *this; ++(*this); return retval; };  \
		inline const_iterator operator--(int) { const_iterator retval = *this; --(*this); return retval; };  \
        inline difference_type operator-(const_iterator const& other) const { return state.distance(other.state); };  \
		inline const_iterator operator+(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };	 \
        inline const_iterator operator-(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };	 \
		friend inline const_iterator operator+(difference_type lhs, const const_iterator& rhs) { return rhs + lhs; }  \
		friend inline const_iterator operator-(difference_type lhs_pos, const const_iterator& rhs) { const_iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++){ ++retval; } return retval; }  \
		inline bool operator==(const const_iterator& other) const { return !(operator!=(other)); }  \
		inline bool operator!=(const const_iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }  \
		inline bool operator>(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }  \
		inline bool operator<(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos;  }  \
		inline bool operator>=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos;  }  \
		inline bool operator<=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos;  }  \
		const_iterator& begin() { state.begin(ref); return *this; };  \
		const_iterator& end() { state.end(ref); return *this; };  \
	}; \
	const_iterator cbegin() const { return const_iterator(this).begin(); };																		 \
	const_iterator cend() const { return const_iterator(this).end(); };																			 \
	const_iterator begin() const { return cbegin(); };																							 \
	const_iterator end() const { return cend(); };
#endif
#pragma endregion 

#include <minwinbase.h>
#include <synchapi.h>
#include <processthreadsapi.h>
#include <handleapi.h>
#include <libloaderapi.h>
#include <errhandlingapi.h>
#include <winerror.h>
#include <Windows.h>
#include <winnt.h>
#include <Psapi.h>

/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#if defined(_WIN32)
#include <psapi.h>
#include <windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) ||                 \
    (defined(__APPLE__) && defined(__MACH__))
#include <sys/resource.h>
#include <unistd.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) ||                              \
    (defined(__sun__) || defined(__sun) ||                                     \
     defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) ||              \
    defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif

namespace fibers::utilities {
	 /**
	  * Returns the peak (maximum so far) resident set size (physical
	  * memory use) measured in bytes, or zero if the value cannot be
	  * determined on this OS.
	  */
	inline size_t getPeakRSS() {
#if defined(_WIN32)
		/* Windows -------------------------------------------------- */
		PROCESS_MEMORY_COUNTERS info;
		GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
		return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) ||                              \
    (defined(__sun__) || defined(__sun) ||                                     \
     defined(sun) && (defined(__SVR4) || defined(__svr4__)))
		/* AIX and Solaris ------------------------------------------ */
		struct psinfo psinfo;
		int fd = -1;
		if ((fd = open("/proc/self/psinfo", O_RDONLY)) == -1)
			return (size_t)0L; /* Can't open? */
		if (read(fd, &psinfo, sizeof(psinfo)) != sizeof(psinfo)) {
			close(fd);
			return (size_t)0L; /* Can't read? */
		}
		close(fd);
		return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) ||                 \
    (defined(__APPLE__) && defined(__MACH__))
		/* BSD, Linux, and OSX -------------------------------------- */
		struct rusage rusage;
		getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__) && defined(__MACH__)
		return (size_t)rusage.ru_maxrss;
#else
		return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
		/* Unknown OS ----------------------------------------------- */
		return (size_t)0L; /* Unsupported. */
#endif
	}

	/**
	 * Returns the current resident set size (physical memory use) measured
	 * in bytes, or zero if the value cannot be determined on this OS.
	 */
	inline size_t getCurrentRSS() {
#if defined(_WIN32)
		/* Windows -------------------------------------------------- */
		PROCESS_MEMORY_COUNTERS info;
		GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
		return (size_t)info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
		/* OSX ------------------------------------------------------ */
		struct mach_task_basic_info info;
		mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
		if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info,
			&infoCount) != KERN_SUCCESS)
			return (size_t)0L; /* Can't access? */
		return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) ||              \
    defined(__gnu_linux__)
		/* Linux ---------------------------------------------------- */
		long rss = 0L;
		FILE* fp = NULL;
		if ((fp = fopen("/proc/self/statm", "r")) == NULL)
			return (size_t)0L; /* Can't open? */
		if (fscanf(fp, "%*s%ld", &rss) != 1) {
			fclose(fp);
			return (size_t)0L; /* Can't read? */
		}
		fclose(fp);
		return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);

#else
		/* AIX, BSD, Solaris, and Unknown OS ------------------------ */
		return (size_t)0L; /* Unsupported. */
#endif
	}

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
			, virtualMemUsedByProcess(0)
			, currentRSS(0)
			, maxRSS(0)
		{
			Init();
		};
		Computer_Usage(Computer_Usage const& o) = default;
		Computer_Usage(Computer_Usage&& o) = default;
		Computer_Usage& operator=(Computer_Usage const& o) = default;
		Computer_Usage& operator=(Computer_Usage&& o) = default;
		~Computer_Usage() = default;

	public:
		long double PercentMemoryUsed() const { return percentMemoryUsed; }
		long double TotalPhysMemory() const { return totalPhysMemory; }
		long double Ram_MB() const { return system_ram_MB; }
		long double CurrentPhysMemoryUsed() const { return currentPhysMemoryUsed; }
		long double TotalPageMemory() const { return totalPageMemory; }
		long double CurrentPageMemoryUsed() const { return currentPageMemoryUsed; }
		long double TotalVirtualMemory() const { return totalVirtualMemory; }
		long double CurrentVirtualMemoryUsed() const { return currentVirtualMemoryUsed; }
		long double CurrentVirtualMemoryUsedByProcess() const { return virtualMemUsedByProcess; }
		long double CurrentRSS() const { return currentRSS; }
		long double CurrentMaxRSS() const { return maxRSS; }
		long double CurrentMemoryUsed() const { return currentPhysMemoryUsed + currentPageMemoryUsed + currentVirtualMemoryUsed; }
	private:
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

			PROCESS_MEMORY_COUNTERS_EX pmc;
			GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
			virtualMemUsedByProcess = pmc.PrivateUsage;

			currentRSS = getCurrentRSS();
			maxRSS = getPeakRSS();
		};
		float percentMemoryUsed;
		float totalPhysMemory;
		float system_ram_MB;
		float currentPhysMemoryUsed;
		float totalPageMemory;
		float currentPageMemoryUsed;
		float totalVirtualMemory;
		float currentVirtualMemoryUsed;
		size_t virtualMemUsedByProcess;
		size_t currentRSS;
		size_t maxRSS;
	};
};

#include "Actions.h"
#include "Concurrent_Queue.h"
#include "../WaterWatchCpp/enum.h"
#include <set>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <assert.h>
#include <thread>
#include <iostream>
#include <memory>
#include <atomic>
#include <cstdarg>
#include <iostream>
#include <array>
#include <cstring>
#include <string>
#include <sstream>
#include <mutex>
#include <map>
#include <type_traits>
#include "MultiWord_CompareAndSwap.h"

namespace fibers {
	namespace containers {
		/* Fiber- and thread-safe vector. Objects are stored and returned as std::shared_ptr. Growth, iterations, and push_back operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire vector. */
		template<typename _Ty>
		class vector {
		protected:
			using underlying = typename concurrency::concurrent_vector<std::shared_ptr<_Ty>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const vector* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const vector* ref) { ++pos; }
				inline void end(const vector* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(vector* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const vector* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const vector* ref) const { return *pos; }
			};

			SETUP_STL_ITERATOR(vector, typename underlying::value_type, it_state);

			vector() : data(new underlying()) {};
			explicit vector(size_type _N) : data(new underlying(_N)) {};
			vector(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			vector(vector const& r) : data(new underlying()) {
				operator=(r);
			}
			vector(vector&& r) = default;
			vector& operator=(vector const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					vector out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			vector& operator=(vector&& r) = default;
			~vector() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(std::make_shared<_Ty>(_Item)); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::make_shared<_Ty>(std::forward<_Ty>(_Item))); };
			decltype(auto) push_back(std::shared_ptr<_Ty> const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(std::shared_ptr<_Ty>&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			std::shared_ptr<_Ty> operator[](size_type _Index) { return data->operator[](_Index); };
			std::shared_ptr<_Ty> operator[](size_type _Index) const { return data->operator[](_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) { return data->at(_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
			decltype(auto) erase(const_iterator _Where) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) clear() {
				vector out;
				data.swap(out.data);
			};
			decltype(auto) swap(vector& r) {
				data.swap(r.data);
			};
		};

		/* Fiber- and thread-safe vector that cannot erase elements. Objects are stored as-is, and therefore the array must outlive the underlying objects. All operations are concurrent except for 'operator=(array)'.
		Higher performance is expected with the array than the vector, but the user needs to be slighly more careful with their timing to ensure the array remaings alive until access is complete. */
		template<typename _Ty>
		class array {
		protected:
			using underlying = typename concurrency::concurrent_vector<_Ty>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const array* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const array* ref) { ++pos; }
				inline void end(const array* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(array* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const array* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const array* ref) const { return *pos; }
			};

			SETUP_STL_ITERATOR(array, typename underlying::value_type, it_state);

			array() : data(new underlying()) {};
			explicit array(size_type _N) : data(new underlying(_N)) {};
			array(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			array(array const& r) : data(new underlying()) {
				operator=(r);
			}
			array(array&& r) = default;
			array& operator=(array const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					array out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			array& operator=(array&& r) = default;
			~array() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			decltype(auto) operator[](size_type _Index) { return data->operator[](_Index); };
			decltype(auto) operator[](size_type _Index) const { return data->operator[](_Index); };
			decltype(auto) at(size_type _Index) { return data->at(_Index); };
			decltype(auto) at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
		};

		/* Fiber- and thread-safe map / dictionary. Objects are stored and returned as std::shared_ptr. Growth, iterations, and insert/emplace operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire map. */
		template<typename _Key_type, typename _Element_type>
		class unordered_map {
		protected:
			using underlying = typename concurrency::concurrent_unordered_map<_Key_type, std::shared_ptr<_Element_type>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;

				inline void begin(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const unordered_map* ref) { ++pos; }
				inline void end(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(unordered_map* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				inline void prev(const unordered_map* ref) { --pos; }
				inline const typename underlying::value_type& get(const unordered_map* ref) const { return *pos; }
			};

			SETUP_STL_ITERATOR(unordered_map, typename underlying::value_type, it_state);

			typedef typename underlying::key_type key_type;
			typedef typename underlying::mapped_type mapped_type;
			typedef typename underlying::key_equal key_equal;
			typedef typename underlying::hasher hasher;
			typedef iterator local_iterator;
			typedef const_iterator const_local_iterator;

			unordered_map() : data(new underlying()) {};
			explicit unordered_map(size_type _N) : data(new underlying(_N)) {};
			unordered_map(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			template<class _InputIterator> unordered_map(_InputIterator _Begin, _InputIterator _End) : data(new underlying(_Begin, _End)) {};
			unordered_map(unordered_map const& r) : data(new underlying()) {
				this->operator=(r);
			};
			unordered_map(unordered_map&& r) = default;
			unordered_map& operator=(unordered_map const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					unordered_map out;
					for (auto& x : *r.data) out[x.first] = x.second;
					out.data.swap(data);
				}
				return *this;
			};
			unordered_map& operator=(unordered_map&& r) = default;

			decltype(auto) insert(const value_type& _Value) { return data->insert(_Value); };
			decltype(auto) insert(const_iterator _Where, const value_type& _Value) { return data->insert(_Where, _Value); };
			template<class _Iterator> decltype(auto) insert(_Iterator _First, _Iterator _Last) { return data->insert(_First, _Last); };
			template<class _Valty> decltype(auto) insert(_Valty&& _Value) { return data->insert(_Value); };
			template<class _Valty> decltype(auto) insert(const_iterator _Where, _Valty&& _Value) { return data->insert(_Where, _Value); };
			decltype(auto) hash_function() const { return data->hash_function(); };
			decltype(auto) key_eq() const { return data->key_eq(); };
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) const {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) { return data->at(_Keyval); };
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) const { return data->at(_Keyval); };
			decltype(auto) front() { return data->front(); };
			decltype(auto) front() const { return data->front(); };
			decltype(auto) back() { return data->back(); };
			decltype(auto) back() const { return data->back(); };
			decltype(auto) erase(const_iterator _Where) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const key_type& _Keyval) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter->first == _Keyval) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) unsafe_erase(const key_type& _Keyval) {
				data->unsafe_erase(_Keyval);
			};
			decltype(auto) count(const key_type& _Keyval) const { return data->count(_Keyval); };
			decltype(auto) emplace(const key_type& _Keyval, const std::shared_ptr<_Element_type>& _Value) { return insert(value_type(_Keyval, _Value)); };
			decltype(auto) emplace(const key_type& _Keyval, std::shared_ptr<_Element_type>&& _Value) { return insert(value_type(_Keyval, std::forward<typename underlying::value_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, const _Element_type& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, _Element_type&& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(std::forward<typename underlying::value_type>(_Value)))); };
			decltype(auto) clear() {
				unordered_map out;
				data.swap(out.data);
			};
		};

		template<typename _Key_type> using unordered_set = concurrency::concurrent_unordered_set<_Key_type>; /* Wrapper To-Do */
		template<typename _Value_type> using queue = moodycamel::ConcurrentQueue<_Value_type>; //  concurrency::concurrent_queue<_Value_type>; /* Wrapper To-Do */
	};

	namespace synchronization {
		/* *THREAD SAFE* Simple atomic spin-lock that unfairly synchronizes threads or fibers. (No guarrantee of order). Suggest using the ticket spin lock instead. */
		class SpinLock {
		private:
			std::atomic_flag lck = ATOMIC_FLAG_INIT;
		public:
			inline void lock() {
				int spin = 0;
				while (!try_lock())
				{
					if (spin < 10)
					{
						_mm_pause(); // SMT thread swap can occur here
					}
					else
					{
						std::this_thread::yield(); // OS thread swap can occur here. It is important to keep it as fallback, to avoid any chance of lockup by busy wait
					}
					spin++;
				}
			};
			inline bool try_lock() {
				return !lck.test_and_set(std::memory_order_acquire);
			};
			inline void unlock() {
				lck.clear(std::memory_order_release);
			};
		};
		
		/* *THREAD SAFE* Windows-specific high-performance lock that only locks the OS (slow) when contention actually happens. When there is no contention, this is very fast. 
		Generally speaking, out-performs std::mutex under most conditions. */
		class mutex {
		private:
			using mutexHandle_t = CRITICAL_SECTION;
			static void				Sys_MutexCreate(mutexHandle_t& handle) noexcept { InitializeCriticalSection(&handle); };
			static void				Sys_MutexDestroy(mutexHandle_t& handle) noexcept { DeleteCriticalSection(&handle); };
			static void				Sys_MutexLock(mutexHandle_t& handle) noexcept { EnterCriticalSection(&handle); };
			static bool				Sys_MutexTryLock(mutexHandle_t& handle) noexcept { return TryEnterCriticalSection(&handle) != 0; };
			static void				Sys_MutexUnlock(mutexHandle_t& handle) noexcept { LeaveCriticalSection(&handle); };
		public:
			mutex() noexcept { Sys_MutexCreate(Handle); };
			~mutex() noexcept { Sys_MutexDestroy(Handle); };

			inline void lock() {
				Sys_MutexLock(Handle);
			};
			inline bool try_lock() {
				return Sys_MutexTryLock(Handle);
			};
			inline void unlock() {
				Sys_MutexUnlock(Handle);
			};

			mutex(const mutex&) = delete;
			mutex(mutex&&) = delete;
			mutex& operator=(mutex const&) = delete;
			mutex& operator=(mutex&&) = delete;

		protected:
			mutexHandle_t Handle;

		};

		/* mutex which allows multiple readers OR one writer to access a critical section at the same time. */
		template <typename mutex = fibers::synchronization::mutex> class shared_mutex {
		private:
			static auto GetUnderlyingConditionalVariableExample() {
				if constexpr (std::is_same<mutex, std::mutex>::value) {
					return std::condition_variable();
				}
				else {
					return std::condition_variable_any();
				}
			};

		public:
			using cond_var = typename fibers::utilities::function_traits<decltype(std::function(GetUnderlyingConditionalVariableExample))>::result_type;

		private:
			mutex    mut_;
			cond_var gate1_;
			cond_var gate2_;
			unsigned state_;

			static const unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
			static const unsigned n_readers_ = ~write_entered_;

		public:
			shared_mutex() : mut_(), gate1_(), gate2_(), state_(0) {}

			// Exclusive ownership
			void lock() {
				std::unique_lock<mutex> lk(mut_);
				while (state_ & write_entered_) gate1_.wait(lk);
				state_ |= write_entered_;
				while (state_ & n_readers_) gate2_.wait(lk);
			};
			bool try_lock() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock_t);
				if (lk.owns_lock() && state_ == 0) {
					state_ = write_entered_;
					return true;
				}
				return false;
			};
			void unlock() {
				{
					std::scoped_lock<mutex> _(mut_);
					state_ = 0;
				}
				gate1_.notify_all();
			};

			// Shared ownership
			void lock_shared() {
				std::unique_lock<mutex> lk(mut_);
				while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_) gate1_.wait(lk);
				unsigned num_readers = (state_ & n_readers_) + 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
			};
			bool try_lock_shared() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock_t);
				unsigned num_readers = state_ & n_readers_;
				if (lk.owns_lock() && !(state_ & write_entered_) && num_readers != n_readers_) {
					++num_readers;
					state_ &= ~n_readers_;
					state_ |= num_readers;
					return true;
				}
				return false;
			};
			void unlock_shared() {
				std::scoped_lock<mutex> _(mut_);
				unsigned num_readers = (state_ & n_readers_) - 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
				if (state_ & write_entered_) {
					if (num_readers == 0) gate2_.notify_one();
				}
				else {
					if (num_readers == n_readers_ - 1) gate1_.notify_one();
				}
			};
		
		};

		/* *THREAD SAFE* Thread-safe and fiber-safe wrapper for any type of number, from integers to doubles. 
		   Significant performance boost if the data type is an integer type or one of: long, unsigned int, unsigned long, unsigned __int64		   
		   Slower, but still atomic using multi-word CAS algorithms, if using floating-point numbers like doubles or floats. 
		*/
		template<typename type = double>
		struct atomic_number {
		public:
			static constexpr bool isFloatingPoint = std::is_floating_point<type>::value || std::is_same_v<type, long long>;
			static constexpr bool isSigned = std::is_signed<type>::value;

		private:
			static auto ValidTypeExample() {
				if constexpr (isFloatingPoint) return fibers::utilities::CAS_Container<type>{}; // this makes it actually thread-safe. DoubleWrapper does not. 
				else if constexpr (std::is_same_v<type, long long>) return fibers::utilities::CAS_Container<type>{}; // this makes it actually thread-safe. DoubleWrapper does not. 
				else { // Integral. Only 4 integral types are actually supported. long, unsigned int, unsigned long, unsigned __int64
					if constexpr (isSigned) {
						return static_cast<long>(0);
					}
					else {
						// get the largest type that can contain the requested type. 
						if constexpr (sizeof(type) <= sizeof(unsigned int)) {
							return static_cast<unsigned int>(0);
						}
						else if constexpr (sizeof(type) <= sizeof(unsigned long)) {
							return static_cast<unsigned long>(0);
						}
						else {
							return static_cast<unsigned __int64>(0);
						}
					}
				}
			};
			using internalType = typename std::remove_const_t<typename fibers::utilities::function_traits<decltype(std::function(ValidTypeExample))>::result_type>;

		public:
			constexpr atomic_number() : value{ static_cast<internalType>(0) }  {};
			constexpr atomic_number(const type& a) : value{ static_cast<internalType>(a) } {};
			constexpr atomic_number(type&& a) : value{ static_cast<internalType>(std::forward<type>(a)) } {};
			atomic_number(const atomic_number& other) : value{ other.load() } {};
			atomic_number(atomic_number&& other) : value{ other.load() } {};
			atomic_number& operator=(const atomic_number& other) { SetValue(other.load()); return *this; };
			atomic_number& operator=(atomic_number&& other) { SetValue(other.load()); return *this; };
			~atomic_number() = default;

			operator type() { return GetValue(); };
			operator const type() const { return GetValue(); };

			template <typename T> decltype(auto) operator+(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) + static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) + static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) + static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) + static_cast<type>(b.value);
						return out;
					}
				}
			};
			template <typename T> decltype(auto) operator-(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) - static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) - static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) - static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) - static_cast<type>(b.value);
						return out;
					}
				}
			};
			template <typename T> decltype(auto) operator/(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) / static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) / static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) / static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) / static_cast<type>(b.value);
						return out;
					}
				}
			};
			template <typename T> decltype(auto) operator*(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) * static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) * static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) * static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) * static_cast<type>(b.value);
						return out;
					}
				}
			};
			
			atomic_number& operator--() { 
				if constexpr (isFloatingPoint) {
					value.Add(-1);
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(-1));
					return *this;
				}
			};
			atomic_number& operator++() { 
				if constexpr (isFloatingPoint) {
					value.Add(1);
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(1));
					return *this;
				}
			};
			atomic_number operator--(int) { return Decrement() + 1; };
			atomic_number operator++(int) { return Increment() - 1; };

			atomic_number& operator+=(const atomic_number& i) { 
				if constexpr (isFloatingPoint) {
					value.Add(i.load());
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(i.value));
					return *this;
				}
			};
			atomic_number& operator-=(const atomic_number& i) { 
				if constexpr (isFloatingPoint) {
					value.Add(-1.0 * (i.load()));
					return *this;
				}
				else {
					//type previousV;
					//while (true) {
					//	previousV = value;
					//	if (previousV == InterlockedCompareExchange(&value, previousV - static_cast<internalType>(i.value), previousV)) {
					//		break;
					//	}
					//}
					InterlockedExchangeAdd(&value, static_cast<internalType>(-i.value));
					return *this;
				}
			};
			atomic_number& operator/=(const atomic_number& i) {
				if constexpr (isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV /= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}
						
					return *this;
				}
				else {
					InterlockedExchange(&value, static_cast<internalType>(value / i.value));
					return *this;
				}
			};			
			atomic_number& operator*=(const atomic_number& i) {
				if constexpr (isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV *= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}

					return *this;
				}
				else {
					InterlockedExchange(&value, static_cast<internalType>(value * i.value));
					return *this;
				}
			};

			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator+=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					value.Add(static_cast<type>(i.load()));
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(i.load()));
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator-=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					value.Add(-(i.load()));
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(-i.load()));
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator/=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV /= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}
				}
				else {
					InterlockedExchange(&value, value / static_cast<internalType>(i.load()));
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator*=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV *= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}
				}
				else {
					InterlockedExchange(&value, value * static_cast<internalType>(i.load()));
				}
				return *this;
			};

			template <typename T> bool operator==(const atomic_number<T>& b) const {
				return std::abs(static_cast<long double>(load()) - static_cast<long double>(b.load())) <= 0.00005l;
			};
			template <typename T> bool operator!=(const atomic_number<T>& b) const {
				return !operator==(b);
			};
			template <typename T> bool operator<=(const atomic_number<T>& b) {
				if constexpr (sizeof(T) > sizeof(type)) {
					return static_cast<T>(load()) <= static_cast<T>(b.load());
				}
				else {
					return static_cast<type>(load()) <= static_cast<type>(b.load());
				}
			};
			template <typename T> bool operator>=(const atomic_number<T>& b) {
				if constexpr (sizeof(T) > sizeof(type)) {
					return static_cast<T>(load()) >= static_cast<T>(b.load());
				}
				else {
					return static_cast<type>(load()) >= static_cast<type>(b.load());
				}
			};
			template <typename T> bool operator<(const atomic_number<T>& b) {
				return !operator>=(b);
			};
			template <typename T> bool operator>(const atomic_number<T>& b) {
				return !operator<=(b);
			};

			atomic_number pow(atomic_number const& V) const { 
				if constexpr (isFloatingPoint) {
					return std::pow(value.load(), V.load());
				}
				else {
					return std::pow(value, V.value);
				}
			};
			atomic_number sqrt() const { 
				if constexpr (isFloatingPoint) {
					return std::sqrt(value.load());
				}
				else {
					return std::sqrt(value);
				}
			};
			atomic_number floor() const { 
				if constexpr (isFloatingPoint) {
					return std::floor(value.load());
				}
				else {
					return std::floor(value);
				}
			};
			atomic_number ceiling() const { 
				if constexpr (isFloatingPoint) {
					return std::ceil(value.load());
				}
				else {
					return std::ceil(value);
				}
			};
			//static atomic_number min(atomic_number const& a, atomic_number const& b) const {
			//	if constexpr (isFloatingPoint) {
			//		return std::min(a.load(), b.load());
			//	}
			//	else {
			//		return std::min(a, b);
			//	}
			//};
			//static atomic_number max(atomic_number const& a, atomic_number const& b) const {
			//	if constexpr (isFloatingPoint) {
			//		return std::max(a.load(), b.load());
			//	}
			//	else {
			//		return std::max(a, b);
			//	}
			//};

			bool CompareExchange(type expected, type newValue) {
				if constexpr (isFloatingPoint) {
					return value.CompareSwap(expected, newValue);
				}
				else {
					return InterlockedCompareExchange(&value, newValue, expected) == expected;
				}
			}; // atomically sets the value if the old value equals expected. Returns true on success.
			type Max(type newValue) {
				while (true) {
					auto previous = load();
					if (CompareExchange(previous, std::max<type>(previous, newValue))) return previous;
				}
			}; // Sets the atomic_number to the max of its current value and the newValue. Returns the previous value once successful.
			type Min(type newValue) {
				while (true) {
					auto previous = load();
					if (CompareExchange(previous, std::min<type>(previous, newValue))) return previous;
				}
			}; // Sets the atomic_number to the min of its current value and the newValue. Returns the previous value once successful.

			type Increment() { 
				if constexpr (isFloatingPoint) {
					return value.Add(1) + 1;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(1)) + 1;
				}
			} // atomically increments the value and returns the new value
			type Decrement() { 
				if constexpr (isFloatingPoint) {
					return value.Add(-1) - 1;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-1)) - 1;
				}
			} // atomically decrements the value and returns the new value
			type Add(const type& v) { 
				if constexpr (isFloatingPoint) {
					return value.Add(v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(v)) + v;
				}
			} // atomically adds a value and returns the new value
			type Sub(const type& v) {
				if constexpr (isFloatingPoint) {
					return value.Add(-v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-v)) - v;
				}
			}; // atomically subtracts a value and returns the new value
			type GetValue() const { 
				if constexpr (isFloatingPoint) {
					return value.load();
				}
				else {
					return value;
				}				
			}
			type SetValue(const type& v) {
				if constexpr (isFloatingPoint) {
					return value.Swap(v);
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting with the new value
			type SetValue(type&& v) {
				if constexpr (isFloatingPoint) {
					return value.Swap(std::forward<type>(v));
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting with the new value
		
        public: // std::atomic compatability
			type fetch_add(type const& v) {
				if constexpr (isFloatingPoint) {
					return value.fetch_add(v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while incrementing the actual counter
			type fetch_sub(type const& v) {
				if constexpr (isFloatingPoint) {
					return value.fetch_sub(v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-v));
				}
			}; // returns the previous value while decrementing the actual counter
			static constexpr bool is_lock_free() {
				if constexpr (isFloatingPoint) {
					return true; // thanks to the MwCAS algorithm
				}
				else {
					return true; // built-in to 99.99% of hardware nowadays
				}
			}; // returns whether a lock is utilized or the number is actually lock-free
			type exchange(type const& v) {
				if constexpr (isFloatingPoint) {
					return value.exchange(v);
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting the value to the input
			type load() const { return GetValue(); };
			void store(type const& v) {
				(void)exchange(v);
			}; // sets the value to the input

		public: // std::scoped_lock compatability
			inline void lock() {
				while (Increment() != static_cast<type>(1)) Decrement();
			};
			inline bool try_lock() {
				return Increment() == static_cast<type>(1);
			};
			inline void unlock() {
				Decrement();
			};

		private:
			internalType value;

		};
	};
	namespace containers {
		/* *THREAD SAFE* Thread- and Fiber-safe queue with Last-In-First-Out functionality.
		* POD types are stored by-value, non-POD types are stored as shared_ptr's.
		* POD types have a speed-up by avoiding destructor calls.
		* Example: [1,2]. Push(5) -> [1,2,5]. Pop(&Out) -> [1,2] & sets Out to 5.
		*/
		template <typename value> class Stack {
		private:
			static auto GetValueStorageType() {
				if constexpr (std::is_pod<value>::value) {
					value out{};
					return out;
				}
				else {
					auto out{ std::make_shared<value>() };
					return out;
				}
			};
			static auto MakeValueStorageType(value const& v) {
				if constexpr (std::is_pod<value>::value) {
					return static_cast<value const&>(v);
				}
				else {
					return std::make_shared<value>(v);
				}
			};
			static auto MakeValueStorageType(value&& v) {
				if constexpr (std::is_pod<value>::value) {
					return static_cast<value const&>(v);
				}
				else {
					return std::make_shared<value>(std::forward<value>(v));
				}
			};
			using ValueStorageType = typename fibers::utilities::function_traits<decltype(std::function(GetValueStorageType))>::result_type;
			static value& GetValueFromStorageType(ValueStorageType& v) {
				if constexpr (std::is_pod<value>::value) {
					return v;
				}
				else {
					return *v;
				}
			};

			/* storage class for each node in the stack. May or may not be POD, depending on the value type being stored. */
			class linkedListItem {
			public:
				ValueStorageType value; // may be copied-as-value or may be a shared_ptr. May or may not be POD. 
				synchronization::atomic_ptr<linkedListItem> prev; // ptr to the "prev" ptr. Non-POD but can be "forgotten" without penalty

				linkedListItem() : value(), prev(nullptr) {}
				linkedListItem(ValueStorageType const& mvalue) : value(mvalue), prev(nullptr) {}
				linkedListItem(linkedListItem const&) = default;
				linkedListItem(linkedListItem&&) = default;
				linkedListItem& operator=(linkedListItem const&) = default;
				linkedListItem& operator=(linkedListItem&&) = default;
				~linkedListItem() = default;
			};

			/* allocator is forced to use POD optimization if the value type is POD. */
			utilities::Allocator< linkedListItem > nodeAlloc;
			/* the current "tip" of the stack, which will be pop'd on request. */
			synchronization::atomic_ptr<linkedListItem> head;

		public:
			Stack() = default;
			Stack(Stack const&) = delete;
			Stack(Stack&&) = delete;
			Stack& operator=(Stack const&) = delete;
			Stack& operator=(Stack&&) = delete;
			~Stack() { clear(); };

			/**
			 * @brief current size of the list.
			 * @return number of items in list
			 */
			unsigned long size() const {
				return nodeAlloc.TotalAlive();
			};

			/**
			 * @brief clears the list.
			 * @return void
			 */
			void clear() {
				linkedListItem* p = head.Set(nullptr);
				linkedListItem* n = nullptr;

				while (p) {
					n = p->prev.Set(nullptr);
					nodeAlloc.Free(p);
					p = n;
					if (!p) // At end, lets check if new stuff came in
						p = head.Set(nullptr);
				}

				//nodeAlloc.Clean();
			};

			/**
			 * @brief pushes a copy of Value at the end of the list.
			 * @return true if Stack was previously empty, false otherwise.
			 */
			bool push(value const& Value) {
				auto* node = nodeAlloc.Alloc(); {
					node->value = MakeValueStorageType(Value);
				}

				while (true) {
					node->prev = head.load();
					if (head.CompareExchange(node->prev, node) == node->prev) {
						return node->prev == nullptr;
						// break;
					}
				}
			};

			/**
			 * @brief pushes a copy of Value at the end of the list.
			 * @return true if Stack was previously empty, false otherwise.
			 */
			bool push(value&& Value) {
				auto* node = nodeAlloc.Alloc(); {
					node->value = MakeValueStorageType(std::forward<value>(Value));
				}

				while (true) {
					node->prev = head.load();
					if (head.CompareExchange(node->prev, node) == node->prev) {
						return node->prev == nullptr;
						break;
					}
				}
			};

			/**
			 * @brief pushes a shared_ptr of the Value at the end of the list. Only available if value type is non-POD. (POD data are stored-by-value and a shared_ptr would not be respected)
			 * @return true if Stack was previously empty, false otherwise.
			 */
			template<typename = std::enable_if_t<!std::is_pod<value>::value>> bool push(std::shared_ptr<value> const& Value) {
				auto* node = nodeAlloc.Alloc(); {
					node->value = Value;
				}

				while (true) {
					node->prev = head.load();
					if (head.CompareExchange(node->prev, node) == node->prev) {
						return node->prev == nullptr;
						break;
					}
				}
			};

			/**
			 * @brief Searches for the first match with Value and removes it from the list.
			 * @return successful or not
			 */
			bool try_remove(value const& Value) {
				bool found = false;
				linkedListItem* next = nullptr;
				linkedListItem* current = head.load();
				if (current) {
					linkedListItem* prev = current->prev.load();
					while (!found && current) {
						if (GetValueFromStorageType(current->value) == Value) {
							// found it
							if (next) {
								// set it's next->prev to prev, not current
								if (next->prev.CompareExchange(current, prev) == current) {
									found = true;
									break;
								}
								else {
									// something interupted -- try again
									continue;
								}
							}
							else {
								// no "next" implies current was the head.
								if (head.CompareExchange(current, prev) == current) {
									found = true;
									break;
								}
								else {
									continue;
								}
							}
						}

						next = current;
						current = prev;
						if (current) {
							prev = current->prev.load();
						}
						else {
							prev = nullptr;
						}
					}
					if (found && current) {
						nodeAlloc.Free(current);
					}
				}
				return found;
			};

			/**
			 * @brief Trys to retrieve the item at the end of the list. If found, sets Out to that value.
			 * @return successful or not, and will set Out if successful
			 */
			bool try_pop(ValueStorageType& out) {
				linkedListItem* current{ head.load() };
				linkedListItem* prev{ nullptr };
				bool found = false;

				while (!found && current) {
					current = head.load();
					prev = current->prev.load();
					// no "next" implies current was the head.
					if (head.CompareExchange(current, prev) == current) {
						found = true;
						break;
					}
					else { // something interupted -- try again
						continue;
					}
				}
				if (found) {
					out = std::move(current->value);
					nodeAlloc.Free(current);
				}
				return found;
			};

			/**
			 * @brief Trys to remove the last (most recent) element from the list.
			 * @return successful or not
			 */
			bool try_pop() {
				linkedListItem* current{ head.load() };
				linkedListItem* prev{ nullptr };
				bool found = false;

				while (!found && current) {
					current = head.load();
					prev = current->prev.load();
					// no "next" implies current was the head.
					if (head.CompareExchange(current, prev) == current) {
						found = true;
						break;
					}
					else { // something interupted -- try again
						continue;
					}
				}
				if (found) {
					nodeAlloc.Free(current);
				}
				return found;
			};

			/**
			 * @brief Trys to remove the last (most recent) element from the list.
			 * @return successful or not
			 */
			bool try_pop_and_return_if_empty() {
				linkedListItem* current{ head.load() };
				linkedListItem* prev{ nullptr };
				bool found = false;
				bool isEmpty{ true };
				while (!found && current) {
					current = head.load();
					prev = current->prev.load();
					// no "next" implies current was the head.
					if (head.CompareExchange(current, prev) == current) {
						if (!prev) {
							isEmpty = true;
						}

						found = true;
						break;
					}
					else { // something interupted -- try again
						continue;
					}
				}
				if (found) {
					nodeAlloc.Free(current);
				}
				return isEmpty;
			};

			/**
			 * @brief Trys to retrieve the item at the end of the list. If found, sets Out to that value.
			 * @return successful or not, and will set Out if successful
			 */
			template<typename = std::enable_if_t<!std::is_pod<value>::value>> bool try_pop(value& out) {
				linkedListItem* current{ head.load() };
				linkedListItem* prev{ nullptr };
				bool found = false;

				while (!found && current) {
					current = head.load();
					prev = current->prev.load();
					// no "next" implies current was the head.
					if (head.CompareExchange(current, prev) == current) {
						found = true;
						break;
					}
					else { // something interupted -- try again
						continue;
					}
				}
				if (found) {
					out = std::move(*current->value);
					nodeAlloc.Free(current);
				}
				return found;
			};

			/**
			 * @brief counts how many times the Value is found in the list.
			 * @return number of matched items in list
			 */
			unsigned long count(value const& Value, unsigned long maxCount = std::numeric_limits<unsigned long>::max()) {
				linkedListItem* current = head.load();
				unsigned long counted{ 0 };
				while (current && (counted < maxCount)) {
					if (GetValueFromStorageType(current->value) == Value) {
						counted++;
					}
					current = current->prev.load();
				}
				return counted;
			};

			/**
			 * @brief Determines if the Value is in the list.
			 * @return whether or not the Value was found
			 */
			bool contains(value const& Value) {
				return count(Value, 1) > 0;
			};

		};
	};

	namespace utilities {
		/* 
		Allows user to queue ptrs for deletion or actions to take place during garbage collection phases. Garbage collection or deletion can be postponed until safe, utilizing scope guards. 
		If a thread is accessing a pointer and is at-risk for another thread deleting the pointer, then this GC tool may be a solution. Guarrantees that any ptr's queued for deletion will be deleted, even if this GC goes out-of-scope.
		\n
		CleanupFrequencyMilliseconds = will not perform GC if it was already performed within the last X milliseconds.
		*/
		template <uint64_t CleanupFrequencyMilliseconds = 1>
		class EpochGarbageCollector {
		private: // Support functions
			template <typename T> class Queue {
			public:
#if 1 // breaks so far, for reasons unknown. May be a bug with AtomicQueue.
				moodycamel::ConcurrentQueue<T> queue{};

				__forceinline void push(T&& item) {
					queue.push(std::forward<T>(item));
				};
				__forceinline void push(const T& item) {
					queue.push(item);
				};
				__forceinline bool try_pop(T& item) {
					return queue.try_pop(item);
				};
				__forceinline bool front(T& item) {
					return queue.front(item);
				};
#else
#if 0 // utilizes a lock-free design that guarrantees progress under heavy contention
				concurrency::concurrent_queue<T> queue{};

				__forceinline void push(T&& item) {
					queue.push(std::forward<T>(item));
				};
				__forceinline void push(const T& item) {
					queue.push(item);
				};
				__forceinline bool try_pop(T& item) {
					return queue.try_pop(item);
				};
				__forceinline bool front(T& item) = delete; // this function is not supported under this mode.
#else
				std::deque<T> queue{};
				fibers::synchronization::mutex locker{};

				__forceinline void push(T&& item) {
					std::scoped_lock lock(locker);
					queue.push_back(std::forward<T>(item));
				};
				__forceinline void push(const T& item) {
					std::scoped_lock lock(locker);
					queue.push_back(item);
				};
				__forceinline bool try_pop(T& item) {
					std::scoped_lock lock(locker);
					if (queue.empty()) return false;
					item = std::move(queue.front());
					queue.pop_front();
					return true;
				};
				__forceinline bool front(T& item) {
					std::scoped_lock lock(locker);
					if (queue.empty()) return false;
					item = std::move(queue.front());
					return true;
				};
				__forceinline bool try_pop_back(T& item) {
					std::scoped_lock lock(locker);
					if (queue.empty()) return false;
					item = std::move(queue.back());
					queue.pop_back();
					return true;
				};
				__forceinline bool back(T& item) {
					std::scoped_lock lock(locker);
					if (queue.empty()) return false;
					item = std::move(queue.back());
					return true;
				};
#endif
#endif
				Queue() = default;
				Queue(Queue const&) = default;
				Queue(Queue&&) = default;
				Queue& operator=(Queue const&) = default;
				Queue& operator=(Queue&&) = default;
				~Queue() = default;
			};
			
			static auto GetCurrentEpoch() {
				return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
			};

		private: // Using statements
			using EpochStorageType = typename fibers::utilities::function_traits<decltype(std::function(GetCurrentEpoch))>::result_type;
			using EpochDeleteType = std::function<void()>;
			using EpochQueueType = std::pair<EpochGarbageCollector::EpochStorageType, EpochDeleteType>;
			using IDManager = fibers::utilities::dbgroup::thread::IDManager;

		protected: // Reclamation Queue
			Queue< EpochQueueType> DeleteList;	// Queue of pointers to reclaim. We try to quickly keep the queue partially sorted. When unsorted, we move the first out-of-position item to the end and exit, to try again at a later date.
			bool Dequeue(EpochQueueType& out) { return DeleteList.try_pop(out); }; // Try to get a pointer for reclamation.
			void Enqueue(EpochQueueType&& obj) { DeleteList.push(std::forward<EpochQueueType>(obj)); }; // add a pointer or function for reclamation
			void Enqueue(EpochQueueType const& obj) { DeleteList.push(obj); }; // add a pointer or function for reclamation
			void Enqueue(EpochDeleteType&& obj) { DeleteList.push({ GetCurrentEpoch(), std::forward<EpochDeleteType>(obj) }); }; // add a pointer or function for reclamation

		private:
			class ThreadLocalStorage {
			public:
				ThreadLocalStorage() = default;
				~ThreadLocalStorage() = default;
				ThreadLocalStorage(ThreadLocalStorage const&) = delete;
				ThreadLocalStorage(ThreadLocalStorage&&) = delete;
				ThreadLocalStorage& operator=(ThreadLocalStorage const&) = delete;
				ThreadLocalStorage& operator=(ThreadLocalStorage&&) = delete;

				EpochGarbageCollector::EpochStorageType EpochLimit{ 0 }; // to be reclaimed if a pointer is older than this 
				fibers::synchronization::atomic_number<unsigned long long> Active{ 0 }; // incremented when a TLS is utilized. Should not (but is allowed to) exceed a value of 1. 
				EpochGarbageCollector* parent{ nullptr }; // set once without race condition, and never changed thereafter. 
			
			private:
				EpochGarbageCollector::EpochStorageType Epoch_3{ 0 }; // oldest Epoch
				EpochGarbageCollector::EpochStorageType Epoch_2{ 0 }; // middle Epoch
				EpochGarbageCollector::EpochStorageType Epoch_1{ 0 }; // youngest Epoch
				fibers::synchronization::atomic_number<long> StackLevel{ 0 };
				
				// 
				class EpochGuard {
				private:
					ThreadLocalStorage* _parent;
					EpochStorageType _CurrentEpoch;

				public:
					EpochGuard() = default;
					EpochGuard(ThreadLocalStorage* parent, EpochStorageType CurrentEpoch) :
						_parent{ parent }
						, _CurrentEpoch{ CurrentEpoch }
					{};
					EpochGuard(EpochGuard const&) = delete;
					EpochGuard(EpochGuard&&) = delete;
					EpochGuard& operator=(EpochGuard const&) = delete;
					EpochGuard& operator=(EpochGuard&&) = delete;
					~EpochGuard() {
						if (_parent->StackLevel.Decrement() == 0) {
							_parent->ForwardEpoch(_CurrentEpoch);
							_parent->parent->RunGC();
						}
					};
				};
				// forwards the current Epoch, and returns the epoch for which it is 100% safe to delete for (previous EpochLimit).
				EpochGarbageCollector::EpochStorageType ForwardEpoch(EpochGarbageCollector::EpochStorageType CurrentEpoch) {
					auto cas{ fibers::utilities::MultiItemCAS(&EpochLimit, &Epoch_1, &Epoch_2, &Epoch_3) };

					auto oldSwap = cas.Update([this, &CurrentEpoch](auto data) {
						auto& EpochLimit{ data.get<0>() };
						auto& Epoch_1{ data.get<1>() };
						auto& Epoch_2{ data.get<2>() };
						auto& Epoch_3{ data.get<3>() };

						// ptrs that are as old as Epoch_3 or older are now available for deleting
						EpochLimit = Epoch_3;
						Epoch_3 = Epoch_2;
						Epoch_2 = Epoch_1;
						Epoch_1 = CurrentEpoch;

						return data;
						});

					return oldSwap.get<0>();
				};

			public:
				[[nodiscard]] const auto ProtectCurrentEpoch() {
					if (StackLevel.Increment() == 1 && Active == 0) { Active++; }
					return EpochGuard(this, GetCurrentEpoch());
				};

			};
			// Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
			void RunGC() {
				EpochQueueType out{};
				EpochGarbageCollector::EpochStorageType _EpochLimit{ std::numeric_limits<EpochGarbageCollector::EpochStorageType>::max() };
				auto currentGC{ std::chrono::milliseconds(GetCurrentEpoch()) };
				static constexpr auto duration{ std::chrono::milliseconds(CleanupFrequencyMilliseconds) };

				if ((currentGC - std::chrono::milliseconds(lastGC.load())) > duration) {
					lastGC.SetValue(currentGC.count());

					for (auto& tls : TLS_arr) 
						if (tls.Active > 0) 
							_EpochLimit = std::min(_EpochLimit, fibers::utilities::MultiItemCAS(&tls.EpochLimit).Read<0>());
						
					if (_EpochLimit > 0) 
						while (Dequeue(out)) // while jobs are available...
							if (out.first < _EpochLimit) // ...if the data is in the correct time period for reclamation...
								out.second(); // ... then do the clean-up, and try again (hoping that the list is semi-sorted).
							else 
								return Enqueue(out); // ... otherwise push to the end of the queue, which is a form of lazy sorting (also prevents endless looping without additional checks / handles). 
				}
			};

		private:
			static constexpr size_t kMaxThreadNum = fibers::utilities::dbgroup::thread::kMaxThreadNum;
			static auto GetThreadID() { return IDManager::GetThreadID(); };
			std::array<ThreadLocalStorage, kMaxThreadNum> TLS_arr;
			fibers::synchronization::atomic_number<EpochStorageType> lastGC;
			auto& GetTLS() { return TLS_arr[GetThreadID()]; };

		public:
			EpochGarbageCollector() : TLS_arr{}, lastGC{ 0 }, DeleteList{} { for (auto& tls : TLS_arr) tls.parent = this; };
			EpochGarbageCollector(EpochGarbageCollector const&) = delete;
			EpochGarbageCollector(EpochGarbageCollector&&) = delete;
			EpochGarbageCollector& operator=(EpochGarbageCollector const&) = delete;
			EpochGarbageCollector& operator=(EpochGarbageCollector&&) = delete;
			~EpochGarbageCollector() { EpochQueueType out; while (Dequeue(out)) out.second(); };

		public:
			// Attempts to cleanup unused memory
			const auto TryCleanupUnusedMemory() { return RunGC(); };
			// Stalls deallocation / free calls made after this guard until this guard expires.
			[[nodiscard]] const auto CreateEpochGuard() {
				return GetTLS().ProtectCurrentEpoch();
			};
			// Queues a pointer for deletion, which may happen immediately or after some delay, depending on barriers and timing.
			template <class Target, typename = std::enable_if_t<!std::is_same_v<Target, void> > > void AddGarbage(const Target* garbage_ptr) {
				Enqueue([garbage_ptr]() {
					const Target* p = static_cast<const Target*>(garbage_ptr);
					delete p;
				});
			};
			// Queues a pointer for deletion, which may happen immediately or after some delay, depending on barriers and timing.
			template <class Target> void AddGarbage(const void* garbage_ptr) {
				Enqueue([garbage_ptr]() {
					const Target* p = static_cast<const Target*>(garbage_ptr);
					delete p;
				});
			};
			// Queues an action to take place during the collection process, including custom destruction calls. Can be blocked like normal by the Epoch guards.
			template <class Target = void> void AddCollectionAction(EpochDeleteType&& functor) {
				Enqueue(std::forward<EpochDeleteType>(functor));
			};
		};

		/* 
		Allocator (enables efficient use of memory) which also allows free / deletion operations to be postponed until safe, utilizing scope guards. If a thread is accessing a pointer and is at-risk for another thread deleting the pointer, then this allocator tool may be a solution. Guarrantees that any memory allocated will be cleaned-up, even if this allocator goes out-of-scope. 
		\n
		T = typename to be allocated.
		forcedSize = can be used to allocate larger blocks than just the sizeof(T). 
		forcePOD = can be used to guarrantee that the destructor is not called on each pointer, T. Speed-up when correctly used, otherwise can cause a memory leak.
		CleanupFrequencyMilliseconds = will not perform GC if it was already performed within the last X milliseconds.
		*/
		template <typename T, unsigned int forcedSize = sizeof(T), bool forcePOD = std::is_pod_v<T>, uint64_t CleanupFrequencyMilliseconds = 1>
		class GarbageCollectedAllocator {
		private:
			std::shared_ptr<EpochGarbageCollector<CleanupFrequencyMilliseconds>> _gc; // thread-safe, single-threaded GC which allows delaying the GC until after it is safe to reclame memory, and guarrantees reclamation of all queued data on destruction. 
			std::shared_ptr<fibers::utilities::Allocator<T, 256, forcePOD, forcedSize>> _alloc; // thread-safe allocator that guarrantees reclamation of data on close-out. 

		public:
			GarbageCollectedAllocator() : _gc(std::make_shared<EpochGarbageCollector<CleanupFrequencyMilliseconds>>()), _alloc(std::make_shared<fibers::utilities::Allocator<T, 256, forcePOD, forcedSize>>()) {};
			GarbageCollectedAllocator(GarbageCollectedAllocator const&) = default;
			GarbageCollectedAllocator(GarbageCollectedAllocator&&) = default;
			GarbageCollectedAllocator& operator=(GarbageCollectedAllocator const&) = default;
			GarbageCollectedAllocator& operator=(GarbageCollectedAllocator&&) = default;
			~GarbageCollectedAllocator() { // _gc *must* release first, _alloc afterwards. This odd code helps guarrantee the compiler won't re-arrange the code. 
				_alloc = std::static_pointer_cast<fibers::utilities::Allocator<T, 256, forcePOD, forcedSize>>(std::static_pointer_cast<void>(_gc = nullptr /* set to null, return result (null) */)) /* casts appropriate type. Is actually null, so this is OK. */;
			};

			// Request a new memory pointer. May be recovered from a previously-used location. Will be cleared and correctly initialized, if appropriate.
			template <typename... TArgs> T* Alloc(TArgs &&... a) {
				return this->_alloc->Alloc(std::forward<TArgs>(a)...);
			};

			// Frees the memory pointer, previously provided by this allocator. Calls the destructor for non-POD types, and will store the pointer for later use.
			void				Free(const T* element) {
				_gc->AddCollectionAction([this, element]() {
					this->_alloc->Free(element);
				});
			};
			size_t				size() const {
				return _alloc->TotalAlive();
			};

			// Request a new memory pointer that will self-delete and return to the memory pool automatically. Important: This allocator must out-live the shared_ptr.
			template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs &&... a) {
				return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { this->Free(p); });
			};

			// Stalls deallocation / free calls made after this guard until this guard expires.
			[[nodiscard]] const auto CreateEpochGuard() { return this->_gc->CreateEpochGuard(); };

			// Attempts to cleanup unused memory
			const auto TryCleanupUnusedMemory() { 
				this->_gc->TryCleanupUnusedMemory();
				this->_alloc->TryCleanupUnusedMemory();
			};

		};

		// bw tree
		namespace dbgroup::index::bw_tree
		{
			/**
			 * @brief A class for representing Bw-trees.
			 *
			 * @tparam Key a class of stored keys.
			 * @tparam Payload a class of stored payloads (only fixed-length data for simplicity).
			 * @tparam Comp a class for ordering keys.
			 */
			template <class Key,
				class Payload,
				class Comp = ::std::less<Key>
			>
				class BwTree
			{
			public:
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/
				using KeyWOPtr = std::remove_pointer_t<Key>;
				using PageID = uint64_t;
				using NodePage = component::NodePage;
				using DeltaPage = component::DeltaPage;
				using DeltaRC = component::DeltaRC;
				using DeltaType = component::DeltaType;
				using LogicalPtr = component::LogicalPtr;
				using NodeFixLen_t = component::fixlen::Node<Key, Comp>;
				using Node_t = NodeFixLen_t;
				using DeltaFixLen_t = component::fixlen::DeltaRecord<Key, Comp>;
				using Delta_t = DeltaFixLen_t;
				using Record = typename Delta_t::Record;
				using RecordIterator_t = component::RecordIterator<Key, Payload, Comp>;
				friend RecordIterator_t;  // call sibling scan from iterators
				using MappingTable_t = component::MappingTable<Node_t, Delta_t>;
				using DC = component::DeltaChain<Delta_t>;
				using ConsolidateInfo = std::pair<const void*, const void*>;
				using ScanKey = std::optional<std::tuple<const Key&, size_t, bool>>;

				template <class Entry>
				using BulkIter = typename std::vector<Entry>::const_iterator;
				using NodeEntry = std::tuple<Key, PageID, size_t>;
				using BulkResult = std::pair<size_t, std::vector<NodeEntry>>;
				using BulkPromise = std::promise<BulkResult>;
				using BulkFuture = std::future<BulkResult>;

				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct a new BwTree object.
				  *
				  * @param gc_interval_microsec GC internal [us] (default: 10ms).
				  * @param gc_thread_num the number of GC threads (default: 1).
				  */
				explicit BwTree(  //
					const size_t gc_interval_microsec = kDefaultGCTime,
					const size_t gc_thread_num = kDefaultGCThreadNum)
					: mapping_table_([this](LogicalPtr& lid) { this->DeallocateLogicalPtr(lid); })
				{
					// create an empty Bw-tree
					auto* root_node = new (GetNodePage()) Node_t{};
					auto root_id = mapping_table_.GetNewPageID();
					auto* root_ptr = mapping_table_.GetLogicalPtr(root_id);
					root_ptr->Store(root_node);
					root_.store(root_id, std::memory_order_relaxed);
				}

				BwTree(const BwTree&) = delete;
				BwTree(BwTree&&) = delete;

				auto operator=(const BwTree&)->BwTree & = delete;
				auto operator=(BwTree&&)->BwTree & = delete;

				void DeallocateLogicalPtr(LogicalPtr& lid) {
					return;
				};


				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the BwTree object.
				  *
				  */
				~BwTree() {};

				/*####################################################################################
				 * Public read APIs
				 *##################################################################################*/

				 /**
				  * @brief Read the payload corresponding to a given key if it exists.
				  *
				  * @param key a target key.
				  * @param key_len the length of the target key.
				  * @retval the payload of a given key wrapped with std::optional if it is in this tree.
				  * @retval std::nullopt otherwise.
				  */
				auto
					Read(  //
						const Key& key,
						[[maybe_unused]] const size_t key_len = sizeof(Key)) //
					-> std::optional<Payload>
				{
					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					// check whether the leaf node has a target key
					auto&& stack = SearchLeafNode(key, kClosed);

					for (Payload payload{}; true;) {
						// check whether the node is active and has a target key
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();

						uintptr_t out_ptr{};
						auto rc = DC::SearchRecord(head, key, out_ptr);
						switch (rc) {
						case DeltaRC::kRecordFound:
							payload = reinterpret_cast<Delta_t*>(out_ptr)->template GetPayload<Payload>();
							break;

						case DeltaRC::kRecordNotFound:
							break;

						case DeltaRC::kKeyIsInSibling:
							// swap a current node in a stack and retry
							stack.back() = out_ptr;
							continue;

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							SearchChildNode(key, kClosed, stack);
							continue;

						case DeltaRC::kReachBaseNode:
						default: {
							// search a target key in the base node
							const auto* node = reinterpret_cast<Node_t*>(out_ptr);
							std::tie(rc, out_ptr) = node->SearchRecord(key);
							if (rc == DeltaRC::kRecordFound) {
								payload = node->template GetPayload<Payload>(out_ptr);
							}
							break;
						}
						}

						if (rc == DeltaRC::kRecordNotFound) return std::nullopt;
						return payload;
					}
				}

				/**
				 * @brief Perform a range scan with given keys.
				 *
				 * @param begin_key a pair of a begin key and its openness (true=closed).
				 * @param end_key a pair of an end key and its openness (true=closed).
				 * @return an iterator to access scanned records.
				 */
				auto
					Scan(  //
						const ScanKey& begin_key = std::nullopt,
						const ScanKey& end_key = std::nullopt)   //
					-> RecordIterator_t
				{
					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();


					std::unique_ptr<void, std::function<void(void*)>> page{
						PageAllocator.Alloc(),
						[this](void* p) { PageAllocator.Free(static_cast<NodePage*>(p)); } 
					};

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};
					if (begin_key) {
						// traverse to a leaf node and sort records for scanning
						const auto& [b_key, b_key_len, b_closed] = *begin_key;
						auto&& stack = SearchLeafNode(b_key, b_closed);
						begin_pos = ConsolidateForScan(node, b_key, b_closed, stack);
					}
					else {
						Node_t* dummy_node = nullptr;
						// traverse to the leftmost leaf node directly
						auto&& stack = SearchLeftmostLeaf();
						while (true) {
							const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
							const auto* head = lptr->template Load<Delta_t*>();
							if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
							TryConsolidate(head, node, dummy_node, kIsScan);
							break;
						}
						begin_pos = 0;
					}

					// check the end position of scanning
					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);

					return RecordIterator_t{ this, node, begin_pos, end_pos, end_key, is_end };
				}

				auto 
					TryCleanupUnusedMemory() -> void {

					PageAllocator.TryCleanupUnusedMemory();
					DeltaAllocator.TryCleanupUnusedMemory();
					mapping_table_.TryCleanupUnusedMemory();
				};

				/**
				 * @brief Try to find the node the smallest key that is equal to or greater than the requested key. (e.g. search for 55.5, returns 56)
				 *
				 * @param key a key to search for.
				 * @return an iterator to access the scanned record.
				 */
				auto
					FindSmallestLargerEqual(const Key& key, std::optional<Key> const& maxKey = std::nullopt)
					-> RecordIterator_t
				{
					ScanKey begin_key(std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true })); // true = may include the value if found
					ScanKey end_key;
					if (maxKey.has_value()) {
						end_key.emplace(std::tuple<const Key&, size_t, bool>({ maxKey.value(), (size_t)(sizeof(Key)), true })); // true = may include the value if found
					}

					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					std::unique_ptr<void, std::function<void(void*)>> page{ 
						PageAllocator.Alloc(), 
						[this](void* p) { PageAllocator.Free(static_cast<NodePage*>(p)); }
					};

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					{
						// traverse to a leaf node and sort records for scanning
						const auto& [b_key, b_key_len, b_closed] = *begin_key;
						auto&& stack = SearchLeafNode(b_key, b_closed);
						begin_pos = ConsolidateForScan(node, b_key, b_closed, stack);

						RecordIterator_t record{ this, node, begin_pos, begin_pos + 1, std::nullopt, true };

						if (record && record.GetKey() >= key) { // success. Accounts for ~ 95% - 99% of cases. 
							// check the end position of scanning
							const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
							if (end_key.has_value()) {
								record.end_key_.emplace(end_key.value());
							}
							record.is_end_ = is_end;
							record.rec_count_ = end_pos;
							return record;
						}
						else {
							// We are at the end of a node, and the solution is either on the next node, or the solution does not exist. 
							if (record.node_->has_high_key_) {
								// go to the next sibling/node and continue scanning
								const auto& next_key = record.node_->GetHighKey();
								const auto sib_pid = record.node_->template GetNext<PageID>();
								record = this->SiblingScan(sib_pid, record.node_, next_key, std::nullopt);

								//record.bw_tree_ = this;
								RecordIterator_t record2{ record };
								while (record && record.GetKey() < key) {
									record++;
									if (record) {
										record2++;
									}
								}

								if (record) {
									const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
									if (end_key.has_value()) {
										record2.end_key_.emplace(end_key.value());
									}
									record2.is_end_ = is_end;
									record2.rec_count_ = end_pos;
								} // otherwise we are already at the end and nothing can be done -- no need to search.
								return record2;
							}
							else {
								// this happens when we are out-of-bounds for the search region and the desired key is larger than our highest available key.
								// Because we attempted to find a key that was out-of-bounds, the "node" we got is slightly broken, and must be repaired by re-searching:
								auto lowKey = node->GetLowKey();

								node = new (page.get()) Node_t{};
								// traverse to a leaf node and sort records for scanning
								auto&& stack2 = SearchLeafNode(lowKey, b_closed);
								begin_pos = ConsolidateForScan(node, lowKey, b_closed, stack2);

								const auto [is_end, end_pos] = node->SearchEndPositionFor(std::nullopt);

								record = RecordIterator_t{ this, node, begin_pos, end_pos, std::nullopt, is_end };
								RecordIterator_t record2{ record };
								while (record && record.GetKey() < key) {
									record++;
									if (record) {
										record2++;
									}
								}
								return record2;
							}
						}
					}
				};

				/**
				 * @brief Try to find the first, smallest node in the tree.
				 *
				 * @return an iterator to access the first, smallest keyed record.
				 */
				auto
					First()
					-> RecordIterator_t
				{
					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					std::unique_ptr<void, std::function<void(void*)>> page{ 
						PageAllocator.Alloc(), 
						[this](void* p) { PageAllocator.Free(static_cast<NodePage*>(p)); } 
					};

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					Node_t* dummy_node = nullptr;
					// traverse to the leftmost leaf node directly
					auto&& stack = SearchLeftmostLeaf();
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}
					begin_pos = 0;

					const auto [is_end, end_pos] = node->SearchEndPositionFor(std::nullopt);

					return RecordIterator_t{ this, node, begin_pos, end_pos, std::nullopt, is_end };
				};

				/**
				 * @brief Try to find the node the largest key that is equal to or less than the requested key. (e.g. search for 55.5, returns 55)
				 *
				 * @param key a key to search for.
				 * @return an iterator to access the scanned record.
				 */
				auto FindLargestSmallerEqual(const Key& key, std::optional<Key> const& maxKey = std::nullopt)
					-> RecordIterator_t
				{
					ScanKey keyFind{ std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true }) };
					ScanKey end_key;
					if (maxKey.has_value()) {
						end_key.emplace(std::tuple<const Key&, size_t, bool>({ maxKey.value(), (size_t)(sizeof(Key)), true })); // true = may include the value if found
					}

					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					std::unique_ptr<void, std::function<void(void*)>> page{ 
						PageAllocator.Alloc(), 
						[this](void* p) { PageAllocator.Free(static_cast<NodePage*>(p)); } 
					};

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					Node_t* dummy_node = nullptr;
					// traverse to the leftmost leaf node directly
					auto&& stack = SearchLeftmostLeaf();
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}
					begin_pos = 0;

					RecordIterator_t record{ this, node, begin_pos, begin_pos + 1, std::nullopt, true }; // first "node" in the entire tree. If this is not valid, we are hopeless.
					RecordIterator_t FinalRecord{ record };
					if (!record) { return record; }

					while (record) {
						const auto [is_end, end_pos] = record.node_->SearchEndPositionFor(keyFind);
						if (is_end) { // indicates that a node larger than ours exists on this node.
							if (end_pos > 0) {
								FinalRecord = RecordIterator_t{ this, record.node_, end_pos - 1, end_pos, std::nullopt, true };
								break;
							}
							else {
								// end_pos was 0, meaning the best value was the last one!
								break;
							}
						}
						else {
							// indicates that there will be another node after this, likely with a node larger than ours. BUT it could be the first item in that new node, so we capture the last item in this one just in case.
							FinalRecord = RecordIterator_t{ this, record.node_, end_pos, end_pos + 1, std::nullopt, true };

							// go to the next sibling/node and continue scanning
							const auto& next_key = record.node_->GetHighKey();
							const auto sib_pid = record.node_->template GetNext<PageID>();
							record = this->SiblingScan(sib_pid, record.node_, next_key, keyFind);
						}
					}

					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
					if (end_key.has_value()) {
						FinalRecord.end_key_.emplace(end_key.value());
					}
					FinalRecord.is_end_ = is_end;
					FinalRecord.rec_count_ = end_pos;

					return FinalRecord;
				};

				/**
				 * @brief Try to find the node the largest key that is equal to or less than the requested key. (e.g. search for 55.5, returns 55)
				 *
				 * @param key a key to search for.
				 * @return an iterator to access the scanned record.
				 */
				auto FindSecondLargestSmallerEqual(const Key& key, std::optional<Key> const& maxKey = std::nullopt) -> RecordIterator_t {
					ScanKey keyFind{ std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true }) };
					ScanKey end_key;
					if (maxKey.has_value()) {
						end_key.emplace(std::tuple<const Key&, size_t, bool>({ maxKey.value(), (size_t)(sizeof(Key)), true })); // true = may include the value if found
					}

					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					std::unique_ptr<void, std::function<void(void*)>> page{
						PageAllocator.Alloc(),
						[this](void* p) { PageAllocator.Free(static_cast<NodePage*>(p)); }
					};

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					Node_t* dummy_node = nullptr;
					// traverse to the leftmost leaf node directly
					auto&& stack = SearchLeftmostLeaf();
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}
					begin_pos = 0;

					RecordIterator_t record{ this, node, begin_pos, begin_pos + 1, std::nullopt, true }; // first "node" in the entire tree. If this is not valid, we are hopeless.
					RecordIterator_t FinalRecord{ record };
					if (!record) { return record; }

					while (record) {
						const auto [is_end, end_pos] = record.node_->SearchEndPositionFor(keyFind);
						if (is_end) { // final node
							if (end_pos > 1) { // 
								FinalRecord = RecordIterator_t{ this, record.node_, end_pos - 2, end_pos - 1, std::nullopt, true };
								break;
							}
							else {
								// end_pos was 0 or 1, meaning the best value was the last available one!
								break;
							}
						}
						else {
							// indicates that there will be another node after this, likely with a node larger than ours. BUT it could be the first item in that new node, so we capture the last item in this one just in case.
							if (end_pos == 0)
								FinalRecord = RecordIterator_t{ this, record.node_, end_pos, end_pos + 1, std::nullopt, true };
							else 
								FinalRecord = RecordIterator_t{ this, record.node_, end_pos - 1, end_pos, std::nullopt, true };

							// go to the next sibling/node and continue scanning
							const auto& next_key = record.node_->GetHighKey();
							const auto sib_pid = record.node_->template GetNext<PageID>();
							record = this->SiblingScan(sib_pid, record.node_, next_key, keyFind);
						}
					}

					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
					if (end_key.has_value()) {
						FinalRecord.end_key_.emplace(end_key.value());
					}
					FinalRecord.is_end_ = is_end;
					FinalRecord.rec_count_ = end_pos;

					return FinalRecord;
				};

				/**
				 * @brief Try to find the node the largest key that is equal to or less than the requested key. (e.g. search for 55.5, returns 55)
				 *
				 * @param key a key to search for.
				 * @return an iterator to access the scanned record.
				 */
				auto FindNthLargestSmallerEqual(const Key& key, int N, std::optional<Key> const& maxKey = std::nullopt) -> RecordIterator_t {
					ScanKey keyFind{ std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true }) };
					ScanKey end_key;
					if (maxKey.has_value()) {
						end_key.emplace(std::tuple<const Key&, size_t, bool>({ maxKey.value(), (size_t)(sizeof(Key)), true })); // true = may include the value if found
					}

					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					std::unique_ptr<void, std::function<void(void*)>> page{
						PageAllocator.Alloc(),
						[this](void* p) { PageAllocator.Free(static_cast<NodePage*>(p)); }
					};

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					Node_t* dummy_node = nullptr;
					// traverse to the leftmost leaf node directly
					auto&& stack = SearchLeftmostLeaf();
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}
					begin_pos = 0;

					RecordIterator_t record{ this, node, begin_pos, begin_pos + 1, std::nullopt, true }; // first "node" in the entire tree. If this is not valid, we are hopeless.
					RecordIterator_t FinalRecord{ record };
					if (!record) { return record; }

					while (record) {
						const auto [is_end, end_pos] = record.node_->SearchEndPositionFor(keyFind);
						if (is_end) { // final node
							if (end_pos > 1) { // 
								FinalRecord = RecordIterator_t{ this, record.node_, (end_pos - 1) - (N-1), end_pos, std::nullopt, true };
								break;
							}
							else {
								// end_pos was 0 or 1, meaning the best value was the last available one!
								break;
							}
						}
						else {
							// indicates that there will be another node after this, likely with a node larger than ours. BUT it could be the first item in that new node, so we capture the last item in this one just in case.
							if (end_pos == 0)
								FinalRecord = RecordIterator_t{ this, record.node_, end_pos, end_pos + 1, std::nullopt, true };
							else
								FinalRecord = RecordIterator_t{ this, record.node_, end_pos - (N - 1), (end_pos + 1) - (N - 1), std::nullopt, true };

							// go to the next sibling/node and continue scanning
							const auto& next_key = record.node_->GetHighKey();
							const auto sib_pid = record.node_->template GetNext<PageID>();
							record = this->SiblingScan(sib_pid, record.node_, next_key, keyFind);
						}
					}

					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
					if (end_key.has_value()) {
						FinalRecord.end_key_.emplace(end_key.value());
					}
					FinalRecord.is_end_ = is_end;
					FinalRecord.rec_count_ = end_pos;

					return FinalRecord;
				};

				/*####################################################################################
				 * Public write APIs
				 *##################################################################################*/

				 /**
				  * @brief Write (i.e., put) a given key/payload pair. E.g. if not exist, insert, and if exists, overwrites.
				  *
				  * This function always overwrites a payload and can be optimized for that purpose;
				  * the procedure may omit the key uniqueness check.
				  *
				  * @param key a target key.
				  * @param payload a target payload.
				  * @param key_len the length of the target key.
				  * @param pay_len the length of the target payload.
				  * @return kSuccess.
				  */
				auto
					Write(  //
						const Key& key,
						const Payload& payload,
						const size_t key_len = sizeof(Key),
						[[maybe_unused]] const size_t pay_len = sizeof(Payload))  //
					-> ReturnCode
				{
					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					const auto rec_len = key_len + kPayLen + kMetaLen;

					void* recPage{ GetRecPage() };
					auto* write_d = new (recPage) Delta_t{ DeltaType::kInsert, key, key_len, payload };
					auto rc = kSuccess;
					while (true) {
						// check whether the target node includes incomplete SMOs
						const auto [head, existence] = GetHeadWithKeyCheck(key, stack);
						if (existence == DeltaRC::kRecordFound) {
							rc = kKeyExist;
							write_d->SetDeltaType(DeltaType::kModify);
							write_d->SetNext(head, 0);
						}
						else {
							rc = kSuccess;
							write_d->SetDeltaType(DeltaType::kInsert);
							write_d->SetNext(head, rec_len);
						}

						// try to insert the delta record
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, write_d)) break;
					}

					if (write_d->NeedConsolidation()) {
						TrySMOs(write_d, stack);
					}

					return rc;
				}

				/**
				 * @brief Insert a given key/payload pair.
				 *
				 * This function performs a uniqueness check on its processing. If the given key does
				 * not exist in this tree, this function inserts a target payload into this tree. If
				 * the given key exists in this tree, this function does nothing and returns kKeyExist.
				 *
				 * @param key a target key.
				 * @param payload a target payload.
				 * @param key_len the length of the target key.
				 * @param pay_len the length of the target payload.
				 * @retval kSuccess if inserted.
				 * @retval kKeyExist otherwise.
				 */
				auto
					Insert(  //
						const Key& key,
						const Payload& payload,
						const size_t key_len = sizeof(Key),
						[[maybe_unused]] const size_t pay_len = sizeof(Payload))  //
				{
					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					const auto rec_len = key_len + kPayLen + kMetaLen;
					auto* insert_d = new (GetRecPage()) Delta_t{ DeltaType::kInsert, key, key_len, payload };
					auto rc = kSuccess;
					while (true) {
						// check target record's existence and get a head pointer
						const auto [head, existence] = GetHeadWithKeyCheck(key, stack);
						if (existence == DeltaRC::kRecordFound) {
							rc = kKeyExist;

							DeltaAllocator.Free((DeltaPage*)(void*)insert_d);

							break;
						}

						// try to insert the delta record
						insert_d->SetNext(head, rec_len);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, insert_d)) {
							if (insert_d->NeedConsolidation()) {
								TrySMOs(insert_d, stack);
							}
							break;
						}
					}

					return rc;
				}

				/**
				 * @brief Update the record corresponding to a given key with a given payload.
				 *
				 * This function performs a uniqueness check on its processing. If the given key
				 * exists in this tree, this function updates the corresponding payload. If the given
				 * key does not exist in this tree, this function does nothing and returns
				 * kKeyNotExist.
				 *
				 * @param key a target key.
				 * @param payload a target payload.
				 * @param key_len the length of the target key.
				 * @param pay_len the length of the target payload.
				 * @retval kSuccess if updated.
				 * @retval kKeyNotExist otherwise.
				 */
				auto
					Update(  //
						const Key& key,
						const Payload& payload,
						const size_t key_len = sizeof(Key),
						[[maybe_unused]] const size_t pay_len = sizeof(Payload))  //
				{
					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					auto* modify_d = new (GetRecPage()) Delta_t{ DeltaType::kModify, key, key_len, payload };
					auto rc = kSuccess;
					while (true) {
						// check target record's existence and get a head pointer
						const auto [head, existence] = GetHeadWithKeyCheck(key, stack);
						if (existence == DeltaRC::kRecordNotFound) {
							rc = kKeyNotExist;

							DeltaAllocator.Free((DeltaPage*)(void*)modify_d);

							break;
						}

						// try to insert the delta record
						modify_d->SetNext(head, 0);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, modify_d)) {
							if (modify_d->NeedConsolidation()) {
								TrySMOs(modify_d, stack);
							}
							break;
						}
					}

					return rc;
				}

				/**
				 * @brief Delete the record corresponding to a given key from this tree.
				 *
				 * This function performs a uniqueness check on its processing. If the given key
				 * exists in this tree, this function deletes it. If the given key does not exist in
				 * this tree, this function does nothing and returns kKeyNotExist.
				 *
				 * @param key a target key.
				 * @param key_len the length of the target key.
				 * @retval kSuccess if deleted.
				 * @retval kKeyNotExist otherwise.
				 */
				auto
					Delete(const Key& key,
						const size_t key_len = sizeof(Key))  //
					-> ReturnCode
				{
					[[maybe_unused]] const auto& guard2 = PageAllocator.CreateEpochGuard();
					[[maybe_unused]] const auto& guard3 = DeltaAllocator.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					const auto rec_len = key_len + kPayLen + kMetaLen;
					auto* delete_d = new (GetRecPage()) Delta_t{ key, key_len };
					auto rc = kSuccess;
					while (true) {
						// check target record's existence and get a head pointer
						auto [head, existence] = GetHeadWithKeyCheck(key, stack);
						if (existence == DeltaRC::kRecordNotFound) {
							rc = kKeyNotExist;

							DeltaAllocator.Free((DeltaPage*)(void*)delete_d);

							break;
						}

						// try to insert the delta record
						delete_d->SetNext(head, rec_len); // delete_d->SetNext(head, -rec_len);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, delete_d)) {
							if (delete_d->NeedConsolidation()) {
								TrySMOs(delete_d, stack);
							}
							break;
						}
					}

					return rc;
				}

				/*####################################################################################
				 * Public bulkload API
				 *##################################################################################*/

				 /**
				  * @brief Bulkload specified kay/payload pairs.
				  *
				  * This function loads the given entries into this index, assuming that the entries
				  * are given as a vector of key/payload pairs (or the tuples key/payload/key-length
				  * for variable-length keys). Note that keys in records are assumed to be unique and
				  * sorted.
				  *
				  * @tparam Entry a container of a key/payload pair.
				  * @param entries the vector of entries to be bulkloaded.
				  * @param thread_num the number of threads used for bulk loading.
				  * @return kSuccess.
				  */
				template <class Entry>
				auto
					Bulkload(  //
						const std::vector<Entry>& entries,
						const size_t thread_num = 1)  //
					-> ReturnCode
				{
					if (entries.empty()) return ReturnCode::kSuccess;

					std::vector<NodeEntry> nodes{};
					auto&& iter = entries.cbegin();
					const auto rec_num = entries.size();
					if (thread_num <= 1 || rec_num < thread_num) {
						// bulkloading with a single thread
						nodes = BulkloadWithSingleThread<Entry>(iter, rec_num).second;
					}
					else {
						// bulkloading with multi-threads
						std::vector<BulkFuture> futures{};
						futures.reserve(thread_num);

						// a lambda function for bulkloading with multi-threads
						auto loader = [&](BulkPromise p, BulkIter<Entry> iter, size_t n) {
							p.set_value(BulkloadWithSingleThread<Entry>(iter, n));
						};

						// create threads to construct partial BzTrees
						for (size_t i = 0; i < thread_num; ++i) {
							// create a partial BzTree
							BulkPromise p{};
							futures.emplace_back(p.get_future());
							const size_t n = (rec_num + i) / thread_num;
							std::thread{ loader, std::move(p), iter, n }.detach();

							// forward the iterator to the next begin position
							iter += n;
						}

						// wait for the worker threads to create partial trees
						std::vector<BulkResult> partial_trees{};
						partial_trees.reserve(thread_num);
						size_t height = 1;
						for (auto&& future : futures) {
							partial_trees.emplace_back(future.get());
							const auto partial_height = partial_trees.back().first;
							height = (partial_height > height) ? partial_height : height;
						}

						// align the height of partial trees
						nodes.reserve(kInnerNodeCap * thread_num);
						PageID prev_pid = kNullPtr;
						for (auto&& [p_height, p_nodes] : partial_trees) {
							while (p_height < height) {  // NOLINT
								p_nodes = ConstructSingleLayer<NodeEntry>(p_nodes.cbegin(), p_nodes.size(), kIsInner);
								++p_height;
							}
							nodes.insert(nodes.end(), p_nodes.begin(), p_nodes.end());

							// link partial trees
							Node_t::LinkVerticalBorderNodes(prev_pid, std::get<1>(p_nodes.front()), mapping_table_);
							prev_pid = std::get<1>(p_nodes.back());
						}
					}

					// create upper layers until a root node is created
					while (nodes.size() > 1) {
						nodes = ConstructSingleLayer<NodeEntry>(nodes.cbegin(), nodes.size(), kIsInner);
					}
					const auto new_pid = std::get<1>(nodes.front());
					Node_t::RemoveLeftmostKeys(new_pid, mapping_table_);

					// set a new root
					const auto old_pid = root_.exchange(new_pid, std::memory_order_release);
					auto* old_lptr = mapping_table_.GetLogicalPtr(old_pid);
					PageAllocator.Free<NodePage>(old_lptr->template Load<Delta_t*>());
					old_lptr->Clear();

					return ReturnCode::kSuccess;
				}

				/*####################################################################################
				 * Public utilities
				 *##################################################################################*/

				 /**
				  * @brief Collect statistical data of this tree.
				  *
				  * @retval 1st: the number of nodes.
				  * @retval 2nd: the actual usage in bytes.
				  * @retval 3rd: the virtual usage (i.e., reserved memory) in bytes.
				  */
				auto
					CollectStatisticalData()  //
					-> std::vector<std::tuple<size_t, size_t, size_t>>
				{
					std::vector<std::tuple<size_t, size_t, size_t>> stat_data{};
					const auto pid = root_.load(std::memory_order_acquire);

					CollectStatisticalData(pid, 0, stat_data);
					stat_data.emplace_back(mapping_table_.CollectStatisticalData());

					return stat_data;
				}

			private:
				/*####################################################################################
				 * Internal constants
				 *##################################################################################*/

				 /// an expected maximum height of a tree.
				static constexpr size_t kExpectedTreeHeight = 8;

				/// the NULL value for uintptr_t
				static constexpr uintptr_t kNullPtr = 0;

				/// the maximum length of keys.
				static constexpr size_t kMaxKeyLen = sizeof(Key);

				/// the length of payloads.
				static constexpr size_t kPayLen = sizeof(Payload);

				/// the length of child pointers.
				static constexpr size_t kPtrLen = sizeof(PageID);

				/// the length of record metadata.
				static constexpr size_t kMetaLen = 0;

				/// Header length in bytes.
				static constexpr size_t kHeaderLen = sizeof(Node_t);

				/// the maximum size of delta records.
				static constexpr size_t kDeltaRecSize = Delta_t::template GetMaxDeltaSize<Payload>();

				/// the expected length of keys for bulkloading.
				static constexpr size_t kBulkKeyLen = sizeof(Key);

				/// the expected length of records in leaf nodes for bulkloading.
				static constexpr size_t kLeafRecLen = kBulkKeyLen + kPayLen;

				/// the expected capacity of leaf nodes for bulkloading.
				static constexpr size_t kLeafNodeCap =
					(kPageSize - kHeaderLen - kBulkKeyLen) / (kLeafRecLen + kMetaLen);

				/// the expected length of records in internal nodes for bulkloading.
				static constexpr size_t kInnerRecLen = kBulkKeyLen + kPtrLen;

				/// the expected capacity of internal nodes for bulkloading.
				static constexpr size_t kInnerNodeCap =
					(kPageSize - kHeaderLen - kBulkKeyLen) / (kInnerRecLen + kMetaLen);

				/// a flag for preventing a consolidate-operation from splitting a node.
				static constexpr bool kIsScan = true;

				/// a flag for indicating leaf nodes.
				static constexpr bool kIsLeaf = false;

				/// a flag for indicating inner nodes.
				static constexpr auto kIsInner = true;

				/**
				 * @brief An internal enum for distinguishing a partial SMO status.
				 *
				 */
				enum SMOsRC {
					kConsolidate,
					kTrySplit,
					kTryMerge,
					kAlreadyConsolidated,
				};

				/*####################################################################################
				 * Internal utility functions
				 *##################################################################################*/

				 /**
				  * @brief Allocate or reuse a memory region for a base node.
				  *
				  * @returns the reserved memory page.
				  */
				[[nodiscard]] auto GetNodePage() -> Node_t* {
					return (Node_t*)(void*)PageAllocator.Alloc();
				};

				/**
				 * @brief Allocate or reuse a memory region for a delta record.
				 *
				 * @returns the reserved memory page.
				 */
				[[nodiscard]] auto GetRecPage() -> void* {
					return (void*)DeltaAllocator.Alloc();
				};

				/**
				 * @brief Add a given delta-chain to GC targets.
				 *
				 * If a given delta-chain has multiple delta records and base nodes, this function
				 * adds all of them to GC.
				 *
				 * @tparam T a templated class for simplicity.
				 * @param head the head pointer of a target delta-chain.
				 */
				template <class T>
				void
					AddToGC(const T* head)
				{
					static_assert(std::is_same_v<T, Node_t> || std::is_same_v<T, Delta_t>);

					// delete delta records
					const auto* garbage = reinterpret_cast<const Delta_t*>(head);
					while (garbage->GetDeltaType() != DeltaType::kNotDelta) {
						// register this delta record with GC
						DeltaAllocator.Free((DeltaPage*)(void*)garbage);

						// if the delta record is merge-delta, delete the merged sibling node
						if (garbage->GetDeltaType() == DeltaType::kMerge) {
							auto* removed_node = garbage->template GetPayload<Node_t*>();
							PageAllocator.Free((NodePage*)(void*)removed_node);
						}

						// check the next delta record or base node
						garbage = garbage->GetNext();
						if (garbage == nullptr) return;
					}

					// register a base node with GC
					PageAllocator.Free((const NodePage*)(const void*)reinterpret_cast<const Node_t*>(garbage));
				}

				/**
				 * @brief Collect statistical data recursively.
				 *
				 * @param pid the page ID of a target node.
				 * @param level the current level in the tree.
				 * @param stat_data an output statistical data.
				 */
				void
					CollectStatisticalData(  //
						const PageID pid,
						const size_t level,
						std::vector<std::tuple<size_t, size_t, size_t>>& stat_data)
				{
					// add an element for a new level
					if (stat_data.size() <= level) {
						stat_data.emplace_back(0, 0, 0);
					}

					// get the head of the current logical ID
					const auto* head = LoadValidHead(pid);
					while (head->GetDeltaType() == DeltaType::kRemoveNode) {
						head = LoadValidHead(pid);
					}

					// add statistical data of this node
					auto& [node_num, actual_usage, virtual_usage] = stat_data.at(level);
					const auto [node_size, delta_num] = head->GetNodeUsage();
					const auto delta_size = delta_num * kDeltaRecSize;
					++node_num;
					actual_usage += node_size + delta_size;
					virtual_usage += kPageSize + delta_size;

					// collect data recursively
					if (!head->IsLeaf()) {
						// consolidate the node to traverse child nodes
						auto* page = PageAllocator.Alloc();
						auto* consolidated = new (page) Node_t{ !kIsLeaf };
						Node_t* dummy_node = nullptr;
						TryConsolidate(head, consolidated, dummy_node, kIsScan);

						for (size_t i = 0; i < consolidated->GetRecordCount(); ++i) {
							const auto child_pid = consolidated->template GetPayload<PageID>(i);
							CollectStatisticalData(child_pid, level + 1, stat_data);
						}

						PageAllocator.Free(static_cast<NodePage*>(consolidated));
					}
				}

				/**
				 * @brief Search a child node of the top node in a given stack.
				 *
				 * @param key a search key.
				 * @param closed a flag for indicating closed/open-interval.
				 * @param stack a stack of traversed nodes.
				 * @param target_pid an optional node to prevent this function from searching a child.
				 * @retval true if the search for the target node is successful.
				 * @retval false otherwise.
				 */
				auto
					SearchChildNode(  //
						const Key& key,
						const bool closed,
						std::vector<PageID>& stack,
						const PageID target_pid = kNullPtr) const  //
					-> bool
				{
					for (uintptr_t out_ptr{}; true;) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						switch (DC::SearchChildNode(head, key, closed, out_ptr)) {
						case DeltaRC::kRecordFound:
							stack.emplace_back(out_ptr);
							break;

						case DeltaRC::kKeyIsInSibling: {
							// swap a current node in a stack and retry
							if (out_ptr == target_pid) return true;
							stack.back() = out_ptr;
							continue;
						}

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							if (stack.empty()) {
								if (target_pid != kNullPtr) return false;
								stack.emplace_back(root_.load(std::memory_order_relaxed));
							}
							else {
								if (SearchChildNode(key, closed, stack, target_pid)) return true;
								if (stack.empty()) return false;  // the tree structure has modified
							}
							continue;

						case DeltaRC::kReachBaseNode:
						default: {
							// search a child node in a base node
							const auto* node = reinterpret_cast<Node_t*>(out_ptr);
							stack.emplace_back(node->SearchChild(key, closed));
							break;
						}
						}

						return false;
					}
				}

				/**
				 * @brief Search a leaf node that may have a target key.
				 *
				 * @param key a search key.
				 * @param closed a flag for indicating closed/open-interval.
				 * @return a stack of traversed nodes.
				 */
				[[nodiscard]] auto
					SearchLeafNode(  //
						const Key& key,
						const bool closed) const  //
					-> std::vector<PageID>
				{
					std::vector<PageID> stack{};
					stack.reserve(kExpectedTreeHeight);
					stack.emplace_back(root_.load(std::memory_order_relaxed));

					// traverse a Bw-tree
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* node = lptr->template Load<Node_t*>();
						if (node->IsLeaf()) return stack;
						SearchChildNode(key, closed, stack);
					}
				}

				///**
				// * @brief Search a rightmost leaf node in this tree.
				// *
				// * @return a stack of traversed nodes.
				// */
				//[[nodiscard]] auto
				//	SearchRightmostLeaf() const  //
				//	-> std::vector<PageID>
				//{
				//	std::vector<PageID> stack{};
				//	stack.reserve(kExpectedTreeHeight);
				//	stack.emplace_back(root_.load(std::memory_order_relaxed));

				//	// traverse a Bw-tree
				//	while (true) {
				//		const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
				//		const Node_t* node = lptr->template Load<Node_t*>();
				//		if (node->IsLeaf()) break;
				//		stack.emplace_back(node->GetRightmostChild());
				//	}

				//	return stack;
				//}

				/**
				 * @brief Search a leftmost leaf node in this tree.
				 *
				 * @return a stack of traversed nodes.
				 */
				[[nodiscard]] auto
					SearchLeftmostLeaf() const  //
					-> std::vector<PageID>
				{
					std::vector<PageID> stack{};
					stack.reserve(kExpectedTreeHeight);
					stack.emplace_back(root_.load(std::memory_order_relaxed));

					// traverse a Bw-tree
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const Node_t* node = lptr->template Load<Node_t*>();
						if (node->IsLeaf()) break;
						stack.emplace_back(node->GetLeftmostChild());
					}

					return stack;
				}

				/**
				 * @brief Search a target node to trace a current node path.
				 *
				 * @param stack a stack of traversed nodes.
				 * @param key a search key.
				 * @param target_pid the page ID of a target node.
				 */
				void
					SearchTargetNode(  //
						std::vector<PageID>& stack,
						const Key& key,
						const PageID target_pid)
				{
					do {
						auto pid = root_.load(std::memory_order_relaxed);
						const auto* lptr = mapping_table_.GetLogicalPtr(pid);
						const auto* node = lptr->template Load<Node_t*>();
						stack.emplace_back(pid);

						while (!node->IsLeaf()) {
							if (SearchChildNode(key, kClosed, stack, target_pid)) return;
							if (stack.empty()) break;
							lptr = mapping_table_.GetLogicalPtr(stack.back());
							node = lptr->template Load<Node_t*>();
						}
					} while (stack.empty());
				}

				/**
				 * @brief Load a head of a delta chain in a given logical node.
				 *
				 * This function waits for other threads if the given logical node in SMOs.
				 *
				 * @param pid the page ID of a target node.
				 * @return a head of a delta chain.
				 */
				const Delta_t*
					LoadValidHead(const PageID pid)  //
				{
					const auto* lptr = mapping_table_.GetLogicalPtr(pid);
					while (true) {
						for (size_t i = 1; true; ++i) {
							const auto* head = lptr->template Load<Delta_t*>();
							if (!head->NeedWaitSMOs()) return head;
							if (i >= kRetryNum) break;
						}
						std::this_thread::sleep_for(kShortSleep);
					}
				}

				/**
				 * @brief Get the head pointer of a logical node.
				 *
				 * @param key a search key.
				 * @param closed a flag for indicating closed/open-interval.
				 * @param stack a stack of traversed nodes.
				 * @param target_pid an optional node to prevent this function from searching a head.
				 * @return the head of this logical node.
				 */
				auto
					GetHead(  //
						const Key& key,
						const bool closed,
						std::vector<PageID>& stack,
						const PageID target_pid = kNullPtr)  //
					-> const Delta_t*
				{
					for (uintptr_t out_ptr{}; true;) {
						// check whether the node is active and can include a target key
						const auto* head = LoadValidHead(stack.back());
						switch (DC::Validate(head, key, closed, out_ptr)) {
						case DeltaRC::kKeyIsInSibling: {
							// swap a current node in a stack and retry
							if (out_ptr == target_pid) return head;
							stack.back() = out_ptr;
							continue;
						}

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							if (stack.empty()) {
								if (target_pid != kNullPtr) return nullptr;
								stack = SearchLeafNode(key, closed);
							}
							else {
								SearchChildNode(key, closed, stack, target_pid);
								if (stack.empty()) return nullptr;
							}
							continue;

						case DeltaRC::kReachBaseNode:
						default:
							break;  // do nothing
						}

						return head;
					}
				}

				/**
				 * @brief Get the head pointer of a logical node and check key existence.
				 *
				 * @param key a search key.
				 * @param stack a stack of traversed nodes.
				 * @retval 1st: the head of this logical node.
				 * @retval 2nd: key existence.
				 */
				auto
					GetHeadWithKeyCheck(  //
						const Key& key,
						std::vector<PageID>& stack)  //
					-> std::pair<const Delta_t*, DeltaRC>
				{
					int maxDepth = 100000;

					for (uintptr_t out_ptr{}; --maxDepth >= 0;) {
						// check whether the node is active and has a target key
						const auto* head = LoadValidHead(stack.back());
						auto rc = DC::SearchRecord(head, key, out_ptr);
						switch (rc) {
						case DeltaRC::kRecordFound:
						case DeltaRC::kRecordNotFound:
							break;

						case DeltaRC::kKeyIsInSibling:
							// swap a current node in a stack and retry
							stack.back() = out_ptr;
							continue;

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							if (stack.empty()) {  // the tree structure has modified
								stack = SearchLeafNode(key, kClosed);
							}
							else {
								SearchChildNode(key, kClosed, stack);
							}
							continue;

						case DeltaRC::kReachBaseNode:
						default: {
							// search a target key in the base node
							rc = reinterpret_cast<Node_t*>(out_ptr)->SearchRecord(key).first;
							break;
						}
						}

						return { head, rc };
					}
					return { LoadValidHead(stack.back()), DeltaRC::kRecordNotFound };
				}

				/**
				 * @brief Get the head pointer of a logical node and check keys existence.
				 *
				 * @param key a search key.
				 * @param sib_key a separator key of a right-sibling node.
				 * @param stack a stack of traversed nodes.
				 * @retval 1st: the head of this logical node.
				 * @retval 2nd: key existence.
				 */
				auto
					GetHeadForMerge(  //
						const Key& key,
						const std::optional<Key>& sib_key,
						std::vector<PageID>& stack)  //
					-> std::pair<const Delta_t*, DeltaRC>
				{
					for (uintptr_t out_ptr{}; true;) {
						// check whether the node is active and has a target key
						const auto* head = LoadValidHead(stack.back());
						auto key_found = false;
						auto sib_key_found = !sib_key;
						auto rc = DC::SearchForMerge(head, key, sib_key, out_ptr, key_found, sib_key_found);
						switch (rc) {
						case DeltaRC::kRecordFound:
						case DeltaRC::kAbortMerge:
							break;

						case DeltaRC::kKeyIsInSibling:
							// swap a current node in a stack and retry
							stack.back() = out_ptr;
							continue;

						case DeltaRC::kNodeRemoved:
							rc = DeltaRC::kAbortMerge;
							break;

						case DeltaRC::kReachBaseNode:
						default: {
							const auto* node = reinterpret_cast<Node_t*>(out_ptr);
							if (!key_found) {
								if (node->SearchRecord(key).first == DeltaRC::kRecordFound) {
									key_found = true;
								}
							}
							if (!sib_key_found) {
								if (node->SearchRecord(*sib_key).first != DeltaRC::kRecordFound) {
									rc = DeltaRC::kAbortMerge;
									break;
								}
								sib_key_found = true;
							}
							rc = (key_found && sib_key_found) ? DeltaRC::kRecordFound : DeltaRC::kAbortMerge;
							break;
						}
						}

						return { head, rc };
					}
				}

				/*####################################################################################
				 * Internal scan utilities
				 *##################################################################################*/

				 /**
				  * @brief Perform consolidation for scanning.
				  *
				  * @param node a node page to store records.
				  * @param begin_key a search key.
				  * @param closed a flag for indicating closed/open-interval.
				  * @param stack a stack of traversed nodes.
				  * @return the begin position for scanning.
				  */
				auto
					ConsolidateForScan(  //
						Node_t*& node,
						const Key& begin_key,
						const bool closed,
						std::vector<PageID>& stack)  //
					-> size_t
				{
					Node_t* dummy_node = nullptr;

					while (true) {
						const auto* head = GetHead(begin_key, closed, stack);
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}

					// check the begin position for scanning
					const auto [rc, pos] = node->SearchRecord(begin_key);

					return (rc == DeltaRC::kRecordNotFound || closed) ? pos : pos + 1;
				}

				/**
				 * @brief Perform scanning with a given sibling node.
				 *
				 * @param sib_pid the page ID of a sibling node.
				 * @param node a node page to store records.
				 * @param begin_key a begin key (i.e., the highest key of the previous node).
				 * @param end_key an optional end key for scanning.
				 * @return the next iterator for scanning.
				 */
				auto
					SiblingScan(  //
						const PageID sib_pid,
						Node_t* node,
						const Key& begin_key,
						const ScanKey& end_key)  //
					-> RecordIterator_t
				{
					// consolidate a sibling node
					std::vector<PageID> stack{ sib_pid };
					stack.reserve(kExpectedTreeHeight);
					const auto begin_pos = ConsolidateForScan(node, begin_key, kClosed, stack);

					// check the end position of scanning
					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);

					return RecordIterator_t{ this, node, begin_pos, end_pos, is_end };
				}

				/*####################################################################################
				 * Internal structure modifications
				 *##################################################################################*/

				 /**
				  * @brief Create a temporary array for sorting delta records.
				  *
				  */
				static auto& CreateTempRecords() {
					thread_local static std::array<Record, kMaxDeltaRecordNum> arr{};
					std::memset(&arr, 0, sizeof(decltype(arr)));
					return arr;
				}

				/**
				 * @brief Try consolidation of a given node.
				 *
				 * This function will perform splitting/merging if needed.
				 *
				 * @param head a head delta record of a target delta chain.
				 * @param stack a stack of traversed nodes.
				 */
				void
					TrySMOs(  //
						Delta_t* head,
						std::vector<PageID>& stack)
				{
					std::unique_ptr<Node_t, std::function<void(void*)>> tls_node{ 
						nullptr, 
						[this](void* p) { PageAllocator.Free(static_cast<NodePage*>(p)); } 
					};
					Node_t* r_node = nullptr;

					// recheck other threads have modifed this delta chain
					auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
					if (head != lptr->template Load<Delta_t*>()) return;

					// prepare a consolidated node
					auto* new_node = (tls_node) ? tls_node.release() : GetNodePage();
					switch (TryConsolidate(head, new_node, r_node)) {
					case kTrySplit:
						// we use fixed-length pages, and so splitting a node must succeed
						Split(new_node, r_node, stack); // adds it to the mapping_table_?
						break;

					case kTryMerge:
						if (!TryMerge(head, new_node, stack)) {
							tls_node.reset(new_node); // tls_node should handle this
							return;
						}
						break;

					case kConsolidate:
					default:
						// install a consolidated node
						if (!lptr->CASStrong(head, new_node)) {
							tls_node.reset(new_node); // tls_node should handle this
							return;
						}
						break;
					}
					AddToGC(head);
				}

				/**
				 * @brief Consolidate a given node.
				 *
				 * @param head the head pointer of a terget node.
				 * @param new_node a node page to store consolidated records.
				 * @param r_node a node page to store split-right records.
				 * @param is_scan a flag to prevent a split-operation.
				 * @return the status of a consolidation result.
				 */
				auto
					TryConsolidate(  //
						const Delta_t* head,
						Node_t* new_node,
						Node_t*& r_node,
						const bool is_scan = false)  //
					-> SMOsRC
				{
					thread_local static std::vector<const void*> nodes{ kDeltaRecordThreshold, nullptr };
					nodes.clear(); 

					auto& records = CreateTempRecords();

					// sort delta records
					const auto rec_num = DC::Sort(head, records, nodes);

					// check whether splitting is needed
					const auto node_size = head->GetNodeSize();
					const auto do_split = !is_scan && node_size > kPageSize;

					// consolidate a target node
					const auto is_inner = !(head->IsLeaf());
					new (new_node) Node_t{ is_inner };
					if (do_split) {
						r_node = new (GetNodePage()) Node_t{ is_inner };
					}
					if (is_inner) {
						Consolidate<PageID>(new_node, r_node, nodes, records, rec_num, is_scan);
					}
					else {
						Consolidate<Payload>(new_node, r_node, nodes, records, rec_num, is_scan);
					}

					if (do_split) return kTrySplit;
					if (node_size <= kMinNodeSize) return kTryMerge;
					return kConsolidate;
				}

				/**
				 * @brief Consolidate given leaf nodes and delta records.
				 *
				 * @tparam T a class of expected payloads.
				 * @param new_node a node page to store consolidated records.
				 * @param r_node a node page to store split-right records.
				 * @param nodes the set of leaf nodes to be consolidated.
				 * @param arr insert/modify/delete-delta records.
				 * @param rec_num The number of delta records.
				 * @param is_scan a flag to prevent a split-operation.
				 */
				template <class T>
				void
					Consolidate(  //
						Node_t* new_node,
						Node_t* r_node,
						const std::vector<const void*>& nodes,
						const std::array<Record, kMaxDeltaRecordNum>& arr,
						const size_t new_rec_num,
						const bool is_scan)
				{
					constexpr auto kIsSplitLeft = true;
					auto* l_node = (r_node != nullptr) ? new_node : nullptr;

					// perform merge-sort to consolidate a node
					size_t offset = kPageSize * (is_scan ? 2 : 1);
					size_t j = 0;
					for (int64_t k = nodes.size() - 1; k >= 0; --k) {
						const auto* node = reinterpret_cast<const Node_t*>(nodes[k]);
						const auto node_rec_num = node->GetRecordCount();

						// check a null key for inner nodes
						size_t i = 0;
						if (!node->IsLeaf() && node->IsLeftmost()) {
							offset = Node_t::template CopyRecordFrom<T>(new_node, node, i++, offset, r_node);
						}
						for (; i < node_rec_num; ++i) {
							// copy new records
							const auto& node_key = node->GetKey(i);
							for (; j < new_rec_num && Comp{}(arr[j].key, node_key); ++j) {
								offset = Node_t::template CopyRecordFrom<T>(new_node, arr[j].ptr, offset, r_node);
							}

							// check a new record is updated one
							if (j < new_rec_num && !Comp{}(node_key, arr[j].key)) {
								offset = Node_t::template CopyRecordFrom<T>(new_node, arr[j++].ptr, offset, r_node);
							}
							else {
								offset = Node_t::template CopyRecordFrom<T>(new_node, node, i, offset, r_node);
							}
						}
					}

					// copy remaining new records
					for (; j < new_rec_num; ++j) {
						offset = Node_t::template CopyRecordFrom<T>(new_node, arr[j].ptr, offset, r_node);
					}

					// copy the lowest/highest keys
					if (l_node == nullptr) {
						// consolidated node
						offset = new_node->CopyLowKeyFrom(nodes.back());
						new_node->CopyHighKeyFrom(nodes.front(), offset);
					}
					else {
						// split nodes
						offset = l_node->CopyLowKeyFrom(nodes.back());
						l_node->CopyHighKeyFrom(new_node, offset, kIsSplitLeft);
						offset = new_node->CopyLowKeyFrom(new_node);
						new_node->CopyHighKeyFrom(nodes.front(), offset);
					}

					if (is_scan) {
						new_node->SetNodeSizeForScan();
					}
				}

				/**
				 * @brief Try splitting a target node.
				 *
				 * @param l_node a split-left node to be updated.
				 * @param r_node a split-right node to be inserted to this tree.
				 * @param stack a stack of traversed nodes.
				 */
				void
					Split(  //
						Node_t* l_node,
						const Node_t* r_node,
						std::vector<PageID>& stack)
				{
					// install the split nodes
					const auto r_pid = mapping_table_.GetNewPageID();
					auto* r_lptr = mapping_table_.GetLogicalPtr(r_pid);
					r_lptr->Store(r_node);
					l_node->SetNext(r_pid);
					const auto l_pid = stack.back();
					auto* l_lptr = mapping_table_.GetLogicalPtr(l_pid);
					l_lptr->Store(l_node);
					stack.pop_back();  // remove the split child node to modify its parent node

					// create an index-entry delta record to complete split
					const auto* r_node_d = reinterpret_cast<const Delta_t*>(r_node);
					auto* entry_d = new (GetRecPage()) Delta_t{ DeltaType::kInsert, r_node_d, r_pid };
					const auto& key = r_node->GetLowKey();
					const auto rec_len = r_node->GetLowKeyLen() + kPtrLen + kMetaLen;

					while (true) {
						// check the current node is a root node
						if (stack.empty()) {
							if (TryRootSplit(entry_d, l_pid)) {
								DeltaAllocator.Free((DeltaPage*)(void*)entry_d);
								return;
							}
							SearchTargetNode(stack, key, r_pid);
							stack.pop_back();  // remove the split node
							continue;
						}

						// insert the delta record into a parent node
						while (true) {
							const auto* head = GetHead(key, kClosed, stack, r_pid);
							if (head == nullptr) break;  // the tree structure has modified, so retry

							// try to insert the index-entry delta record
							entry_d->SetNext(head, rec_len);
							auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
							if (lptr->CASWeak(head, entry_d)) {
								if (entry_d->NeedConsolidation()) {
									TrySMOs(entry_d, stack);
								}
								return;
							}
						}
					}
				}

				/**
				 * @brief Perform splitting a root node.
				 *
				 * @param entry_d a insert-entry delta record.
				 * @param old_pid a logical node ID of an old root node.
				 * @retval true if splitting succeeds.
				 * @retval false otherwise.
				 */
				auto
					TryRootSplit(  //
						const Delta_t* entry_d,
						const PageID old_pid)  //
					-> bool
				{
					if (root_.load(std::memory_order_relaxed) != old_pid) return false;

					// create a new root node
					const auto* entry_delta_n = reinterpret_cast<const Node_t*>(entry_d);
					auto* new_root = new (GetNodePage()) Node_t{ entry_delta_n, old_pid };
					const auto new_pid = mapping_table_.GetNewPageID();
					mapping_table_.GetLogicalPtr(new_pid)->Store(new_root);

					// install a new root page
					root_.store(new_pid, std::memory_order_relaxed);
					return true;
				}

				/**
				 * @brief
				 *
				 * @param head the head pointer of a terget node.
				 * @param removed_node a removed (i.e., merged) node.
				 * @param stack a stack of traversed nodes.
				 * @retval true if partial merging succeeds.
				 * @retval false otherwise.
				 */
				auto
					TryMerge(  //
						const Delta_t* head,
						Node_t* removed_node,
						std::vector<PageID>& stack)  //
					-> bool
				{
					auto* removed_node_d = reinterpret_cast<Delta_t*>(removed_node);

					// insert a remove-node delta to prevent other threads from modifying this node
					auto* remove_d = new (GetRecPage()) Delta_t{ removed_node->IsLeaf() };
					const auto rem_pid = stack.back();
					auto* rem_lptr = mapping_table_.GetLogicalPtr(rem_pid);
					if (!rem_lptr->CASStrong(head, remove_d)) {
						DeltaAllocator.Free((DeltaPage*)(void*)remove_d);
						return false;
					}
					stack.pop_back();  // remove the child node

					// remove the index entry before merging
					const auto del_key_len = removed_node->GetLowKeyLen();
					const auto& del_key = component::DeepCopy<Key>(removed_node->GetLowKey(), del_key_len);
					auto* delete_d = TryDeleteIndexEntry(removed_node_d, del_key, del_key_len, stack);
					if (delete_d == nullptr) {
						// check this tree should be shrinked
						if (!TryRemoveRoot(removed_node, rem_pid, stack)) {
							// merging has failed, but consolidation succeeds
							rem_lptr->Store(removed_node);
							AddToGC(remove_d);
						}
						return true;
					}

					// insert a merge delta into the left sibling node
					const auto rem_uintptr = reinterpret_cast<uintptr_t>(removed_node);
					auto* merge_d = new (GetRecPage()) Delta_t{ DeltaType::kMerge, removed_node_d, rem_uintptr };
					const auto diff = removed_node->GetNodeDiff();
					while (true) {
						if (stack.empty()) {
							// concurrent SMOs have modified the tree structure, so reconstruct a stack
							SearchTargetNode(stack, del_key, rem_pid);
						}
						else {
							SearchChildNode(del_key, kOpen, stack, rem_pid);
							if (stack.empty()) continue;
						}

						while (true) {  // continue until insertion succeeds
							const auto* sib_head = GetHead(del_key, kOpen, stack, rem_pid);
							if (sib_head == nullptr) break;  // retry from searching the left sibling node

							// try to insert the merge-delta record
							merge_d->SetNext(sib_head, diff);
							auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
							if (lptr->CASWeak(sib_head, merge_d)) {
								delete_d->SetSiblingPID(stack.back());  // set a shortcut
								if (merge_d->NeedConsolidation()) {
									TrySMOs(merge_d, stack);
								}
								return true;
							}
						}
					}
				}

				/**
				 * @brief Complete partial merging by deleting index-entry from this tree.
				 *
				 * @param removed_node a consolidated node to be removed.
				 * @param del_key a lowest key of a removed node.
				 * @param del_key_len the length of the lowest key.
				 * @param stack a copied stack of traversed nodes.
				 * @retval the delete-delta record if successful.
				 * @retval nullptr otherwise.
				 */
				auto
					TryDeleteIndexEntry(  //
						const Delta_t* removed_node,
						const Key& del_key,
						const size_t del_key_len,
						std::vector<PageID> stack)  //
					-> Delta_t*
				{
					// check a current node can be merged
					if (stack.empty()) return nullptr;     // a root node cannot be merged
					if (del_key_len == 0) return nullptr;  // the leftmost nodes cannot be merged

					// insert the delta record into a parent node
					auto* delete_d = new (GetRecPage()) Delta_t{ removed_node };
					const auto rec_len = delete_d->GetKeyLength() + kPtrLen + kMetaLen;
					const auto& sib_key = removed_node->GetHighKey();
					while (true) {
						// check the removed node is not leftmost in its parent node
						auto [head, rc] = GetHeadForMerge(del_key, sib_key, stack);
						if (rc == DeltaRC::kAbortMerge) {
							// the leftmost nodes cannot be merged
							DeltaAllocator.Free((DeltaPage*)(void*)delete_d);
							return nullptr;
						}

						// try to insert the index-delete delta record
						delete_d->SetNext(head, rec_len); // delete_d->SetNext(head, -rec_len);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, delete_d)) break;
					}

					if (delete_d->NeedConsolidation()) {
						TrySMOs(delete_d, stack);
					}
					return delete_d;
				}

				/**
				 * @brief Remove a root node and shrink a tree.
				 *
				 * @param root an old root node to be removed.
				 * @param old_pid the page ID of an old root node.
				 * @param stack a stack of ancestor nodes.
				 * @retval true if a root node is removed.
				 * @return false otherwise.
				 */
				auto
					TryRemoveRoot(  //
						const Node_t* root,
						PageID old_pid,
						std::vector<PageID>& stack)  //
					-> bool
				{
					// check a given node can be shrinked
					if (!stack.empty() || root->GetRecordCount() > 1 || root->IsLeaf()) return false;

					// shrink the tree by removing a useless root node
					const auto new_pid = root->GetLeftmostChild();
					auto* new_lptr = mapping_table_.GetLogicalPtr(new_pid);
					if (new_lptr->template Load<Node_t*>()->IsLeaf()
						|| !root_.compare_exchange_strong(old_pid, new_pid, std::memory_order_relaxed)) {
						return false;
					}
					AddToGC(root);
					return true;
				}

				/*####################################################################################
				 * Internal bulkload utilities
				 *##################################################################################*/

				 /**
				  * @brief Bulkload specified kay/payload pairs with a single thread.
				  *
				  * Note that this function does not create a root node. The main process must create a
				  * root node by using the nodes constructed by this function.
				  *
				  * @tparam Entry a container of a key/payload pair.
				  * @param iter the begin position of target records.
				  * @param n the number of entries to be bulkloaded.
				  * @retval 1st: the height of a constructed tree.
				  * @retval 2nd: constructed nodes in the top layer.
				  */
				template <class Entry>
				auto
					BulkloadWithSingleThread(  //
						BulkIter<Entry> iter,
						const size_t n)  //
					-> BulkResult
				{
					// construct a data layer (leaf nodes)
					auto&& nodes = ConstructSingleLayer<Entry>(iter, n, kIsLeaf);

					// construct index layers (inner nodes)
					size_t height = 1;
					for (auto n = nodes.size(); n > kInnerNodeCap; n = nodes.size(), ++height) {
						// continue until the number of inner nodes is sufficiently small
						nodes = ConstructSingleLayer<NodeEntry>(nodes.cbegin(), n, kIsInner);
					}

					return { height, std::move(nodes) };
				}

				/**
				 * @brief Construct nodes based on given entries.
				 *
				 * @tparam Entry a container of a key/payload pair.
				 * @param iter the begin position of target records.
				 * @param n the number of entries to be bulkloaded.
				 * @param is_inner a flag for indicating inner nodes.
				 * @return constructed nodes.
				 */
				template <class Entry>
				auto
					ConstructSingleLayer(  //
						BulkIter<Entry> iter,
						const size_t n,
						const bool is_inner)  //
					-> std::vector<NodeEntry>
				{
					// reserve space for nodes in the upper layer
					std::vector<NodeEntry> nodes{};
					nodes.reserve((n / (is_inner ? kInnerNodeCap : kLeafNodeCap)) + 1);

					// load child nodes into parent nodes
					const auto& iter_end = iter + n;
					for (Node_t* prev_node = nullptr; iter < iter_end;) {
						auto* node = new (GetNodePage()) Node_t{ is_inner };
						const auto pid = mapping_table_.GetNewPageID();
						auto* lptr = mapping_table_.GetLogicalPtr(pid);
						lptr->Store(node);
						node->template Bulkload<Entry>(iter, iter_end, prev_node, pid, nodes, is_inner);
						prev_node = node;
					}

					return nodes;
				}

				/*####################################################################################
				 * Static assertions
				 *##################################################################################*/

				 /**
				  * @retval true if a target key class is trivially copyable.
				  * @retval false otherwise.
				  */
				[[nodiscard]] static constexpr auto
					KeyIsTriviallyCopyable()  //
					-> bool
				{
					// check a given key type is trivially copyable
					return std::is_trivially_copyable_v<Key>;
				}

				// target keys must be trivially copyable.
				static_assert(KeyIsTriviallyCopyable());

				// target payloads must be trivially copyable.
				static_assert(std::is_trivially_copyable_v<Payload>);

				// node pages have sufficient capacity for records.
				static_assert(kMaxKeyLen + kPayLen <= kPageSize / 4);

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// a root node of this Bw-tree.
				std::atomic_uint64_t root_{}; // holds a ptr, allocated elsewhere

				/// a table to map logical IDs with physical pointers.
				MappingTable_t mapping_table_{}; // allocates and destroys its own data. Logical ptrs provided to it, though, must be destroyed elsewhere.

				fibers::utilities::GarbageCollectedAllocator<NodePage, 2 * kPageSize, true, 1> PageAllocator{}; // allocator which provides memory re-use at runtime and memory clean-up on destruction
				fibers::utilities::GarbageCollectedAllocator<DeltaPage, kDeltaRecSize, true, 1> DeltaAllocator{}; // allocator which provides memory re-use at runtime and memory clean-up on destruction
			};
		};  // namespace dbgroup::index::bw_tree
	};

	namespace containers {
		enum class interp_t { LEFT, RIGHT, LINEAR, SPLINE };

		/* Fiber- and thread-safe lock-free sorted container for time-series patterns, where the x- and y-values may be integers, floating numbers, long doubles, etc. */
		template <typename xType, typename yType> class Pattern {
		protected:
			using underlying = fibers::utilities::dbgroup::index::bw_tree::BwTree< 
				typename utilities::impl::CAS_Safe_Type < xType >::type, 
				typename utilities::impl::CAS_Safe_Type < yType >::type 
			>;
			using dataType = std::pair< underlying , fibers::synchronization::atomic_number<long> >;
			std::shared_ptr< dataType > data;

		public:
			Pattern() : data(std::make_shared<dataType>()) {};
			Pattern(Pattern const& r) : data(std::make_shared<dataType>()) {
				operator=(r);
			}
			Pattern(Pattern&& r) = default;
			Pattern& operator=(Pattern const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					Pattern out;
					for (auto& x : r)
						out.Insert(x.first, x.second);
					out.data.swap(data);
				}
				return *this;
			};
			Pattern& operator=(Pattern&& r) = default;
			~Pattern() = default;

			void TryCleanupUnusedMemory() {
				data->first.TryCleanupUnusedMemory();
			};

			/// <summary>
			/// get iterator for the knot whose X-position is the smallest possible that is equal to or larger than the provided position. Optionally can provide the end-position for the iterator.
			/// </summary>
			/// <param name="position"></param>
			/// <param name="end"></param>
			/// <returns></returns>
			typename underlying::RecordIterator_t FindSmallestLargerEqual(xType position, std::optional<xType> const& end = std::nullopt) {
				if (end.has_value()) {
					return data->first.FindSmallestLargerEqual(position, end.value());
				}
				else {
					return data->first.FindSmallestLargerEqual(position);
				}
			};
			
			/// <summary>
			/// get iterator for the knot whose X-position is the largest possible that is less than or equal to than the provided position. Optionally can provide the end-position for the iterator.
			/// </summary>
			/// <param name="position"></param>
			/// <param name="end"></param>
			/// <returns></returns>
			typename underlying::RecordIterator_t FindLargestSmallerEqual(xType position, std::optional<xType> const& end = std::nullopt) {
				if (end.has_value()) {
					return data->first.FindLargestSmallerEqual(position, end.value());
				}
				else {
					return data->first.FindLargestSmallerEqual(position);
				}
			};

			/// <summary>
			/// get iterator for the knot whose X-position is the second-largest possible that is less than or equal to than the provided position. Optionally can provide the end-position for the iterator.
			/// </summary>
			/// <param name="position"></param>
			/// <param name="end"></param>
			/// <returns></returns>
			typename underlying::RecordIterator_t FindSecondLargestSmallerEqual(xType position, std::optional<xType> const& end = std::nullopt) {
				if (end.has_value()) {
					return data->first.FindSecondLargestSmallerEqual(position, end.value());
				}
				else {
					return data->first.FindSecondLargestSmallerEqual(position);
				}
			};

			/// <summary>
			/// get iterator for the knot whose X-position is the Nth-largest possible that is less than or equal to than the provided position. Optionally can provide the end-position for the iterator.
			/// N == 1 indicates the LEFT SNAP behavior.
			/// N == 2 indicates the iterator just left of that iterator returned with N==1..
			/// N == 3 indicates the iterator just left of that iterator returned with N==2...
			/// </summary>
			/// <param name="position"></param>
			/// <param name="end"></param>
			/// <returns></returns>
			typename underlying::RecordIterator_t FindNthLargestSmallerEqual(xType position, int N, std::optional<xType> const& end = std::nullopt) {
				if (end.has_value()) {
					return data->first.FindNthLargestSmallerEqual(position, N, end.value());
				}
				else {
					return data->first.FindNthLargestSmallerEqual(position, N);
				}
			};

			/// <summary>
			/// attempts to compress the pattern if there is a unecessary value stored in the final 5 slots.
			/// </summary>
			/// <returns>whether a value was deleted or not</returns>
			bool CompressLastValueAdded() {
				int num, index; 
				constexpr yType epsilon = 0.001f;
				yType val1{ 0 }, val2{ 0 }, val3{ 0 }, val4{ 0 }, val5{ 0 };
				xType t3{ 0 };

				{
					num = 0; 

					for (auto iter = FindNthLargestSmallerEqual(std::numeric_limits <xType>::max(), 5); iter; ++iter) {
						num++;
						if (!val1) {
							val1 = iter.GetPayload();
						}
						else if (!val2) {
							val2 = iter.GetPayload();
						}
						else if (!val3) {
							val3 = iter.GetPayload();
							t3 = iter.GetKey();
						}
						else if (!val4) {
							val4 = iter.GetPayload();
						}
						else if (!val5) {
							val5 = iter.GetPayload();
							break;
						}
					}

					if (num >= 5) {
						if (::abs(val1 - val2) <= epsilon &&
							::abs(val2 - val3) <= epsilon &&
							::abs(val3 - val4) <= epsilon &&
							::abs(val4 - val5) <= epsilon)
							return Delete(t3);
					}
				}

				return false;
			};
			
			/// <summary>
			/// Resets the pattern
			/// </summary>
			void Clear() {
				data.swap(std::make_shared<dataType>());
			};

			/// <summary>
			/// Attempts to read the y-value at the given x-position, if it exists.
			/// </summary>
			/// <param name="position"></param>
			/// <returns></returns>
			std::optional<yType> Read(xType position) const {
				auto payload = data->first.Read(position);
				if (payload.has_value()) {
					return (yType)payload.value();
				}
				else {
					return std::nullopt;
				}
			};

			/// <summary>
			/// Gets the minimum time, if any values exist.
			/// </summary>
			/// <returns></returns>
			std::optional<xType> GetMinTime() {
				auto iter = FindSmallestLargerEqual(-std::numeric_limits<xType>::max());
				if (iter) {
					return (xType)iter.GetKey(); // iter.GetPayload()
				}
				else {
					return std::nullopt;
				}
			};

			/// <summary>
			/// Gets the maximum time, if any values exist.
			/// </summary>
			/// <returns></returns>
			std::optional<xType> GetMaxTime() {
				auto iter = FindLargestSmallerEqual(std::numeric_limits<xType>::max());
				if (iter) {
					return (xType)iter.GetKey();
				}
				else {
					return std::nullopt;
				}
			};
			
			/// <summary>
			/// Iterates through the list and gets the 
			/// </summary>
			/// <returns></returns>
			size_t GetNumValues() const { return data->second.load(); };

			/// <summary>
			/// Finds the smallest Y-value in the pattern, if any knots exist.
			/// </summary>
			/// <param name="start"></param>
			/// <param name="end"></param>
			/// <returns></returns>
			std::optional<yType> GetMinValue(std::optional<xType> start = std::nullopt, std::optional<xType> end = std::nullopt) const {
				std::optional<yType> out{ std::nullopt };

				for (auto knot = const_cast<Pattern*>(this)->Scan(start, end); knot; ++knot) {
					if (out.has_value()){
						if (out.value() > knot.GetPayload())
							out = knot.GetPayload();
					}
					else {
						out = knot.GetPayload();
					}
				}

				return out;
			};

			/// <summary>
			/// Finds the largest Y-value in the pattern, if any knots exist.
			/// </summary>
			/// <param name="start"></param>
			/// <param name="end"></param>
			/// <returns></returns>
			std::optional<yType> GetMaxValue(std::optional<xType> start = std::nullopt, std::optional<xType> end = std::nullopt) const {
				std::optional<yType> out{ std::nullopt };
				
				for (auto knot = const_cast<Pattern*>(this)->Scan(start, end); knot; ++knot) {
					if (out.has_value()) {
						if (out.value() < knot.GetPayload())
							out = knot.GetPayload();
					}
					else {
						out = knot.GetPayload();
					}
				}

				return out;
			};
			
			std::optional<yType> GetCurrentValue(xType position, interp_t interpolationType = interp_t::LINEAR) const {
				switch (interpolationType) {
				case interp_t::LEFT: {
					if (auto iter = const_cast<Pattern*>(this)->FindLargestSmallerEqual(position)) {
						return iter.GetPayload();
					}
					break;
				}
				case interp_t::RIGHT: {
					if (auto iter = const_cast<Pattern*>(this)->FindSmallestLargerEqual(position)) {
						return iter.GetPayload();
					}
					break;
				}
				case interp_t::LINEAR: {
					if (auto iter = const_cast<Pattern*>(this)->FindLargestSmallerEqual(position)) {
						xType leftX = iter.GetKey();
						yType leftY = iter.GetPayload();

						++iter;
						if (iter) {
							xType rightX = iter.GetKey();
							yType rightY = iter.GetPayload();

							auto t = (position - leftX) / (rightX - leftX);

							return ::fma(t, rightY, ::fma(-t, leftY, leftY));
						}
						else {
							return leftY;
						}
					}
					break;
				}
				case interp_t::SPLINE: { 
					if (auto iter = const_cast<Pattern*>(this)->FindSecondLargestSmallerEqual(position)) {
						xType 
							X0{ iter.GetKey() }, 
							X1{ 0 }, 
							X2{ 0 }, 
							X3{ 0 },
							s{ 0 },
							t{ 0 };
						yType 
							Y0{ iter.GetPayload() }, 
							Y1{ 0 }, 
							Y2{ 0 }, 
							Y3{ 0 };

						++iter;
						if (iter) {
							X1 = iter.GetKey();
							Y1 = iter.GetPayload();

							++iter;
							if (iter) {
								X2 = iter.GetKey();
								Y2 = iter.GetPayload();

								++iter;
								if (iter) {
									X3 = iter.GetKey();
									Y3 = iter.GetPayload();

									if ((X1 <= position) && (X2 >= position)) {
										s = (position - X1) / (X2 - X1);
										if (!::isfinite(s)) s = 0;

										return 
											::fma(Y0, ((2.0f - s) * s - 1.0f) * s * 0.5f,          // -0.5f s * s * s + s * s - 0.5f * s
											::fma(Y1, (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f, // 1.5f * s * s * s - 2.5f * s * s + 1.0f
											::fma(Y2, ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f,  // -1.5f * s * s * s - 2.0f * s * s + 0.5f s
											::fma(Y3, ((s - 1.0f) * s * s) * 0.5f,                 // 0.5f * s * s * s - 0.5f * s * s
											      0))));
									}
								}
								else {
									X3 = X2 + (X2 - X1);
									t = (X3 - X1) / (X2 - X1);
									Y3 = ::fma(t, Y2, ::fma(-t, Y1, Y1));

									if ((X1 <= position) && (X2 >= position)) {
										s = (position - X1) / (X2 - X1);
										if (!::isfinite(s)) s = 0;

										return
											::fma(Y0, ((2.0f - s) * s - 1.0f) * s * 0.5f,          // -0.5f s * s * s + s * s - 0.5f * s
												::fma(Y1, (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f, // 1.5f * s * s * s - 2.5f * s * s + 1.0f
													::fma(Y2, ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f,  // -1.5f * s * s * s - 2.0f * s * s + 0.5f s
														::fma(Y3, ((s - 1.0f) * s * s) * 0.5f,                 // 0.5f * s * s * s - 0.5f * s * s
															0))));
									}
								}
							} 
							else {
								t = (position - X0) / (X1 - X0);
								return ::fma(t, Y1, ::fma(-t, Y0, Y0));
							}
						}
					}
					return GetCurrentValue(position, interp_t::LINEAR);
					break; 
				}		
				default: throw(std::runtime_error("Unhandled interp_t value."));
				}
				return std::nullopt;
			};

			/// <summary>
			/// Deletes the knot at the provided position, if it exists. Returns true if successful.
			/// </summary>
			/// <param name="position"></param>
			/// <returns></returns>
			bool Delete(xType position) {
				if (data->first.Delete(position) == fibers::utilities::dbgroup::index::bw_tree::ReturnCode::kSuccess) {
					data->second--;
					return true;
				}
				return false;				
			};

			/// <summary>
			/// Inserts a new knot into the pattern. if (overwiteIfExists), then it will overwrite the value if it is found to already exist. Returns true if added or overwritten.
			/// </summary>
			/// <param name="position"></param>
			/// <param name="value"></param>
			/// <param name="overwiteIfExists"></param>
			/// <returns></returns>
			bool Insert(xType position, yType value, bool overwiteIfExists = true) {
				if (overwiteIfExists) {
					if (data->first.Write(position, value) == fibers::utilities::dbgroup::index::bw_tree::ReturnCode::kKeyExist) {
						return true; // key exists but was overwritten
					}
					else {
						data->second++;
						return true; // key did not exist
					}
				}
				else {
					if (data->first.Insert(position, value) == fibers::utilities::dbgroup::index::bw_tree::ReturnCode::kSuccess) {
						data->second++;
						return true; // key did not exist
					}
				}
				return false;
			};
			
			/// <summary>
			/// Returns an iterator that can be used to iterate over the Pattern in-order, from (optional) start to (optional) end.
			/// </summary>
			/// <param name="start"></param>
			/// <param name="end"></param>
			/// <returns></returns>
			typename underlying::RecordIterator_t Scan(std::optional<xType> start = std::nullopt, std::optional<xType> end = std::nullopt) {
				using scanKeyType = typename underlying::ScanKey;
				if (start.has_value()) {
					if (end.has_value()) {
						return data->first.FindSmallestLargerEqual(start.value(), end.value());
					}
					else {
						return data->first.FindSmallestLargerEqual(start.value());
					}
				}
				else {
					if (end.has_value()) {
						return data->first.Scan(
							std::nullopt,
							scanKeyType(
								std::tuple<const typename utilities::impl::CAS_Safe_Type < xType >::type&, size_t, bool> {
							end.value(), sizeof(typename utilities::impl::CAS_Safe_Type < xType >::type), true
						}
						)
						);
					}
					else {
						return data->first.Scan();
					}
				}
			};

			struct Iterator : public std::iterator<std::forward_iterator_tag, std::pair<xType, yType>> {
			public:
				using difference_type = typename std::iterator<std::forward_iterator_tag, std::pair<xType, yType>>::difference_type;

				Iterator() = default;
				Iterator(Pattern*&& _parent, int pos, std::optional<xType> const& _start = std::nullopt, std::optional<xType> const& _end = std::nullopt) :
					start{ std::nullopt }
					, parent{ std::forward<Pattern*>(_parent) }
					, _ptr{ begin_impl(_parent, _start, _end) }
					, result{}
					, position{ pos }
				{
					if (_start.has_value()) start.emplace(_start.value());
					while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator(const Iterator& rhs) :
					start{ std::nullopt }
					, parent{ rhs.parent }
					, _ptr{ begin_impl(rhs.parent, rhs.start, rhs._ptr.GetEndKey<xType>()) }
					, result{}
					, position{ rhs.position }
				{
					if (rhs.start.has_value()) start.emplace(rhs.start.value());
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator(Iterator&& rhs) = default;
				Iterator& operator=(const Iterator& rhs) {
					start.reset();;
					parent = rhs.parent;
					_ptr = begin_impl(rhs.parent, rhs.start, rhs._ptr.GetEndKey<xType>());
					position = rhs.position;

					if (rhs.start.has_value()) start.emplace(rhs.start.value());
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator& operator=(Iterator&& rhs) {
					start.reset();;
					parent = rhs.parent;
					_ptr = begin_impl(rhs.parent, rhs.start, rhs._ptr.GetEndKey<xType>());
					position = rhs.position;

					if (rhs.start.has_value()) start.emplace(rhs.start.value());
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				~Iterator() = default;

				std::pair<xType, yType>& operator*() const { LoadResult(); return result; };
				std::pair<xType, yType>* operator->() const { LoadResult(); return &result; };

				Iterator& operator++() { Increment(); return *this; }

				explicit operator bool() const { return Valid(); };
				bool operator==(const Iterator& rhs) const {
					if (!rhs.Valid() && !Valid()) return true;
					if (!rhs.Valid()) {
						return !Valid();
					}
					if (!Valid()) {
						return !rhs.Valid();
					}
					return _ptr == rhs._ptr;
				}
				bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }

				Iterator begin() const { return Iterator(*this); };
				Iterator end() const { return Iterator(nullptr, 0, std::nullopt, std::nullopt); };

			private:
				bool Valid() const { return (bool)_ptr; };
				void Increment() { if (Valid()) ++_ptr; ++position; };
				void LoadResult() const { if (Valid()) result = { (xType)_ptr.GetKey(), (yType)_ptr.GetPayload() }; };

				static typename underlying::RecordIterator_t begin_impl(Pattern* parent, std::optional<xType> const& start = std::nullopt, std::optional<xType> const& end = std::nullopt) {
					if (parent) {
						if (start.has_value()) {
							if (end.has_value()) {
								return parent->Scan(start.value(), end.value());
							}
							else {
								return parent->Scan(start.value());
							}
						}
						else {
							if (end.has_value()) {
								return parent->Scan(std::nullopt, end.value());
							}
							else {
								return parent->Scan();
							}
						}
					}
					return typename underlying::RecordIterator_t();
				};

			public:
				std::optional<xType> start{ std::nullopt };
				Pattern* parent{ nullptr };
				mutable typename underlying::RecordIterator_t _ptr{};
				mutable std::pair<xType, yType> result{};
				int position{ 0 };

			};
			using iterator = Iterator;
			using const_iterator = Iterator;

			Iterator begin(std::optional<xType> const& start = std::nullopt, std::optional<xType> const& end = std::nullopt) const { return Iterator(const_cast<Pattern*>(this), 0, start, end); };
			Iterator end() const { return Iterator(nullptr, 0, std::nullopt, std::nullopt); };
			Iterator cbegin(std::optional<xType> start = std::nullopt, std::optional<xType> end = std::nullopt) const { return begin(start, end); };
			Iterator cend() const { return end(); };

			auto GetKnotSeries(std::optional<xType> const& start = std::nullopt, std::optional<xType> const& end = std::nullopt) const {
				if (start.has_value()) {
					if (end.has_value()) {
						return begin(start.value(), end.value());
					}
					else {
						return begin(start.value(), std::nullopt);
					}
				}
				else {
					if (end.has_value()) {
						return begin(std::nullopt, end.value());
					}
					else {
						return begin(std::nullopt, std::nullopt);
					}
				}
			};
			
			/// <summary>
			/// Creates an iterator that will step through from Start to End at interval Step, sampling the pattern using the InterpolationType. 
			/// </summary>
			/// <param name="start"></param>
			/// <param name="end"></param>
			/// <param name="step"></param>
			/// <param name="interpolationType"></param>
			/// <returns>Iterator that will sample at the requested interval using the interpolationType</returns>
			auto GetTimeSeries(xType start, xType end, xType step, interp_t interpolationType = interp_t::LINEAR) {
				return 
					fibers::utilities::CustomizedSequence<std::pair<xType, std::optional<yType>>, xType>(
						std::function([this, interpolationType](xType x) -> std::pair<xType, std::optional<yType>> {
							auto optional = this->GetCurrentValue(x, interpolationType);
							if (optional.has_value()) {
								return std::pair<xType, std::optional<yType>>{ x, optional.value() };
							}
							else {
								return std::pair<xType, std::optional<yType>>{ x, std::nullopt };
							}

							
						})
						, start
						, end
						, step
					);
			};



		};

		/* *THREAD SAFE* Thread-safe and fiber-safe sorted map. KeyType may be anything that can be hashed, and ObjType must be copy-by-value. */
		template <typename KeyType = std::string, typename ObjType = std::string, typename Hasher = std::hash<KeyType>> class Map {
		private:
			mutable fibers::utilities::GarbageCollectedAllocator<std::pair<KeyType, ObjType>>
				nodeAllocator;
			fibers::containers::Pattern < size_t, std::pair<KeyType, ObjType>*>
				sort;

		public:
			Map() = default;
			Map(Map const&) = default;
			Map(Map&&) = default;
			Map& operator=(Map const&) = default;
			Map& operator=(Map&&) = default;
			~Map() = default;

		public:
			/* emplaces the object at the key in the map. */
			bool emplace(KeyType const& key, ObjType const& object, bool overwriteIfExists = true) {
				size_t key_hash = Hasher()(key);
				std::pair<KeyType, ObjType>* newObj = nodeAllocator.Alloc(key, object);
				if (sort.Insert(key_hash, newObj, overwriteIfExists)) {
					return true;
				}
				else {
					nodeAllocator.Free(newObj);
					return false;
				}
			};
			/* emplaces the object at the key in the map. */
			bool emplace(KeyType const& key, ObjType&& object, bool overwriteIfExists = true) {
				size_t key_hash = Hasher()(key);
				std::pair<KeyType, ObjType>* newObj = nodeAllocator.Alloc(key, std::forward<ObjType>(object));
				if (sort.Insert(key_hash, newObj, overwriteIfExists)) {
					return true;
				}
				else {
					nodeAllocator.Free(newObj);
					return false;
				}
			};
			/* returns a COPY of the value at the key. Returns empty if the value is not found. */
			std::optional<ObjType> at(KeyType const& key) const {
				auto g{ nodeAllocator.CreateEpochGuard() };

				size_t key_hash = Hasher()(key);

				{
					std::optional<std::pair<KeyType, ObjType>*> optionalFind = sort.Read(key_hash);
					if (optionalFind.has_value()) {
						return optionalFind.value()->second;
					}
					else {
						return std::nullopt;
					}
				}
			};
			/* returns a COPY of the value at the hashed key. Returns empty if the value is not found.
			Allows user to calculate the hash externally from the Map. */
			std::optional<ObjType> at_hash(size_t const& key_hash) const {
				auto g{ nodeAllocator.CreateEpochGuard() };

				{
					std::optional<std::pair<KeyType, ObjType>*> optionalFind = sort.Read(key_hash);
					if (optionalFind.has_value()) {
						return optionalFind.value()->second;
					}
					else {
						return std::nullopt;
					}
				}
			};
			/* queues the key to be erased. Note that the erasure may be delayed depending on use of the map. */
			bool erase(KeyType const& key) {
				auto g{ nodeAllocator.CreateEpochGuard() };
				size_t key_hash = Hasher()(key);
				std::optional<std::pair<KeyType, ObjType>*> optionalFind = sort.Read(key_hash);
				if (optionalFind.has_value()) {
					nodeAllocator.Free(optionalFind.value());
					return sort.Delete(key_hash);
				}
				return false;
			};
			/* returns true if the key is in the map. */
			bool contains(KeyType const& key) const {
				return at(key).has_value();
			};
			/* returns the list of all keys currently in the map */
			std::vector<KeyType> keys() const {
				auto g{ nodeAllocator.CreateEpochGuard() };

				std::vector<KeyType> out;
				for (auto& x : sort) {
					out.push_back(x.second->first);
				}
				return out;
			};
			/* attempts to find the key associated to the provided object, if found. */
			std::optional<KeyType> key_of(ObjType const& obj) const {
				auto g{ nodeAllocator.CreateEpochGuard() };

				for (auto& x : sort) {
					if (x.second->second == obj) {
						return x.second->first;
					}
				}
				return std::nullopt;
			};

			size_t size() const {
				return nodeAllocator.size();
			};

			struct Iterator : public std::iterator<std::forward_iterator_tag, std::pair<KeyType, ObjType>> {
			public:
				using difference_type = typename std::iterator<std::forward_iterator_tag, std::pair<KeyType, ObjType>>::difference_type;

				Iterator() = default;
				Iterator(Map*&& _parent, int pos) :
					parent{ std::forward<Map*>(_parent) }
					, _ptr{ begin_impl(_parent) }
					, result{}
					, position{ pos }
				{
					while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator(const Iterator& rhs) :
					parent{ rhs.parent }
					, _ptr{ begin_impl(rhs.parent) }
					, result{}
					, position{ rhs.position }
				{
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator(Iterator&& rhs) = default;
				Iterator& operator=(const Iterator& rhs) {
					parent = rhs.parent;
					_ptr = begin_impl(rhs.parent);
					position = rhs.position;

					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator& operator=(Iterator&& rhs) {
					parent = rhs.parent;
					_ptr = begin_impl(rhs.parent);
					position = rhs.position;

					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				~Iterator() = default;

				std::pair<KeyType, ObjType>& operator*() const { LoadResult(); return result; };
				std::pair<KeyType, ObjType>* operator->() const { LoadResult(); return &result; };

				Iterator& operator++() { Increment(); return *this; }

				explicit operator bool() const { return Valid(); };
				bool operator==(const Iterator& rhs) const {
					if (!rhs.Valid() && !Valid()) return true;
					if (!rhs.Valid()) {
						return !Valid();
					}
					if (!Valid()) {
						return !rhs.Valid();
					}
					return _ptr == rhs._ptr;
				}
				bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }

				Iterator begin() const { return Iterator(*this); };
				Iterator end() const { return Iterator(nullptr, 0); };

			private:
				bool Valid() const { return (bool)_ptr; };
				void Increment() { if (Valid()) ++_ptr; ++position; };
				void LoadResult() const { if (Valid() && parent) { auto g{ parent->nodeAllocator.CreateEpochGuard() };  result = { (KeyType)_ptr->second->first, (ObjType)_ptr->second->second }; } };

				static typename decltype(sort)::Iterator begin_impl(Map* parent) {
					if (parent) {
						return parent->sort.begin();
					}
					return typename decltype(sort)::Iterator();
				};

			public:
				Map* parent{ nullptr };
				mutable typename decltype(sort)::Iterator _ptr{};
				mutable std::pair<KeyType, ObjType> result{};
				int position{ 0 };

			};
			using iterator = Iterator;
			using const_iterator = Iterator;

			Iterator begin() const { return Iterator(const_cast<Map*>(this), 0); };
			Iterator end() const { return Iterator(nullptr, 0); };
			Iterator cbegin() const { return begin(); };
			Iterator cend() const { return end(); };
		};

		/* *THREAD SAFE* Thread-safe and fiber-safe wrapper for any type of number, from integers to doubles.
		   Significant performance boost if the data type is an integer type or one of: long, unsigned int, unsigned long, unsigned __int64
		   Slower, but still atomic using multi-word CAS algorithms, if using floating-point numbers like doubles or floats.
		*/
		template<typename _Value_type> using number = fibers::synchronization::atomic_number<_Value_type>;
	
		template <typename value> class AtomicQueue {
		public:
			/* storage class for each node in the stack. May or may not be POD, depending on the value type being stored. */
			class linkedListItem {
			public:
				value Value; // may be copied-as-value or may be a shared_ptr. May or may not be POD. 
				linkedListItem* prev;
				linkedListItem* next;

				linkedListItem(value const& mvalue, linkedListItem* mprev = nullptr, linkedListItem* mnext = nullptr) : Value(mvalue), prev(mprev), next(mnext) {}
				linkedListItem() = default; 
				linkedListItem(linkedListItem const&) = default;
				linkedListItem(linkedListItem&&) = default;
				linkedListItem& operator=(linkedListItem const&) = default;
				linkedListItem& operator=(linkedListItem&&) = default;
				~linkedListItem() = default;
			};
			// GarbageCollected
			fibers::synchronization::shared_mutex<fibers::synchronization::mutex> deleteGuard;
			utilities::Allocator< linkedListItem, 128, std::is_pod<value>::value > nodeAlloc; /* the allocator, which supports deferred deletion when we want to access the data of a node that may have been deleted concurrently. */
			linkedListItem* head; /* the current "beginning" of the line, which will be pushed forward on request. */
			linkedListItem* tail; /* the current "end" of the line, which will be pop'd (e.g. somebody leaves the line) on request. */

			// try to add new element to the start of the line. May fail under competition with other threads.
			bool try_push(value const& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				auto* newNode = nodeAlloc.Alloc(element);
				if (true) {
					newNode->prev = newNode;
					newNode->next = newNode;

					linkedListItem* currentHead; 
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}
					if (currentHead && currentTail) {
						// there is already a head and tail. 
						linkedListItem* currentHeadNext; {
							auto container{ fibers::utilities::MultiItemCAS(
								&currentHead->next
							) };
							currentHeadNext = container.Read<0>();
						}

						if (currentHeadNext == currentHead) {
							// the head points to itself -- e.g. the list only has one element
							newNode->next = currentHead;
							newNode->prev = currentHead;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->next, // becomes newNode
								&currentHead->prev, // becomes newNode
								& tail // becomes currentHead
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentHead, currentHead, currentTail);
							if (container.TrySwap(OldValues, newNode, newNode, newNode, currentHead)) return true;
						}
						else {
							// the head points to something -- e.g. the list has two or more elements
							newNode->next = currentHead;
							newNode->prev = currentTail;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->prev, // becomes newNode
								& currentTail->next // becomes newNode
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentHead);
							if (container.TrySwap(OldValues, newNode, newNode, newNode)) return true;
						}
					}
					else {
						// if there is no head, try to swap the head and tail for the new node.
						fibers::utilities::Union< linkedListItem*, linkedListItem*> OldValues(nullptr, nullptr);
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						if (container.TrySwap(OldValues, newNode, newNode)) return true; 						
					}
				}
				nodeAlloc.Free(newNode); // easy replacement
				return false;
			}
			// add new element to the start of the line. Will always succeed, eventually. 
			bool push(value const& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				auto* newNode = nodeAlloc.Alloc(element);
				while (true) {
					newNode->prev = newNode;
					newNode->next = newNode;

					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}
					if (currentHead && currentTail) {
						// there is already a head and tail. 
						linkedListItem* currentHeadNext; {
							auto container{ fibers::utilities::MultiItemCAS(
								&currentHead->next
							) };
							currentHeadNext = container.Read<0>();
						}

						if (currentHeadNext == currentHead) {
							// the head points to itself -- e.g. the list only has one element
							newNode->next = currentHead;
							newNode->prev = currentHead;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->next, // becomes newNode
								&currentHead->prev, // becomes newNode
								&tail // becomes currentHead
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentHead, currentHead, currentTail);
							if (container.TrySwap(OldValues, newNode, newNode, newNode, currentHead)) return true;
						}
						else {
							// the head points to something -- e.g. the list has two or more elements
							newNode->next = currentHead;
							newNode->prev = currentTail;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->prev, // becomes newNode
								&currentTail->next // becomes newNode
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentHead);
							if (container.TrySwap(OldValues, newNode, newNode, newNode)) return true;
						}
					}
					else {
						// if there is no head, try to swap the head and tail for the new node.
						fibers::utilities::Union< linkedListItem*, linkedListItem*> OldValues(nullptr, nullptr);
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						if (container.TrySwap(OldValues, newNode, newNode)) return true;
					}
				}
				nodeAlloc.Free(newNode); // easy replacement
				return false;
			}
			// Remove the front element from line or return false if the list is empty. Will always succeed, eventually. 
			bool pop(value& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				while (true) {
					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}

					if (!currentHead || !currentTail) return false; // the list is empty
					
					linkedListItem* currentHeadPrev;
					linkedListItem* currentTailPrev; {
						auto container{ fibers::utilities::MultiItemCAS(
							&currentTail->prev,
							& currentHead->prev
						) };
						currentTailPrev = container.Read<0>();
						currentHeadPrev = container.Read<1>();
					}

					if (currentHead == currentTail) {
						// they are the same node, meaning the list has only one item. Clear it out and allow us to start fresh. 
						auto container{ fibers::utilities::MultiItemCAS(
							&head, // becomes nullptr
	                        &currentHead->prev, // becomes nullptr
							&currentTailPrev->next, // becomes nullptr
							&tail // becomes nullptr
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, nullptr, nullptr, nullptr, nullptr)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
					else {
						auto container{ fibers::utilities::MultiItemCAS(
							&tail, // becomes currentTailPrev
							&currentHead->prev, // becomes currentTailPrev
							&currentTailPrev->next // becomes currentHead
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, currentTailPrev, currentTailPrev, currentHead)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
				}

				return false;
			}
			// try to remove the front element from line. May fail under competition with other threads or if the list is empty.
			bool try_pop(value& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				if (true) {
					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}

					if (!currentHead || !currentTail) return false; // the list is empty

					linkedListItem* currentHeadPrev;
					linkedListItem* currentTailPrev; {
						auto container{ fibers::utilities::MultiItemCAS(
							&currentTail->prev,
							& currentHead->prev
						) };
						currentTailPrev = container.Read<0>();
						currentHeadPrev = container.Read<1>();
					}

					if (currentHead == currentTail) {
						// they are the same node, meaning the list has only one item. Clear it out and allow us to start fresh. 
						auto container{ fibers::utilities::MultiItemCAS(
							&head, // becomes nullptr
							&currentHead->prev, // becomes nullptr
							&currentTailPrev->next, // becomes nullptr
							&tail // becomes nullptr
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, nullptr, nullptr, nullptr, nullptr)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
					else {
						auto container{ fibers::utilities::MultiItemCAS(
							&tail, // becomes currentTailPrev
							&currentHead->prev, // becomes currentTailPrev
							&currentTailPrev->next // becomes currentHead
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, currentTailPrev, currentTailPrev, currentHead)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
				}

				return false;
			}
			// try to remove the front element from line if its value matches the element.
			bool try_remove_front_if(std::function<bool(value const&)> test) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };
				
				while (true) {
					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}

					if (!currentHead || !currentTail) return false; // the list is empty

					linkedListItem* currentHeadPrev;
					linkedListItem* currentTailPrev; {
						auto container{ fibers::utilities::MultiItemCAS(
							&currentTail->prev,
							& currentHead->prev
						) };
						currentTailPrev = container.Read<0>();
						currentHeadPrev = container.Read<1>();
					}

					if (currentHead == currentTail) {
						// they are the same node, meaning the list has only one item. Clear it out and allow us to start fresh. 
						auto container{ fibers::utilities::MultiItemCAS(
							&head, // becomes nullptr
							&currentHead->prev, // becomes nullptr
							&currentTailPrev->next, // becomes nullptr
							&tail // becomes nullptr
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentTail, currentTail);

						if (test(currentTail->Value)) {
							if (container.TrySwap(OldValues, nullptr, nullptr, nullptr, nullptr)) {
								auto guard{ std::shared_lock(deleteGuard) };
								nodeAlloc.Free(currentTail);
								return true;
							}
							else {
								// try again
							}
						}
						else {
							return false;
						}
					}
					else {
						auto container{ fibers::utilities::MultiItemCAS(
							&tail, // becomes currentTailPrev
							&currentHead->prev, // becomes currentTailPrev
							&currentTailPrev->next // becomes currentHead
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentTail, currentTail, currentTail);
						
						if (test(currentTail->Value)) {
							if (container.TrySwap(OldValues, currentTailPrev, currentTailPrev, currentHead)) {
								auto guard{ std::shared_lock(deleteGuard) };
								nodeAlloc.Free(currentTail);
								return true;
							}
							else {
								// try again
							}
						}
						else {
							return false;
						}
					}
				}

				return false;
			}

			bool front(value& Value) {
				//auto guard{ nodeAlloc.CreateEpochGuard() };
				auto guard{ std::scoped_lock(deleteGuard) };

				linkedListItem* currentTail; {
					auto container{ fibers::utilities::MultiItemCAS(
						&tail
					) };
					currentTail = container.Read<0>();
				}
				if (currentTail) {
					Value = currentTail->Value;
					return true;
				}
				else {
					return false;
				}
			};
		};
	};

	namespace impl {
		bool Initialize(uint32_t maxThreadCount = std::numeric_limits< uint32_t>::max());
		void ShutDown();

		/* job arguments used to perform work as part of a loop over a Task */
		struct JobArgs {
			uint32_t jobIndex;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
			uint32_t groupID;		// group index relative to dispatch (like SV_GroupID in HLSL)
			uint32_t groupIndex;	// job index relative to group (like SV_GroupIndex in HLSL)
			void* sharedmemory;		// stack memory shared within the current group (jobs within a group execute serially)
		};

		// Defines a state of execution, can be waited on
		struct context {
			synchronization::atomic_number<long> counter{ 0 }; // how many Tasks* are awaited
			synchronization::atomic_ptr<std::exception_ptr> e{ nullptr }; // shared error PTR for re-throwing at the end of the Tasks.
		};

		/* job arguments used to perform work as part of a loop over a Task */
		struct Task {
			std::function<void(JobArgs)> task;
			context* ctx;
			uint32_t groupID;
			uint32_t groupJobOffset;
			uint32_t groupJobEnd;
			uint32_t sharedmemory_size;
			std::function<void(void*)> GroupStartJob; // callback func with memory for type T
			std::function<void(void*)> GroupEndJob; // callback func with memory for type T
		};

		template <typename T> class Queue {
		public:
#if 1 // breaks so far, for reasons unknown. May be a bug with AtomicQueue.
			moodycamel::ConcurrentQueue<T> queue;
			// fibers::containers::AtomicQueue<T> queue;

			__forceinline void push(T&& item) {
				queue.push(std::forward<T>(item));
			};
			__forceinline void push(const T& item) {
				queue.push(item);
			};
			__forceinline bool try_pop(T& item) {
				return queue.try_pop(item);
			};
			__forceinline bool front(T& item) {
				return queue.front(item);
			};
#else
#if 0 // utilizes a lock-free design that guarrantees progress under heavy contention
			concurrency::concurrent_queue<T> queue;

			__forceinline void push(T&& item) {
				queue.push(std::forward<T>(item));
			};
			__forceinline void push(const T& item) {
				queue.push(item);
			};
			__forceinline bool try_pop(T& item) {
				return queue.try_pop(item);
			};
			__forceinline bool front(T& item) = delete; // this function is not supported under this mode.
#else
			std::deque<T> queue;
			fibers::synchronization::mutex locker;

			__forceinline void push(T&& item) {
				std::scoped_lock lock(locker);
				queue.push_back(std::forward<T>(item));
			};
			__forceinline void push(const T& item) {
				std::scoped_lock lock(locker);
				queue.push_back(item);
			};
			__forceinline bool try_pop(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.front());
				queue.pop_front();
				return true;
			};
			__forceinline bool front(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.front());				
				return true;
			};
			__forceinline bool try_pop_back(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.back());
				queue.pop_back();
				return true;
			};
			__forceinline bool back(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.back());
				return true;
			};
#endif
#endif
			Queue() = default;
			Queue(Queue const&) = default;
			Queue(Queue &&) = default;
			Queue& operator=(Queue const&) = default;
			Queue& operator=(Queue&&) = default;
			~Queue() = default;
		};

		struct InternalState {
			uint32_t numCores = 0;
			uint32_t numThreads = 0;

			std::unique_ptr<Queue<Task>[]> jobQueuePerThread;
			// std::unique_ptr<Task[]> currentTaskPerThread;

			fibers::containers::number<bool> alive{ true };
			std::condition_variable_any wakeCondition; // std::condition_variable wakeCondition; //  
			synchronization::mutex wakeMutex; // std::mutex wakeMutex; //  
			fibers::containers::number<long long> nextQueue{ 0 };
			std::vector<std::thread> threads;
			void ShutDown() {
				alive = false; // indicate that new jobs cannot be started from this point
				bool wake_loop = true;
				std::thread waker([&] {
					while (wake_loop) wakeCondition.notify_all(); // wakes up sleeping worker threads
				});
				for (auto& thread : threads) {
					thread.join();
				}
				wake_loop = false;
				waker.join();
				jobQueuePerThread.reset();
				// currentTaskPerThread.reset();
				threads.clear();
				numCores = 0;
				numThreads = 0;
			};
			~InternalState() {
				ShutDown();
			};
		} static internal_state;

		__forceinline uint32_t GetThreadCount() { return internal_state.numThreads; };

		// Add a task to execute asynchronously. Any idle thread will execute this.
		void Execute(context& ctx, std::function<void(JobArgs const&)> const& task) noexcept;

		// Divide a task onto multiple jobs and execute in parallel.
		//	jobCount	: how many jobs to generate for this task.
		//	groupSize	: how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
		//	task		: receives a JobArgs as parameter
		void Dispatch(
			context& ctx, 
			uint32_t jobCount, 
			std::function<void(JobArgs const&)> const& task
		) noexcept;

		void Dispatch(
			context& ctx,
			uint32_t jobCount,
			std::function<void(JobArgs const&)> const& task,
			size_t sharedmemory_size,
			std::function<void(void*)> const& GroupStartJob, // callback func with memory for type T
			std::function<void(void*)> const& GroupEndJob // callback func with memory for type T
		) noexcept;

		// Returns the amount of job groups that will be created for a set number of jobs and group size
		__forceinline constexpr uint32_t DispatchGroupCount(uint32_t jobCount, uint32_t groupSize) { return (jobCount + groupSize - 1) / groupSize; /* Calculate the amount of job groups to dispatch (overestimate, or "ceil"): */ };

		// Check if any threads are working currently or not
		bool IsBusy(const context& ctx);

		void HandleExceptions(context& ctx);

		// Wait until all threads become idle. Current thread will become a worker thread, executing jobs.
		void Wait(context& ctx);

		struct TaskGroup {
		private:
			context ctx{0, nullptr};

		public:
			auto Wait() { return impl::Wait(ctx); };
			auto IsBusy() const { return impl::IsBusy(ctx); };
			auto Queue(std::function<void(JobArgs const&)> const& task) { return impl::Execute(ctx, task); };
			
			/* Dispatch a function that does not need to share memory within a group / cluster of the Task jobs. */
			auto Dispatch(
				uint32_t jobCount,
				std::function<void(JobArgs const&)> const& task
			) {
				return impl::Dispatch(ctx, jobCount, task);
			};	
			
			/* Dispatch a function that intends to share memory serially within a group / cluster of the Task jobs. */
			template <typename T> auto Dispatch(
				uint32_t jobCount,
				std::function<void(JobArgs const&)> const& task,
				std::function<void(void*)> const& GroupStartJob, // callback func with memory for type T
				std::function<void(void*)> const& GroupEndJob // callback func with memory for type T
			){
				return impl::Dispatch(ctx, jobCount, task, sizeof(T), GroupStartJob, GroupEndJob);
			};
		};
	};

	class JobGroup;

	/*! Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wrapper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	auto __awaiter__ = Job([](){ return std::string("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, and guarrantees its completion before the scope ends. */
	class Job {
		friend JobGroup;
	protected:
		mutable std::shared_ptr<fibers::Action_Base> impl;

	private:
		// Scoped to return whetever the function desires / requires. Note: Can only be used in a "decltype" context, as it cannot actually do anything at all. 
		struct passepartout {
			template <typename T> operator T& ();
			template <typename T> operator T&& ();
		};

	private:
		template <typename T> static constexpr const bool IsStaticFunction() {
			return (std::is_pointer<T>::value && std::is_function<typename std::remove_pointer_t<T>>::value) || std::is_function<T>::value;
		};
		template <typename T> static constexpr const bool IsLambda() {
			if constexpr (IsStaticFunction<T>() || std::is_member_function_pointer<T>::value) {
				return false;
			}
			else if constexpr (std::is_invocable<T>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else return false;
		};
		template<typename T, typename Arguments> static constexpr const bool IsStatelessTest() {
			if constexpr (IsStaticFunction<T>()) { return true; }
			else {
				// using Arguments = typename fibers::utilities::function_traits<std::function<T(Args...)>>::arguments; // tuple

#define Ty(n) typename std::tuple_element<n, Arguments>::type
#define TIter0() Ty(0)
#define TIter1() Ty(0), Ty(1)
#define TIter2() Ty(0), Ty(1), Ty(2)
#define TIter3() Ty(0), Ty(1), Ty(2), Ty(3)
#define TIter4() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4)
#define TIter5() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5)
#define TIter6() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6)
#define TIter7() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7)
#define TIter8() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8)
#define TIter9() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9)
#define TIter10() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10)
#define TIter11() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11)
#define TIter12() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12)
#define TIter13() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12), Ty(13)
#define TIter14() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12), Ty(13), Ty(14)
#define TIter15() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12), Ty(13), Ty(14), Ty(15)
				if constexpr (std::tuple_size_v< Arguments> > 16) return IsStaticFunction<T>();
				if constexpr (std::tuple_size_v< Arguments> == 0) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T>::type(*)()>::value;
				if constexpr (std::tuple_size_v< Arguments> == 1) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter0()>::type(*)(TIter0())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 2) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter1()>::type(*)(TIter1())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 3) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter2()>::type(*)(TIter2())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 4) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter3()>::type(*)(TIter3())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 5) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter4()>::type(*)(TIter4())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 6) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter5()>::type(*)(TIter5())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 7) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter6()>::type(*)(TIter6())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 8) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter7()>::type(*)(TIter7())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 9) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter8()>::type(*)(TIter8())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 10) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter9()>::type(*)(TIter9())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 11) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter10()>::type(*)(TIter10())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 12) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter11()>::type(*)(TIter11())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 13) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter12()>::type(*)(TIter12())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 14) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter13()>::type(*)(TIter13())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 15) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter14()>::type(*)(TIter14())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 16) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter15()>::type(*)(TIter15())>::value;
				return false;
#undef Ty
#undef TIter0
#undef TIter1
#undef TIter2
#undef TIter3
#undef TIter4
#undef TIter5
#undef TIter6
#undef TIter7
#undef TIter8
#undef TIter9
#undef TIter10
#undef TIter11
#undef TIter12
#undef TIter13
#undef TIter14
#undef TIter15
			}
		};
		template <typename T, typename Args> static constexpr const bool IsLambdaStateless() {
			return IsLambda<T>() && IsStatelessTest<T, Args>();
		};
		template<typename T, typename Args> static constexpr const bool IsStateless() {
			return IsStaticFunction<T>() || IsLambdaStateless<T, Args>();
		};

	public:
		Job() : impl(nullptr) {};
		Job(const Job& other) : impl(other.impl) {};
		Job(Job&& other) : impl(std::move(other.impl)) {};
		Job& operator=(const Job& other) { impl = other.impl; return *this; };
		Job& operator=(Job&& other) { impl = std::move(other.impl); return *this; };

		/* Creates a job from a function and (optionally) input parameters. Can handle basic type-casting from inputs to parameters, and supports shared_ptr casting (to and from). */
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<fibers::Job, std::decay_t<T>> && !std::is_same_v<fibers::Any, std::decay_t<T>> >> explicit Job(T&& function, Args && ... Fargs) : impl(nullptr) {
			auto func{ fibers::Function(std::function(std::forward<T>(function)), std::forward<Args>(Fargs)...) };

			static constexpr const bool is_stateless{ IsStateless<T, typename decltype(func)::Arguments>() };
			
			
		
			if constexpr (decltype(func)::returnsNothing) {
				auto* action{ new fibers::Action_NoReturn(std::move(func), is_stateless) }; // create ptr
				impl = std::static_pointer_cast<fibers::Action_Base>(std::shared_ptr<typename std::remove_pointer_t<decltype(action)>>(std::move(action))); // move to smart ptr and then cast-down to base. Base handle counter will do destruction
			}
			else {
				auto* action{ new fibers::Action_Returns(std::move(func), is_stateless) }; // create ptr
				impl = std::static_pointer_cast<fibers::Action_Base>(std::shared_ptr<typename std::remove_pointer_t<decltype(action)>>(std::move(action))); // move to smart ptr and then cast-down to base. Base handle counter will do destruction
			}
		};
		~Job() = default;

	public:
		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		fibers::Any& Invoke() const noexcept {
			static fibers::Any staticVal{};
			if (impl) {
				return impl->Invoke();
			}
			else {
				return staticVal;
			}
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. The awaiter group guarrantees job completion before the awaiter or job goes out-of-scope. Useful for most basic task scheduling. */
		[[nodiscard]] JobGroup AsyncInvoke();

		/* Add the task to a thread / fiber, and then "forgets" the job. 
		CAUTION: user is responsible for guarranteeing that all data used by the job outlives the job itself, if using this mode of tasking. 
		This will throw an error if the underlying job is a capturing lambda, to reinforce the above requirement. 
		*/
		void AsyncFireAndForget();

		bool IsStatic() const noexcept {
			static bool staticVal{ true };
			if (impl) {
				return impl->IsStatic();
			}
			return staticVal;
		};

		/* Returns the potential name of the static function, if one is known. */
		std::string FunctionName() const {
			static std::string staticVal{ "" };
			if (impl) {
				return impl->FunctionName();
			}
			return staticVal;
		};

		/* Returns the result of the job, if any, if already performed. If not performed, the result will be empty. */
		const fibers::Any& GetResult() const {
			static fibers::Any staticVal{};
			if (impl) {
				return impl->GetResult();
			}
			return staticVal;
		};

		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		fibers::Any& operator()() {
			return Invoke();
		};
	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup {
		friend Job;
	private:
		class JobGroupImpl {
		public:
			std::shared_ptr<void> waitGroup;
			Job last_job;

			JobGroupImpl() : waitGroup(nullptr), last_job() {};
			JobGroupImpl(std::shared_ptr<void> wg) : waitGroup(wg), last_job() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;

			void Queue(Job const& job);
			void Queue(std::vector<Job> const& listOfJobs);
			void Wait();
			~JobGroupImpl() { Wait(); };
		};

	public:
		JobGroup();
		JobGroup(Job const& job);

		// The waiter should not be passed around. Ideally we want to follow Fiber job logic, e.g. splitting jobs quickly and 
		// then finishing them in the same job that started them, continuing like the split never happened.
		JobGroup(JobGroup const&) = delete;
		JobGroup(JobGroup&& a) : impl(std::move(a.impl)) {};
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() = default;

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			impl->Queue(job);
			return *this;
		};

		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			impl->Queue(listOfJobs);
			return *this;
		};

		/* Await all jobs in this group, and gets the return value of the last job submitted */
		template <typename T = void>
		decltype(auto) Wait_Get() {
			impl->Wait();

			if constexpr (std::is_same<T, void>::value) {
				return impl->last_job.GetResult();
			}
			else {
				return impl->last_job.GetResult().cast<T>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
		};

		impl::TaskGroup& GetTaskGroup() const {
			return *static_cast<impl::TaskGroup*>(impl->waitGroup.get());
		};

	protected:
		std::unique_ptr<JobGroupImpl> impl{};

	};

	namespace parallel {
#define UseStdForEachForParallelManager // without, we are very stable (>1.5hr) and memory-leak free (so far).
		
		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, F ToDo) {			
#ifdef UseStdForEachForParallelManager			
			fibers::utilities::Sequence seq(start, end); // 0..999
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(seq.begin(), seq.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
			});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}
#else
			impl::context ctx;
			impl::Dispatch(ctx, end - start, [&ToDo, start](impl::JobArgs const& _args)->void { 
				iteratorType t{ static_cast<iteratorType>(_args.jobIndex) + start };
				ToDo(t); 
			});
			impl::Wait(ctx);
#endif
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, iteratorType step, F ToDo) {
#ifdef UseStdForEachForParallelManager			
			fibers::utilities::Sequence seq(start, end, step); // 0..999
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(seq.begin(), seq.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
			});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}
#else
			impl::context ctx;
			impl::Dispatch(ctx, (end - start) / step, [&ToDo, start, step](impl::JobArgs const& _args)->void { 
				iteratorType t{ (static_cast<iteratorType>(_args.jobIndex) * step) + start };
				ToDo(t); 
			});
			impl::Wait(ctx);
#endif

		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType& container, F ToDo) {
#ifdef UseStdForEachForParallelManager		
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(container.begin(), container.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
			});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}			
#else
			auto begin = container.begin();
			auto end = container.end();
			using iterType = decltype(begin);
			impl::context ctx;
			impl::Dispatch(ctx, 
				std::distance(begin, end), 
				[&ToDo](impl::JobArgs const& _args)-> void { 
					iterType& iter = *((iterType*)_args.sharedmemory);
					if (_args.groupIndex == 0) {
						std::advance(iter, _args.jobIndex);
					}
					else {
						std::advance(iter, 1);
					}
					ToDo(*iter); 
				},
				sizeof(iterType),
				[&begin](void* p)->void { 
					new (p) iterType{ begin };
				},
				[](void* p)->void {
					((iterType*)p)->~iterType();
				}
			);
			impl::Wait(ctx);
#endif
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType const& container, F ToDo) {
#ifdef UseStdForEachForParallelManager		
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(container.begin(), container.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
				});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}
#else
			auto begin = container.begin();
			auto end = container.end();
			using iterType = decltype(begin);
			impl::context ctx;
			impl::Dispatch(ctx,
				std::distance(begin, end),
				[&ToDo](impl::JobArgs const& _args)-> void {
					iterType& iter = *((iterType*)_args.sharedmemory);
					if (_args.groupIndex == 0) {
						std::advance(iter, _args.jobIndex);
					}
					else {
						std::advance(iter, 1);
					}
					ToDo(*iter);
				},
				sizeof(iterType),
				[&begin](void* p)->void {
					new (p) iterType{ begin };
				},
				[](void* p)->void {
					((iterType*)p)->~iterType();
				}
				);
			impl::Wait(ctx);
#endif
		};

		/* wrapper for std::find_if. 
		This is not parallelized, it is linear, which appears to be the fastest search for some reason under most cases. */
		template<typename containerType, typename F> decltype(auto) Find(containerType& container, F const& ToDo) {
			return std::find_if(container.begin(), container.end(), [&](auto& x) ->bool { return ToDo(x); }); 
		};

		/* wrapper for std::find_if. 
		This is not parallelized, it is linear, which appears to be the fastest search for some reason under most cases. */
		template<typename containerType, typename F> decltype(auto) Find(containerType const& container, F const& ToDo) {
			return std::find_if(container.cbegin(), container.cend(), [&](auto const& x) ->bool { return ToDo(x); }); 
		};

		/* outputType x;
		for (auto& v : resultList){ x += v; }
		return x; */
		template<typename outputType> decltype(auto) Accumulate(std::vector<outputType> const& resultList) {
			return std::reduce(std::execution::par, resultList.begin(), resultList.end(), 0, [](outputType a, outputType b) { return a + b; });
		};

		/* Generic form of a future<T>, which can be used to wait on and get the results of any job. Can be safely shared if multiple places will need access to the result once available. */
		class promise {
		protected:
			std::shared_ptr< synchronization::atomic_ptr<JobGroup> > shared_state;
			std::shared_ptr< synchronization::atomic_ptr<Any> > result;
			std::shared_ptr< synchronization::mutex > waiting;

		public:
			promise() : shared_state(nullptr), result(nullptr), waiting(nullptr) {};
			promise(Job const& job) :
				shared_state(std::shared_ptr<synchronization::atomic_ptr<JobGroup>>(new synchronization::atomic_ptr<JobGroup>(new JobGroup(job)), [](synchronization::atomic_ptr<JobGroup>* anyP) { if (anyP) { auto* p = anyP->Set(nullptr); if (p) { delete p; } delete anyP; } })),
				result(std::shared_ptr<synchronization::atomic_ptr<Any>>(new synchronization::atomic_ptr<Any>(), [](synchronization::atomic_ptr<Any>* anyP) { if (anyP) { auto* p = anyP->Set(nullptr); if (p) { delete p; } delete anyP; } })),
				waiting(std::shared_ptr<synchronization::mutex>(new synchronization::mutex()))
			{};
			promise(promise const&) = default;
			promise(promise&&) = default;
			promise& operator=(promise const&) = default;
			promise& operator=(promise&&) = default;
			virtual ~promise() {};

			/* Returns true if this promise has been initialized correctly. Otherwise, false. */
			bool valid() const noexcept { return (bool)shared_state; };
			/* Wait until the requested job is completed. Repeated waiting is OK, however only the first "waiting" thread actually helps to complete the job - the remaining waiters will spin-wait. */
			void wait() {
				JobGroup* p{ nullptr };
				Any* p2{ nullptr };
				
				defer(if (p) { delete p; });
				defer(if (p2) { delete p2; });

				if (valid() && !result->load()) {
					auto guard{ std::lock_guard(*waiting) };

					p = shared_state->Set(nullptr);
					if (p) {
						p2 = result->Set(new Any(p->Wait_Get()));					
					}					
				}
			};
			/* Try to get the result, if available. Does not wait. */
			Any get_any() const noexcept {
				if (result) {
					Any* p = result->Get();
					if (p) {
						return Any(*p);
					}
				}
				return Any();
			};
			/* Get the result, once available. Waits for the result, if necessary. */
			Any wait_get_any() {
				wait();
				return get_any();
			};
		};

		/* A secondary type tag used to identify if a template type is a future<T> type. */
		class future_type { public: virtual ~future_type() {}; };

		/* Specialized form of a promise, which can be used to handle type-casting for lambdas automatically, while still being useful for waiting on and getting the results of any job.
		Note: Only the first thread that "waits" on a future<T> assists the thread pool. More waiters != more jobs, and therefore additional waiters are spin-locking.
		Recommended that only the thread (or consuming thread) that scheduled the future<T> object should wait for it. */
		template <typename T> class future final : public promise/*, public future_type*/ {
		public:
			future() : promise()/*, future_type()*/ {};
			future(Job const& job) : promise(job)/*, future_type()*/ {};
			future(promise const& p_promise) : promise(p_promise)/*, future_type()*/ {};
			future(future const&) = default;
			future(future&&) = default;
			future& operator=(future const&) = default;
			future& operator=(future&&) = default;
			virtual ~future() {};

			/* Cast-down to a generic promise that erases the information on the return type. Useful for sharing tasks between libraries where type info itself cannot be shared. */
			promise as_promise() const { return promise(reinterpret_cast<const promise&>(*this)); };

			/* get a copy of the result of the task. must have already waited. */
			decltype(auto) get() {
				if (result) {
					Any* p = result->Get();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get();
							//}
							//else {
							return static_cast<T>(p->cast<T>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};
			/* get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. must have already waited. */
			decltype(auto) get_ref() {
				if (result) {
					Any* p = result->Get();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get_ref();
							//}
							//else {
							return static_cast<T&>(p->cast<T&>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};
			/* get a shared_pointer of the result of the task. must have already waited. */
			decltype(auto) get_shared() {
				if (result) {
					Any* p = result->Get();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get_shared();
							//}
							//else {
							return static_cast<std::shared_ptr<T>>(p->cast<std::shared_ptr<T>>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};

			/* wait to get a copy of the result of the task. Repeated waiting is OK. */
			decltype(auto) wait_get() {
				wait();
				return get();
			};
			/* wait to get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. Repeated waiting is OK. */
			decltype(auto) wait_get_ref() {
				wait();
				return get_ref();
			};
			/* wait to get a shared_pointer of the result of the task. Repeated waiting is OK. */
			decltype(auto) wait_get_shared() {
				wait();
				return get_shared();
			};
		};

		/* returns a future<T> object for awaiting the results of the job. */
		template < typename F, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<F>> && !std::is_same_v<Any, std::decay_t<F>> >>
		__forceinline static decltype(auto) async(F function, Args... Fargs) {
			return future<typename utilities::function_traits<decltype(std::function(function))>::result_type>(Job(function, Fargs...));
		};
	};
};

#include "Units.h"

#include <boost/date_time.hpp>
// Thread- and Fiber-safe wrapper for Units::day which supports fundamental DateTime and Duration math. Utilizes Boost for the timezone math, AM/PM extractions, string conversions, etc.
// Meant to be used in parallel with Units for proper unit management while manipulating DateTime ranges.
// E.g: using namespace literals; return DateTime::make_time(1940, 1, 1) + 1_d - 30_s;
class DateTime {
protected:
	Units::day time; // stored as a thread-safe double
	static boost::posix_time::ptime const& Shared_Epoch_posixTime() {
		static boost::posix_time::ptime rc{ boost::posix_time::time_from_string("1970/1/1 0:0:0") };
		return rc;
	};
	boost::posix_time::ptime ToPTime() const {
		Units::millisecond D{ time };
		if (D < 0) {
			return Shared_Epoch_posixTime() + boost::posix_time::milliseconds((long long)(D()));
		}
		else {
			return Shared_Epoch_posixTime() - boost::posix_time::milliseconds((long long)(-(D())));
		}
	};
	Units::day FromPTime(boost::posix_time::ptime const& time) {
		boost::posix_time::ptime const& epoch = Shared_Epoch_posixTime();

		if (time >= epoch) {
			return Units::millisecond((time - epoch).total_milliseconds());
		}
		else {
			return -Units::millisecond((epoch - time).total_milliseconds());
		}
	};

private:
	explicit DateTime(const boost::posix_time::ptime& t) : time{ FromPTime(t) } {};

public:
	DateTime() : time{ FromPTime(Shared_Epoch_posixTime()) } {};
	DateTime(Units::second const& a) : time{ a } {};
	DateTime(Units::second&& a) : time{ a } {};
	DateTime(const DateTime& other) = default;
	DateTime(DateTime&& other) = default;
	DateTime& operator=(const DateTime& other) = default;
	DateTime& operator=(DateTime && other) = default;
	DateTime& operator=(Units::second const& other) { time = other; return *this; };
	DateTime(std::string const& t) : time{ FromPTime(Shared_Epoch_posixTime()) } { this->FromString(t); };
	~DateTime() = default;

public:
	auto& Add(const Units::day& v) { return time += v; }; // atomically adds a value and returns the new value
	auto& Sub(const Units::day& v) { return time -= v; }; // atomically subtracts a value and returns the new value
	Units::day Add(const Units::day& v) const { return time + v; }; // adds a value and returns the new value
	Units::day Sub(const Units::day& v) const { return time - v; }; // subtracts a value and returns the new value
	const auto& GetValue() const { return time; };
	auto& SetValue(const Units::day& v) { return time = v; }; // returns the previous value while setting with the new value
	auto& SetValue(Units::day&& v) { return time = std::forward<Units::day>(v); }; // returns the previous value while setting with the new value
	const auto& load() const { return GetValue(); };

private:
	static Units::second getUtcOffset_impl() {
		bool isNegative;

		time_t ts = 0;
		char buf[16];
		decltype(auto) t = ::localtime(&ts);
		::strftime(buf, sizeof(buf), "%z", t);
		std::string offset = buf; // -0800
		isNegative = offset.find('-') >= 0;
		// get the right 2 values
		decltype(auto) minuteOffset = std::atof(offset.substr(offset.length() - 2, 2).c_str()); // 00
		decltype(auto) hourOffset = std::atof(offset.substr(offset.length() - 4, 4).substr(0, 2).c_str()); // 08

		Units::second offsetV = ((hourOffset * 3600.0) + (minuteOffset * 60.0)) * (isNegative ? -1.0 : 1.0);

		if (t->tm_isdst) {
			// offsetV -= Units::second(3600);
		}

		return offsetV;
	};
	static Units::second getUtcOffset() {
		static Units::second tr(getUtcOffset_impl());
		return tr;
	};
	static Units::second getUtcOffset_impl(boost::posix_time::ptime const& pt) {
		bool isNegative;

		// time_t ts = boost::posix_time::to_tm(pt);
		char buf[16];
		decltype(auto) t = boost::posix_time::to_tm(boost::posix_time::ptime(pt.date()));
		::mktime(&t);
		::strftime(buf, sizeof(buf), "%z", &t);
		std::string offset = buf; // -0800

		isNegative = offset.find('-') >= 0;
		// get the right 2 values

		decltype(auto) minuteOffset = std::atof(offset.substr(offset.length() - 2, 2).c_str()); // 00
		decltype(auto) hourOffset = std::atof(offset.substr(offset.length() - 4, 4).substr(0, 2).c_str()); // 08

		Units::second offsetV = ((hourOffset * 3600.0) + (minuteOffset * 60.0)) * (isNegative ? -1.0 : 1.0);

		if (t.tm_isdst) {
			// offsetV -= Units::second(3600);
		}

		return offsetV;
	};
	static Units::second getUtcOffset(boost::posix_time::ptime const& pt) {
		return getUtcOffset_impl(pt);
	};
	static DateTime const& Shared_Epoch() { static DateTime rc{ Shared_Epoch_posixTime() }; return rc; }
	std::string	ToString() const {
		DateTime temp{ this->time + getUtcOffset(ToPTime()) };

		return Units::printf("%i/%i/%i %i:%i:%f",
			temp.tm_year() + 1900,
			temp.tm_mon() + 1,
			temp.tm_mday(),
			temp.tm_hour(),
			temp.tm_min(),
			temp.tm_sec()
		);
	};
	DateTime&	FromString(const std::string& timeStr) {
		DateTime t{ boost::posix_time::time_from_string(timeStr.c_str()) };
		return operator=(DateTime{ t.time - getUtcOffset(t.ToPTime()) });
	};

public:
	inline static DateTime timeFromString(const std::string& timeStr = "1970/1/1 0:0:0") { return DateTime().FromString(timeStr); };
	static DateTime Epoch() { return Shared_Epoch(); };
	static DateTime Now() { return DateTime{ static_cast<long double>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) / 1000.0 }; };
	static DateTime createTimeFromMinutes(float minutes) {
		decltype(auto) t = Now();
		t.ToStartOfDay();
		t += (minutes * 60.0f);
		return t;
	}
	static int			getNumDaysInSameMonth(DateTime const& in) {
		Units::day duration = (Units::day)(DateTime(in).ToEndOfMonth() - DateTime(in).ToStartOfMonth());
		duration += 0.5;
		return (int)duration.floor()();
		// return (int)std::floor((((long double)(DateTime(in).ToEndOfMonth() - DateTime(in).ToStartOfMonth())) / (24.0 * 60.0 * 60.0)) + 0.5);
	};
	static DateTime	make_time(int year = 1970, int month = 1, int day = 1, int hour = 0, int minute = 0, float second = 0, bool useLocalTime = true) {
		DateTime t = DateTime::timeFromString(Units::printf("%i/%i/%i %i:%i:%f", year, month, day, hour, minute, second));
		//if (useLocalTime) { t -= getUtcOffset(t.ToPTime())(); }
		return t;
	};
	static Units::second GetUtcOffset(DateTime const& in) {
		return getUtcOffset(in.ToPTime());
	};

public:
	DateTime& ToStartOfMonth() {
		this->operator-=(Units::second(tm_sec()));
		this->operator-=(Units::minute(tm_min()));
		this->operator-=(Units::hour(tm_hour()));
		this->operator-=(Units::day(tm_mday() - 1));
		return *this;
	};
	DateTime& ToStartOfDay() {
		this->operator-=(Units::second(tm_sec()));
		this->operator-=(Units::minute(tm_min()));
		this->operator-=(Units::hour(tm_hour()));
		return *this;
	};
	DateTime& ToStartOfHour() {
		this->operator-=(Units::second(tm_sec()));
		this->operator-=(Units::minute(tm_min()));
		return *this;
	};
	DateTime& ToStartOfMinute() {
		this->operator-=(Units::second(tm_sec()));
		return *this;
	};
	DateTime& ToEndOfMonth() {
		if (tm_mon() >= 11) {
			DateTime out(make_time(tm_year() + 1901, 1, 1, 0, 0, 0));
			out -= 1;
			*this = out; //  (make_time(tm_year() + 1901, 1, 1, 0, 0, 0) - 1);
		}
		else {
			DateTime out(make_time(tm_year() + 1900, tm_mon() + 2, 1, 0, 0, 0));
			out -= 1;
			*this = out; // (make_time(tm_year() + 1900, tm_mon() + 2, 1, 0, 0, 0) - 1);
		}
		return *this;
	};
	DateTime& ToEndOfDay() {
		this->operator+=(Units::second(60 - tm_sec()));
		this->operator+=(Units::second((59 - tm_min()) * 60));
		this->operator+=(Units::second((23 - tm_hour()) * 3600));
		return *this;
	};
	DateTime& ToEndOfHour() {
		this->operator+=(Units::second(60 - tm_sec()));
		this->operator+=(Units::second((59 - tm_min()) * 60));
		return *this;
	};
	DateTime& ToEndOfMinute() {
		this->operator+=(Units::second(60 - tm_sec()));
		return *this;
	};

public:
	/* milliseconds after the second - [0, 1000] including leap second */
	long double tm_fractionalsec() const {
		boost::posix_time::time_duration td(0, 0, 0, ToPTime().time_of_day().fractional_seconds());
		decltype(auto) t = (long double)(td.total_nanoseconds()) / 1000000000.0L;
		return t;
	};

	/* seconds after the minute - [0, 60] including leap second */
	long double tm_sec() const {
		return ((long double)ToPTime().time_of_day().seconds()) + tm_fractionalsec();
	};

	/* minutes after the hour - [0, 59] */
	int tm_min() const { return ToPTime().time_of_day().minutes(); };

	/* hours since midnight - [0, 23] */
	int tm_hour() const { return ToPTime().time_of_day().hours(); };

	/* day of the month - [1, 31] */
	int tm_mday() const { return ToPTime().date().year_month_day().day; };

	/* months since January - [0, 11] */
	int tm_mon() const { return ToPTime().date().year_month_day().month - 1; };

	/* years since 1900 */
	int tm_year() const { return ToPTime().date().year_month_day().year - 1900; };

	/* days since Sunday - [0, 6] */
	int tm_wday() const { return ToPTime().date().day_of_week().as_number(); };

	/* days since January 1 - [0, 365] */
	int tm_yday() const { return ToPTime().date().day_of_year() - 1; };

public:
	std::string c_str() const {
		return ToString();
	};
	operator std::string() const { return ToString(); };
	operator Units::second() const { return time; };
	operator Units::millisecond() const { return time; };
	operator Units::minute() const { return time; };
	operator Units::hour() const { return time; };
	operator Units::day() const { return time; };
	operator Units::year() const { return time; };

	friend bool	operator==(const DateTime& a, DateTime const& t) { return a.time == t.time; };
	friend bool	operator!=(const DateTime& a, DateTime const& t) { return !(a == t); };
	friend bool	operator>=(const DateTime& a, DateTime const& t) { return a.time >= t.time; };
	friend bool	operator<=(const DateTime& a, DateTime const& t) { return a.time <= t.time; };
	friend bool	operator>(const DateTime& a, DateTime const& t) { return !(a <= t); };
	friend bool	operator<(const DateTime& a, DateTime const& t) { return !(a >= t); };

	friend bool	operator==(const DateTime& a, Units::second const& t) { return a.time == t; };
	friend bool	operator!=(const DateTime& a, Units::second const& t) { return !(a == t); };
	friend bool	operator>=(const DateTime& a, Units::second const& t) { return a.time >= t; };
	friend bool	operator<=(const DateTime& a, Units::second const& t) { return a.time <= t; };
	friend bool	operator>(const DateTime& a, Units::second const& t) { return !(a <= t); };
	friend bool	operator<(const DateTime& a, Units::second const& t) { return !(a >= t); };

	friend bool	operator==(const Units::second& a, DateTime const& t) { return a == t.time; };
	friend bool	operator!=(const Units::second& a, DateTime const& t) { return !(a == t); };
	friend bool	operator>=(const Units::second& a, DateTime const& t) { return a >= t.time; };
	friend bool	operator<=(const Units::second& a, DateTime const& t) { return a <= t.time; };
	friend bool	operator>(const Units::second& a, DateTime const& t) { return !(a <= t); };
	friend bool	operator<(const Units::second& a, DateTime const& t) { return !(a >= t); };

	friend inline std::ostream& operator<<(std::ostream& os, DateTime const& obj) { os << obj.ToString(); return os; };
	friend inline std::stringstream& operator>>(std::stringstream& os, DateTime& obj) { Units::second v = 0; os >> v; obj = v; return os; };

	DateTime& operator+=(Units::second seconds) {
		time += Units::second(seconds);
		return *this;
	};
	DateTime& operator-=(Units::second seconds) {
		time -= Units::second(seconds);
		return *this;
	};
	DateTime& operator*=(Units::second seconds) {
		time *= seconds;
		return *this;
	};
	DateTime& operator/=(Units::second seconds) {
		time /= seconds;
		return *this;
	};

	friend DateTime operator+(const DateTime& a, const DateTime& b) {
		DateTime out(a);
		out += b.time;
		return out;
	};
	friend DateTime operator-(const DateTime& a, const DateTime& b) {
		DateTime out(a);
		out -= b.time;
		return out;
	};
	friend DateTime operator*(const DateTime& a, const DateTime& b) {
		DateTime out(a);
		out *= b.time;
		return out;
	};
	friend DateTime operator/(const DateTime& a, const DateTime& b) {
		DateTime out(a);
		out /= b.time;
		return out;
	};

	friend DateTime operator+(const DateTime& a, const Units::second& b) {
		DateTime out(a);
		out.time += b;
		return out;
	};
	friend DateTime operator-(const DateTime& a, const Units::second& b) {
		DateTime out(a);
		out.time -= b;
		return out;
	};
	friend DateTime operator*(const DateTime& a, const Units::second& b) {
		DateTime out(a);
		out.time *= b;
		return out;
	};
	friend DateTime operator/(const DateTime& a, const Units::second& b) {
		DateTime out(a);
		out.time /= b;
		return out;
	};

	friend DateTime operator+(const Units::second& a, const DateTime& b) {
		DateTime out{ Units::second(a) };
		out += b.time;
		return out;
	};
	friend DateTime operator-(const Units::second& a, const DateTime& b) {
		DateTime out{ Units::second(a) };
		out -= b.time;
		return out;
	};
	friend DateTime operator*(const Units::second& a, const DateTime& b) {
		DateTime out{ Units::second(a) };
		out *= b.time;
		return out;
	};
	friend DateTime operator/(const Units::second& a, const DateTime& b) {
		DateTime out{ Units::second(a) };
		out /= b.time;
		return out;
	};


};

class MultithreadingInstanceManager {
public:
	MultithreadingInstanceManager() {};
	virtual ~MultithreadingInstanceManager() {};
};
/* Instances the fiber system, and destroys it if the DLL / library is unloaded. */
extern std::shared_ptr<MultithreadingInstanceManager> multithreadingInstance;