#pragma once
#include "Parallel.h"

// FiberPool based on:
// http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
#include <assert.h>
#include <stdint.h>
#include <atomic>
#include <array>
#include <thread>
#include <iostream>

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

namespace fibers::platform {
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
};

#ifdef PLATFORM_LINUX
#include <pthread.h>
#endif // PLATFORM_LINUX


namespace GoodLang {
	namespace parallel {
		static std::atomic_bool m_rethrowExceptions{ true };
		bool options::RethrowsExceptions() {
			return m_rethrowExceptions;
		}; // Get
		void options::RethrowsExceptions(bool TF) {
			m_rethrowExceptions = TF;
		}; // Set

	};
};


namespace GoodLang {
	__forceinline static void* Mem_Alloc64(const size_t& size) { if (!size) return nullptr; const size_t paddedSize = (size + 63) & ~63; return ::_aligned_malloc(paddedSize, 64); };
	__forceinline static void* Mem_Alloc16(const size_t& size) { if (!size) return nullptr; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16); };
	__forceinline static void  Mem_Free64(void* ptr) { if (ptr) ::_aligned_free(ptr); };
	__forceinline static void  Mem_Free16(void* ptr) { if (ptr) ::_aligned_free(ptr); };
	__forceinline static void* Mem_ClearedAlloc(const size_t& size) { void* mem = Mem_Alloc16(size); ::memset(mem, 0, size); return mem; };
	__forceinline static void  Mem_Free(void* ptr) { Mem_Free16(ptr); }
	__forceinline static void* Mem_Alloc(const size_t size) { return Mem_ClearedAlloc(size); }
	__forceinline static char* Mem_CopyString(const char* in) { size_t L{ strlen(in) + 1 }; char* out = (char*)Mem_Alloc(L); ::strncpy(out, in, L - 1);  return out; };

	class Hardware {
	public:
		static int GetNumCpuCores() {

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
		static float GetPercentCpuLoad() {
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

	private:
		enum e_LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL {
			e_localRelationProcessorCore,
			e_localRelationNumaNode,
			e_localRelationCache,
			e_localRelationProcessorPackage
		};
		static __forceinline DWORD CountSetBits(ULONG_PTR bitMask) {
			DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
			DWORD bitSetCount = 0;
			ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

			for (DWORD i = 0; i <= LSHIFT; i++) {
				bitSetCount += ((bitMask & bitTest) ? 1 : 0);
				bitTest /= 2;
			}

			return bitSetCount;
		};
		class CpuInfo_t {
		public:
			CpuInfo_t() : processorPackageCount(0), processorCoreCount(0), logicalProcessorCount(0), numaNodeCount(0), cacheLevel() {};
			int processorPackageCount;
			int processorCoreCount;
			int logicalProcessorCount; // the value we care about -- indicated the number of actual "threads" that can run at once. 
			int numaNodeCount;
			class cacheInfo_t {
			public:
				cacheInfo_t() : count(0), associativity(0), lineSize(0), size(0) {};
				int count;
				int associativity;
				int lineSize;
				int size;
			};
			cacheInfo_t cacheLevel[3];
		};
	};



	namespace impl {

		inline void work(uint32_t startingQueue, const context* parentCtx) noexcept {
			uint32_t i, j, threadID;
			Queue<Task>* job_queue;
			Task job;
			JobArgs args{
				0 // jobIndex
				, 0 // groupID
				, 0 // groupIndex
				, nullptr // sharedmemory
			};

			uint32_t sizeOfData{ 0 };
			void* data{ nullptr };
			defer(if (data) { Mem_Free16(data); });

			bool didWork = true;
			while (didWork && (!parentCtx || (parentCtx && IsBusy(*parentCtx)))) {
				didWork = false;
				for (i = 0; i < internal_state.numThreads && (!parentCtx || (parentCtx && IsBusy(*parentCtx))); i++) {
					threadID = (i + startingQueue) % internal_state.numThreads;
					job_queue = &internal_state.jobQueuePerThread[threadID];
					while (job_queue->try_pop(job)) {
						didWork = true;
						if (!job.ctx->e) { // if another group threw an error, do not process this group at all.
							args.groupID = job.groupID;
							// Allocate Shared Group Memory (heap allocates only when more memory is needed than was previously used).
							{
								if (job.sharedmemory_size > 0) {
									if (sizeOfData < job.sharedmemory_size) {
										if (data) Mem_Free16(data);
										data = Mem_Alloc16(job.sharedmemory_size);
										sizeOfData = job.sharedmemory_size;
									}

									::memset(data, 0, job.sharedmemory_size);

									args.sharedmemory = data;

									if (job.GroupStartJob) {
										job.GroupStartJob->operator()(args.sharedmemory);
									}
								}
								else {
									args.sharedmemory = nullptr;
								}
							}

							// Do Group Jobs Until Done or Error is Thrown
							auto& ToDo = *job.task;
							for (j = job.groupJobOffset; !job.ctx->e && j < job.groupJobEnd; ++j) {
								args.jobIndex = j;
								args.groupIndex = j - job.groupJobOffset;
								try {
									ToDo(args);
								}
								catch (...) {
									if (!job.ctx->e) {
										auto eptr = job.ctx->e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
										if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
											delete eptr;
										}
									}
									break;
								}
							}

							// Deallocate Shared Group Memory
							if (args.sharedmemory && job.GroupEndJob) job.GroupEndJob->operator()(args.sharedmemory);
						}
						job.ctx->counter.Decrement(); // one group got finished, regardless of the outcome.

						if (!parentCtx || (parentCtx && IsBusy(*parentCtx))) {

						}
						else {
							break;
						}
					}
					startingQueue++; // go to next queue
				}
			}
		};
		inline void work(uint32_t startingQueue) noexcept {
			uint32_t i, j, threadID;
			Queue<Task>* job_queue;
			Task job;
			JobArgs args{
				0 // jobIndex
				, 0 // groupID
				, 0 // groupIndex
				, nullptr // sharedmemory
			};

			uint32_t sizeOfData{ 0 };
			void* data{ nullptr };
			defer(if (data) { Mem_Free16(data); });

			bool didWork = true;
			while (didWork) {
				didWork = false;
				for (i = 0; i < internal_state.numThreads; i++) {
					threadID = (i + startingQueue) % internal_state.numThreads;
					job_queue = &internal_state.jobQueuePerThread[threadID];
					while (job_queue->try_pop(job)) {
						didWork = true;
						if (!job.ctx->e) { // if another group threw an error, do not process this group at all.
							args.groupID = job.groupID;
							// Allocate Shared Group Memory (heap allocates only when more memory is needed than was previously used).
							{
								if (job.sharedmemory_size > 0) {
									if (sizeOfData < job.sharedmemory_size) {
										if (data) Mem_Free16(data);
										data = Mem_Alloc16(job.sharedmemory_size);
										sizeOfData = job.sharedmemory_size;
									}

									::memset(data, 0, job.sharedmemory_size);

									args.sharedmemory = data;

									if (job.GroupStartJob) {
										job.GroupStartJob->operator()(args.sharedmemory);
									}
								}
								else {
									args.sharedmemory = nullptr;
								}
							}

							// Do Group Jobs Until Done or Error is Thrown
							auto& ToDo = *job.task;
							for (j = job.groupJobOffset; !job.ctx->e && j < job.groupJobEnd; ++j) {
								args.jobIndex = j;
								args.groupIndex = j - job.groupJobOffset;
								try {
									ToDo(args);
								}
								catch (...) {
									if (!job.ctx->e) {
										auto eptr = job.ctx->e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
										if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
											delete eptr;
										}
									}
									break;
								}
							}

							// Deallocate Shared Group Memory
							if (args.sharedmemory && job.GroupEndJob) job.GroupEndJob->operator()(args.sharedmemory);

						}
						job.ctx->counter.Decrement(); // one group got finished, regardless of the outcome.
					}
					startingQueue++; // go to next queue
				}
			}
		};

		bool Initialize(uint32_t maxThreadCount) {
			if (internal_state.numThreads > 0) return false;
			maxThreadCount = std::max(1u, maxThreadCount);

			// Retrieve the number of hardware threads in this system:
			internal_state.numCores = Hardware::GetNumCpuCores();

			// Calculate the actual number of worker threads we want (-1 main thread):
			internal_state.numThreads = std::min(maxThreadCount, std::max(1u, internal_state.numCores - 1));
			internal_state.jobQueuePerThread.reset(new Queue<Task>[internal_state.numThreads]());
			// internal_state.currentTaskPerThread.reset(new Task[internal_state.numThreads]);
			internal_state.threads.reserve(internal_state.numThreads);

			for (uint32_t threadID = 0; threadID < internal_state.numThreads; ++threadID) {
				internal_state.threads.emplace_back([threadID] {
					// pre-warm this thread's heap
					for (int i = 0; i < 100000; i++) delete (new int(5));

					while (internal_state.alive.GetValue()) {
						// Work until no more jobs are found
						work(threadID);

						// go to sleep, to be awoken when new jobs are added
						auto lock{ std::unique_lock(internal_state.wakeMutex) };
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

			return true;
		};
		void ShutDown() { internal_state.ShutDown(); };

		void Execute(context& ctx, std::function<void(JobArgs const&)> task) noexcept {
			ctx.counter.Increment(); // Context state is updated:
			internal_state.jobQueuePerThread[internal_state.nextQueue.Increment() % internal_state.numThreads].push({ std::make_shared<std::function<void(JobArgs const&)>>(std::move(task)), &ctx, 0, 0, 1, 0 });
			internal_state.wakeCondition.notify_one(); // 
		};
		void Dispatch(
			context& ctx,
			uint32_t jobCount,
			std::function<void(JobArgs const&)> task
		) noexcept {
			if (jobCount == 0) { return; }

			uint32_t groupCount = std::max<uint32_t>(1, std::min<uint32_t>(jobCount, internal_state.numThreads * 4));
			uint32_t groupSize = jobCount / groupCount;
			while ((uint32_t)(groupCount * groupSize) < jobCount) groupCount++;

			// context state is updated to its maximum:
			ctx.counter.Add(groupCount);

			// create the overarching task:
			Task job{
				std::make_shared<std::function<void(JobArgs const&)>>(std::move(task)),
				&ctx, 0, 0, 1, 0, nullptr, nullptr
			};

			// submit groups evenly into the thread pool:
			for (uint32_t groupID = 0; ; ++groupID) { // groupID < groupCount
				// For each group, generate one real job:
				job.groupID = groupID;
				job.groupJobOffset = groupID * groupSize;
				job.groupJobEnd = std::min(job.groupJobOffset + groupSize, jobCount);
				if (job.groupJobOffset >= job.groupJobEnd) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.
				internal_state.jobQueuePerThread[internal_state.nextQueue.Increment() % internal_state.numThreads].push(job);
			}

			// wake any threads that might be sleeping:
			internal_state.wakeCondition.notify_all();
		};
		void Dispatch(
			context& ctx,
			uint32_t jobCount,
			std::function<void(JobArgs const&)> task,
			size_t sharedmemory_size,
			std::function<void(void*)> GroupStartJob, // callback func with memory for type T
			std::function<void(void*)> GroupEndJob // callback func with memory for type T
		) noexcept {
			if (jobCount == 0) { return; }

			uint32_t groupCount = std::max<uint32_t>(1, std::min<uint32_t>(jobCount, internal_state.numThreads * 4));
			uint32_t groupSize = jobCount / groupCount;
			while ((uint32_t)(groupCount * groupSize) < jobCount) groupCount++;

			// context state is updated to its maximum:
			ctx.counter.Add(groupCount);

			// create the overarching task:
			Task job{
				std::make_shared<std::function<void(JobArgs const&)>>(std::move(task)),
				&ctx, 0, 0, 1, (uint32_t)sharedmemory_size,
				std::make_shared<std::function<void(void*)>>(std::move(GroupStartJob)),
				std::make_shared<std::function<void(void*)>>(std::move(GroupEndJob))
			};

			// submit groups evenly into the thread pool:
			for (uint32_t groupID = 0; ; ++groupID) { // groupID < groupCount
				// For each group, generate one real job:
				job.groupID = groupID;
				job.groupJobOffset = groupID * groupSize;
				job.groupJobEnd = std::min(job.groupJobOffset + groupSize, jobCount);
				if (job.groupJobOffset >= job.groupJobEnd) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.
				internal_state.jobQueuePerThread[internal_state.nextQueue.Increment() % internal_state.numThreads].push(job);
			}

			// wake any threads that might be sleeping:
			internal_state.wakeCondition.notify_all();
		};
		bool IsBusy(const context& ctx) { return ctx.counter.GetValue() > 0; /* Whenever the context label is greater than zero, it means that there is still work that needs to be done */ };
		void HandleExceptions(context& ctx) {
			if (ctx.e) {
				auto eptr = ctx.e.Set(nullptr);
				if (eptr) {
					std::exception_ptr copy{ *eptr };
					delete eptr;
					if (parallel::options::RethrowsExceptions()) {
						std::rethrow_exception(std::move(copy));
					}
					else {
						// should at least announce the error...
						std::cout << ExceptionHandling::what(copy) << std::endl;
					}
				}
			}
		};
		void Wait(context& ctx) {
#if 0 // Does not support jobs calling jobs 
			internal_state.wakeCondition.notify_all();
			work(internal_state.nextQueue.Increment() % internal_state.numThreads, &ctx);
			while (IsBusy(ctx)) { std::this_thread::yield(); };
			HandleExceptions(ctx);
#else // supports jobs calling jobs
			int i{ 0 };
			internal_state.wakeCondition.notify_all(); // Wake any threads that might be sleeping:
			while (IsBusy(ctx)) { // Do work
				if (++i < 40) {
					std::this_thread::yield();
				}
				else {
					// internal_state.wakeCondition.notify_all(); // Wake any threads that might be sleeping:
					work(internal_state.nextQueue.Increment() % internal_state.numThreads, &ctx);
				}
			}
			HandleExceptions(ctx);  // re-throw any exceptions that were caught during the workload
#endif
		};
	};

	JobGroup::JobGroup() :
		impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<impl::TaskGroup>(new impl::TaskGroup()))))
	{};
	JobGroup::JobGroup(Job const& job) :
		impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<impl::TaskGroup>(new impl::TaskGroup()))))
	{
		Queue(job);
	};
	void JobGroup::JobGroupImpl::Queue(Job const& job) {
		std::shared_ptr<impl::TaskGroup> wg = std::static_pointer_cast<impl::TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Queue([impl = job](impl::JobArgs const& args) {
			impl.Invoke();
		});
		last_job = job;
	};
	void JobGroup::JobGroupImpl::Queue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<impl::TaskGroup> wg = std::static_pointer_cast<impl::TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));

		wg->Dispatch(listOfJobs.size(), [listOfJobs](impl::JobArgs const& args) {
			listOfJobs[args.jobIndex].Invoke();
		});

		if (listOfJobs.size() > 0)
			last_job = listOfJobs[listOfJobs.size() - 1];
	};
	void JobGroup::JobGroupImpl::Wait() {
		std::shared_ptr<impl::TaskGroup> wg = std::static_pointer_cast<impl::TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Wait();
	};
	[[nodiscard]] JobGroup Job::AsyncInvoke() { return JobGroup(*this); };

};

namespace {
	class MultithreadingInstanceManagerImpl final : public GoodLang::MultithreadingInstanceManager {
	public:
		MultithreadingInstanceManagerImpl() : GoodLang::MultithreadingInstanceManager() {
			GoodLang::impl::Initialize();
		};
		~MultithreadingInstanceManagerImpl() {
			GoodLang::impl::ShutDown();
		};
	};
};
/* Instances the fiber system, and destroys it if the DLL / library is unloaded. */
std::shared_ptr<GoodLang::MultithreadingInstanceManager> multithreadingInstance = std::static_pointer_cast<GoodLang::MultithreadingInstanceManager>(std::make_shared<MultithreadingInstanceManagerImpl>());

