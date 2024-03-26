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

#include "TaskScheduler.h"
#include "WaitGroup.h"
#include "Fibtex.h"

namespace fibers {
	extern containers::DelayedInstantiation< TaskScheduler > Fibers = containers::DelayedInstantiation<TaskScheduler>([]()-> TaskScheduler* {
		auto* p = new TaskScheduler();
		p->Init({ GetNumHardwareThreads() * 50, 0, EmptyQueueBehavior::Sleep, false });
		return p;		
	});
	namespace utilities {
		int Hardware::GetNumCpuCores() {
			return static_cast<int>(GetNumHardwareThreads());
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
		class AnyFunctionStruct {
		public:
			AnyFunctionStruct() = default;
			AnyFunctionStruct(std::shared_ptr<Action> const& j, bool&& f) : job(j), force(std::forward<bool>(f)) {};
			AnyFunctionStruct(AnyFunctionStruct const&) = default;
			AnyFunctionStruct(AnyFunctionStruct&&) = default;
			AnyFunctionStruct& operator=(AnyFunctionStruct const&) = default;
			AnyFunctionStruct& operator=(AnyFunctionStruct&&) = default;
			~AnyFunctionStruct() = default;

			std::shared_ptr<Action> job;
			bool force;
		};
		extern containers::DelayedInstantiation< containers::queue<AnyFunctionStruct*> > AnyFunctionStructCache = containers::DelayedInstantiation<containers::queue<AnyFunctionStruct*>>([]()-> containers::queue<AnyFunctionStruct*>* {
			return new containers::queue<AnyFunctionStruct*>(1024);
		}, [](containers::queue<AnyFunctionStruct*>* p) { 
			AnyFunctionStruct* p2; 
			while (p->try_pop(p2)) { delete p2; } 
			delete p; 
		});
		static AnyFunctionStruct* getFunctionArgPtr(std::shared_ptr<Action> const& job, bool&& force) {
			AnyFunctionStruct* out;
			if (!AnyFunctionStructCache->try_pop(out)) {
				out = new AnyFunctionStruct(job, std::forward<bool>(force));
			}
			else {
				out->job = job;
				out->force = std::forward<bool>(force);
			}
			return out;
		};
		static void recycleFunctionArgPtr(AnyFunctionStruct* p) {
			if (p) {
				p->job = nullptr;
				AnyFunctionStructCache->push(std::move(p));
			}
		};
		static void DoAnyFuncStruct(void* arg) {
			AnyFunctionStruct* data(static_cast<AnyFunctionStruct*>(arg));
			if (data && data->job) {
				if (data->force) {
					data->job->ForceInvoke();
				}
				else {
					data->job->Invoke();
				}
			}
			recycleFunctionArgPtr(data);
		};


		static void DoFunctionBase(void* arg) {
			FunctionBase* data(static_cast<FunctionBase*>(arg));
			if (data) {
				data->Invoke();
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
				Fibtex m_lock;

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

	JobGroup::JobGroup() : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<WaitGroup>(new WaitGroup(&*Fibers))))) {};
	JobGroup::JobGroup(Job const& job) : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<WaitGroup>(new WaitGroup(&*Fibers))))) { Queue(job); };
	
	void JobGroup::JobGroupImpl::Queue(FunctionBase* basicjob) {
		std::shared_ptr<WaitGroup> wg = std::static_pointer_cast<WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		Fibers->AddTask({ DoFunctionBase, static_cast<void*>(basicjob) }, wg.get());
	};
	void JobGroup::JobGroupImpl::Queue(std::vector< FunctionBase* > const& basicjobs) {
		std::shared_ptr<WaitGroup> wg = std::static_pointer_cast<WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));

		std::vector<fibers::TaskScheduler::TaskBundle> taskbundles;
		taskbundles.reserve(basicjobs.size() + 1);
		for (FunctionBase* j : basicjobs) taskbundles.push_back({ { DoFunctionBase, static_cast<void*>(j) }, wg.get() });
		Fibers->AddTasks(basicjobs.size(), &taskbundles[0], wg.get());
	};


	void JobGroup::JobGroupImpl::Queue(Job const& job) {
		std::shared_ptr<WaitGroup> wg = std::static_pointer_cast<WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		Fibers->AddTask({ DoAnyFuncStruct, getFunctionArgPtr(job.impl, false) }, wg.get());
		jobs.push_back(job);
	};

	void JobGroup::JobGroupImpl::ForceQueue(Job const& job) {
		std::shared_ptr<WaitGroup> wg = std::static_pointer_cast<WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		Fibers->AddTask({ DoAnyFuncStruct, getFunctionArgPtr(job.impl, true) }, wg.get());
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::Queue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<WaitGroup> wg = std::static_pointer_cast<WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));

		std::vector<fibers::TaskScheduler::TaskBundle> taskbundles;
		taskbundles.reserve(listOfJobs.size()+1);
		for (Job const& j : listOfJobs) taskbundles.push_back({ { DoAnyFuncStruct, getFunctionArgPtr(j.impl, false) }, wg.get() });
		Fibers->AddTasks(listOfJobs.size(), &taskbundles[0], wg.get());

		for (Job const& j : listOfJobs) {
			jobs.push_back(j);
		}
	};
	void JobGroup::JobGroupImpl::ForceQueue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<WaitGroup> wg = std::static_pointer_cast<WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));

		std::vector<fibers::TaskScheduler::TaskBundle> taskbundles;
		taskbundles.reserve(listOfJobs.size() + 1);
		for (Job const& j : listOfJobs)  taskbundles.push_back({ { DoAnyFuncStruct, getFunctionArgPtr(j.impl, true) }, wg.get() });
		Fibers->AddTasks(listOfJobs.size(), &taskbundles[0], wg.get());

		for (Job const& j : listOfJobs) {
			jobs.push_back(j);
		}
	};
	void JobGroup::JobGroupImpl::Wait() {
		std::shared_ptr<WaitGroup> wg = std::static_pointer_cast<WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		//if (Fibers->GetCurrentThreadIndex_NoFail() == 0) 
		//	wg->Wait(true); 
		//else
			wg->Wait(false);
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

		ThreadType type;
		bool success = CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
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
		ThreadType type;
		bool success = CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
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
		ThreadType type;
		bool success = CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
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
		ThreadType type;
		bool success = CreateThread(1024, ([](void* _anon_ptr) -> unsigned int {
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
						YieldThread();
						return WaitForSingleObject(_handle.get(), 1) == ((((DWORD)0x00000000L)) + 0);
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
	};

};

#undef FTL_NUM_WAITING_FIBER_SLOTS
#undef FTL_CPP_17
