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
#include "thread_object.h"
#include "atomic_shared_ptr.h"
#include "atomic_queue.h"
#include "types.h"
#include <functional>
#include <tuple>
#include "ticket_dispensor.h"
#include "atomic_vector.h"
#include <concurrent_vector.h>
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
							if (InterlockedCompareExchangePointerNoFence(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
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
							if (InterlockedCompareExchangePointerNoFence(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
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
							if (InterlockedCompareExchangePointerNoFence(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
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
							if (InterlockedCompareExchangePointerNoFence(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
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
					if (std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(::InterlockedExchangePointerNoFence(reinterpret_cast<volatile PVOID*>(&e), nullptr))) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				};
				void clear_exception() {
					if (e == nullptr) return;
					if (std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(::InterlockedExchangePointerNoFence(reinterpret_cast<volatile PVOID*>(&e), nullptr))) {
						delete eptr;
					}
				};
				void catch_exception() {
					if (!e) {
						if (std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(::InterlockedExchangePointerNoFence(reinterpret_cast<volatile PVOID*>(&e), new std::exception_ptr(std::current_exception())))) {
							delete eptr;
						}
					}
				};
			};

			/* job arguments used to perform work as part of a loop over a Task */
			struct job_argument {
				size_t
					job_index;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
				long long
					group_id;		// group index relative to dispatch (like SV_GroupID in HLSL)
				size_t
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
			void Wait(dispatch_context& ctx, bool rethrow = true);
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

		/* parallel_for (auto i = start; i < end; i++){ todo(i); } */
		template<typename iteratorType, class F> void For(iteratorType start, iteratorType end, F const& ToDo) {
			if (end >= start) {
				using f_t = impl::function_traits<decltype(std::function(ToDo))>;
				impl::dispatch_context ctx{ 0, nullptr, nullptr };
				if (start == static_cast<iteratorType>(0)) {
					struct IterData {
						const F& _to_do;
						const iteratorType& _start;

						static void DoTask(impl::job_argument const& _args) {
							const IterData& data = *static_cast<const IterData*>(_args.task_memory);
							if constexpr (std::tuple_size_v<f_t::arguments> == 0) {
								(void)data._to_do();
							}
							else if constexpr (std::is_reference_v<std::tuple_element_t<0, f_t::arguments> > && !std::is_const_v< std::remove_reference_t<std::tuple_element_t<0, f_t::arguments>>>) {
								iteratorType t = static_cast<iteratorType>(_args.job_index);
								(void)data._to_do(t);
							}
							else {
								(void)data._to_do(static_cast<iteratorType>(_args.job_index));
							}
						};
					} data{ ToDo, start };
					impl::Dispatch(ctx, static_cast<size_t>(end - start), &IterData::DoTask, static_cast<void*>(&data));
				}
				else {
					struct IterData {
						const F& _to_do;
						const iteratorType& _start;

						static void DoTask(impl::job_argument const& _args) {
							const IterData& data = *static_cast<const IterData*>(_args.task_memory);
							if constexpr (std::tuple_size_v<f_t::arguments> == 0) {
								(void)data._to_do();
							}
							else if constexpr (std::is_reference_v<std::tuple_element_t<0, f_t::arguments> > && !std::is_const_v< std::remove_reference_t<std::tuple_element_t<0, f_t::arguments>>>) {
								iteratorType t = static_cast<iteratorType>(_args.job_index) + data._start;
								(void)data._to_do(t);
							}
							else {
								(void)data._to_do(static_cast<iteratorType>(_args.job_index) + data._start);
							}
						};
					} data{ ToDo, start };
					impl::Dispatch(ctx, static_cast<size_t>(end - start), &IterData::DoTask, static_cast<void*>(&data));
				}
				impl::Wait(ctx);
			}
			else {
				For(end, start, ToDo);
			}
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> void For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
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
			impl::Dispatch(ctx, static_cast<size_t>((static_cast<size_t>(end) - static_cast<size_t>(start)) / static_cast<size_t>(step)), &IterData::DoTask, reinterpret_cast<void*>(&data));
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
		template<typename F, typename G> F Dispatch(size_t numToDispatch, F&& SharedObject, G const& ToDo) {
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
		template<typename F, typename G> F& Dispatch(size_t numToDispatch, F& SharedObject, G const& ToDo) {
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
			template<typename F, typename Parent> class job;
			template<typename F, typename Parent> class jobs;
		}
	};

	class job_base {
	public:
		job_base() = default;
		job_base(job_base const&) = delete;
		job_base(job_base &&) = default;
		job_base& operator=(job_base const&) = delete;
		job_base& operator=(job_base&&) = default;
		virtual ~job_base() = default;

		GL::any::fast_any result;		

		// Returns true if the job was dispatched. Returns false if the job had already been dispatched previously. 
		virtual bool dispatch() = 0;
		// Will wait until job is and may help with parallel computing. If an exception is caught during the async operation, it will be re-thrown at this time. 
		virtual job_base& wait() = 0;
		// returns a pointer to the parent of this job, if one exists. Can be used to get the results of past jobs. 
		virtual job_base* parent_ptr() const = 0;

		template<typename F, typename Parent> friend class GL::parallel::impl::job;
		template<typename F, typename Parent> friend class GL::parallel::impl::jobs;
	};

	namespace parallel {
		namespace impl {
			template<typename F, typename Parent>
			class job final : public job_base {
				friend class job_base;
				template<typename G, typename ParentB> friend class job;
				template<typename G, typename ParentB> friend class jobs;
			public:
				using functionType = impl::function_traits<decltype(std::function(std::declval<F>()))>;
				using returnType = typename functionType::result_type;
				static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
				static constexpr size_t this_num_args = std::tuple_size_v<typename functionType::arguments>;
				static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

			public:
				std::weak_ptr<job> self;
				concurrency::concurrent_vector< std::weak_ptr<job_base> > children;

			protected:
				std::atomic<bool> dispatch_once;
				impl::dispatch_context ctx;
				std::shared_ptr<Parent> parent;
				F todo;

				// called once the dispatched job ends
				static void Callback(void* _args) {
					if (auto* data = reinterpret_cast<job*>(_args); data != nullptr)
						for (auto& x : data->children)
							if (auto p = x.lock(); p != nullptr)
								(void)p->dispatch();
				};

				template <size_t i>
				static decltype(auto) Argument(impl::job_argument const& _args) {
					if constexpr (this_num_args > i) {
						using arg = typename std::tuple_element_t<i, typename functionType::arguments>;
						static constexpr bool arg_is_jobBase = std::is_same_v<std::remove_const_t <arg>, job_base&>;
						static constexpr bool arg_is_any = std::is_same_v<std::decay_t<arg>, GL::any>;
						static constexpr bool arg_is_fast_any = std::is_same_v<std::decay_t <arg>, GL::any::fast_any>;

						job* data = reinterpret_cast<job*>(_args.task_memory);
						if constexpr (arg_is_jobBase) {
							return *dynamic_cast<job_base*>(data->parent.get());
						}
						else if constexpr (arg_is_any || arg_is_fast_any) {
							return data->parent->result;
						}
						else if constexpr (!this_is_job_start) {
							return data->parent->result.cast<typename Parent::returnType>();
						}
						else {
							return;
						}
					}
					else {
						return;
					}
				};

				static void DoTask(impl::job_argument const& _args) {
					job* data = reinterpret_cast<job*>(_args.task_memory);

					if constexpr (this_is_job_start || (this_num_args == 0)) {
						if constexpr (this_returns_void) data->todo();
						else data->result = GL::any::fast_any::instance(data->todo());
					}
					else {
						if constexpr (this_num_args == 1) {
							if constexpr (this_returns_void) data->todo(Argument<0>(_args));
							else data->result = GL::any::fast_any::instance(data->todo(Argument<0>(_args)));
						}
						else if constexpr (this_num_args == 2) {
							if constexpr (this_returns_void) data->todo(Argument<0>(_args), Argument<1>(_args));
							else data->result = GL::any::fast_any::instance(data->todo(Argument<0>(_args), Argument<1>(_args)));
						}
						else {
							static_assert("Not able to process async 'job' task with more than 2 arguments. One argument may the a jobBase&, and the other argument may recieve the return of the parent job.");
						}
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
					dispatch();
					impl::Wait(ctx, false);
				};

				bool dispatch() override {
					static bool expected{ false };
					if (!dispatch_once.load()) {
						if (dispatch_once.compare_exchange_strong(expected, true)) {
							if constexpr (!this_is_job_start) parent->wait();
							impl::DispatchOnce(ctx, &job::DoTask, reinterpret_cast<void*>(this));
							return true;
						}
					}
					return false;
				};
				job_base& wait() override {
					dispatch();
					impl::Wait(ctx);
					return *dynamic_cast<job_base*>(this);
				};
				job_base* parent_ptr() const  override {
					if constexpr (this_is_job_start) return nullptr;
					else return dynamic_cast<job_base*>(const_cast<Parent*>(&*parent));
				};
				// task(...).and_then([](){ ... });
				// task([]() -> int { return 0; }).and_then([](int prev_job_result){ ... });
				// task(...).and_then([](job_base& prev_job){ ... });
				// task([]() -> int { return 0; }).and_then([](int prev_job_result, job_base& prev_job){ ... });
				template<typename G> decltype(auto) and_then(G&& ToDo);
				// task(...).and_then(0, 100, [](size_t index){ ... });
				// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result){ ... });
				// task(...).and_then(0, 100, [](size_t index, job_base& prev_job){ ... });
				// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result, job_base& prev_job){ ... });
				template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo);
			};

			template<typename F, typename Parent>
			class jobs final : public job_base {
				friend class job_base;
				template<typename G, typename ParentB> friend class job;
				template<typename G, typename ParentB> friend class jobs;
			public:
				using functionType = impl::function_traits<decltype(std::function(std::declval<F>()))>;
				using returnType = typename functionType::result_type;
				static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
				static constexpr size_t this_num_args = std::tuple_size_v<typename functionType::arguments>;
				static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

			public:
				std::weak_ptr<jobs> self;
				concurrency::concurrent_vector< std::weak_ptr<job_base> > children;

			protected:
				std::atomic<bool> dispatch_once;
				impl::dispatch_context ctx;
				std::shared_ptr<Parent> parent;
				size_t start;
				size_t end;
				F todo;

				// called once the dispatched job ends
				static void Callback(void* _args) {
					if (auto* data = reinterpret_cast<jobs*>(_args); data != nullptr)
						for (auto& x : data->children)
							if (auto p = x.lock(); p != nullptr)
								(void)p->dispatch();
				};

				template <size_t i>
				static decltype(auto) Argument(impl::job_argument const& _args) {
					if constexpr (this_num_args > i) {
						using arg = typename std::tuple_element_t<i, typename functionType::arguments>;
						static constexpr bool arg_is_jobBase = std::is_same_v<std::remove_const_t <arg>, job_base&>;
						static constexpr bool arg_is_any = std::is_same_v<std::decay_t<arg>, GL::any>;
						static constexpr bool arg_is_fast_any = std::is_same_v<std::decay_t <arg>, GL::any::fast_any>;

						jobs* data = reinterpret_cast<jobs*>(_args.task_memory);
						if constexpr (arg_is_jobBase) return *dynamic_cast<job_base*>(data->parent.get());
						else if constexpr (arg_is_any || arg_is_fast_any) return data->parent->result;
						else if constexpr (!this_is_job_start) return data->parent->result.cast<typename Parent::returnType>();
						else return;
					}
					else return;
				};

				static void DoTask(impl::job_argument const& _args) {
					jobs* data = reinterpret_cast<jobs*>(_args.task_memory);

					if constexpr (this_num_args == 0) {
						if constexpr (this_returns_void) (void)data->todo();
						else {
							if (_args.job_index == 0) data->result = GL::any::fast_any::instance(data->todo());
							else (void)data->todo();
						}
					}
					else {
						size_t t{ static_cast<size_t>(_args.job_index) + data->start };
						// first item must be the size_t
						using arg = typename std::tuple_element_t<0, typename functionType::arguments>;
						if constexpr (!std::is_same_v<std::decay_t<arg>, size_t>) {
							if constexpr (!std::is_constructible_v<arg, size_t>) {
								static_assert("The [optional] first argument to a jobs task must be a size_t index or castable from a size_t index.");
							}
						}

						if constexpr (this_num_args == 1) {
							if constexpr (this_returns_void) (void)data->todo(t);
							else {
								if (_args.job_index == 0) data->result = GL::any::fast_any::instance(data->todo(t));
								else (void)data->todo(t);
							}
						}
						else if constexpr (this_num_args == 2) {
							if constexpr (this_returns_void) (void)data->todo(t, Argument<1>(_args));
							else {
								if (_args.job_index == 0) data->result = GL::any::fast_any::instance(data->todo(t, Argument<1>(_args)));
								else (void)data->todo(t, Argument<1>(_args));
							}
						}
						else if constexpr (this_num_args == 3) {
							if constexpr (this_returns_void) (void)data->todo(t, Argument<1>(_args), Argument<2>(_args));
							else {
								if (_args.job_index == 0) data->result = GL::any::fast_any::instance(data->todo(t, Argument<1>(_args), Argument<2>(_args)));
								else (void)data->todo(t, Argument<1>(_args), Argument<2>(_args));
							}
						}
						else {
							static_assert("Not able to process async 'jobs' task with more than 3 arguments");
						}
					}
				};

			public:
				jobs(size_t _start, size_t _end, F&& _todo, Parent* _parent)
					: job_base()
					, dispatch_once{ false }
					, start{ _start }
					, end{ _end }
					, ctx{ 0, nullptr, &jobs::Callback, reinterpret_cast<void*>(this) }
					, parent{  }
					, todo{ std::move(_todo) }
				{
					if constexpr (!this_is_job_start) parent = _parent->self.lock();
				};
				jobs(jobs&&) = delete;
				jobs(jobs const&) = delete;
				jobs& operator=(jobs&&) = delete;
				jobs& operator=(jobs const&) = delete;
				virtual ~jobs() {
					dispatch();
					impl::Wait(ctx, false);
				};

				bool dispatch() override {
					static bool expected{ false };
					if (!dispatch_once.load()) {
						if (dispatch_once.compare_exchange_strong(expected, true)) {
							if constexpr (!this_is_job_start) parent->wait();
							impl::Dispatch(ctx, end - start, &jobs::DoTask, reinterpret_cast<void*>(this));
							return true;
						}
					}
					return false;
				};
				job_base& wait()  override {
					dispatch();
					impl::Wait(ctx);
					return *dynamic_cast<job_base*>(this);
				};
				job_base* parent_ptr() const override {
					if constexpr (this_is_job_start) return nullptr;
					else return dynamic_cast<job_base*>(const_cast<Parent*>(&*parent));
				};
				// task(...).and_then([](){ ... });
				// task([]() -> int { return 0; }).and_then([](int prev_job_result){ ... });
				// task(...).and_then([](job_base& prev_job){ ... });
				// task([]() -> int { return 0; }).and_then([](int prev_job_result, job_base& prev_job){ ... });
				template<typename G> decltype(auto) and_then(G&& ToDo);
				// task(...).and_then(0, 100, [](size_t index){ ... });
				// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result){ ... });
				// task(...).and_then(0, 100, [](size_t index, job_base& prev_job){ ... });
				// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result, job_base& prev_job){ ... });
				template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo);
			};
		};

		// start a task-chain with a argument-free function, e.g.:
		// task([]() -> int { return 10; })
		template<typename F, typename Parent = void> __forceinline decltype(auto) task(F&& ToDo, Parent* parent = nullptr) {
			auto out = std::make_shared<impl::job<F, Parent>>(std::move(ToDo), std::move(parent));
			out->self = out;
			if constexpr (!std::is_same_v<void, Parent>) parent->children.push_back(out);			
			else out->dispatch();			
			return out;
		};
		// start a task-chain with a for-loop with an indexed function, e.g.:
		// task(0, 1'000'000, [](size_t index) -> int { return index + 10; })
		// returning values from a for-loop task will only return the value from the first call. Returns from future calls will be discarded.
		template<typename F, typename Parent = void> __forceinline decltype(auto) task(size_t start, size_t end, F&& ToDo, Parent* parent = nullptr) {
			auto out = std::make_shared < impl::jobs<F, Parent> >(start, end, std::move(ToDo), std::move(parent));
			out->self = out;
			if constexpr (!std::is_same_v<void, Parent>) parent->children.push_back(out);			
			else out->dispatch();			
			return out;
		};
		// task(...).and_then([](){ ... });
		// task([]() -> int { return 0; }).and_then([](int prev_job_result){ ... });
		// task(...).and_then([](job_base& prev_job){ ... });
		// task([]() -> int { return 0; }).and_then([](int prev_job_result, job_base& prev_job){ ... });
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) impl::job<F, Parent>::and_then(G&& ToDo) {
			return task(std::move(ToDo), this);
		};
		// task(...).and_then(0, 100, [](size_t index){ ... });
		// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result){ ... });
		// task(...).and_then(0, 100, [](size_t index, job_base& prev_job){ ... });
		// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result, job_base& prev_job){ ... });
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) impl::job<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return task(start, end, std::move(ToDo), this);
		};
		// task(...).and_then([](){ ... });
		// task([]() -> int { return 0; }).and_then([](int prev_job_result){ ... });
		// task(...).and_then([](job_base& prev_job){ ... });
		// task([]() -> int { return 0; }).and_then([](int prev_job_result, job_base& prev_job){ ... });
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) impl::jobs<F, Parent>::and_then(G&& ToDo) {
			return task(std::move(ToDo), this);
		};
		// task(...).and_then(0, 100, [](size_t index){ ... });
		// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result){ ... });
		// task(...).and_then(0, 100, [](size_t index, job_base& prev_job){ ... });
		// task([]() -> int { return 0; }).and_then(0, 100, [](size_t index, int prev_job_result, job_base& prev_job){ ... });
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) impl::jobs<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return task(start, end, std::move(ToDo), this);
		};

		__forceinline void fire_and_forget(void(*Func)(void)) {
			struct Wrapper {
				static void Callback(void* _args) {
					if (impl::dispatch_context* data = reinterpret_cast<impl::dispatch_context*>(_args); data != nullptr)
						delete data;
				};
				static void DoTask(impl::job_argument const& _args) {
					reinterpret_cast<void(*)(void)>(_args.task_memory)();
				};
			};
			impl::dispatch_context* ctx = new impl::dispatch_context{ 0, nullptr, &Wrapper::Callback, nullptr };
			ctx->callback_data = reinterpret_cast<void*>(ctx);
			impl::DispatchOnce(*ctx, &Wrapper::DoTask, Func);
		};

		/* Generic form of a future<T>, which can be used to wait on and get the results of any job. Can be safely shared if multiple places will need access to the result once available. */
		class promise {
		protected:
			std::shared_ptr< job_base >
				_state;
			GL::type
				_type;

		public:
			promise() :
				_state(nullptr),
				_type()
			{};
			promise(std::shared_ptr< job_base >&& job, type const& Type) :
				_state(std::move(job)),
				_type(Type)
			{};
			promise(promise const&) = default;
			promise(promise&&) = default;
			promise& operator=(promise const&) = default;
			promise& operator=(promise&&) = default;
			virtual ~promise() = default;

			/* Returns true if this promise has been initialized correctly. Otherwise, false. */
			bool valid() const noexcept { return (bool)_state; };
			/* Wait until the requested job is completed. Repeated or simultaneous waiting is OK. */
			void wait() { if (_state) _state->wait(); };
			/* Get the result, waiting if necessary. */
			GL::any::fast_any get_any() const noexcept {
				if (_state) {
					_state->wait();
					return _state->result;
				}
				return GL::any::fast_any();
			};
			/* Get the anticipated return type, without needing to wait for the result. */
			GL::type Type() const { return _type; };
		};

		/* Specialized form of a promise, which can be used to handle type-casting for lambdas automatically, while still being useful for waiting on and getting the results of any job. */
		template <typename T> class future final : public promise {
		public:
			future() : promise() {};
			future(std::shared_ptr< job_base >&& job) : promise(std::move(job), GL::type_of<T>()) {};
			future(promise const& p_promise) : promise(p_promise) {};
			future(future const&) = default;
			future(future&&) = default;
			future& operator=(future const&) = default;
			future& operator=(future&&) = default;
			virtual ~future() = default;

			/* Cast-down to a generic promise that erases the information on the return type. Useful for sharing tasks between libraries where type info itself cannot be shared. */
			promise as_promise() const { return promise(reinterpret_cast<const promise&>(*this)); };

			/* get a copy of the result of the task. */
			decltype(auto) get() {
				if constexpr (std::is_same<void, T>()) {
					wait();
					return;
				}
				else {
					return static_cast<T>(get_any().cast<T>());
				}
			};
			/* get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. */
			decltype(auto) get_ref() {
				if constexpr (std::is_same<void, T>()) {
					wait();
					return;
				}
				else {
					return get_any().cast<T>();
				}
			};
			/* get a shared_pointer of the result of the task. must have already waited. */
			decltype(auto) get_shared() {
				if constexpr (std::is_same<void, T>()) {
					wait();
					return;
				}
				else {
					return get_any().cast<GL::shared_ptr<T>>();
				}
			};
		};

		/* returns a future<T> object for awaiting the results of the task, with optional inputs to the job, that are submitted at the time the job is performed. */
		template < typename F, typename... Args >
		static auto async(F&& function, Args... Fargs) {
			static constexpr size_t num_args = sizeof...(Args);
			using function_t = impl::function_traits<decltype(std::function(function))>;
			using tuple_t = std::tuple<Args...>;

			if constexpr (num_args == 0) {
				return future<typename function_t::result_type>(task(std::move(function)));
			}
			else {
				return future<typename function_t::result_type>(task([ToDo = std::forward<F>(function), Arg = std::array<any::fast_any, num_args>{ GL::any::fast_any::instance(Fargs)... }]() {
					if constexpr (num_args == 1) {
						return ToDo(Arg[0].cast<std::tuple_element_t<0, tuple_t>>());
					}
					else if constexpr (num_args == 2) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>()
						);
					}
					else if constexpr (num_args == 3) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>()
						);
					}
					else if constexpr (num_args == 4) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>()
						);
					}
					else if constexpr (num_args == 5) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>()
						);
					}
					else if constexpr (num_args == 6) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>()
						);
					}
					else if constexpr (num_args == 7) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>()
						);
					}
					else if constexpr (num_args == 8) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>()
						);
					}
					else if constexpr (num_args == 9) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>()
						);
					}
					else if constexpr (num_args == 10) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>(), Arg[9].cast<std::tuple_element_t<9, tuple_t>>()
						);
					}
					else if constexpr (num_args == 11) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>(), Arg[9].cast<std::tuple_element_t<9, tuple_t>>(), Arg[10].cast<std::tuple_element_t<10, tuple_t>>()
						);
					}
					else if constexpr (num_args == 12) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>(), Arg[9].cast<std::tuple_element_t<9, tuple_t>>(), Arg[10].cast<std::tuple_element_t<10, tuple_t>>(), Arg[11].cast<std::tuple_element_t<11, tuple_t>>()
						);
					}
					else if constexpr (num_args == 13) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>(), Arg[9].cast<std::tuple_element_t<9, tuple_t>>(), Arg[10].cast<std::tuple_element_t<10, tuple_t>>(), Arg[11].cast<std::tuple_element_t<11, tuple_t>>(), Arg[12].cast<std::tuple_element_t<12, tuple_t>>()
						);
					}
					else if constexpr (num_args == 14) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>(), Arg[9].cast<std::tuple_element_t<9, tuple_t>>(), Arg[10].cast<std::tuple_element_t<10, tuple_t>>(), Arg[11].cast<std::tuple_element_t<11, tuple_t>>(), Arg[12].cast<std::tuple_element_t<12, tuple_t>>(), Arg[13].cast<std::tuple_element_t<13, tuple_t>>()
						);
					}
					else if constexpr (num_args == 15) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>(), Arg[9].cast<std::tuple_element_t<9, tuple_t>>(), Arg[10].cast<std::tuple_element_t<10, tuple_t>>(), Arg[11].cast<std::tuple_element_t<11, tuple_t>>(), Arg[12].cast<std::tuple_element_t<12, tuple_t>>(), Arg[13].cast<std::tuple_element_t<13, tuple_t>>(), Arg[14].cast<std::tuple_element_t<14, tuple_t>>()
						);
					}
					else if constexpr (num_args == 16) {
						return ToDo(
							Arg[0].cast<std::tuple_element_t<0, tuple_t>>(), Arg[1].cast<std::tuple_element_t<1, tuple_t>>(), Arg[2].cast<std::tuple_element_t<2, tuple_t>>(), Arg[3].cast<std::tuple_element_t<3, tuple_t>>(), Arg[4].cast<std::tuple_element_t<4, tuple_t>>(), Arg[5].cast<std::tuple_element_t<5, tuple_t>>(), Arg[6].cast<std::tuple_element_t<6, tuple_t>>(), Arg[7].cast<std::tuple_element_t<7, tuple_t>>(),
							Arg[8].cast<std::tuple_element_t<8, tuple_t>>(), Arg[9].cast<std::tuple_element_t<9, tuple_t>>(), Arg[10].cast<std::tuple_element_t<10, tuple_t>>(), Arg[11].cast<std::tuple_element_t<11, tuple_t>>(), Arg[12].cast<std::tuple_element_t<12, tuple_t>>(), Arg[13].cast<std::tuple_element_t<13, tuple_t>>(), Arg[14].cast<std::tuple_element_t<14, tuple_t>>(), Arg[15].cast<std::tuple_element_t<15, tuple_t>>()
						);
					}
					else {
						static_assert("Not able to process more than 16 inputs to an async job without further specialization");
					}
			    }));
			}
		};
	};

};