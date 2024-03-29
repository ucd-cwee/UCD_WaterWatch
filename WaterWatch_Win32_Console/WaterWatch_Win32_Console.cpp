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

#include "WaterWatch_Win32_Console.h"

#include "../FiberTasks/Fibers.h"
#include "../FiberTasks/TaskScheduler.h"
#include "../FiberTasks/WaitGroup.h"

/* Wicked jobs are faster than everything (even single-threading!) when you use the dispatch system for parallel_for loops */
#pragma region WickedJob

#pragma once

#include <functional>
#include <atomic>

namespace wi::jobsystem
{
	void Initialize(uint32_t maxThreadCount = ~0u);
	void ShutDown();

	struct JobArgs
	{
		uint32_t jobIndex;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
		uint32_t groupID;		// group index relative to dispatch (like SV_GroupID in HLSL)
		uint32_t groupIndex;	// job index relative to group (like SV_GroupIndex in HLSL)
		bool isFirstJobInGroup;	// is the current job the first one in the group?
		bool isLastJobInGroup;	// is the current job the last one in the group?
		void* sharedmemory;		// stack memory shared within the current group (jobs within a group execute serially)
	};

	uint32_t GetThreadCount();

	// Defines a state of execution, can be waited on
	struct context
	{
		std::atomic<uint32_t> counter{ 0 };
	};

	// Add a task to execute asynchronously. Any idle thread will execute this.
	void Execute(context& ctx, const std::function<void(JobArgs)>& task);

	// Divide a task onto multiple jobs and execute in parallel.
	//	jobCount	: how many jobs to generate for this task.
	//	groupSize	: how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
	//	task		: receives a JobArgs as parameter
	void Dispatch(context& ctx, uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>& task, size_t sharedmemory_size = 0);

	// Returns the amount of job groups that will be created for a set number of jobs and group size
	uint32_t DispatchGroupCount(uint32_t jobCount, uint32_t groupSize);

	// Check if any threads are working currently or not
	bool IsBusy(const context& ctx);

	// Wait until all threads become idle
	//	Current thread will become a worker thread, executing jobs
	void Wait(const context& ctx);
}

#pragma once
#include <atomic>
#include <thread>
#include <emmintrin.h> // _mm_pause()
namespace wi {
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
};

#pragma once
#ifndef WICKEDENGINE_COMMONINCLUDE_H
#define WICKEDENGINE_COMMONINCLUDE_H

// This is a helper include file pasted into all engine headers, try to keep it minimal!
// Do not include engine features in this file!

#include <cstdint>
#include <type_traits>

#define arraysize(a) (sizeof(a) / sizeof(a[0]))

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

// CPU intrinsics:
#if defined(_WIN32)
// Windows, Xbox:
#include <intrin.h>
inline long AtomicAnd(volatile long* ptr, long mask)
{
	return _InterlockedAnd(ptr, mask);
}
inline long long AtomicAnd(volatile long long* ptr, long long mask)
{
	return _InterlockedAnd64(ptr, mask);
}
inline long AtomicOr(volatile long* ptr, long mask)
{
	return _InterlockedOr(ptr, mask);
}
inline long long AtomicOr(volatile long long* ptr, long long mask)
{
	return _InterlockedOr64(ptr, mask);
}
inline long AtomicXor(volatile long* ptr, long mask)
{
	return _InterlockedXor(ptr, mask);
}
inline long long AtomicXor(volatile long long* ptr, long long mask)
{
	return _InterlockedXor64(ptr, mask);
}
inline long AtomicAdd(volatile long* ptr, long val)
{
	return _InterlockedExchangeAdd(ptr, val);
}
inline long long AtomicAdd(volatile long long* ptr, long long val)
{
	return _InterlockedExchangeAdd64(ptr, val);
}
inline unsigned int countbits(unsigned int value)
{
	return __popcnt(value);
}
inline unsigned long long countbits(unsigned long long value)
{
	return __popcnt64(value);
}
inline unsigned long firstbithigh(unsigned long value)
{
	unsigned long bit_index;
	if (_BitScanReverse(&bit_index, value))
	{
		return 31ul - bit_index;
	}
	return 0;
}
inline unsigned long firstbithigh(unsigned long long value)
{
	unsigned long bit_index;
	if (_BitScanReverse64(&bit_index, value))
	{
		return 31ull - bit_index;
	}
	return 0;
}
inline unsigned long firstbitlow(unsigned long value)
{
	unsigned long bit_index;
	if (_BitScanForward(&bit_index, value))
	{
		return bit_index;
	}
	return 0;
}
inline unsigned long firstbitlow(unsigned long long value)
{
	unsigned long bit_index;
	if (_BitScanForward64(&bit_index, value))
	{
		return bit_index;
	}
	return 0;
}
#else
// Linux, PlayStation:
inline long AtomicAnd(volatile long* ptr, long mask)
{
	return __atomic_fetch_and(ptr, mask, __ATOMIC_SEQ_CST);
}
inline long long AtomicAnd(volatile long long* ptr, long long mask)
{
	return __atomic_fetch_and(ptr, mask, __ATOMIC_SEQ_CST);
}
inline long AtomicOr(volatile long* ptr, long mask)
{
	return __atomic_fetch_or(ptr, mask, __ATOMIC_SEQ_CST);
}
inline long long AtomicOr(volatile long long* ptr, long long mask)
{
	return __atomic_fetch_or(ptr, mask, __ATOMIC_SEQ_CST);
}
inline long AtomicXor(volatile long* ptr, long mask)
{
	return __atomic_fetch_xor(ptr, mask, __ATOMIC_SEQ_CST);
}
inline long long AtomicXor(volatile long long* ptr, long long mask)
{
	return __atomic_fetch_xor(ptr, mask, __ATOMIC_SEQ_CST);
}
inline long AtomicAdd(volatile long* ptr, long val)
{
	return __atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST);
}
inline long long AtomicAdd(volatile long long* ptr, long long val)
{
	return __atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST);
}
inline unsigned int countbits(unsigned int value)
{
	return __builtin_popcount(value);
}
inline unsigned long long countbits(unsigned long value)
{
	return __builtin_popcountl(value);
}
inline unsigned long long countbits(unsigned long long value)
{
	return __builtin_popcountll(value);
}
inline unsigned long firstbithigh(unsigned int value)
{
	if (value == 0)
	{
		return 0;
	}
	return __builtin_clz(value);
}
inline unsigned long firstbithigh(unsigned long value)
{
	if (value == 0)
	{
		return 0;
	}
	return __builtin_clzl(value);
}
inline unsigned long firstbithigh(unsigned long long value)
{
	if (value == 0)
	{
		return 0;
	}
	return __builtin_clzll(value);
}
inline unsigned long firstbitlow(unsigned int value)
{
	if (value == 0)
	{
		return 0;
	}
	return __builtin_ctz(value);
}
inline unsigned long firstbitlow(unsigned long value)
{
	if (value == 0)
	{
		return 0;
	}
	return __builtin_ctzl(value);
}
inline unsigned long firstbitlow(unsigned long long value)
{
	if (value == 0)
	{
		return 0;
	}
	return __builtin_ctzll(value);
}
#endif // _WIN32

// Enable enum flags:
//	https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
template<typename E>
struct enable_bitmask_operators {
	static constexpr bool enable = false;
};
template<typename E>
constexpr typename std::enable_if<enable_bitmask_operators<E>::enable, E>::type operator|(E lhs, E rhs)
{
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(
		static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}
template<typename E>
constexpr typename std::enable_if<enable_bitmask_operators<E>::enable, E&>::type operator|=(E& lhs, E rhs)
{
	typedef typename std::underlying_type<E>::type underlying;
	lhs = static_cast<E>(
		static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
	return lhs;
}
template<typename E>
constexpr typename std::enable_if<enable_bitmask_operators<E>::enable, E>::type operator&(E lhs, E rhs)
{
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(
		static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}
template<typename E>
constexpr typename std::enable_if<enable_bitmask_operators<E>::enable, E&>::type operator&=(E& lhs, E rhs)
{
	typedef typename std::underlying_type<E>::type underlying;
	lhs = static_cast<E>(
		static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
	return lhs;
}
template<typename E>
constexpr typename std::enable_if<enable_bitmask_operators<E>::enable, E>::type operator~(E rhs)
{
	typedef typename std::underlying_type<E>::type underlying;
	rhs = static_cast<E>(
		~static_cast<underlying>(rhs));
	return rhs;
}
template<typename E>
constexpr bool has_flag(E lhs, E rhs)
{
	return (lhs & rhs) == rhs;
}

#endif //WICKEDENGINE_COMMONINCLUDE_H

#include <string>

#pragma once
// This file includes platform, os specific libraries and supplies common platform specific resources

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <SDKDDKVer.h>
#include <windows.h>
#include <tchar.h>

#if WINAPI_FAMILY == WINAPI_FAMILY_APP
#define PLATFORM_UWP
#define wiLoadLibrary(name) LoadPackagedLibrary(_T(name),0)
#define wiGetProcAddress(handle,name) GetProcAddress(handle, name)
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#else
#if WINAPI_FAMILY == WINAPI_FAMILY_GAMES
#define PLATFORM_XBOX
#else
#define PLATFORM_WINDOWS_DESKTOP
#endif // WINAPI_FAMILY_GAMES
#define wiLoadLibrary(name) LoadLibraryA(name)
#define wiGetProcAddress(handle,name) GetProcAddress(handle, name)
#endif // WINAPI_FAMILY_APP
#elif defined(__SCE__)
#define PLATFORM_PS5
#else
#define PLATFORM_LINUX
#include <dlfcn.h>
#define wiLoadLibrary(name) dlopen(name, RTLD_LAZY)
#define wiGetProcAddress(handle,name) dlsym(handle, name)
typedef void* HMODULE;
#endif // _WIN32

#ifdef SDL2
#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include "sdl2.h"
#endif


namespace wi::platform
{
#ifdef _WIN32
#ifdef PLATFORM_UWP
	using window_type = const winrt::Windows::UI::Core::CoreWindow*;
#else
	using window_type = HWND;
#endif // PLATFORM_UWP
#elif SDL2
	using window_type = SDL_Window*;
#else
	using window_type = void*;
#endif // _WIN32

	inline void Exit()
	{
#ifdef _WIN32
#ifndef PLATFORM_UWP
		PostQuitMessage(0);
#else
		winrt::Windows::ApplicationModel::Core::CoreApplication::Exit();
#endif // PLATFORM_UWP
#endif // _WIN32
#ifdef SDL2
		SDL_Event quit_event;
		quit_event.type = SDL_QUIT;
		SDL_PushEvent(&quit_event);
#endif
	}

	struct WindowProperties
	{
		int width = 0;
		int height = 0;
		float dpi = 96;
	};
	inline void GetWindowProperties(window_type window, WindowProperties* dest)
	{
#ifdef PLATFORM_WINDOWS_DESKTOP
		dest->dpi = (float)GetDpiForWindow(window);
#endif // WINDOWS_DESKTOP

#ifdef PLATFORM_XBOX
		dest->dpi = 96.f;
#endif // PLATFORM_XBOX

#if defined(PLATFORM_WINDOWS_DESKTOP) || defined(PLATFORM_XBOX)
		RECT rect;
		GetClientRect(window, &rect);
		dest->width = int(rect.right - rect.left);
		dest->height = int(rect.bottom - rect.top);
#endif // PLATFORM_WINDOWS_DESKTOP || PLATFORM_XBOX

#ifdef PLATFORM_UWP
		dest->dpi = winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView().LogicalDpi();
		float dpiscale = dest->dpi / 96.f;
		dest->width = uint32_t(window->Bounds().Width * dpiscale);
		dest->height = uint32_t(window->Bounds().Height * dpiscale);
#endif // PLATFORM_UWP

#ifdef PLATFORM_LINUX
		int window_width, window_height;
		SDL_GetWindowSize(window, &window_width, &window_height);
		SDL_Vulkan_GetDrawableSize(window, &dest->width, &dest->height);
		dest->dpi = ((float)dest->width / (float)window_width) * 96.f;
#endif // PLATFORM_LINUX
	}
}






#pragma once
#include <chrono>
namespace wi
{
	struct Timer {
		std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();

		// Record a reference timestamp
		inline void record()
		{
			timestamp = std::chrono::high_resolution_clock::now();
		}

		// Elapsed time in seconds between the wi::Timer creation or last recording and "timestamp2"
		inline double elapsed_seconds_since(std::chrono::high_resolution_clock::time_point timestamp2)
		{
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timestamp2 - timestamp);
			return time_span.count();
		}

		// Elapsed time in seconds since the wi::Timer creation or last recording
		inline double elapsed_seconds()
		{
			return elapsed_seconds_since(std::chrono::high_resolution_clock::now());
		}

		// Elapsed time in milliseconds since the wi::Timer creation or last recording
		inline double elapsed_milliseconds()
		{
			return elapsed_seconds() * 1000.0;
		}

		// Elapsed time in milliseconds since the wi::Timer creation or last recording
		inline double elapsed()
		{
			return elapsed_milliseconds();
		}

		// Record a reference timestamp and return elapsed time in seconds since the wi::Timer creation or last recording
		inline double record_elapsed_seconds()
		{
			auto timestamp2 = std::chrono::high_resolution_clock::now();
			auto elapsed = elapsed_seconds_since(timestamp2);
			timestamp = timestamp2;
			return elapsed;
		}
	};
}



#include <memory>
#include <algorithm>
#include <deque>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef PLATFORM_LINUX
#include <pthread.h>
#endif // PLATFORM_LINUX

namespace wi::jobsystem {
	struct Task {
		std::function<void(JobArgs)> task;
		context* ctx;
		uint32_t groupID;
		uint32_t groupJobOffset;
		uint32_t groupJobEnd;
		uint32_t sharedmemory_size;
	};
	struct TaskQueue {
		std::deque<Task> queue;
		std::mutex locker;
		inline void push(Task&& item) {
			std::scoped_lock lock(locker);
			queue.push_back(std::forward<Task>(item));
		};
		inline void push(const Task& item) {
			std::scoped_lock lock(locker);
			queue.push_back(item);
		};
		inline bool try_pop(Task& item) {
			std::scoped_lock lock(locker);
			if (queue.empty()) return false;
			item = std::move(queue.front());
			queue.pop_front();
			return true;
		};
	};

	// This structure is responsible to stop worker thread loops. Once this is destroyed, worker threads will be woken up and end their loops.
	struct InternalState {
		uint32_t numCores = 0;
		uint32_t numThreads = 0;
		std::unique_ptr<TaskQueue[]> jobQueuePerThread;
		std::atomic_bool alive{ true };
		std::condition_variable wakeCondition;
		std::mutex wakeMutex;
		std::atomic<uint32_t> nextQueue{ 0 };
		std::vector<std::thread> threads;
		void ShutDown() {
			alive.store(false); // indicate that new jobs cannot be started from this point
			bool wake_loop = true;
			std::thread waker([&] {
				while (wake_loop)
				{
					wakeCondition.notify_all(); // wakes up sleeping worker threads
				}
			});
			for (auto& thread : threads)
			{
				thread.join();
			}
			wake_loop = false;
			waker.join();
			jobQueuePerThread.reset();
			threads.clear();
			numCores = 0;
			numThreads = 0;
		};
		~InternalState() {
			ShutDown();
		};
	} static internal_state;

	// Start working on a job queue. After the job queue is finished, it can switch to an other queue and steal jobs from there
	inline void work(uint32_t startingQueue) {
		Task job;
		for (uint32_t i = 0; i < internal_state.numThreads; ++i) {
			TaskQueue& job_queue = internal_state.jobQueuePerThread[startingQueue % internal_state.numThreads];
			while (job_queue.try_pop(job))
			{
				JobArgs args;
				args.groupID = job.groupID;
				if (job.sharedmemory_size > 0)
				{
					thread_local static std::vector<uint8_t> shared_allocation_data;
					shared_allocation_data.reserve(job.sharedmemory_size);
					args.sharedmemory = shared_allocation_data.data();
				}
				else
				{
					args.sharedmemory = nullptr;
				}

				for (uint32_t j = job.groupJobOffset; j < job.groupJobEnd; ++j)
				{
					args.jobIndex = j;
					args.groupIndex = j - job.groupJobOffset;
					args.isFirstJobInGroup = (j == job.groupJobOffset);
					args.isLastJobInGroup = (j == job.groupJobEnd - 1);
					job.task(args);
				}

				job.ctx->counter.fetch_sub(1);
			}
			startingQueue++; // go to next queue
		}
	};

	void Initialize(uint32_t maxThreadCount) {
		if (internal_state.numThreads > 0)
			return;
		maxThreadCount = std::max(1u, maxThreadCount);

		wi::Timer timer;

		// Retrieve the number of hardware threads in this system:
		internal_state.numCores = std::thread::hardware_concurrency();

		// Calculate the actual number of worker threads we want (-1 main thread):
		internal_state.numThreads = std::min(maxThreadCount, std::max(1u, internal_state.numCores - 1));
		internal_state.jobQueuePerThread.reset(new TaskQueue[internal_state.numThreads]);
		internal_state.threads.reserve(internal_state.numThreads);

		for (uint32_t threadID = 0; threadID < internal_state.numThreads; ++threadID)
		{
			internal_state.threads.emplace_back([threadID] {

				while (internal_state.alive.load())
				{
					work(threadID);

					// finished with jobs, put to sleep
					std::unique_lock<std::mutex> lock(internal_state.wakeMutex);
					internal_state.wakeCondition.wait(lock);
				}

				});
			std::thread& worker = internal_state.threads.back();

#ifdef _WIN32
			// Do Windows-specific thread setup:
			HANDLE handle = (HANDLE)worker.native_handle();

			// Put each thread on to dedicated core:
			DWORD_PTR affinityMask = 1ull << threadID;
			DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
			assert(affinity_result > 0);

			//// Increase thread priority:
			//BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
			//assert(priority_result != 0);

			// Name the thread:
			std::wstring wthreadname = L"wi::jobsystem_" + std::to_wstring(threadID);
			HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
			assert(SUCCEEDED(hr));
#elif defined(PLATFORM_LINUX)
#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); } while (0)

			int ret;
			cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			size_t cpusetsize = sizeof(cpuset);

			CPU_SET(threadID, &cpuset);
			ret = pthread_setaffinity_np(worker.native_handle(), cpusetsize, &cpuset);
			if (ret != 0)
				handle_error_en(ret, std::string(" pthread_setaffinity_np[" + std::to_string(threadID) + ']').c_str());

			// Name the thread
			std::string thread_name = "wi::job::" + std::to_string(threadID);
			ret = pthread_setname_np(worker.native_handle(), thread_name.c_str());
			if (ret != 0)
				handle_error_en(ret, std::string(" pthread_setname_np[" + std::to_string(threadID) + ']').c_str());
#undef handle_error_en
#elif defined(PLATFORM_PS5)
			wi::jobsystem::ps5::SetupWorker(worker, threadID);
#endif // _WIN32
		}

		std::cout << "wi::jobsystem Initialized with [" + std::to_string(internal_state.numCores) + " cores] [" + std::to_string(internal_state.numThreads) + " threads] (" + std::to_string((int)std::round(timer.elapsed())) + " ms)" << std::endl;
	};

	void ShutDown() {
		internal_state.ShutDown();
	};

	uint32_t GetThreadCount() {
		return internal_state.numThreads;
	};

	void Execute(context& ctx, const std::function<void(JobArgs)>& task) {
		// Context state is updated:
		ctx.counter.fetch_add(1);
		internal_state.jobQueuePerThread[internal_state.nextQueue.fetch_add(1) % internal_state.numThreads].push({ task, &ctx, 0, 0, 1, 0 });
		internal_state.wakeCondition.notify_one();
	};

	void Dispatch(context& ctx, uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>& task, size_t sharedmemory_size) {
		if (jobCount == 0 || groupSize == 0) { return; }

		const uint32_t groupCount = DispatchGroupCount(jobCount, groupSize);

		// Context state is updated:
		ctx.counter.fetch_add(groupCount);

		Task job{ task, &ctx, 0, 0, 1, (uint32_t)sharedmemory_size };

		for (uint32_t groupID = 0; groupID < groupCount; ++groupID) {
			// For each group, generate one real job:
			job.groupID = groupID;
			job.groupJobOffset = groupID * groupSize;
			job.groupJobEnd = std::min(job.groupJobOffset + groupSize, jobCount);
			internal_state.jobQueuePerThread[internal_state.nextQueue.fetch_add(1) % internal_state.numThreads].push(job);
		}

		internal_state.wakeCondition.notify_all();
	};

	uint32_t DispatchGroupCount(uint32_t jobCount, uint32_t groupSize) {
		// Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
		return (jobCount + groupSize - 1) / groupSize;
	};

	bool IsBusy(const context& ctx) {
		// Whenever the context label is greater than zero, it means that there is still work that needs to be done
		return ctx.counter.load() > 0;
	};

	void Wait(const context& ctx) {
		if (IsBusy(ctx)) {
			// Wake any threads that might be sleeping:
			internal_state.wakeCondition.notify_all();

			// work() will pick up any jobs that are on stand by and execute them on this thread:
			work(internal_state.nextQueue.fetch_add(1) % internal_state.numThreads);

			while (IsBusy(ctx)) {
				// If we are here, then there are still remaining jobs that work() couldn't pick up.
				//	In this case those jobs are not standing by on a queue but currently executing
				//	on other threads, so they cannot be picked up by this thread.
				//	Allow to swap out this thread by OS to not spin endlessly for nothing
				std::this_thread::yield();
			}
		}
	};

	struct TaskGroup {
	private:
		context ctx;

	public:
		auto Wait() const { return wi::jobsystem::Wait(ctx); };
		auto IsBusy() const { return wi::jobsystem::IsBusy(ctx); };
		auto Queue(const std::function<void(JobArgs)>& task) { return wi::jobsystem::Execute(ctx, task); };
		auto Dispatch(uint32_t jobCount, const std::function<void(JobArgs)>& task) { return wi::jobsystem::Dispatch(ctx, jobCount, (10000.0 / 400.0) * jobCount, task); };
	};

	/*  */
	template<typename iteratorType, typename F> decltype(auto) parallel_for(iteratorType start, iteratorType end, F const& ToDo) {
		constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
		
		if constexpr (retNo) {
			if (end <= start) return;

			wi::jobsystem::TaskGroup group;
			
			group.Dispatch(end - start, [&](wi::jobsystem::JobArgs args) {
				ToDo(start + args.jobIndex);
			});

			group.Wait();
		}
		else {
			using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

			if (end <= start) return std::vector< returnT >();

			std::vector< returnT > out(end - start, returnT());

			wi::jobsystem::TaskGroup group;

			group.Dispatch(end - start, [&](wi::jobsystem::JobArgs args) {
				out[args.jobIndex] = ToDo(start + args.jobIndex);
			});

			group.Wait();

			return out;
		}
	};

	/*  */
	template<typename iteratorType, typename F> decltype(auto) parallel_for(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
		constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
		
		if constexpr (retNo) {
			if (end <= start) return; 
			
			wi::jobsystem::TaskGroup group;

			group.Dispatch((end - start) / step, [&](wi::jobsystem::JobArgs args) {
				ToDo(start + args.jobIndex * step);
			});

			group.Wait();
		}
		else {
			using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

			if (end <= start) return std::vector< returnT >();

			std::vector< returnT > out((end - start) / step, returnT());

			wi::jobsystem::TaskGroup group;

			group.Dispatch((end - start) / step, [&](wi::jobsystem::JobArgs args) {
				out[args.jobIndex] = ToDo(start + args.jobIndex * step);
			});

			group.Wait();

			return out;
		}
	};

	/*  */
	template<typename containerType, typename F> decltype(auto) parallel_for_each(containerType& container, F const& ToDo) {
		constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
		
		int n = container.size();
		if constexpr (retNo) {
			if (n <= 0) return;

			std::vector<containerType::iterator> iterators(n, container.begin());

			wi::jobsystem::TaskGroup group;

			group.Dispatch(n, [&](wi::jobsystem::JobArgs args) {
				std::advance(iterators[args.jobIndex], args.jobIndex);
				ToDo(*iterators[args.jobIndex]);
			});

			group.Wait();
		}
		else {
			using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

			if (n <= 0) return std::vector< returnT >();

			std::vector<containerType::iterator> iterators(n, container.begin());

			std::vector< returnT > out(n, returnT());

			wi::jobsystem::TaskGroup group;

			group.Dispatch(n, [&](wi::jobsystem::JobArgs args) {
				std::advance(iterators[args.jobIndex], args.jobIndex);
				out[args.jobIndex] = ToDo(*iterators[args.jobIndex]);
			});

			group.Wait();

			return out;
		}
	};

	/*  */
	template<typename containerType, typename F> decltype(auto) parallel_for_each(containerType const& container, F const& ToDo) {
		constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
		
		int n = container.size();
		if constexpr (retNo) {
			if (n <= 0) return;

			std::vector<containerType::const_iterator> iterators(n, container.begin());

			wi::jobsystem::TaskGroup group;

			group.Dispatch(n, [&](wi::jobsystem::JobArgs args) {
				std::advance(iterators[args.jobIndex], args.jobIndex);
				ToDo(*iterators[args.jobIndex]);
			});

			group.Wait();
		}
		else {
			using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

			if (n <= 0) return std::vector< returnT >();

			std::vector<containerType::const_iterator> iterators(n, container.begin());

			std::vector< returnT > out(n, returnT());

			wi::jobsystem::TaskGroup group;

			group.Dispatch(n, [&](wi::jobsystem::JobArgs args) {
				std::advance(iterators[args.jobIndex], args.jobIndex);
				out[args.jobIndex] = ToDo(*iterators[args.jobIndex]);
			});

			group.Wait();

			return out;
		}
	};

	class JobGroup;

	/*!
	Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wraper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	Job([](){ return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, allowing you to continue work on this thread.
	*/
	class Job {
		friend JobGroup;
	protected:
		mutable std::shared_ptr<Action> impl;

	public:
		Job() : impl(std::make_shared<Action>()) {};
		Job(const Job& other) : impl(other.impl) {};
		Job(Job&& other) : impl(other.impl) {};
		Job& operator=(const Job& other) { impl = other.impl; return *this; };
		Job& operator=(Job&& other) { impl = std::move(other.impl); return *this; };
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<T>> && !std::is_same_v<Any, std::decay_t<T>> >>
		explicit Job(T function, Args && ... Fargs) : impl(new Action(function, std::forward<Args>(Fargs)...)) {};
		~Job() = default;

	public:
		/* Do the task immediately, without using any thread/fiber tools. Does not do the task if it has been previously performed. */
		Any Invoke() const noexcept {
			Any out;
			if (impl) {
				out = impl->Invoke();
			}
			return out;
		};

		/* Do the task immediately, without using any thread/fiber tools, whether or not it has been performed before. */
		Any ForceInvoke() const noexcept {
			Any out;
			if (impl) {
				out = impl->ForceInvoke();
			}
			return out;
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. Does not do the task if it has been previously performed. */
		JobGroup AsyncInvoke();

		/* Add the task to a thread / fiber, and retrieve an awaiter group, whether or not it has been performed before. */
		JobGroup AsyncForceInvoke();

		/* Returns the potential name of the static function, if one was provided. */
		const char* FunctionName() const {
			if (impl) {
				return impl->FunctionName();
			}
			return "";
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any GetResult() {
			return Invoke();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any operator()() {
			return Invoke();
		};

		/* Checks if the job has been completed before */
		bool IsFinished() const {
			if (impl) {
				return impl->IsFinished();
			}
			return true;
		};

		bool ReturnsNothing() const {
			if (impl) {
				return impl->ReturnsNothing();
			}
			return true;
		};
	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup {
		friend Job;
	private:
		class JobGroupImpl {
		public:
			std::shared_ptr<void> waitGroup;
			fibers::containers::vector<Job> jobs;


			JobGroupImpl() : waitGroup(nullptr), jobs() {};
			JobGroupImpl(std::shared_ptr<void> wg) : waitGroup(wg), jobs() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;

			void Queue(Job const& job);
			void ForceQueue(Job const& job);
			void Queue(std::vector<Job> const& listOfJobs);
			void ForceQueue(std::vector<Job> const& listOfJobs);
			void Wait();
			~JobGroupImpl() {
				// Wait(); // any jobs queued with this waiter will want to talk w/it when finished -- we must wait to ensure they go out of scope before we do.
			};
		};

	public:
		JobGroup();
		JobGroup(Job const& job);

		// The waiter should not be passed around. Ideally we want to follow Fiber job logic, e.g. splitting jobs quickly and 
		// then finishing them in the same job that started them, continuing like the split never happened.
		JobGroup(JobGroup const&) = delete;
		JobGroup(JobGroup&&) = delete;
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() {};

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			impl->Queue(job);
			return *this;
		};
		/* Queue job, and return tool to await the result */
		JobGroup& ForceQueue(Job const& job) {
			impl->ForceQueue(job);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			impl->Queue(listOfJobs);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& ForceQueue(std::vector<Job> const& listOfJobs) {
			impl->ForceQueue(listOfJobs);
			return *this;
		};

		/* Await all jobs in this group, and get the return values (which may be empty) for each job */
		std::vector<Any> Wait_Get() {
			impl->Wait();

			typename decltype(JobGroupImpl::jobs) out;
			out.swap(impl->jobs);

			if (out.size() > 0) {
				std::vector<Any> any;
				for (auto& job : out) {
					if (job) {
						any.push_back(job->GetResult());
					}
				}
				return any;
			}
			else {
				return std::vector<Any>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
			impl->jobs.clear();
		};

	private:
		std::unique_ptr<JobGroupImpl> impl;

	};

	JobGroup::JobGroup() : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<TaskGroup>(new TaskGroup())))) {};
	JobGroup::JobGroup(Job const& job) : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<TaskGroup>(new TaskGroup())))) { Queue(job); };
	void JobGroup::JobGroupImpl::Queue(Job const& job) {
		std::shared_ptr<TaskGroup> wg = std::static_pointer_cast<TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Queue([impl = job.impl](JobArgs args) {
			impl->Invoke();
		});
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::ForceQueue(Job const& job) {
		std::shared_ptr<TaskGroup> wg = std::static_pointer_cast<TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Queue([impl = job.impl](JobArgs args) {
			impl->ForceInvoke();
		});
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::Queue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<TaskGroup> wg = std::static_pointer_cast<TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));

		wg->Dispatch(listOfJobs.size(), [listOfJobs](JobArgs args) {
			listOfJobs[args.jobIndex].Invoke();
		}); 

		for (Job const& j : listOfJobs) {
			jobs.push_back(j);
		}
	};
	void JobGroup::JobGroupImpl::ForceQueue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<TaskGroup> wg = std::static_pointer_cast<TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));

		wg->Dispatch(listOfJobs.size(), [listOfJobs](JobArgs args) {
			listOfJobs[args.jobIndex].ForceInvoke();
		});

		for (Job const& j : listOfJobs) {
			jobs.push_back(j);
		}
	};
	void JobGroup::JobGroupImpl::Wait() {
		std::shared_ptr<TaskGroup> wg = std::static_pointer_cast<TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Wait();
	};
	JobGroup Job::AsyncInvoke() { return JobGroup(*this); };
	JobGroup Job::AsyncForceInvoke() { return JobGroup(*this); };


};
namespace wi {
	struct timer
	{
		std::string name;
		std::chrono::high_resolution_clock::time_point start;

		timer(const std::string& name) : name(name), start(std::chrono::high_resolution_clock::now()) {}
		~timer()
		{
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << name << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds" << std::endl;
		}
	};
	void Spin(float milliseconds)
	{
		milliseconds /= 1000.0f;
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
		double ms = 0;
		while (ms < milliseconds)
		{
			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			ms = time_span.count();
		}
	}
}


#pragma endregion


namespace testing {
	namespace utilities {
		class typenames {
		public:
			template<typename T>
			struct identity { typedef T type; };

			class detail {
			public:
				using type_name_prober = void;
				template <typename T> static constexpr std::string_view wrapped_type_name() {
#ifdef __clang__
					return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
					return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
					return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
				};
				static constexpr std::size_t wrapped_type_name_prefix_length() { return wrapped_type_name<type_name_prober>().find(sv_type_name<type_name_prober>()); };
				static constexpr std::size_t wrapped_type_name_suffix_length() { return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - sv_type_name<type_name_prober>().length(); };
			};

			template <typename T>
			static constexpr std::string_view sv_type_name() {
				return sv_type_name(identity<T>());
			};

			template <typename T>
			static constexpr const char* type_name() {
				return type_name(identity<T>());
			};

		private:
			template <typename T>
			static constexpr std::string_view sv_type_name(identity<T>)
			{
				constexpr std::string_view wrapped_name = utilities::typenames::detail::wrapped_type_name<T>();
				constexpr auto prefix_length = utilities::typenames::detail::wrapped_type_name_prefix_length();
				constexpr auto suffix_length = utilities::typenames::detail::wrapped_type_name_suffix_length();
				constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
				return wrapped_name.substr(prefix_length, type_name_length);
			};

			template <typename T>
			static constexpr const char* type_name(identity<T>)
			{
				constexpr std::string_view wrapped_name = utilities::typenames::detail::wrapped_type_name<T>();
				constexpr auto prefix_length = utilities::typenames::detail::wrapped_type_name_prefix_length();
				constexpr auto suffix_length = utilities::typenames::detail::wrapped_type_name_suffix_length();
				constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
				return wrapped_name.substr(prefix_length, type_name_length).data();
			};

			static constexpr std::string_view sv_type_name(identity<void>) { return "void"; };
			static constexpr const char* type_name(identity<void>) { return "void"; };
		};
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };
	};
	namespace {
		template<class T> struct get_type { using type = T; };
		template<class T> struct get_type<std::shared_ptr<T>> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<std::shared_ptr<T>&> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<std::shared_ptr<T>*> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>&> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>*> { using type = typename get_type<T>::type; };
	};

	template <typename F = void()> class Function { 
	public:
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; static constexpr const size_t num_arguments = 0; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; static constexpr const size_t num_arguments = 0; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; static constexpr const size_t num_arguments = sizeof...(Args); };

		using Type = typename F;
		using ResultType = typename function_traits<std::function<Type>>::result_type;
		using Arguments = typename function_traits<std::function<Type>>::arguments;
		static constexpr const size_t NumArguments = function_traits<std::function<Type>>::num_arguments;
	
	private:
		template<int N, bool badIndex> struct function_destination_impl {
			using type = typename std::tuple_element<N, typename Arguments>::type;
		};
		template<int N> struct function_destination_impl<N, true> {
			using type = int;
		};

		template<int N> struct ArgumentType {
			static constexpr bool is_bad_index = N >= NumArguments;
			
			struct function_destination {
				using type = typename function_destination_impl<N, is_bad_index>::type;
				using underlying_type = typename testing::get_type<typename type>::type;

				static constexpr bool is_shared_ptr = !std::is_same<typename testing::get_type<type>::type, type>::value;
				static constexpr bool is_reference = std::is_reference<underlying_type>::value;
				static constexpr bool is_pointer = std::is_pointer<underlying_type>::value;
				static constexpr bool is_const = std::is_const<typename std::remove_pointer<typename std::remove_reference<underlying_type>::type>::type>::value;
			};
			struct parameter_pack {
			private:
				template<bool is_shared_ptr> struct underlying_type_impl {
					using type = typename std::remove_pointer<typename std::decay<typename function_destination::underlying_type>::type>::type;
					using package_type = type;
				};
				template<> struct underlying_type_impl<true> {
					using type = typename function_destination::underlying_type;
					using package_type = std::shared_ptr<type>;
				};

			public:
				using underlying_type = typename underlying_type_impl<function_destination::is_shared_ptr>::type;
				using shared_ptr_type = std::shared_ptr<underlying_type>;
				using unique_ptr_type = std::unique_ptr<underlying_type>;
				using type = typename underlying_type_impl<function_destination::is_shared_ptr>::package_type;

				static constexpr bool is_trivial = std::is_trivial<underlying_type>::value;
				static constexpr bool is_move_constructible = std::is_move_constructible<underlying_type>::value;
				static constexpr bool is_copy_constructible = std::is_copy_constructible<underlying_type>::value;
			};
		};
#define parameterPackFoundation std::tuple
#define getParam(N, O) std::get<N>(O)
//#define parameterPackFoundation cweeUnion
//#define getParam(N, O) O.get<N>()
		template <int N> struct ParameterPackImpl {
			using type = parameterPackFoundation<>;
		};
		template <> struct ParameterPackImpl<0> {
			using type = parameterPackFoundation<>;
		};
		template <> struct ParameterPackImpl<1> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<2> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<3> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<4> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<5> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<6> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<7> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<8> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<9> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<10> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<11> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<12> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<13> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<14> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type,
				typename ArgumentType<13>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<15> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type,
				typename ArgumentType<13>::parameter_pack::type,
				typename ArgumentType<14>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<16> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type,
				typename ArgumentType<13>::parameter_pack::type,
				typename ArgumentType<14>::parameter_pack::type,
				typename ArgumentType<15>::parameter_pack::type
			>;
		};
#undef parameterPackFoundation
	public:
		std::function<typename F> function;
		typename ParameterPackImpl<NumArguments>::type parameter_pack;

		template<int N> auto& GetParameter() noexcept {
			return getParam(N, parameter_pack);
		};
		template<int N> const auto& GetParameter() const noexcept {
			return getParam(N, parameter_pack);
		};

	private:
		template <int N = 0> static void AddData(typename ParameterPackImpl<NumArguments>::type& d) {};
		template<int N = 0, typename T, typename... Targs> static void AddData(typename ParameterPackImpl<NumArguments>::type& d, T&& value, Targs && ... Fargs) {// recursive function		
			if constexpr (std::is_same<T, void>::value) {
				AddData<N + 1>(d, std::forward<Targs>(Fargs)...);
			}
			else {
				static constexpr bool desire_shared_ptr = !std::is_same<typename get_type<typename ArgumentType<N>::parameter_pack::type>::type, typename ArgumentType<N>::parameter_pack::type>::value;
				static constexpr bool got_shared_ptr = !std::is_same<typename get_type<T>::type, T>::value;

				if constexpr (desire_shared_ptr) {
					// we WANT a shared ptr. Did we get one? 
					if constexpr (got_shared_ptr) {
						getParam(N, d) = std::forward<T>(value);
					}
					else {
						getParam(N, d) = std::make_shared<typename ArgumentType<N>::parameter_pack::underlying_type>(std::forward<T>(value));
					}
				}
				else {
					// we DO NOT want a shared ptr. Did we get one?
					if constexpr (got_shared_ptr) {
						getParam(N, d) = *value;
					}
					else {
						getParam(N, d) = std::forward<T>(value);
					}
				}

				if constexpr (N + 1 < NumArguments) {
					AddData<N + 1>(d, std::forward<Targs>(Fargs)...);
				}
			}
		};
#undef getParam

    public:
		template <typename... Args>
		Function(std::function<F>&& function, Args && ... Fargs) noexcept : function(std::forward<std::function<F>>(function)), parameter_pack() {
			AddData(parameter_pack, std::forward<Args>(Fargs)...);
		};
		ResultType operator()() {
			if constexpr (NumArguments == 16) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>(), GetParameter<13>(), GetParameter<14>(),
					GetParameter<15>()
				);
			}
			else if constexpr (NumArguments == 15) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>(), GetParameter<13>(), GetParameter<14>()
				);
			}
			else if constexpr (NumArguments == 14) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>(), GetParameter<13>()
				);
			}
			else if constexpr (NumArguments == 13) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>()
				);
			}
			else if constexpr (NumArguments == 12) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>()
				);
			}
			else if constexpr (NumArguments == 11) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>()
				);
			}
			else if constexpr (NumArguments == 10) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>()
				);
			}
			else if constexpr (NumArguments == 9) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>()
				);
			}
			else if constexpr (NumArguments == 8) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>()
				);
			}
			else if constexpr (NumArguments == 7) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>()
				);
			}
			else if constexpr (NumArguments == 6) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>()
				);
			}
			else if constexpr (NumArguments == 5) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>()
				);
			}
			else if constexpr (NumArguments == 4) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>()
				);
			}
			else if constexpr (NumArguments == 3) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>()
				);
			}
			else if constexpr (NumArguments == 2) {
				return function(
					GetParameter<0>(), GetParameter<1>()
				);
			}
			else if constexpr (NumArguments == 1) {
				return function(
					GetParameter<0>()
				);
			}
			else if constexpr (NumArguments == 0) {
				return function();
			}
			else {
			    throw(std::runtime_error("The number of arguments and number of parameters did not match"));
			}
		}

		static constexpr size_t NumInputs() noexcept { return NumArguments; };
		static constexpr bool ReturnsNothing() {
			static constexpr bool returnsNothing = std::is_same<ResultType, void>::value;
			return returnsNothing;
		};
		const char* FunctionName() const {
			return function.target_type().name();
		};

	};

	namespace {
		class Action_Interface {
		public:
			virtual ~Action_Interface() = default;

			virtual boost::typeindex::type_info const& type() const noexcept { return boost::typeindex::type_id<void>().type_info(); };
			virtual const char* typeName() const noexcept { return ""; };
			virtual Action_Interface* clone() const noexcept {
				return new Action_Interface();
			};
			virtual fibers::Any& Invoke() noexcept { static fibers::Any staticTemp{}; return staticTemp; };
			virtual fibers::Any& ForceInvoke() noexcept { static fibers::Any staticTemp{}; return staticTemp; };
			virtual const char* FunctionName() const noexcept { return ""; };
			virtual const fibers::Any& Result() const noexcept { static fibers::Any staticTemp{}; return staticTemp; };
			virtual bool IsFinished() const noexcept { return true; };
			virtual bool ReturnsNothing() const noexcept { return true; };
			virtual std::function<void(Action_Interface*)>& deleter() const noexcept {
				static auto deleteFunc{ std::function<void(Action_Interface*)>([](Action_Interface* p) {
					if (p) {
						delete p;
					}
				}) };
				return deleteFunc;
			};
		};
		template<typename ValueType> class Action_Impl final : public Action_Interface {
		private:
			using ReturnType = typename Function<ValueType>::ResultType;
			static constexpr bool returnsNothing{ std::is_same<ReturnType, void>::value };

		public:
			Action_Impl() : Action_Interface() {};
			Action_Impl(Function<ValueType> const& f) noexcept : Action_Interface(), data(f), result(), running(false), finished(false) {};
			Action_Impl(Function<ValueType>&& f) noexcept : Action_Interface(), data(std::forward<Function<ValueType>>(f)), result(), running(false), finished(false) {};

			boost::typeindex::type_info const& type() const noexcept override final {
				return boost::typeindex::type_id<Function<ValueType>>().type_info();
			};
			const char* typeName() const noexcept final {
				return boost::typeindex::type_id<Function<ValueType>>().type_info().name();
			};
			Action_Interface* clone() const noexcept override final {
				return dynamic_cast<Action_Interface*>(new Action_Impl<ValueType>(data));
			};
			fibers::Any& Invoke() noexcept override final {
				if (finished.load()) return result;
				bool compare(false);
				if (running.compare_exchange_strong(compare, true)) {
					if (finished.load()) return result;

					if constexpr (returnsNothing) {
						data();
					}
					else {
						result.container = fibers::Any::CreateContainer(data());
					}
					finished.store(true);
					running.store(false);
				}
				else {
					while (running.load()) {}
				}
				return result;
			};
			fibers::Any& ForceInvoke() noexcept override final {
				if constexpr (returnsNothing) {
					data();
				}
				else {
					result.container = fibers::Any::CreateContainer(data());
				}
				return result;
			};
			const char* FunctionName() const noexcept override final {
				return data.FunctionName();
			};
			const fibers::Any& Result() const noexcept override final {
				return result;
			};
			bool IsFinished() const noexcept override final {
				return finished.load();
			};
			bool ReturnsNothing() const noexcept override final {
				return data.ReturnsNothing();
			};
			std::function<void(Action_Interface*)>& deleter() const noexcept override final {
				static auto deleteFunc{ std::function<void(Action_Interface*)>([](Action_Interface* p) {
					if (p) {
						auto* p2 = dynamic_cast<Action_Impl<ValueType>*>(p);
						if (p2) {
							delete p2;
						}
						else {
							delete p;
						}
					}
				}) };
				return deleteFunc;
			};

			Function<ValueType> data;
			fibers::Any result;
			std::atomic<bool> running;
			std::atomic<bool> finished;
		};
	};

	/* Wrapper for Function<> which allows for sharing and capture of lambdas, functions, etc. that can be evaluated at later times/date/threads/fibers. e.g:
	. auto f = Action([](int i, double x)->int{ return i+x; }, 10, 0.0);
	. int value = f.Invoke().cast();
	. assert(value == 10); */
	class Action {
	public: // structors
		/*! Init */ Action() noexcept : content(nullptr) {};
		/*! Copy */ Action(const Action& other) noexcept : content(other.content ? other.content->clone() : nullptr) {};
		/*! Perfect forwarding */ Action(Action&& other) noexcept : content(std::move(other.content)) {};
		/*! Data Assignment */ template<typename ValueType> explicit Action(Function<ValueType>&& value) : content(ToPtr<ValueType>(std::forward< Function<ValueType>>(value))) {};
		/*! Direct instantiation2 */ template <typename F, typename... Args> Action(F&& function, Args &&... Fargs) : Action(Function(std::function(std::forward<F>(function)), std::forward<Args>(Fargs)...)) {};
		~Action() {
			if (content) {
				auto& deleter = content->deleter();
				deleter(content);
			}
		};

	public: // modifiers
		/*! Copy Data */ Action& operator=(const Action& rhs) = delete;
		/*! Move Data */ Action& operator=(Action&& rhs) = delete;

	public: // queries
		explicit operator bool() { return !IsEmpty(); };
		explicit operator bool() const { return !IsEmpty(); };

		/*! Checks if the Action has been assigned something */
		bool IsEmpty() const noexcept { return !content; };

		template <typename ValueT> static constexpr const char* TypeNameOf() { return utilities::typenames::type_name<ValueT>(); };
		template <typename ValueT> static const boost::typeindex::type_info& TypeOf() { return boost::typeindex::type_id<ValueT>().type_info(); };

		const char* TypeName() const noexcept { 
			static const char* staticVal{""};
			if (content) return content->typeName();
			return staticVal;
		};
		const boost::typeindex::type_info& Type() const noexcept {
			static const boost::typeindex::type_info& staticVal{ boost::typeindex::type_id<void>().type_info() };
			if (content) return content->type();
			return staticVal;
		};

	private:		
		template <class ValueType> static Action_Interface* ToPtr(Function<ValueType>&& rhs) noexcept {
			return dynamic_cast<Action_Interface*>(new Action_Impl<ValueType>(std::forward<Function<ValueType>>(rhs)));			
		};

	public:
		Action_Interface* content;

	public:
		fibers::Any& operator()() {
			return Invoke();
		};
		fibers::Any& Invoke() {
			static fibers::Any staticVal{};
			if (content) return content->Invoke();
			return staticVal;
		};
		fibers::Any& ForceInvoke() {
			static fibers::Any staticVal{};
			if (content) return content->ForceInvoke();
			return staticVal;
		};
		const char* FunctionName() const {
			static const char* staticVal{""};
			if (content) return content->FunctionName();
			return staticVal;
		};
		bool     IsFinished() const {
			if (content) return content->IsFinished();
			return true;
		};
		bool     ReturnsNothing() const {
			if (content) return content->ReturnsNothing();
			return true;
		};
		const fibers::Any& Result() const {
			static fibers::Any staticVal{};
			if (content) return content->Result();
			return staticVal;
		};
	};
};


int Example::ExampleF(int numTasks, int numSubTasks) {
	int* xyzwabc = new int[10000];
	defer(delete[] xyzwabc); // does clean-up on our behalf on scope end
	
	// need a MUCH faster and lighter-weight job / action tool.
	//while (true) 
	{
		{
			printf("SpeedTest (New Functions): ");

			Stopwatch sw; sw.Start();
			for (int i = 0; i < 1000; i++) {
				static auto f1 = [](int const& i, int const& j = 0)->double {
					return j + i;
				};
				static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
					return *i;
				};
				static auto f3 = [](cweeStr& i)->cweeStr {
					return i;
				};

				auto x1 = testing::Function(std::function(f1), 2, 2);
				auto x2 = testing::Function(std::function(f1), 4, 0);
				auto x3 = testing::Function(std::function(f1), 8, 0);
				if (x1() == x2() && x2() != x3()) {}
				else {
					throw(std::runtime_error("Something went wrong 1"));
				}

				auto y1 = testing::Function(std::function(f2), cweeStr("TEST"));
				auto y2 = testing::Function(std::function(f2), std::make_shared<cweeStr>("TEST"));
				auto y3 = testing::Function(std::function(f2), std::make_shared<cweeStr>("TESTING"));
				if (y1() == y2() && y2() != y3()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				}

				auto w1 = testing::Function(std::function(f3), cweeStr("TEST"));
				auto w2 = testing::Function(std::function(f3), std::make_shared<cweeStr>("TEST"));
				auto w3 = testing::Function(std::function(f3), std::make_shared<cweeStr>("TESTING"));
				if (w1() == w2() && w2() != w3()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				}
			}

			cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
			std::cout << timePassed.ToString() << std::endl;
		}
		{
			printf("SpeedTest (Old Functions): ");

			Stopwatch sw; sw.Start();
			for (int i = 0; i < 1000; i++) {
				static auto f1 = [](int const& i, int const& j = 0)->double {
					return j + i;
				};
				static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
					return *i;
				};
				static auto f3 = [](cweeStr& i)->cweeStr {
					return i;
				};

				auto x1 = fibers::Function(std::function(f1), 2, 2);
				auto x2 = fibers::Function(std::function(f1), 4, 0);
				auto x3 = fibers::Function(std::function(f1), 8, 0);
				if (x1() == x2() && x2() != x3()) {}
				else {
					throw(std::runtime_error("Something went wrong 1"));
				};

				auto y1 = fibers::Function(std::function(f2), cweeStr("TEST"));
				auto y2 = fibers::Function(std::function(f2), std::make_shared<cweeStr>("TEST"));
				auto y3 = fibers::Function(std::function(f2), std::make_shared<cweeStr>("TESTING"));
				if (y1() == y2() && y2() != y3()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				};

				auto w1 = fibers::Function(std::function(f3), cweeStr("TEST"));
				auto w2 = fibers::Function(std::function(f3), std::make_shared<cweeStr>("TEST"));
				auto w3 = fibers::Function(std::function(f3), std::make_shared<cweeStr>("TESTING"));
				if (w1() == w2() && w2() != w3()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				};
			}

			cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
			std::cout << timePassed.ToString() << std::endl;
		}

		{
			printf("SpeedTest (New Actions): ");

			Stopwatch sw; sw.Start();
			for (int i = 0; i < 1000; i++) {
				static auto f1 = [](int const& i, int const& j = 0)->double {
					return j + i;
				};
				static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
					return *i;
				};
				static auto f3 = [](cweeStr& i)->cweeStr {
					return i;
				};

				auto x1{ testing::Action(f1, 2, 2) };
				auto x2{ testing::Action(f1, 4, 0) };
				auto x3{ testing::Action(f1, 8, 0) };
				if (x1.ForceInvoke().cast<double>() == x2.ForceInvoke().cast<double>() && x2.ForceInvoke().cast<double>() != x3.ForceInvoke().cast<double>()) {}
				else {
					throw(std::runtime_error("Something went wrong 1"));
				};

				auto y1{ testing::Action(f2, cweeStr("TEST")) };
				auto y2{ testing::Action(f2, std::make_shared<cweeStr>("TEST")) };
				auto y3{ testing::Action(f2, std::make_shared<cweeStr>("TESTING")) };
				if (y1.ForceInvoke().cast<cweeStr>() == y2.ForceInvoke().cast<cweeStr>() && y2.ForceInvoke().cast<cweeStr>() != y3.ForceInvoke().cast<cweeStr>()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				};

				auto w1{ testing::Action(f3, cweeStr("TEST")) };
				auto w2{ testing::Action(f3, std::make_shared<cweeStr>("TEST")) };
				auto w3{ testing::Action(f3, std::make_shared<cweeStr>("TESTING")) };
				if (w1.ForceInvoke().cast<cweeStr>() == w2.ForceInvoke().cast<cweeStr>() && w2.ForceInvoke().cast<cweeStr>() != w3.ForceInvoke().cast<cweeStr>()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				};
			}

			cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
			std::cout << timePassed.ToString() << std::endl;
		}
		{
			printf("SpeedTest (Old Actions): ");

			Stopwatch sw; sw.Start();
			for (int i = 0; i < 1000; i++) {
				static auto f1 = [](int const& i, int const& j = 0)->double {
					return j + i;
				};
				static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
					return *i;
				};
				static auto f3 = [](cweeStr& i)->cweeStr {
					return i;
				};

				auto x1 = fibers::Action(f1, 2, 2);
				auto x2 = fibers::Action(f1, 4, 0);
				auto x3 = fibers::Action(f1, 8, 0);
				if (x1.ForceInvoke().cast<double>() == x2.ForceInvoke().cast<double>() && x2.ForceInvoke().cast<double>() != x3.ForceInvoke().cast<double>()) {}
				else {
					throw(std::runtime_error("Something went wrong 1"));
				};

				auto y1 = fibers::Action(f2, cweeStr("TEST"));
				auto y2 = fibers::Action(f2, std::make_shared<cweeStr>("TEST"));
				auto y3 = fibers::Action(f2, std::make_shared<cweeStr>("TESTING"));
				if (y1.ForceInvoke().cast<cweeStr>() == y2.ForceInvoke().cast<cweeStr>() && y2.ForceInvoke().cast<cweeStr>() != y3.ForceInvoke().cast<cweeStr>()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				};

				auto w1 = fibers::Action(f3, cweeStr("TEST"));
				auto w2 = fibers::Action(f3, std::make_shared<cweeStr>("TEST"));
				auto w3 = fibers::Action(f3, std::make_shared<cweeStr>("TESTING"));
				if (w1.ForceInvoke().cast<cweeStr>() == w2.ForceInvoke().cast<cweeStr>() && w2.ForceInvoke().cast<cweeStr>() != w3.ForceInvoke().cast<cweeStr>()) {}
				else {
					throw(std::runtime_error("Something went wrong 2"));
				};
			}

			cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
			std::cout << timePassed.ToString() << std::endl;
		}
	}

	// wicked
	if (true) {
	    wi::jobsystem::Initialize();

		// Serial test
		{
			auto t = wi::timer("Serial() test: ");
			wi::Spin(100);
			wi::Spin(100);
			wi::Spin(100);
			wi::Spin(100);
			wi::Spin(100);
			wi::Spin(100);
		}

		// Execute test
		{
			auto t = wi::timer("Execute() test: ");
			wi::jobsystem::TaskGroup group;

			group.Queue([](wi::jobsystem::JobArgs args) { wi::Spin(100); });
			group.Queue([](wi::jobsystem::JobArgs args) { wi::Spin(100); });
			group.Queue([](wi::jobsystem::JobArgs args) { wi::Spin(100); });
			group.Queue([](wi::jobsystem::JobArgs args) { wi::Spin(100); });
			group.Queue([](wi::jobsystem::JobArgs args) { wi::Spin(100); });
			group.Queue([](wi::jobsystem::JobArgs args) { wi::Spin(100); });
			group.Wait();
		}

		struct Data
		{
			float m[16];
			void Compute(uint32_t value)
			{
				for (int i = 0; i < 16; ++i)
				{
					m[i] += float(value + i);
				}
			}
		};
		uint32_t dataCount = 400;

		// Loop test:
		{
			Data* dataSet = new Data[dataCount];
			{
				auto t = wi::timer("loop test: ");

				for (uint32_t i = 0; i < dataCount; ++i)
				{
					dataSet[i].Compute(i);
				}
			}
			delete[] dataSet;
		}

		// Dispatch test:
		{
			Data* dataSet = new Data[dataCount];
			{
				auto t = wi::timer("Dispatch() test: ");
				wi::jobsystem::context context;

				const uint32_t groupSize = 10000;
				wi::jobsystem::Dispatch(context, dataCount, groupSize, [&dataSet](wi::jobsystem::JobArgs args) {
					dataSet[args.jobIndex].Compute(1);
				});
				wi::jobsystem::Wait(context);
			}
			delete[] dataSet;
		}
    }

	{
		fibers::TaskScheduler scheduler;
		scheduler.Init();

		for (int j = 1; j < 10; j += 2) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				
				for (int i = 0; i < numLoops; i++) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();				

				fibers::parallel::For(0, numLoops, [&pat, &j](int i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}
			
			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Wicked Fibers, Individual) : ");
			{
				cweeBalancedPattern pat;

				Stopwatch sw; sw.Start();

				auto data = std::make_shared<std::vector< cweeUnion<int, int, decltype(pat)*> >>(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				wi::jobsystem::TaskGroup group;

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->AddValue(i + k, i + k);
					}
				};

				for (int i = 0; i < numLoops; i++) {
					data->operator[](i).get<0>() = i;
					
					group.Queue([=](wi::jobsystem::JobArgs args) {
						todo(&data->operator[](i));
					});
				}
				group.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Wicked Fibers, Dispatch) : ");
			{
				cweeBalancedPattern pat;

				Stopwatch sw; sw.Start();

				auto data = std::make_shared<std::vector< cweeUnion<int, int, decltype(pat)*> >>(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				wi::jobsystem::TaskGroup group;

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->AddValue(i + k, i + k);
					}
				};

				group.Dispatch(numLoops, [=](wi::jobsystem::JobArgs args) {
					data->operator[](args.jobIndex).get<0>() = args.jobIndex;
					todo(&data->operator[](args.jobIndex));
				});

				group.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for(0, numLoops, [&pat, &j](int i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Wicked Fibers, jobs) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();

				wi::jobsystem::JobGroup group;
				for (int i = 0; i < numLoops; i++) {
					group.Queue(wi::jobsystem::Job([&pat, &j](int i) {
						for (int k = 0; k < j; k++) {
							pat.AddValue(i + k, i + k);
						}
					}, (int)i));
				}
				group.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for w/i parallel_for) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for(0, numLoops, [&pat, &j](int i) {
					wi::jobsystem::parallel_for(0, j, [&pat, &j, &i](int k) {
						pat.AddValue(i + k, i + k);
					});
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for { step }) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for(0, numLoops, 1, [&pat, &j](int i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for_each) : ");
			{
				std::vector<int> test(numLoops, 0);
				for (int i = 0; i < numLoops; i++) test[i] = i;

				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for_each(test, [&pat, &j](int& i) {
					for (int k = 0; k < j; k++) {
						pat.AddValue(i + k, i + k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers, Idealized) : ");
			{
				cweeBalancedPattern pat;

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0,j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->AddValue(i + k, i + k);
					}
				};

				Stopwatch sw; sw.Start();
				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers, Likely) : ");
			{
				cweeBalancedPattern pat;

				Stopwatch sw; sw.Start();

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->AddValue(i + k, i + k);
					}
				};

				
				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.GetNumValues() << ")" << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 2) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				std::atomic<int> count;
				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++)
						count.fetch_add(k);
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				std::atomic<int> count;
				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&count, &j](int i) {
					for (int k = 0; k < j; k++)
						count.fetch_add(k);
					});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Wicked Fibers, Individual) : ");
			{
				std::atomic<int> count;

				Stopwatch sw; sw.Start();

				wi::jobsystem::TaskGroup group;

				for (int i = 0; i < numLoops; i++) {
					group.Queue([&](wi::jobsystem::JobArgs args) {
						for (int k = 0; k < j; k++)
							count.fetch_add(k);
					});
				}
				group.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Wicked Fibers, Dispatch) : ");
			{
				std::atomic<int> count;

				Stopwatch sw; sw.Start();

				wi::jobsystem::TaskGroup group;

				group.Dispatch(numLoops, [&count, &j](wi::jobsystem::JobArgs args) {
					for (int k = 0; k < j; k++)
						count.fetch_add(k);
				});

				group.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for) : ");
			{
				std::atomic<int> count;
				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for(0, numLoops, [&count, &j](int i) {
					for (int k = 0; k < j; k++)
						count.fetch_add(k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for w/i parallel_for) : ");
			{
				std::atomic<int> count;
				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for(0, numLoops, [&count, &j](int i) {
					wi::jobsystem::parallel_for(0, j, [&count, &j](int k) {
						count.fetch_add(k);
					});
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for_each) : ");
			{
				std::atomic<int> count;
				Stopwatch sw; sw.Start();

				std::vector<int> n(numLoops, 0);

				wi::jobsystem::parallel_for_each(n, [&count, &j](int& i) {
					for (int k = 0; k < j; k++)
						count.fetch_add(k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << count.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers, Idealized) : ");
			{
				std::atomic<int> pat;

				std::vector< cweeUnion<int, decltype(pat)*> > data(numLoops, cweeUnion<int, decltype(pat)*>(j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& j = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<0>();
					auto& p = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<1>();

					for (k = 0; k < j; k++) {
						p->fetch_add(k);
					}
				};

				Stopwatch sw; sw.Start();
				for (int i = 0; i < numLoops; i++) {
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.load() << ")" << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers, Likely) : ");
			{
				std::atomic<int> pat;

				Stopwatch sw; sw.Start();

				std::vector< cweeUnion<int, decltype(pat)*> > data(numLoops, cweeUnion<int, decltype(pat)*>(j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& j = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<0>();
					auto& p = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<1>();

					for (k = 0; k < j; k++) {
						p->fetch_add(k);
					}
				};

				
				for (int i = 0; i < numLoops; i++) {
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << " (num = " << pat.load() << ")" << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 2) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++) {
						vec[i] = cweeStr(k);
					}
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&vec, &j](int i) {
					for (int k = 0; k < j; k++) {
						vec[i] = cweeStr(k);
					}
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Wicked Fibers, Individual) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();

				wi::jobsystem::TaskGroup group;

				for (int i = 0; i < numLoops; i++) {
					group.Queue([i, j, &vec](wi::jobsystem::JobArgs args) {
						for (int k = 0; k < j; k++)
							vec[i] = cweeStr(k);
					});
				}
				group.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Wicked Fibers, Dispatch) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();

				wi::jobsystem::TaskGroup group;

				group.Dispatch(numLoops, [&vec, &j](wi::jobsystem::JobArgs args) {
					for (int k = 0; k < j; k++)
						vec[args.jobIndex] = cweeStr(k);
				});

				group.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for(0, numLoops, [&vec, &j](int i) {
					for (int k = 0; k < j; k++)
						vec[i] = cweeStr(k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Wicked Fibers, parallel_for_each) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();

				wi::jobsystem::parallel_for_each(vec, [&j](cweeStr& i) {
					for (int k = 0; k < j; k++)
						i = cweeStr(k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers, Idealized) : ");
			{
				std::vector<cweeStr> pat(numLoops, cweeStr("TEST"));

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->operator[](i) = cweeStr(k);
					}
				};

				Stopwatch sw; sw.Start();
				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers, Likely) : ");
			{
				std::vector<cweeStr> pat(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->operator[](i) = cweeStr(k);
					}
				};

				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("\n");
		}
	}







	printf("Job 1\n");
	if (true) {
		fibers::parallel::For(0, numTasks * numSubTasks, [](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {}
		});
	}
	printf("Job 2\n");
	if (true) {
		cweeBalancedPattern pat;
		fibers::JobGroup group;
		for (int i = 0; i < numTasks * numSubTasks; i++) {
			auto job = fibers::Job([&pat](int j) {
				pat.AddUniqueValue(j, j);
			}, (int)i);
			group.Queue(job);
		}
		group.Wait();

		int sizeIs = pat.GetNumValues();
		if (sizeIs == 0) throw(std::runtime_error("Something went wrong"));
	}
	printf("Job 3\n");
	if (true) {
		fibers::parallel::For(0, numTasks * numSubTasks * 2, [](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {}
		});
	}
	printf("Job 4\n");
	if (true) {
		fibers::JobGroup group;
		//fibers::ftl_wrapper::TaskScheduler scheduler;
		for (int i = 0; i < numTasks; i++) {
			auto job = fibers::Job([]() {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}
			});
			group.Queue(job);
			//scheduler.AddTask(job);
		}
		group.Wait();
		//scheduler.Wait();
	}
	printf("Job 5\n");
	if (true) {
		std::atomic<int> numJobsDone;
		fibers::parallel::For(0, numTasks, [&numSubTasks, &numJobsDone](int i) {
			fibers::parallel::For(0, numSubTasks, [&numJobsDone](int j) {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}

				numJobsDone.fetch_add(1);
			});
		});
		if (numJobsDone.load() != numTasks * numSubTasks) {
			printf("Job 5 failed");
		}
	}
	printf("Job 6\n");
	if (true) {
		cweeBalancedPattern pat;
		fibers::parallel::For(0, numTasks, [&numSubTasks, &pat](int i) {
			fibers::parallel::For(0, numSubTasks, [&pat, &i, &numSubTasks](int j) {
				pat.AddUniqueValue(i*numSubTasks + j, i* numSubTasks + j);
			});
		});
		if (pat.GetNumValues() != numSubTasks * numTasks) {
			auto err = cweeStr::printf("Pattern had %i values instead of %i", pat.GetNumValues(), numSubTasks * numTasks);
			printf(err);
		}
	}
	printf("Job 7a\n");
	if (true) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		fibers::containers::vector<int> list;

		fibers::parallel::For(0, numTasks, [&counter, &list, numSubTasks](int i) {
			fibers::parallel::For(0, numSubTasks, [&counter, &list](int j) {
				list.push_back(
					counter->fetch_add(1)
				); // do work
			});
		});
	}
	printf("Job 7b\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));

		fibers::JobGroup group;
		for (int i = 0; i < numTasks; ++i){
			group.Queue(fibers::Job([&numSubTasks, &counter](int i) {
				fibers::JobGroup group;
				for (int j = 0; j < numSubTasks; ++j) {
					group.Queue(fibers::Job([&counter](int j) {
						counter->fetch_add(1);
					}, (int)j));
				}
				group.Wait();
			}, (int)i));
		};
		group.Wait();
	}
	printf("Job 7c\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		auto job = cweeJob([&counter, &numTasks, &numSubTasks]() {
			fibers::JobGroup group;
			for (int i = 0; i < numTasks; ++i) {
				group.Queue(fibers::Job([&numSubTasks, &counter](int i) {
					fibers::JobGroup group;
					for (int j = 0; j < numSubTasks; ++j) {
						group.Queue(fibers::Job([&counter](int j) {
							counter->fetch_add(1);
						}, (int)j));
					}
					group.Wait();
				}, (int)i));
			};
			group.Wait();
		});
		job.AsyncInvoke();
		job.Await();
	}
	printf("Job 7d\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		fibers::containers::vector<int> list;
		auto job = cweeJob([&counter, &numTasks, &numSubTasks, &list]() {
			fibers::parallel::For(0, numTasks, [&counter, &list, numSubTasks](int i) {
				fibers::parallel::For(0, numSubTasks, [&counter, &list](int j) {
					list.push_back(
						counter->fetch_add(1)
					); // do work
				});
			});
		});
		job.AsyncInvoke();
		job.Await();
		return counter->load();
	}

	auto x = fibers::parallel::async([](int x) { return 100.0f; }, 10).wait_get(); // returns 100.0f
	size_t t = fibers::parallel::For(0, numTasks * numSubTasks, [](int i) { return i; }).size();

	printf("Loop done\n");

	return t;
};












#include "../ExcelInterop/Wrapper.h"

class Win32ConsoleSupport {
public:
	static void clearLine() {
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen;
		DWORD written;

		GetConsoleScreenBufferInfo(console, &screen);

		COORD pos = { 0, screen.dwCursorPosition.Y };

		FillConsoleOutputCharacterA(
			console, ' ', screen.dwSize.X * 1, pos, &written
		);
		FillConsoleOutputAttribute(
			console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
			screen.dwSize.X * 1, pos, &written
		);
		SetConsoleCursorPosition(console, pos);
	};
};

/* Linux style terminal for multithread friendly processing. */
static auto GetUserInput() {
	return fibers::parallel::async([]() {
		std::cout << std::endl; // new line, since we'll be refreshing constantly on the current line, then using carriage return to reset the origin.
		cweeStr temp;
		while (1) {
			// if the user hasn't typed anything, keep thinking.                
			Win32ConsoleSupport::clearLine();
			Win32ConsoleSupport::clearLine();
			std::cout << "\r" << temp; // print the current text here, so that it doesn't appear to flash on screen.
			while (!_kbhit()) {
				cweeSysThreadTools::Sys_Yield();
			}
			unsigned char character = _getch(); // grab user key press. 	
			// bool shift_pressed = GetAsyncKeyState(VK_SHIFT);

			if (character == '\n' || character == '\r') { // new line
				std::cout << std::endl;
				break;
			}
			if (character == '\b') { // backspace
				temp = temp.Left(cweeMath::max(0, temp.Length() - 1)); // remove one character
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if ((int)character == 27) { // escape
				temp.Clear();
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if (character == 0 || character == 0xE0) {
				character = _getch();

				if (character == 72) { // up arrow		
					continue;
				}
				if (character == 80) { // down arrow	
					continue;
				}
				if (character == 75) { // left arrow	
					continue;
				}
				if (character == 77) { // right arrow	
					continue;
				}
				continue; // If another not-programmed key was pressed, skip. (i.e. Home button, PGUP, END, PGDOWN)
			}

			temp += (char)character; // all other keys, record to temp repo
		}
		return temp;
	});
};
static cweeStr UserMustSelectFile(fileType_t fileType = fileType_t::ANY_EXT) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().wait_get();// Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return fileSystem->getDataFolder() + "\\" + files[(int)reply];
	}
	else {
		return fileSystem->getDataFolder() + "\\" + reply.BestMatch(files);
	}
};
static cweeStr UserMustSelectFile(cweeStr fileType) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().wait_get();//Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return files[(int)reply];
	}
	else {
		return reply.BestMatch(files);
	}
};

/* Parallel thread to occasionally look for and process toast messages. Sleeps most of the time and wakes up to check for toasts. */
#if 1
static Timer parallel_toast_manager = Timer(0.1, Action(std::function([](cweeStr& title, cweeStr& content) {
	while (cweeToasts->tryGetToast(title, content)) std::cout << cweeStr::printf("\n/* WaterWatch Toast: \t\"%s\": \t\"%s\" */\n\n", title.c_str(), content.c_str());
}), cweeStr(), cweeStr()));
#endif

// Handle async or scripted AppRequests. 
#if 1
static Timer AppLayerRequestProcessor = Timer(0.01, Action(std::function([]() {
	std::pair<
		int, // ID
		cweeUnion<
			cweeStr, // Function Name
			cweeThreadedList<cweeStr>, // Arguments (optional)
			cweeSharedPtr<cweeStr> // result ptr?
		>
	> request = AppLayerRequests->DequeueRequest();
	if (request.first >= 0) {
#pragma warning(disable : 4573)			// the usage of 'cweeStr::Hash' requires the compiler to capture 'this' but the current default capture mode does not allow it
		cweeStr result;
		cweeStr& funcName = request.second.get<0>();
		cweeThreadedList<cweeStr>& args = request.second.get<1>();
		switch (static_cast<size_t>(funcName.Hash())) {
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFile")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFolder")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SavePassword")): // server, username, password
			if (args.Num() >= 3) fileSystem->saveWindowsPassword(args[0], args[1], args[2]);
			else result = "Arguments required: 'account_name', 'user_name', 'password'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_LoadPassword")):
			if (args.Num() >= 2) result = fileSystem->retrieveWindowsPassword(args[0], args[1]);
			else result = "Arguments required: 'account_name', 'user_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_ThemeColor")):
			if (args.Num() >= 1) result = "[255,255,255,255]";
			else result = "Arguments required: 'color_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetUserName")):
			result = "Win32 Project";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetMousePosition")):
			result = "[0,0]";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SaveSetting")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetSetting")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber")): 
		    {
				if (args.Num() >= 2) result = std::to_string(Example::ExampleF(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
				//if (args.Num() >= 2) result = std::to_string(Example::ExampleF(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
				else result = "Arguments required: 'num_tasks', 'num_subtasks'";
			}
			break;
		default:
			// unknown function.
			result = "Function Not Found";
			break;
		}
		AppLayerRequests->FinishRequest(request.first, result); // let the app set the result and end this dequeue
#pragma warning(default : 4573)	
	}
})));
#endif

static cweeStr GetHeaderString() {
	cweeStr toRet = "Welcome to the WaterWatch Sample App.\n";
	toRet += "Data Directory = " + fileSystem->getDataFolder() + "\n";
	toRet += "Begin Scripting:\n\n";
	return toRet;
}

#define AddFunctionToLib(lib, name, todo, ...){ \
	auto varNames = cweeStr(#__VA_ARGS__).RemoveBetween("<", ">").ReplaceInline("*", " ").ReplaceInline("&", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").Split(",").Trim(' ').ReplaceInline("  ", " ").SplitAgain(" ").Trim(' ').GetEveryOtherVar(); \
	lib->add(fun([](__VA_ARGS__) { todo; }, varNames), #name);	\
}

int main() {
	using namespace cwee_units;
	
	std::cout << GetHeaderString() << std::endl;

	cweeStr prevLine = "";
	cweeStr command = "";
	std::shared_ptr<chaiscript::WaterWatch_ChaiScript> engine = std::make_shared<chaiscript::WaterWatch_ChaiScript>();
	while (true) {
		AUTO input = GetUserInput();
		cweeStr str = input.wait_get();//Await().cast();

		if (str == "Fiber") {
			int x = 1000;
			int y = 10;
			int z;
			while (true) {
				z = Example::ExampleF(y, x);
				if (y * x != z) {
					cweeStr err = cweeStr::printf("Something went wrong with the job system and a job returned %i instead of %i", z, y * x);
					throw(std::runtime_error(err.c_str()));
				}


				auto job = cweeJob(Example::ExampleF, (int)y, (int)x);
				job.AsyncInvoke();
				job.Await();

			}
		}
		if (str == "Exit")
		{
			return 0;
		}
		if (str == "Reset") { 
			engine = std::make_shared<chaiscript::WaterWatch_ChaiScript>();
			command = ""; 
			continue; 
		} 
		if (str == "" && prevLine == "") {
			Stopwatch sw;
			sw.Start();

			cweeStr result;
			try {
				AUTO bv = engine->eval(command.c_str());
				result = engine->to_string(bv).c_str();
			} catch (std::exception e) {
				result = e.what();
			}

			sw.Stop();
			std::cout << "Time Elapsed; " << (second_t)sw.Seconds_Passed() << std::endl;
			std::cout << "Current DateTime: " << ((cweeTime)fileSystem->getCurrentTime()).c_str() << std::endl;

			std::cout << result << std::endl;

			command = "";
		}

		command += str;
		command += "\n";
		prevLine = str;
	}
}