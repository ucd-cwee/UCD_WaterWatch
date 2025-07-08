#pragma once

#include "Any.h"
#include "Proxy_Function.h"
#include "ThreadSafeContainers.h"
#include "Units_Base.h"
#include <deque>
#include <mutex>
#include <functional>
#include <ppl.h>
#include <execution>
#include <exception>

namespace GoodLang {
	namespace impl {
		bool Initialize(long long maxThreadCount = std::numeric_limits< long long>::max());
		void ShutDown();

		/* job arguments used to perform work as part of a loop over a Task */
		struct JobArgs {
			long long jobIndex;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
			long long groupID;		// group index relative to dispatch (like SV_GroupID in HLSL)
			long long groupIndex;	// job index relative to group (like SV_GroupIndex in HLSL)
			void* sharedmemory;		// stack memory shared within the current group (jobs within a group execute serially)
		};

		// Defines a state of execution, can be waited on
		class context {
		public:
			std::atomic<size_t> workers{ 0 }; // how many threads are working for this context to be finished
			std::atomic<size_t> waiters{ 0 }; // how many threads are waiting for this context to be finished
			std::atomic<size_t> counter{ 0 }; // how many Tasks* are awaited
			atomic_ptr<std::exception_ptr> e{ nullptr }; // shared error PTR for re-throwing at the end of the Tasks.

			context() = default;
			context(context const&) = delete;
			context(context&&) = delete;
			context& operator=(context const&) = delete;
			context& operator=(context&&) = delete;
			~context() = default;
		};

		/* job arguments used to perform work as part of a loop over a Task */
		struct Task {
			std::shared_ptr<std::function<void(JobArgs const&)>> task;
			context* ctx;
			long long groupID;
			long long groupJobOffset;
			long long groupJobEnd;
			long long sharedmemory_size;
			std::shared_ptr < std::function<void(void*)>> GroupStartJob; // callback func with memory for type T
			std::shared_ptr < std::function<void(void*)>> GroupEndJob; // callback func with memory for type T
			size_t SubmittingThread;
		};

		template <typename T> class Queue {
		public:
			moodycamel::ConcurrentQueue<T> queue;

			// std::deque<T> queue;
			// std::mutex locker;

			__forceinline void push(T&& item) {
				//std::scoped_lock lock(locker);
				//queue.push_back(std::forward<T>(item));
				
				queue.push(std::move(item));
			};
			__forceinline void push(const T& item) {
				//std::scoped_lock lock(locker);
				// queue.push_back(item);

				queue.push(item);
			};
			__forceinline bool try_pop(T& item) {
				//std::scoped_lock lock(locker);
				//if (queue.empty()) return false;
				//item = std::move(queue.front());
				//queue.pop_front();
				//return true;
				return queue.try_pop(item);
			};

			Queue() = default;
			Queue(Queue const&) = default;
			Queue(Queue&&) = default;
			Queue& operator=(Queue const&) = default;
			Queue& operator=(Queue&&) = default;
			~Queue() = default;
		};

		struct ThreadWrap {
			std::thread thread;
			size_t thread_hash;
			size_t thread_index;
		};

		struct InternalState {
			long long numCores = 0;
			long long numThreads = 0;

			std::unique_ptr<Queue<Task>[]> jobQueuePerThread;

			InterlockedLong alive{ 1 };
			std::condition_variable wakeCondition; 
			std::mutex wakeMutex; 
			InterlockedLong nextQueue{ 0 };
			std::vector<ThreadWrap> threads;

			void ShutDown() {
				alive = false; // indicate that new jobs cannot be started from this point
				bool wake_loop = true;
				std::thread waker([&] {
					while (wake_loop) wakeCondition.notify_all(); // wakes up sleeping worker threads
				});
				for (auto& thread : threads) {
					thread.thread.join();
				}
				wake_loop = false;
				waker.join();
				jobQueuePerThread.reset();
				threads.clear();
				numCores = 0;
				numThreads = 0;
			};
			~InternalState() {
				ShutDown();
			};
		} static internal_state;

		__forceinline long long GetThreadCount() { return internal_state.numThreads; };

		// Add a task to execute asynchronously. Any idle thread will execute this.
		void Execute(context& ctx, std::function<void(JobArgs const&)> task, size_t SubmittingThread = 0) noexcept;

		// Divide a task onto multiple jobs and execute in parallel.
		//	jobCount	: how many jobs to generate for this task.
		//	groupSize	: how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
		//	task		: receives a JobArgs as parameter
		void Dispatch(
			context& ctx,
			long long jobCount,
			std::function<void(JobArgs const&)> task
		) noexcept;

		void Dispatch(
			context& ctx,
			long long jobCount,
			std::function<void(JobArgs const&)> task,
			size_t sharedmemory_size,
			std::function<void(void*)> GroupStartJob, // callback func with memory for type T
			std::function<void(void*)> GroupEndJob // callback func with memory for type T
		) noexcept;

		// Returns the amount of job groups that will be created for a set number of jobs and group size
		__forceinline constexpr long long DispatchGroupCount(long long jobCount, long long groupSize) { return (jobCount + groupSize - 1) / groupSize; /* Calculate the amount of job groups to dispatch (overestimate, or "ceil"): */ };

		// Check if any threads are working currently or not
		bool IsBusy(const context& ctx);

		void HandleExceptions(context& ctx);

		// Wait until all threads become idle. Current thread will become a worker thread, executing jobs.
		void Wait(context& ctx);

		class TaskGroup {
		private:
			context ctx;

		public:
			TaskGroup() = default;
			TaskGroup(TaskGroup const&) = delete;
			TaskGroup(TaskGroup &&) = delete;
			TaskGroup& operator=(TaskGroup const&) = delete;
			TaskGroup& operator=(TaskGroup&&) = delete;
			~TaskGroup() = default;

			bool Waiting() const { 
				if (ctx.counter > 0) {
					return ctx.waiters > 0;
				}
				else {
					return false;
				}				
			};
			bool Working() const {
				if (ctx.counter > 0) {
					return ctx.workers > 0;
				}
				else {
					return false;
				}
			};
			auto Wait() { return impl::Wait(ctx); };
			auto IsBusy() const { return impl::IsBusy(ctx); };
			auto Queue(std::function<void(JobArgs const&)> task, size_t SubmittingThread = 0) {
				if (SubmittingThread == 0) SubmittingThread = std::hash<std::thread::id>{}(std::this_thread::get_id());
				return impl::Execute(ctx, std::move(task), SubmittingThread);
			};

			/* Dispatch a function that does not need to share memory within a group / cluster of the Task jobs. */
			auto Dispatch(
				long long jobCount,
				std::function<void(JobArgs const&)> task
			) {
				return impl::Dispatch(ctx, jobCount, std::move(task));
			};

			/* Dispatch a function that intends to share memory serially within a group / cluster of the Task jobs. */
			template <typename T> auto Dispatch(
				long long jobCount,
				std::function<void(JobArgs const&)> task,
				std::function<void(void*)> GroupStartJob, // callback func with memory for type T
				std::function<void(void*)> GroupEndJob // callback func with memory for type T
			) {
				return impl::Dispatch(ctx, jobCount, std::move(task), sizeof(T), std::move(GroupStartJob), std::move(GroupEndJob));
			};
		};
	};

	class JobGroup;

	/*! Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wrapper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	auto __awaiter__ = Job([](){ return std::string("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, and guarrantees its completion before the scope ends. */
	class Job {
		friend JobGroup;
	protected:
		Proxy_Function impl{
			nullptr
		};
		std::shared_ptr < std::vector<Any>> inputs{
			std::make_shared<std::vector<Any>>()
		};
		mutable std::shared_ptr<GoodLang::Lockable<std::shared_ptr<Any>>> result{
			std::make_shared<GoodLang::Lockable<std::shared_ptr<Any>>>()
		};
		size_t SubmittingThread{ 0 };

		static void AddItem(std::vector<Any>&) {};
		template<typename T, typename... Args> static void AddItem(std::vector<Any>& AddTo, T const& I, Args const&...  A) {
			AddTo.push_back(I);
			AddItem(AddTo, A...);
		};
		template<typename T, typename... Args> static void AddItem(std::vector<Any>& AddTo, T && I, Args &&...  A) {
			AddTo.push_back(std::forward<T>(I));
			AddItem(AddTo, std::forward<Args>(A)...);
		};

	private:
		// Scoped to return whetever the function desires / requires. Note: Can only be used in a "decltype" context, as it cannot actually do anything at all. 
		struct passepartout {
			template <typename T> operator T& ();
			template <typename T> operator T && ();
		};

	private:
		template <typename T> static constexpr const bool IsStaticFunction() {
			return (std::is_pointer<T>::value && std::is_function<typename std::remove_pointer_t<T>>::value) || std::is_function<T>::value;
		};
		template <typename T> static constexpr const bool IsLambda() {
			if constexpr (IsStaticFunction<T>() || std::is_member_function_pointer<T>::value) {
				return false;
			}
			else if constexpr (std::is_invocable<T>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else if constexpr (std::is_invocable<T, passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&,
				passepartout&, passepartout&, passepartout&, passepartout&>::value) {
				return true;
			}
			else return false;
		};
		template<typename T, typename Arguments> static constexpr const bool IsStatelessTest() {
			if constexpr (IsStaticFunction<T>()) { return true; }
			else {
				// using Arguments = typename GoodLang::utilities::function_traits<std::function<T(Args...)>>::arguments; // tuple

#define Ty(n) typename std::tuple_element<n, Arguments>::type
#define TIter0() Ty(0)
#define TIter1() Ty(0), Ty(1)
#define TIter2() Ty(0), Ty(1), Ty(2)
#define TIter3() Ty(0), Ty(1), Ty(2), Ty(3)
#define TIter4() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4)
#define TIter5() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5)
#define TIter6() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6)
#define TIter7() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7)
#define TIter8() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8)
#define TIter9() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9)
#define TIter10() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10)
#define TIter11() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11)
#define TIter12() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12)
#define TIter13() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12), Ty(13)
#define TIter14() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12), Ty(13), Ty(14)
#define TIter15() Ty(0), Ty(1), Ty(2), Ty(3), Ty(4), Ty(5), Ty(6), Ty(7), Ty(8), Ty(9), Ty(10), Ty(11), Ty(12), Ty(13), Ty(14), Ty(15)
				if constexpr (std::tuple_size_v< Arguments> > 16) return IsStaticFunction<T>();
				if constexpr (std::tuple_size_v< Arguments> == 0) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T>::type(*)()>::value;
				if constexpr (std::tuple_size_v< Arguments> == 1) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter0()>::type(*)(TIter0())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 2) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter1()>::type(*)(TIter1())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 3) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter2()>::type(*)(TIter2())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 4) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter3()>::type(*)(TIter3())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 5) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter4()>::type(*)(TIter4())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 6) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter5()>::type(*)(TIter5())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 7) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter6()>::type(*)(TIter6())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 8) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter7()>::type(*)(TIter7())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 9) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter8()>::type(*)(TIter8())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 10) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter9()>::type(*)(TIter9())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 11) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter10()>::type(*)(TIter10())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 12) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter11()>::type(*)(TIter11())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 13) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter12()>::type(*)(TIter12())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 14) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter13()>::type(*)(TIter13())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 15) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter14()>::type(*)(TIter14())>::value;
				if constexpr (std::tuple_size_v< Arguments> == 16) return IsStaticFunction<T>() || std::is_convertible<T, typename std::invoke_result<T, TIter15()>::type(*)(TIter15())>::value;
				return false;
#undef Ty
#undef TIter0
#undef TIter1
#undef TIter2
#undef TIter3
#undef TIter4
#undef TIter5
#undef TIter6
#undef TIter7
#undef TIter8
#undef TIter9
#undef TIter10
#undef TIter11
#undef TIter12
#undef TIter13
#undef TIter14
#undef TIter15
			}
		};
		template <typename T, typename Args> static constexpr const bool IsLambdaStateless() {
			return IsLambda<T>() && IsStatelessTest<T, Args>();
		};
		template<typename T, typename Args> static constexpr const bool IsStateless() {
			return IsStaticFunction<T>() || IsLambdaStateless<T, Args>();
		};

	public:
		Job() = default;
		Job(const Job& other) = default;
		Job(Job&& other) = default;
		Job& operator=(const Job& other) = default;
		Job& operator=(Job&& other) = default;
		~Job() = default;

		/* Creates a job from a function and (optionally) input parameters. Can handle basic type-casting from inputs to parameters, and supports shared_ptr casting (to and from). */
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<T>> && !std::is_same_v<Any, std::decay_t<T>> >> 
		explicit Job(T&& function, Args && ... Fargs) 
			: impl(nullptr) 
		{
			impl = make_callable(std::forward<T>(function));
			AddItem(*inputs, std::forward<Args>(Fargs)...);
			SubmittingThread = std::hash<std::thread::id>{}(std::this_thread::get_id());
		};		

	public:
		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		GoodLang::Any Invoke() const {
			static GoodLang::Any staticVal{};
			static GoodLang::TypeConverter converter{};
			if (impl) {
				auto locked = result->Read();
				if (*locked) return *locked;
				else return *locked = std::make_shared<Any>(call(impl, *inputs, converter));
			}
			else {
				return staticVal;
			}
		};
		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		GoodLang::Any Invoke(std::vector<Any> const& p_inputs) const {
			static GoodLang::Any staticVal{};
			static GoodLang::TypeConverter converter{};
			if (impl) {
				return call(impl, p_inputs, converter);
			}
			else {
				return staticVal;
			}
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. The awaiter group guarrantees job completion before the awaiter or job goes out-of-scope. Useful for most basic task scheduling. */
		[[nodiscard]] JobGroup AsyncInvoke();
		/* Add the task to a thread / fiber, and retrieve an awaiter group. The awaiter group guarrantees job completion before the awaiter or job goes out-of-scope. Useful for most basic task scheduling. */
		[[nodiscard]] JobGroup AsyncInvoke(std::vector<Any> const& p_inputs) const;

		/* Returns the result of the job, if any, if already performed. If not performed, the result will be empty. */
		GoodLang::Any GetResult() const {
			return Invoke();
		};

		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		GoodLang::Any operator()() const {
			return Invoke();
		};
		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		GoodLang::Any operator()(std::vector<Any> const& p_inputs) const {
			return Invoke(p_inputs);
		};

		std::weak_ptr<Type_Info> Type() const {
			if (impl) {
				return impl->Returns();
			}
			else {
				return {};
			}
		};
		size_t GetSubmittingThread() const {
			return SubmittingThread; 
		};
	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup {
		friend Job;
	private:
		class JobGroupImpl {
		public:
			std::shared_ptr<void> waitGroup;
			Job last_job;

			JobGroupImpl() : waitGroup(nullptr), last_job() {};
			JobGroupImpl(std::shared_ptr<void> wg) : waitGroup(wg), last_job() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;

			void Queue(Job const& job);
			void Queue(std::vector<Job> const& listOfJobs);
			void Wait();
			bool Waiting() const;
			bool Working() const;
			bool TryWait();
			~JobGroupImpl() { Wait(); };
		};

	public:
		JobGroup();
		JobGroup(Job const& job);

		// The waiter should not be passed around. Ideally we want to follow Fiber job logic, e.g. splitting jobs quickly and 
		// then finishing them in the same job that started them, continuing like the split never happened.
		JobGroup(JobGroup const&) = delete;
		JobGroup(JobGroup&& a) : impl(std::move(a.impl)) {};
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() = default;

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			impl->Queue(job);
			return *this;
		};

		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			impl->Queue(listOfJobs);
			return *this;
		};

		/* Await all jobs in this group, and gets the return value of the last job submitted */
		template <typename T = void>
		decltype(auto) Wait_Get() {
			impl->Wait();
			
			if constexpr (std::is_same<T, void>::value) {
				return impl->last_job.GetResult();
			}
			else {
				auto result = impl->last_job.GetResult();
				return (T)result.cast<T>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
		};
		/* Returns true if at least one thread is waiting for this task to finish. */
		bool Waiting() {
			return impl->Waiting();
		};
		bool Working() {
			return impl->Working();
		};
		/* Await all jobs in this group */
		bool TryWait() {
			return impl->TryWait();
		};

		impl::TaskGroup& GetTaskGroup() const {
			return *static_cast<impl::TaskGroup*>(impl->waitGroup.get());
		};

		Job GetLastJob() const {
			return impl->last_job;
		};

	protected:
		std::unique_ptr<JobGroupImpl> impl{};

	};

	namespace parallel {
		class options {
		public:
			static bool RethrowsExceptions(); // Get
			static void RethrowsExceptions(bool TF); // Set
		};

		// #define UseStdForEachForParallelManager // without, we are very stable (>1.5hr) and memory-leak free (so far). However, the competition from high parallelism causes memory usage overloads.

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, F const& ToDo) {
#ifdef UseStdForEachForParallelManager			
			fibers::utilities::Sequence seq(start, end); // 0..999
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(seq.begin(), seq.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
				});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}
#else
			impl::context ctx;
			impl::Dispatch(ctx, end - start, [&ToDo, start](impl::JobArgs const& _args)->void {
				iteratorType t{ static_cast<iteratorType>(_args.jobIndex) + start };
				ToDo(t);
			});
			impl::Wait(ctx);
#endif
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
#ifdef UseStdForEachForParallelManager			
			fibers::utilities::Sequence seq(start, end, step); // 0..999
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(seq.begin(), seq.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
				});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}
#else
			impl::context ctx;
			impl::Dispatch(ctx, (end - start) / step, [&ToDo, start, step](impl::JobArgs const& _args)->void {
				iteratorType t{ (static_cast<iteratorType>(_args.jobIndex) * step) + start };
				ToDo(t);
				});
			impl::Wait(ctx);
#endif

		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType& container, F const& ToDo) {
#ifdef UseStdForEachForParallelManager		
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(container.begin(), container.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
				});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}
#else
			auto begin = container.begin();
			auto end = container.end();
			using iterType = decltype(begin);
			impl::context ctx;
			impl::Dispatch(ctx,
				std::distance(begin, end),
				[&ToDo](impl::JobArgs const& _args)-> void {
					iterType& iter = *static_cast<iterType*>(_args.sharedmemory);
					if (_args.groupIndex == 0) {
						// start of a group, so help it
						std::advance(iter, _args.jobIndex);
					}
					else {
						// within a group, we know the jobs are done in sequence, so we can safely increment by 1.
						std::advance(iter, 1);
					}
					// user-defined task
					ToDo(*iter);
				},
				sizeof(iterType),
				[&begin](void* p) -> void {
					new (p) iterType{ begin };
				},
				[](void* p) -> void {
					((iterType*)p)->~iterType();
				}
			);
			impl::Wait(ctx);
#endif
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType const& container, F const& ToDo) {
#ifdef UseStdForEachForParallelManager		
			fibers::synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

			std::for_each(container.begin(), container.end(), [&](auto& x) {
				try {
					if (!e) ToDo(x);
				}
				catch (...) {
					if (!e) {
						if (auto* p = e.Set(new std::exception_ptr(std::current_exception()))) delete p;
					}
				}
				});
			if (auto* p = e.Set(nullptr)) {
				std::exception_ptr copy{ *p };
				delete p;
				std::rethrow_exception(std::move(copy));
			}
#else
			auto begin = container.begin();
			auto end = container.end();
			using iterType = decltype(begin);
			impl::context ctx;
			impl::Dispatch(ctx,
				std::distance(begin, end),
				[&ToDo](impl::JobArgs const& _args)-> void {
					iterType& iter = *((iterType*)_args.sharedmemory);
					if (_args.groupIndex == 0) {
						std::advance(iter, _args.jobIndex);
					}
					else {
						std::advance(iter, 1);
					}
					ToDo(*iter);
				},
				sizeof(iterType),
					[&begin](void* p)->void {
					new (p) iterType{ begin };
				},
					[](void* p)->void {
					((iterType*)p)->~iterType();
				}
				);
			impl::Wait(ctx);
#endif
		};

		/* wrapper for std::find_if.
		This is not parallelized, it is linear, which appears to be the fastest search for some reason under most cases. */
		template<typename containerType, typename F> decltype(auto) Find(containerType& container, F const& ToDo) {
			return std::find_if(container.begin(), container.end(), [&](auto& x) ->bool { return ToDo(x); });
		};

		/* wrapper for std::find_if.
		This is not parallelized, it is linear, which appears to be the fastest search for some reason under most cases. */
		template<typename containerType, typename F> decltype(auto) Find(containerType const& container, F const& ToDo) {
			return std::find_if(container.cbegin(), container.cend(), [&](auto const& x) ->bool { return ToDo(x); });
		};

		/* outputType x;
		for (auto& v : resultList){ x += v; }
		return x; */
		template<typename outputType> decltype(auto) Accumulate(std::vector<outputType> const& resultList) {
			return std::reduce(std::execution::par, resultList.begin(), resultList.end(), 0, [](outputType a, outputType b) { return a + b; });
		};

		/* Generic form of a future<T>, which can be used to wait on and get the results of any job. Can be safely shared if multiple places will need access to the result once available. */
		class promise {
		protected:
			std::shared_ptr< JobGroup > 
				shared_state;
			std::shared_ptr< SharedLockable<atomic_ptr<GoodLang::Any>> > 
				result;
			std::weak_ptr<Type_Info> 
				type;

		public:
			promise() : 
				shared_state(nullptr), 
				result(nullptr), 
				type() 
			{};
			promise(Job const& job, std::weak_ptr<Type_Info> const& Type) :
				shared_state(std::make_shared<JobGroup>(job)),
				result(std::shared_ptr<SharedLockable<atomic_ptr<GoodLang::Any>>>(new SharedLockable<atomic_ptr<GoodLang::Any>>(), [](SharedLockable<atomic_ptr<GoodLang::Any>>* anyP) { 
				    if (anyP) { 
						if (auto* p = anyP->Unique()->Set(nullptr)) delete p;
						delete anyP;
					}
				})),
				type(Type)
			{};
			promise(Job const& job) :
				shared_state(std::make_shared<JobGroup>(job)),
				result(std::shared_ptr<SharedLockable<atomic_ptr<GoodLang::Any>>>(new SharedLockable<atomic_ptr<GoodLang::Any>>(), [](SharedLockable<atomic_ptr<GoodLang::Any>>* anyP) {
				    if (anyP) {
					    if (auto* p = anyP->Unique()->Set(nullptr)) delete p;
					    delete anyP;
				    }
				})),
				type(job.Type())
			{};
			promise(promise const&) = default;
			promise(promise&&) = default;
			promise& operator=(promise const&) = default;
			promise& operator=(promise&&) = default;
			virtual ~promise() {};

			/* Returns true if this promise has been initialized correctly. Otherwise, false. */
			bool valid() const noexcept { return (bool)shared_state; };
			/* Wait until the requested job is completed. Repeated or simultaneous waiting is OK. */
			void wait() {
				if (shared_state && result) {
					if (auto s = result->Shared()) {
						if (auto p = s->load()) { // 
							return; // we are already done
						}
					}
					Any R = shared_state->Wait_Get(); // multiple threads are allowed to contribute
					if (auto s = result->Unique()) {
						if (auto p = s->load()) {
							return; // we are already done
						}
						else {
							auto* p2 = s->Set(new Any(R)); // should only happen once
							if (p2) delete p2;
						}
					}
				}
			};
			/* Returns true if at least one thread is waiting for this task to finish. */
			bool waiting() const {
				if (shared_state && result) {
					return shared_state->Waiting();
				}
				return false;
			};
			/* Returns true if the job is being worked on. */
			bool working() const {
				if (shared_state && result) {
					return shared_state->Working();
				}
				return false;
			};
			/* Returns true if the job is done, otherwise false. */
			bool try_wait() const noexcept {
				if (shared_state && result) {
					if (shared_state->TryWait()) {
						if (auto s = result->Shared()) {
							if (auto p = s->load()) { // 
								return true; // we are already done
							}
						}
						Any R = shared_state->Wait_Get(); // multiple threads are allowed to contribute
						if (auto s = result->Unique()) {
							if (auto p = s->load()) {
								return true; // we are already done
							}
							else {
								auto* p2 = s->Set(new Any(R)); // should only happen once
								if (p2) delete p2;
							}
						}	
						return true;
					}
					else {
						return false;
					}
				}
				return false;
			};
			/* Try to get the result, if available. Does not wait. */
			GoodLang::Any get_any() const noexcept {
				if (result) {
					if (auto s = result->Shared()) {
						if (auto p = s->load()) {
							return *p;
						}
					}
				}
				return GoodLang::Any();
			};
			/* Get the result, once available. Waits for the result, if necessary. */
			GoodLang::Any wait_get_any() {
				wait();
				return get_any();
			};

			std::weak_ptr<Type_Info> Type() const {
				return type;
			};
		};

		/* A secondary type tag used to identify if a template type is a future<T> type. */
	    class future_type { public: virtual ~future_type() {}; };

		/* Specialized form of a promise, which can be used to handle type-casting for lambdas automatically, while still being useful for waiting on and getting the results of any job. */
		template <typename T> class future final : public promise/*, public future_type*/ {
		private:
			static std::weak_ptr<Type_Info> ThisType() {
				typedef decltype(details::detail::function_signature(&future::get)) function_header;
				return user_type_shared<typename function_header::Return_Type>();
			};

		public:
			future() : promise()/*, future_type()*/ {};
			future(Job const& job) : promise(job, ThisType())/*, future_type()*/ {};
			future(promise const& p_promise) : promise(p_promise)/*, future_type()*/ {};
			future(future const&) = default;
			future(future&&) = default;
			future& operator=(future const&) = default;
			future& operator=(future&&) = default;
			virtual ~future() {};

			/* Cast-down to a generic promise that erases the information on the return type. Useful for sharing tasks between libraries where type info itself cannot be shared. */
			promise as_promise() const { return promise(reinterpret_cast<const promise&>(*this)); };

			/* get a copy of the result of the task. must have already waited. */
			decltype(auto) get() {
				if constexpr (std::is_same<void, T>()) {
					return;
				}
				else {
					return static_cast<T>(get_any().cast<T&>());
				}
			};
			/* get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. must have already waited. */
			decltype(auto) get_ref() {
				if constexpr (std::is_same<void, T>()) {
					return;
				}
				else {
					return static_cast<T&>(get_any().cast<T>());
				}
			};
			/* get a shared_pointer of the result of the task. must have already waited. */
			decltype(auto) get_shared() {
				if constexpr (std::is_same<void, T>()) {
					return;
				}
				else {
					return static_cast<std::shared_ptr<T>>(get_any().cast<std::shared_ptr<T>>());
				}
			};

			/* wait to get a copy of the result of the task. Repeated or simultaneous waiting is OK. */
			decltype(auto) wait_get() {
				wait();
				return get();
			};
			/* wait to get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. Repeated or simultaneous waiting is OK. */
			decltype(auto) wait_get_ref() {
				wait();
				return get_ref();
			};
			/* wait to get a shared_pointer of the result of the task. Repeated or simultaneous waiting is OK. */
			decltype(auto) wait_get_shared() {
				wait();
				return get_shared();
			};
		};

		/* returns a future<T> object for awaiting the results of the job. */
		template < typename F, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<F>> && !std::is_same_v<GoodLang::Any, std::decay_t<F>> >>
		__forceinline static decltype(auto) async(F function, Args... Fargs) {
			return future<typename GoodLang::utilities::function_traits<decltype(std::function(function))>::result_type>(Job(function, Fargs...));
		};

		/* while (WhileBoolean()) { Do(); } */
		template<typename F, typename G> decltype(auto) While(F const& WhileBoolean, G const& Do) {
			struct WhileException : public std::exception {};
			int num_thread_jobs = 1024; 
			while (WhileBoolean()) {
				try {
					For(0, num_thread_jobs, [&](int i) {
						if (WhileBoolean()) Do();						
						else throw WhileException{};						
					});
				}				
				catch (WhileException const&) {
					break;
				}
				num_thread_jobs *= 10; // increase the number of parallel jobs, to reduce the down-time of waiting for jobs to collapse to 0.
			}
		};

		/* while (true){ Do(); if (Until()){ return; } } */
		template<typename F, typename G> decltype(auto) Until(F const& Do, G const& UntilBoolean) {
			struct UntilException : public std::exception {};
			int num_thread_jobs = 1024;
			while (true) {
				try {
					For(0, num_thread_jobs, [&](int i) {
						Do();
						if (UntilBoolean()) throw UntilException{};						
					});
				}				
				catch (UntilException const&) {
					break;
				}
				if (UntilBoolean()) break;	
				num_thread_jobs *= 10; // increase the number of parallel jobs, to reduce the down-time of waiting for jobs to collapse to 0.
			}
		};

		/* auto lifetime_object = AsThread([](){ Loop Me While Lifetime Exists! }, [](){ Optionally Do Me When Lifetime Dies, As Clean-Up! }); */
		std::shared_ptr<void> AsThread(std::function<void(void)> Loop, std::function<void(void)> OnThreadEnd = {});

		/* for (int i = 0; i < numToDispatch; i++){ ToDo(i, SharedObject); } return SharedObject; */
		template<typename F, typename G> decltype(auto) Dispatch(size_t numToDispatch, F&& SharedObject, G const& ToDo) {
			F out{ std::forward<F>(SharedObject) };

			using sharedType = std::shared_ptr<F>;
			sharedType shared = std::shared_ptr<F>(&out, [](F*) { /* do nothing */ });

			impl::context ctx;
			impl::Dispatch(ctx,
				numToDispatch,
				[&ToDo](impl::JobArgs const& _args)-> void {
					sharedType& iter = *static_cast<sharedType*>(_args.sharedmemory);

					ToDo(_args.jobIndex, *iter); // user-defined task
				},
				sizeof(sharedType),
				[&shared](void* p) -> void {
					new (p) sharedType{ shared };
				},
				[](void* p) -> void {
					((sharedType*)p)->~sharedType();
				}
			);
			impl::Wait(ctx);

			return out;
		};

		/* for (int i = 0; i < numToDispatch; i++){ ToDo(i, SharedObject); } return SharedObject; */
		template<typename F, typename G> decltype(auto) Dispatch(size_t numToDispatch, F& SharedObject, G const& ToDo) {
			using sharedType = std::shared_ptr<F>;
			sharedType shared = std::shared_ptr<F>(&SharedObject, [](F*) { /* do nothing */ });
			impl::context ctx;
			impl::Dispatch(ctx,
				numToDispatch,
				[&ToDo](impl::JobArgs const& _args)-> void {
					sharedType& iter = *static_cast<sharedType*>(_args.sharedmemory);
					ToDo(_args.jobIndex, *iter); // user-defined task
				},
				sizeof(sharedType),
				[&shared](void* p) -> void {
					new (p) sharedType{ shared };
				},
				[](void* p) -> void {
					((sharedType*)p)->~sharedType();
				}
			);
			impl::Wait(ctx);
			return SharedObject;
		};

	};

	namespace Impl {
		template <typename... Args> __forceinline void ToString(Tag< parallel::future<Args...> >, parallel::future<Args...> const& r, std::string& out) {
			out = GoodLang::ToString(const_cast<parallel::future<Args...>&>(r).wait_get_any());
		};
		__forceinline void ToString(Tag< parallel::promise >, parallel::promise const& r, std::string& out) {
			out = GoodLang::ToString(const_cast<parallel::promise&>(r).wait_get_any());
		};

		template <typename... Args> __forceinline void GetChildren(Tag<parallel::future<Args...>>, parallel::future<Args...> const& r, std::vector< NodeCache >& out) {
			out.push_back(GoodLang::GetChildren(const_cast<parallel::future<Args...>&>(r).wait_get_any()));
		};
		__forceinline void GetChildren(Tag<parallel::promise>, parallel::promise const& r, std::vector< NodeCache >& out) {
			out.push_back(GoodLang::GetChildren(const_cast<parallel::promise&>(r).wait_get_any()));
		};
	};

	class MultithreadingInstanceManager {
	public:
		MultithreadingInstanceManager() {};
		virtual ~MultithreadingInstanceManager() {};
	};
	/* Instances the fiber system, and destroys it if the DLL / library is unloaded. */
	extern std::shared_ptr<MultithreadingInstanceManager> multithreadingInstance;

};
