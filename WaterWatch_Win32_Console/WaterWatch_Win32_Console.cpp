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

#include "../FiberTasks/TaskScheduler.h"
#include "../FiberTasks/WaitGroup.h"

#pragma region newFiberTools







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
	~basic_cached_allocator(){
		free_internal(this);
	}

	template<typename T> T* alloc() {
		size_t size = sizeof(T);
		while (!memblocks || memblocks->sizefree < (size + sizeof(void*))) {
			size_t blocksize = 16 * 1024;
			basic_cached_allocator_memoryblock* block = (basic_cached_allocator_memoryblock*)ClearedAlloc(blocksize, memTag_t::TAG_TEMP); // zero's out
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

	void*               mem;
	basic_cached_allocator_memoryblock*    memblocks;

private:
	static void free_internal(basic_cached_allocator* allocator) {
		while (allocator->memblocks) {
			basic_cached_allocator_memoryblock* p = allocator->memblocks;
			allocator->memblocks = allocator->memblocks->next;
			if (p) ::_aligned_free((void*)p);
		}
		if (allocator->mem) ::_aligned_free(allocator->mem);
	};
	static void* Alloc16(const size_t& size, const memTag_t& tag) { if (!size) return nullptr; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16); };
	static void* ClearedAlloc(const size_t& size, const memTag_t& tag) { void* memP = Alloc16(size, tag); ::memset(memP, 0, size); return memP; };
};












class FiberWrapper;
class WaitingFiberBundle {
public:
	// The fiber
	FiberWrapper* Fiber;
	// A flag used to signal if the fiber has been successfully switched out of and "cleaned up". See @TaskScheduler::CleanUpOldFiber()
	std::atomic<bool> FiberIsSwitched;

	WaitingFiberBundle* Next;
	WaitingFiberBundle* QueueTail;
};

class TaskScheduler;
class WaitGroup {
public:
	explicit WaitGroup(TaskScheduler* taskScheduler);
	WaitGroup(WaitGroup const&) = delete;
	WaitGroup(WaitGroup&&) noexcept = delete;
	WaitGroup& operator=(WaitGroup const&) = delete;
	WaitGroup& operator=(WaitGroup&&) noexcept = delete;
	~WaitGroup() = default;
private:
	/* The TaskScheduler this WaitGroup is associated with */
	TaskScheduler* m_taskScheduler;

	/* The counter that can be waited on. Once it is zero, all waiters will be released */
	std::atomic<int32_t> m_counter;

	/* We store the queue lock and the queue all in a single uintptr_t */
	std::atomic<uintptr_t> m_word;

	static constexpr uintptr_t kIsQueueLockedBit = 1;

	static constexpr uintptr_t kQueueHeadMask = 1;

public:
	void Add(int32_t delta);
	void Done() { Add(-1); };
	void Wait();
};
class ThreadWrapper;
class FiberWrapper {
public:
	std::shared_ptr<fibers::Fiber> fiber;
	std::shared_ptr<std::atomic<bool>> free;
	ThreadWrapper* currentThread;
};


struct TaskSchedulerInitOptions {
	/* The size of the fiber pool.The fiber pool is used to run new tasks when the current task is waiting on a counter */
	unsigned FiberPoolSize = 400;
	/* The size of the thread pool to run. 0 corresponds to NumHardwareThreads() */
	unsigned ThreadPoolSize = 0;
	/* The behavior of the threads after they have no work to do */
	fibers::EmptyQueueBehavior Behavior = fibers::EmptyQueueBehavior::Yield;
};
struct TaskBundle {
	std::shared_ptr<fibers::Action> TaskToExecute;
	WaitingFiberBundle* argData;
	WaitGroup* WG;
};

enum class FiberDestination {
	None = 0,
	ToPool = 1,
	ToWaiting = 2,
};

struct alignas(fibers::kCacheLineSize) ThreadLocalStorage {
	ThreadLocalStorage() : CurrentFiber(nullptr), OldFiber(nullptr) { };

public:
	/* The queue of thread-specific high priority waiting tasks. This also contains the ready waiting fibers, which are differentiated by the argData and TaskToExecute being a nullptr. */
	concurrency::concurrent_queue<TaskBundle> HiPriTaskQueue; 
	/* The queue of shared low priority waiting tasks. All threads / fibers will pull from this shared queue at runtime. */
	concurrency::concurrent_queue<TaskBundle>* LoPriTaskQueue;

	std::atomic<bool>* OldFiberStoredFlag{ nullptr }; // someone is waiting on us

	/**
	 * The current fiber implementation requires that fibers created from threads finish on the same thread where
	 * they started
	 *
	 * To accommodate this, we have save the initial fibers created in each thread, and immediately switch
	 * out of them into the general fiber pool. Once the 'mainTask' has finished, we signal all the threads to
	 * start quitting. When they receive the signal, they switch back to the ThreadFiber, allowing it to
	 * safely clean up.
	 */
	fibers::Fiber ThreadFiber;

	/* The current fiber */
	FiberWrapper* CurrentFiber;

	/* The previously executed fiber */
	FiberWrapper* OldFiber;

	/* Where OldFiber should be stored when we call CleanUpPoolAndWaiting() */
	FiberDestination OldFiberDestination{ FiberDestination::None };

	std::atomic<int> FailedQueuePopAttempts{ 0 };
};

class ThreadWrapper {
public:
	fibers::ThreadType type;
	std::shared_ptr<fibers::Fiber> quitFiber;
	std::shared_ptr<ThreadLocalStorage> tls;
	int threadAffinity;
};

class ThreadStartArgs {
public:
	TaskScheduler* Scheduler;
	ThreadWrapper* Thread;
};

class FiberStartArgs {
public:
	TaskScheduler* Scheduler;
	FiberWrapper* Fiber;
};

class TaskScheduler {
public:
	TaskScheduler();
	TaskScheduler(TaskSchedulerInitOptions options);
	TaskScheduler(TaskScheduler const&) = delete;
	TaskScheduler(TaskScheduler&&) noexcept = delete;
	TaskScheduler& operator=(TaskScheduler const&) = delete;
	TaskScheduler& operator=(TaskScheduler&&) noexcept = delete;
	~TaskScheduler();

private:
	// Member variables

	constexpr static unsigned kInvalidIndex = std::numeric_limits<unsigned>::max();
	constexpr static unsigned kNoThreadPinning = std::numeric_limits<unsigned>::max();
	constexpr static unsigned kFailedPopAttemptsHeuristic = 5;
	constexpr static unsigned kFailedPopAttemptsHeuristic2 = 50;
	constexpr static int kInitErrorDoubleCall = -30;
	constexpr static int kInitErrorFailedToCreateWorkerThread = -60;
	
	std::vector<ThreadWrapper> threadPool;
	std::vector<FiberWrapper> fiberPool;
	concurrency::concurrent_queue<TaskBundle> LoPriTaskQueue;

	std::atomic<bool> m_initialized{ false };
	std::atomic<bool> m_quit{ false };
	std::atomic<unsigned> m_quitCount{ 0 };

	std::atomic<fibers::EmptyQueueBehavior> m_emptyQueueBehavior{ fibers::EmptyQueueBehavior::Spin };

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

public:
	static void ReadyFiberDummyTask(TaskScheduler* taskScheduler, void* arg) {
		(void)taskScheduler;
		(void)arg;
	};

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
	void AddTask(fibers::Action task, WaitGroup* waitGroup = nullptr);

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

	void AddTasks(uint32_t numTasks, fibers::Action* tasks, WaitGroup* waitGroup = nullptr);
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
	FTL_NOINLINE ThreadWrapper* GetCurrentThread();

private:
	/**
	 * Pops the next task off the high priority queue into nextTask. If there are no tasks in the
	 * the queue, it will return false.
	 *
	 * @param nextTask      If the queue is not empty, will be filled with the next task
	 * @param taskBuffer    An empty buffer the function can use
	 * @return              True: Successfully popped a task out of the queue
	 */
	bool GetNextHiPriTask(TaskBundle* nextTask, std::vector<TaskBundle>* taskBuffer, ThreadWrapper* currentThread);
	/**
	 * Pops the next task off the low priority queue into nextTask. If there are no tasks in the
	 * the queue, it will return false.
	 *
	 * @param nextTask    If the queue is not empty, will be filled with the next task
	 * @return            True: Successfully popped a task out of the queue
	 */
	bool GetNextLoPriTask(TaskBundle* nextTask, ThreadWrapper* currentThread);

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
	FiberWrapper* GetNextFreeFiber();
	/**
	 * If necessary, moves the old fiber to the fiber pool or the waiting list
	 * The old fiber is the last fiber to run on the thread before the current fiber
	 */
	void CleanUpOldFiber(ThreadWrapper* currentThread);

	/**
	 * @brief Initializes the values of a WaitingFiberBundle with the current fiber info
	 *
	 * @param bundle                The bundle to initialize
	 * @param pinToCurrentThread    If true, the current fiber won't be resumed on another thread
	 */
	void InitWaitingFiberBundle(WaitingFiberBundle* bundle, ThreadWrapper* currentThread);

	/**
	 * @brief Get's a free fiber from the pool and switches to it
	 *
	 * @param fiberIsSwitched    The boolean to set when the fiber is successfully switched away
	 */
	void SwitchToFreeFiber(std::atomic<bool>* fiberIsSwitched, ThreadWrapper* currentThread);

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




FTL_NOINLINE ThreadWrapper* TaskScheduler::GetCurrentThread() {
	DWORD const threadId = ::GetCurrentThreadId();
	for (auto& thread : threadPool) {
		if (thread.type.Id == threadId) {
			return &thread;
		}
	}
	return nullptr;
};
FTL_THREAD_FUNC_RETURN_TYPE TaskScheduler::ThreadStartFunc(void* const arg) {
	auto* const threadArgs = reinterpret_cast<ThreadStartArgs*>(arg);
	TaskScheduler* taskScheduler = threadArgs->Scheduler;
	ThreadWrapper* thread = threadArgs->Thread;
	if (thread->threadAffinity >= 0) { fibers::SetCurrentThreadAffinity(thread->threadAffinity); }
	delete threadArgs;

	// Spin wait until everything is initialized
	while (!taskScheduler->m_initialized.load(std::memory_order_acquire)) FTL_PAUSE();
	
	// Get a free fiber to switch to
	FiberWrapper* freeFiber = taskScheduler->GetNextFreeFiber();

	// Initialize tls
	thread->tls->CurrentFiber = freeFiber;
	freeFiber->currentThread = thread;

	// Switch
	thread->tls->ThreadFiber.SwitchToFiber(&*thread->tls->CurrentFiber->fiber);

	// And we've returned

	// Cleanup and shutdown
	fibers::EndCurrentThread();

	FTL_THREAD_FUNC_END;
}
void TaskScheduler::FiberStartFunc(void* const arg) {
	auto* const fiberArgs = reinterpret_cast<FiberStartArgs*>(arg);
	TaskScheduler* taskScheduler = fiberArgs->Scheduler;
	FiberWrapper* fiber = fiberArgs->Fiber;
	delete fiberArgs;

	// If we just started from the pool, we may need to clean up from another fiber
	taskScheduler->CleanUpOldFiber(fiber->currentThread);

	std::vector<TaskBundle> taskBuffer;

	// Process tasks infinitely, until quit
	while (!taskScheduler->m_quit.load(std::memory_order_acquire)) {
		FiberWrapper* waitingFiber = nullptr;

		ThreadLocalStorage* tls = &*fiber->currentThread->tls;

		bool readyWaitingFibers = false;

		TaskBundle nextTask{};
		bool foundTask = false;

		// If nothing was found, check if there is a high priority task to run
		if (!waitingFiber) {
			foundTask = taskScheduler->GetNextHiPriTask(&nextTask, &taskBuffer, fiber->currentThread);

			// Check if the found task is a ReadyFiber dummy task
			if (foundTask && !nextTask.TaskToExecute) {
				// Get the waiting fiber index
				WaitingFiberBundle* bundle = nextTask.argData;
				waitingFiber = bundle->Fiber;
			}
		}

		if (waitingFiber) {
			// Found a waiting task that is ready to continue
			tls->OldFiber = tls->CurrentFiber;
			tls->CurrentFiber = waitingFiber;
			tls->OldFiberDestination = FiberDestination::ToPool;

			// Switch
			tls->CurrentFiber->currentThread = tls->OldFiber->currentThread;
			tls->OldFiber->fiber->SwitchToFiber(&*tls->CurrentFiber->fiber);

			// And we're back
			taskScheduler->CleanUpOldFiber(fiber->currentThread);

			// Get a fresh instance of TLS, since we could be on a new thread now
			tls = &*fiber->currentThread->tls;

			if (taskScheduler->m_emptyQueueBehavior.load(std::memory_order_relaxed) == fibers::EmptyQueueBehavior::Sleep) {
				tls->FailedQueuePopAttempts = 0;
			}
		}
		else {
			// If we didn't find a high priority task, look for a low priority task
			if (!foundTask) {
				foundTask = taskScheduler->GetNextLoPriTask(&nextTask, fiber->currentThread);
			}

			fibers::EmptyQueueBehavior const behavior = taskScheduler->m_emptyQueueBehavior.load(std::memory_order_relaxed);

			if (foundTask) {
				if (behavior == fibers::EmptyQueueBehavior::Sleep) {
					tls->FailedQueuePopAttempts = 0;
				}
				if (nextTask.TaskToExecute != nullptr) {
					nextTask.TaskToExecute->Invoke();
				}
				if (nextTask.WG != nullptr) {
					nextTask.WG->Done();
				}
			}
			else {
				// We failed to find a Task from any of the queues
				// What we do now depends on m_emptyQueueBehavior, which we loaded above
				switch (behavior) {
				case fibers::EmptyQueueBehavior::Sleep: // something went wrong with the sleep behavior without the pinned fiber mechanic
				case fibers::EmptyQueueBehavior::Yield:
					fibers::YieldThread();
					break;
				case fibers::EmptyQueueBehavior::Spin:
				default:
					// Just fall through and continue the next loop
					break;
				}
			}
		}
	}

	// Switch to the quit fibers
	fiber->fiber->SwitchToFiber(&*fiber->currentThread->quitFiber);

	// We should never get here
	printf("Fibers cannot return anything -- they simply do not have the mechanism for it, despite C++ not knowing this.");
	throw(std::runtime_error("Fibers cannot return anything -- they simply do not have the mechanism for it, despite C++ not knowing this."));
};
void TaskScheduler::ThreadEndFunc(void* arg) {
	auto* const threadArgs = reinterpret_cast<ThreadStartArgs*>(arg);
	TaskScheduler* taskScheduler = threadArgs->Scheduler;
	ThreadWrapper* thread = threadArgs->Thread;
	delete threadArgs;

	// Wait for all other threads to quit
	taskScheduler->m_quitCount.fetch_add(1, std::memory_order_seq_cst);
	while (taskScheduler->m_quitCount.load(std::memory_order_seq_cst) != taskScheduler->threadPool.size()) {
		fibers::SleepThread(50);
	}

	thread->quitFiber->SwitchToFiber(&thread->tls->ThreadFiber);//->CurrentFiber->fiber);

	// We should never get here
	printf("ThreadEndFunc should not return anything, and instead should exit silently after the previous context switch.");
	throw(std::runtime_error("ThreadEndFunc should not return anything, and instead should exit silently after the previous context switch."));
}
TaskScheduler::TaskScheduler() {};
TaskScheduler::TaskScheduler(TaskSchedulerInitOptions options) {
	Init(options);
};
TaskScheduler::~TaskScheduler() {
	for (auto& thread : threadPool) {
		ThreadStartArgs* args = new ThreadStartArgs();
		args->Scheduler = this;
		args->Thread = &thread;
		thread.quitFiber = std::shared_ptr<fibers::Fiber>(new fibers::Fiber(524288, ThreadEndFunc, args));
	}

	// Request that all the threads quit
	m_quit.store(true, std::memory_order_release);

	// Signal any waiting threads so they can finish
	if (m_emptyQueueBehavior.load(std::memory_order_relaxed) == fibers::EmptyQueueBehavior::Sleep) {
		ThreadSleepCV.notify_all();
	}

	// Wait for the worker threads to finish
	for (auto& thread : threadPool) {
		fibers::JoinThread(thread.type);
	}

	threadPool.clear();
	fiberPool.clear();
}
int TaskScheduler::Init(TaskSchedulerInitOptions options) {
	// Sanity check to make sure the user doesn't double init
	if (m_initialized.load()) {
		return kInitErrorDoubleCall;
	}

	// Initialize the flags
	m_emptyQueueBehavior.store(options.Behavior);

	int m_numThreads = fibers::GetNumHardwareThreads();

	// Create and populate the fiber pool
	int m_fiberPoolSize = options.FiberPoolSize;

	fiberPool.resize(m_fiberPoolSize, FiberWrapper());
	for (auto& fiber : fiberPool) {
		FiberStartArgs* args = new FiberStartArgs();
		args->Scheduler = this;
		args->Fiber = &fiber;
		fiber.fiber = std::shared_ptr<fibers::Fiber>(new fibers::Fiber(524288, FiberStartFunc, (void*)args));
		fiber.free = std::make_shared<std::atomic<bool>>();
		fiber.free->store(true, std::memory_order_release);
	}

	fibers::SetCurrentThreadAffinity(0);

	threadPool.resize(m_numThreads, ThreadWrapper());
	int threadPoolNumber = 1;
	for (auto& thread : threadPool) {		
		thread.threadAffinity = threadPoolNumber++;
		thread.tls = std::shared_ptr<ThreadLocalStorage>(new ThreadLocalStorage());
		thread.tls->LoPriTaskQueue = &LoPriTaskQueue;

		ThreadStartArgs* threadArgs = new ThreadStartArgs();
		threadArgs->Scheduler = this;
		threadArgs->Thread = &thread;
		
		char threadName[256];
		snprintf(threadName, sizeof(threadName), "FTL Worker Thread %i", thread.threadAffinity);
		
		if (!fibers::CreateThread(524288, ThreadStartFunc, threadArgs, threadName, &threadArgs->Thread->type)) {
			return kInitErrorFailedToCreateWorkerThread;
		}
	}

	// Signal the worker threads that we're fully initialized
	m_initialized.store(true, std::memory_order_release);

	return 0;
};
void TaskScheduler::AddTask(fibers::Action task, WaitGroup* waitGroup) {
	if (waitGroup != nullptr) {
		waitGroup->Add(1);
	}

	const TaskBundle bundle = { std::shared_ptr<fibers::Action>(new fibers::Action(task)), nullptr, waitGroup };
	this->LoPriTaskQueue.push(bundle);

	const fibers::EmptyQueueBehavior behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
	if (behavior == fibers::EmptyQueueBehavior::Sleep) {
		// Wake a sleeping thread
		ThreadSleepCV.notify_one();
	}
};
void TaskScheduler::AddTasks(uint32_t numTasks, fibers::Action* tasks, WaitGroup* waitGroup) {
	if (waitGroup != nullptr) {
		waitGroup->Add(static_cast<int32_t>(numTasks));
	}

	for (int i = 0; i < numTasks; i++)
		LoPriTaskQueue.push({ std::shared_ptr<fibers::Action>(new fibers::Action(tasks[i])), nullptr, waitGroup });

	const fibers::EmptyQueueBehavior behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
	if (behavior == fibers::EmptyQueueBehavior::Sleep) {
		// Wake all the threads
		ThreadSleepCV.notify_all();
	}
};
inline bool TaskScheduler::TaskIsReadyToExecute(TaskBundle* bundle) const {
	// "Real" tasks are always ready to execute
	if (bundle->TaskToExecute) {
		return true;
	}

	// If it's a ready fiber task, the arg is a WaitingFiberBundle
	WaitingFiberBundle* waitingFiberBundle = bundle->argData;
	return waitingFiberBundle->FiberIsSwitched.load(std::memory_order_acquire);
}
bool TaskScheduler::GetNextHiPriTask(TaskBundle* nextTask, std::vector<TaskBundle>* taskBuffer, ThreadWrapper* currentThread) {
	ThreadLocalStorage& tls = *currentThread->tls;

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
				fibers::EmptyQueueBehavior const behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
				if (behavior == fibers::EmptyQueueBehavior::Sleep) {
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

	// Force a scope so the `goto cleanup` above doesn't skip initialization
	{
		for (auto& thread : threadPool) {
			if (&thread != currentThread) {
				ThreadLocalStorage& otherTLS = *thread.tls;

				while (otherTLS.HiPriTaskQueue.try_pop(*nextTask)) {
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
							fibers::EmptyQueueBehavior const behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
							if (behavior == fibers::EmptyQueueBehavior::Sleep) {
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
			}
		}
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
		fibers::EmptyQueueBehavior const behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
		if (behavior == fibers::EmptyQueueBehavior::Sleep) {
			// Wake all the threads
			ThreadSleepCV.notify_all();
		}
	}

	return result;
}
bool TaskScheduler::GetNextLoPriTask(TaskBundle* nextTask, ThreadWrapper* currentThread) {
	ThreadLocalStorage& tls = *currentThread->tls;

	// Try to pop from our own queue
	if (tls.LoPriTaskQueue->try_pop(*nextTask)) {
		return true;
	}

	return false;
}
FiberWrapper* TaskScheduler::GetNextFreeFiber() {
	unsigned i, j;
	bool expected;

	for (j = 0; ; ++j) {
		for (auto& fiber : fiberPool) {
			if (!fiber.free->load(std::memory_order_relaxed)) {
				continue;
			}
			if (!fiber.free->load(std::memory_order_acquire)) {
				continue;
			}
			expected = true;
			if (std::atomic_compare_exchange_weak_explicit(&*fiber.free, &expected, false, std::memory_order_release, std::memory_order_relaxed)) {
				return &fiber;
			}
		}

		if (j > 10) {
			// printf("No free fibers in the pool. Possible deadlock");
			fibers::YieldThread();
		}
	}
}
void TaskScheduler::CleanUpOldFiber(ThreadWrapper* currentThread) {
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

	ThreadLocalStorage& tls = *currentThread->tls;
	switch (tls.OldFiberDestination) {
	case FiberDestination::ToPool:
		// In this specific implementation, the fiber pool is a flat array signaled by atomics
		// So in order to "Push" the fiber to the fiber pool, we just set its corresponding atomic to true

		tls.OldFiber->free->store(true, std::memory_order_release);
		tls.OldFiberDestination = FiberDestination::None;
		tls.OldFiber = nullptr;
		break;
	case FiberDestination::ToWaiting:
		// The waiting fibers are stored directly in their counters
		// They have an atomic<bool> that signals whether the waiting fiber can be consumed if it's ready
		// We just have to set it to true
		tls.OldFiberStoredFlag->store(true, std::memory_order_release);
		tls.OldFiberDestination = FiberDestination::None;
		tls.OldFiber = nullptr;
		break;
	case FiberDestination::None:
	default:
		break;
	}
}
void TaskScheduler::AddReadyFiber(WaitingFiberBundle* bundle) {
	ThreadLocalStorage* tls = &*bundle->Fiber->currentThread->tls;

	// Push a dummy task to the high priority queue
	TaskBundle taskBundle{ nullptr, bundle , nullptr };
	tls->HiPriTaskQueue.push(taskBundle);

	// If we're using EmptyQueueBehavior::Sleep, the other threads could be sleeping
	// Therefore, we need to kick a thread awake to ensure that the readied task is taken
	const fibers::EmptyQueueBehavior behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
	if (behavior == fibers::EmptyQueueBehavior::Sleep) {
		ThreadSleepCV.notify_one();
	}
};
void TaskScheduler::InitWaitingFiberBundle(WaitingFiberBundle* bundle, ThreadWrapper* currentThread) {
	ThreadLocalStorage& tls = *currentThread->tls;

	bundle->Fiber = tls.CurrentFiber;
	bundle->FiberIsSwitched.store(false);
	bundle->Next = nullptr;
};
void TaskScheduler::SwitchToFreeFiber(std::atomic<bool>* fiberIsSwitched, ThreadWrapper* currentThread) {
	ThreadLocalStorage& tls = *currentThread->tls;

	// Get a free fiber
	decltype(auto) freeFiber = GetNextFreeFiber();

	// Fill in tls
	tls.OldFiber = tls.CurrentFiber;
	tls.CurrentFiber = freeFiber;
	tls.OldFiberDestination = FiberDestination::ToWaiting;
	tls.OldFiberStoredFlag = fiberIsSwitched;

	// Switch
	tls.CurrentFiber->currentThread = tls.OldFiber->currentThread;
	tls.OldFiber->fiber->SwitchToFiber(&*tls.CurrentFiber->fiber);

	// And we're back
	CleanUpOldFiber(currentThread);
};

WaitGroup::WaitGroup(TaskScheduler* taskScheduler)
	: m_taskScheduler(taskScheduler),
	m_counter(0),
	m_word(0)
{}
void WaitGroup::Add(int32_t delta) {
	int32_t prev = m_counter.fetch_add(delta);
	int32_t newValue = prev + delta;

	if (newValue > 0) {
		return;
	}

	// This fiber reduced the value to zero.
	// Wake all the waiting fibers

	// Fast path
	// There are no waiters
	uintptr_t expected = 0;
	if (std::atomic_compare_exchange_weak(&m_word, &expected, kIsQueueLockedBit)) {
		// Release the lock and return
		// We own the lock, so there's no need for a CAS
		m_word.store(0);
		return;
	}

	// Slow path
	while (true) {
		uintptr_t currentWordValue = m_word.load();

		// The fast path CAS spurriously failed
		// Try again
		if (currentWordValue == 0) {
			if (std::atomic_compare_exchange_weak(&m_word, &currentWordValue, kIsQueueLockedBit)) {
				// Release the lock and return
				// We own the lock, so there's no need for a CAS
				m_word.store(0);
				return;
			}
			// Loop around and try again.
			fibers::YieldThread();
			continue;
		}

		// There *are* waiters in the queue

		// Acquire the queue lock
		// We proceed only if the queue lock is not held and we succeed in acquiring the queue lock.
		if ((currentWordValue & kIsQueueLockedBit) == kIsQueueLockedBit || !std::atomic_compare_exchange_weak(&m_word, &currentWordValue, currentWordValue | kIsQueueLockedBit)) {
			fibers::YieldThread();
			continue;
		}

		break;
	}

	uintptr_t currentWordValue = m_word.load();

	// After we acquire the queue lock the queue must be non-empty. The queue must be non-empty since the fast path failed
	// and we're the only one that can remove from the queue (since WaitGroup will hit zero only once for an Add() / Wait() cycle)
	WaitingFiberBundle* queueHead = reinterpret_cast<WaitingFiberBundle*>(currentWordValue & ~kQueueHeadMask);

	// Resume all waiters
	while (queueHead != nullptr) {
		WaitingFiberBundle* next = queueHead->Next;

		queueHead->Next = nullptr;
		queueHead->QueueTail = nullptr;
		m_taskScheduler->AddReadyFiber(queueHead);
		queueHead = next;
	}

	// Reset the lock and the queue
	m_word.store(0);
}
void WaitGroup::Wait() {
	// if this thread does not exist (as far as the scheduler is concerned) then we must handle it differently.
	auto* currentThread = m_taskScheduler->GetCurrentThread();
	ThreadWrapper thisThread; // temp, in case we need it
	if (currentThread == nullptr) {
#if 0
		thisThread.tls = std::shared_ptr<ThreadLocalStorage>(new ThreadLocalStorage());
		thisThread.tls->LoPriTaskQueue = &m_taskScheduler->LoPriTaskQueue;
		thisThread.threadAffinity = -1;

		auto* freeFiber = m_taskScheduler->GetNextFreeFiber();

		thisThread.tls->CurrentFiber = freeFiber;
		freeFiber->currentThread = &thisThread;

		currentThread = &thisThread;
#else
		while (m_counter.load(std::memory_order_relaxed) != 0) {
			fibers::YieldThread();
		}
		return;
#endif
	}

	while (true) {
		// Fast path
		// Counter is zero. No need to wait
		if (m_counter.load(std::memory_order_relaxed) == 0) {
			return;
		}

		uintptr_t currentWordValue = m_word.load();

		// Acquire the queue lock
		// We proceed only if the queue lock is not held and we succeed in acquiring the queue lock.
		if ((currentWordValue & kIsQueueLockedBit) == kIsQueueLockedBit || !std::atomic_compare_exchange_weak(&m_word, &currentWordValue, currentWordValue | kIsQueueLockedBit)) {
			fibers::YieldThread();
			continue;
		}

		break;
	}

	// We now own the lock
	// Check the counter one last time
	// It's possible Add() released the lock after it already woke up other threads
	if (m_counter.load() == 0) {
		// Release the lock and return
		// We own the lock, so there's no need for a CAS
		uintptr_t currentWordValue = m_word.load();
		m_word.store(currentWordValue & ~kIsQueueLockedBit);

		return;
	}

	uintptr_t currentWordValue = m_word.load();

	WaitingFiberBundle currentFiber{};
	m_taskScheduler->InitWaitingFiberBundle(&currentFiber, currentThread);

	WaitingFiberBundle* queueHead = reinterpret_cast<WaitingFiberBundle*>(currentWordValue & ~kQueueHeadMask);
	if (queueHead != nullptr) {
		// Put this fiber at the end of the queue.
		queueHead->QueueTail->Next = &currentFiber;
		queueHead->QueueTail = &currentFiber;

		// Release the queue lock.
		m_word.store(currentWordValue & ~kIsQueueLockedBit);
	}
	else {
		// Make this fiber be the queue-head.
		queueHead = &currentFiber;
		currentFiber.QueueTail = &currentFiber;

		// Release the queue lock and install ourselves as the head. No need for a CAS loop, since
		// we own the queue lock.
		uintptr_t newWordValue = currentWordValue;
		newWordValue |= reinterpret_cast<uintptr_t>(queueHead);
		newWordValue &= ~kIsQueueLockedBit;
		m_word.store(newWordValue);
	}

	// At this point everyone who acquires the queue lock will see `currentFiber` on the queue.
	// `currentFiber.FiberIsSwitched` will still be false though, so any any other threads trying
	// to resume this thread will wait for the current fiber to switch away below

	// Now switch
	m_taskScheduler->SwitchToFreeFiber(&currentFiber.FiberIsSwitched, currentThread);

	// We're back
};
#pragma endregion


namespace {
	struct AnyJobStruct {
		std::shared_ptr<fibers::Action> job;
		bool force;
	};
	static void DoAnyJobStruct(fibers::TaskScheduler* taskScheduler, void* arg) {
		std::unique_ptr<AnyJobStruct> data(static_cast<AnyJobStruct*>(arg));
		if (data && data->job) {
			if (data->force) {
				data->job->ForceInvoke();
			}
			else {
				data->job->Invoke();
			}
		}
	};
};
int Example::ExampleF(int numTasks, int numSubTasks) {

	if (false) {
		int count = 0;
		fibers::TaskScheduler scheduler;
		scheduler.Init();
		while (true) {
			cweeBalancedPattern pat;

			for (int i = 0; i < numTasks; i++) {
				fibers::WaitGroup wg(&scheduler);
				for (int j = 0; j < numSubTasks; j++) {
					auto action = fibers::Action([&pat](int j) {
						pat.AddValue(j, j);
						}, (int)((i * numSubTasks) + j));

					scheduler.AddTask({ DoAnyJobStruct, new AnyJobStruct({ std::make_shared<fibers::Action>(action), false }) }, fibers::TaskPriority::Normal, &wg);
				}
				wg.Wait(); // spin waiter
			}

			int sizeIs = pat.GetNumValues();
			if (sizeIs != numTasks * numSubTasks) {
				printf("Something went wrong -- not all tasks executed");
			}

			for (int i = 0; i < numTasks; i++) {
				std::vector<fibers::Task> actions;
				for (int j = 0; j < numSubTasks; j++) {
					auto todo = fibers::Action([&pat](int j) {
						pat.AddValue(j, j);
						}, (int)((i * numSubTasks) + j));


					actions.push_back(
						{ DoAnyJobStruct, new AnyJobStruct({ std::make_shared<fibers::Action>(todo), false }) }
					);
				}

				if (actions.size() > 0) {
					fibers::WaitGroup wg(&scheduler);
					scheduler.AddTasks(actions.size(), &actions[0], fibers::TaskPriority::Normal, &wg);
					wg.Wait(); // spin waiter
				}
			}

			fibers::WaitGroup wg(&scheduler);
			for (int i = 0; i < numTasks; i++) {
				auto todo = fibers::Action([&numSubTasks, &pat, &scheduler](int i) {
					std::vector<fibers::Task> actions;
					for (int j = 0; j < numSubTasks; j++) {
						auto todo = fibers::Action([&pat](int j) {
							pat.AddValue(j, j);
							}, (int)((i * numSubTasks) + j));
						actions.push_back(
							{ DoAnyJobStruct, new AnyJobStruct({ std::make_shared<fibers::Action>(todo), false }) }
						);
					}

					if (actions.size() > 0) {
						fibers::WaitGroup wg(&scheduler);
						scheduler.AddTasks(actions.size(), &actions[0], fibers::TaskPriority::Normal, &wg);
						wg.Wait(); // actual waiter
					}
					}, (int)i);

				scheduler.AddTask({ DoAnyJobStruct, new AnyJobStruct({ std::make_shared<fibers::Action>(todo), false }) }, fibers::TaskPriority::Normal, &wg);
			}
			wg.Wait(); // spin waiter

		}
	}

	if (false) {
		int count = 0;
		TaskScheduler scheduler;
		scheduler.Init();
		while (true) {			
			cweeBalancedPattern pat;

			count++;
			if (count == 30) {
				pat.Clear();				
			}
			else if (count == 40) {
				pat.Clear();
			}
			else if (count == 50) {
				pat.Clear();
			}
			else if (count == 51) {
				pat.Clear();
			}
			else if (count == 52) {
				pat.Clear();
			}
			else if (count == 53) {
				pat.Clear();
			}


			for (int i = 0; i < numTasks; i++) {
				WaitGroup wg(&scheduler);
				for (int j = 0; j < numSubTasks; j++) {
					scheduler.AddTask(fibers::Action([&pat](int j) {
						pat.AddValue(j, j);
						}, (int)((i * numSubTasks) + j)), &wg);
				}
				wg.Wait(); // spin waiter
			}

			int sizeIs = pat.GetNumValues();
			if (sizeIs != numTasks * numSubTasks) {
				printf("Something went wrong -- not all tasks executed");
			}

			for (int i = 0; i < numTasks; i++) {
				std::vector<fibers::Action> actions;
				for (int j = 0; j < numSubTasks; j++) {
					actions.push_back(
						fibers::Action([&pat](int j) {
							pat.AddValue(j, j);
							}, (int)((i * numSubTasks) + j))
					);
				}

				if (actions.size() > 0) {
					WaitGroup wg(&scheduler);
					scheduler.AddTasks(actions.size(), &actions[0], &wg);
					wg.Wait(); // spin waiter
				}
			}

			WaitGroup wg(&scheduler);
			for (int i = 0; i < numTasks; i++) {
				scheduler.AddTask(fibers::Action([&numSubTasks, &pat, &scheduler](int i) {
					std::vector<fibers::Action> actions;
					for (int j = 0; j < numSubTasks; j++) {
						actions.push_back(
							fibers::Action([&pat](int j) {
								pat.AddValue(j, j);
								}, (int)((i * numSubTasks) + j))
						);
					}

					if (actions.size() > 0) {
						WaitGroup wg(&scheduler);
						scheduler.AddTasks(actions.size(), &actions[0], &wg);
						wg.Wait(); // actual waiter
					}
					}, (int)i), &wg);
			}
			wg.Wait(); // spin waiter

		}
	}



	if (true) {
		fibers::ftl_wrapper::TaskScheduler scheduler;
		fibers::parallel::For(scheduler, 0, numTasks * numSubTasks, [](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {}
		});
	}
	if (true) {
		cweeBalancedPattern pat;
		fibers::ftl_wrapper::TaskScheduler scheduler;
		for (int i = 0; i < numTasks * numSubTasks; i++) {
			scheduler.AddTask(fibers::Job([&pat](int j) {
				pat.AddUniqueValue(j, j);
			}, (int)i));
		}
		scheduler.Wait();


		int sizeIs = pat.GetNumValues();
		if (sizeIs == 0) throw(std::runtime_error("Something went wrong"));
	}

	if (true) {
		fibers::ftl_wrapper::TaskScheduler scheduler;
		fibers::parallel::For(scheduler, 0, numTasks * numSubTasks * 2, [](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {}
		});
	}

	if (true) {
		fibers::ftl_wrapper::TaskScheduler scheduler;
		for (int i = 0; i < numTasks; i++) {
			auto job = fibers::Job([]() {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}
			});

			scheduler.AddTask(job);
		}
		scheduler.Wait();
	}

	if (true) {
		fibers::ftl_wrapper::TaskScheduler scheduler;
		fibers::parallel::For(scheduler, 0, numTasks, 1, [&scheduler, numSubTasks](int i) {
			fibers::parallel::For(scheduler, 0, numSubTasks, 1, [](int j) {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}
			});
		});
	}

	if (true) {
		cweeBalancedPattern pat;
		fibers::ftl_wrapper::TaskScheduler scheduler;
		fibers::parallel::For(scheduler, 0, numTasks, [&scheduler, &numSubTasks, &pat](int i) {
			fibers::parallel::For(scheduler, 0, numSubTasks, [&pat, &i, &numSubTasks](int j) {
				pat.AddUniqueValue(i*numSubTasks + j, i* numSubTasks + j);
			});
		});
		if (pat.GetNumValues() != numSubTasks * numTasks) {
			auto err = cweeStr::printf("Pattern had %i values instead of %i", pat.GetNumValues(), numSubTasks * numTasks);
			printf(err);
		}
	}

	if (true) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		fibers::containers::vector<int> list;

		fibers::ftl_wrapper::TaskScheduler scheduler;
		fibers::parallel::For(scheduler, 0, numTasks, 1, [&counter, &list, &scheduler, numSubTasks](int i) {
			fibers::parallel::For(scheduler, 0, numSubTasks, 1, [&counter, &list](int j) {
				list.push_back(
					counter->fetch_add(1)
				); // do work
			});
		});

		return counter->load();
	}
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

static Timer parallel_toast_manager = Timer(0.1, Action(std::function([](cweeStr& title, cweeStr& content) {
	while (cweeToasts->tryGetToast(title, content)) std::cout << cweeStr::printf("\n/* WaterWatch Toast: \t\"%s\": \t\"%s\" */\n\n", title.c_str(), content.c_str());
}), cweeStr(), cweeStr()));

// Handle async or scripted AppRequests. 
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