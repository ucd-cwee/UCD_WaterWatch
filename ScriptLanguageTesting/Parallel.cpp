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
			enum alive_state {
				is_dead = 0,
				is_booting = 1,
				is_alive = 2,
				is_debooting = 3
			};

			struct thread_wrap {
				std::thread 
					thread;
				size_t 
					thread_hash;
				size_t 
					thread_index;
				alive_state
					thread_alive;
				// Ratio of "work" this CPU core is capable of. E.g. 0.25 would indicate this core is capable of 25% of the workload of this CPU. 
				// Intended to help identify the primary cores vs. hyperthreaded cores, as their capacity for work is noticibly different. 
				double relative_speed;
			};

			template<typename T>
			struct locking_queue {
				std::mutex mut;
				std::deque<T> q;

				size_t push(T const& obj) {
					auto locked{ std::scoped_lock(mut) };
					q.push_back(obj);
					return q.size();
				};
				size_t push(T&& obj) {
					auto locked{ std::scoped_lock(mut) };
					q.push_back(std::move(obj));
					return q.size();
				};
				size_t push(size_t thread_index, T const& obj) {
					auto locked{ std::scoped_lock(mut) };
					q.push_back(obj);
					return q.size();
				};
				size_t push(size_t thread_index, T&& obj) {
					auto locked{ std::scoped_lock(mut) };
					q.push_back(std::move(obj));
					return q.size();
				};
				bool try_pop(T& out) {
					auto locked{ std::scoped_lock(mut) };
					if (q.size() > 0) {
						out = q.front();
						q.pop_front();
						return true;
					}
					return false;
				};
				size_t size() const {
					auto locked{ std::scoped_lock(mut) };
					return q.size();
				};
			};

			template<typename T>
			struct parallel_queue {
				GL::thread_object_no_default< locking_queue<T> > q;

				size_t push(T const& obj) {
					return q->push(obj);
				};
				size_t push(T&& obj) {
					return q->push(std::move(obj));
				};
				size_t push(size_t thread_index, T const& obj) {
					return q[thread_index].push(obj);
				};
				size_t push(size_t thread_index, T&& obj) {
					return q[thread_index].push(std::move(obj));
				};
				bool try_pop(T& out) {
					if (q.for_each_cancellable([&out](auto& Q) -> bool {
						return Q.try_pop(out);
				    })) {
						return true;
					}
					else {
						return false;
					};
				};
			};

			struct InternalState {
				size_t 
					numCores{ 0 };
				size_t 
					numThreads{ 0 };
				parallel_queue< thread_task > // locking_queue > atomic_parallel_stack > atomic_parallel_queue > parallel_queue
					jobQueue{};
				std::atomic<alive_state> 
					alive{ is_dead };
				std::condition_variable 
					wakeCondition{};
				std::mutex 
					wakeMutex{};
				std::vector<thread_wrap> 
					threads{};
				double
					target_cpu_utilization{ 0.8 };

				void ShutDown() {
					if (alive.load() == is_dead) return;
					else {
						auto prevS = alive_state::is_alive;
						while (!alive.compare_exchange_strong(prevS, is_debooting)) {}
						bool wake_loop = true;
						std::thread waker([&] {
							while (wake_loop) wakeCondition.notify_all(); // wakes up sleeping worker threads
					    });
						for (auto& thread : threads) {
							if (thread.thread_alive == alive_state::is_alive) {
								thread.thread.join();
								thread.thread_alive = alive_state::is_dead;
							}
							while (thread.thread_alive != alive_state::is_dead) {}
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
				if (task.ctx) {
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

					if (0ull == --task.ctx->counter) {
						if (task.ctx->callback) {
							task.ctx->callback(task.ctx->callback_data);
						}
					}
				}
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
					while (try_get_job(internal_state.jobQueue, task)) {							
						do_task(task, args, sizeOfData, data);
					}	
					if (data && (sizeOfData > 0)) ::_aligned_free(data);
				}
			};

			void DoDispatch(thread_task job, size_t groupSize, size_t jobCount) {
#if 1
				if (jobCount > (32 * internal_state.numThreads)) { 	
					// if there are enough jobs, then it is worth spending a few extra moments distributing jobs based on the "effectiveness" of the hyperthreads. Not all threads are built equal, 
					// and some have 1/2 or worse of the performance of others. This strategy attempted to measure the performance at start-up, and then uses that to divy-up jobs. 
#if 1
#if 1
					job.group_job_offset = 0;
					long long job_remaining = jobCount;
					size_t groupID = 0;
					for (size_t thread_index = 0; (thread_index < internal_state.numThreads) && (job_remaining > 0); ++thread_index) {
						long long num_jobs = std::min<long long>(job_remaining, (long long)(unsigned long long)std::round(static_cast<double>(jobCount) * internal_state.threads[thread_index].relative_speed));

						// submit groups evenly into the thread pool:
						for (; ; ) { // groupID < groupCount
							job.group_id = groupID++;							
							job.group_job_end = std::min(job.group_job_offset + groupSize, jobCount);
							if (job.group_job_offset >= job.group_job_end) {								
								break;
							}
							else { // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.	
								internal_state.jobQueue.push(internal_state.threads[thread_index].thread_index, job);
								num_jobs -= groupSize;
								job_remaining -= groupSize;								
								job.group_job_offset += groupSize;
							}
							if (num_jobs <= 0) break;
						}
					}
					while (job_remaining > 0) {
						job.group_id = groupID++;
						job.group_job_end = std::min(job.group_job_offset + groupSize, jobCount);
						if (job.group_job_offset >= job.group_job_end) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.				
						internal_state.jobQueue.push(job);
						job.group_job_offset += groupSize;
					}
#else
					size_t 
						max_job_num = 0, 
						thread_id;
					job.group_id = 0;

					for (size_t thread_index = 0; thread_index < internal_state.numThreads; ++thread_index) {
						thread_id = internal_state.threads[thread_index].thread_index; //  thread_index% internal_state.numThreads;
						max_job_num = std::min<long long>(jobCount, max_job_num + std::round(static_cast<double>(jobCount) * internal_state.threads[thread_index].relative_speed));

						// submit groups evenly into the thread pool:
						for (; ; ++job.group_id) { // groupID < groupCount
							// For each group, generate one real job:
							job.group_job_offset = job.group_id * groupSize;
							job.group_job_end = std::min(job.group_job_offset + groupSize, max_job_num);
							if (job.group_job_offset >= job.group_job_end) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.
							internal_state.jobQueue.push(thread_id, job);
						}
					}
					for (; ; ++job.group_id) { // groupID < groupCount
	                    // For each group, generate one real job:
						job.group_job_offset = job.group_id * groupSize;
						job.group_job_end = std::min(job.group_job_offset + groupSize, jobCount);
						if (job.group_job_offset >= job.group_job_end) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.
						internal_state.jobQueue.push(job);
					}
#endif
#else
					size_t
						thread_counter{ GL::util::get_thread_id() % internal_state.numThreads };
					size_t
						submission_thread{ internal_state.threads[thread_counter].thread_index }
					    , max_job_num{ static_cast<size_t>(static_cast<double>(jobCount) * internal_state.threads[thread_counter].relative_speed) };

					for (job.group_id = 0, job.group_job_offset = 0; ; ++job.group_id) {
						// For each group, generate one real job:					
						job.group_job_end = std::min(job.group_job_offset + groupSize, jobCount);
						if (job.group_job_offset >= job.group_job_end) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.
						internal_state.jobQueue.push(submission_thread, job);
						job.group_job_offset += groupSize;


						if (job.group_job_end > max_job_num) {
							thread_counter = (thread_counter + 1) % internal_state.numThreads;
							submission_thread = internal_state.threads[thread_counter].thread_index;
							max_job_num += static_cast<size_t>(static_cast<double>(jobCount) * internal_state.threads[thread_counter].relative_speed);													
						}
					}
#endif
				}
				else {
#endif
					for (job.group_id = 0, job.group_job_offset = 0; ; ++job.group_id) {
						// For each group, generate one real job:						
						job.group_job_end = std::min(job.group_job_offset + groupSize, jobCount);
						if (job.group_job_offset >= job.group_job_end) break; // this is how we know we've produced enough job groups to cover the number of jobs requested, and no more.
						internal_state.jobQueue.push(job);
						job.group_job_offset += groupSize;
					}
				}
				internal_state.wakeCondition.notify_all();
			};

			void initialize_if_necessary() {
				if (internal_state.alive.load(std::memory_order_relaxed) == alive_state::is_alive) return;
				auto prevS = alive_state::is_dead;
				if (internal_state.alive.compare_exchange_strong(prevS, alive_state::is_booting)) {
					internal_state.numCores = std::max<long long>(1, util::get_hardware_thread_count());
					// Calculate the actual number of worker threads we want (-1 main thread):
					internal_state.numThreads = internal_state.numCores;
					std::atomic<size_t> boot_count{ internal_state.numThreads };
					internal_state.threads.reserve(internal_state.numThreads);

					// initialize the thread pool
					for (size_t threadID = 0; threadID < internal_state.numThreads; ++threadID) {
						internal_state.threads.emplace_back(thread_wrap{ std::thread{ [threadID, &boot_count] {
							// pre-warm this thread's heap
							for (int i = 0; i < 100000; i++) delete (new int(5));

							internal_state.threads[threadID].thread_hash = std::hash<std::thread::id>{}(std::this_thread::get_id());
							internal_state.threads[threadID].thread_index = GL::util::get_thread_id();
							internal_state.threads[threadID].thread_alive = alive_state::is_booting;

							auto start_time = GL::util::get_current_epoch();
							volatile std::atomic<long> count{ 0 };
							for (volatile size_t i = 0; i < 10000000; ++i) ++count;							
							internal_state.threads[threadID].relative_speed = 1.0 / (double)((GL::util::get_current_epoch() - start_time) + 1);

							if (1) {
								thread_task temp{ nullptr, nullptr, 0, 0, 0, 0, nullptr, nullptr, nullptr };
								(void)internal_state.jobQueue.push(temp); // necessary to instantiate the thread_local object
								while (internal_state.jobQueue.try_pop(temp)) {}
							}

							--boot_count;
							internal_state.threads[threadID].thread_alive = alive_state::is_alive;

							while (internal_state.alive == alive_state::is_booting) {} // wait until we stop booting... 

							// find my "final" index
							size_t my_final_index = threadID;
							for (int i = 0; i < internal_state.numThreads; ++i) {
								if (internal_state.threads[i].thread_index == GL::util::get_thread_id()) {
									// found me		
									my_final_index = i;
									break;
								}
							}

							while ((internal_state.alive == alive_state::is_alive) && (internal_state.threads[my_final_index].thread_alive == is_alive)) {
								work(); // Work until no more jobs are found		
								auto lock{ std::unique_lock(internal_state.wakeMutex) };
								internal_state.wakeCondition.wait(lock);
							}
						} }, 0, 0, alive_state::is_booting, 0.0 });
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
					for (size_t threadID = 0; threadID < internal_state.numThreads; ++threadID) {
						double this_speed = internal_state.threads[threadID].relative_speed;
						total += this_speed;
					}
					for (size_t threadID = 0; threadID < internal_state.numThreads; ++threadID) {
						internal_state.threads[threadID].relative_speed /= total;
					}
					std::sort(internal_state.threads.begin(), internal_state.threads.end(), [](auto& lhs, auto& rhs) -> bool { return lhs.relative_speed > rhs.relative_speed; });
					prevS = alive_state::is_booting;
					internal_state.alive.compare_exchange_strong(prevS, alive_state::is_alive);

					// Idea: release threads that are minimally contributing to the actual task of multithreading. Free's those for the UI or other thread tasks, while greedily keeping the stronger/faster threads for us. 
					if (1) {
						bool wake_loop = true;
						std::thread waker([&] {
							while (wake_loop) internal_state.wakeCondition.notify_all(); // wakes up sleeping worker threads
					    });

						total = 1.0;
						while ((total > internal_state.target_cpu_utilization) && (internal_state.numThreads > (internal_state.numCores / 2)) && (internal_state.numThreads >= 2)) {
							total -= internal_state.threads[internal_state.numThreads - 1].relative_speed;
							internal_state.threads[internal_state.numThreads - 1].thread_alive = alive_state::is_debooting;

							internal_state.threads[internal_state.numThreads - 1].thread.join(); // perform the join
							internal_state.threads[internal_state.numThreads - 1].thread_alive = alive_state::is_dead; // declare it dead

							--internal_state.numThreads;
						}

						wake_loop = false;
						waker.join();
					}
				}
				else {
					while (internal_state.alive.load(std::memory_order_relaxed) != alive_state::is_alive) {};
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

			void Wait(dispatch_context& ctx, bool rethrow) {
				while (ctx.is_busy()) { // Do work
					++wait_depth; // allows detection of when job dispatch may be from within an existing job				
					internal_state.wakeCondition.notify_all(); // Wake any threads that might be sleeping:
					std::this_thread::yield();
					work(&ctx);
					--wait_depth; // allows detection of when job dispatch may be from within an existing job
				}		
				if (rethrow) ctx.try_rethrow_exception();
				else ctx.clear_exception();
			};

		}
	}
}


