// FiberPool based on:
// http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
#pragma once

#include "FiberTasks.h"

#include "../WaterWatchCpp/cwee_math.h"
#include "../WaterWatchCpp/cweeTime.h"
#include "../WaterWatchCpp/Clock.h"

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
#include "../WaterWatchCpp/Toasts.h"

namespace fibers {
	extern DelayedInstantiation< ftl::TaskScheduler > Fibers = DelayedInstantiation< ftl::TaskScheduler >([]()-> ftl::TaskScheduler* {
		auto* p = new ftl::TaskScheduler();
		p->Init({ ftl::GetNumHardwareThreads() * 50, 0, ftl::EmptyQueueBehavior::Sleep, {} });
		p->SetEmptyQueueBehavior(ftl::EmptyQueueBehavior::Sleep);
		return p;
	});
	namespace utilities {
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };

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
		/* Fiber mutex */
		class mutex {
		public:
			using Handle_t = ftl::Fibtex;
			using Phandle = std::shared_ptr<Handle_t>;

		public:
			mutex() : Handle(new Handle_t(&*Fibers)) {};
			mutex(const mutex& other) : Handle(new Handle_t(&*Fibers)) {};
			mutex(mutex&& other) : Handle(new Handle_t(&*Fibers)) {};
			mutex& operator=(const mutex& s) { return *this; };
			mutex& operator=(mutex&& s) { return *this; };
			~mutex() {};

			NODISCARD AUTO	Guard() const noexcept { return std::lock_guard<mutex>(const_cast<mutex&>(*this)); };
			bool			Lock(bool blocking = true) const noexcept { Handle->lock(); return true; };
			void			Unlock() const noexcept { Handle->unlock(); };
			void			lock() const noexcept { Lock(); };
			void			unlock() const noexcept { Unlock(); };
			bool            try_lock() const noexcept { return Handle->try_lock(); };

		protected:
			Phandle Handle;

		};
			
		/* Wrapper for non-atomic objects to allow for threads to lock them for exclusive access */
		template< typename type>
		class interlocked {
		public:
			class ExclusiveObject {
			public:
				constexpr ExclusiveObject(const interlocked<type>& mut) : owner(const_cast<interlocked<type>&>(mut)) { this->owner.Lock(); };
				~ExclusiveObject() { this->owner.Unlock(); };

				ExclusiveObject() = delete;
				ExclusiveObject(const ExclusiveObject& other) = delete;
				ExclusiveObject(ExclusiveObject&& other) = delete;
				ExclusiveObject& operator=(const ExclusiveObject& other) = delete;
				ExclusiveObject& operator=(ExclusiveObject&& other) = delete;

				type& operator=(const type& a) { data() = a; return data(); };
				type& operator=(type&& a) { data() = a; return data(); };
				type& operator*() const { return data(); };
				type* operator->() const { return &data(); };

			protected:
				type& data() const { return owner.UnsafeRead(); };
				interlocked<type>& owner;
			};

		public: // construction and destruction
			typedef type Type;

			interlocked() : data() {};
			interlocked(const type& other) : data(other) {};
			interlocked(type&& other) : data(std::forward<type>(other)) {};
			interlocked(const interlocked& other) : data() { this->Copy(other); };
			interlocked(interlocked&& other) : data() { this->Copy(std::forward<interlocked>(other)); };
			~interlocked() {};

		public: // copy and clear
			interlocked<type>& operator=(const interlocked<type>& other) {
				this->Copy(other);
				return *this;
			};
			interlocked<type>& operator=(interlocked<type>&& other) {
				this->Copy(std::forward<interlocked<type>>(other));
				return *this;
			};
			void Copy(const interlocked<type>& copy) {
				if (&copy == this) return;
				lock.Lock();
				copy.Lock();
				data = copy.data;
				copy.Unlock();
				lock.Unlock();
			};
			void Copy(interlocked<type>&& copy) {
				if (&copy == this) return;
				lock.Lock();
				copy.Lock();
				data = copy.data;
				copy.Unlock();
				lock.Unlock();
			};
			void Clear() {
				lock.Lock();
				data = type();
				lock.Unlock();
			};

		public: // read and swap
			type Read() const {
				AUTO g = Guard();
				return data;
			};
			void Swap(const type& replacement) {
				AUTO g = Guard();
				data = replacement;
			};
			interlocked<type>& operator=(const type& other) {
				Swap(other);
				return *this;
			};

			operator type() const { return Read(); };
			type* operator->() const { return &data; };

		public: // lock, unlock, and direct edit
			ExclusiveObject GetExclusive() const { return ExclusiveObject(*this); };

			NODISCARD AUTO Guard() const { return lock.Guard(); };
			void Lock() const { lock.Lock(); };
			void Unlock() const { lock.Unlock(); };
			type& UnsafeRead() const { return data; };

		private:
			mutable type data;
			mutex lock;

		};
	};

	JobGroup::JobGroup() : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<ftl::WaitGroup>(new ftl::WaitGroup(&*Fibers))))) {};
	JobGroup::JobGroup(Job const& job) : impl(new JobGroup::JobGroupImpl(std::static_pointer_cast<void>(std::shared_ptr<ftl::WaitGroup>(new ftl::WaitGroup(&*Fibers))))) { Queue(job); };
	void JobGroup::JobGroupImpl::Queue(Job const& job) {
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		Fibers->AddTask({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, false }) }, ftl::TaskPriority::Normal, wg.get());
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::ForceQueue(Job const& job) {
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		Fibers->AddTask({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, true }) }, ftl::TaskPriority::Normal, wg.get());
		jobs.push_back(job);
	};
	void JobGroup::JobGroupImpl::Queue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty.")); 
		std::vector<ftl::Task> tasks;
		for (auto& job : listOfJobs) {
			tasks.push_back({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, false }) });
			jobs.push_back(job);
		}
		Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, wg.get());
	};
	void JobGroup::JobGroupImpl::ForceQueue(std::vector<Job> const& listOfJobs) {
		std::shared_ptr<ftl::WaitGroup> wg = std::static_pointer_cast<ftl::WaitGroup>(waitGroup);
		if (!wg) throw(std::runtime_error("Job Group was empty."));
		std::vector<ftl::Task> tasks;
		for (auto& job : listOfJobs) {
			tasks.push_back({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, true }) });
			jobs.push_back(job);
		}
		Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, wg.get());
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
	void Job::AsyncDelayedInvoke(u64 milliseconds_delay) {
		cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
		cweeSysThreadTools::Sys_ForgetThread(cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
				::Sleep(T->get<0>());
				auto awaiter = T->get<1>().AsyncInvoke();
				awaiter.Wait();
				delete T;
			}
			return 0;
		}), (void*)data, 1024));
	};
	void Job::AsyncDelayedForceInvoke(u64 milliseconds_delay) {
		cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
		cweeSysThreadTools::Sys_ForgetThread(cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
				::Sleep(T->get<0>());
				auto awaiter = T->get<1>().AsyncForceInvoke();
				awaiter.Wait();
				delete T;
			}
			return 0;
		}), (void*)data, 1024));
	};

	namespace parallel {
		/* Queue all jobs and await all results */
		AUTO Do(std::vector<fibers::Job> const& jobs) {
			JobGroup group;
			group.Queue(jobs);
			return group.Wait_Get();
		};
		
		/* parallel_for (auto i = start; i < end; i++){ todo(i); } */
		template<typename iteratorType, typename F>
		AUTO For(iteratorType start, iteratorType end, F&& ToDo) {
			auto todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;
			for (iteratorType iter = start; iter < end; iter++) {
				jobs.push_back(fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter));
			}
			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};
		
		/* parallel_for (auto i = start; i < end; i += step){ todo(i); } */
		template<typename iteratorType, typename F>
		AUTO For(iteratorType start, iteratorType end, iteratorType step, F&& ToDo) {
			auto todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;
			for (iteratorType iter = start; iter < end; iter += step) {
				jobs.push_back(fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};
		
		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); } */
		template<typename containerType, typename F>
		AUTO ForEach(containerType const& container, F&& ToDo) {
			AUTO todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;

			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs.push_back(fibers::Job([todo](typename containerType::const_iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(const_cast<typename containerType::value_type*>(&*T), [](typename containerType::value_type*) {})).cast()); }, (typename containerType::const_iterator)(iter)));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};
		
		/* parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); } */
		template<typename containerType, typename F>
		AUTO ForEach(containerType& container, F&& ToDo) {
			AUTO todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;

			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs.push_back(fibers::Job([todo](typename containerType::iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(&*T, [](typename containerType::value_type*) {})).cast()); }, (typename containerType::iterator)(iter)));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};
	};
	namespace synchronization {
		/* Tool that allows fibers to busy-work until a signla is raised by another fiber or thread. */
		class signal {
		public:
			class impl {
			public:
				impl(bool manualReset = true) : Handle(CreateEvent(NULL, manualReset, FALSE, NULL), [](void* p) { CloseHandle(p); }), wg() {};
				impl(const impl&) = delete;
				impl(impl&& that) = delete;
				impl& operator=(const impl&) = delete;
				impl& operator=(impl&& that) = delete;
				~impl() {};

			public:
				void	Raise() noexcept {
					SetEvent(Handle.get());
				};
				void	Clear() noexcept {
					ResetEvent(Handle.get());
				};
				void	Wait() noexcept {
					Job job([this]()->bool {
						ftl::YieldThread();
						return TryWait();
						});

					for (; !TryWait(); ) {
						wg.ForceQueue(job);
						auto reply = wg.Wait_Get(); /* Busy-waits. May do the job we just posted, but also may do other jobs on the queue. */
						bool passed = reply[0].cast();
						if (passed) break;
					}
				};
				bool	TryWait() noexcept {
					return WaitForSingleObject(Handle.get(), 1) == ((((DWORD)0x00000000L)) + 0);
				};

			protected:
				std::shared_ptr<void> Handle;
				JobGroup wg;
			};

		public:
			signal(bool manualReset = true) : signal_impl(new impl(manualReset)) {};
			signal(const signal&) = default;
			signal(signal&& that) = default;
			signal& operator=(const signal&) = default;
			signal& operator=(signal&& that) = default;
			~signal() {};

		public:
			/* Raise (set to one) the signal. Free & fast. */
			void	Raise() noexcept { signal_impl->Raise(); };
			/* Clear (zero-out) the signal. Free & fast. */
			void	Clear() noexcept { signal_impl->Clear(); };
			/* Busy-waits for the signal to be raised. */
			void	Wait() noexcept { signal_impl->Wait(); };
			/* Tests if the signal has been raised. Free & fast. */
			bool	TryWait() noexcept { return signal_impl->TryWait(); };

		private:
			std::shared_ptr<impl> signal_impl;

		};

		/* Read-Write mutex that allows multiple readers and one writer to cooperatively access an underlying object. Very fast for 100% reading operations, as (effectively) no locking actually happens. */
		class shared_mutex {
		private:
			mutex    mut_;
			std::condition_variable_any gate1_;
			std::condition_variable_any gate2_;
			unsigned state_;

			static const unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
			static const unsigned n_readers_ = ~write_entered_;

		public:

			shared_mutex() : state_(0) {}

			// Exclusive/Writer ownership
			void lock() {
				std::unique_lock<mutex> lk(mut_);
				while (state_ & write_entered_) gate1_.wait(lk);
				state_ |= write_entered_;
				while (state_ & n_readers_) gate2_.wait(lk);
			};
			// Exclusive/Writer ownership
			bool try_lock() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock);
				if (lk.owns_lock() && state_ == 0)
				{
					state_ = write_entered_;
					return true;
				}
				return false;
			};
			// Exclusive/Writer ownership
			void unlock() {
				{
					std::scoped_lock<mutex> _(mut_);
					state_ = 0;
				}
				gate1_.notify_all();
			};

			// Shared/Reader ownership
			void lock_shared() {
				std::unique_lock<mutex> lk(mut_);
				while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_)
					gate1_.wait(lk);
				unsigned num_readers = (state_ & n_readers_) + 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
			};
			// Shared/Reader ownership
			bool try_lock_shared() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock);
				unsigned num_readers = state_ & n_readers_;
				if (lk.owns_lock() && !(state_ & write_entered_) && num_readers != n_readers_)
				{
					++num_readers;
					state_ &= ~n_readers_;
					state_ |= num_readers;
					return true;
				}
				return false;
			};
			// Shared/Reader ownership
			void unlock_shared() {
				std::scoped_lock<mutex> _(mut_);
				unsigned num_readers = (state_ & n_readers_) - 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
				if (state_ & write_entered_)
				{
					if (num_readers == 0)
						gate2_.notify_one();
				}
				else
				{
					if (num_readers == n_readers_ - 1)
						gate1_.notify_one();
				}
			};

			NODISCARD AUTO Write_Guard() noexcept { return std::lock_guard(*this); };
			NODISCARD AUTO Read_Guard() noexcept { return std::shared_lock(*this); };
		};
	};
};

#undef FTL_NUM_WAITING_FIBER_SLOTS
#undef FTL_CPP_17















class ExampleOptimization {
public:
	static double Evaluation(std::vector<double> const& params, std::shared_ptr<cweeSysInterlockedInteger> count) {
		Stopwatch sw;
		sw.Start();
		
		count->Increment();

		return sw.Stop();
	}
	static std::vector<double> Create() {
		return std::vector<double>();
	}
	static std::vector<double> Iteration(int numTasks, std::shared_ptr<cweeSysInterlockedInteger> count) {
		if (numTasks == 1 || numTasks == -1) {
			Action todo(ExampleOptimization::Evaluation, ExampleOptimization::Create(), count);
			todo.Invoke();
		}
		else {
			if (numTasks < 0) numTasks *= -1;

			
			fibers::parallel::For(0, numTasks, [=](int i) {
				ExampleOptimization::Evaluation(ExampleOptimization::Create(), count);
				return;
			});
		}
		return std::vector<double>();
	}
	static std::vector<double> Solve(int numIterations, int numPolicies, std::shared_ptr<cweeSysInterlockedInteger> count) {
		if (numIterations == 1 || numIterations == -1) {
			Action todo(ExampleOptimization::Iteration, numPolicies, count);
			todo.Invoke();
		}
		else {
			if (numIterations < 0) numIterations *= -1;
			for (int i = 0; i < numIterations; ++i) { // iterations are in series so we can adapt the logic as it goes
				fibers::Job(ExampleOptimization::Iteration, numPolicies, count).AsyncInvoke().Wait();
			}
		}
		return std::vector<double>();
	}
};

fibers::Job FTL::fnFiberTasks2d(int numTasks, int numSubTasks) {
	return fibers::Job([](int numTasks, int numSubTasks) {
		cweeSysInterlockedInteger counter;
		if (numTasks < 0) numTasks *= -1;
		if (numSubTasks < 0) numSubTasks *= -1;
		fibers::Job([&]() { // not required to be a job
			for (int i = 0; i < numTasks; ++i) { // iterations are actually in series
				fibers::Job([&numSubTasks, &counter]() { // not required to be a job
					fibers::parallel::For(0, numSubTasks, [&counter](int j) {  // policies / particles are done simultanously using the fibers::For or fibers::ForEach loops
						counter.Increment(); // do work
					});
				}).AsyncInvoke().Wait();
			}
		}).AsyncInvoke().Wait();

		return counter.GetValue();
	}, (int)numTasks, (int)numSubTasks);
};
fibers::Job FTL::fnFiberTasks2b(int numTasks) {
	return fibers::Job([](int numTasks) {
		cweeSysInterlockedInteger numParallel = 0;
		cweeSysInterlockedInteger maxParallel = 0;

		std::shared_ptr<fibers::synchronization::signal> Signal = std::make_shared< fibers::synchronization::signal>();
	
		Stopwatch sw; sw.Start();
		AUTO timer = Timer(
			2.0, 
			[&sw, &Signal]() {
				cweeToasts->submitToast(
					"I AM A TIMER", 
					cweeStr::printf("%f seconds passed", (float)(cweeUnitValues::second(cweeUnitValues::nanosecond(sw.Stop()))()))
				); 
				Signal->Raise();
			}
		);

		Signal->Wait();

		cweeToasts->submitToast(
			"I WAITED FOR A SIGNAL",
			cweeStr::printf("%f seconds passed", (float)(cweeUnitValues::second(cweeUnitValues::nanosecond(sw.Stop()))()))
		);

		while (cweeUnitValues::nanosecond(sw.Stop()) < cweeUnitValues::second(5)) {
			ftl::YieldThread();
		}

		return 0;
	}, (int)numTasks);
};
fibers::Job FTL::fnFiberTasks2c(int numTasks) {
	return fibers::Job([](int numTasks) {
		cweeSysInterlockedInteger numParallel = 0;
		cweeSysInterlockedInteger maxParallel = 0;
	

		std::map<int, double> x;
		for (int i = 0; i < 10; i++) { x[i] = 0; }

		fibers::parallel::ForEach(x, [](std::pair<const int, double> const& v) {
			return v.second;
		});
		fibers::parallel::ForEach(const_cast<const std::map<int, double>&>(x), [](std::pair<const int, double>& v) {
			return v.first;
		});

		if (numTasks > 0) {
			cweeList<int> intList; 
			for (int i = 0; i < numTasks; i++) intList.push_back(i);
	#if 1
			fibers::parallel::ForEach(intList, [&numParallel, &maxParallel](int& j) {
				int n; Stopwatch sw; auto maxT = cweeUnitValues::millisecond(1);

				sw.Start();

				n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				while (cweeUnitValues::nanosecond(sw.Stop()) < maxT) { ftl::YieldThread(); }

				numParallel.Decrement();
			});
	#else
			fibers::For(0, numTasks, [&numParallel, &maxParallel](int j) {
				int n; Stopwatch sw; auto maxT = cweeUnitValues::millisecond(1);

				sw.Start();

				n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				while (cweeUnitValues::nanosecond(sw.Stop()) < maxT) { ftl::YieldThread(); }

				numParallel.Decrement();
			});
	#endif
			return maxParallel.GetValue();
		}
		else {
			numTasks *= -1;

			std::vector<fibers::Job> tasks;

			for (int i = 0; i < numTasks; ++i) {
				fibers::Job todo;
				switch (cweeRandomInt(1, 4)) {
				default:
				case 1:
					todo = fibers::Job([&numParallel, &maxParallel](double& j) -> std::remove_reference_t<decltype(j)> {
						Stopwatch sw; sw.Start();

						int n = numParallel.Increment();
						if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

						while (cweeUnitValues::nanosecond(sw.Stop()) < cweeUnitValues::millisecond(1)) {
							ftl::YieldThread();
						}

						//auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
						numParallel.Decrement();

						return j + 100.0f;
						}, double(i));
					break;
				case 2:
					todo = fibers::Job([&numParallel, &maxParallel](float j) -> std::remove_reference_t<decltype(j)> {
						Stopwatch sw; sw.Start();

						int n = numParallel.Increment();
						if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

						while (cweeUnitValues::nanosecond(sw.Stop()) < cweeUnitValues::millisecond(1)) {
							ftl::YieldThread();
						}

						numParallel.Decrement();

						return j + 100.0f;
						}, float(i));
					break;
				case 3:
					todo = fibers::Job([&numParallel, &maxParallel](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
						Stopwatch sw; sw.Start();

						int n = numParallel.Increment();
						if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

						while (cweeUnitValues::nanosecond(sw.Stop()) < cweeUnitValues::millisecond(1)) {
							ftl::YieldThread();
						}

						numParallel.Decrement();

						j->ToUpper();
						return *j;
						}, cweeStr(i));
					break;
				case 4:
					todo = fibers::Job([&numParallel, &maxParallel](std::string const& j) -> std::remove_reference_t<decltype(j)> {
						Stopwatch sw; sw.Start();

						int n = numParallel.Increment();
						if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

						while (cweeUnitValues::nanosecond(sw.Stop()) < cweeUnitValues::millisecond(1)) {
							ftl::YieldThread();
						}

						numParallel.Decrement();

						return j + ": TEST";
						}, std::to_string(i));
					break;
				}

				tasks.push_back(todo);
			}

			fibers::JobGroup awaiter;
			awaiter.Queue(tasks);
			awaiter.Wait();

			return maxParallel.GetValue();
		}
	}, (int)numTasks);
};

#include <random>
class fiber_rand {
public:
	class cwee_pcg {
	public:
		using result_type = uint32_t;
		static constexpr result_type(min)() { return 0; }
		static constexpr result_type(max)() { return UINT32_MAX; }

		cwee_pcg() noexcept : m_state(0), m_inc(0), rd() { seed(); };
		void seed() noexcept {
			uint64_t s0 = uint64_t(rd()) << 31 | uint64_t(rd());
			uint64_t s1 = uint64_t(rd()) << 31 | uint64_t(rd());
			m_state = 0;
			m_inc = (s1 << 1) | 1;
			(void)operator()();
			m_state += s0;
			(void)operator()();
		};
		result_type operator()() const noexcept {
			uint64_t oldstate = m_state.load();
			m_state.store(oldstate * 6364136223846793005ULL + m_inc);
			uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
			int rot = oldstate >> 59u;
			return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
		};
		void discard(unsigned long long n) const noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };

	private:
		mutable std::atomic_int64_t m_state;
		uint64_t m_inc;
		std::random_device rd;

	};

private:
	mutable cwee_pcg rand;
	mutable std::uniform_real_distribution<u64> u;

public:
	fiber_rand() noexcept : /*mut(), */rand(), u(0.0, 1.0) { Random_Impl(); /*Instantiate the range*/ };
	u64 Random(u64 t1 = 0.0, u64 t2 = 1.0) const noexcept { return Random_HighRes(std::move(t1), std::move(t2)); };
	double Random(double t1 = 0.0, double t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	float Random(float t1 = 0.0, float t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	int Random(int t1 = 0, int t2 = std::numeric_limits<int>::max()) const noexcept { return std::floor(Random_HighRes(t1, t2) + 0.5); };

private:
	u64 Random_Impl() const noexcept {
		return u(rand);
	};
	u64 Random_HighRes(u64 t1, u64 t2) const noexcept {
		t2 -= t1;
		t2 *= Random_Impl();
		t1 += t2;
		return t1;
	};

};
extern DelayedInstantiation< fiber_rand > FiberRandomGenerator = DelayedInstantiation< fiber_rand >([]()-> fiber_rand* { return new fiber_rand(); });

/*! random float between 0 and 1 */ INLINE float fiberRandomFloat() { return FiberRandomGenerator->Random(0.0f, 1.0f); };
/*! random float between 0 and max */ INLINE float fiberRandomFloat(float max) { return FiberRandomGenerator->Random(0.0f, max); };
/*! random float between min and max */ INLINE float fiberRandomFloat(float min, float max) { return FiberRandomGenerator->Random(min, max); };
/*! random int between 0 and cweeMath::INF */ INLINE int fiberRandomInt() { return FiberRandomGenerator->Random(0, std::numeric_limits<int>::max()); };
/*! random int between 0 and max */ INLINE int fiberRandomInt(int max) { return FiberRandomGenerator->Random(0, max); };
/*! random int between min and max */ INLINE int fiberRandomInt(int min, int max) { return FiberRandomGenerator->Random(min, max); };

extern void DoJob(ftl::TaskScheduler* taskScheduler, void* arg) {
	AUTO job = std::shared_ptr<Action>(static_cast<Action*>(arg));
	job->Invoke();
};
fibers::Job FTL::fnFiberTasks3(int numTasks, int numSubTasks) {
	return fibers::Job([](int numTasks, int numSubTasks) {
		{
			fibers::containers::unordered_map<int, fibers::containers::unordered_map<int, double>> lists;
			fibers::parallel::For(0, numTasks, [&lists, &numSubTasks](int j) {
				auto list = lists[j];
				{
					if (numSubTasks <= 1) {
						*list->operator[](0) = fiberRandomFloat(0, 100);
					}
					else {
						fibers::parallel::For(0, numSubTasks, [&list, &numSubTasks](int index) {
							*list->operator[](index) = fiberRandomFloat(0, 100);
							});
					}
				}
				});
		}

		{
			fibers::containers::vector<fibers::containers::vector<double>> vectors;
			fibers::parallel::For(0, numTasks, [&vectors, &numSubTasks](int j) {
				fibers::containers::vector<double> list;

				if (numSubTasks <= 1) {
					list.push_back(fiberRandomFloat(0, 100));
				}
				else {
					fibers::parallel::For(0, numSubTasks, [&list, &numSubTasks](int index) {
						list.push_back(fiberRandomFloat(0, 100));
						});
				}

				vectors.push_back(std::move(list));
				});
		}

		{
			fibers::containers::array<fibers::containers::array<double>> arrays;
			fibers::parallel::For(0, numTasks, [&arrays, &numSubTasks](int j) {
				fibers::containers::array<double> arr;

				if (numSubTasks <= 1) {
					arr.push_back(fiberRandomFloat(0, 100));
				}
				else {
					fibers::parallel::For(0, numSubTasks, [&arr, &numSubTasks](int index) {
						arr.push_back(fiberRandomFloat(0, 100));
						});
				}

				arrays.push_back(std::move(arr));
				});
		}

		return 0;
	}, (int)numTasks, (int)numSubTasks);
};





