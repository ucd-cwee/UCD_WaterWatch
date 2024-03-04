// FiberPool based on:
// http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
#pragma once

#include "Fibers.h"
#include <assert.h>
#include <stdint.h>
#include <atomic>
#include <array>
#include <thread>

#undef FTL_FIBER_CANARY_BYTES
#define FTL_NUM_WAITING_FIBER_SLOTS 16
#undef FTL_VALGRIND
#undef FTL_FIBER_STACK_GUARD_PAGES
#define FTL_CPP_17
#undef FTL_WERROR
#undef FTL_DISABLE_ITERATOR_DEBUG

#include "ftl/task_scheduler.h"
#include "ftl/wait_group.h"
#include "ftl/fibtex.h"

namespace fibers {
	extern containers::DelayedInstantiation< ftl::TaskScheduler > Fibers = containers::DelayedInstantiation<::ftl::TaskScheduler>([]()-> ftl::TaskScheduler* {
		auto* p = new ftl::TaskScheduler();
		p->Init({ ftl::GetNumHardwareThreads() * 50, 0, ftl::EmptyQueueBehavior::Sleep, {} });
		return p;		
	});
	namespace utilities {
		int Hardware::GetNumCpuCores() {
			return static_cast<int>(ftl::GetNumHardwareThreads());
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
	namespace {
		struct AnyFunctionStruct {
			std::shared_ptr<Action> job;
			Any* destination;
			bool force;
		};
		static void DoAnyFuncStruct(ftl::TaskScheduler* taskScheduler, void* arg) {
			std::unique_ptr<AnyFunctionStruct> data(static_cast<AnyFunctionStruct*>(arg));
			if (data && data->job) {
				if (data->force) {
					if (data->destination) {
						auto* p = data->job->ForceInvoke();
						if (p) *data->destination = *p;
					}
					else {
						data->job->ForceInvoke();
					}
				}
				else {
					if (data->destination) {
						auto* p = data->job->Invoke();
						if (p) *data->destination = *p;
					}
					else {
						data->job->Invoke();
					}
				}
			}
		};
	};
	namespace synchronization {
		namespace {
			class mutex_impl {
			public:
				mutex_impl() : m_lock(&*Fibers) {};
				mutex_impl(const mutex_impl&) = delete;
				mutex_impl(mutex_impl&&) = delete;
				mutex_impl& operator=(const mutex_impl&) = delete;
				mutex_impl& operator=(mutex_impl&&) = delete;
				~mutex_impl() {};

				void			lock() noexcept { m_lock.lock(); };
				void			unlock() noexcept { m_lock.unlock(); };
				decltype(auto)            try_lock() noexcept { return m_lock.try_lock(); };

			private:
				ftl::Fibtex m_lock;

			};
		}

		mutex::mutex() : Handle(std::static_pointer_cast<void>(std::shared_ptr<mutex_impl>(new mutex_impl()))) {};
		mutex::mutex(const mutex& other) : Handle(std::static_pointer_cast<void>(std::shared_ptr<mutex_impl>(new mutex_impl()))) {};
		mutex::mutex(mutex&& other) : Handle(std::static_pointer_cast<void>(std::shared_ptr<mutex_impl>(new mutex_impl()))) {};

		[[nodiscard]] std::lock_guard<mutex>	mutex::guard() noexcept { return std::lock_guard<mutex>(const_cast<mutex&>(*this)); };
		void			mutex::lock() noexcept { std::static_pointer_cast<mutex_impl>(Handle)->lock(); };
		void			mutex::unlock() noexcept { std::static_pointer_cast<mutex_impl>(Handle)->unlock(); };
		bool            mutex::try_lock() noexcept { return std::static_pointer_cast<mutex_impl>(Handle)->try_lock(); };
	};

	JobGroup::JobGroup() : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<ftl::WaitGroup>(new ftl::WaitGroup(&*Fibers))))) {};
	JobGroup::JobGroup(Job const& job) : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<ftl::WaitGroup>(new ftl::WaitGroup(&*Fibers))))) { Queue(job); };
	void JobGroup::JobGroupImpl::Queue(Job const& job) {
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		Fibers->AddTask({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, false }) }, ftl::TaskPriority::Normal, wg.get());
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::ForceQueue(Job const& job) {
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		Fibers->AddTask({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, true }) }, ftl::TaskPriority::Normal, wg.get());
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::Queue(std::vector<Job> const& listOfJobs) {
#if 1
		for (Job const& j : listOfJobs) Queue(j);		
#else
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		std::vector<ftl::Task> tasks;
		for (auto& job : listOfJobs) {
			tasks.push_back({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, false }) });
			jobs.push_back(job);
		}
		Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, wg.get());
#endif
	};
	void JobGroup::JobGroupImpl::ForceQueue(std::vector<Job> const& listOfJobs) {
#if 1
		for (Job const& j : listOfJobs) ForceQueue(j);
#else
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		std::vector<ftl::Task> tasks;
		for (auto& job : listOfJobs) {
			tasks.push_back({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, true }) });
			jobs.push_back(job);
		}
		Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, wg.get());
#endif
	};
	void JobGroup::JobGroupImpl::Wait() {
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		wg->Wait();
	};
	JobGroup Job::AsyncInvoke() {
		return JobGroup(*this);
	};
	JobGroup Job::AsyncForceInvoke() { 
		return JobGroup(*this);
	};
	/* Queue job, and return tool to await the result */
	void Job::DelayedInvoke(double milliseconds_delay) {
		std::tuple< double, Job >* data = new std::tuple<double, Job>(milliseconds_delay, *this);

		ftl::ThreadType type;
		bool success = ftl::CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				std::tuple< double, Job >* T = static_cast<std::tuple< double, Job>*>(_anon_ptr);
				::Sleep(std::get<0>(*T));
				std::get<1>(*T).Invoke();
				delete T;
			}
			return 0;
		}), (void*)data, "DelayedTask", &type);
		if (success) CloseHandle(type.Handle);
	};
	/* Queue job, and return tool to await the result */
	void Job::DelayedForceInvoke(double milliseconds_delay) {
		std::tuple< double, Job >* data = new std::tuple<double, Job>(milliseconds_delay, *this);
		ftl::ThreadType type;
		bool success = ftl::CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				std::tuple< double, Job >* T = static_cast<std::tuple< double, Job>*>(_anon_ptr);
				::Sleep(std::get<0>(*T));
				std::get<1>(*T).ForceInvoke();
				delete T;
			}
			return 0;
			}), (void*)data, "DelayedTask", &type);
		if (success) CloseHandle(type.Handle);
	};
	void Job::AsyncDelayedInvoke(double milliseconds_delay) {
		std::tuple< double, Job >* data = new std::tuple<double, Job>(milliseconds_delay, *this);
		ftl::ThreadType type;
		bool success = ftl::CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				std::tuple< double, Job >* T = static_cast<std::tuple< double, Job>*>(_anon_ptr);
				::Sleep(std::get<0>(*T));
				auto awaiter = std::get<1>(*T).AsyncInvoke();
				awaiter.Wait();
				delete T;
			}
			return 0;
			}), (void*)data, "DelayedTask", &type);
		if (success) CloseHandle(type.Handle);
	};
	void Job::AsyncDelayedForceInvoke(double milliseconds_delay) {
		std::tuple< double, Job >* data = new std::tuple<double, Job>(milliseconds_delay, *this);
		ftl::ThreadType type;
		bool success = ftl::CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				std::tuple< double, Job >* T = static_cast<std::tuple< double, Job>*>(_anon_ptr);
				::Sleep(std::get<0>(*T));
				auto awaiter = std::get<1>(*T).AsyncForceInvoke();
				awaiter.Wait();
				delete T;
			}
			return 0;
			}), (void*)data, "DelayedTask", &type);
		if (success) CloseHandle(type.Handle);
	};

	namespace synchronization {
		namespace {
			class signal_impl {
			public:
				signal_impl(bool manualReset = true) : Handle(CreateEvent(NULL, manualReset, FALSE, NULL), [](void* p) { CloseHandle(p); }) {};
				signal_impl(const signal_impl&) = delete;
				signal_impl(signal_impl&& that) = delete;
				signal_impl& operator=(const signal_impl&) = delete;
				signal_impl& operator=(signal_impl&& that) = delete;
				~signal_impl() {};

			public:
				void	Raise() noexcept {
					SetEvent(Handle.get());
				};
				void	Clear() noexcept {
					ResetEvent(Handle.get());
				};
				void	Wait() noexcept {
					std::shared_ptr<void> _handle = Handle;

					Job job([_handle]()->bool {
						ftl::YieldThread();
						return signal_impl::TryWait(_handle);
					});

					for (; !signal_impl::TryWait(_handle); ) {
						if (job.AsyncInvoke().Wait_Get()[0].cast<bool>()) break;
					}
				};
				bool	TryWait() noexcept {
					return TryWait(Handle);
				};

			protected:
				std::shared_ptr<void> Handle;
				static bool	TryWait(std::shared_ptr<void> const& p) noexcept {
					return WaitForSingleObject(p.get(), 1) == ((((DWORD)0x00000000L)) + 0);
				};
			};
		};
		signal::signal(bool manualReset) : impl(std::static_pointer_cast<void>(std::shared_ptr<signal_impl>(new signal_impl(manualReset)))) {};
		void signal::Raise() noexcept { std::static_pointer_cast<signal_impl>(impl)->Raise(); };
		void signal::Clear() noexcept { std::static_pointer_cast<signal_impl>(impl)->Clear(); };
		void signal::Wait() noexcept { std::static_pointer_cast<signal_impl>(impl)->Wait(); };
		bool signal::TryWait() noexcept { return std::static_pointer_cast<signal_impl>(impl)->TryWait(); };
	};

	namespace parallel {
		namespace {
			class DispatchTimerImpl {
			public:
				DispatchTimerImpl(long double millisecondsBetweenDispatch, Job const& queuedActivity) : handle(), stop(new std::atomic<long>(0)) {
					std::tuple< long double, Job, std::shared_ptr<std::atomic<long>>>* data = new std::tuple<long double, Job, std::shared_ptr<std::atomic<long>>>(millisecondsBetweenDispatch, queuedActivity, stop);
					bool success = ftl::CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
						if (_anon_ptr != nullptr) {
							std::tuple< long double, Job, std::shared_ptr<std::atomic<long>>>* T = static_cast<std::tuple< long double, Job, std::shared_ptr<std::atomic<long>>>*>(_anon_ptr);
							while (1) {
								if (std::get<2>(*T)->load() >= 1) { break; }

								::Sleep(std::get<0>(*T));

								if (std::get<2>(*T)->load() >= 1) { break; }

								std::get<1>(*T).ForceInvoke();
							}
							delete T;
						}
						return 0;
					}), (void*)data, "DelayedTask", &handle);
				};
				~DispatchTimerImpl() { stop->fetch_add(1); WaitForSingleObject(handle.Handle, INFINITE); CloseHandle(handle.Handle); };

			private:
				std::shared_ptr<std::atomic<long>> stop;
				ftl::ThreadType handle;

			};
		};
		Timer::Timer(long double millisecondsBetweenDispatch, Job const& queuedActivity) : data(std::static_pointer_cast<void>(std::shared_ptr<DispatchTimerImpl>(new DispatchTimerImpl(millisecondsBetweenDispatch, queuedActivity)))) {};
	};

};

#undef FTL_NUM_WAITING_FIBER_SLOTS
#undef FTL_CPP_17

#if 0
fibers::Job FTL::fnFiberTasks(int numTasks, int numSubTasks) {
	return fibers::Job([](int numTasks, int numSubTasks) {
		std::atomic<int> counter;
		if (numTasks < 0) numTasks *= -1;
		if (numSubTasks < 0) numSubTasks *= -1;
		fibers::Job([&]() { // not required to be a job
			for (int i = 0; i < numTasks; ++i) { // iterations are actually in series
				fibers::Job([&numSubTasks, &counter]() { // not required to be a job
					fibers::parallel::For(0, numSubTasks, [&counter](int j) {  // policies / particles are done simultanously using the fibers::For or fibers::ForEach loops
						counter.fetch_add(1); // do work
					});
				}).AsyncInvoke().Wait();
			}
		}).AsyncInvoke().Wait();

		return counter.load();
	}, (int)numTasks, (int)numSubTasks);
};
#endif

