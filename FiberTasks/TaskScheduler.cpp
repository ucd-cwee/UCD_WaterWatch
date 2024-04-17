/**
 * FiberTaskingLib - A tasking library that uses fibers for efficient task switching
 *
 * This library was created as a proof of concept of the ideas presented by
 * Christian Gyrling in his 2015 GDC Talk 'Parallelizing the Naughty Dog Engine Using Fibers'
 *
 * http://gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
 *
 * FiberTaskingLib is the legal property of Adrian Astley
 * Copyright Adrian Astley 2015 - 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TaskScheduler.h"
#include "Callbacks.h"
#include "ThreadAbstraction.h"
#include "TaskSchedulerInternal.h"
#include "../WaterWatchCpp/Clock.h"

#if defined(FTL_OS_WINDOWS)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif

namespace fibers {
	constexpr static unsigned kFailedPopAttemptsHeuristic = 5;
	constexpr static int kInitErrorDoubleCall = -30;
	constexpr static int kInitErrorFailedToCreateWorkerThread = -60;

	struct ThreadStartArgs {
		TaskScheduler* Scheduler;
		unsigned ThreadIndex;
	};

	FTL_THREAD_FUNC_RETURN_TYPE TaskScheduler::ThreadStartFunc(void* const arg) {
		auto* const threadArgs = reinterpret_cast<ThreadStartArgs*>(arg);
		TaskScheduler* taskScheduler = threadArgs->Scheduler;
		unsigned const index = threadArgs->ThreadIndex;
		SetCurrentThreadAffinity(index);

		// Clean up
		delete threadArgs;

		// Spin wait until everything is initialized
		while (!taskScheduler->m_initialized.load(std::memory_order_acquire)) {
			// Spin
			FTL_PAUSE();
		}

		// Get a free fiber to switch to
		unsigned const freeFiberIndex = taskScheduler->GetNextFreeFiberIndex();

		// Initialize tls
		taskScheduler->m_threads[index]->tls.CurrentFiberIndex = freeFiberIndex;
		taskScheduler->m_threads[index]->tls.LoPriTaskQueue = &taskScheduler->LoPriTaskQueue;
		taskScheduler->m_threads[index]->tls.LoPriToken = std::make_unique<moodycamel::ConsumerToken>(taskScheduler->LoPriTaskQueue);
		taskScheduler->m_threads[index]->tls.ProducerToken = std::make_unique<moodycamel::ProducerToken>(taskScheduler->LoPriTaskQueue);

		// Switch
		taskScheduler->m_threads[index]->tls.ThreadFiber.SwitchToFiber(&taskScheduler->m_fibers[freeFiberIndex]->fiber);

		// And we've returned

		// Cleanup and shutdown
		EndCurrentThread();
		FTL_THREAD_FUNC_END;
	}

	// This Task is never used directly
	// However, a function pointer to it is the signal that the task is a Ready fiber, not a "real" task
	// See @FiberStartFunc() for more details
	static void ReadyFiberDummyTask(void* arg) {
		(void)arg;
	}

	void TaskScheduler::FiberStartFunc(void* const arg) {
		TaskScheduler* taskScheduler = reinterpret_cast<TaskScheduler*>(arg);

		// If we just started from the pool, we may need to clean up from another fiber
		taskScheduler->CleanUpOldFiber();

		std::vector<TaskBundle> taskBuffer;

		unsigned waitingFiberIndex;
		unsigned int threadIndex;
		ThreadLocalStorage* tls;
		bool readyWaitingFibers;
		bool foundTask;
		WaitingFiberBundle* bundle;
		EmptyQueueBehavior behavior;
		unsigned index;

		// Process tasks infinitely, until quit
		while (!taskScheduler->m_quit.load(std::memory_order_acquire)) {
			waitingFiberIndex = kInvalidIndex;
			threadIndex = taskScheduler->GetCurrentThreadIndex_NoFail();
			tls = &taskScheduler->m_threads[threadIndex]->tls;
			readyWaitingFibers = false;

			// Check if there is a ready pinned waiting fiber
			{
				std::lock_guard<std::mutex> guard(tls->PinnedReadyFibersLock);

				for (auto bundle = tls->PinnedReadyFibers.begin(); bundle != tls->PinnedReadyFibers.end(); ++bundle) {
					readyWaitingFibers = true;

					if (!(*bundle)->FiberIsSwitched.load(std::memory_order_acquire)) {
						// The wait condition is ready, but the "source" thread hasn't switched away from the fiber yet
						// Skip this fiber until the next round
						continue;
					}

					waitingFiberIndex = (*bundle)->FiberIndex;
					tls->PinnedReadyFibers.erase(bundle);
					break;
				}
			}

			TaskBundle nextTask{};
			foundTask = false;

			// If nothing was found, check if there is a high priority task to run
			if (waitingFiberIndex == kInvalidIndex) {
				foundTask = taskScheduler->GetNextHiPriTask(&nextTask, &taskBuffer, &threadIndex);

				// Check if the found task is a ReadyFiber dummy task
				if (foundTask && nextTask.TaskToExecute.Function == ReadyFiberDummyTask) {
					// Get the waiting fiber index
					bundle = reinterpret_cast<WaitingFiberBundle*>(nextTask.TaskToExecute.ArgData);
					if (bundle) {
						waitingFiberIndex = bundle->FiberIndex;
					}
				}
			}

			if (waitingFiberIndex != kInvalidIndex) {
				// Found a waiting task that is ready to continue
				tls->OldFiberIndex = tls->CurrentFiberIndex;
				tls->CurrentFiberIndex = waitingFiberIndex;
				tls->OldFiberDestination = FiberDestination::ToPool;

				// Switch
				taskScheduler->m_fibers[tls->OldFiberIndex]->fiber.SwitchToFiber(&taskScheduler->m_fibers[tls->CurrentFiberIndex]->fiber);

				// And we're back
				taskScheduler->CleanUpOldFiber();

				// Get a fresh instance of TLS, since we could be on a new thread now
				tls = &taskScheduler->m_threads[taskScheduler->GetCurrentThreadIndex_NoFail()]->tls;

				if (taskScheduler->m_emptyQueueBehavior.load(std::memory_order_relaxed) == EmptyQueueBehavior::Sleep) {
					tls->FailedQueuePopAttempts = 0;
				}
			}
			else {
				// If we didn't find a high priority task, look for a low priority task
				if (!foundTask) {
					foundTask = taskScheduler->GetNextLoPriTask(&nextTask);
				}

				behavior = taskScheduler->m_emptyQueueBehavior.load(std::memory_order_relaxed);

				if (foundTask) {
					if (behavior == EmptyQueueBehavior::Sleep) tls->FailedQueuePopAttempts = 0;
					
					if (nextTask.TaskToExecute.Function != nullptr) {
						tls->workingOnTask.store(true);
						nextTask.TaskToExecute.Function(nextTask.TaskToExecute.ArgData); // does the task
						tls->workingOnTask.store(false);
					}

					if (nextTask.WG) nextTask.WG->Done();
				}
				else {
					// We failed to find a Task from any of the queues
					// What we do now depends on m_emptyQueueBehavior, which we loaded above
					switch (behavior) {
					case EmptyQueueBehavior::Yield:
						YieldThread();
						break;

					case EmptyQueueBehavior::Sleep: {

						// If we have a ready waiting fiber, prevent sleep
						if (!readyWaitingFibers) {
							++tls->FailedQueuePopAttempts;
							// Go to sleep if we've failed to find a task kFailedPopAttemptsHeuristic times
							if (tls->FailedQueuePopAttempts >= kFailedPopAttemptsHeuristic) {
								std::unique_lock<std::mutex> lock(taskScheduler->ThreadSleepLock);
								// Acquire the pinned ready fibers lock here and check if there are any pinned fibers ready
								// Acquiring the lock here prevents a race between readying a pinned fiber (on another thread) and going to sleep
								// Either this thread wins, then notify_*() will wake it
								// Or the other thread wins, then this thread will observe the pinned fiber, and will not go to sleep
								std::unique_lock<std::mutex> readyfiberslock(tls->PinnedReadyFibersLock);
								if (tls->PinnedReadyFibers.empty()) {
									// Unlock before going to sleep (the other lock is released by the CV wait)
									readyfiberslock.unlock();

									taskScheduler->ThreadSleepCV.wait(lock); // mutex lock == puts the thread to sleep until the condition is tripped
								}
								tls->FailedQueuePopAttempts = 0;
							}
						}

						break;
					}
					case EmptyQueueBehavior::Spin:
					default:
						// Just fall through and continue the next loop
						break;
					}
				}
			}
		}

		// Switch to the quit fibers
		index = taskScheduler->GetCurrentThreadIndex_NoFail();
		taskScheduler->m_fibers[taskScheduler->m_threads[index]->tls.CurrentFiberIndex]->fiber.SwitchToFiber(&taskScheduler->m_quitFibers[index]);

		// We should never get here
		printf("Error: FiberStart should never return");
	}

	void TaskScheduler::ThreadEndFunc(void* arg) {
		TaskScheduler* taskScheduler = reinterpret_cast<TaskScheduler*>(arg);

		// Jump to the thread fibers
		unsigned threadIndex = taskScheduler->GetCurrentThreadIndex_NoFail();
		size_t currentThreadIndex = static_cast<size_t>(taskScheduler->GetCurrentThreadIndex());

		// Wait for all other threads to quit
		std::atomic<unsigned>* quitCount(&taskScheduler->m_quitCount);
		quitCount->fetch_add(1, std::memory_order_seq_cst);
		size_t numThreads = taskScheduler->m_threads.size();

		taskScheduler->m_threads[threadIndex]->tls.quitting.store(true);
		while (quitCount->load(std::memory_order_seq_cst) < static_cast<unsigned int>(numThreads)) {
			if (taskScheduler->m_emptyQueueBehavior.load(std::memory_order_relaxed) == EmptyQueueBehavior::Sleep) {
				taskScheduler->ThreadSleepCV.notify_all();
			}
			YieldThread();
		}

		if (threadIndex == 0) {
			// Special case for the main thread fiber
			taskScheduler->m_quitFibers[0].SwitchToFiber(&taskScheduler->m_fibers[0]->fiber); // taskScheduler->instanceFiber.fiber); // taskScheduler->m_fibers[0]->fiber);
		}
		else {
			taskScheduler->m_quitFibers[threadIndex].SwitchToFiber(&taskScheduler->m_threads[threadIndex]->tls.ThreadFiber);
		}

		// We should never get here
		printf("Error: ThreadEndFunc should never return");
	};

	TaskScheduler::TaskScheduler() {
		FTL_VALGRIND_HG_DISABLE_CHECKING(&m_initialized, sizeof(m_initialized));
		FTL_VALGRIND_HG_DISABLE_CHECKING(&m_quit, sizeof(m_quit));
		FTL_VALGRIND_HG_DISABLE_CHECKING(&m_quitCount, sizeof(m_quitCount));
	};

	int TaskScheduler::Init(TaskSchedulerInitOptions options) {
		// Sanity check to make sure the user doesn't double init
		if (m_initialized.load()) {
			return kInitErrorDoubleCall;
		}

		useMainThread = options.useMainThread;

		// Initialize the flags
		m_emptyQueueBehavior.store(options.Behavior);

		if (options.ThreadPoolSize == 0) {
			// 1 thread for each logical processor
			options.ThreadPoolSize = GetNumHardwareThreads();
		}

		// Create and populate the fiber pool
		for (unsigned i = 0; i < static_cast<unsigned>(options.FiberPoolSize); i++) {
			m_fibers.push_back(std::make_unique<FiberWrapper>());
		}

		// Leave the first slot for the bound main thread
		for (unsigned i = 1; i < options.FiberPoolSize; ++i) {
			m_fibers[i]->fiber = Fiber(524288, FiberStartFunc, this);
			freeFiberQueue.push(i);
		}


		// Initialize threads and TLS
		for (unsigned i = 0; i < static_cast<unsigned>(options.ThreadPoolSize); i++) {
			m_threads.push_back(std::make_unique<ThreadWrapper>());
		}
        
#if defined(FTL_WIN32_THREADS)
		// Temporarily set the main thread ID to -1, so when the worker threads start up, they don't accidentally use it
		// I don't know if Windows thread id's can ever be 0, but just in case.
		m_threads[0]->type.Id = static_cast<DWORD>(-1);
#endif
		// Set the properties for the main thread
		SetCurrentThreadAffinity(0);
		m_threads[0]->type = GetCurrentThread();
		m_threadsHandleToIndex[m_threads[0]->type.Id] = 0;
#if defined(FTL_WIN32_THREADS)
		// Set the thread handle to INVALID_HANDLE_VALUE
		// ::GetCurrentThread is a pseudo handle, that always references the current thread.
		// Aka, if we tried to use this handle from another thread to reference the main thread,
		// it would instead reference the other thread. We don't currently use the handle anywhere.
		// Therefore, we set this to INVALID_HANDLE_VALUE, so any future usages can take this into account
		// Reported by @rtj
		m_threads[0]->type.Handle = INVALID_HANDLE_VALUE;
#endif
		// Set the fiber index
		m_threads[0]->tls.CurrentFiberIndex = 0;
		m_threads[0]->tls.LoPriTaskQueue = &this->LoPriTaskQueue;
		m_threads[0]->tls.LoPriToken = std::make_unique<moodycamel::ConsumerToken>(LoPriTaskQueue);
		m_threads[0]->tls.ProducerToken = std::make_unique<moodycamel::ProducerToken>(LoPriTaskQueue);

		// Create the worker threads
		for (unsigned i = 1; i < options.ThreadPoolSize; ++i) {
			auto* const threadArgs = new ThreadStartArgs();
			threadArgs->Scheduler = this;
			threadArgs->ThreadIndex = i;

			char threadName[256];
			snprintf(threadName, sizeof(threadName), "FTL Worker Thread %u", i);

			if (!CreateThread(524288, ThreadStartFunc, threadArgs, threadName, &m_threads[i]->type)) {
				return kInitErrorFailedToCreateWorkerThread;
			}

			m_threadsHandleToIndex[m_threads[i]->type.Id] = i;
		}

		// Signal the worker threads that we're fully initialized
		m_initialized.store(true, std::memory_order_release);

		return 0;
	};

	TaskScheduler::TaskScheduler(TaskSchedulerInitOptions options) {
		Init(options);
	};


	TaskScheduler::~TaskScheduler() {
		// Signal any waiting threads so they can finish
		if (m_emptyQueueBehavior.load(std::memory_order_relaxed) == EmptyQueueBehavior::Sleep) {
			ThreadSleepCV.notify_all();
		}

		/* Wait on all jobs */
		//while (this->LoPriTaskQueue.unsafe_size() != 0) YieldThread();		
		//while (true) {
		//	bool finished = true;
		//	for (auto& thread : m_threads) {
		//		if (thread->tls.workingOnTask.load()) {
		//			finished = true;
		//			YieldThread();
		//			break;
		//		}
		//	}
		//	if (finished) break;
		//};

		// Create the quit fibers
		m_quitFibers = new Fiber[m_threads.size()];
		for (unsigned i = 0; i < m_threads.size(); ++i) {
			m_quitFibers[i] = Fiber(524288, ThreadEndFunc, this);
		}

		// Request that all the threads quit
		m_quit.store(true, std::memory_order_release);

		// Signal any waiting threads so they can finish
		if (m_emptyQueueBehavior.load(std::memory_order_relaxed) == EmptyQueueBehavior::Sleep) {
			ThreadSleepCV.notify_all();
		}

		// Jump to the quit fiber
		// Create a scope so index isn't used after we come back from the switch. It will be wrong if we started on a non-main thread
		unsigned index{ GetCurrentThreadIndex() }; // should be 0
		m_fibers[m_threads[index]->tls.CurrentFiberIndex]->fiber.SwitchToFiber(&m_quitFibers[index]);
				
		// Wait for the worker threads to finish
		for (unsigned i = 1; i < m_threads.size(); ++i) {
			JoinThread(m_threads[i]->type);
		}

		// Cleanup
		delete[] m_quitFibers;
	}

	void TaskScheduler::AddTask(Task&& task, WaitGroup* waitGroup) {
		if (waitGroup != nullptr) { waitGroup->Add(1); }
		LoPriTaskQueue.push(*m_threads[GetCurrentThreadIndex_NoFail()]->tls.ProducerToken, { std::forward<Task>(task), waitGroup });
		if (m_emptyQueueBehavior.load(std::memory_order_relaxed) == EmptyQueueBehavior::Sleep) { ThreadSleepCV.notify_one(); }
	};
	void TaskScheduler::AddTask(Task const& task, WaitGroup* waitGroup) {
		if (waitGroup != nullptr) { waitGroup->Add(1); }
		LoPriTaskQueue.push(*m_threads[GetCurrentThreadIndex_NoFail()]->tls.ProducerToken, { task, waitGroup });
		if (m_emptyQueueBehavior.load(std::memory_order_relaxed) == EmptyQueueBehavior::Sleep) { ThreadSleepCV.notify_one();  }
	};
	void TaskScheduler::AddTasks(uint32_t numTasks, Task* tasks, WaitGroup* waitGroup) {
		if (numTasks > 0) {
			auto items{ std::vector<TaskBundle>(numTasks, { Task(), waitGroup }) };
			for (uint32_t i = 0; i < numTasks; i++) items[i].TaskToExecute = tasks[i];
			AddTasks(numTasks, &items[0], waitGroup);
		}
	};
	void TaskScheduler::AddTasks(uint32_t numTasks, TaskBundle* items, WaitGroup* waitGroup) {
		if (numTasks > 0) {
			if (waitGroup != nullptr) { waitGroup->Add(static_cast<int32_t>(numTasks)); }
			LoPriTaskQueue.push_bulk(*m_threads[GetCurrentThreadIndex_NoFail()]->tls.ProducerToken, items, numTasks);
			if (m_emptyQueueBehavior.load(std::memory_order_relaxed) == EmptyQueueBehavior::Sleep) {
				ThreadSleepCV.notify_all(); // Wake all the threads
			}
		}
	};

#if defined(FTL_WIN32_THREADS)

	FTL_NOINLINE unsigned TaskScheduler::GetCurrentThreadIndex() const {
		DWORD const threadId = ::GetCurrentThreadId();
#if 1
		auto f = m_threadsHandleToIndex.find(threadId);
		if (f != m_threadsHandleToIndex.end()) {
			return f->second;
		}
		return kInvalidIndex;
#else
		for (unsigned i = 0; i < m_numThreads; ++i) {
			if (m_threads[i].Id == threadId) {
				return i;
			}
		}
		return kInvalidIndex;
#endif
	}
	FTL_NOINLINE unsigned TaskScheduler::GetCurrentThreadIndex_NoFail() const {
		DWORD const threadId = ::GetCurrentThreadId();
#if 1
		auto f = m_threadsHandleToIndex.find(threadId);
		if (f != m_threadsHandleToIndex.end()) {
			return f->second;
		}
		return 0;
#else
		for (unsigned i = 0; i < m_numThreads; ++i) {
			if (m_threads[i].Id == threadId) {
				return i;
			}
		}

		return 0;
#endif
	};

#elif defined(FTL_POSIX_THREADS)

	FTL_NOINLINE unsigned TaskScheduler::GetCurrentThreadIndex() const {
		pthread_t const currentThread = pthread_self();
		for (unsigned i = 0; i < m_numThreads; ++i) {
			if (pthread_equal(currentThread, m_threads[i]) != 0) {
				return i;
			}
		}

		return kInvalidIndex;
	}
	FTL_NOINLINE unsigned TaskScheduler::GetCurrentThreadIndex_NoFail() const {
		pthread_t const currentThread = pthread_self();
		for (unsigned i = 0; i < m_numThreads; ++i) {
			if (pthread_equal(currentThread, m_threads[i]) != 0) {
				return i;
			}
		}

		return 0;
	}

#endif

	unsigned TaskScheduler::GetCurrentFiberIndex() const {
		ThreadLocalStorage const& tls = m_threads[GetCurrentThreadIndex_NoFail()]->tls;
		return tls.CurrentFiberIndex;
	}

	inline bool TaskScheduler::TaskIsReadyToExecute(TaskBundle* bundle) const {
		// "Real" tasks are always ready to execute
		if (bundle->TaskToExecute.Function != ReadyFiberDummyTask) {
			return true;
		}

		// If it's a ready fiber task, the arg is a WaitingFiberBundle
		WaitingFiberBundle* waitingFiberBundle = reinterpret_cast<WaitingFiberBundle*>(bundle->TaskToExecute.ArgData);
		return waitingFiberBundle->FiberIsSwitched.load(std::memory_order_acquire);
	}

	bool TaskScheduler::GetNextHiPriTask(TaskBundle* nextTask, std::vector<TaskBundle>* taskBuffer, unsigned* currentThreadIndex_p) {
		unsigned const currentThreadIndex = currentThreadIndex_p ? *currentThreadIndex_p : GetCurrentThreadIndex_NoFail();
		ThreadLocalStorage& tls = m_threads[currentThreadIndex]->tls;

		bool result = false;

		// Try to pop from our own queue
		while (tls.HiPriTaskQueue.try_pop(*nextTask)) {
			if (TaskIsReadyToExecute(nextTask)) {
				result = true;
				// Break to cleanup
				if (!taskBuffer->empty()) {
					// Re-push all the tasks we found that we're ready to execute
					// We (or another thread) will get them next round
					do {
						// Push them in the opposite order we popped them, to restore the order
						tls.HiPriTaskQueue.push(taskBuffer->back());
						taskBuffer->pop_back();
					} while (!taskBuffer->empty());

					// If we're using Sleep mode, we need to wake up the other threads
					// They may have looked for tasks while we had them all in our temp buffer and thus not
					// found anything and gone to sleep.
					EmptyQueueBehavior const behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
					if (behavior == EmptyQueueBehavior::Sleep) {
						// Wake all the threads
						ThreadSleepCV.notify_all();
					}
				}
				return result;
			}

			// It's a ReadyTask whose fiber hasn't switched away yet
			// Add it to the buffer
			taskBuffer->emplace_back(*nextTask);
		}

		// cleanup:
		if (!taskBuffer->empty()) {
			// Re-push all the tasks we found that we're ready to execute
			// We (or another thread) will get them next round
			do {
				// Push them in the opposite order we popped them, to restore the order
				tls.HiPriTaskQueue.push(taskBuffer->back());
				taskBuffer->pop_back();
			} while (!taskBuffer->empty());

			// If we're using Sleep mode, we need to wake up the other threads
			// They may have looked for tasks while we had them all in our temp buffer and thus not
			// found anything and gone to sleep.
			EmptyQueueBehavior const behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
			if (behavior == EmptyQueueBehavior::Sleep) {
				// Wake all the threads
				ThreadSleepCV.notify_all();
			}
		}

		return result;
	};

	bool TaskScheduler::GetNextLoPriTask(TaskBundle* nextTask) {
		unsigned const currentThreadIndex = GetCurrentThreadIndex_NoFail();
		ThreadLocalStorage& tls = m_threads[currentThreadIndex]->tls;

		return LoPriTaskQueue.try_pop(*tls.LoPriToken, *nextTask);

		//if (tls.LoPriTaskQueue_local.try_pop(*nextTask)) {
		//	return true; // Try to pop from our local, private queue (fastest)
		//}
		//
		//if (tls.LoPriTaskQueue->try_pop(*nextTask)) {
		//	return true; // Try to pop the global queue (high contention)
		//}
		//
		//const unsigned threadIndex = tls.LoPriLastSuccessfulSteal;
		//auto m_numThreads = m_threads.size(); // Try to pop from the others' local, private queues (moderate contention, slow loop)
		//for (unsigned i = 0; i < m_numThreads; ++i) {
		//	const unsigned threadIndexToStealFrom = (threadIndex + i) % m_numThreads;
		//	if (threadIndexToStealFrom == currentThreadIndex) { continue; }

		//	ThreadLocalStorage& otherTLS = m_threads[threadIndexToStealFrom]->tls;
		//	if (otherTLS.LoPriTaskQueue_local.try_pop(*nextTask)) {
		//		tls.LoPriLastSuccessfulSteal = threadIndexToStealFrom;
		//		return true;
		//	}
		//}
		// return false;
	};

	unsigned TaskScheduler::GetNextFreeFiberIndex() const {
		unsigned j{ 0 };
#if 1
		int freeFiberIndex{ 0 };
		while (!freeFiberQueue.try_pop(freeFiberIndex)) {
			if (++j > 100) {
				// printf("No free fibers in the pool. Possible deadlock");
				YieldThread();
				j = 0;
			}
		}
		return freeFiberIndex;

#else
		bool expected;
		unsigned i;
		for (j = 0; ; ++j) {
			for (i = 0; i < m_fibers.size(); ++i) {
				// Double lock
				if (!m_fibers[i]->freeFiber.load(std::memory_order_relaxed)) {
					continue;
				}

				if (!m_fibers[i]->freeFiber.load(std::memory_order_acquire)) {
					continue;
				}

				expected = true;
				if (std::atomic_compare_exchange_weak_explicit(&m_fibers[i]->freeFiber, &expected, false, std::memory_order_release, std::memory_order_relaxed)) {
					return i;
				}
			}

			if (j > 100) {
				//auto wrapper = std::make_shared< FiberWrapper>();
				//wrapper->fiber = Fiber(524288, FiberStartFunc, const_cast<TaskScheduler*>(this));
				//wrapper->freeFiber.store(true);
				//const_cast<TaskScheduler*>(this)->m_fibers.push_back(wrapper);


				// printf("No free fibers in the pool. Possible deadlock");
				
				//YieldThread();

				j = 0;
			}
		}
#endif
	};

	void TaskScheduler::CleanUpOldFiber() {
		// Clean up from the last Fiber to run on this thread
		//
		// Explanation:
		// When switching between fibers, there's the innate problem of tracking the fibers.
		// For example, let's say we discover a waiting fiber that's ready. We need to put the currently
		// running fiber back into the fiber pool, and then switch to the waiting fiber. However, we can't
		// just do the equivalent of:
		//     m_fibers.Push(currentFiber)
		//     currentFiber.SwitchToFiber(waitingFiber)
		// In the time between us adding the current fiber to the fiber pool and switching to the waiting fiber, another
		// thread could come along and pop the current fiber from the fiber pool and try to run it.
		// This leads to stack corruption and/or other undefined behavior.
		//
		// In the previous implementation of TaskScheduler, we used helper fibers to do this work for us.
		// AKA, we stored currentFiber and waitingFiber in TLS, and then switched to the helper fiber. The
		// helper fiber then did:
		//     m_fibers.Push(currentFiber)
		//     helperFiber.SwitchToFiber(waitingFiber)
		// If we have 1 helper fiber per thread, we can guarantee that currentFiber is free to be executed by any thread
		// once it is added back to the fiber pool
		//
		// This solution works well, however, we actually don't need the helper fibers
		// The code structure guarantees that between any two fiber switches, the code will always end up in WaitForCounter
		// or FiberStart. Therefore, instead of using a helper fiber and immediately pushing the fiber to the fiber pool or
		// waiting list, we defer the push until the next fiber gets to one of those two places
		//
		// Proof:
		// There are only two places where we switch fibers:
		// 1. When we're waiting for a counter, we pull a new fiber from the fiber pool and switch to it.
		// 2. When we found a counter that's ready, we put the current fiber back in the fiber pool, and switch to the
		// waiting fiber.
		//
		// Case 1:
		// A fiber from the pool will always either be completely new or just come back from switching to a waiting fiber
		// In both places, we call CleanUpOldFiber()
		// QED
		//
		// Case 2:
		// A waiting fiber will always resume in WaitForCounter()
		// Here, we call CleanUpOldFiber()
		// QED

		ThreadLocalStorage& tls = m_threads[GetCurrentThreadIndex_NoFail()]->tls;
		switch (tls.OldFiberDestination) {
		case FiberDestination::ToPool:
			// In this specific implementation, the fiber pool is a flat array signaled by atomics
			// So in order to "Push" the fiber to the fiber pool, we just set its corresponding atomic to true
			freeFiberQueue.push(tls.OldFiberIndex);
			tls.OldFiberDestination = FiberDestination::None;
			tls.OldFiberIndex = kInvalidIndex;
			break;
		case FiberDestination::ToWaiting:
			// The waiting fibers are stored directly in their counters
			// They have an atomic<bool> that signals whether the waiting fiber can be consumed if it's ready
			// We just have to set it to true
			tls.OldFiberStoredFlag->store(true, std::memory_order_release);
			tls.OldFiberDestination = FiberDestination::None;
			tls.OldFiberIndex = kInvalidIndex;
			break;
		case FiberDestination::None:
		default:
			break;
		}
	};

	void TaskScheduler::AddReadyFiber(WaitingFiberBundle* bundle) {
		unsigned const pinnedThreadIndex = bundle->PinnedThreadIndex;

		if (pinnedThreadIndex == kNoThreadPinning || pinnedThreadIndex >= m_threads.size()) {
			ThreadLocalStorage* tls = &m_threads[GetCurrentThreadIndex_NoFail()]->tls;

			// Push a dummy task to the high priority queue
			Task task{};
			task.Function = ReadyFiberDummyTask;
			task.ArgData = bundle;
			TaskBundle taskBundle{};
			taskBundle.TaskToExecute = task;
			taskBundle.WG = nullptr;

			tls->HiPriTaskQueue.push(taskBundle);

			// If we're using EmptyQueueBehavior::Sleep, the other threads could be sleeping
			// Therefore, we need to kick a thread awake to ensure that the readied task is taken
			const EmptyQueueBehavior behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
			if (behavior == EmptyQueueBehavior::Sleep) {
				ThreadSleepCV.notify_one();
			}
		}
		else {
			ThreadLocalStorage* tls = &m_threads[pinnedThreadIndex]->tls;

			{
				std::lock_guard<std::mutex> guard(tls->PinnedReadyFibersLock);
				tls->PinnedReadyFibers.emplace_back(bundle);
			}

			// If the Task is pinned, we add the Task to the pinned thread's PinnedReadyFibers queue
			// Normally, this works fine; the other thread will pick it up next time it
			// searches for a Task to run.
			// However, if we're using EmptyQueueBehavior::Sleep, the other thread could be sleeping
			// Therefore, we need to kick all the threads so that the pinned-to thread can take it
			const EmptyQueueBehavior behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
			if (behavior == EmptyQueueBehavior::Sleep) {
				if (GetCurrentThreadIndex() != pinnedThreadIndex) {
					std::unique_lock<std::mutex> lock(ThreadSleepLock);
					// Kick all threads
					ThreadSleepCV.notify_all();
				}
			}
		}
	};

	void TaskScheduler::InitWaitingFiberBundle(WaitingFiberBundle* bundle, bool pinToCurrentThread) {
		ThreadLocalStorage& tls = m_threads[GetCurrentThreadIndex_NoFail()]->tls;
		unsigned const currentFiberIndex = tls.CurrentFiberIndex;

		unsigned pinnedThreadIndex;
		if (pinToCurrentThread && GetCurrentThreadIndex() != kInvalidIndex) {
			pinnedThreadIndex = GetCurrentThreadIndex();
		}
		else {
			pinnedThreadIndex = kNoThreadPinning;
		}

		bundle->FiberIndex = currentFiberIndex;
		bundle->FiberIsSwitched.store(false);
		bundle->PinnedThreadIndex = pinnedThreadIndex;
		bundle->Next = nullptr;
	};

	void TaskScheduler::SwitchToFreeFiber(std::atomic<bool>* fiberIsSwitched) {
		ThreadLocalStorage& tls = m_threads[GetCurrentThreadIndex_NoFail()]->tls;
		unsigned const currentFiberIndex = tls.CurrentFiberIndex;

		// Get a free fiber
		unsigned const freeFiberIndex = GetNextFreeFiberIndex();

		// Fill in tls
		tls.OldFiberIndex = currentFiberIndex;
		tls.CurrentFiberIndex = freeFiberIndex;
		tls.OldFiberDestination = FiberDestination::ToWaiting;
		tls.OldFiberStoredFlag = fiberIsSwitched;

		// Switch
		m_fibers[currentFiberIndex]->fiber.SwitchToFiber(&m_fibers[freeFiberIndex]->fiber);

		// And we're back
		CleanUpOldFiber();
	};

}; // End of namespace ftl