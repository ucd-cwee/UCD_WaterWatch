// FiberPool based on:
// http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
#pragma once

#include "FiberTasks.h"

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

	namespace containers {
		/* Fiber- and thread-safe vector. Objects are stored and returned as std::shared_ptr. Growth, iterations, and push_back operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire vector. */
		template<typename _Ty>
		class vector {
		protected:
			using underlying = typename concurrency::concurrent_vector<std::shared_ptr<_Ty>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const vector* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const vector* ref) { ++pos; }
				inline void end(const vector* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(vector* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const vector* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const vector* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(vector, typename underlying::value_type, it_state);

			vector() : data(new underlying()) {};
			explicit vector(size_type _N) : data(new underlying(_N)) {};
			vector(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			vector(vector const& r) : data(new underlying()) {
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
			AUTO push_back(_Ty const& _Item) { return data->push_back(std::make_shared<_Ty>(_Item)); };
			AUTO push_back(_Ty&& _Item) { return data->push_back(std::make_shared<_Ty>(std::forward<_Ty>(_Item))); };
			AUTO push_back(std::shared_ptr<_Ty> const& _Item) { return data->push_back(_Item); };
			AUTO push_back(std::shared_ptr<_Ty>&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			std::shared_ptr<_Ty> operator[](size_type _Index) { return data->operator[](_Index); };
			std::shared_ptr<_Ty> operator[](size_type _Index) const { return data->operator[](_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) { return data->at(_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) const { return data->at(_Index); };
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

		/* Fiber- and thread-safe vector that cannot erase elements. Objects are stored as-is, and therefore the array must outlive the underlying objects. All operations are concurrent except for 'operator=(array)'.
		Higher performance is expected with the array than the vector, but the user needs to be slighly more careful with their timing to ensure the array remaings alive until access is complete. */
		template<typename _Ty>
		class array {
		protected:
			using underlying = typename concurrency::concurrent_vector<_Ty>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const array* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const array* ref) { ++pos; }
				inline void end(const array* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(array* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const array* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const array* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(array, typename underlying::value_type, it_state);

			array() : data(new underlying()) {};
			explicit array(size_type _N) : data(new underlying(_N)) {};
			array(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			array(array const& r) : data(new underlying()) {
				operator=(r);
			}
			array(array&& r) = default;
			array& operator=(array const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					array out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			array& operator=(array&& r) = default;
			~array() {};

			AUTO grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			AUTO grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			AUTO grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			AUTO push_back(_Ty const& _Item) { return data->push_back(_Item); };
			AUTO push_back(_Ty&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			AUTO operator[](size_type _Index) { return data->operator[](_Index); };
			AUTO operator[](size_type _Index) const { return data->operator[](_Index); };
			AUTO at(size_type _Index) { return data->at(_Index); };
			AUTO at(size_type _Index) const { return data->at(_Index); };
			AUTO size() const { return data->size(); };
			AUTO empty() const { return data->empty(); };
			AUTO capacity() const { return data->capacity(); };
			AUTO max_size() const { return data->max_size(); };
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
			fibers::containers::vector<Job> jobs;

			JobGroupImpl() : waitGroup(&*Fibers), jobs() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;
			~JobGroupImpl() {
				waitGroup.Wait(); // any jobs queued with this waiter will want to talk w/it when finished -- we must wait to ensure they go out of scope before we do.
			};
		};

	public:
		JobGroup() : impl(new JobGroupImpl()) {};
		JobGroup(Job const& job) : impl(new JobGroupImpl()) { Queue(job); };

		// The waiter should not be passed around. Ideally we want to follow Fiber job logic, e.g. splitting jobs quickly and 
		// then finishing them in the same job that started them, continuing like the split never happened.
		JobGroup(JobGroup const&) = delete; 
		JobGroup(JobGroup&&) = delete;
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() {};

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			Fibers->AddTask({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, false }) }, ftl::TaskPriority::Normal, &impl->waitGroup);
			impl->jobs.push_back(job);
			return *this;
		};		
		/* Queue job, and return tool to await the result */
		JobGroup& ForceQueue(Job const& job) {
			Fibers->AddTask({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, true }) }, ftl::TaskPriority::Normal, &impl->waitGroup);
			impl->jobs.push_back(job);
			return *this;
		};		
		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				tasks.push_back({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, false }) });
				impl->jobs.push_back(job);
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};
		JobGroup& Queue(fibers::containers::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				if (job) {
					tasks.push_back({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job->impl, nullptr, false }) });
					impl->jobs.push_back(job);
				}
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& ForceQueue(std::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				tasks.push_back({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job.impl, nullptr, true }) });
				impl->jobs.push_back(job);
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};
		JobGroup& ForceQueue(fibers::containers::vector<Job> const& listOfJobs) {
			std::vector<ftl::Task> tasks;
			for (auto& job : listOfJobs) {
				if (job) {
					tasks.push_back({ utilities::DoAnyFuncStruct, new utilities::AnyFunctionStruct({ job->impl, nullptr, true }) });
					impl->jobs.push_back(job);
				}
			}
			Fibers->AddTasks(tasks.size(), &tasks[0], ftl::TaskPriority::Normal, &impl->waitGroup);

			return *this;
		};

		/* Await all jobs in this group, and get the return values (which may be empty) for each job */
		std::vector<Any> Wait_Get() {
			impl->waitGroup.Wait();

			typename decltype(JobGroupImpl::jobs) out;
			out.swap(impl->jobs);

			if (out.size() > 0) {
				std::vector<Any> any(out.size());
				for (auto& job : out) {
					if (job) {
						any.push_back(job->GetResult());
					}
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
			impl->jobs.clear();
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

	namespace parallel {
		/* Queue all jobs and await all results */
		AUTO Do(std::vector<fibers::Job> const& jobs) {
			JobGroup group;
			group.Queue(jobs);
			return group.Wait_Get();
		};
		/* Queue all jobs and await all results */
		AUTO Do(fibers::containers::vector<fibers::Job> const& jobs) {
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

	namespace containers {
		/* Fiber- and thread-safe map / dictionary. Objects are stored and returned as std::shared_ptr. Growth, iterations, and insert/emplace operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire map. */
		template<typename _Key_type, typename _Element_type>
		class unordered_map {
		protected:
			using underlying = typename concurrency::concurrent_unordered_map<_Key_type, std::shared_ptr<_Element_type>>;
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
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) const {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) { return data->at(_Keyval); };
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) const { return data->at(_Keyval); };
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
				for (auto iter = begin(); iter != endIter; iter++) {
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
			AUTO emplace(const key_type& _Keyval, const std::shared_ptr<_Element_type>& _Value) { return insert(value_type(_Keyval, _Value)); };
			AUTO emplace(const key_type& _Keyval, std::shared_ptr<_Element_type>&& _Value) { return insert(value_type(_Keyval, std::forward<typename underlying::value_type>(_Value))); };
			AUTO emplace(const key_type& _Keyval, const _Element_type& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(_Value))); };
			AUTO emplace(const key_type& _Keyval, _Element_type&& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(std::forward<typename underlying::value_type>(_Value)))); };
			AUTO clear() {
				unordered_map out;
				data.swap(out.data);
			};
		};

		template<typename _Key_type> using unordered_set = concurrency::concurrent_unordered_set<_Key_type>; /* Wrapper To-Do */
		template<typename _Value_type> using queue = concurrency::concurrent_queue<_Value_type>; /* Wrapper To-Do */
	};
};
















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

uint64_t FTL::fnFiberTasks2d(int numTasks, int numSubTasks) {
// These are nearly equivalent implementations of the same optimization example. 
// The first inlines the functions, while the second uses static function pointers. 
#if 1 
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
#else
	std::shared_ptr<cweeSysInterlockedInteger> count = std::make_shared<cweeSysInterlockedInteger>();
	fibers::Job(ExampleOptimization::Solve, (int)numTasks, (int)numSubTasks, count).AsyncInvoke().Wait();
	return count->GetValue();
#endif
};
uint64_t FTL::fnFiberTasks2b(int numTasks) {
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
};
uint64_t FTL::fnFiberTasks2c(int numTasks) {
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

		fibers::containers::vector<fibers::Job> tasks;

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
uint64_t FTL::fnFiberTasks3(int numTasks, int numSubTasks) {
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
};





