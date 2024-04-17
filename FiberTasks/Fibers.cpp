// FiberPool based on:
// http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
#pragma once

#include "Fibers.h"
#include <assert.h>
#include <stdint.h>
#include <atomic>
#include <array>
#include <thread>

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

namespace fibers {
	namespace utilities {
		int Hardware::GetNumCpuCores() {
			return static_cast<int>(std::thread::hardware_concurrency());
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

	namespace impl {
		inline void work(uint32_t startingQueue) noexcept {
			uint32_t i, j;
			Queue<Task>* job_queue;
			Task job;
			JobArgs args{
				0 // jobIndex
				, 0 // groupID
				, 0 // groupIndex
				, nullptr // sharedmemory
			};

			for (i = 0; i < internal_state.numThreads; ++i) {
				job_queue = &internal_state.jobQueuePerThread[startingQueue % internal_state.numThreads];
				while (job_queue->try_pop(job)) {
					args.groupID = job.groupID;
					if (job.sharedmemory_size > 0) {
						thread_local static std::vector<uint8_t> shared_allocation_data;
						shared_allocation_data.reserve(job.sharedmemory_size);
						args.sharedmemory = shared_allocation_data.data();
					}
					else {
						args.sharedmemory = nullptr;
					}

					{
						for (j = job.groupJobOffset; !job.ctx->e && j < job.groupJobEnd; ++j) {
							args.jobIndex = j;
							args.groupIndex = j - job.groupJobOffset;
							try {
								job.task(args);
							}
							catch (...) {
								if (!job.ctx->e) {
									auto eptr = job.ctx->e.Set(new std::exception_ptr(std::current_exception()));
									if (eptr) {
										delete eptr;
									}
								}
								break;
							}							
						}
					}
					job.ctx->counter.Decrement(); // one got finished
				}
				startingQueue++; // go to next queue
			}
		};
		bool Initialize(uint32_t maxThreadCount) {
			if (internal_state.numThreads > 0)
				return false;
			maxThreadCount = std::max(1u, maxThreadCount);

			// Retrieve the number of hardware threads in this system:
			internal_state.numCores = std::thread::hardware_concurrency();

			// Calculate the actual number of worker threads we want (-1 main thread):
			internal_state.numThreads = std::min(maxThreadCount, std::max(1u, internal_state.numCores - 1));
			internal_state.jobQueuePerThread.reset(new Queue<Task>[internal_state.numThreads]);
			internal_state.threads.reserve(internal_state.numThreads);

			for (uint32_t threadID = 0; threadID < internal_state.numThreads; ++threadID)
			{
				internal_state.threads.emplace_back([threadID] {
					while (internal_state.alive.GetValue()) {
						work(threadID);

						// finished with jobs, put to sleep
						std::unique_lock<decltype(internal_state.wakeMutex)> lock(internal_state.wakeMutex); // std::unique_lock<std::mutex> lock(internal_state.wakeMutex);
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

		void Execute(context& ctx, const std::function<void(JobArgs)>& task) noexcept {
			ctx.counter.Increment(); // Context state is updated:
			internal_state.jobQueuePerThread[internal_state.nextQueue.Increment() % internal_state.numThreads].push({ task, &ctx, 0, 0, 1, 0 });
			internal_state.wakeCondition.notify_one();
		};
		void Dispatch(context& ctx, uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>& task, size_t sharedmemory_size) noexcept {
			if (jobCount == 0) { return; }

			const uint32_t groupCount = DispatchGroupCount(jobCount, groupSize);

			// Context state is updated:
			ctx.counter.Add(groupCount);

			Task job{ task, &ctx, 0, 0, 1, (uint32_t)sharedmemory_size };

			for (uint32_t groupID = 0; groupID < groupCount; ++groupID) {
				// For each group, generate one real job:
				job.groupID = groupID;
				job.groupJobOffset = groupID * groupSize;
				job.groupJobEnd = std::min(job.groupJobOffset + groupSize, jobCount);
				internal_state.jobQueuePerThread[internal_state.nextQueue.Increment() % internal_state.numThreads].push(job);
			}

			internal_state.wakeCondition.notify_all();
		};
		bool IsBusy(const context& ctx) { return ctx.counter.GetValue() > 0; /* Whenever the context label is greater than zero, it means that there is still work that needs to be done */ };
		void HandleExceptions(context& ctx) {
			if (ctx.e) {
				auto eptr = ctx.e.Set(nullptr);
				if (eptr) {
					std::exception_ptr copy{ *eptr };
					delete eptr;
					std::rethrow_exception(std::move(copy));
				}
			}
		};
		void Wait(context& ctx) {
			int j{ 0 };
			if (IsBusy(ctx)) {
				// Wake any threads that might be sleeping:
				internal_state.wakeCondition.notify_all();

				// work() will pick up any jobs that are on stand by and execute them on this thread:
				work(internal_state.nextQueue.Increment() % internal_state.numThreads);

				while (IsBusy(ctx)) {
					// If we are here, then there are still remaining jobs that work() couldn't pick up.
					//	In this case those jobs are not standing by on a queue but currently executing
					//	on other threads, so they cannot be picked up by this thread.
					//	Allow to swap out this thread by OS to not spin endlessly for nothing
					if (++j >= 40)
						std::this_thread::yield(); 
				}
			}
			HandleExceptions(ctx);			
		};
	};

	JobGroup::JobGroup() : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<impl::TaskGroup>(new impl::TaskGroup())))) {};
	JobGroup::JobGroup(Job const& job) : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<impl::TaskGroup>(new impl::TaskGroup())))) { Queue(job); };
	void JobGroup::JobGroupImpl::Queue(Job const& job) {
		std::shared_ptr<impl::TaskGroup> wg = std::static_pointer_cast<impl::TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Queue([impl = job.impl](impl::JobArgs args) {
			if (impl) {
				impl->Invoke();
			}
		});
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::Queue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<impl::TaskGroup> wg = std::static_pointer_cast<impl::TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));

		wg->Dispatch(listOfJobs.size(), [listOfJobs](impl::JobArgs args) {
			listOfJobs[args.jobIndex].Invoke();
			});

		for (Job const& j : listOfJobs) {
			jobs.push_back(j);
		}
	};
	void JobGroup::JobGroupImpl::Wait() {
		std::shared_ptr<impl::TaskGroup> wg = std::static_pointer_cast<impl::TaskGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Wait();
	};
	[[nodiscard]] JobGroup Job::AsyncInvoke() { return JobGroup(*this); };
	void Job::AsyncFireAndForget() {
		if (this->IsStatic()) {
			std::shared_ptr< impl::context > ctx = std::make_shared<impl::context>();
			ctx->counter.Increment();
			impl::Execute(*ctx, [action = this->impl, context = ctx](impl::JobArgs args) {
				context->counter.Decrement();
				if (action) {
					action->Invoke();
				}
			});
		}
		else {
			throw(std::runtime_error("Only static or stateless functions can be called from a fire-and-forget methodology."));
		}
	};

};

namespace {
	class MultithreadingInstanceManagerImpl final : public MultithreadingInstanceManager {
	public:
		MultithreadingInstanceManagerImpl() : MultithreadingInstanceManager() {
			fibers::impl::Initialize();
		};
		~MultithreadingInstanceManagerImpl() {
			fibers::impl::ShutDown();
		};
	};
};
/* Instances the fiber system, and destroys it if the DLL / library is unloaded. */
std::shared_ptr<MultithreadingInstanceManager> multithreadingInstance = std::static_pointer_cast<MultithreadingInstanceManager>(std::make_shared<MultithreadingInstanceManagerImpl>());

namespace fibers {
	namespace synchronization {
		namespace impl {
			static CriticalMutexLock atomic_number_lock_impl;
			CriticalMutexLock* atomic_number_lock = &atomic_number_lock_impl;
		};
	};
};