#pragma region "Includes"
#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <thread>
#include <execution>
#include "util.h"
#include "../FiberTasks/Concurrent_Queue.h"
#include "thread_object.h"
#include "atomic_shared_ptr.h"
#include "atomic_queue.h"
#include "types.h"
#include <functional>
#include <tuple>
#include "ticket_dispensor.h"

#pragma endregion

// Good Language namespace
namespace GL {
	namespace parallel {
		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) Std_For(iteratorType start, iteratorType end, F const& ToDo) {
			GL::sequence<iteratorType> seq(start, end); // 0..999
			std::exception_ptr* e{ nullptr };

			std::for_each(
				std::execution::par,
				seq.begin(),
				seq.end(),
				[&](auto& x) { // copies are safer, and the resulting code will be as quick.
					try {
						if (!e) ToDo(x);
					}
					catch (...) {
						if (!e) {
							auto ptr = new std::exception_ptr(std::current_exception());
							if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
								delete ptr;
							}
						}
					}
				}
			);
			if (e) {
				std::exception_ptr copy{ *e };
				delete e;
				std::rethrow_exception(std::move(copy));
			}
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) Std_For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
			GL::sequence<iteratorType> seq(start, end, step); // 0..999
			std::exception_ptr* e{ nullptr };

			std::for_each(
				std::execution::par,
				seq.begin(),
				seq.end(),
				[&](auto& x) { // copies are safer, and the resulting code will be as quick.
					try {
						if (!e) ToDo(x);
					}
					catch (...) {
						if (!e) {
							auto ptr = new std::exception_ptr(std::current_exception());
							if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
								delete ptr;
							}
						}
					}
				}
			);
			if (e) {
				std::exception_ptr copy{ *e };
				delete e;
				std::rethrow_exception(std::move(copy));
			}
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) Std_ForEach(containerType& container, F const& ToDo) {
			std::exception_ptr* e{ nullptr };

			std::for_each(
				std::execution::par,
				std::begin(container),
				std::end(container),
				[&](auto& x) { // copies are safer, and the resulting code will be as quick.
					try {
						if (!e) ToDo(x);
					}
					catch (...) {
						if (!e) {
							auto ptr = new std::exception_ptr(std::current_exception());
							if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
								delete ptr;
							}
						}
					}
				}
			);
			if (e) {
				std::exception_ptr copy{ *e };
				delete e;
				std::rethrow_exception(std::move(copy));
			}
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) Std_ForEach(containerType const& container, F const& ToDo) {
			std::exception_ptr* e{ nullptr };

			std::for_each(
				std::execution::par,
				std::begin(container),
				std::end(container),
				[&](auto const& x) { // copies are safer, and the resulting code will be as quick.
					try {
						if (!e) ToDo(x);
					}
					catch (...) {
						if (!e) {
							auto ptr = new std::exception_ptr(std::current_exception());
							if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
								delete ptr;
							}
						}
					}
				}
			);
			if (e) {
				std::exception_ptr copy{ *e };
				delete e;
				std::rethrow_exception(std::move(copy));
			}
		};

		namespace impl {
			// each dispatch call will be "observed" by this context wrapper.
			struct dispatch_context {					
				std::atomic<size_t> counter; // how many Tasks* are awaited
				std::exception_ptr* e; // shared error PTR for re-throwing at the end of the Tasks.
				void (*callback)(void*);
				void* callback_data;

				bool is_busy() const {
					return counter > 0;
				};

				void try_rethrow_exception() {
					if (e == nullptr) return;
					if (std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(::InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&e), nullptr))) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				};
				void catch_exception() {
					if (!e) {
						if (std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(::InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&e), new std::exception_ptr(std::current_exception())))) {
							delete eptr;
						}
					}
				};
			};

			/* job arguments used to perform work as part of a loop over a Task */
			struct job_argument {
				long long 
					job_index;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
				long long 
					group_id;		// group index relative to dispatch (like SV_GroupID in HLSL)
				long long 
					group_index;	// job index relative to group (like SV_GroupIndex in HLSL)
				void* 
					group_memory;		// stack memory shared within the current group (jobs within a group execute serially)
				void*
					task_memory;
			};

			// to be looped over by a single thread, and used to create individual job_arguments for execution. 
			struct thread_task {
				dispatch_context* ctx;
				void (*task)(job_argument const&);
				size_t group_id;
				size_t group_job_offset;
				size_t group_job_end;
				size_t group_memory_size;
				void (*group_start_job)(void* const&); // callback func with memory for type T
				void (*group_end_job)(void* const&); // callback func with memory for type T
				void* task_memory;
			};

			void Dispatch(
				dispatch_context& ctx,
				size_t jobCount,
				void (*Task)(job_argument const&),
				void* task_memory,
				size_t sharedmemory_size,
				void (*group_start_job)(void* const&), // callback func with memory for type T
				void (*group_end_job)(void* const&) // callback func with memory for type T
			) noexcept;
			void Dispatch(
				dispatch_context& ctx,
				size_t jobCount,
				void (*Task)(job_argument const&),
				void* task_memory
			) noexcept;
			void DispatchOnce(
				dispatch_context& ctx,
				void (*Task)(job_argument const&),
				void* task_memory
			) noexcept;
			void Wait(dispatch_context& ctx);
		}



















		


		namespace impl {
			template<typename T> struct count_arg;
			template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
			template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
			template<typename R> struct function_traits {
				typedef R result_type;
				typedef std::tuple<> arguments;
			};
			template<typename R> struct function_traits<std::function<R(void)>> {
				typedef R result_type;
				typedef std::tuple<> arguments;
			};
			template<typename R, typename... Args> struct function_traits<std::function<R(Args...)>> {
				typedef R result_type;
				typedef std::tuple<Args...> arguments;
			};
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, F const& ToDo) {
			struct IterData {
				const F* _to_do;
				iteratorType _start;

				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					iteratorType t{ static_cast<iteratorType>(_args.job_index) + data->_start };
					(*data->_to_do)(t);
				};
			} data { &ToDo, start };

			impl::dispatch_context ctx{ 0, nullptr, nullptr };
			impl::Dispatch(ctx, static_cast<size_t>(end - start), &IterData::DoTask, reinterpret_cast<void*>(&data));
			impl::Wait(ctx);
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
			struct IterData {
				const F* _to_do;
				iteratorType _start;
				iteratorType _step;

				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					iteratorType t{ (static_cast<iteratorType>(_args.job_index) * data->_step) + data->_start };
					(*data->_to_do)(t);
				};
			} data{ &ToDo, start, step };

			impl::dispatch_context ctx{ 0, nullptr, nullptr };
			impl::Dispatch(ctx, static_cast<size_t>((end - start) / step), &IterData::DoTask, reinterpret_cast<void*>(&data));
			impl::Wait(ctx);
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) For_Each(containerType& container, F const& ToDo) {
			auto begin = container.begin();
			auto end = container.end();
			using iterType = decltype(begin);

			struct IterData {
				const F* _to_do;
				iterType _begin;

				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					iterType& iter = *static_cast<iterType*>(_args.group_memory);
					if (_args.group_index == 0) {
						// start of a group, so help it
						new (&iter) iterType{ data->_begin };						
						std::advance(iter, _args.job_index);
					}
					else {
						// within a group, we know the jobs are done in sequence, so we can safely increment by 1.
						std::advance(iter, 1);
					}
					// user-defined task
					(*data->_to_do)(*iter);
				};
				static void GroupStart(void* const&) {
					// do nothing
				};
				static void GroupEnd(void* const& p) {
					// delete
					((iterType*)p)->~iterType();
				};
			} data{ &ToDo, begin };

			impl::dispatch_context ctx{ 0, nullptr, nullptr };
			impl::Dispatch(
				ctx, 
				std::distance(begin, end),
				&IterData::DoTask, 
				reinterpret_cast<void*>(&data), 
				sizeof(iterType),
				&IterData::GroupStart,
				&IterData::GroupEnd
			);
			impl::Wait(ctx);
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) For_Each(containerType const& container, F const& ToDo) {
			auto begin = container.begin();
			auto end = container.end();
			using iterType = decltype(begin);

			struct IterData {
				const F* _to_do;
				iterType _begin;

				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					iterType& iter = *static_cast<iterType*>(_args.group_memory);
					if (_args.group_index == 0) {
						// start of a group, so help it
						new (&iter) iterType{ data->_begin };
						std::advance(iter, _args.job_index);
					}
					else {
						// within a group, we know the jobs are done in sequence, so we can safely increment by 1.
						std::advance(iter, 1);
					}
					// user-defined task
					(*data->_to_do)(*iter);
				};
				static void GroupStart(void* const&) {
					// do nothing
				};
				static void GroupEnd(void* const& p) {
					// delete
					((iterType*)p)->~iterType();
				};
			} data{ &ToDo, begin };

			impl::dispatch_context ctx{ 0, nullptr, nullptr };
			impl::Dispatch(
				ctx,
				std::distance(begin, end),
				&IterData::DoTask,
				reinterpret_cast<void*>(&data),
				sizeof(iterType),
				&IterData::GroupStart,
				&IterData::GroupEnd
			);
			impl::Wait(ctx);
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
				num_thread_jobs = num_thread_jobs << 2; // increase the number of parallel jobs, to reduce the down-time of waiting for jobs to collapse to 0.
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
				num_thread_jobs = num_thread_jobs << 2; // increase the number of parallel jobs, to reduce the down-time of waiting for jobs to collapse to 0.
			}
		};

		/* for (int i = 0; i < numToDispatch; i++){ ToDo(i, SharedObject); } return SharedObject; */
		template<typename F, typename G> decltype(auto) Dispatch(size_t numToDispatch, F&& SharedObject, G const& ToDo) {
			F out{ std::forward<F>(SharedObject) };
			struct IterData {
				const G* _to_do;
				F* _obj;

				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					(*data->_to_do)(_args.job_index, *data->_obj);
				};
			} data{ &ToDo, &out };
			impl::dispatch_context ctx{ 0, nullptr, nullptr };
			impl::Dispatch(
				ctx,
				numToDispatch,
				&IterData::DoTask,
				reinterpret_cast<void*>(&data)
			);
			impl::Wait(ctx);
			return out;
		};

		/* for (int i = 0; i < numToDispatch; i++){ ToDo(i, SharedObject); } return SharedObject; */
		template<typename F, typename G> decltype(auto) Dispatch(size_t numToDispatch, F& SharedObject, G const& ToDo) {
			struct IterData {
				const G* _to_do;
				F* _obj;

				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					(*data->_to_do)(_args.job_index, *data->_obj);
				};
			} data{ &ToDo, &SharedObject };
			impl::dispatch_context ctx{ 0, nullptr, nullptr };
			impl::Dispatch(
				ctx,
				numToDispatch,
				&IterData::DoTask,
				reinterpret_cast<void*>(&data)
			);
			impl::Wait(ctx);
			return SharedObject;
		};


#if 0		
		class job;
		template<typename Callable, bool SkipDispatch = false> GL::shared_ptr<job> async(Callable&& ToDo) {
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
			static constexpr bool returns_void = std::is_same_v<returnType, void>;

			class impl_job final : public job {
			public:
				impl_job(Callable&& ToDo) : job(&impl_job::Callback, &impl_job::DoTask), _to_do{ std::move(ToDo) } {};
				virtual ~impl_job() = default;

				Callable _to_do;

				static void DoTask(impl::job_argument const& _args) {
					impl_job* data = reinterpret_cast<impl_job*>(_args.task_memory);
					if constexpr (returns_void) {
						data->_to_do();
					}
					else {
						data->job_result = data->_to_do();
					}
				};
				static void Callback(void* _args) {
					impl_job* data = reinterpret_cast<impl_job*>(_args);
					data->callback();
				};
				void actual_dispatch() override {
					impl::DispatchOnce(
						this->ctx,
						DoTaskPtr,
						reinterpret_cast<void*>(this)
					);
				};
				GL::shared_ptr<job> do_dispatch() {
					if constexpr (!SkipDispatch) {
						this->actual_dispatch();
					}
					return GL::shared_ptr<job>(GL::shared_ptr< impl_job >(this));
				};

				void set_ctx_callback_data() {
					this->ctx.callback_data = this;
				};
			};

			impl_job* data = new impl_job(std::move(ToDo));
			data->set_ctx_callback_data();
			return data->do_dispatch();
		};
		template<typename iteratorType, typename Callable, bool SkipDispatch = false> GL::shared_ptr<job> async(iteratorType start, iteratorType end, Callable&& ToDo) {
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
			static constexpr bool returns_void = std::is_same_v<returnType, void>;

			class impl_job final : public job {
			public:
				impl_job(iteratorType start_p, iteratorType end_p, Callable&& ToDo) : job(&impl_job::Callback, &impl_job::DoTask), _to_do{ std::move(ToDo) }, start{ start_p }, end{ end_p } {};
				virtual ~impl_job() = default;

				Callable _to_do;
				iteratorType start;
				iteratorType end;

				static void DoTask(impl::job_argument const& _args) {
					impl_job* data = reinterpret_cast<impl_job*>(_args.task_memory);
					iteratorType t{ static_cast<iteratorType>(_args.job_index) + data->start };

					if constexpr (returns_void) {
						data->_to_do(t);
					}
					else {
						data->job_result = data->_to_do(t);
					}
				};
				static void Callback(void* _args) {
					impl_job* data = reinterpret_cast<impl_job*>(_args);
					data->callback();
				};

				void actual_dispatch() override {
					impl::Dispatch(
						this->ctx,
						static_cast<size_t>(end - start),
						DoTaskPtr,
						reinterpret_cast<void*>(this)
					);
				};
				GL::shared_ptr<job> do_dispatch() {
					if constexpr (!SkipDispatch) {
						this->actual_dispatch();
					}
					return GL::shared_ptr<job>(GL::shared_ptr< impl_job >(this));
				};

				void set_ctx_callback_data() {
					this->ctx.callback_data = this;
				};
			};

			impl_job* data = new impl_job(start, end, std::move(ToDo));
			data->set_ctx_callback_data();
			return data->do_dispatch();
		};
		class job {
		public:
			job(void (*callback_ptr)(void*), void (*task_ptr)(impl::job_argument const&))
				: job_result{ nullptr }
			    , ctx{ 0, nullptr, callback_ptr, nullptr }
			    , DoTaskPtr{ task_ptr }
			    , and_then_list{} 
				, and_then_progress{ 0 }
			{};

		protected:
			GL::any job_result; // atomicly updated before the job is completed.
			impl::dispatch_context ctx; // indicates whether the job is completed or not. 
			void (*DoTaskPtr)(impl::job_argument const&);
			GL::atomic_vector<std::pair<char, GL::shared_ptr<job>>> and_then_list;
			std::atomic<long> and_then_progress;

			virtual void actual_dispatch() = 0;

		public:
			// gets the result of the job. 
			GL::any const& result() const {
				return job_result;
			};
			virtual bool try_wait() const {
				return ctx.counter.load() == 0;
			};
			// waits for this job to complete. It does NOT wait for the downstream or and_then jobs. That will only happen when this job goes out-of-scope. 
			void wait() {
				impl::Wait(ctx);
				DoAndThens();
			};

			template<typename Callable>
			GL::shared_ptr<job> and_then(Callable&& to_do) {
				auto next_job = async<Callable, true>(std::move(to_do));
				and_then_list.push_back({ false, next_job });
				return next_job;
			};

			template<typename iteratorType, typename Callable>
			GL::shared_ptr<job> and_then(iteratorType start, iteratorType end, Callable&& to_do) {
				auto next_job = async<iteratorType, Callable, true>(start, end, std::move(to_do));
				and_then_list.push_back({ false, next_job });
				return next_job;
			};

			// this is called when the job is completed on the threading side
			void callback() {
				DoAndThens();
			};
			virtual ~job() {
				wait();
				DoAndThens();
			};

		protected:
			void DoAndThens() {
				if (and_then_progress.load() < and_then_list.size()) {
					auto iter = and_then_list.begin();
					iter += and_then_progress.load();

					for (; iter._ptr < and_then_list.size(); ++iter) {
						if (iter->first == 0) {
							if (InterlockedExchange8(reinterpret_cast<volatile char*>(&iter->first), 1) == 0) {
								iter->second->actual_dispatch();
								++and_then_progress;
							}
						}
					}
				}
			};
		};

#else
#if 1
		template<typename F, typename Parent>
		class job;

		template<typename F, typename Parent>
		class jobs;
	};
	
	// Type-erasure container that holds a fixed length of bytes. 
	// If the required amount of data needed is greater than can be stored locally, it will use a pointer. 
	// Allowed to set-and-forget, as it will use the type system to get the deleter for the object, albiet for a performance penalty. 
	template <size_t Size = 16 > class StaticContainer {
		union Unioned {
			unsigned char local[Size];
			void* global;
		};
		Unioned data;
		GL::type type;

		void do_delete() {
			if (type.size() > Size) {
				type.destroy(data.global);
				delete data.global;
			}
			else {
				type.destroy(reinterpret_cast<void*>(&data.local[0]));
			}
			type = GL::type_of<void>();	
		};

	public:
		StaticContainer()
			: data{ 0 }
			, type{ GL::type_of<void>() }
		{};
		StaticContainer(StaticContainer const&) = delete;
		StaticContainer(StaticContainer&& rhs) noexcept {
			std::memcpy(&data.local[0], &rhs.data.local[0], sizeof(Unioned));
			type = rhs.type;
			rhs.type = GL::type_of<void>();
		};
		StaticContainer& operator=(StaticContainer const&) = delete;
		StaticContainer& operator=(StaticContainer&& rhs) noexcept {
			if (!type.is_void()) do_delete();

			std::memcpy(&data.local[0], &rhs.data.local[0], sizeof(Unioned));
			type = rhs.type;
			rhs.type = GL::type_of<void>();

			return *this;
		};
		~StaticContainer() {
			if (!type.is_void()) do_delete();			
		};

		template <typename T, class... _Types> void New(_Types&&... _Args) {
			if (!type.is_void()) do_delete();

			type = GL::type_of<T>();
			if constexpr (sizeof(T) > Size) {
				data.global = new T(_STD forward<_Types>(_Args)...);
			}
			else {
				new (reinterpret_cast<T*>(&data.local[0])) T(_STD forward<_Types>(_Args)...);
			}
		};
		template <typename T> void Delete() {
			if constexpr (sizeof(T) > Size) {
				delete reinterpret_cast<T*>(data.global);
				data.global = nullptr;
			}
			else if constexpr (!std::is_pod_v<T>) 
				reinterpret_cast<T*>(&data.local[0])->~T();			

			type = GL::type_of<void>();
		};
		template <typename T> T& Get() {
			if constexpr (sizeof(T) > Size)
				return *reinterpret_cast<T*>(data.global);
			else
				return *reinterpret_cast<T*>(&data.local[0]);
		};
		template <typename T> const T& Get() const {
			if constexpr (sizeof(T) > Size)
				return *reinterpret_cast<const T*>(data.global);
			else
				return *reinterpret_cast<const T*>(&data.local[0]);
		};
	};

	class job_base {
	public:
		std::vector< std::weak_ptr<job_base> > children;

	public:
		job_base() = default;
		job_base(job_base const&) = delete;
		job_base(job_base &&) = default;
		job_base& operator=(job_base const&) = delete;
		job_base& operator=(job_base&&) = default;
		virtual ~job_base() = default;

		GL::any::fast_any result;		

		virtual void dispatch() = 0;
		virtual void wait() = 0;
		virtual job_base* parent_ptr() const = 0;

		template<typename F, typename Parent> friend class GL::parallel::job;
		template<typename F, typename Parent> friend class GL::parallel::jobs;
	};

	namespace parallel {
#if 1

		template<typename F, typename Parent>
		class job final : public job_base {
			friend class job_base;
			template<typename G, typename ParentB> friend class jobs;

		public:
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
			static constexpr size_t this_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
			static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

		public:
			std::weak_ptr<job> self;
		protected:
			std::atomic<bool> dispatch_once;
			impl::dispatch_context ctx;
			std::shared_ptr<Parent> parent;
			F todo;

			// called once the dispatched job ends
			static void Callback(void* _args) {
				job* data = reinterpret_cast<job*>(_args);
				std::this_thread::yield(); // helps with scheduling
				for (auto& x : data->children) if (auto p = x.lock()) p->dispatch();	
			};
			static void DoTask(impl::job_argument const& _args) {
				job* data = reinterpret_cast<job*>(_args.task_memory);

				if constexpr (!this_is_job_start)
					data->parent->wait();

				if constexpr (this_is_job_start || (this_num_args == 0)) {
					if constexpr (this_returns_void) data->todo();
					else data->result = GL::any(data->todo()).fast();
				}
				else {
					if constexpr (this_returns_void) data->todo(*dynamic_cast<job_base*>(&*data->parent));
					else data->result = GL::any(data->todo(*dynamic_cast<job_base*>(&*data->parent))).fast();
				}
			};

		public:
			job(F&& _todo, Parent* _parent)
				: job_base()
				, dispatch_once{ false }
				, ctx{ 0, nullptr, &job::Callback, reinterpret_cast<void*>(this) } 
				, parent{  }
				, todo{ std::move(_todo) }
			{ 
				if constexpr (!this_is_job_start) {
					parent = _parent->self.lock();
				}
			};
			job(job&&) = delete;
			job(job const&) = delete;
			job& operator=(job&&) = delete;
			job& operator=(job const&) = delete;
			~job() {
				wait();
			};

			void dispatch() override {
				static bool expected{ false };
				if (!dispatch_once.load()) {
					if (dispatch_once.compare_exchange_strong(expected, true)) {
						impl::DispatchOnce(ctx, &job::DoTask, reinterpret_cast<void*>(this));
					}
				}
			};
			void wait() override {
				dispatch();
				impl::Wait(ctx);
			};
			job_base* parent_ptr() const  override {
				if constexpr (this_is_job_start)
					return nullptr;
				else
					return dynamic_cast<job_base*>(const_cast<Parent*>(&*parent));
			};
			template<typename G> decltype(auto) and_then(G&& ToDo);
			template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo);
		};

		template<typename F, typename Parent>
		class jobs final : public job_base {
			friend class job_base;
			template<typename G, typename ParentB> friend class job;
		public:
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
			static constexpr size_t this_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
			static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

		public:
			std::weak_ptr<jobs> self;
		protected:		
			std::atomic<bool> dispatch_once;
			impl::dispatch_context ctx;
			std::shared_ptr<Parent> parent;
			size_t start;
			size_t end;
			F todo;

			// called once the dispatched job ends
			static void Callback(void* _args) {
				jobs* data = reinterpret_cast<jobs*>(_args);
				std::this_thread::yield(); // helps with scheduling
				for (auto& x : data->children) if (auto p = x.lock()) p->dispatch();
			};
			static void DoTask(impl::job_argument const& _args) {
				jobs* data = reinterpret_cast<jobs*>(_args.task_memory);

				if constexpr (!this_is_job_start)
					data->parent->wait();

				if constexpr (this_num_args == 0) {
					if constexpr (this_returns_void) (void)data->todo();
					else {
						if (_args.job_index == 0) {
							data->result = GL::any(data->todo()).fast();
						}
						else {
							(void)data->todo();
						}
					}
				}
				else {
					size_t t{ static_cast<size_t>(_args.job_index) + data->start };
					if constexpr (this_is_job_start || (this_num_args <= 1)) {
						if constexpr (this_returns_void) (void)data->todo(t);						
						else {
							if (_args.job_index == 0) {
								data->result = GL::any(data->todo(t)).fast();
							}
							else {
								(void)data->todo(t);
							}
						}
					}
					else {
						if constexpr (this_returns_void) (void)data->todo(t, *dynamic_cast<job_base*>(&*data->parent));						
						else {
							if (_args.job_index == 0){
							    data->result = GL::any(data->todo(t, *dynamic_cast<job_base*>(&*data->parent))).fast();
							}
							else {
								(void)data->todo(t, *dynamic_cast<job_base*>(&*data->parent));
							}
						}
					}
				}
			};

		public:
			jobs(size_t _start, size_t _end, F&& _todo, Parent* _parent)
				: job_base()
				, dispatch_once{ false }
				, start { _start }
				, end{ _end }
				, ctx{ 0, nullptr, &jobs::Callback, reinterpret_cast<void*>(this) }
				, parent{  }
				, todo{ std::move(_todo) }
			{
				if constexpr (!this_is_job_start) {
					parent = _parent->self.lock();
				}
			};
			jobs(jobs&&) = delete;
			jobs(jobs const&) = delete;
			jobs& operator=(jobs&&) = delete;
			jobs& operator=(jobs const&) = delete;
			~jobs() {
				wait();
			};

			void dispatch() override {
				static bool expected{ false };
				if (!dispatch_once.load()) {
					if (dispatch_once.compare_exchange_strong(expected, true)) {
						impl::Dispatch(ctx, end - start, &jobs::DoTask, reinterpret_cast<void*>(this));
					}
				}
			};
			void wait()  override {
				dispatch();
				impl::Wait(ctx);
			};
			job_base* parent_ptr() const override {
				if constexpr (this_is_job_start)
					return nullptr;
				else
					return dynamic_cast<job_base*>(const_cast<Parent*>(&*parent));
			};

			template<typename G> decltype(auto) and_then(G&& ToDo);
			template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo);
		};

		template<typename F, typename Parent = void> __forceinline decltype(auto) async(F&& ToDo, Parent* parent = nullptr) {
			auto out = std::make_shared<job<F, Parent>>(std::move(ToDo), std::move(parent));
			out->self = out;
			if constexpr (!std::is_same_v<void, Parent>) {
				parent->children.push_back(out);
			}
			return out;
		};
		template<typename F, typename Parent = void> __forceinline decltype(auto) async(size_t start, size_t end, F&& ToDo, Parent* parent = nullptr) {
			auto out = std::make_shared < jobs<F, Parent> >(start, end, std::move(ToDo), std::move(parent));
			out->self = out;
			if constexpr (!std::is_same_v<void, Parent>) {
				parent->children.push_back(out);
			}
			return out;
		};

		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) job<F, Parent>::and_then(G&& ToDo) {
			return async(std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) job<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return async(start, end, std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) jobs<F, Parent>::and_then(G&& ToDo) {
			return async(std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) jobs<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return async(start, end, std::move(ToDo), this);
		};

#else


        template<typename F, typename Parent> 
		class job final : public job_base {
		public:
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
			static constexpr size_t this_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
			static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

		protected:
			GL::any result;
			F todo;
			impl::dispatch_context ctx;
			Parent* parent;

			// called once the dispatched job ends
			static void Callback(void* _args) {
				job* data = reinterpret_cast<job*>(_args);
				
			};
			static void DoTask(impl::job_argument const& _args) {
				job* data = reinterpret_cast<job*>(_args.task_memory);

				if constexpr (!this_is_job_start) 
				    data->parent->wait();
				
				if constexpr (this_is_job_start || (this_num_args == 0)) {
					if constexpr (this_returns_void) data->todo();
					else data->result = data->todo();
				}
				else {
					if constexpr (this_returns_void) data->todo(*dynamic_cast<job_base*>(data->parent));
					else data->result = data->todo(*dynamic_cast<job_base*>(data->parent));
				}
			};

		public:
			job(F&& _todo, Parent* _parent)
				: result{}
				, todo(std::move(_todo))
				, ctx{ 0, nullptr, &job::Callback, reinterpret_cast<void*>(this) }
				, parent{ _parent }
			{
				impl::DispatchOnce(ctx, &job::DoTask, reinterpret_cast<void*>(this));
			};
			job(job&&) = default;
			job(job const&) = delete;
			job& operator=(job&&) = delete;
			job& operator=(job const&) = delete;
			~job() {
				wait();
			};

			void wait()  override {
				impl::Wait(ctx);
			};
			GL::any const& get()  override {
				return result;
			};
			job_base* parent_ptr() const  override {
				if constexpr (this_is_job_start)
					return nullptr;
				else
					return dynamic_cast<job_base*>(const_cast<Parent*>(parent));
			};
			template<typename G> decltype(auto) and_then(G&& ToDo);
			template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo);
		};
		
		template<typename F, typename Parent>
		class jobs final : public job_base {
		public:
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
			static constexpr size_t this_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
			static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

		protected:
			GL::any result;
			F todo;
			impl::dispatch_context ctx;
			Parent* parent;
			size_t start;
			size_t end;

			// called once the dispatched job ends
			static void Callback(void* _args) {
				jobs* data = reinterpret_cast<jobs*>(_args);

			};
			static void DoTask(impl::job_argument const& _args) {
				jobs* data = reinterpret_cast<jobs*>(_args.task_memory);
				
				if constexpr (!this_is_job_start)
					data->parent->wait();

				if constexpr (this_num_args == 0) {
					if constexpr (this_returns_void) (void)data->todo();
					else {
						if (data->result.empty())
							data->result = data->todo();
						else
							(void)data->todo();
					}
				}
				else {
					size_t t{ static_cast<size_t>(_args.job_index) + data->start };
					if constexpr (this_is_job_start || (this_num_args <= 1)) {
						if constexpr (this_returns_void) {
							(void)data->todo(t);
						}
						else {
							if (data->result.empty()) 
								data->result = data->todo(t);
							else
								(void)data->todo(t);
						}
					}
					else {
						if constexpr (this_returns_void) {
							(void)data->todo(t, *dynamic_cast<job_base*>(data->parent));
						}
						else {
							if (data->result.empty())
								data->result = data->todo(t, *dynamic_cast<job_base*>(data->parent));
							else
								(void)data->todo(t, *dynamic_cast<job_base*>(data->parent));
						}
					}
				}
			};

		public:
			jobs(size_t _start, size_t _end, F&& _todo, Parent* _parent)
				: result{}
				, start{ _start }
				, end{ _end }
				, todo(std::move(_todo))
				, ctx{ 0, nullptr, &jobs::Callback, reinterpret_cast<void*>(this) }
				, parent{ _parent }
			{
				impl::Dispatch(ctx, _end - _start, &jobs::DoTask, reinterpret_cast<void*>(this));
			};
			jobs(jobs&&) = default;
			jobs(jobs const&) = delete;
			jobs& operator=(jobs&&) = delete;
			jobs& operator=(jobs const&) = delete;
			~jobs() {
				wait();
			};

			void wait()  override {
				impl::Wait(ctx);
			};
			GL::any const& get()  override {
				return result;
			};
			job_base* parent_ptr() const override {
				if constexpr (this_is_job_start)
					return nullptr;
				else 
					return dynamic_cast<job_base*>(const_cast<Parent*>(parent));
			};

			template<typename G> decltype(auto) and_then(G&& ToDo);
			template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo);
		};

		template<typename F, typename Parent = void> __forceinline decltype(auto) async(F&& ToDo, Parent* parent = nullptr) {
			return job<F, Parent>(std::move(ToDo), std::move(parent));
		};
		template<typename F, typename Parent = void> __forceinline decltype(auto) async(size_t start, size_t end, F&& ToDo, Parent* parent = nullptr) {
			return jobs<F, Parent>(start, end, std::move(ToDo), std::move(parent));
		};

		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) job<F, Parent>::and_then(G&& ToDo) {
			return async(std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) job<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return async(start, end, std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) jobs<F, Parent>::and_then(G&& ToDo) {
			return async(std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) jobs<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return async(start, end, std::move(ToDo), this);
		};


#endif



#else
		class job;
		template<class F, bool delayed_dispatch = false, bool parent_returns = false> GL::shared_ptr<job> async(F&& ToDo) {
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool ReturnsVoid = std::is_same_v<returnType, void>;
			static constexpr size_t num_args_over_default = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;

			class IterData final : public job {
			public:
				IterData(F&& ToDo)
					: job()
					, dispatch_once{ 0 }
					, _to_do(std::move(ToDo))
					, ctx{ 0, nullptr, &IterData::Callback, reinterpret_cast<void*>(this) }
					, parent_result(nullptr)
				{
					returns_void = ReturnsVoid;
				};
				virtual ~IterData() {
					wait_all();
				}

				GL::any parent_result;
				size_t dispatch_once;
				F _to_do;
				impl::dispatch_context ctx;

				static void Callback(void* _args) {
					IterData* data = reinterpret_cast<IterData*>(_args);
					for (auto& x : data->and_then_list) {
						x->dispatch();
					}
				};
				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					if constexpr (parent_returns && (num_args_over_default > 0)) {
						if constexpr (ReturnsVoid) data->_to_do(data->parent_result.cast());
						else data->job_result = data->_to_do(data->parent_result.cast());
					}
					else if constexpr (num_args_over_default <= 0) {
						if constexpr (ReturnsVoid) data->_to_do();
						else data->job_result = data->_to_do();
					}
				};
				void dispatch() override {					
					if (dispatch_once == 0) {
						if (InterlockedExchange(reinterpret_cast<volatile size_t*>(&dispatch_once), 1) == 0) {
							if (this->parent) {
								this->parent->wait();
								if constexpr (parent_returns) {
									this->parent_result = this->parent->result();									
								}
								this->parent = nullptr;
							}
							impl::DispatchOnce(ctx, &IterData::DoTask, reinterpret_cast<void*>(this));
						}
					}
				};
				void wait() override {
					dispatch();
					impl::Wait(ctx);
				};
				void wait_all() override {
					wait();
					for (auto& x : and_then_list) {
						x->wait_all();
					}
				};
			};

			GL::shared_ptr<IterData> out(new IterData(std::move(ToDo)));
			if constexpr (!delayed_dispatch) {
				out->dispatch();
			}
			return GL::shared_ptr<job>(std::move(out));
		};
		template<class F, bool delayed_dispatch = false, bool parent_returns = false> GL::shared_ptr<job> async(size_t start, size_t end, F&& ToDo) {
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool ReturnsVoid = std::is_same_v<returnType, void>;
			static constexpr size_t num_args_over_default = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments> - 1;

			class IterData final : public job {
			public:
				IterData(F&& ToDo, size_t start, size_t end)
					: job()
					, dispatch_once{ 0 }
					, _to_do(std::move(ToDo))
					, _start{ start }
					, _end{ end }
					, ctx{ 0, nullptr, &IterData::Callback, reinterpret_cast<void*>(this) }
					, parent_result(nullptr)
				{
					returns_void = ReturnsVoid;
				};
				virtual ~IterData() {
					wait_all();
				}

				GL::any parent_result;
				size_t dispatch_once;
				F _to_do;
				size_t _start;
				size_t _end;
				impl::dispatch_context ctx;

				static void Callback(void* _args) {
					IterData* data = reinterpret_cast<IterData*>(_args);
					for (auto& x : data->and_then_list) {
						x->dispatch();
					}
				};
				static void DoTask(impl::job_argument const& _args) {
					IterData* data = reinterpret_cast<IterData*>(_args.task_memory);
					size_t t{ static_cast<size_t>(_args.job_index) + data->_start };

					if constexpr (parent_returns && (num_args_over_default > 0)) {
						if constexpr (ReturnsVoid) data->_to_do(t, data->parent_result.cast());
						else {
							data->job_result = data->_to_do(t, data->parent_result.cast());
						}
					}
					else if constexpr (num_args_over_default <= 0) {
						if constexpr (ReturnsVoid) data->_to_do(t);
						else {
							if (data->job_result.empty()) {
								data->job_result = data->_to_do(t);
							}
							else {
								data->_to_do(t);
							}							
						}
					}
				};
				void dispatch() override {	
					if (dispatch_once == 0) {
						if (InterlockedExchange(reinterpret_cast<volatile size_t*>(&dispatch_once), 1) == 0) {
							if (this->parent) {
								this->parent->wait();
								if constexpr (parent_returns) {
									this->parent_result = this->parent->result();									
								}
								this->parent = nullptr;
							}
							impl::Dispatch(ctx, static_cast<size_t>((size_t)_end - (size_t)_start), &IterData::DoTask, reinterpret_cast<void*>(this));							
						}
					}
				};
				void wait() override {
					dispatch();
					impl::Wait(ctx);
				};
				void wait_all() override {
					wait();
					for (auto& x : and_then_list) {
						x->wait_all();
					}
				};
			};

			GL::shared_ptr<IterData> out(new IterData(std::move(ToDo), start, end));
			if constexpr (!delayed_dispatch) {
				out->dispatch();
			}
			return GL::shared_ptr<job>(std::move(out));
		};
		// Atomic job that the user can use to schedule a series of jobs using the and_then sequence.
		class job {
		protected:
			GL::any job_result;
			GL::atomic_vector<GL::shared_ptr<job>> and_then_list;
			job* parent;			
			bool returns_void;

		public:
			job() 
				: parent(nullptr)
				, job_result()
				, and_then_list()
			    , returns_void{ false }
			{
				parent = nullptr;
			};

			template<typename Callable>
			GL::shared_ptr<job> and_then(Callable&& to_do) {
				if (this->returns_null()) {
					auto out = async<Callable, true, false>(std::move(to_do));
					out->parent = this;
					and_then_list.push_back(out);
					return out;
				}
				else {
					auto out = async<Callable, true, true>(std::move(to_do));
					out->parent = this;
					and_then_list.push_back(out);
					return out;
				}				
			};
			template<typename Callable>
			GL::shared_ptr<job> and_then(size_t start, size_t end, Callable&& to_do) {
				if (this->returns_null()) {
					auto out = async<Callable, true, false>(start, end, std::move(to_do));
					out->parent = this;
					and_then_list.push_back(out);
					return out;
				}
				else {
					auto out = async<Callable, true, true>(start, end, std::move(to_do));
					out->parent = this;
					and_then_list.push_back(out);
					return out;
				}
			};
			virtual void wait() = 0;
			virtual void wait_all() = 0;
			virtual void dispatch() = 0;

		public:
			bool returns_null() const {
				return returns_void;
			};
			GL::any const& result() const {
				return job_result;
			};
			virtual ~job() { };

		};
#endif



#endif


	};
};