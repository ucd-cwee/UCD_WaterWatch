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

				// size_t submitting_thread;
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

#if 1
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

				GL::shared_ptr<job> do_dispatch() {
					if constexpr (!SkipDispatch) {
						impl::DispatchOnce(
							this->ctx,
							DoTaskPtr,
							reinterpret_cast<void*>(this)
						);
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
			GL::atomic_vector<GL::shared_ptr<job>> and_then_list;
			std::atomic<long> and_then_progress;

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
				and_then_list.push_back(next_job);
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
				while (true) {
					long this_index{ 0 };
					while (true) {
						long this_index = and_then_progress.load();
						if (and_then_list.size() > this_index) {
							if (and_then_progress.compare_exchange_strong(this_index, this_index + 1)) {
								break;
							}
						}
						else {
							return;
						}
					}

					auto& func = and_then_list[this_index];
					impl::DispatchOnce(
						func->ctx,
						func->DoTaskPtr,
						reinterpret_cast<void*>(func.get())
					);
				}
			};
		};
#else
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

				GL::shared_ptr<job> do_dispatch() {
					if constexpr (!SkipDispatch) {
						impl::DispatchOnce(
							this->ctx,
							DoTaskPtr,
							reinterpret_cast<void*>(this)
						);
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
		class job {
		public:
			job(void (*callback_ptr)(void*), void (*task_ptr)(impl::job_argument const&))
				: job_result{ nullptr }
			    , ctx{ 0, nullptr, callback_ptr, nullptr }
			    , DoTaskPtr{ task_ptr }
			    , and_then_list{} {};

		protected:
			GL::any job_result; // atomicly updated before the job is completed.
			impl::dispatch_context ctx; // indicates whether the job is completed or not. 
			void (*DoTaskPtr)(impl::job_argument const&);
			GL::atomic_parallel_queue<GL::shared_ptr<job>> and_then_list;
			GL::atomic_vector<GL::shared_ptr<job>> and_then_cache;

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
				and_then_list.push(next_job);
				if (try_wait()) DoAndThens();
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
				GL::shared_ptr<job> func;
				while (and_then_list.try_pop(func)) {
					impl::DispatchOnce(
						func->ctx,
						func->DoTaskPtr,
						reinterpret_cast<void*>(func.get())
					);
					and_then_cache.push_back(func);
				}
			};
		};


#endif

	};
};