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


struct WaitingFiberBundle {
	// The fiber
	unsigned FiberIndex;
	// A flag used to signal if the fiber has been successfully switched out of and "cleaned up". See @TaskScheduler::CleanUpOldFiber()
	std::atomic<bool> FiberIsSwitched;
	/**
	 * The ID of the thread this fiber is pinned to
	 * If the fiber *isn't* pinned, this will equal std::numeric_limits<unsigned>::max()
	 */
	unsigned PinnedThreadId;

	WaitingFiberBundle* Next;
	WaitingFiberBundle* QueueTail;
};

class ThreadLocalStorage;
class Fiber {
public:
	Fiber() : readyFlag(new std::atomic<bool>(true)), fiber(new fibers::Fiber(524288, Fiber::Startup, nullptr)), quit_fiber() {};

	std::shared_ptr<std::atomic<bool>> readyFlag;
	std::shared_ptr<fibers::Fiber> fiber;
	std::shared_ptr<fibers::Fiber> quit_fiber;

	static void Startup(void*);
};

class WaitGroup {
public:
	WaitGroup() = default;
	WaitGroup(WaitGroup const&) = delete;
	WaitGroup(WaitGroup&&) noexcept = delete;
	WaitGroup& operator=(WaitGroup const&) = delete;
	WaitGroup& operator=(WaitGroup&&) noexcept = delete;
	~WaitGroup() = default;

	void Add(int32_t delta);
	void Done() { Add(-1); };
	void Wait(bool pinToCurrentThread = false);
private:
	/* The counter that can be waited on. Once it is zero, all waiters will be released */
	std::atomic<int32_t> m_counter;
	/* We store the queue lock and the queue all in a single uintptr_t */
	std::atomic<uintptr_t> m_word;

	static constexpr uintptr_t kIsQueueLockedBit = 1;
	static constexpr uintptr_t kQueueHeadMask = 1;
};

class TaskBundle {
public:
	std::shared_ptr<fibers::Action> action;
	std::shared_ptr<WaitGroup> wg;
};

class FiberPool {
public:
	FiberPool() { 
		Init();
	};
	~FiberPool();

	concurrency::concurrent_queue<TaskBundle> Queue;
	concurrency::concurrent_unordered_map<int, Fiber> Fibers;
	std::atomic<bool> m_initialized{ false };
	std::atomic<bool> m_quit{ false };
	std::atomic<unsigned> m_quitCount{ 0 };
	std::atomic<fibers::EmptyQueueBehavior> m_emptyQueueBehavior{ fibers::EmptyQueueBehavior::Sleep };
	std::mutex ThreadSleepLock;
	std::condition_variable ThreadSleepCV;

	void AddReadyFiber(WaitingFiberBundle* bundle);
	void InitWaitingFiberBundle(WaitingFiberBundle* bundle, bool pinToCurrentThread);
	void SwitchToFreeFiber(std::atomic<bool>* fiberIsSwitched);
	unsigned GetNextFreeFiberIndex() const;

	int Init();
	void AddTask(std::shared_ptr<fibers::Action> task, std::shared_ptr<WaitGroup> waitGroup = nullptr);
	static FTL_THREAD_FUNC_RETURN_TYPE ThreadStartFunc(void* const arg);
	static void ThreadEndFunc(void* arg);
};

static FiberPool fiberPool;

class alignas(64) ThreadLocalStorage {
public:
	constexpr static unsigned kInvalidIndex = std::numeric_limits<unsigned>::max();
	constexpr static unsigned kNoThreadPinning = std::numeric_limits<unsigned>::max();

	DWORD const threadId{ ::GetCurrentThreadId() };

	std::atomic<bool>* OldFiberStoredFlag{ nullptr };
	fibers::TaskScheduler::FiberDestination OldFiberDestination;
	unsigned OldFiberIndex;

	std::mutex PinnedReadyFibersLock;
	std::vector<std::shared_ptr<WaitingFiberBundle>> PinnedReadyFibers;

	unsigned CurrentFiberIndex{ 0 };

	unsigned FailedQueuePopAttempts{ 0 };

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

	void CleanUp() {
		switch (OldFiberDestination) {
		case fibers::TaskScheduler::FiberDestination::ToPool:
			// In this specific implementation, the fiber pool is a flat array signaled by atomics
			// So in order to "Push" the fiber to the fiber pool, we just set its corresponding atomic to true

			// set the previous fiber as "ready"
			fiberPool.Fibers[OldFiberIndex].readyFlag->store(true, std::memory_order_release);

			OldFiberDestination = fibers::TaskScheduler::FiberDestination::None;
			OldFiberIndex = kInvalidIndex;

			break;
		case fibers::TaskScheduler::FiberDestination::ToWaiting:
			// The waiting fibers are stored directly in their counters
			// They have an atomic<bool> that signals whether the waiting fiber can be consumed if it's ready
			// We just have to set it to true

			if (OldFiberStoredFlag) OldFiberStoredFlag->store(true, std::memory_order_release);
			OldFiberDestination = fibers::TaskScheduler::FiberDestination::None;
			OldFiberIndex = kInvalidIndex;
			break;
		case fibers::TaskScheduler::FiberDestination::None:
		default:
			break;
		}
	};
};
static thread_local ThreadLocalStorage tls;

void FiberPool::AddReadyFiber(WaitingFiberBundle* bundle) {
	unsigned const pinnedThreadIndex = bundle->PinnedThreadId;

	if (pinnedThreadIndex == ThreadLocalStorage::kNoThreadPinning) {
		// If we're using EmptyQueueBehavior::Sleep, the other threads could be sleeping
		// Therefore, we need to kick a thread awake to ensure that the readied task is taken
		const auto behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
		if (behavior == fibers::EmptyQueueBehavior::Sleep) {
			ThreadSleepCV.notify_one();
		}
	}
	else {
		{
			std::lock_guard<std::mutex> guard(tls.PinnedReadyFibersLock);
			tls.PinnedReadyFibers.emplace_back(bundle);
		}

		// If the Task is pinned, we add the Task to the pinned thread's PinnedReadyFibers queue
		// Normally, this works fine; the other thread will pick it up next time it
		// searches for a Task to run.
		//
		// However, if we're using EmptyQueueBehavior::Sleep, the other thread could be sleeping
		// Therefore, we need to kick all the threads so that the pinned-to thread can take it
		const auto behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
		if (behavior == fibers::EmptyQueueBehavior::Sleep) {
			if (tls.threadId != pinnedThreadIndex) {
				std::unique_lock<std::mutex> lock(ThreadSleepLock);
				// Kick all threads
				ThreadSleepCV.notify_all();
			}
		}
	}
};
void FiberPool::InitWaitingFiberBundle(WaitingFiberBundle* bundle, bool pinToCurrentThread) {
	unsigned const currentFiberIndex = tls.CurrentFiberIndex;

	unsigned pinnedThreadIndex;
	if (pinToCurrentThread) {
		pinnedThreadIndex = tls.threadId;
	}
	else {
		pinnedThreadIndex = ThreadLocalStorage::kNoThreadPinning;
	}

	bundle->FiberIndex = currentFiberIndex;
	bundle->FiberIsSwitched.store(false);
	bundle->PinnedThreadId = pinnedThreadIndex;
	bundle->Next = nullptr;
};
unsigned FiberPool::GetNextFreeFiberIndex() const {
	bool expected;

	while (true) {
		for(auto& fiber : Fibers) {
			// Double lock
			if (!fiber.second.readyFlag->load(std::memory_order_relaxed)) {
				continue;
			}

			if (!fiber.second.readyFlag->load(std::memory_order_acquire)) {
				continue;
			}

			expected = true;
			if (std::atomic_compare_exchange_weak_explicit(&*fiber.second.readyFlag, &expected, false, std::memory_order_release, std::memory_order_relaxed)) {
				return fiber.first;
			}
		}
		cweeSysThreadTools::Sys_Yield();
    }
}
void FiberPool::SwitchToFreeFiber(std::atomic<bool>* fiberIsSwitched) {
	unsigned const currentFiberIndex = tls.CurrentFiberIndex;

	// Get a free fiber
	unsigned const freeFiberIndex = GetNextFreeFiberIndex();

	// Fill in tls
	tls.OldFiberIndex = currentFiberIndex;
	tls.CurrentFiberIndex = freeFiberIndex;
	tls.OldFiberDestination = fibers::TaskScheduler::FiberDestination::ToWaiting;
	tls.OldFiberStoredFlag = fiberIsSwitched;

	// Switch
	Fibers[currentFiberIndex].fiber->SwitchToFiber(&*Fibers[freeFiberIndex].fiber);

	// And we're back
	tls.CleanUp();
};

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
			cweeSysThreadTools::Sys_Yield();
			continue;
		}

		// There *are* waiters in the queue

		// Acquire the queue lock
		// We proceed only if the queue lock is not held and we succeed in acquiring the queue lock.
		if ((currentWordValue & kIsQueueLockedBit) == kIsQueueLockedBit || !std::atomic_compare_exchange_weak(&m_word, &currentWordValue, currentWordValue | kIsQueueLockedBit)) {
			cweeSysThreadTools::Sys_Yield();
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

		fiberPool.AddReadyFiber(queueHead);
		queueHead = next;
	}

	// Reset the lock and the queue
	m_word.store(0);
};
void WaitGroup::Wait(bool pinToCurrentThread) {
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
			cweeSysThreadTools::Sys_Yield();
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
	fiberPool.InitWaitingFiberBundle(&currentFiber, pinToCurrentThread);

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
	fiberPool.SwitchToFreeFiber(&currentFiber.FiberIsSwitched);

	// We're back
};
void Fiber::Startup(void*) {
	tls.CleanUp();

	std::vector<TaskBundle> taskBuffer;

	// Process tasks infinitely, until quit
	while (!fiberPool.m_quit.load(std::memory_order_acquire)) {
		unsigned waitingFiberIndex = ThreadLocalStorage::kInvalidIndex;

		bool readyWaitingFibers = false;

		// Check if there is a ready pinned waiting fiber
		{
			std::lock_guard<std::mutex> guard(tls.PinnedReadyFibersLock);

			for (auto bundle = tls.PinnedReadyFibers.begin(); bundle != tls.PinnedReadyFibers.end(); ++bundle) {
				readyWaitingFibers = true;

				if (!(*bundle)->FiberIsSwitched.load(std::memory_order_acquire)) {
					// The wait condition is ready, but the "source" thread hasn't switched away from the fiber yet
					// Skip this fiber until the next round
					continue;
				}

				waitingFiberIndex = (*bundle)->FiberIndex;
				tls.PinnedReadyFibers.erase(bundle);
				break;
			}
		}

		TaskBundle nextTask;
		bool foundTask = false;

		if (waitingFiberIndex != ThreadLocalStorage::kInvalidIndex) {
			// Found a waiting task that is ready to continue

			tls.OldFiberIndex = tls.CurrentFiberIndex;
			tls.CurrentFiberIndex = waitingFiberIndex;
			tls.OldFiberDestination = fibers::TaskScheduler::FiberDestination::ToPool;

			// Switch
			fiberPool.Fibers[tls.OldFiberIndex].fiber->SwitchToFiber(&*fiberPool.Fibers[tls.CurrentFiberIndex].fiber);

			// And we're back
			tls.CleanUp();

			if (fiberPool.m_emptyQueueBehavior.load(std::memory_order_relaxed) == fibers::EmptyQueueBehavior::Sleep) {
				tls.FailedQueuePopAttempts = 0;
			}
		}
		else {
			// If we didn't find a high priority task, look for a low priority task
			if (!foundTask) {
				foundTask = fiberPool.Queue.try_pop(nextTask);
			}

			fibers::EmptyQueueBehavior const behavior = fiberPool.m_emptyQueueBehavior.load(std::memory_order_relaxed);

			if (foundTask) {
				if (behavior == fibers::EmptyQueueBehavior::Sleep) tls.FailedQueuePopAttempts = 0;
				if (nextTask.action) nextTask.action->Invoke();
				if (nextTask.wg) nextTask.wg->Done(); 				
			}
			else {
				// We failed to find a Task from any of the queues
				// What we do now depends on m_emptyQueueBehavior, which we loaded above
				switch (behavior) {
				case fibers::EmptyQueueBehavior::Yield:
					cweeSysThreadTools::Sys_Yield();
					break;

				case fibers::EmptyQueueBehavior::Sleep: {
					// If we have a ready waiting fiber, prevent sleep
					if (!readyWaitingFibers) {
						++tls.FailedQueuePopAttempts;
						// Go to sleep if we've failed to find a task kFailedPopAttemptsHeuristic times
						if (tls.FailedQueuePopAttempts >= 5) {
							std::unique_lock<std::mutex> lock(fiberPool.ThreadSleepLock);
							// Acquire the pinned ready fibers lock here and check if there are any pinned fibers ready
							// Acquiring the lock here prevents a race between readying a pinned fiber (on another thread) and going to sleep
							// Either this thread wins, then notify_*() will wake it
							// Or the other thread wins, then this thread will observe the pinned fiber, and will not go to sleep
							std::unique_lock<std::mutex> readyfiberslock(tls.PinnedReadyFibersLock);
							if (tls.PinnedReadyFibers.empty()) {
								// Unlock before going to sleep (the other lock is released by the CV wait)
								readyfiberslock.unlock();
								fiberPool.ThreadSleepCV.wait(lock);
							}
							tls.FailedQueuePopAttempts = 0;
						}
					}

					break;
				}
				case fibers::EmptyQueueBehavior::Spin:
				default:
					// Just fall through and continue the next loop
					break;
				}
			}
		}
	}

	// Switch to the quit fibers
	fiberPool.Fibers[tls.CurrentFiberIndex].fiber->SwitchToFiber(&*fiberPool.Fibers[tls.CurrentFiberIndex].quit_fiber);

	// We should never get here
	printf("Error: FiberStart should never return");
};

FTL_THREAD_FUNC_RETURN_TYPE FiberPool::ThreadStartFunc(void* const arg) {
	// Spin wait until everything is initialized
	while (!fiberPool.m_initialized.load(std::memory_order_acquire)) {
		// Spin
		FTL_PAUSE();
	}

	// Get a free fiber to switch to
	unsigned const freeFiberIndex = fiberPool.GetNextFreeFiberIndex();

	tls.CurrentFiberIndex = freeFiberIndex;
	tls.ThreadFiber.SwitchToFiber(&*fiberPool.Fibers[freeFiberIndex].fiber);

	// And we've returned

	// Cleanup and shutdown
	fibers::EndCurrentThread();
	FTL_THREAD_FUNC_END;
};
void FiberPool::ThreadEndFunc(void* arg) {
	// Wait for all other threads to quit
	fiberPool.m_quitCount.fetch_add(1, std::memory_order_seq_cst);
	while (fiberPool.m_quitCount.load(std::memory_order_seq_cst) != fibers::GetNumHardwareThreads()) {
		fibers::SleepThread(50);
	}

	// Jump to the thread fibers
	unsigned threadIndex = tls.threadId;

	fiberPool.Fibers[threadIndex].quit_fiber->SwitchToFiber(&*fiberPool.Fibers[threadIndex].fiber);

	// We should never get here
	printf("Error: ThreadEndFunc should never return");
};

int FiberPool::Init() {
	if (m_initialized.load()) {
		return -30;
	}

	int numThreads = fibers::GetNumHardwareThreads();
	int numFibers = 400;

	for (unsigned i = 1; i < numFibers; ++i) {
		Fibers[i].readyFlag->store(true, std::memory_order_release);
		// Fibers[i].quit_fiber;
	}
	Fibers[0].readyFlag->store(false, std::memory_order_release);

	tls.CurrentFiberIndex = 0;

	// Create the worker threads
	for (unsigned i = 1; i < numThreads; ++i) {
		char threadName[256];
		snprintf(threadName, sizeof(threadName), "FTL Worker Thread %u", i);
		fibers::ThreadType type;
		fibers::CreateThread(524288, ThreadStartFunc, nullptr, threadName, &type);
	}

	// Signal the worker threads that we're fully initialized
	m_initialized.store(true, std::memory_order_release);

	return 0;
};
FiberPool::~FiberPool() {
	for (auto& f : Fibers) {
		f.second.quit_fiber = std::shared_ptr<fibers::Fiber>(new fibers::Fiber(524288, ThreadEndFunc, nullptr));
	}

	// Request that all the threads quit
	m_quit.store(true, std::memory_order_release);

	// Signal any waiting threads so they can finish
	if (m_emptyQueueBehavior.load(std::memory_order_relaxed) == fibers::EmptyQueueBehavior::Sleep) {
		ThreadSleepCV.notify_all();
	}

	// Jump to the quit fiber
	// Create a scope so index isn't used after we come back from the switch. It will be wrong if we started on a non-main thread
	{
		unsigned index = tls.threadId;
		Fibers[tls.CurrentFiberIndex].fiber->SwitchToFiber(&*Fibers[tls.CurrentFiberIndex].quit_fiber);
	}

	// We're back. We should be on the main thread now
};
void FiberPool::AddTask(std::shared_ptr<fibers::Action> task, std::shared_ptr<WaitGroup> waitGroup) {
	if (waitGroup != nullptr) {
		waitGroup->Add(1);
	}

	TaskBundle bundle;
	bundle.action = task;
	bundle.wg = waitGroup;

	Queue.push(bundle);

	const auto behavior = m_emptyQueueBehavior.load(std::memory_order_relaxed);
	if (behavior == fibers::EmptyQueueBehavior::Sleep) {
		// Wake a sleeping thread
		ThreadSleepCV.notify_one();
	}
};




int Example::ExampleF(int numTasks, int numSubTasks) {

	std::shared_ptr<WaitGroup> wg = std::make_shared<WaitGroup>();
	fiberPool.AddTask(std::make_shared<fibers::Action>([]() {
		return 10.0f;
	}), wg);
	wg->Wait();







	














	if (true) {
		for (int i = 0; i < numTasks; i++) {
			fibers::Job([]() {
				::Sleep(100);
			}).AsyncInvoke().Wait();
		}
	}

	if (true) {
		for (int i = 0; i < numTasks; i++) {
			fibers::Job([numSubTasks]() {
				fibers::parallel::For(0, numSubTasks * 2, 2, [](int j) {
					::Sleep(10);
				});
			}).AsyncInvoke().Wait();
		}
	}

	if (true) {
		fibers::parallel::For(0, numTasks * 2, 2, [numSubTasks](int i) {
			fibers::parallel::For(0, numSubTasks * 2, 2, [](int j) {
				::Sleep(10);
			});
		});
	}

	if (true) {
		std::shared_ptr<std::atomic<int>> p = std::shared_ptr<std::atomic<int>>(new std::atomic<int>(0));
		cweeUnion<int, int, std::shared_ptr<std::atomic<int>>>* data = new cweeUnion<int, int, std::shared_ptr<std::atomic<int>>>(numTasks, numSubTasks, p);
		using targetT = decltype(data);
		auto handle = cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				targetT T = static_cast<targetT>(_anon_ptr);

				fibers::parallel::For(0, T->get<0>(), [T](int i) {
					fibers::parallel::For(0, T->get<1>(), [](int j) {
						::Sleep(10);
					});
				});

				T->get<2>()->store(1);

				delete T;
			}
			return 0;
		}), (void*)data, 1024);

		while (p->load() != 1) {
			cweeSysThreadTools::Sys_Yield();
		}

		cweeSysThreadTools::Sys_DestroyThread(handle);
	}


	if (true) {
		std::shared_ptr<std::atomic<int>> p = std::shared_ptr<std::atomic<int>>(new std::atomic<int>(0));
		cweeUnion<int, int, std::shared_ptr<std::atomic<int>>>* data = new cweeUnion<int, int, std::shared_ptr<std::atomic<int>>>(numTasks, numSubTasks, p);
		using targetT = decltype(data);
		auto handle = cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				targetT T = static_cast<targetT>(_anon_ptr);

				fibers::parallel::For(0, T->get<0>() * 2, 2, [T](int i) {
					fibers::parallel::For(0, T->get<1>() * 2, 2, [](int j) {
						::Sleep(10);
					});
				});

				T->get<2>()->store(1);

				delete T;
			}
			return 0;
		}), (void*)data, 1024);

		while (p->load() != 1) {
			cweeSysThreadTools::Sys_Yield();
		}

		cweeSysThreadTools::Sys_DestroyThread(handle);
	}


















	fibers::containers::vector<int> list;
	//return fibers::parallel::async([&]() {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		if (numTasks < 0) numTasks *= -1;
		if (numSubTasks < 0) numSubTasks *= -1;
		for (int i = 0; i < numTasks; ++i) { // iterations are actually in series
			fibers::parallel::For(0, numSubTasks, [&list, &counter](int j) {  // policies / particles are done simultanously using the fibers::For or fibers::ForEach loops
				list.push_back(
					counter->fetch_add(1)
				); // do work
			});
		}
		//return fibers::parallel::async([=]()->int {
			return counter->load();
		//});		
	//}).wait_get().wait_get();
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

static fibers::parallel::Timer parallel_toast_manager = fibers::parallel::Timer(0.1, fibers::Job([](cweeStr& title, cweeStr& content) {
	while (cweeToasts->tryGetToast(title, content)) std::cout << cweeStr::printf("\n/* WaterWatch Toast: \t\"%s\": \t\"%s\" */\n\n", title.c_str(), content.c_str());
}, cweeStr(), cweeStr()));


// Handle async or scripted AppRequests. 
static fibers::parallel::Timer AppLayerRequestProcessor = fibers::parallel::Timer(0.01, fibers::Job([]() {
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
}));

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
			int x = 100000;
			int y = 10;
			int z;
			while (true) {
				z = Example::ExampleF(y, x);
				if (y * x != z) {
					cweeStr err = cweeStr::printf("Something went wrong with the job system and a job returned %i instead of %i", z, y * x);
					throw(std::runtime_error(err.c_str()));
				}
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
