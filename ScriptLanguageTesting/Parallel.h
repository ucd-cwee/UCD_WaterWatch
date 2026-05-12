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
				void clear_exception() {
					if (e == nullptr) return;
					if (std::exception_ptr* eptr = reinterpret_cast<std::exception_ptr*>(::InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&e), nullptr))) {
						delete eptr;
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

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, F const& ToDo) {
			struct IterData {
				const F* _to_do;
				iteratorType _start;

				static void DoTask(impl::job_argument const& _args) {
					IterData* data{ reinterpret_cast<IterData*>(_args.task_memory) };
					// using f_t = impl::function_traits<decltype(std::function(std::declval<F>()))>;

					//if constexpr (std::tuple_size_v<decltype(impl::function_traits(ToDo))::arguments> == 0) {
					//	(*data->_to_do)();
					//	return;
					//}
					//else if constexpr (std::is_reference_v<std::tuple_element_t<0, impl::function_traits<F>::arguments> > 
					//	&& !std::is_const_v<std::tuple_element_t<0, impl::function_traits<F>::arguments>> 
					//) {
					//	iteratorType t(static_cast<iteratorType>(_args.job_index) + data->_start);
					//	(*data->_to_do)(t);
					//	return;
					//}
					//else {
						(*data->_to_do)(static_cast<iteratorType>(_args.job_index) + data->_start);
						//return;
					//}
				};
			} data { &ToDo, start };

			impl::dispatch_context ctx{ 0, nullptr, nullptr };
			impl::Dispatch(ctx, static_cast<size_t>(static_cast<size_t>(end) - static_cast<size_t>(start)), &IterData::DoTask, reinterpret_cast<void*>(&data));
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

        template<typename F, typename Parent> class job;
		template<typename F, typename Parent> class jobs;
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

		template<typename F, typename Parent> friend class GL::parallel::job;
		template<typename F, typename Parent> friend class GL::parallel::jobs;
	};

	namespace parallel {
		template<typename F, typename Parent>
		class job final : public job_base {
			friend class job_base;
			template<typename G, typename ParentB> friend class job;
			template<typename G, typename ParentB> friend class jobs;
		public:
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
			static constexpr size_t this_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
			static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

		public:			
			std::weak_ptr<job> self;
			GL::atomic_vector< std::weak_ptr<job_base> > children;
			std::atomic<size_t> num_children;

		protected:
			std::atomic<bool> dispatch_once;
			impl::dispatch_context ctx;
			std::shared_ptr<Parent> parent;
			F todo;

			// called once the dispatched job ends
			static void Callback(void* _args) {
				job* data;
				size_t s, sz;
				if (data = reinterpret_cast<job*>(_args)) {
					std::this_thread::yield(); // helps with scheduling
					sz = data->num_children.load();
					for (s = 0; s < sz; ++s) {
						auto& x = data->children[s];
						if (auto p = x.lock()) {
							p->dispatch();
						}						
					}
				}
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
					if constexpr (this_num_args == 1) {
					    using arg0 = typename std::tuple_element_t<0, typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
						static constexpr bool arg0_jobBase = std::is_same_v<std::remove_const_t < arg0>, job_base&>;

						if constexpr (arg0_jobBase) {
							if constexpr (this_returns_void) data->todo(*dynamic_cast<job_base*>(&*data->parent));
							else data->result = GL::any(data->todo(*dynamic_cast<job_base*>(&*data->parent))).fast();
						}
						else {
							if constexpr (this_returns_void) data->todo(data->parent->result.cast());
							else data->result = GL::any(data->todo(data->parent->result.cast())).fast();
						}
					}
					else if constexpr (this_num_args == 2) {
						using arg0 = typename std::tuple_element_t<0, typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
						static constexpr bool arg0_jobBase = std::is_same_v<std::remove_const_t < arg0 >, job_base&>;

						using arg1 = typename std::tuple_element_t<1, typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
						static constexpr bool arg1_jobBase = std::is_same_v<std::remove_const_t < arg1 >, job_base&>;

						if constexpr (arg0_jobBase) {
							if constexpr (this_returns_void) data->todo(*dynamic_cast<job_base*>(&*data->parent), data->parent->result.cast());
							else data->result = GL::any(data->todo(*dynamic_cast<job_base*>(&*data->parent), data->parent->result.cast())).fast();
						}
						else if constexpr (arg1_jobBase) {
							if constexpr (this_returns_void) data->todo(data->parent->result.cast(), *dynamic_cast<job_base*>(&*data->parent));
							else data->result = GL::any(data->todo(data->parent->result.cast(), *dynamic_cast<job_base*>(&*data->parent))).fast();
						}
						else {
							static_assert("Not able to process async 'job' task with 2 arguments when none of them are the parent's job_base& reference");
						}
					}
					else {
						static_assert("Not able to process async 'job' task with more than 2 arguments");
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
				, num_children{ 0 }
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
			template<typename G, typename ParentB> friend class jobs;
		public:
			using returnType = typename impl::function_traits<decltype(std::function(std::declval<F>()))>::result_type;
			static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
			static constexpr size_t this_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments>;
			static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

		public:
			std::weak_ptr<jobs> self;
			GL::atomic_vector< std::weak_ptr<job_base> > children;
			std::atomic<size_t> num_children;

		protected:		
			std::atomic<bool> dispatch_once;
			impl::dispatch_context ctx;
			std::shared_ptr<Parent> parent;
			size_t start;
			size_t end;
			F todo;

			// called once the dispatched job ends
			static void Callback(void* _args) {
				jobs* data;
				size_t s, sz;
				if (data = reinterpret_cast<jobs*>(_args)) {
					std::this_thread::yield(); // helps with scheduling
					sz = data->num_children.load();
					for (s = 0; s < sz; ++s) {
						auto& x = data->children[s];
						if (auto p = x.lock()) {
							p->dispatch();
						}						
					}
				}
			};
			static void DoTask(impl::job_argument const& _args) {
				jobs* data = reinterpret_cast<jobs*>(_args.task_memory);

				if constexpr (!this_is_job_start) 
					data->parent->wait();				

				if constexpr (this_num_args == 0) {
					if constexpr (this_returns_void) (void)data->todo();
					else {
						if (_args.job_index == 0) data->result = GL::any(data->todo()).fast();
						else (void)data->todo();
					}
				}
				else {
					size_t t{ static_cast<size_t>(_args.job_index) + data->start };

					if constexpr (this_num_args == 1) {
						// first item must be the size_t
						if constexpr (this_returns_void) (void)data->todo(t);
						else {
							if (_args.job_index == 0) data->result = GL::any(data->todo(t)).fast();							
							else (void)data->todo(t);							
						}
					} 
					else {
						if constexpr (this_num_args == 2) {
							// first item must be the size_t
							// next may be the job_base or parent result
							static constexpr bool arg1_jobBase = std::is_same_v<std::remove_const_t < typename std::tuple_element_t<1, typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments> >, job_base&>;

							if constexpr (arg1_jobBase) {
								if constexpr (this_returns_void) (void)data->todo(t, *dynamic_cast<job_base*>(&*data->parent));
								else {
									if (_args.job_index == 0) data->result = GL::any(data->todo(t, *dynamic_cast<job_base*>(&*data->parent))).fast();
									else (void)data->todo(t, *dynamic_cast<job_base*>(&*data->parent));
								}
							}
							else {
								if constexpr (this_returns_void) (void)data->todo(t, data->parent->result.cast());
								else {
									if (_args.job_index == 0) data->result = GL::any(data->todo(t, data->parent->result.cast())).fast();
									else (void)data->todo(t, data->parent->result.cast());
								}
							}
						}
						else if constexpr (this_num_args == 3) {
							// first item must be the size_t
							// next may be the job_base or parent result
							// next may be the remainder
							static constexpr bool arg1_jobBase = std::is_same_v<std::remove_const_t < typename std::tuple_element_t<1, typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments> >, job_base&>;
							static constexpr bool arg2_jobBase = std::is_same_v<std::remove_const_t < typename std::tuple_element_t<2, typename impl::function_traits<decltype(std::function(std::declval<F>()))>::arguments> >, job_base&>;

							if constexpr (arg1_jobBase) {
								if constexpr (this_returns_void) (void)data->todo(t, *dynamic_cast<job_base*>(&*data->parent), data->parent->result.cast());
								else {
									if (_args.job_index == 0) data->result = GL::any(data->todo(t, *dynamic_cast<job_base*>(&*data->parent), data->parent->result.cast())).fast();
									else (void)data->todo(t, *dynamic_cast<job_base*>(&*data->parent), data->parent->result.cast());
								}
							}
							else {
								if constexpr (this_returns_void) (void)data->todo(t, data->parent->result.cast(), *dynamic_cast<job_base*>(&*data->parent));
								else {
									if (_args.job_index == 0) data->result = GL::any(data->todo(t, data->parent->result.cast(), *dynamic_cast<job_base*>(&*data->parent))).fast();
									else (void)data->todo(t, data->parent->result.cast(), *dynamic_cast<job_base*>(&*data->parent));
								}
							}
						}
						else {
							static_assert("Not able to process async 'jobs' task with more than 3 arguments");
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
				, num_children{ 0 }
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
				dispatch();
				impl::Wait(ctx, false);
			};

			bool dispatch() override {
				static bool expected{ false };
				if (!dispatch_once.load()) {
					if (dispatch_once.compare_exchange_strong(expected, true)) {
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
				if constexpr (this_is_job_start)
					return nullptr;
				else
					return dynamic_cast<job_base*>(const_cast<Parent*>(&*parent));
			};

			template<typename G> decltype(auto) and_then(G&& ToDo);
			template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo);
		};

		template<typename F, typename Parent = void> __forceinline decltype(auto) task(F&& ToDo, Parent* parent = nullptr) {
			auto out = std::make_shared<job<F, Parent>>(std::move(ToDo), std::move(parent));
			out->self = out;
			if constexpr (!std::is_same_v<void, Parent>) {
				auto index = parent->children.push_back(out);
				while (true) {
					auto prev = parent->num_children.load();
					if (prev < index) parent->num_children.compare_exchange_weak(prev, index);					
					else break;					
				}
			}
			else {
				out->dispatch();
			}
			return out;
		};
		template<typename F, typename Parent = void> __forceinline decltype(auto) task(size_t start, size_t end, F&& ToDo, Parent* parent = nullptr) {
			auto out = std::make_shared < jobs<F, Parent> >(start, end, std::move(ToDo), std::move(parent));
			out->self = out;
			if constexpr (!std::is_same_v<void, Parent>) {
				auto index = parent->children.push_back(out);
				while (true) {
					auto prev = parent->num_children.load();
					if (prev < index) parent->num_children.compare_exchange_weak(prev, index);					
					else break;					
				}
			}
			else {
				out->dispatch();
			}
			return out;
		};

		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) job<F, Parent>::and_then(G&& ToDo) {
			return task(std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) job<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return task(start, end, std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) jobs<F, Parent>::and_then(G&& ToDo) {
			return task(std::move(ToDo), this);
		};
		template<typename F, typename Parent> template<typename G> __forceinline decltype(auto) jobs<F, Parent>::and_then(size_t start, size_t end, G&& ToDo) {
			return task(start, end, std::move(ToDo), this);
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
			virtual ~promise() {};

			/* Returns true if this promise has been initialized correctly. Otherwise, false. */
			bool valid() const noexcept { return (bool)_state; };
			/* Wait until the requested job is completed. Repeated or simultaneous waiting is OK. */
			void wait() {
				if (_state) {
					_state->wait();
				}
			};
			/* Get the result, waiting if necessary. */
			GL::any::fast_any get_any() const noexcept {
				if (_state) {
					_state->wait();
					return _state->result;
				}
				return GL::any::fast_any();
			};
			/* Get the anticipated return type, without needing to wait for the result. */
			GL::type Type() const {
				return _type;
			};
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
			virtual ~future() {};

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
		__forceinline static auto async(F&& function, Args... Fargs) {
			static constexpr size_t num_args = sizeof...(Args);
			if constexpr (num_args == 0) {
				return future<typename impl::function_traits<decltype(std::function(function))>::result_type>(task(std::move(function)));
			}
			else {
				F func = std::move(function);
				std::array<any::fast_any, num_args> arr; {
					std::array<any, num_args> arr1{ Fargs... };
					std::transform(arr1.begin(), arr1.end(), arr.begin(), [](any& from) -> any::fast_any { return from.fast(); });
				}

				return future<typename impl::function_traits<decltype(std::function(func))>::result_type>(task([ToDo = std::move(func), Arg = std::move(arr)]() {
					if constexpr (num_args == 1) {
						return ToDo(Arg[0].cast());
					}
					else if constexpr (num_args == 2) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast()
						);
					}
					else if constexpr (num_args == 3) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast()
						);
					}
					else if constexpr (num_args == 4) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast()
						);
					}
					else if constexpr (num_args == 5) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast()
						);
					}
					else if constexpr (num_args == 6) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast()
						);
					}
					else if constexpr (num_args == 7) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast()
						);
					}
					else if constexpr (num_args == 8) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast()
						);
					}
					else if constexpr (num_args == 9) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast()
						);
					}
					else if constexpr (num_args == 10) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast(), Arg[9].cast()
						);
					}
					else if constexpr (num_args == 11) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast(), Arg[9].cast(), Arg[10].cast()
						);
					}
					else if constexpr (num_args == 12) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast(), Arg[9].cast(), Arg[10].cast(), Arg[11].cast()
						);
					}
					else if constexpr (num_args == 13) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast(), Arg[9].cast(), Arg[10].cast(), Arg[11].cast(), Arg[12].cast()
						);
					}
					else if constexpr (num_args == 14) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast(), Arg[9].cast(), Arg[10].cast(), Arg[11].cast(), Arg[12].cast(), Arg[13].cast()
						);
					}
					else if constexpr (num_args == 15) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast(), Arg[9].cast(), Arg[10].cast(), Arg[11].cast(), Arg[12].cast(), Arg[13].cast(), Arg[14].cast()
						);
					}
					else if constexpr (num_args == 16) {
						return ToDo(
							Arg[0].cast(), Arg[1].cast(), Arg[2].cast(), Arg[3].cast(), Arg[4].cast(), Arg[5].cast(), Arg[6].cast(), Arg[7].cast(),
							Arg[8].cast(), Arg[9].cast(), Arg[10].cast(), Arg[11].cast(), Arg[12].cast(), Arg[13].cast(), Arg[14].cast(), Arg[15].cast()
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