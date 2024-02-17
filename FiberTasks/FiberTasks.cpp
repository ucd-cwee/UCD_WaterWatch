// FiberPool based on:
// http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
#pragma once
#include "../WaterWatchCpp/Precompiled.h"
#if 0

#include <optional>
#include <boost/fiber/all.hpp> 
namespace FiberPool
{

	inline auto
		no_of_defualt_threads()
	{
		return std::max(std::thread::hardware_concurrency(), 2u) - 1u;
	}

	/**
	 * A wrapper primarly for boost::fibers::buffered_channel
	 * The buffered_channel has non-virtual member functions
	 * thus cant inherit from it and use it polyformically.
	 * This makes it diffuclt to mock its behaviour in unit tests
	 * The wrapper solves this (see tests for example mock channel)
	 */
	template <typename BaseChannel>
	class TaskQueue
	{
	public:
		using value_type = typename BaseChannel::value_type;

		explicit TaskQueue(std::size_t capacity)
			: m_base_channel{ capacity }
		{}

		TaskQueue(const TaskQueue& rhs) = delete;
		TaskQueue& operator=(TaskQueue const& rhs) = delete;

		TaskQueue(TaskQueue&& other) = default;
		TaskQueue& operator=(TaskQueue&& other) = default;

		virtual ~TaskQueue() = default;

		boost::fibers::channel_op_status
			push(typename BaseChannel::value_type const& value)
		{
			return m_base_channel.push(value);
		}

		boost::fibers::channel_op_status
			push(typename BaseChannel::value_type&& value)
		{
			return m_base_channel.push(std::move(value));
		}

		boost::fibers::channel_op_status
			pop(typename BaseChannel::value_type& value)
		{
			return m_base_channel.pop(value);
		}

		void close() noexcept
		{
			m_base_channel.close();
		}

	private:
		BaseChannel m_base_channel;
	};

	/**
	 * All tasks executed by the FiberPool are
	 * automatically wrapped to use the
	 * following interface
	 */
	class IFiberTask
	{
	public:

		// how many running fibers there are
		inline static std::atomic<size_t> no_of_fibers{ 0 };

		IFiberTask(void) = default;

		virtual ~IFiberTask(void) = default;
		IFiberTask(const IFiberTask& rhs) = delete;
		IFiberTask& operator=(const IFiberTask& rhs) = delete;
		IFiberTask(IFiberTask&& other) = default;
		IFiberTask& operator=(IFiberTask&& other) = default;

		/**
		 * Run the task.
		 */
		virtual void execute() = 0;
	};


	template<
		bool use_work_steal = false,
		template<typename> typename task_queue_t
		= boost::fibers::buffered_channel,
		typename work_task_t = std::tuple<boost::fibers::launch,
		std::unique_ptr<IFiberTask>>
		>
		class FiberPool
	{
	private:

		/**
		 * A wrapper for packaged fiber task
		 */
		template <typename Func>
		class FiberTask : public IFiberTask
		{
		public:
			FiberTask(Func&& func)
				:m_func{ std::move(func) }
			{}

			~FiberTask(void) override = default;
			FiberTask(const FiberTask& rhs) = delete;
			FiberTask& operator=(const FiberTask& rhs) = delete;
			FiberTask(FiberTask&& other) = default;
			FiberTask& operator=(FiberTask&& other) = default;

			/**
			 * Run the task.
			 */
			void execute() override
			{
				++no_of_fibers;
				m_func();
				--no_of_fibers;
			}

		private:
			Func m_func;
		};

	public:

		static constexpr bool work_stealing
			= use_work_steal;

		FiberPool()
			: FiberPool{ no_of_defualt_threads() }
		{}

		FiberPool(
			size_t no_of_threads,
			size_t work_queue_size = 32)
			: m_threads_no{ no_of_threads },
			m_work_queue{ work_queue_size }
		{
			try
			{
				for (std::uint32_t i = 0; i < m_threads_no; ++i)
				{
					m_threads.emplace_back(
						&FiberPool::worker, this);
				}
			}
			catch (...)
			{
				close_queue();
				throw;
			}
		}

		/**
		 * Submit a task to be executed as fiber by worker threads
		 */
		template<typename Func, typename... Args>
		auto submit(boost::fibers::launch launch_policy,
			Func&& func, Args&&... args)
		{
			// capature our task into lambda with all its parameters
			auto capture = [func = std::forward<Func>(func),
				args = std::make_tuple(std::forward<Args>(args)...)]()
				mutable
			{
				// run the tesk with the parameters provided
				// this will be what our fibers execute
				return std::apply(std::move(func), std::move(args));
			};

			// get return type of our task
			using task_result_t = std::invoke_result_t<decltype(capture)>;

			// create fiber package_task
			using packaged_task_t
				= boost::fibers::packaged_task<task_result_t()>;

			packaged_task_t task{ std::move(capture) };

			using task_t = FiberTask<packaged_task_t>;

			// get future for obtaining future result when 
			// the fiber completes
			auto result_future = task.get_future();

			// finally submit the packaged task into work queue
			auto status = m_work_queue.push(
				std::make_tuple(launch_policy,
					std::make_unique<task_t>(
						std::move(task))));

			if (status != boost::fibers::channel_op_status::success)
			{
				return std::optional<std::decay_t<decltype(result_future)>> {};
			}

			// return the future to the caller so that 
			// we can get the result when the fiber with our task 
			// completes
			return std::make_optional(std::move(result_future));
		}


		/**
		 * Use boost::fibers:launch::post as
		 * default lanuch strategy for fibers
		 */
		template<typename Func, typename... Args>
		auto submit(Func&& func, Args&&... args)
		{
			return submit(boost::fibers::launch::post,
				std::forward<Func>(func),
				std::forward<Args>(args)...);
		}

		/**
		 * Non-copyable.
		 */
		FiberPool(FiberPool const& rhs) = delete;

		/**
		 * Non-assignable.
		 */
		FiberPool& operator=(FiberPool const& rhs) = delete;

		void close_queue() noexcept
		{
			m_work_queue.close();
		}

		auto threads_no() const noexcept
		{
			return m_threads.size();
		}

		auto fibers_no() const noexcept
		{
			return IFiberTask::no_of_fibers.load();
		}

		~FiberPool()
		{
			for (auto& thread : m_threads)
			{
				if (thread.joinable())
				{
					thread.join();
				}
			}
		}

	private:

		/**
		 * worker thread method. It participates with
		 * shared_work sheduler of fibers.
		 *
		 * It takes packaged taskes from the work_queue
		 * and launches fibers executing the tasks
		 */
		void worker()
		{
			// make this thread participate in shared_work 
			// fiber sharing
			//


			if constexpr (work_stealing)
			{
				// work_stealing sheduling is much faster
				// than work_shearing, but it does not 
				// allow for modifying number of threads
				// at runtime. Therefore if one uses
				// DefaultFiberPool, no other instance 
				// of the fiber pool can be created
				// as this would change the number of
				// worker threads
				boost::fibers::use_scheduling_algorithm<
					boost::fibers::algo::work_stealing>(
						m_threads_no, true);
			}
			else
			{
				// it is slower but, can vary number of 
				// worker threads at runtime. So you can
				// use DefaultFiberPool in one part of 
				// you application, and custom instance
				// of the fiber pool in other part. 
				boost::fibers::use_scheduling_algorithm<
					boost::fibers::algo::shared_work>(true);
			}

			// create a placeholder for packaged task for 
			// to-be-created fiber to execute
			auto task_tuple
				= typename decltype(m_work_queue)::value_type{};

			// fetch a packaged task from the work queue.
			// if there is nothing, we are just going to wait
			// here till we get some task
			while (boost::fibers::channel_op_status::success
				== m_work_queue.pop(task_tuple))
			{
				// creates a fiber from the pacakged task.
				//
				// the fiber is immedietly detached so that we
				// fetch next task from the queue without blocking
				// the thread and waiting here for the fiber to 
				// complete

				// the task is tuple with launch policy and
				// accutal packaged_task to run 
				auto& [launch_policy, task_to_run] = task_tuple;

				// earlier we already got future for the fiber
				// so we can get the result of our task if we want
				boost::fibers::fiber(launch_policy,
					[task = std::move(task_to_run)]()
				{
					// execute our task in the newly created
					// fiber
					task->execute();
				}).detach();
			}
		}

		size_t m_threads_no{ 1 };

		// worker threads. these are the threads which will 
		// be executing our fibers. Since we use work_shearing scheduling
		// algorithm, the fibers should be shared evenly
		// between these threads
		std::vector<std::thread> m_threads;

		// use buffered_channel (by default) so that we dont block when 
		// there is no  reciver for the fiber. we are only 
		// going to block when the buffered_channel is full. 
		// Otherwise, tasks will be just waiting in the 
		// queue till some fiber picks them up.
		TaskQueue<task_queue_t<work_task_t>> m_work_queue;
	};


	template<
		template<typename> typename task_queue_t
		= boost::fibers::buffered_channel,
		typename work_task_t = std::tuple<boost::fibers::launch,
		std::unique_ptr<IFiberTask>>
		>
		using FiberPoolStealing = FiberPool<true, task_queue_t, work_task_t>;


	template<
		template<typename> typename task_queue_t
		= boost::fibers::buffered_channel,
		typename work_task_t = std::tuple<boost::fibers::launch,
		std::unique_ptr<IFiberTask>>
		>
		using FiberPoolSharing = FiberPool<false, task_queue_t, work_task_t>;

}
/**
 * A static default FiberPool in which
 * number of threads is set automatically based
 * on your hardware
 */
namespace DefaultFiberPool
{

	inline auto&
		get_pool()
	{
		static FiberPool::FiberPool<true> default_fp{};
		return default_fp;
	};


	template <typename Func, typename... Args>
	inline auto
		submit_job(boost::fibers::launch launch_policy,
			Func&& func, Args&&... args)
	{
		return get_pool().submit(
			launch_policy,
			std::forward<Func>(func),
			std::forward<Args>(args)...);
	}

	template <typename Func, typename... Args>
	inline auto
		submit_job(Func&& func, Args&&... args)
	{
		return get_pool().submit(
			std::forward<Func>(func),
			std::forward<Args>(args)...);
	}

	inline void
		close()
	{
		get_pool().close_queue();
	}

}

#endif
#include "FiberTasks.h"
#if 0
#include "../WaterWatchCpp/cweeJob.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/Clock.h"

uint64_t FTL::fnBoostFiber() {
	// Define the constants to test
	constexpr uint64_t triangleNum = 47593243ULL;
	constexpr uint64_t numAdditionsPerTask = 10000ULL;
	constexpr uint64_t numTasks = (triangleNum + numAdditionsPerTask - 1ULL) / numAdditionsPerTask;

	// Create the tasks
	uint64_t nextNumber = 1ULL;

	cweeSharedPtr<uint64_t> total = make_cwee_shared<uint64_t>();
	*total = 0;

	cweeSharedPtr<boost::fibers::mutex> mut = make_cwee_shared<boost::fibers::mutex>();

	cweeList<boost::fibers::shared_future<cweeAny>> tasks;
	for (uint64_t i = 0ULL; i < numTasks; ++i) {
		cweeJob job([](uint64_t start, uint64_t const& end, uint64_t& total, boost::fibers::mutex& mut) {
			Stopwatch sw;
			sw.Start();

			uint64_t subtotal = 0;

			while (start != end) {
				subtotal += start;
				++start;
			}
			subtotal += end;

			mut.lock();
			total += subtotal;
			mut.unlock();

			sw.Stop();
			return sw.Seconds_Passed();
		}, (uint64_t)nextNumber, (uint64_t)(nextNumber + numAdditionsPerTask - 1ULL), total, mut);

		std::optional<boost::fibers::future<cweeAny>> task = DefaultFiberPool::submit_job([job]() {
			return cweeJob(job).Invoke();
		});

		tasks.Append(task->share());
	}

	for (auto& task : tasks) {
		task.wait();
	}

	return *total;
};




#else

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






#include <boost/any.hpp>
#include <map>
#include <memory>
#include <type_traits>
namespace {
	//template<class T> struct extract_type { using type = T; };
	//template<class T> struct extract_type<std::shared_ptr<T>> { using type = typename extract_type<T>::type; };
	//template<class T> struct extract_type<std::shared_ptr<T>&> { using type = typename extract_type<T>::type; };
	//template<class T> struct extract_type<std::shared_ptr<T>*> { using type = typename extract_type<T>::type; };
	//template<class T> struct extract_type<const std::shared_ptr<T>> { using type = typename extract_type<T>::type; };
	//template<class T> struct extract_type<const std::shared_ptr<T>&> { using type = typename extract_type<T>::type; };
	//template<class T> struct extract_type<const std::shared_ptr<T>*> { using type = typename extract_type<T>::type; };

	class AnyData {
	public:
		AnyData(std::shared_ptr<void> const& t_ptr, const boost::typeindex::type_info& t_type, bool t_const) noexcept :
			m_ptr(t_ptr), m_type(t_type), m_const(t_const) {};
		virtual ~AnyData() noexcept {};

	public:
		template<typename ToType> std::shared_ptr<ToType> cast() const {
			return std::static_pointer_cast<ToType>(m_ptr);
		};

	public:
		std::shared_ptr<void>					m_ptr;
		const boost::typeindex::type_info& m_type;
		const bool							m_const;

	};
	template<typename T> class AnyData_Impl final : public AnyData {
	public:
		AnyData_Impl() noexcept : AnyData(nullptr, boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
		AnyData_Impl(std::shared_ptr<T> const& d) noexcept : AnyData(AnyData_Impl<T>::get_data(d), boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
		AnyData_Impl(std::shared_ptr<T>&& d) noexcept : AnyData(AnyData_Impl<T>::get_data(std::forward<std::shared_ptr<T>>(d)), boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
		~AnyData_Impl() noexcept {};

		static std::shared_ptr<void> get_data(const std::shared_ptr<T>& data) {			
			if constexpr (std::is_const< T >::value) {
				return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(data));
			}
			else {
				return std::static_pointer_cast<void>(data);
			}			
		};
		static std::shared_ptr<void> get_data(std::shared_ptr<T>&& data) { 
			return std::static_pointer_cast<void>(std::forward<std::shared_ptr<T>>(data));
		};
	};
}
class AnyAutoCast; // forward
/*! Generic container that enables the containment and sharing of any data type to/from std::shared_ptrs */
class Any {
public:
	struct Object_Data {
		template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static AUTO get(const H<S>* obj) { return get(*obj); };
		template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static AUTO get(const H<S>& obj) {			
			return std::static_pointer_cast<AnyData>(std::shared_ptr<AnyData_Impl<S>>(new AnyData_Impl<S>(obj)));
		};
		template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static AUTO get(H<S>&& obj) {
			return std::static_pointer_cast<AnyData>(std::shared_ptr<AnyData_Impl<S>>(new AnyData_Impl<S>(std::forward<H<S>>(obj))));
		};
		template<typename T, typename = std::enable_if_t<!std::is_same_v<AnyAutoCast, T>>> static AUTO get(T* t) { std::shared_ptr<T> sp = std::make_shared<T>(t); return get(sp); };
		template<typename T, typename = std::enable_if_t<!std::is_same_v<AnyAutoCast, T>>> static AUTO get(const T* t) { return get(*t); };
		template<typename T, typename = std::enable_if_t<!std::is_same_v<AnyAutoCast, T>>> static AUTO get(const T& obj) { std::shared_ptr<T> sp = std::make_shared<T>(obj); return get(sp); };
		static AUTO get(const AnyAutoCast& obj);
		static AUTO get(const AnyAutoCast* t);
	};
	template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(const ValueType& r) { return Object_Data::get(r); };
	template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(ValueType&& r) { return Object_Data::get(std::forward<ValueType>(r)); };

public: /*! Init */
	constexpr Any() noexcept : container(nullptr) {};
	constexpr Any(std::nullptr_t) noexcept : container(nullptr) {};
	Any(const Any& rhs) noexcept : container(rhs.container) {};
	Any(Any&& rhs) noexcept : container(rhs.container) { rhs.container = nullptr; };

public: /*! Init w/ DATA ASSIGNMENT */
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType& value) noexcept : container(CreateContainer(value)) {};
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType* value) noexcept : container(CreateContainer(value)) {};
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType* value) noexcept : container(CreateContainer(value)) {};
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType&& value) noexcept : container(CreateContainer(std::forward<ValueType>(value))) {};

public: /*! Destroy */
	~Any() noexcept { container = nullptr; };

public: /*! Data Assignment AFTER INIT */
	Any& swap(Any& rhs) noexcept {
		if (this == &rhs) { return *this; }
		container.swap(rhs.container);
		return *this;
	};
	Any& operator=(const Any& rhs) noexcept {
		Any(rhs).swap(*this);
		return *this;
	};
	Any& operator=(Any&& rhs) noexcept {
		Any(std::forward<Any>(rhs)).swap(*this);
		return *this;
	};

	Any& operator=(std::nullptr_t) noexcept { Clear(); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(const ValueType& rhs) noexcept { CreateContainer(rhs).swap(container); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(const ValueType* rhs) noexcept { CreateContainer(rhs).swap(container); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(ValueType* rhs) noexcept { CreateContainer(rhs).swap(container); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(ValueType&& rhs) noexcept { CreateContainer(std::forward<ValueType>(rhs)).swap(container); return *this; };

public:
	/*! Checks if the Any has been assigned something */
	bool IsEmpty() const noexcept { return (bool)container; };

	/*! Empties the Any and frees the memory. */
	void Clear() noexcept { Any().swap(*this); };

	template <typename ValueT> static const char* TypeNameOf() { return TypeOf<ValueT>().name(); };
	template <typename ValueT> static const boost::typeindex::type_info& TypeOf() { return boost::typeindex::type_id<ValueT>().type_info(); };

	const char* TypeName() const noexcept { return Type().name(); };
	const boost::typeindex::type_info& Type() const noexcept {
		std::shared_ptr<AnyData> m = container;
		if (m) { return m->m_type; }
		else { return boost::typeindex::type_id<void>().type_info(); }
	};
	template<typename VType> bool IsTypeOf() const noexcept {
		decltype(auto) targetType = TypeOf<VType>();
		decltype(auto) thisType = Type();
		bool out = (thisType == targetType);
		return out;
	};

#pragma region Boolean Operators
public:
	explicit operator bool() const { return (bool)container; };
	friend bool operator==(const Any& a, const Any& b) noexcept { return a.container == b.container; };
	friend bool operator!=(const Any& a, const Any& b) noexcept { return a.container != b.container; };
	friend bool operator<(const Any& a, const Any& b) noexcept { return a.container < b.container; };
	friend bool operator<=(const Any& a, const Any& b) noexcept { return a.container <= b.container; };
	friend bool operator>(const Any& a, const Any& b) noexcept { return a.container > b.container; };
	friend bool operator>=(const Any& a, const Any& b) noexcept { return a.container >= b.container; };
	friend bool operator==(const Any& a, std::nullptr_t) noexcept { return a.container == nullptr; };
	friend bool operator!=(const Any& a, std::nullptr_t) noexcept { return a.container != nullptr; };
	friend bool operator<(const Any& a, std::nullptr_t) noexcept { return a.container < nullptr; };
	friend bool operator<=(const Any& a, std::nullptr_t) noexcept { return a.container <= nullptr; };
	friend bool operator>(const Any& a, std::nullptr_t) noexcept { return a.container > nullptr; };
	friend bool operator>=(const Any& a, std::nullptr_t) noexcept { return a.container >= nullptr; };
	friend bool operator==(std::nullptr_t, const Any& a) noexcept { return nullptr == a.container; };
	friend bool operator!=(std::nullptr_t, const Any& a) noexcept { return nullptr != a.container; };
	friend bool operator<(std::nullptr_t, const Any& a) noexcept { return nullptr < a.container; };
	friend bool operator<=(std::nullptr_t, const Any& a) noexcept { return nullptr <= a.container; };
	friend bool operator>(std::nullptr_t, const Any& a) noexcept { return nullptr > a.container; };
	friend bool operator>=(std::nullptr_t, const Any& a) noexcept { return nullptr >= a.container; };
#pragma endregion

public:
	class DataCaster {
	public:
		template<typename T> struct is_SharedPtr_class { using type = std::false_type; };
		template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>> { using type = std::true_type; };
		template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>&> { using type = std::true_type; };
		template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>*> { using type = std::true_type; };
		template<typename T> struct is_SharedPtr_class<const std::shared_ptr<T>> { using type = std::true_type; };
		template<typename T> struct is_SharedPtr_class<const std::shared_ptr<T>&> { using type = std::true_type; };
		template<typename T> struct is_SharedPtr_class<const std::shared_ptr<T>*> { using type = std::true_type; };
		template<typename T> struct is_SharedPtr_class<std::shared_ptr<T>&&> { using type = std::true_type; };

	private:
		template <class VType> static decltype(auto) DoCast_Shared(Any* p) noexcept {
			std::shared_ptr<AnyData> m = p->container;
			AUTO ptr = m.get();
			if (ptr) {
				return ptr->cast<VType>();
			}
			else {
				AUTO q = std::make_shared<VType>();
				p->container = Any::CreateContainer(q);
				return q;
			}
		};

		template <class VType> static decltype(auto) DoCast_Shared_Sentinel(Any* p) noexcept {
			throw("Casting Any to  cweeSharedPtr<T>* or  cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
			std::shared_ptr<VType>* finalOut = nullptr;

			return finalOut;
		};

		template<typename VType> static decltype(auto) DoCast_Unshared(Any* p) noexcept {
			constexpr bool is_ptr = std::is_pointer_v<VType>;

			typedef typename std::remove_reference<typename std::remove_pointer<VType>::type>::type desiredT;
			std::shared_ptr<AnyData> m = p->container;
			if (m) {
				std::shared_ptr<desiredT> ptr = m->cast<desiredT>();
				if constexpr (is_ptr) {
					return ptr.get();
				}
				else {
					return *ptr.get();
				}
			}
			else {
				std::shared_ptr<desiredT> q = std::make_shared<desiredT>();
				p->container = Any::CreateContainer(q);
				if constexpr (is_ptr) {
					return q.get();
				}
				else {
					return *q.get();
				}
			}
		};

	public:
		template<typename T> static decltype(auto) DoCast(Any* p) noexcept {
			typedef typename is_SharedPtr_class<T>::type isShared;
			constexpr bool is_cwee_shared_ptr = isShared::value;
			constexpr bool is_ptr = std::is_pointer_v<T>;
			constexpr bool is_ref = std::is_reference_v<T>;
			if constexpr (is_cwee_shared_ptr) {
				typedef typename extract_type<T>::type innertype;
				if constexpr (is_ptr) {
					throw("Casting Any to cweeSharedPtr<T>* or cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
				}
				else if constexpr (is_ref) {
					throw("Casting Any to cweeSharedPtr<T>* or cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
				}
				else {
					return DoCast_Shared<innertype>(p);
				}
			}
			else {
				return DoCast_Unshared<T>(p);
			}
		};
	};

	template<typename VType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
	AUTO cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<Any*>(this)); };

	template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
	Any& cast() const noexcept { return *const_cast<Any*>(this); };

	template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
	Any* cast() const noexcept { return const_cast<Any*>(this); };

	AnyAutoCast cast() const noexcept;

public:
	mutable std::shared_ptr<AnyData> container;
};
class AnyAutoCast {
public:
	AnyAutoCast(const Any* _parent) : parent(const_cast<Any*>(_parent)), parentCopy(*_parent) {};
	AnyAutoCast(AnyAutoCast&& other) : parent(std::move(other.parent)), parentCopy(std::move(other.parentCopy)) {};

	AnyAutoCast() = delete;
	AnyAutoCast(const AnyAutoCast&) = delete;
	AnyAutoCast& operator=(const AnyAutoCast&) = delete;
	AnyAutoCast& operator=(AnyAutoCast&&) = delete;
	~AnyAutoCast() {};

	explicit operator Any& () const noexcept { return *parent; };
	explicit operator Any* () const noexcept { return parent; };

	template <typename T> operator std::shared_ptr<T>() const noexcept {
		return parentCopy.cast<std::shared_ptr<T>>();
	};
	template <typename T> operator std::shared_ptr<T>* () const noexcept {
		return parentCopy.cast<std::shared_ptr<T>*>();
	};

	template< bool cond, typename U > using resolvedType = typename std::enable_if< cond, U >::type;
	template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
	operator ValueTypeT& () const noexcept {
		return *parentCopy.cast<ValueTypeT*>();
	};
	template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
	operator ValueTypeT* () const noexcept {
		return parentCopy.cast<ValueTypeT*>();
	};

	Any* parent;
	Any parentCopy; /* Allows for the following type of interaction:
						cweeStr foo() const noexcept {
							return cweeJob([](float R){ return R + 1; }, 100.0f).Invoke().cast(); // without the parentCopy, the Any may go out of scope before the AnyAutoCast and will caush an exception.
						}
						*/
};
INLINE AnyAutoCast Any::cast() const noexcept { return AnyAutoCast(this); };
INLINE AUTO Any::Object_Data::get(const AnyAutoCast& obj) { Any* t = const_cast<Any*>(obj.parent); std::shared_ptr<AnyData> out; if (t) { out = t->container; } return out; };
INLINE AUTO Any::Object_Data::get(const AnyAutoCast* t) { return get(*t); };




using anyType = cweeAny;
template <typename F = void()> class fiberFunction {
public:
	typedef F Type;
	typedef typename std::function<F>::result_type ResultType;
	typedef typename function_traits<std::function<F>>::arguments Arguments;

	fiberFunction() noexcept
		: _function()
		, _data()
		, Result()
		, IsFinished(false)
	{};

	template <typename... Args>
	fiberFunction(const std::function<F>& function, Args... Fargs) noexcept
		: _function(function)
		, _data(GetData(Fargs...))
		, Result()
		, IsFinished(false)
	{};

private:
	static void AddData(std::vector<anyType>& d) { return; };
	template<typename T, typename... Targs> static void AddData(std::vector<anyType>& d, const T& value, Targs... Fargs) // recursive function
	{
		if constexpr (std::is_same<T, void>::value) {
			AddData(d, Fargs...);
			return;
		}
		else {
			d.push_back(value);
			AddData(d, Fargs...);
			return;
		}
	};
	template <typename... Args> std::vector<anyType> GetData(Args... Fargs) {
		constexpr size_t NumNeededInputs = NumInputs();
		constexpr size_t NumProvidedInputs = sizeof...(Args);
		static_assert(NumNeededInputs <= NumProvidedInputs, "Providing fewer inputs than required is unsupported. C++ Lambdas cannot support default arguments and therefore all arguments must be provided for.");

		std::vector<anyType> out;
		AddData(out, Fargs...);
		return out;
	};

public:
	fiberFunction(const fiberFunction& copy) noexcept
		: _function(copy._function)
		, _data(copy._data)
		, Result(copy.Result)
		, IsFinished(copy.IsFinished.load())
	{};

	static fiberFunction Finished() {
		fiberFunction to_return;

		to_return.IsFinished.store(true);

		return to_return;
	};
	template <typename T> static fiberFunction Finished(const T& returnMe) {
		fiberFunction to_return;

		to_return.Result = returnMe;
		to_return.IsFinished.store(true);

		return to_return;
	};

	anyType& Invoke() {
		DoJob();
		return Result;
	};
	anyType& ForceInvoke() {
		ForceDoJob();
		return Result;
	};

	static constexpr size_t NumInputs() noexcept {
		constexpr size_t numArgs = count_arg<std::function<F>>::value;
		return numArgs;
	};
	static constexpr bool ReturnsNothing() {
		return  std::is_same<typename std::function<F>::result_type, void>::value;
	};

	const char* FunctionName() const {
		return _function.target_type().name();
	};

	anyType& GetResult() {
		return Result;
	};
	anyType& GetResult() const {
		return Result;
	};

private:
	void						DoJob() {
		static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a fiberFunction without further specialization.");

		if (!IsFinished.load()) {
			if constexpr (NumInputs() == 0) {
				DoJob_Internal_0();
			}
			else if constexpr (NumInputs() == 1) {
				DoJob_Internal_1();
			}
			else if constexpr (NumInputs() == 2) {
				DoJob_Internal_2();
			}
			else if constexpr (NumInputs() == 3) {
				DoJob_Internal_3();
			}
			else if constexpr (NumInputs() == 4) {
				DoJob_Internal_4();
			}
			else if constexpr (NumInputs() == 5) {
				DoJob_Internal_5();
			}
			else if constexpr (NumInputs() == 6) {
				DoJob_Internal_6();
			}
			else if constexpr (NumInputs() == 7) {
				DoJob_Internal_7();
			}
			else if constexpr (NumInputs() == 8) {
				DoJob_Internal_8();
			}
			else if constexpr (NumInputs() == 9) {
				DoJob_Internal_9();
			}
			else if constexpr (NumInputs() == 10) {
				DoJob_Internal_10();
			}
			else if constexpr (NumInputs() == 11) {
				DoJob_Internal_11();
			}
			else if constexpr (NumInputs() == 12) {
				DoJob_Internal_12();
			}
			else if constexpr (NumInputs() == 13) {
				DoJob_Internal_13();
			}
			else if constexpr (NumInputs() == 14) {
				DoJob_Internal_14();
			}
			else if constexpr (NumInputs() == 15) {
				DoJob_Internal_15();
			}
			else if constexpr (NumInputs() == 16) {
				DoJob_Internal_16();
			}

			SetAsFinished();
		}
	};
	void						ForceDoJob() {
		static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a fiberFunction without further specialization.");

		if (true) {
			if constexpr (NumInputs() == 0) {
				DoJob_Internal_0();
			}
			else if constexpr (NumInputs() == 1) {
				DoJob_Internal_1();
			}
			else if constexpr (NumInputs() == 2) {
				DoJob_Internal_2();
			}
			else if constexpr (NumInputs() == 3) {
				DoJob_Internal_3();
			}
			else if constexpr (NumInputs() == 4) {
				DoJob_Internal_4();
			}
			else if constexpr (NumInputs() == 5) {
				DoJob_Internal_5();
			}
			else if constexpr (NumInputs() == 6) {
				DoJob_Internal_6();
			}
			else if constexpr (NumInputs() == 7) {
				DoJob_Internal_7();
			}
			else if constexpr (NumInputs() == 8) {
				DoJob_Internal_8();
			}
			else if constexpr (NumInputs() == 9) {
				DoJob_Internal_9();
			}
			else if constexpr (NumInputs() == 10) {
				DoJob_Internal_10();
			}
			else if constexpr (NumInputs() == 11) {
				DoJob_Internal_11();
			}
			else if constexpr (NumInputs() == 12) {
				DoJob_Internal_12();
			}
			else if constexpr (NumInputs() == 13) {
				DoJob_Internal_13();
			}
			else if constexpr (NumInputs() == 14) {
				DoJob_Internal_14();
			}
			else if constexpr (NumInputs() == 15) {
				DoJob_Internal_15();
			}
			else if constexpr (NumInputs() == 16) {
				DoJob_Internal_16();
			}

			SetAsFinished();			
		}
	};

	void						DoJob_Internal_16() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
		}
	};
	void						DoJob_Internal_15() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
		}
	};
	void						DoJob_Internal_14() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
		}
	};
	void						DoJob_Internal_13() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
		}
	};
	void						DoJob_Internal_12() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
		}
	};
	void						DoJob_Internal_11() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
		}
	};
	void						DoJob_Internal_10() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
		}
	};
	void						DoJob_Internal_9() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
		}
	};
	void						DoJob_Internal_8() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
		}
	};
	void						DoJob_Internal_7() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
		}
	};
	void						DoJob_Internal_6() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
		}
	};
	void						DoJob_Internal_5() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
		}
	};
	void						DoJob_Internal_4() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
		}
	};
	void						DoJob_Internal_3() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
		}
	};
	void						DoJob_Internal_2() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast());
		}
	};
	void						DoJob_Internal_1() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast());
			anyType().swap(Result);
		}
		else {
			Result = _function(_data[0].cast());
		}
	};
	void						DoJob_Internal_0() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function();
			anyType().swap(Result);
		}
		else {
			Result = _function();
		}
	};

private:
	std::function<F>			_function;
	std::vector<anyType>		_data;

public:
	mutable anyType				Result;
	mutable std::atomic<bool>	IsFinished;

private:
	void						SetAsFinished() {
		IsFinished.store(true);
	};

};
class fiberAction_Interface {
public:
	explicit fiberAction_Interface() {};
	explicit fiberAction_Interface(fiberAction_Interface const&) = delete;
	explicit fiberAction_Interface(fiberAction_Interface&&) = delete;
	fiberAction_Interface& operator=(fiberAction_Interface const&) = delete;
	fiberAction_Interface& operator=(fiberAction_Interface&&) = delete;
	virtual ~fiberAction_Interface() noexcept {};

	virtual boost::typeindex::type_info const& type() const noexcept = 0;
	virtual const char* typeName() const noexcept = 0;
	virtual std::shared_ptr<fiberAction_Interface> clone() const noexcept = 0;
	virtual anyType& Invoke() noexcept = 0;
	virtual anyType& ForceInvoke() noexcept = 0;
	virtual const char* FunctionName() const noexcept = 0;
	virtual anyType& Result() const noexcept = 0;
	virtual bool IsFinished() const noexcept = 0;
};
template<typename ValueType> class fiberAction_Impl final : public fiberAction_Interface {
public:
	explicit fiberAction_Impl() = delete;
	explicit fiberAction_Impl(fiberFunction<ValueType> const& f) noexcept : data(f) {};
	explicit fiberAction_Impl(fiberFunction<ValueType>&& f) noexcept : data(std::forward<fiberFunction<ValueType>>(f)) {};
	virtual ~fiberAction_Impl() noexcept {};

	virtual boost::typeindex::type_info const& type() const noexcept final {
		return boost::typeindex::type_id<fiberFunction<ValueType>>().type_info();
	};
	virtual const char* typeName() const noexcept final {
		return boost::typeindex::type_id<fiberFunction<ValueType>>().type_info().name();
	};
	virtual std::shared_ptr<fiberAction_Interface> clone() const noexcept final {
		return std::static_pointer_cast<fiberAction_Interface>(std::make_shared<fiberAction_Impl<ValueType>>(data));
	};
	virtual anyType& Invoke() noexcept final {
		return data.Invoke();
	};
	virtual anyType& ForceInvoke() noexcept final {
		return data.ForceInvoke();
	};
	virtual const char* FunctionName() const noexcept final {
		return data.FunctionName();
	};
	virtual anyType& Result() const noexcept final {
		return data.GetResult();
	};
	virtual bool IsFinished() const noexcept final {
		return data.IsFinished.load();
	};

	fiberFunction<ValueType> data;
};
class fiberAction {
public: // structors
	/*! Init */ fiberAction() noexcept : content(BasePtr()) {};
	/*! Copy */ fiberAction(const fiberAction& other) noexcept : content(BasePtr()) { std::shared_ptr<fiberAction_Interface> c = other.content; content = c->clone(); };
	/*! Data Assignment */ template<typename ValueType> explicit fiberAction(const fiberFunction<ValueType>& value) : content(ToPtr<ValueType>(value)) {};
	~fiberAction() noexcept {};

public: // modifiers
	/*! Swap Data */ fiberAction& swap(fiberAction& rhs) noexcept {
		std::shared_ptr<fiberAction_Interface> c1 = this->content;
		std::shared_ptr<fiberAction_Interface> c2 = rhs.content;

		auto copy1 = c1->clone();
		auto copy2 = c2->clone();

		rhs.content = copy1;
		content = copy2;

		return *this;
	}
	/*! Copy Data */ fiberAction& operator=(const fiberAction& rhs) noexcept { fiberAction(rhs).swap(*this); return *this; };
	/* Perfect forwarding of ValueType */ template <class ValueType> fiberAction& operator=(const fiberFunction<ValueType>& rhs) noexcept { fiberAction(rhs).swap(*this); return *this; };

public: // queries
	explicit operator bool() { return !IsEmpty(); };
	explicit operator bool() const { return !IsEmpty(); };

	/*! Checks if the fiberAction has been assigned something */
	bool IsEmpty() const noexcept { AUTO c = content; return !c; };

	/*! Empties the fiberAction and frees the memory. */
	void Clear() noexcept { fiberAction().swap(*this); };

	template <typename ValueT> static constexpr const char* TypeNameOf() { return typenames::type_name<ValueT>(); };
	template <typename ValueT> static const boost::typeindex::type_info& TypeOf() { return boost::typeindex::type_id<ValueT>().type_info(); };

	const char* TypeName() const noexcept { std::shared_ptr<fiberAction_Interface> c = content; if (c) return c->typeName(); return typenames::type_name<void>(); };
	const boost::typeindex::type_info& Type() const noexcept { std::shared_ptr<fiberAction_Interface> c = content; return c ? c->type() : boost::typeindex::type_id<void>().type_info(); };

private:
	template <class ValueType> static std::shared_ptr<fiberAction_Interface> ToPtr(const fiberFunction<ValueType>& rhs) noexcept { return std::static_pointer_cast<fiberAction_Interface>(std::make_shared<fiberAction_Impl<ValueType>>(rhs)); };
	template <class ValueType> static std::shared_ptr<fiberAction_Interface> ToPtr(fiberFunction<ValueType>&& rhs) noexcept { return std::static_pointer_cast<fiberAction_Interface>(std::make_shared<fiberAction_Impl<ValueType>>(std::forward<fiberFunction<ValueType>>(rhs))); };
	static std::shared_ptr<fiberAction_Interface> BasePtr() noexcept { return ToPtr(fiberFunction<void()>()); };

public:
	std::shared_ptr<fiberAction_Interface> content;

public:
	anyType* Invoke() noexcept { std::shared_ptr<fiberAction_Interface> c = content; if (c) { return &c->Invoke(); } return nullptr; };
	anyType* ForceInvoke() noexcept { std::shared_ptr<fiberAction_Interface> c = content; if (c) { return &c->ForceInvoke(); } return nullptr; };
	const char* FunctionName() const {
		std::shared_ptr<fiberAction_Interface> c = content;
		if (c)
		{
			return c->FunctionName();
		}
		return "No Function";
	};
	bool     IsFinished() const {
		std::shared_ptr<fiberAction_Interface> c = content;
		if (c)
		{
			return c->IsFinished();
		}
		return false;
	};
	anyType* Result() const { std::shared_ptr<fiberAction_Interface> c = content; if (c) { return &c->Result(); } return nullptr; };
	static fiberAction Finished() { return fiberAction(fiberFunction<void()>::Finished()); };
	template <typename T> static fiberAction Finished(const T& returnMe) { return fiberAction(fiberFunction<T()>::Finished(returnMe)); };

};





struct FunctionStruct {
	fiberAction job;
};
static void DoFuncStruct(ftl::TaskScheduler* taskScheduler, void* arg) {
	std::shared_ptr<FunctionStruct> data(static_cast<FunctionStruct*>(arg));
	anyType* anyType = data->job.Invoke();
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

uint64_t FTL::fnFiberTasks2b(int numTasks) {
	ftl::TaskScheduler taskScheduler;
	taskScheduler.Init();

	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;

	ftl::WaitGroup wg(&taskScheduler);

	std::vector<ftl::Task> tasks;

	for (int i = 0; i < numTasks; ++i) {
		fiberAction todo;
		switch (cweeRandomInt(1, 4)) {
		default:
		case 1:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](double& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				// auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j + 100.0f;
				}), double(i)));
			break;
		case 2:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](float j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				// auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j + 100.0f;
				}), float(i)));
			break;
		case 3:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				// auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				j->ToUpper();
				return *j;
				}), cweeStr(i)));
			break;
		case 4:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](std::string const& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				// auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j + ": TEST";
				}), std::to_string(i)));
			break;
		}
		tasks.push_back({ DoFuncStruct, new FunctionStruct({ std::move(todo) }) });
	}

	taskScheduler.AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);

	wg.Wait();

	return maxParallel.GetValue();
};









extern DelayedInstantiation< ftl::TaskScheduler > Fibers = DelayedInstantiation< ftl::TaskScheduler >([]()-> ftl::TaskScheduler* {
	auto* p = new ftl::TaskScheduler();
	p->Init({ 400, 0, ftl::EmptyQueueBehavior::Sleep, {} });
	p->SetEmptyQueueBehavior(ftl::EmptyQueueBehavior::Sleep);
	return p;
	});
class cweeFiberMutex {
public:
	using Handle_t = ftl::Fibtex;
	using Phandle = std::shared_ptr<Handle_t>;

	class cweeSysMutexLifetimeGuard {
	public:
		explicit operator bool() const { return (bool)owner; };
		explicit cweeSysMutexLifetimeGuard(const Phandle& mut) noexcept : owner(mut) { owner->lock(); };
		~cweeSysMutexLifetimeGuard() noexcept { owner->unlock(); };
		explicit cweeSysMutexLifetimeGuard() = delete;
		explicit cweeSysMutexLifetimeGuard(const cweeSysMutexLifetimeGuard& other) = delete;
		explicit cweeSysMutexLifetimeGuard(cweeSysMutexLifetimeGuard&& other) = delete;
		cweeSysMutexLifetimeGuard& operator=(const cweeSysMutexLifetimeGuard&) = delete;
		cweeSysMutexLifetimeGuard& operator=(cweeSysMutexLifetimeGuard&&) = delete;
	protected:
		Phandle owner;
	};

public:
	cweeFiberMutex() : Handle(new Handle_t(&*Fibers)) {};
	cweeFiberMutex(const cweeFiberMutex& other) : Handle(new Handle_t(&*Fibers)) {};
	cweeFiberMutex(cweeFiberMutex&& other) : Handle(new Handle_t(&*Fibers)) {};
	cweeFiberMutex& operator=(const cweeFiberMutex& s) { return *this; };
	cweeFiberMutex& operator=(cweeFiberMutex&& s) { return *this; };
	~cweeFiberMutex() {};

	NODISCARD cweeSysMutexLifetimeGuard	Guard() const { return cweeSysMutexLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const { Handle->lock(); return true; };
	void			Unlock() const { Handle->unlock(); };

protected:
	Phandle Handle;

};







uint64_t FTL::fnFiberTasks2c(int numTasks) {
	ftl::TaskScheduler& taskScheduler = *Fibers;

	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;
	
	ftl::WaitGroup wg(&taskScheduler);

	std::vector<ftl::Task> tasks;

	for (int i = 0; i < numTasks; ++i) {
		fiberAction todo;
		switch (cweeRandomInt(1, 4)) {
		default:
		case 1:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](double& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j + 100.0f;
			}), double(i)));
			break;
		case 2:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](float j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j + 100.0f;
			}), float(i)));
			break;
		case 3:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				j->ToUpper();
				return *j;
			}), cweeStr(i)));
			break;
		case 4:
			todo = fiberAction(fiberFunction(std::function([&numParallel, &maxParallel](std::string const& j) -> std::remove_reference_t<decltype(j)> {
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) { /* wasted work */ }
				numParallel.Decrement();

				return j + ": TEST";
			}), std::to_string(i)));
			break;
		}
		tasks.push_back({ DoFuncStruct, new FunctionStruct({ std::move(todo) }) });
	}

	taskScheduler.AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);

	wg.Wait();

	return maxParallel.GetValue();
};







uint64_t FTL::fnFiberTasks2d(int numTasks) {
	ftl::TaskScheduler& taskScheduler = *Fibers;
	// taskScheduler.Init();
	cweeFiberMutex mutex; 

	struct FuncStruct {
		cweeJob job;
		ftl::WaitGroup* wg;
	};
	auto lambda = [](ftl::TaskScheduler* taskScheduler, void* arg) {
		std::shared_ptr<FuncStruct> data(static_cast<FuncStruct*>(arg));
		data->job.Invoke();
	};

	cweeSysInterlockedInteger numParallel = 0;
	cweeSysInterlockedInteger maxParallel = 0;

	ftl::WaitGroup wg(&taskScheduler);

	std::vector<ftl::Task> tasks;

	for (int i = 0; i < numTasks; ++i) {
		cweeJob todo;
		switch (cweeRandomInt(1, 4)) {
		default:
		case 1:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](double& j) -> std::remove_reference_t<decltype(j)> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();
				
				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return j;
			}, double(i));
			break;
		case 2:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](float j) -> std::remove_reference_t<decltype(j)> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return j;
			}, float(i));
			break;
		case 3:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](cweeStr* j) -> std::remove_pointer_t<std::remove_reference_t<decltype(j)>> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return *j;
			}, cweeStr(i));
			break;
		case 4:
			todo = cweeJob([&numParallel, &maxParallel, &mutex](std::string const& j) -> std::remove_reference_t<decltype(j)> {
				mutex.Lock();
				int n = numParallel.Increment();
				if (n > maxParallel.GetValue()) maxParallel.SetValue(n);
				mutex.Unlock();

				auto t = cweeTime::Now(); while ((cweeUnitValues::second)(cweeTime::Now() - t) <= cweeUnitValues::millisecond(5)) {}

				numParallel.Decrement();

				return j;
			}, std::to_string(i));
			break;
		}

		tasks.push_back({ lambda, new FuncStruct({ todo, &wg }) });
	}

	mutex.Lock();
	taskScheduler.AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
	mutex.Unlock();

	wg.Wait();

	return maxParallel.GetValue();
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
		mutable cweeFiberMutex mut;
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
template< typename Key, typename Value>
class fiberMap {
public:
	typedef Value						Type;
	typedef std::shared_ptr < Type >		PtrType;
	typedef std::pair<Key, PtrType>		_iterType;
	typedef long long					IdxType;

	class fiberMap_Impl {
	public:
		/*! Prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock -- A "Unlock" must be called to re-enable access to the list. */
		void			Lock(void) const { // assumes unlocked!
			lock.Lock();
		}
		/*! Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior. */
		void			Unlock(void) const { // assumes already locked! 
			lock.Unlock();
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
		cweeThreadedList<Key> UnsafeList(void) const {
			cweeThreadedList<Key> out;

			if (lastCreatedListVersion == CreatedListVersion) {
				out = indexList;

				return out;
			}
			int size = list.size();

			indexList.Clear();
			indexList.SetGranularity(size + 16);
			for (auto& kv : list) indexList.Append(kv.first);
			lastCreatedListVersion = CreatedListVersion;

			out = indexList;

			return out;
		};
		AUTO			UnsafeIndexForKey(Key index) const {
			return std::distance(list.begin(), list.find(index));
		};

		/* Clear the list */
		void Clear() {
			Lock();

			list_version = 0;
			CreatedListVersion = 0;

			for (auto& kv : list) {
				PtrType& p = const_cast<PtrType&>(kv.second); // need to access the underlying thing
				p = nullptr;
			}
			list.clear();

			indexList.Clear();
			lastSearchID = Key();
			lastResult = list.end();
			lastVersion = -1;
			lastCreatedListVersion = -1;

			Unlock();
		};

		fiberMap_Impl() : lock(), list(), indexList(), list_version(0), lastSearchID(), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1) {
			list.reserve(16);
			indexList.SetGranularity(16);
		};
		~fiberMap_Impl() {
			Clear();
		};

		/*! Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection */
		mutable cweeFiberMutex													    lock; // cweeSysMutex
		/* Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions. */
		mutable tsl::robin_map<Key, PtrType, robin_hood::hash<Key>, std::equal_to<Key>, std::allocator<_iterType>, true>	list;
		/* Optimized search parameters */
		mutable cweeThreadedList<Key>												indexList;
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
		mutable cweeThreadedList<Key>	listOfIndexes;
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
	operator cweeThreadedList<Key>() const {
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
		PtrType out{ nullptr };
		auto* p = impl.get();
		if (p) {
			Lock();
			if (p->list.count(index) > 0) {
				out = p->list.at(index);
			}
			Unlock();
		}
		return out;
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
			for (int i = 0; i < List.Num(); i++) {
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
			}
		}
		else {
			// Go through and perform swaps		
			for (int i = 0; i < List.Num(); i++) {
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
			}
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
	cweeThreadedList<Key> GetList(void) const {
		cweeThreadedList<Key> out;

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
		for (auto& kv : impl->list) {
			impl->indexList.Append(kv.first);
		}
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
	cweeThreadedList<Key> UnsafeList(void) const {
		return impl->UnsafeList();
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			//Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<_iterType> Select(std::function<bool(const Value*)> predicate) {
		cweeThreadedList<_iterType> out;
		PtrType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(_iterType(i, x));
			}
			//Unlock();
		}
		return out;
	};
	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<Key> SelectIndexes(std::function<bool(const Value*)> predicate) const {
		cweeThreadedList<Key> out;
		_iterType x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			//SharedLock();
			if (x && predicate(x.Ptr())) {
				out.Append(i);
			}
			//SharedUnlock();
		}
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
		Lock();
		result = UnsafeExtract(std::move(index), out);
		Unlock();
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
		Lock();
		result = UnsafeExtractIndex(std::move(idx), out);
		Unlock();
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
		impl->lock.Lock();
	};
	/*! Duplicate method to allow for advanced locking mechanisms. */
	void			SharedUnlock(void) const {
		impl->lock.Unlock();
	};

	ImplType impl;
};

extern void DoJob(ftl::TaskScheduler* taskScheduler, void* arg) {
	AUTO job = std::shared_ptr<fiberAction>(static_cast<fiberAction*>(arg));
	job->Invoke();
};
uint64_t FTL::fnFiberTasks3(int numTasks, int numSubTasks) {
	auto* fibers = &*Fibers; 
	std::shared_ptr<cweeFiberMutex> fibersLock = std::make_shared<cweeFiberMutex>();

	fiberMap<int, fiberMap <int, double>> lists;
	ftl::WaitGroup wg(fibers);

	std::vector<ftl::Task> tasks;
	for (int i = 0; i < numTasks; ++i) {	
		tasks.push_back({ DoJob, new fiberAction(fiberFunction(std::function([&lists, &numSubTasks, &fibers, fibersLock](int j) {
			AUTO list = lists[j];
			if (numSubTasks <= 1) {
				*list->operator[](0) = fiberRandomFloat(0, 100);
			}
			else {
				std::vector<ftl::Task> tasks;

				ftl::WaitGroup wg(fibers);
				for (int k = 0; k < numSubTasks; ++k) {
					tasks.push_back({ DoJob, new fiberAction(fiberFunction(std::function([list](int index) {
					  *list->operator[](index) = fiberRandomFloat(0,100);
				  }), int(k))) });
				}
				// fibersLock->Lock();
				fibers->AddTasks(numSubTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
				// fibersLock->Unlock();
				wg.Wait();
			}
		}), int(i))) });
	}
	//fibersLock->Lock();
	fibers->AddTasks(numTasks, &tasks[0], ftl::TaskPriority::Normal, &wg);
	//fibersLock->Unlock();

	wg.Wait();

	int n = 0; 
	for (auto& x : lists) {
		for (auto& y : *x.second) {
			n++;
		}
	}

	return n;
};



#endif

