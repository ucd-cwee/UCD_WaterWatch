
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
#pragma once

#include "Config.h"
#include "Fiber.h"
#include "Tasks.h"
#include "ThreadAbstraction.h"
#include <ppl.h>
#include <concurrent_queue.h>
#include <concurrent_vector.h>
#include "WaitGroup.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace fibers {
#if 0
	class basic_cached_allocator {
	public:
		struct basic_cached_allocator_memoryblock {
			size_t sizefree;
			basic_cached_allocator_memoryblock* next;
			char* memory;
		};

	public:
		basic_cached_allocator() :
			mem(nullptr),
			memblocks(nullptr)
		{}
		basic_cached_allocator(basic_cached_allocator const&) = delete;
		basic_cached_allocator(basic_cached_allocator&&) = delete;
		basic_cached_allocator& operator=(basic_cached_allocator const&) = delete;
		basic_cached_allocator& operator=(basic_cached_allocator&&) = delete;
		~basic_cached_allocator() {
			free_internal(this);
		}

		template<typename T> T* alloc() {
			size_t size = sizeof(T);
			while (!memblocks || memblocks->sizefree < (size + sizeof(void*))) {
				size_t blocksize = 16 * 1024;
				basic_cached_allocator_memoryblock* block = (basic_cached_allocator_memoryblock*)ClearedAlloc(blocksize); // zero's out
				size_t offset = sizeof(basic_cached_allocator_memoryblock);
				block->sizefree = blocksize - offset;
				block->next = memblocks;
				block->memory = ((char*)block) + offset;
				memblocks = block;
			}
			void* p_raw = memblocks->memory;
			void* p_aligned = p_raw;
			size += (uintptr_t)p_aligned - (uintptr_t)p_raw;
			memblocks->memory += size;
			memblocks->sizefree -= size;
			return static_cast<T*>(p_aligned);
		};

	private:

		void* mem;
		basic_cached_allocator_memoryblock* memblocks;

	private:
		static void free_internal(basic_cached_allocator* allocator) {
			while (allocator->memblocks) {
				basic_cached_allocator_memoryblock* p = allocator->memblocks;
				allocator->memblocks = allocator->memblocks->next;
				if (p) ::_aligned_free((void*)p);
			}
			if (allocator->mem) ::_aligned_free(allocator->mem);
		};
		static void* Alloc16(const size_t& size) { if (!size) return nullptr; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16); };
		static void* ClearedAlloc(const size_t& size) { void* memP = Alloc16(size); ::memset(memP, 0, size); return memP; };
	};
#endif

	enum class EmptyQueueBehavior {
		Spin, // Spin in a loop, actively searching for tasks		
		Yield, // Same as spin, except yields to the OS after each round of searching		
		Sleep // Puts the thread to sleep. Will be woken when more tasks are added to the remaining awake threads.
	};
	struct TaskSchedulerInitOptions {
		/* The size of the fiber pool.The fiber pool is used to run new tasks when the current task is waiting on a counter */
		unsigned FiberPoolSize = 400;
		/* The size of the thread pool to run. 0 corresponds to NumHardwareThreads() */
		unsigned ThreadPoolSize = 0;
		/* The behavior of the threads after they have no work to do */
		EmptyQueueBehavior Behavior = EmptyQueueBehavior::Sleep;
		/* Should use the main thread for processing and calculations */
		bool useMainThread = true;
	};
	struct WaitingFiberBundle;

	/**
	 * A class that enables task-based multithreading.
	 *
	 * Underneath the covers, it uses fibers to allow cores to work on other tasks
	 * when the current task is waiting on a synchronization atomic
	 */
	class TaskScheduler {
	public:
		TaskScheduler();
		TaskScheduler(TaskSchedulerInitOptions options);
		TaskScheduler(TaskScheduler const&) = delete;
		TaskScheduler(TaskScheduler&&) noexcept = delete;
		TaskScheduler& operator=(TaskScheduler const&) = delete;
		TaskScheduler& operator=(TaskScheduler&&) noexcept = delete;
		~TaskScheduler();

	public:
		// Inner struct definitions
		enum class FiberDestination {
			None = 0,
			ToPool = 1,
			ToWaiting = 2,
		};

		class FiberWrapper {
		public:
			FiberWrapper() = default;
			~FiberWrapper() = default;
			FiberWrapper(FiberWrapper const&) = delete;
			FiberWrapper(FiberWrapper&&) = delete;
			FiberWrapper& operator=(FiberWrapper const&) = delete;
			FiberWrapper& operator=(FiberWrapper&&) = delete;

			Fiber fiber;
			std::atomic<bool> freeFiber;
		};

	private:
		/**
		 * Holds a task that is ready to to be executed by the worker threads
		 * Counter is the counter for the task(group). It will be decremented when the task completes
		 */
		struct TaskBundle {
			Task TaskToExecute;
			WaitGroup* WG;
		};

		class alignas(kCacheLineSize) ThreadLocalStorage {
		public:
			ThreadLocalStorage() : 
				HiPriTaskQueue(), LoPriTaskQueue(nullptr), OldFiberStoredFlag(nullptr), PinnedReadyFibers(),
				ThreadFiber(), PinnedReadyFibersLock(),
				CurrentFiberIndex(kInvalidIndex), OldFiberIndex(kInvalidIndex),
				OldFiberDestination(FiberDestination::None), HiPriLastSuccessfulSteal(1), FailedQueuePopAttempts(0), workingOnTask(false), quitting(false)
			{ }
			ThreadLocalStorage(ThreadLocalStorage const&) = default;
			ThreadLocalStorage(ThreadLocalStorage&&) = default;
			ThreadLocalStorage& operator=(ThreadLocalStorage const&) = default;
			ThreadLocalStorage& operator=(ThreadLocalStorage&&) = default;
			~ThreadLocalStorage() = default;

		public:
			// NOTE: The order of these variables may seem odd / jumbled. However, it is to optimize the padding required

			/* The queue of high priority waiting tasks. This also contains the ready waiting fibers, which are differentiated by the Task function == ReadyFiberDummyTask */
			concurrency::concurrent_queue<TaskBundle> HiPriTaskQueue;
			/* The queue of high priority waiting tasks */
			concurrency::concurrent_queue<TaskBundle>* LoPriTaskQueue;

			std::atomic<bool>* OldFiberStoredFlag;

			/* The queue of ready waiting Fibers that were pinned to this thread */
			std::vector<WaitingFiberBundle*> PinnedReadyFibers;

			/**
			 * The current fiber implementation requires that fibers created from threads finish on the same thread where
			 * they started
			 *
			 * To accommodate this, we have save the initial fibers created in each thread, and immediately switch
			 * out of them into the general fiber pool. Once the 'mainTask' has finished, we signal all the threads to
			 * start quitting. When they receive the signal, they switch back to the ThreadFiber, allowing it to
			 * safely clean up.
			 */
			Fiber ThreadFiber;

			/* Lock protecting access to PinnedReadyFibers */
			std::mutex PinnedReadyFibersLock;

			/* The index of the current fiber in m_fibers */
			unsigned CurrentFiberIndex;
			/* The index of the previously executed fiber in m_fibers */
			unsigned OldFiberIndex;
			/* Where OldFiber should be stored when we call CleanUpPoolAndWaiting() */
			FiberDestination OldFiberDestination;

			/* The last high priority queue that we successfully stole from. This is an offset index from the current thread index */
			unsigned HiPriLastSuccessfulSteal;

			unsigned FailedQueuePopAttempts;

			std::atomic<bool> workingOnTask;
			std::atomic<bool> quitting;
			std::atomic<int> fiberState;
		};

		class ThreadWrapper {
		public:
			ThreadWrapper() = default;
			~ThreadWrapper() = default;
			ThreadWrapper(ThreadWrapper const&) = delete;
			ThreadWrapper(ThreadWrapper&&) = delete;
			ThreadWrapper& operator=(ThreadWrapper const&) = delete;
			ThreadWrapper& operator=(ThreadWrapper&&) = delete;

			ThreadType type;
			ThreadLocalStorage tls;
		};
	protected:
		// Member variables

		constexpr static unsigned kInvalidIndex = std::numeric_limits<unsigned>::max();
		constexpr static unsigned kNoThreadPinning = std::numeric_limits<unsigned>::max();

		concurrency::concurrent_queue<TaskBundle> LoPriTaskQueue;

		concurrency::concurrent_vector < std::shared_ptr<ThreadWrapper> > m_threads;
		std::unordered_map<DWORD, unsigned> m_threadsHandleToIndex;

		// std::atomic<int> m_fiberPoolSize{ 0 };

		/* The backing storage for the fiber pool */
		concurrency::concurrent_vector < std::shared_ptr<FiberWrapper> > m_fibers;
		// FiberWrapper instanceFiber;

		/* Should use the main thread for processing and calculations */
		bool useMainThread = true;

		Fiber* m_quitFibers{ nullptr };

		std::atomic<bool> m_initialized{ false };
		std::atomic<bool> m_quit{ false };
		std::atomic<unsigned> m_quitCount{ 0 };

		std::atomic<EmptyQueueBehavior> m_emptyQueueBehavior{ EmptyQueueBehavior::Spin };
		/**
		 * This lock is used with the CV below to put threads to sleep when there
		 * is no work to do.
		 */
		std::mutex ThreadSleepLock;
		std::condition_variable ThreadSleepCV;

		/**
		 * We friend WaitGroup and Fibtex so we can keep InitWaitingFiberBundle() and SwitchToFreeFiber() private
		 * This makes the public API cleaner
		 */
		friend class WaitGroup;
		friend class Fibtex;

	public:
		bool quitting() const { return m_quit.load(); };

		/**
		 * Initializes the TaskScheduler and binds the current thread as the "main" thread
		 *
		 * TaskScheduler functions can *only* be called from this thread or inside tasks on the worker threads
		 *
		 * @param options    The configuration options for the TaskScheduler. See the struct defintion for more details
		 * @return           0 on sucess. One of the following error code on failure:
		 *                       -30  Init was called more than once
		 *                       -60  Failed to create worker threads
		 */
		int Init(TaskSchedulerInitOptions options = TaskSchedulerInitOptions());

		/**
		 * Adds a task to the internal queue.
		 *
		 * NOTE: This can *only* be called from the main thread or inside tasks on the worker threads
		 *
		 * @param task        The task to queue
		 * @param priority    Which priority queue to put the task in
		 * @param counter     An atomic counter corresponding to this task. Initially it will be incremented by 1. When the task
		 *                    completes, it will be decremented.
		 */
		void AddTask(Task task, TaskPriority priority, WaitGroup* waitGroup = nullptr);

		/**
		 * Adds a group of tasks to the internal queue
		 *
		 * NOTE: This can *only* be called from the main thread or inside tasks on the worker threads
		 *
		 * @param numTasks    The number of tasks
		 * @param tasks       The tasks to queue
		 * @param priority    Which priority queue to put the tasks in
		 * @param counter     An atomic counter corresponding to the task group as a whole. Initially it will be incremented by
		 *                    numTasks. When each task completes, it will be decremented.
		 */

		void AddTasks(uint32_t numTasks, Task* tasks, TaskPriority priority, WaitGroup* waitGroup = nullptr);

		/**
		 * Gets the 0-based index of the current thread
		 * This is useful for m_tls[GetCurrentThreadIndex()]
		 *
		 * NOTE: This can *only* be called from the main thread or inside tasks on the worker threads
		 *
		 * We force no-inline because inlining seems to cause some tls-type caching on max optimization levels
		 * Discovered by @cwfitzgerald. Documented in issue #57
		 *
		 * @return    The index of the current thread
		 */
		FTL_NOINLINE unsigned GetCurrentThreadIndex() const;
		/**
		 * Gets the 0-based index of the current thread
		 * This is useful for m_tls[GetCurrentThreadIndex()]
		 *
		 * NOTE: This can *only* be called from the main thread or inside tasks on the worker threads
		 *
		 * We force no-inline because inlining seems to cause some tls-type caching on max optimization levels
		 * Discovered by @cwfitzgerald. Documented in issue #57
		 *
		 * @return    The index of the current thread
		 */
		FTL_NOINLINE unsigned GetCurrentThreadIndex_NoFail() const;

		/**
		 * Gets the 0-based index of the current fiber.
		 *
		 * NOTE: main fiber index is 0
		 */
		unsigned GetCurrentFiberIndex() const;

		/**
		 * Gets the amount of backing threads.
		 *
		 * @return    Backing thread count
		 */
		unsigned GetThreadCount() const noexcept {
			return static_cast<unsigned>(m_threads.size());
		}

		/**
		 * Gets the amount of fibers in the fiber pool.
		 *
		 * @return    Fiber pool size
		 */
		unsigned GetFiberCount() const noexcept {
			return static_cast<unsigned>(m_fibers.size());
		}

		/**
		 * Set the behavior for how worker threads handle an empty queue
		 *
		 * @param behavior
		 * @return
		 */
		void SetEmptyQueueBehavior(EmptyQueueBehavior const behavior) {
			m_emptyQueueBehavior.store(behavior, std::memory_order_relaxed);
		}

	private:
		/**
		 * Pops the next task off the high priority queue into nextTask. If there are no tasks in the
		 * the queue, it will return false.
		 *
		 * @param nextTask      If the queue is not empty, will be filled with the next task
		 * @param taskBuffer    An empty buffer the function can use
		 * @return              True: Successfully popped a task out of the queue
		 */
		bool GetNextHiPriTask(TaskBundle* nextTask, std::vector<TaskBundle>* taskBuffer, unsigned* currentThreadIndex_p = nullptr);
		/**
		 * Pops the next task off the low priority queue into nextTask. If there are no tasks in the
		 * the queue, it will return false.
		 *
		 * @param nextTask    If the queue is not empty, will be filled with the next task
		 * @return            True: Successfully popped a task out of the queue
		 */
		bool GetNextLoPriTask(TaskBundle* nextTask);

		/**
		 * Checks if the Task is ready to execute
		 * "Real" tasks are always ready. ReadyFiber dummy tasks may be still waiting for the fiber to be switched
		 * away from.
		 *
		 * @param bundle    The task bundle to check
		 * @return          True: task bundle is a "real" task or a ReadyFiber dummy task which is safe to execute
		 *                  False: task bundle is a ReadyFiber dummy task which is not safe to execute
		 */
		bool TaskIsReadyToExecute(TaskBundle* bundle) const;

		/**
		 * Gets the index of the next available fiber in the pool
		 *
		 * @return    The index of the next available fiber in the pool
		 */
		unsigned GetNextFreeFiberIndex() const;
		/**
		 * If necessary, moves the old fiber to the fiber pool or the waiting list
		 * The old fiber is the last fiber to run on the thread before the current fiber
		 */
		void CleanUpOldFiber();

		/**
		 * @brief Initializes the values of a WaitingFiberBundle with the current fiber info
		 *
		 * @param bundle                The bundle to initialize
		 * @param pinToCurrentThread    If true, the current fiber won't be resumed on another thread
		 */
		void InitWaitingFiberBundle(WaitingFiberBundle* bundle, bool pinToCurrentThread);

		/**
		 * @brief Get's a free fiber from the pool and switches to it
		 *
		 * @param fiberIsSwitched    The boolean to set when the fiber is successfully switched away
		 */
		void SwitchToFreeFiber(std::atomic<bool>* fiberIsSwitched);

		/**
		 * Add a fiber to the "ready list". Fibers in the ready list will be resumed the next time a fiber goes searching
		 * for a new task
		 *
		 * @param pinnedThreadIndex    The index of the thread this fiber is pinned to. If not pinned, this will equal kNoThreadPinning
		 * @param bundle               The fiber bundle to add
		 * up"
		 */
		void AddReadyFiber(WaitingFiberBundle* bundle);

		/**
		 * The threadProc function for all worker threads
		 *
		 * @param arg    An instance of ThreadStartArgs
		 * @return       The return status of the thread
		 */
		static FTL_THREAD_FUNC_DECL ThreadStartFunc(void* arg);
		/**
		 * The fiberProc function for all fibers in the fiber pool
		 *
		 * @param arg    An instance of TaskScheduler
		 */
		static void FiberStartFunc(void* arg);
		/**
		 * The fiberProc function that fibers will jump to when Term() is called
		 * This allows us to jump back to the worker thread original stacks and clean up
		 * In addition, this allows the "main thread" to jump back to the "main thread" stack
		 *
		 * @param arg    An instance of ThreadTermArgs struct
		 */
		static void ThreadEndFunc(void* arg);
	};

} // End of namespace ftl