#pragma once
#include "Parallel.h"
#include "stopwatch.h"
#include "util.h"

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

#include "util.h"
#include "atomic_allocator.h"
#include "ticket_dispensor.h"
#include "atomic_vector.h"
#include "atomic_stack.h"
#include "atomic_queue.h"

namespace GL {
	namespace parallel {
		namespace impl {
			struct thread_wrap {
				std::thread thread;
				size_t thread_hash;
				size_t thread_index;
				// Ratio of "work" this CPU core is capable of. E.g. 0.25 would indicate this core is capable of 25% of the workload of this CPU. 
				// Intended to help identify the primary cores vs. hyperthreaded cores, as their capacity for work is noticibly different. 
				double relative_speed; 
			};

			struct InternalState {
				enum alive_state {
					is_dead = 0,
					is_booting = 1,
					is_alive = 2,
					is_debooting = 3
				};
				size_t numCores = 0;
				size_t numThreads = 0;

				// GL::aba_problem::stack< thread_task > jobQueue;
				GL::atomic_parallel_queue< thread_task > jobQueue;

				std::atomic<alive_state> alive{ is_dead };
				std::condition_variable wakeCondition;
				std::mutex wakeMutex;

				std::vector<thread_wrap> threads;

				void ShutDown() {
					if (alive == is_dead) return;
					else {
						auto prevS = alive_state::is_alive;
						while (!alive.compare_exchange_strong(prevS, is_debooting)) {}
						bool wake_loop = true;
						std::thread waker([&] {
							while (wake_loop) wakeCondition.notify_all(); // wakes up sleeping worker threads
							});
						for (auto& thread : threads) {
							thread.thread.join();
						}
						wake_loop = false;
						waker.join();
						//consumer_tokens.for_each([](auto* p) { if (p) delete p; });
						threads.clear();

						alive = is_dead;
					}
				};
				~InternalState() {
					ShutDown();
				};
			} static internal_state;

			bool try_get_job(decltype(InternalState::jobQueue)& queue, thread_task& job) {
				return queue.try_pop(job);
			};
			void submit_job(decltype(InternalState::jobQueue)& queue, thread_task& job) {
				queue.push(job);
			};
			void do_task(thread_task& task, impl::job_argument& args, size_t& sizeOfData, void*& data) {
				if (task.task && !task.ctx->e) { // if another group threw an error, do not process this group at all.
					args.group_id = task.group_id;
					args.task_memory = task.task_memory;
					// Allocate Shared Group Memory (heap allocates only when more memory is needed than was previously used).
					{
						if (task.group_memory_size > 0) {
							if (sizeOfData < ((task.group_memory_size + 15) & ~15)) {
								if (data) ::_aligned_free(data);
								sizeOfData = (task.group_memory_size + 15) & ~15;
								data = ::_aligned_malloc(sizeOfData, 16);
							}
							::memset(data, 0, sizeOfData);
							args.group_memory = data;
							if (task.group_start_job) {
								task.group_start_job(args.group_memory);
							}
						}
						else {
							args.group_memory = nullptr;
						}
					}

					// Do Group Jobs Until Done or Error is Thrown
					for (size_t j = task.group_job_offset; !task.ctx->e && j < task.group_job_end; ++j) {
						args.group_index = (args.job_index = j) - task.group_job_offset;
						try {
							task.task(args);
						}
						catch (...) {
							task.ctx->catch_exception();
							break;
						}
					}

					// Deallocate Shared Group Memory
					if (args.group_memory && task.group_end_job) task.group_end_job(args.group_memory);
				}
				--task.ctx->counter;
			};
		
			// potentially (not always) called by the main thread
			void work(const dispatch_context* parentCtx = nullptr) noexcept {
				size_t threadID{ util::get_thread_id() }, sizeOfData{ 0 };
				void* data{ nullptr };
				thread_task task;
				impl::job_argument args{ 
					0 // jobIndex
					, 0 // groupID
					, 0 // groupIndex
					, nullptr // sharedmemory
					, nullptr
				};

				//auto*& consumer = *internal_state.consumer_tokens;
				//if (!consumer) consumer = new moodycamel::ConsumerToken(internal_state.jobQueue);

				if (parentCtx) {
					// work until this job is completed
					while (parentCtx->is_busy()) {
						if (try_get_job(internal_state.jobQueue, task)) {
							do_task(task, args, sizeOfData, data);
						}
					}
					if (sizeOfData > 0) ::_aligned_free(data);
				}
				else {
					// work until there are no jobs left
					long long last_job_time{ GL::util::get_current_epoch() };
					while (true) {
						while (try_get_job(internal_state.jobQueue, task)) {							
							do_task(task, args, sizeOfData, data);
							last_job_time = GL::util::get_current_epoch();
						}
						if ((GL::util::get_current_epoch() - last_job_time) > 16) {
							break;
						}
					}			
					if (data && (sizeOfData > 0)) ::_aligned_free(data);
				}
			};

			void DoDispatch(thread_task job, size_t groupSize, size_t jobCount) {
				// submit groups evenly into the thread pool:
				for (size_t groupID = 0; ; ++groupID) { // groupID < groupCount
					// For each group, generate one real job:
					job.group_id = groupID;
					job.group_job_offset = groupID * groupSize;
					job.group_job_end = std::min(job.group_job_offset + groupSize, jobCount);
					if (job.group_job_offset >= job.group_job_end) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.				
					internal_state.jobQueue.push(job);
				}
				internal_state.wakeCondition.notify_all();
			};

			void initialize_if_necessary() {
				if (internal_state.alive.load() == InternalState::alive_state::is_alive) return;
				auto prevS = InternalState::alive_state::is_dead;
				if (internal_state.alive.compare_exchange_strong(prevS, InternalState::alive_state::is_booting)) {
					internal_state.numCores = util::get_hardware_thread_count();
					// Calculate the actual number of worker threads we want (-1 main thread):
					internal_state.numThreads = std::max<long long>(1, internal_state.numCores - 1);

					std::atomic<size_t> boot_count{ internal_state.numThreads };
					internal_state.threads.reserve(internal_state.numThreads);
					for (size_t threadID = 0; threadID < internal_state.numThreads; ++threadID) {
						internal_state.threads.emplace_back(thread_wrap{ std::thread{ [threadID, &boot_count] {
							// pre-warm this thread's heap
							for (int i = 0; i < 100000; i++) delete (new int(5));

							internal_state.threads[threadID].thread_hash = std::hash<std::thread::id>{}(std::this_thread::get_id());
							internal_state.threads[threadID].thread_index = GL::util::get_thread_id();

							auto start_time = GL::util::get_current_epoch();
							volatile std::atomic<long> count{ 0 };
							for (volatile size_t i = 0; i < 1000000; ++i) {
								++count;
							}
							internal_state.threads[threadID].relative_speed = 1.0 / (double)((GL::util::get_current_epoch() - start_time) + 1);

							--boot_count;
							if (1) {
								thread_task temp;
								(void)internal_state.jobQueue.try_pop(temp);
							}

							while (internal_state.alive == InternalState::alive_state::is_booting) {} // wait until we stop booting... 
							while (internal_state.alive == InternalState::alive_state::is_alive) {
								work(); // Work until no more jobs are found		
								auto lock{ std::unique_lock(internal_state.wakeMutex) };
								internal_state.wakeCondition.wait(lock);
							}
						} }, 0, 0 });
						std::thread& worker = internal_state.threads.back().thread;

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
						std::wstring wthreadname = L"GL::thread_" + std::to_wstring(threadID);
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
						std::string thread_name = "GL::job::" + std::to_string(threadID);
						ret = pthread_setname_np(worker.native_handle(), thread_name.c_str());
						if (ret != 0)
							handle_error_en(ret, std::string(" pthread_setname_np[" + std::to_string(threadID) + ']').c_str());
#undef handle_error_en
#endif // _WIN32
					}

					while (boot_count.load() > 0) {}

					double total = 0;
					double best_speed = std::numeric_limits<double>::max();
					double worst_speed = 0;
					for (size_t threadID = 0; threadID < internal_state.numThreads; ++threadID) {
						double this_speed = internal_state.threads[threadID].relative_speed;
						total += this_speed;
					}
					for (size_t threadID = 0; threadID < internal_state.numThreads; ++threadID) {
						internal_state.threads[threadID].relative_speed /= total;
					}
					prevS = InternalState::alive_state::is_booting;
					internal_state.alive.compare_exchange_strong(prevS, InternalState::alive_state::is_alive);
				}
				else {
					while (internal_state.alive.load() != InternalState::alive_state::is_alive) {};
				}
			};

			static thread_local long long wait_depth{ 0 }; // allows detection of when job dispatch may be from within an existing job

			void Dispatch(
				dispatch_context& ctx,
				size_t jobCount,
				void (*Task)(job_argument const&),
				void* task_memory,
				size_t sharedmemory_size,
				void (*group_start_job)(void* const&), // callback func with memory for type T
			    void (*group_end_job)(void* const&) // callback func with memory for type T
			) noexcept {
				if (jobCount == 0) { return; }
				initialize_if_necessary();

				// if a job dispatch is within an existing job, groupCount needs to narrow down to prevent overflow.
				size_t groupCount = std::min(internal_state.numThreads * 32, static_cast<size_t>(jobCount));
				switch (wait_depth) {
				case 0: break; // internal_state.numThreads << 3; break;
					//case 1: groupCount = internal_state.numThreads << 0; break;
					//default: groupCount = internal_state.numThreads >> wait_depth; break;
				default: groupCount = 1; break;
				}
				// groupCount = std::max<size_t>(1, std::min<size_t>(jobCount, groupCount));
				size_t groupSize = jobCount / groupCount;
				while ((size_t)(groupCount * groupSize) < jobCount) groupCount++;

				// context state is updated to its maximum:
				ctx.counter += groupCount;

				if ((wait_depth > 0) || (groupCount <= 1)) {
					// do the work directly:
					void* data{ nullptr };
					size_t threadID{ util::get_thread_id() }, sizeOfData{ 0 };
					thread_task task;
					impl::job_argument args{
						0 // jobIndex
						, 0 // groupID
						, 0 // groupIndex
						, nullptr // sharedmemory
						, task_memory // task memory
					};

					task.ctx = &ctx;
					task.task = Task;
					task.group_end_job = group_end_job;
					task.group_start_job = group_start_job;
					task.group_memory_size = sharedmemory_size;
					task.task_memory = task_memory;
					for (size_t groupID = 0; ; ++groupID) { // groupID < groupCount
						// For each group, generate one real job:
						auto groupJobOffset = groupID * groupSize;
						auto groupJobEnd = std::min<size_t>(groupJobOffset + groupSize, jobCount);
						if (groupJobOffset >= groupJobEnd) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.

						task.group_id = groupID;
						task.group_job_offset = groupJobOffset;
						task.group_job_end = groupJobEnd;

						do_task(task, args, sizeOfData, data);
					}

					// work until this job is completed
					while (ctx.is_busy()) {
						if (try_get_job(internal_state.jobQueue, task)) {
							do_task(task, args, sizeOfData, data);
						}
					}

					if (sizeOfData > 0) ::_aligned_free(data);
				}
				else {
					DoDispatch(thread_task{
						&ctx, Task,
						0, 0, 1, sharedmemory_size,
						group_start_job,
						group_end_job, 
						task_memory
						// std::hash<std::thread::id>{}(std::this_thread::get_id())
				    }
					, groupSize, jobCount);
				}
			};

			void Dispatch(
				dispatch_context& ctx,
				size_t jobCount,
				void (*Task)(job_argument const&),
				void* task_memory
			) noexcept {
				if (jobCount == 0) { return; }
				initialize_if_necessary();

				// if a job dispatch is within an existing job, groupCount needs to narrow down to prevent overflow.
				size_t groupCount = std::min(internal_state.numThreads * 32, static_cast<size_t>(jobCount));
				switch (wait_depth) {
				case 0: break; // internal_state.numThreads << 3; break;
					//case 1: groupCount = internal_state.numThreads << 0; break;
					//default: groupCount = internal_state.numThreads >> wait_depth; break;
				default: groupCount = 1; break;
				}
				// groupCount = std::max<size_t>(1, std::min<size_t>(jobCount, groupCount));
				size_t groupSize = jobCount / groupCount;
				while ((size_t)(groupCount * groupSize) < jobCount) groupCount++;

				// context state is updated to its maximum:
				ctx.counter += groupCount;

				if ((wait_depth > 0) || (groupCount <= 1)) {
					// do the work directly:
					void* data{ nullptr };
					size_t threadID{ util::get_thread_id() }, sizeOfData{ 0 };
					thread_task task;
					impl::job_argument args{
						0 // jobIndex
						, 0 // groupID
						, 0 // groupIndex
						, nullptr // sharedmemory
						, task_memory // task memory
					};

					task.ctx = &ctx;
					task.task = Task;
					task.group_end_job = nullptr;
					task.group_start_job = nullptr;
					task.group_memory_size = 0;
					task.task_memory = task_memory;
					for (size_t groupID = 0; ; ++groupID) { // groupID < groupCount
						// For each group, generate one real job:
						auto groupJobOffset = groupID * groupSize;
						auto groupJobEnd = std::min<size_t>(groupJobOffset + groupSize, jobCount);
						if (groupJobOffset >= groupJobEnd) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.

						task.group_id = groupID;
						task.group_job_offset = groupJobOffset;
						task.group_job_end = groupJobEnd;

						do_task(task, args, sizeOfData, data);
					}

					// work until this job is completed
					while (ctx.is_busy()) {
						if (try_get_job(internal_state.jobQueue, task)) {
							do_task(task, args, sizeOfData, data);
						}
					}

					if (sizeOfData > 0) ::_aligned_free(data);
				}
				else {
					DoDispatch(thread_task{
						&ctx, Task,
						0, 0, 1, 0,
						nullptr,
						nullptr,
						task_memory
						// std::hash<std::thread::id>{}(std::this_thread::get_id())
					}
					, groupSize, jobCount);
				}
			};

			void DispatchOnce(
				dispatch_context& ctx,
				void (*Task)(job_argument const&),
				void* task_memory
			) noexcept {
				initialize_if_necessary();

				// if a job dispatch is within an existing job, groupCount needs to narrow down to prevent overflow.
				constexpr size_t groupCount = 1;
				constexpr size_t groupSize = 1;
				// context state is updated to its maximum:
				ctx.counter += groupCount;

				if (wait_depth > 0) {
					// do the work directly:
					void* data{ nullptr };
					size_t threadID{ util::get_thread_id() }, sizeOfData{ 0 };
					thread_task task;
					impl::job_argument args{
						0 // jobIndex
						, 0 // groupID
						, 0 // groupIndex
						, nullptr // sharedmemory
						, task_memory // task memory
					};

					task.ctx = &ctx;
					task.task = Task;
					task.group_end_job = nullptr;
					task.group_start_job = nullptr;
					task.group_memory_size = 0;
					task.task_memory = task_memory;
					for (size_t groupID = 0; ; ++groupID) { // groupID < groupCount
						// For each group, generate one real job:
						auto groupJobOffset = groupID * groupSize;
						auto groupJobEnd = std::min<size_t>(groupJobOffset + groupSize, 1);
						if (groupJobOffset >= groupJobEnd) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.

						task.group_id = groupID;
						task.group_job_offset = groupJobOffset;
						task.group_job_end = groupJobEnd;

						do_task(task, args, sizeOfData, data);
					}

					// work until this job is completed
					while (ctx.is_busy()) {
						if (try_get_job(internal_state.jobQueue, task)) {
							do_task(task, args, sizeOfData, data);
						}
					}

					if (sizeOfData > 0) ::_aligned_free(data);
				}
				else {
					DoDispatch(thread_task{
						&ctx, Task,
						0, 0, 1, 0,
						nullptr,
						nullptr,
						task_memory
						// std::hash<std::thread::id>{}(std::this_thread::get_id())
						}
					, groupSize, 1);
				}
			};

			void Wait(dispatch_context& ctx) {
				++wait_depth; // allows detection of when job dispatch may be from within an existing job
				internal_state.wakeCondition.notify_all(); // Wake any threads that might be sleeping:
#if 0 // Does not support jobs calling jobs 
				work(internal_state.nextQueue.Increment() % internal_state.numThreads, &ctx);
				while (IsBusy(ctx)) { std::this_thread::yield(); };
				--wait_depth;
				HandleExceptions(ctx);
#else // supports jobs calling jobs
				while (ctx.is_busy()) { // Do work
					std::this_thread::yield();
					work(&ctx);
				}
				--wait_depth; // allows detection of when job dispatch may be from within an existing job
				ctx.try_rethrow_exception();
#endif
			};

		}
	}
}


