// FiberPool based on:
// http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
#pragma once
#include "../WaterWatchCpp/Precompiled.h"

#include "FiberTasks.h"

// FiberTasks.cpp : Defines the functions for the static library.

#undef FTL_FIBER_CANARY_BYTES
#define FTL_NUM_WAITING_FIBER_SLOTS 4
#undef FTL_VALGRIND
#undef FTL_FIBER_STACK_GUARD_PAGES
#define FTL_CPP_17
#undef FTL_WERROR
#undef FTL_DISABLE_ITERATOR_DEBUG

#include "ftl/task_scheduler.h"
#include "ftl/wait_group.h"
#include "ftl/fibtex.h"

#include <assert.h>
#include <stdint.h>


struct NumberSubset {
	uint64_t start;
	uint64_t end;

	uint64_t total;
};
void AddNumberSubset(ftl::TaskScheduler* taskScheduler, void* arg) {
	(void)taskScheduler;
	NumberSubset* subset = static_cast<NumberSubset*>(arg);

	subset->total = 0;

	while (subset->start != subset->end) {
		subset->total += subset->start;
		++subset->start;
	}

	subset->total += subset->end;
}

uint64_t FTL::fnFiberTasks() {
	//if constexpr (true) {
		// Create the task scheduler and bind the main thread to it
		ftl::TaskScheduler taskScheduler;
		taskScheduler.Init();

		// Define the constants to test
		constexpr uint64_t triangleNum = 47593243ULL;
		constexpr uint64_t numAdditionsPerTask = 10000ULL;
		constexpr uint64_t numTasks = (triangleNum + numAdditionsPerTask - 1ULL) / numAdditionsPerTask;

		// Create the tasks
		// FTL allows you to create Tasks on the stack.
		// However, in this case, that would cause a stack overflow
		decltype(auto) tasks = std::shared_ptr< ftl::Task[]>(new ftl::Task[numTasks]);
		decltype(auto) subsets = std::shared_ptr< NumberSubset[]>(new NumberSubset[numTasks]);

		uint64_t nextNumber = 1ULL;

		for (uint64_t i = 0ULL; i < numTasks; ++i) {
			decltype(auto) subset = &subsets[i];

			subset->start = nextNumber;
			subset->end = nextNumber + numAdditionsPerTask - 1ULL;
			if (subset->end > triangleNum) {
				subset->end = triangleNum;
			}

			tasks[i] = { AddNumberSubset, subset };

			nextNumber = subset->end + 1;
		}

		// Schedule the tasks
		ftl::WaitGroup wg(&taskScheduler);
		taskScheduler.AddTasks(numTasks, tasks.get(), ftl::TaskPriority::Normal, &wg);

		// FTL creates its own copies of the tasks, so we can safely delete the memory
		//delete[] tasks;

		// Wait for the tasks to complete
		wg.Wait();

		// Add the results
		uint64_t result = 0ULL;
		for (uint64_t i = 0; i < numTasks; ++i) {
			result += subsets[i].total;
		}

		// Test
		assert(triangleNum * (triangleNum + 1ULL) / 2ULL == result);

		// Cleanup
		//delete[] subsets;

		// The destructor of TaskScheduler will shut down all the worker threads
		// and unbind the main thread
		return result;
	//}
};

#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/cweeInterlocked.h"
#include "../WaterWatchCpp/cwee_math.h"
#include "../WaterWatchCpp/Toasts.h"
#include "../WaterWatchCpp/cweeJob.h"
#include "../WaterWatchCpp/cweeTime.h"

uint64_t FTL::fnFiberTasks2a(int numTasks) {
	ftl::TaskScheduler taskScheduler;
	taskScheduler.Init();

	struct FuncStruct {
		cweeAction job;
		ftl::WaitGroup* wg;
	};
	auto lambda = [](ftl::TaskScheduler* taskScheduler, void* arg) {
		std::shared_ptr< FuncStruct > data(static_cast<FuncStruct*>(arg));
		auto* anyType = data->job.Invoke();
		if (anyType) {
			if (anyType->IsTypeOf<double>()) {
				double* v = anyType->cast();
			}
			else if (anyType->IsTypeOf<float>()) {
				float* v = anyType->cast();
			}
			else if (anyType->IsTypeOf<cweeStr>()) {
				cweeStr* v = anyType->cast();
			}
			else if (anyType->IsTypeOf<std::string>()) {
				std::string* v = anyType->cast();
			}
			else {
				throw(std::runtime_error("ERROR!"));
			}
		}


	};

	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;

	ftl::WaitGroup wg(&taskScheduler);
	for (int i = 0; i < numTasks; ++i) {
		cweeAction todo;
		switch (cweeRandomInt(1, 4)) {
		case 1:
			todo = cweeAction(cweeFunction(std::function([&numParallel, &maxParallel](double& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment(); 
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				//auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) < cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j;
			}), double(i)));
			break;
		case 2:
			todo = cweeAction(cweeFunction(std::function([&numParallel, &maxParallel](float j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				//auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) < cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j;
			}), float(i)));
			break;
		case 3:
			todo = cweeAction(cweeFunction(std::function([&numParallel, &maxParallel](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				//auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) < cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return *j;
			}), cweeStr(i)));
			break;
		case 4:
			todo = cweeAction(cweeFunction(std::function([&numParallel, &maxParallel](std::string const& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				//auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) < cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j;
			}), std::to_string(i)));
			break;
		}
		taskScheduler.AddTask({ lambda, new FuncStruct({ todo, &wg }) }, ftl::TaskPriority::Normal, &wg);
	}
	wg.Wait();

	return maxParallel.GetValue();
};

#include "../WaterWatchCpp/Clock.h"
#include "../WaterWatchCpp/Iterator.h"

#include <atomic>
#include <array>
#include <thread>
/*
*	shared_mutex (C) 2017 E. Oriani, ema <AT> fastwebnet <DOT> it
*
*	This file is part of shared_mutex.
*
*	shared_mutex is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	shared_mutex is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with nettop.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <ppl.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_set.h>


namespace fibers {
	// Original implemtation could not re-use previous ring buffers after increasing the size of the ring buffer, due to concerns of race conditions.
    // I fixed those issues by introducing a reverse-linked list of ring buffers that is thread-safe and allows ring buffer re-use. 
	extern DelayedInstantiation< ftl::TaskScheduler > Fibers = DelayedInstantiation< ftl::TaskScheduler >([]()-> ftl::TaskScheduler* {
		auto* p = new ftl::TaskScheduler();
		p->Init({ ftl::GetNumHardwareThreads() * 50, 0, ftl::EmptyQueueBehavior::Sleep, {} });
		p->SetEmptyQueueBehavior(ftl::EmptyQueueBehavior::Sleep);
		return p;
	});

	namespace {
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
	

	
	}

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

	template<typename _Ty>
	class vector {
	protected:
		std::shared_ptr< concurrency::concurrent_vector<_Ty> > data;

	public:
		struct it_state {
			std::shared_ptr< concurrency::concurrent_vector<_Ty> > lifetime;
			typename concurrency::concurrent_vector<_Ty>::iterator pos;
			inline void begin(const vector* ref) { lifetime = ref->data; pos = lifetime->begin(); }
			inline void next(const vector* ref) { ++pos; }
			inline void end(const vector* ref) { lifetime = ref->data; pos = lifetime->end(); }
			inline _Ty& get(vector* ref) { return *pos; }
			inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
			inline long long distance(const it_state& s) const { return pos - s.pos; };
			// Optional to allow operator--() and reverse iterators:
			inline void prev(const vector* ref) { --pos; }
			// Optional to allow `const_iterator`:
			inline const _Ty& get(const vector* ref) const { return *pos; }
		};
		SETUP_STL_ITERATOR(vector, _Ty, it_state);

		vector() : data(new concurrency::concurrent_vector<_Ty>()) {};
		explicit vector(size_type _N) : data(new concurrency::concurrent_vector<_Ty>(_N)) {};
		vector(size_type _N, const_reference _Item) : data(new concurrency::concurrent_vector<_Ty>(_N, _Item)) {};
		vector(vector const& r) : data(new concurrency::concurrent_vector<_Ty>()) {
			operator=(r);
		}
		vector(vector&& r) = default;
		vector& operator=(vector const& r) {
			if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
				vector out;
				for (auto& x : r)
					out->push_back(x);
				out.data.swap(data);
			}
			return *this;
		};
		vector& operator=(vector&& r) = default;
		~vector() {};

		AUTO grow_by(size_type _Delta) { return data->grow_by(_Delta); };
		AUTO grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
		AUTO grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
		AUTO push_back(const_reference _Item) { return data->push_back(_Item); };
		AUTO push_back(_Ty&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
		AUTO operator[](size_type _Index) { return data->operator[](_Index); };
		AUTO operator[](size_type _Index) const { return data->operator[](_Index); };
		AUTO at(size_type _Index) { return data->at(_Index); };
		AUTO at(size_type _Index) const { return data->at(_Index); };
		AUTO size() const { return data->size(); };
		AUTO empty() const { return data->empty(); };
		AUTO capacity() const { return data->capacity(); };
		AUTO max_size() const { return data->max_size(); };
		AUTO erase(const_iterator _Where) {
			vector out;
			auto endIter = end();
			for (auto iter = begin(); iter != endIter; iter++) {
				if (iter == _Where) continue;
				out.push_back(*iter);
			}
			data.swap(out.data);
		};
		AUTO erase(const_iterator _First, const_iterator _Last) {
			vector out;
			auto endIter = end();
			for (auto iter = begin(); iter != endIter; iter++) {
				if (iter >= _First && iter <= _Last) continue;
				out.push_back(*iter);
			}
			data.swap(out.data);
		};
		AUTO clear() {
			vector out;
			data.swap(out.data);
		};
		AUTO swap(vector& r) {
			data.swap(r.data);
		};
	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup;

	/*! 
	Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wraper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	Job([](){ return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, allowing you to continue work on this thread.
	*/
	class Job { friend JobGroup;
	protected:
		mutable std::shared_ptr<Action> impl;

	public:
		static Job Finished() {
			AUTO toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished());
			return toReturn;
		};
		template <typename T> static Job Finished(const T& returnMe) {
			AUTO toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished(returnMe));
			return toReturn;
		};
		
		Job() : impl(std::make_shared<Action>()) {};
		Job(const Job& other) = default;
		Job(Job && other) = default;
		Job& operator=(const Job & other) = default;
		Job& operator=(Job && other) = default;
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<T>> && !std::is_same_v<Any, std::decay_t<T>> >>
		explicit Job(T function, Args... Fargs) : impl(new Action(function, Fargs...)) {};

	public:
		/* Do the task immediately, without using any thread/fiber tools. Does not do the task if it has been previously performed. */
		Any Invoke() noexcept {
			Any out;
			auto* p = impl->Invoke();
			if (p) out = *p;
			return out;
		};

		/* Do the task immediately, without using any thread/fiber tools, whether or not it has been performed before. */
		Any ForceInvoke() noexcept {
			Any out;
			auto* p = impl->ForceInvoke();
			if (p) out = *p;
			return out;
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. Does not do the task if it has been previously performed. */
		JobGroup AsyncInvoke();

		/* Add the task to a thread / fiber, and retrieve an awaiter group, whether or not it has been performed before. */
		JobGroup AsyncForceInvoke();

		/* Queue job, and return tool to await the result */
		uintptr_t DelayedInvoke(u64 milliseconds_delay) {
			cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
			return cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
				if (_anon_ptr != nullptr) {
					cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
					::Sleep(T->get<0>());
					T->get<1>().Invoke();
					delete T;
				}
				return 0;
				}), (void*)data, 1024);
		};
		
		/* Queue job, and return tool to await the result */
		uintptr_t DelayedForceInvoke(u64 milliseconds_delay) {
			cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
			return cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
				if (_anon_ptr != nullptr) {
					cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
					::Sleep(T->get<0>());
					T->get<1>().ForceInvoke();
					delete T;
				}
				return 0;
				}), (void*)data, 1024);
		};
		
		/* Queues the job onto a new thread to take place in the future */
		uintptr_t AsyncDelayedInvoke(u64 milliseconds_delay);
		
		/* Queues the job onto a new thread to take place in the future */
		uintptr_t AsyncDelayedForceInvoke(u64 milliseconds_delay);

		/* Returns the potential name of the static function, if one was provided. */
		const char* FunctionName() const {
			return impl->FunctionName();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any GetResult() {
			return Invoke();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any operator()() {
			return Invoke();
		};

		/* Checks if the job has been completed before */
		bool IsFinished() const {
			return impl->IsFinished();
		};

		bool ReturnsNothing() const {
			return impl->ReturnsNothing();
		};

	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup { friend Job;
	private:
		class JobGroupImpl {
		public:
			ftl::WaitGroup waitGroup;
			interlocked<fibers::vector<Job>> jobs;

			JobGroupImpl() : waitGroup(&*Fibers), jobs() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;
			~JobGroupImpl() {};
		};

	public:
		JobGroup() : impl(new JobGroupImpl()) {};
		JobGroup(Job const& job) : impl(new JobGroupImpl()) { Queue(job); };
		JobGroup(JobGroup const&) = delete;
		JobGroup(JobGroup&&) = delete;
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() {};

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			Fibers->AddTask({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, false }) }, ftl::TaskPriority::Normal, &impl->waitGroup);
			impl->jobs.GetExclusive()->push_back(job);
			return *this;
		};		
		/* Queue job, and return tool to await the result */
		JobGroup& ForceQueue(Job const& job) {
			Fibers->AddTask({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, true }) }, ftl::TaskPriority::Normal, &impl->waitGroup);
			impl->jobs.GetExclusive()->push_back(job);
			return *this;
		};		
		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				tasks.push_back({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, false }) });
				impl->jobs.GetExclusive()->push_back(job);
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};
		JobGroup& Queue(fibers::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				tasks.push_back({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, false }) });
				impl->jobs.GetExclusive()->push_back(job);
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& ForceQueue(std::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				tasks.push_back({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, true }) });
				impl->jobs.GetExclusive()->push_back(job);
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};
		JobGroup& ForceQueue(fibers::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				tasks.push_back({ DoAnyFuncStruct, new AnyFunctionStruct({ job.impl, nullptr, true }) });
				impl->jobs.GetExclusive()->push_back(job);
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};

		/* Await all jobs in this group, and get the return values (which may be empty) for each job */
		std::vector<Any> Wait_Get() {
			impl->waitGroup.Wait();

			typename decltype(JobGroupImpl::jobs)::Type out;
			impl->jobs.Lock();
			out.swap(impl->jobs.UnsafeRead());
			impl->jobs.Unlock();

			if (out.size() > 0) {
				std::vector<Any> any(out.size(), Any());
				for (auto& job : out) {
					any.push_back(job.GetResult());
				}
				return any;
			}
			else {
				return std::vector<Any>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->waitGroup.Wait();
			impl->jobs.Clear();
		};

	private:
		std::unique_ptr<JobGroupImpl> impl;

	};

	/* Queue job, and return tool to await the result */
	JobGroup Job::AsyncInvoke() { 
		return JobGroup(*this);
	};
	
	/* Queue job, and return tool to await the result */
	JobGroup Job::AsyncForceInvoke() { 
		return JobGroup(*this);
	};
	
	/* Queues the job onto a new thread to take place in the future */
	uintptr_t Job::AsyncDelayedInvoke(u64 milliseconds_delay) {
		cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
		return cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
				::Sleep(T->get<0>());
				auto awaiter = T->get<1>().AsyncInvoke();
				awaiter.Wait();
				delete T;
			}
			return 0;
		}), (void*)data, 1024);
	};
	
	/* Queues the job onto a new thread to take place in the future */
	uintptr_t Job::AsyncDelayedForceInvoke(u64 milliseconds_delay) {
		cweeUnion< u64, Job >* data = new cweeUnion<u64, Job>(milliseconds_delay, *this);
		return cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
			if (_anon_ptr != nullptr) {
				cweeUnion< u64, Job >* T = static_cast<cweeUnion< u64, Job>*>(_anon_ptr);
				::Sleep(T->get<0>());
				auto awaiter = T->get<1>().AsyncForceInvoke();
				awaiter.Wait();
				delete T;
			}
			return 0;
		}), (void*)data, 1024);
	};

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
					auto reply = wg.Wait_Get();
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

		// Exclusive ownership
		void lock() {
			std::unique_lock<mutex> lk(mut_);

			while (state_ & write_entered_)
				gate1_.wait(lk);
			state_ |= write_entered_;
			while (state_ & n_readers_)
				gate2_.wait(lk);
		};
		bool try_lock() {
			std::unique_lock<mutex> lk(mut_, std::try_to_lock);
			if (lk.owns_lock() && state_ == 0)
			{
				state_ = write_entered_;
				return true;
			}
			return false;
		};
		void unlock() {
			{
				std::scoped_lock<mutex> _(mut_);
				state_ = 0;
			}
			gate1_.notify_all();
		};

		// Shared ownership
		void lock_shared() {
			std::unique_lock<mutex> lk(mut_);
			while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_)
				gate1_.wait(lk);
			unsigned num_readers = (state_ & n_readers_) + 1;
			state_ &= ~n_readers_;
			state_ |= num_readers;
		};
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
		constexpr bool retNo = std::is_same<typename function_traits<decltype(todo)>::result_type, void>::value;

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
		constexpr bool retNo = std::is_same<typename function_traits<decltype(todo)>::result_type, void>::value;

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
		constexpr bool retNo = std::is_same<typename function_traits<decltype(todo)>::result_type, void>::value;

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
		constexpr bool retNo = std::is_same<typename function_traits<decltype(todo)>::result_type, void>::value;

		std::vector<fibers::Job> jobs;

		for (auto iter = container.begin(); iter != container.end(); iter++) {
			jobs.push_back(fibers::Job([todo](typename containerType::iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(&*T, [](typename containerType::value_type*) {})).cast()); }, (typename containerType::iterator)(iter)));
		}

		JobGroup group;
		group.Queue(jobs);

		if constexpr (retNo) group.Wait();
		else return group.Wait_Get();
	};


	template<typename _Key_type, typename _Element_type>
	class unordered_map {
	protected:
		using underlying = typename concurrency::concurrent_unordered_map<_Key_type, _Element_type>;
		std::shared_ptr< underlying > data;

	public:
		struct it_state {
			std::shared_ptr< underlying > lifetime;
			typename underlying::iterator pos;

			inline void begin(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->begin(); }
			inline void next(const unordered_map* ref) { ++pos; }
			inline void end(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->end(); }
			inline typename underlying::value_type& get(unordered_map* ref) { return *pos; }
			inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
			inline long long distance(const it_state& s) const { return pos - s.pos; };
			inline void prev(const unordered_map* ref) { --pos; }
			inline const typename underlying::value_type& get(const unordered_map* ref) const { return *pos; }
		};
		SETUP_STL_ITERATOR(unordered_map, typename underlying::value_type, it_state);

		typedef typename underlying::key_type key_type;
		typedef typename underlying::mapped_type mapped_type;
		typedef typename underlying::key_equal key_equal;
		typedef typename underlying::hasher hasher;
		typedef iterator local_iterator;
		typedef const_iterator const_local_iterator;

		unordered_map() : data(new underlying()) {};
		explicit unordered_map(size_type _N) : data(new underlying(_N)) {};
		unordered_map(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
		template<class _InputIterator> unordered_map(_InputIterator _Begin, _InputIterator _End) : data(new underlying(_Begin, _End)) {};
		unordered_map(unordered_map const& r) : data(new underlying()) {
			this->operator=(r);
		};
		unordered_map(unordered_map&& r) = default;
		unordered_map& operator=(unordered_map const& r) {
			if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
				unordered_map out;
				for (auto& x : *r.data) out[x.first] = x.second;
				out.data.swap(data);
			}
			return *this;
		};
		unordered_map& operator=(unordered_map&& r) = default;

		AUTO insert(const value_type& _Value) { return data->insert(_Value); };
		AUTO insert(const_iterator _Where, const value_type& _Value) { return data->insert(_Where, _Value); };
		template<class _Iterator> AUTO insert(_Iterator _First, _Iterator _Last) { return data->insert(_First, _Last); };
		template<class _Valty> AUTO insert(_Valty&& _Value) { return data->insert(_Value); };
		template<class _Valty> AUTO insert(const_iterator _Where, _Valty&& _Value) { return data->insert(_Where, _Value); };
		AUTO hash_function() const { return data->hash_function(); };
		AUTO key_eq() const { return data->key_eq(); };
		AUTO operator[](const key_type& _Keyval) { return data->operator[](_Keyval); };
		AUTO operator[](const key_type& _Keyval) const { return data->operator[](_Keyval); };
		AUTO at(const key_type& _Keyval) { return data->at(_Keyval); };
		AUTO at(const key_type& _Keyval) const { return data->at(_Keyval); };
		AUTO front() { return data->front(); };
		AUTO front() const { return data->front(); };
		AUTO back() { return data->back(); };
		AUTO back() const { return data->back(); };
		AUTO erase(const_iterator _Where) {
			unordered_map out;
			auto endIter = end();
			for (auto iter = begin(); iter != endIter; iter++) {
				if (iter == _Where) continue;
				out.insert(*iter);
			}
			data.swap(out.data);
		};
		AUTO erase(const_iterator _First, const_iterator _Last) {
			unordered_map out;
			auto endIter = end();
			for (auto iter = begin(); iter != endIter; iter++){
				if (iter >= _First && iter <= _Last) continue;
				out.insert(*iter);
			}
			data.swap(out.data);
		};
		AUTO erase(const key_type& _Keyval) {
			unordered_map out;
			auto endIter = end();
			for (auto iter = begin(); iter != endIter; iter++) {
				if (iter->first == _Keyval) continue;
				out.insert(*iter);
			}
			data.swap(out.data);
		};	
		AUTO unsafe_erase(const key_type& _Keyval) {
			data->unsafe_erase(_Keyval);
		};

		AUTO count(const key_type& _Keyval) const { return data->count(_Keyval); };
		AUTO emplace(const key_type& _Keyval, const _Element_type& _Value) { return insert(value_type(_Keyval, _Value)); };
		AUTO emplace(const key_type& _Keyval, _Element_type&& _Value) { return insert(value_type(_Keyval, std::forward<_Element_type>(_Value))); };
		AUTO clear() {
			unordered_map out;			
			data.swap(out.data);
		};
	};

	template<typename _Key_type> using unordered_set = concurrency::concurrent_unordered_set<_Key_type>;
	template<typename _Value_type> using queue = concurrency::concurrent_queue<_Value_type>;

	/*!
	Thread-safe list that performs garbage collection and manages read/write/create/delete operations on data. Intended to act as a multi-threaded database.
	*/
	template< typename Key, typename Value>
	class fiberMap {
	public:
		typedef Value						Type;
		typedef std::shared_ptr < Type >		PtrType;
		typedef std::pair<Key, PtrType>		_iterType;
		typedef long long					IdxType;

		class fiberMap_Impl {
		public:
			/*! prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope. */
			NODISCARD auto	Guard(void) const { // assumes unlocked!
				return lock.Write_Guard();
			}
			NODISCARD auto	SharedGuard(void) const { // assumes unlocked!
				return lock.Read_Guard();
			}
			/*! Prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock -- A "Unlock" must be called to re-enable access to the list. */
			void			Lock(void) const { // assumes unlocked!
				lock.lock();
			}
			/*! Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior. */
			void			Unlock(void) const { // assumes already locked! 
				lock.unlock();
			};
			/*! After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping. */
			PtrType			UnsafeRead(Key index) const { // was non-const			
				auto it = list.find(index);
				if (it != list.end()) {
					return it->second;
				}
				return nullptr;
			}
			/*! After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list. */
			vector<Key> UnsafeList(void) const {
				vector<Key> out;

				if (lastCreatedListVersion == CreatedListVersion) {
					out = indexList;
					return out;
				}
				int size = list.size();

				indexList.clear();
				fibers::ForEach(list, [this](typename decltype(list)::value_type& kv) {
					this->indexList.push_back(kv.first);
				});

				lastCreatedListVersion = CreatedListVersion;

				out = indexList;

				return out;
			};
			AUTO			UnsafeIndexForKey(Key index) const {
				return std::distance(list.begin(), list.find(index));
			};

			/* Clear the list */
			void Clear() {
				AUTO g = Guard();

				list_version = 0;
				CreatedListVersion = 0;

				fibers::ForEach(list, [](typename decltype(list)::value_type& kv) {
					PtrType& p = const_cast<PtrType&>(kv.second); // need to access the underlying thing
					p = nullptr;
				});

				list.clear();

				indexList.clear();
				lastSearchID = Key();
				lastResult = list.end();
				lastVersion = -1;
				lastCreatedListVersion = -1;
			};

			fiberMap_Impl() : lock(), list(), indexList(), list_version(0), lastSearchID(), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1) {
				//list.reserve(16);
				//indexList.SetGranularity(16);
			};
			~fiberMap_Impl() {
				Clear();
			};

			/*! Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection */
			mutable shared_mutex													    lock;
			/* Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions. */
			mutable unordered_map< Key, PtrType>                                        list;
			/* Optimized search parameters */
			mutable vector<Key>												            indexList;
			/* Optimized search parameters */
			mutable cweeSysInterlockedInteger											list_version;
			/* Optimized search parameters */
			mutable Key																	lastSearchID;
			/* Optimized search parameters */
			mutable typename decltype(list)::iterator		                            lastResult;
			/* Optimized search parameters */
			mutable cweeSysInterlockedInteger											lastVersion;
			/* Optimized search parameters */
			mutable cweeSysInterlockedInteger											CreatedListVersion;
			/* Optimized search parameters */
			mutable cweeSysInterlockedInteger											lastCreatedListVersion;
		};

		typedef std::shared_ptr<fiberMap_Impl>		ImplType;

		struct it_state {
			inline void at(const fiberMap* ref, Key index) {
				ref->Lock();
				AUTO h = ref->UnsafeIndexForKey(index);
				ref->Unlock();
				at(ref, index, h);
			};
			inline void at(const fiberMap* ref, Key index, const IdxType t_hint) {
				listOfIndexes = ref->GetList();
				idx = 0;
				IdxType n = listOfIndexes.Num();


				//idx = t_hint <= n ? t_hint : n;
				//return;

				for (idx = t_hint; idx < n; ++idx) {
					if (listOfIndexes[idx] == index) {
						return;
					}
				}
				for (idx = 0; idx < n && idx < t_hint; ++idx) {
					if (listOfIndexes[idx] == index) {
						return;
					}
				}
				idx = n;
			};
			mutable vector<Key>	listOfIndexes;
			mutable IdxType idx;
			mutable _iterType iter;
			inline void begin(const fiberMap* ref) {
				listOfIndexes = ref->GetList();
				idx = 0;
			}
			inline void next(const fiberMap* ref) {
				++idx;
				while (idx < listOfIndexes.Num() && !ref->Check(listOfIndexes[idx])) {
					++idx;
				}
			}
			inline void end(const fiberMap* ref) {
				listOfIndexes = ref->GetList();
				idx = listOfIndexes.Num();
			}
			inline _iterType& get(fiberMap* ref) {
				iter = ref->GetIterator(listOfIndexes[idx]);
				return iter;
			}
			inline bool cmp(const it_state& s) const { return (idx == s.idx) ? false : true; }
			inline IdxType distance(const it_state& s) const { return idx - s.idx; }

			// Optional to allow operator--() and reverse iterators:
			inline void prev(const fiberMap* ref) {
				--idx;
				while (idx >= 0 && !ref->Check(listOfIndexes[idx])) {
					--idx;
				}
			}
			// Optional to allow `const_iterator`:
			inline const _iterType& get(const fiberMap* ref) const { iter = ref->GetIterator(listOfIndexes[idx]); return iter; }
		};

		SETUP_STL_ITERATOR(fiberMap, _iterType, it_state);

		/*! Default constructor */
		fiberMap() : impl(new fiberMap_Impl()) {};
		/*! Default copy */
		fiberMap(const fiberMap& other) : impl(new fiberMap_Impl()) { this->Copy(other); };
		/*! Take reference to underlying data */
		fiberMap(fiberMap&& other) : impl(other.impl) {};
		/*! Default copy */
		fiberMap& operator=(const fiberMap& other) {
			this->Copy(other);
			return *this;
		};
		/*! Default destructor */
		~fiberMap() {
			impl = nullptr;
		};
		/*! Convert UnorderedMap to a list of keys */
		operator vector<Key>() const {
			return GetList();
		};
		/*! Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack. */
		PtrType			operator[](Key index) const { return GetPtr(index); };
		/*! Swap if the object exists, otherwise append at the index and then swap. */
		bool			Emplace(Key index, const Value& obj) {
			bool out;
			Lock();
			out = UnsafeEmplace(index, obj);
			Unlock();
			return out;
		};
		/*! Swap if the object exists, otherwise append at the index and then swap. */
		bool			Emplace(Key index, Value&& obj) {
			bool out;
			Lock();
			out = UnsafeEmplace(index, std::forward<Value>(obj));
			Unlock();
			return out;
		};
		/*! Swap if the object exists, otherwise append at the index and then swap. */
		bool			UnsafeEmplace(Key index, const Value& obj) {
			bool out;
			auto ptr = UnsafeRead(index);
			if (ptr) {
				// already exists
				*ptr = obj;
				out = false;
			}
			else {
				// does not yet exist
				ptr = UnsafeAppendAt(index);
				if (ptr) {
					*ptr = obj;
				}
				out = true;
			}
			return out;
		};
		/*! Swap if the object exists, otherwise append at the index and then swap. */
		bool			UnsafeEmplace(Key index, Value&& obj) {
			bool out;
			auto ptr = UnsafeRead(index);
			if (ptr) {
				// already exists
				*ptr = std::forward<Value>(obj);
				out = false;
			}
			else {
				// does not yet exist
				ptr = UnsafeAppendAt(index);
				if (ptr) {
					*ptr = std::forward<Value>(obj);
				}
				out = true;
			}
			return out;
		};
		/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does.   */
		PtrType			UnsafeGetPtr(Key index) const {
			if (impl->list.count(index) <= 0)
				return UnsafeAppendAt(index);
			else
				return impl->list[index];
		};
		/*!  Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does.   */
		PtrType			GetPtr(Key index) const {
			PtrType out{ TryGetPtr(index) };
			if (!out) {
				Lock();
				out = UnsafeAppendAt(index);
				Unlock();
			}
			return out;
		};
		/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
		PtrType			TryGetPtr(Key index) const {
			auto* p = impl.get();
			if (p) {
				AUTO g{ p->lock.Read_Guard() };
				if (p->list.count(index) > 0) {
					return p->list.at(index);
				}
			}
			return nullptr;
		};
		/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
		_iterType		GetIterator(Key index) const {
			return _iterType(index, GetPtr(index));
		};
		/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
		_iterType		TryGetIterator(Key index) const {
			_iterType out;
			SharedLock();
			if (impl->list.count(index) > 0) {
				out = _iterType(index, impl->list[index]);
			}
			SharedUnlock();
			return out;
		};
		/*! True/False if the index exists on the heap */
		bool			Check(Key index) const {
			bool result = true;
			if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
				SharedLock();
				result = (impl->indexList.FindIndex(index) != -1);
				SharedUnlock();
				return result;
			}
			SharedLock();
			result = (impl->list.count(index) != 0);
			SharedUnlock();
			return result;
		};
		/*! True/False if the index exists on the heap */
		bool			UnsafeCheck(Key index) const {
			bool result = true;
			if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
				result = (impl->indexList.FindIndex(index) != -1);
				return result;
			}
			result = (impl->list.count(index) != 0);
			return result;
		};
		/*! Clear the current list and create a copy of the incoming list. */
		void			Copy(const fiberMap& copy, const bool threadSafePtr = false) {
			Clear();

			AUTO List = copy.GetList();

			if (threadSafePtr) {
				// Go through and perform swaps		
				fibers::For(0, List.Num(), [&](int i) {
					copy.Lock();
					auto readCopy = copy.UnsafeRead(List[i]);
					copy.Unlock();

					if (readCopy) {
						Lock();
						UnsafeAppendAt(List[i]);
						auto access = UnsafeRead(List[i]);
						Unlock();

						if (access) *access = *readCopy;
					}
				});
			}
			else {
				fibers::For(0, List.Num(), [&](int i) {
					copy.Lock();
					auto readCopy = copy.UnsafeRead(List[i]);
					if (readCopy) {
						Lock();
						UnsafeAppendAt(List[i]);
						auto access = UnsafeRead(List[i]);
						if (access) *access = *readCopy;
						Unlock();
					}
					copy.Unlock();
				});
			}
		};
		/*! Copy the Value object from the stack into the heap at the index specified. If the index does not exist on the heap, nothing happens. */
		void			Swap(Key index, const Value& replacement) {
			if (Check(index)) {
				while (!CheckCanDeleteIndex(index)) {};
				Lock();
				AUTO it = impl->lastResult;
				if (impl->lastSearchID == index && impl->list.count(impl->lastSearchID) != 0 && impl->lastVersion == impl->list_version)	it = impl->lastResult;
				else { it = impl->list.find(index); impl->lastSearchID = index; impl->lastResult = it; impl->lastVersion = impl->list_version; }
				impl->list_version.Increment();
				if (it != impl->list.end()) *it->second = replacement;
				Unlock();
			}
		};
		/*! get the number of objects on the heap managed by this list */
		int				Num(void) const {
			int i = 0;
			if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
				SharedLock();
				i = impl->indexList.Num();
				SharedUnlock();
				return i;
			}

			SharedLock();
			i = impl->list.size();
			SharedUnlock();
			return i;
		};
		/*! get the number of objects on the heap managed by this list */
		int				UnsafeNum(void) const {
			int i = 0;
			if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
				i = impl->indexList.Num();
				return i;
			}
			i = impl->list.size();
			return i;
		};
		/*! get the index position of the key */
		AUTO			UnsafeIndexForKey(Key index) const {
			return impl->UnsafeIndexForKey(std::move(index));
		};
		/*! get a list of all indexes currently on the heap that are currently valid */
		vector<Key> GetList(void) const {
			vector<Key> out;

			if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
				SharedLock();
				out = impl->indexList;
				SharedUnlock();
				return out;
			}
			int size = Num();

			Lock();
			impl->indexList.Clear();
			impl->indexList.SetGranularity(size + 16);
			fibers::ForEach(impl->list, [&](typename decltype(impl->list)::value_type& kv) {
				impl->indexList.Append(kv.first);
			});
			impl->lastCreatedListVersion = impl->CreatedListVersion;

			out = impl->indexList;

			Unlock();
			return out;
		};
		/*! Erase the indexed object from the heap and free the memory for the list to re-use. */
		void			Erase(Key index) {
			Lock();
			if (impl->list.count(index) > 0) {
				impl->list[index] = nullptr;
				impl->list.erase(index);
			}
			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
			Unlock();
		};
		/*! Clear the current list and free all memory back to the operating system */
		void			Clear(void) {
			impl->Clear();
		};
		/*!
		prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock
		A "Unlock" must be called to re-enable access to the list.
		*/
		void			Lock(void) const {
			impl->Lock();
		};
		/*!
		Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior.
		*/
		void			Unlock(void) const {
			impl->Unlock();
		};
		/*!
		prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope.
		*/
		NODISCARD auto	Guard(void) const { // assumes unlocked!
			return impl->Guard();
		}
		/*!
		After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping.
		*/
		PtrType			UnsafeRead(Key index) const {
			return impl->UnsafeRead(index);
		};
		/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying fiberMap lifetime does. */
		_iterType		UnsafeIterator(Key index) const {
			PtrType out;
			if (impl->list.count(index) <= 0) {
				out = UnsafeAppendAt(index);
			}
			else
			{
				out = impl->list[index];
			}
			return _iterType(index, out);
		};
		/*!
		After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list.
		*/
		vector<Key> UnsafeList(void) const {
			return impl->UnsafeList();
		};
		/*
		Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

		vector<int> obj;
		obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		vector<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
		for (auto& z : ptrsWithValuesGreaterThanFive){
			std::cout << *z << std::endl;
		}
		*/
		vector<_iterType> Select(std::function<bool(const Value*)> predicate) const {
			vector<_iterType> out;
			PtrType x;
			auto thisList = GetList();

			fibers::ForEach(thisList, [&](typename decltype(thisList)::value_type& i) {
				x = this->GetPtr(i);
				if (x && predicate(x.get())) {
					out.push_back(_iterType(i, x));
				}
			});

			return out;
		};
		/*
		Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

		vector<int> obj;
		obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		vector<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
		for (auto& z : ptrsWithValuesGreaterThanFive){
			std::cout << *z << std::endl;
			*z = -1; // modifies the original list
		}
		*/
		vector<_iterType> Select(std::function<bool(const Value*)> predicate) {
			vector<_iterType> out;
			PtrType x;
			auto thisList = GetList();
			fibers::ForEach(thisList, [&](typename decltype(thisList)::value_type& i) {
				x = this->GetPtr(i);
				if (x && predicate(x.get())) {
					out.push_back(_iterType(i, x));
				}
			});
			return out;
		};
		/*
		Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

		vector<int> obj;
		obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		vector<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
		for (auto& z : indexesWithValuesGreaterThanFive){
			std::cout << obj[z] << std::endl;
		}
		*/
		vector<Key> SelectIndexes(std::function<bool(const Value*)> predicate) const {
			vector<Key> out;
			_iterType x;
			auto thisList = GetList();
			fibers::ForEach(thisList, [&](typename decltype(thisList)::value_type& i) {
				x = this->GetPtr(i);
				if (x && predicate(x.get())) {
					out.push_back(i);
				}
			});
			return out;
		};
		/*!
		After calling "Lock", Append new data at a specified key location, regardless of the counter.
		*/
		PtrType			UnsafeAppendAt(Key index) const {
			if (impl->list.count(index) != 0) { return impl->list[index]; }

			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
			/* Allocate a _type_ and store at specified index */
			AUTO newPtr = std::make_shared<Value>();
			impl->list.emplace(index, newPtr);
			return newPtr;
		};
		/*!
		After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing.
		*/
		void			UnsafeErase(Key index) {
			if (impl->list.count(index) > 0) {
				impl->list.erase(index);
			}
			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
		};
		/*!
		After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
		*/
		bool	UnsafeExtract(Key index, PtrType& out) {
			bool result = false;
			if (impl->list.count(index) > 0) {
				out = impl->list[index];
				impl->list.erase(index);
				result = true;
			}
			impl->list_version.Increment();
			impl->CreatedListVersion.Increment();
			return result;
		};
		/*!
		After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
		*/
		bool	UnsafeExtractAny(PtrType& out) {
			bool result = false;
			for (auto& x : impl->list) {
				out = x.second;
				impl->list.erase(x.first);
				impl->list_version.Increment();
				impl->CreatedListVersion.Increment();
				return true;
			}
			return false;
		};
		/*!
		After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
		*/
		/*!
		After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
		*/
		bool	ExtractAny(Type& out) {
			bool res(false);
			Lock();
			AUTO bg = impl->list.cbegin();
			if (bg != impl->list.cend()) {
				if (bg->second) out = *bg->second;
				impl->list.erase(bg->first);
				impl->list_version.Increment();
				impl->CreatedListVersion.Increment();
				res = true;
			}
			Unlock();
			return res;
		};
		bool	Extract(Key index, PtrType& out) {
			bool result = false;
			AUTO G = Guard();
			result = UnsafeExtract(std::move(index), out);
			return result;
		};
		/*!
		After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
		*/
		bool	UnsafeExtractIndex(IdxType idx, PtrType& out) {
			_iterType p = unsafe_pair_at_index(idx);
			if (p.second) {
				UnsafeErase(p.first);
				out = p.second;
				return true;
			}
			return false;
		};
		/*!
		After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing, and return a copy of the object
		*/
		bool	ExtractIndex(IdxType idx, PtrType& out) {
			bool result = false;
			AUTO G = Guard();
			result = UnsafeExtractIndex(std::move(idx), out);
			return result;
		};

	public: // Compatability functions
		PtrType			at(Key index) const { return GetPtr(index); };
		PtrType			unsafe_at_index(IdxType idx) const {
			return unsafe_pair_at_index(idx).second;
		};
		_iterType		unsafe_pair_at_index(IdxType idx) const {
			_iterType out(-1, nullptr);
			bool foundKey = false;
			if (impl->lastCreatedListVersion == impl->CreatedListVersion) {
				if (impl->indexList.Num() > idx) {
					out.first = impl->indexList[idx];
					foundKey = true;
				}
			}
			if (!foundKey) {
				auto _keys = UnsafeList();
				if (_keys.Num() > idx) {
					out.first = _keys[idx];
					foundKey = true;
				}
			}
			if (foundKey) {
				out.second = UnsafeGetPtr(out.first);
			}
			return out;
		};
		PtrType			at_index(IdxType idx) const {
			PtrType out;
			Lock();
			out = unsafe_pair_at_index(std::move(idx)).second;
			Unlock();
			return out;
		};
		_iterType		pair_at_index(IdxType idx) const {
			_iterType out;
			Lock();
			out = unsafe_pair_at_index(std::move(idx));
			Unlock();
			return out;
		};
		void			erase_at(IdxType idx) {
			AUTO p = pair_at_index(idx);
			if (p.second) {
				Erase(p.first);
			}
		};
		void			erase(Key index) {
			Erase(index);
		};
		bool			empty() const {
			return (size() == 0);
		};
		void			clear() {
			Clear();
		};
		_iterType		back() const {
			return pair_at_index(Num() - 1);
		};
		iterator		find(Key s) noexcept {
			iterator itr = iterator(this);
			itr.state.at(this, s);
			return itr;
		}
		iterator		find(Key s, const IdxType t_hint) noexcept {
			iterator itr = iterator(this);
			itr.state.at(this, s, t_hint);
			return itr;
		}
		const_iterator	find(Key s) const noexcept {
			const_iterator itr = const_iterator(this);
			itr.state.at(this, s);
			return itr;
		}
		const_iterator	find(Key s, const IdxType t_hint) const noexcept {
			const_iterator itr = const_iterator(this);
			itr.state.at(this, s, t_hint);
			return itr;
		}
		AUTO			size() const noexcept { return Num(); }
		template<typename M> AUTO insert_or_assign(Key&& key, M&& m) {
			Emplace(key, m);
			return GetPtr(key);
		};
		template<typename M> AUTO insert_or_assign(const Key& key, M&& m) {
			Emplace(key, m);
			return GetPtr(key);
		};
		IdxType count(const Key& s) const noexcept {
			return Check(s) ? 1 : 0;
			// return (find(s) != end()) ? 1 : 0;
		}
		std::pair<iterator, bool> insert(std::pair<Key, Value>&& value) {
			iterator out = iterator(this);
			Lock();
			bool didNotExist = UnsafeEmplace(value.first, value.second);
			Unlock();
			out.state.at(this, value.first);
			return std::pair(out, didNotExist);
		};
		template<typename M> AUTO insert(Key&& key, M&& m) {
			Emplace(key, m);
			return GetPtr(key);
		};
		template<typename M> AUTO insert(const Key& key, M&& m) {
			Emplace(key, m);
			return GetPtr(key);
		};

	private:
		/*! Duplicate method to allow for advanced locking mechanisms. */
		void			SharedLock(void) const {
			impl->lock.shared_lock();
		};
		/*! Duplicate method to allow for advanced locking mechanisms. */
		void			SharedUnlock(void) const {
			impl->lock.shared_unlock();
		};

		ImplType impl;
	};






};

class ExampleOptimization {
public:
	static double Evaluation(std::vector<double> const& params, std::shared_ptr<cweeSysInterlockedInteger> count) {
		Stopwatch sw; 
		sw.Start(); 

		return count->Increment();

		//while (cweeUnitValues::nanosecond(sw.Stop()) < cweeUnitValues::millisecond(5)) {
			//ftl::YieldThread();
		//};
		return 0.0;
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

			fibers::For(0, numTasks, [=](int i) {
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
			for (int i = 0; i < numIterations; ++i) {
				fibers::Job(ExampleOptimization::Iteration, numPolicies, count).AsyncInvoke().Wait();
			}
		}
		return std::vector<double>();
	}
};

uint64_t FTL::fnFiberTasks2d(int numTasks, int numSubTasks) {
	std::shared_ptr<cweeSysInterlockedInteger> count = std::make_shared<cweeSysInterlockedInteger>();

	auto awaiter = fibers::Job(ExampleOptimization::Solve, (int)numTasks, (int)numSubTasks, count).AsyncInvoke();
	awaiter.Wait();
	
	return count->GetValue();
};
uint64_t FTL::fnFiberTasks2b(int numTasks) {
	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;

	std::shared_ptr<fibers::signal> Signal = std::make_shared< fibers::signal>();
	
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
};

#if 1

#else
using mutex = cweeReadWriteMutex; // cweeSysMutex;
#endif

uint64_t FTL::fnFiberTasks2c(int numTasks) {
	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;
	

	std::map<int, double> x;
	for (int i = 0; i < 10; i++) { x[i] = 0; }

	fibers::ForEach(x, [](std::pair<const int, double> const& v) {
		return v.second;
	});
	fibers::ForEach(const_cast<const std::map<int, double>&>(x), [](std::pair<const int, double>& v) {
		return v.first;
	});

	if (numTasks > 0) {
		cweeList<int> intList; 
		for (int i = 0; i < numTasks; i++) intList.push_back(i);
#if 1
		fibers::ForEach(intList, [&numParallel, &maxParallel](int& j) {
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

		fibers::vector<fibers::Job> tasks;

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
};










#include <random>
class fiber_rand {
public:
	class cwee_pcg {
	public:
		using result_type = uint32_t;
		static constexpr result_type(min)() { return 0; }
		static constexpr result_type(max)() { return UINT32_MAX; }

		cwee_pcg() noexcept : mut(), m_state(0), m_inc(0), rd() { seed(); };
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
			uint64_t oldstate;
			mut.Lock(); {
				oldstate = m_state;
				m_state = oldstate * 6364136223846793005ULL + m_inc;
			} mut.Unlock();
			uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
			int rot = oldstate >> 59u;
			return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
		};
		void discard(unsigned long long n) const noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };

	private:
		mutable fibers::mutex mut;
		mutable uint64_t m_state;
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

/*! Thread-safe list that performs garbage collection and manages read/write/create/delete operations on data. Intended to act as a multi-threaded database. */






extern void DoJob(ftl::TaskScheduler* taskScheduler, void* arg) {
	AUTO job = std::shared_ptr<Action>(static_cast<Action*>(arg));
	job->Invoke();
};
uint64_t FTL::fnFiberTasks3(int numTasks, int numSubTasks) {
	{
		fibers::unordered_map<int, fibers::unordered_map<int, double>> lists;
		fibers::For(0, numTasks, [&lists, &numSubTasks](int j) {
			auto& list = lists[j];
			if (numSubTasks <= 1) {
				list[0] = fiberRandomFloat(0, 100);
			}
			else {
				fibers::For(0, numSubTasks, [&list, &numSubTasks](int index) {
					list[index] = fiberRandomFloat(0, 100);
				});
			}
		});
	}

	{
		fibers::vector<fibers::vector<double>> vectors;
		fibers::For(0, numTasks, [&vectors, &numSubTasks](int j) {
			fibers::vector<double> list;

			if (numSubTasks <= 1) {
				list.push_back(fiberRandomFloat(0, 100));
			}
			else {
				fibers::For(0, numSubTasks, [&list, &numSubTasks](int index) {
					list.push_back(fiberRandomFloat(0, 100));
				});
			}

			vectors.push_back(std::move(list));
		});
	}

	{
		fibers::fiberMap<int, fibers::fiberMap<int, double>> lists;
		// set-up the list in-line
		for (int i = 0; i < numTasks; i++) {
			auto list = lists[i];
			for (int j = 0; j < numSubTasks; j++) {
				*list->operator[](j) = fiberRandomFloat(0, 100);
			}
		}

		fibers::For(0, numTasks, [&lists, &numSubTasks](int j) {
			auto list = lists[j];
			if (numSubTasks <= 1) {
				*list->operator[](0) = fiberRandomFloat(0, 100);
			}
			else {
				fibers::For(0, numSubTasks, [&list, &numSubTasks](int index) {
					*list->operator[](index) = fiberRandomFloat(0, 100);
				});
			}
		});
	}

	return 0;




	//std::vector<fibers::Job> jobs;
	//for (int i = 0; i < numTasks; ++i) {	
	//	jobs.push_back(fibers::Job([&lists, &numSubTasks, &fibers](int j) {
	//		auto list = (lists.GetExclusive()->operator[](j) = std::make_shared<Interlocked<std::unordered_map<int, double>>>());
	//		if (numSubTasks <= 1) {
	//			list->GetExclusive()->operator[](0) = fiberRandomFloat(0, 100);
	//		}
	//		else {
	//			std::vector<fibers::Job> jobs;
	//			for (int k = 0; k < numSubTasks; ++k) {
	//				jobs.push_back(fibers::Job([=](int index) { list->GetExclusive()->operator[](index) = fiberRandomFloat(0, 100); }, (int)k));
	//			}
	//			fibers::JobGroup awaiter;
	//			awaiter.Queue(jobs);
	//			awaiter.Wait();
	//		}
	//	}, int(i)));
	//}
	//
	//fibers::JobGroup awaiter;
	//awaiter.Queue(jobs);
	//awaiter.Wait();

	//int n = 0; 
	//for (auto& x : lists) {
	//	for (auto& y : x.second) {
	//		n++;
	//	}
	//}

	//return n;
};





