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
				void* data{ nullptr };
				while (job_queue->try_pop(job)) {
					args.groupID = job.groupID;

					{
						// Allocate Shared Group Memory
						data = nullptr; {
							if (job.sharedmemory_size > 0) {
								data = fibers::utilities::Mem_Alloc16(job.sharedmemory_size);
								args.sharedmemory = data;

								if (job.GroupStartJob) {
									job.GroupStartJob(args.sharedmemory);
								}								
							}
							else {
								args.sharedmemory = nullptr;
							}
						}

						// Do Group Jobs Until Done or Error is Thrown
						for (j = job.groupJobOffset; !job.ctx->e && j < job.groupJobEnd; ++j) {
							args.jobIndex = j;
							args.groupIndex = j - job.groupJobOffset;
							try {
								job.task(args);
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
						if (data) { 
							if (job.GroupEndJob) {
								job.GroupEndJob(args.sharedmemory);
							}
							fibers::utilities::Mem_Free16(data); 
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
			internal_state.numCores = fibers::utilities::Hardware::GetNumCpuCores();

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

		void Execute(context& ctx, const std::function<void(JobArgs)>& task) noexcept {
			ctx.counter.Increment(); // Context state is updated:
			internal_state.jobQueuePerThread[internal_state.nextQueue.Increment() % internal_state.numThreads].push({ task, &ctx, 0, 0, 1, 0 });
			internal_state.wakeCondition.notify_one(); // 
		};
		void Dispatch(
			context& ctx, 
			uint32_t jobCount, 
			const std::function<void(JobArgs)>& task, 
			size_t sharedmemory_size,
			const std::function<void(void*)>& GroupStartJob, // callback func with memory for type T
			const std::function<void(void*)>& GroupEndJob // callback func with memory for type T
		) noexcept {
			if (jobCount == 0) { return; }

			uint32_t groupCount = std::max<uint32_t>(1, std::min<uint32_t>(jobCount, internal_state.numThreads));
			uint32_t groupSize = jobCount / groupCount;
			while ((uint32_t)(groupCount * groupSize) < jobCount) groupCount++;

			// Context state is updated:
			ctx.counter.Add(groupCount);

			Task job{ task, &ctx, 0, 0, 1, (uint32_t)sharedmemory_size, GroupStartJob, GroupEndJob };

			for (uint32_t groupID = 0; ; ++groupID) { // groupID < groupCount
				// For each group, generate one real job:
				job.groupID = groupID;
				job.groupJobOffset = groupID * groupSize; 
				job.groupJobEnd = std::min(job.groupJobOffset + groupSize, jobCount);
				if (job.groupJobOffset >= job.groupJobEnd) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.
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

		wg->Dispatch<void>(listOfJobs.size(), [listOfJobs](impl::JobArgs args) {
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

