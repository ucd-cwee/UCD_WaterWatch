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
	namespace detail {
		template <typename T>
		struct is_static_member_function : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) const> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...)> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) const volatile> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) volatile> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...)&> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) const&> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...)&&> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) const&&> : std::false_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) noexcept> : std::true_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) const noexcept> : std::true_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) volatile noexcept> : std::true_type {};

		template <typename R, typename C, typename... Args>
		struct is_static_member_function<R(C::*)(Args...) const volatile noexcept> : std::true_type {};

		template<typename... Param> struct Function_Params {
			typedef std::tuple<Param...> argType;
			static constexpr auto numArgs = std::tuple_size_v<argType>;

			// Specialization to unpack the tuple's internal parameter pack
			template <typename Ret>
			struct to_function_pointer {
				using type = Ret(*)(Param...);
			};
		};

		template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false/*, bool IsStatelessObject = false*/>
		struct Function_Signature {
			typedef Params Param_Types;
			typedef Class Class_Type;
			typedef Ret Return_Type;

			// constexpr static const bool is_stateless_object = IsStatelessObject; // e.g. lambda object that does not capture anything
			constexpr static const bool is_object = IsObject; // e.g. lambda object
			constexpr static const bool is_member = IsMember; // e.g. first param MUST be an alive Class type. May be function or parameter.
			constexpr static const bool is_member_object = IsMemberObject; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
			constexpr static const bool is_member_function = !is_member_object && is_member; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
			constexpr static const bool is_static_member_function = std::is_same_v< Class_Type, void>; // e.g. free function

			template<typename T> constexpr Function_Signature(T&&) noexcept { };
			constexpr Function_Signature() noexcept = default;
		};

		// Free functions
		template<typename Ret, typename... Param>
		Function_Signature(Ret(*f)(Param...))
			->Function_Signature<Ret, void, Function_Params<Param...>, false, false, false>; // static function

		// no reference specifier
		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile)
			->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile const)
			->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...))
			->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) const)
			->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

		// & reference specifier
		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile&)
			->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile const&)
			->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...)&)
			->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) const&)
			->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

		// && reference specifier
		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile&&)
			->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile const&&)
			->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...)&&)
			->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) const&&)
			->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true, false, false>; // member function

		// Free functions
		template<typename Ret, typename... Param>
		Function_Signature(Ret(*f)(Param...) noexcept)
			->Function_Signature<Ret, void, Function_Params<Param...>, false, false, false>; // static function

		// no reference specifier
		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile noexcept)
			->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile const noexcept)
			->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) noexcept)
			->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) const noexcept)
			->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

		// & reference specifier
		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile& noexcept)
			->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile const& noexcept)
			->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) & noexcept)
			->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) const& noexcept)
			->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true, false, false>; // member function

		// && reference specifier
		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile&& noexcept)
			->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) volatile const&& noexcept)
			->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) && noexcept)
			->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class, typename... Param>
		Function_Signature(Ret(Class::* f)(Param...) const&& noexcept)
			->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true, false, false>; // member function

		template<typename Ret, typename Class>
		Function_Signature(Ret Class::* f)
			->Function_Signature<Ret, Class, Function_Params<Class&>, true, true, false>; // member object

		namespace impl {
			template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false/*, bool IsStatelessObject = false*/>
			struct Function_Signature_m1 {
				typedef Params Param_Types;
				typedef Class Class_Type;
				typedef Ret Return_Type;

				// constexpr static const bool is_stateless_object = IsStatelessObject; // e.g. lambda object that does not capture anything
				constexpr static const bool is_object = IsObject; // e.g. lambda object
				constexpr static const bool is_member = IsMember; // e.g. first param MUST be an alive Class type. May be function or parameter.
				constexpr static const bool is_member_object = IsMemberObject; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
				constexpr static const bool is_member_function = !is_member_object && is_member; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
				constexpr static const bool is_static_member_function = std::is_same_v< Class_Type, void>; // e.g. free function

				template<typename T> constexpr Function_Signature_m1(T&&) noexcept { };
				constexpr Function_Signature_m1() noexcept = default;
			};

			// Free functions
			template<typename Ret, typename... Param>
			Function_Signature_m1(Ret(*f)(Param...))
				->Function_Signature_m1<Ret, void, Function_Params<Param...>, false, false, false>; // static function

			// no reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile const)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...))
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) const)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			// & reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile const&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...)&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) const&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			// && reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile&&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile const&&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...)&&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) const&&)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			// Free functions
			template<typename Ret, typename... Param>
			Function_Signature_m1(Ret(*f)(Param...) noexcept)
				->Function_Signature_m1<Ret, void, Function_Params<Param...>, false, false, false>; // static function

			// no reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile const noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) const noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			// & reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile& noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile const& noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) & noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) const& noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			// && reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile&& noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) volatile const&& noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) && noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature_m1(Ret(Class::* f)(Param...) const&& noexcept)
				->Function_Signature_m1<Ret, Class, Function_Params<Param...>, true, false, false>; // member function

		}

		// primary template handles types that have no nested ::type member:
		template<class, class = std::void_t<>>
		struct has_call_operator : std::false_type {};

		// specialization recognizes types that do have a nested ::type member:
		template<class T>
		struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

		template<typename Func>
		auto function_signature(Func const& f) {
			if constexpr (has_call_operator<Func>::value) {
				return Function_Signature<
					typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Return_Type,
					void,
					typename decltype(impl::Function_Signature_m1{ &std::decay_t<Func>::operator() })::Param_Types,
					false,
					false,
					true
					// , std::is_empty_v<Func>
				> {};
			}
			else {
				return Function_Signature{ f };
			}
		};

		template<typename Obj, typename Param1, typename... Rest>
		Param1 get_first_param(Function_Params<Param1, Rest...>, Obj&& obj) {
			return static_cast<Param1>(std::forward<Obj>(obj));
		};
	}

	namespace parallel {
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

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> void Std_For(iteratorType start, iteratorType end, F const& ToDo) {
			using f_t = impl::function_traits<decltype(std::function(ToDo))>;

			//if constexpr (noexcept(ToDo(start))) {
			//	GL::sequence<iteratorType> seq(start, end); // 0..999
			//	std::for_each(
			//		std::execution::par,
			//		seq.begin(),
			//		seq.end(),
			//		[&](auto& x) { // copies are safer, and the resulting code will be as quick.
			//			ToDo(x);
			//		}
			//	);
			//}
			//else {
			GL::sequence<iteratorType> seq(start, end); // 0..999
			std::exception_ptr* e{ nullptr };
			std::for_each(
				std::execution::par,
				seq.begin(),
				seq.end(),
				[&](auto& x) { // copies are safer, and the resulting code will be as quick.
					try {
						if (!e) {
							if constexpr (std::tuple_size_v<f_t::arguments> == 0) {
								(void)ToDo();
							}
							else if constexpr (std::is_reference_v<std::tuple_element_t<0, f_t::arguments> > && !std::is_const_v< std::remove_reference_t<std::tuple_element_t<0, f_t::arguments>>>) {
								iteratorType t = static_cast<iteratorType>(x);
								(void)ToDo(t);
							}
							else {
								(void)ToDo(x);
							}
						}
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
			//}
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> void Std_For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
			using f_t = impl::function_traits<decltype(std::function(ToDo))>;
			GL::sequence<iteratorType> seq(start, end, step); // 0..999
			std::exception_ptr* e{ nullptr };

			std::for_each(
				std::execution::par,
				seq.begin(),
				seq.end(),
				[&](auto& x) { // copies are safer, and the resulting code will be as quick.
					try {
						if (!e) {
							if constexpr (std::tuple_size_v<f_t::arguments> == 0) {
								(void)ToDo();
							}
							else if constexpr (std::is_reference_v<std::tuple_element_t<0, f_t::arguments> > && !std::is_const_v< std::remove_reference_t<std::tuple_element_t<0, f_t::arguments>>>) {
								iteratorType t = static_cast<iteratorType>(x);
								(void)ToDo(t);
							}
							else {
								(void)ToDo(x);
							}
						}
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
		template<typename containerType, typename F> void Std_ForEach(containerType& container, F const& ToDo) {
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
		template<typename containerType, typename F> void Std_ForEach(containerType const& container, F const& ToDo) {
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

		/* parallel_for (auto i = start; i < end; i++){ todo(i); } */
		template<typename iteratorType, class F> void For(iteratorType start, iteratorType end, F const& ToDo) {
			if (end >= start) {
#if 0
				return Std_For(start, end, ToDo);
#else
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
								std::tuple_element_t<0, typename f_t::arguments> t = (std::tuple_element_t<0, typename f_t::arguments>)_args.job_index;
								(void)data._to_do(t);
							}
							else {
								(void)data._to_do((std::tuple_element_t<0, typename f_t::arguments>)_args.job_index);
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
								std::tuple_element_t<0, typename f_t::arguments> t = (std::tuple_element_t<0, typename f_t::arguments>)(static_cast<iteratorType>(_args.job_index) + data._start);
								(void)data._to_do(t);
							}
							else {
								(void)data._to_do((std::tuple_element_t<0, typename f_t::arguments>)(static_cast<iteratorType>(_args.job_index) + data._start));
							}
						};
					} data{ ToDo, start };
					impl::Dispatch(ctx, static_cast<size_t>(end - start), &IterData::DoTask, static_cast<void*>(&data));
				}
				impl::Wait(ctx);
#endif
			}
			else {
				For(end, start, ToDo);
			}
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> void For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
#if 0
			return Std_For(start, end, step, ToDo);
#else		
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
#endif
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> void For_Each(containerType& container, F const& ToDo) {
#if 0
			return Std_ForEach(container, ToDo);
#else
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
#endif
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> void For_Each(containerType const& container, F const& ToDo) {
#if 0
			return Std_ForEach(container, ToDo);
#else		
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
#endif
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
#if 0
			F out{ std::forward<F>(SharedObject) };
			Std_For(0, numToDispatch, [&](size_t i) {
				ToDo(i, out);
				});
			return out;
#else
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
#endif
		};

		/* for (int i = 0; i < numToDispatch; i++){ ToDo(i, SharedObject); } return SharedObject; */
		template<typename F, typename G> F& Dispatch(size_t numToDispatch, F& SharedObject, G const& ToDo) {
#if 0
			Std_For(0, numToDispatch, [&](size_t i) {
				ToDo(i, SharedObject);
				});
			return SharedObject;
#else
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
#endif
		};

		namespace impl {
			template<typename F, typename Parent> class job;
			template<typename F, typename Parent> class jobs;
		}
	};

	namespace parallel {
		class job_base {
		public:
			job_base() = default;
			job_base(job_base const&) = delete;
			job_base(job_base&&) = default;
			job_base& operator=(job_base const&) = delete;
			job_base& operator=(job_base&&) = default;
			virtual ~job_base() = default;

			GL::atomic_any result;

			// Returns true if the job was dispatched. Returns false if the job had already been dispatched previously. 
			virtual bool dispatch() = 0;
			// Will wait until job is and may help with parallel computing. If an exception is caught during the async operation, it will be re-thrown at this time. 
			virtual job_base& wait() = 0;
			// returns a pointer to the parent of this job, if one exists. Can be used to get the results of past jobs. 
			virtual job_base* parent_ptr() const = 0;

			template<typename F, typename Parent> friend class GL::parallel::impl::job;
			template<typename F, typename Parent> friend class GL::parallel::impl::jobs;
		};

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
				GL::atomic_vector< std::weak_ptr<job_base> > children;

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
						static constexpr bool arg_is_jobBase = std::is_same_v<std::decay_t<arg>, job_base>;
						static constexpr bool arg_is_any =
							std::is_same_v<std::decay_t<arg>, GL::any> ||
							std::is_same_v<std::decay_t <arg>, GL::atomic_any>;

						job* data = reinterpret_cast<job*>(_args.task_memory);
						if constexpr (!this_is_job_start) {
							static constexpr bool returnVal_is_any =
								std::is_same_v<std::decay_t<typename Parent::returnType>, GL::any> ||
								std::is_same_v<std::decay_t<typename Parent::returnType>, GL::atomic_any>;

							typedef typename GL::any::DataCaster::is_SharedPtr_class<typename Parent::returnType>::type isShared;
							typedef typename GL::any::DataCaster::is_stdSharedPtr_class<typename Parent::returnType>::type isStdShared;
							static constexpr bool is_shared_ptr{ isShared::value };
							static constexpr bool is_std_shared_ptr{ isStdShared::value };
							static constexpr bool returnVal_is_shared_ptr =
								isShared::value ||
								isStdShared::value;

							if constexpr (arg_is_any) {
								//std::cout << GL::printf("ARG %i IS ANY @ %i\n", i, __LINE__);
								return data->parent->result;
							}
							else if constexpr (arg_is_jobBase) {
								//std::cout << GL::printf("ARG %i IS JOB BASE @ %i\n", i, __LINE__);
								return *dynamic_cast<job_base*>(data->parent.get());
							}
							else if constexpr (returnVal_is_any) {
								if constexpr (std::is_convertible_v<typename Parent::returnType, arg>) {
									//std::cout << GL::printf("ARG %i IS CONSTRUCTABLE FROM PARENT'S ANY RESULT @ %i\n", i, __LINE__);
									return data->parent->result.cast<typename Parent::returnType>();
								}
								else {
									//std::cout << GL::printf("ARG %i IS NOT CONSTRUCTABLE FROM PARENT'S ANY RESULT -- !GUESSING THE TYPE! -- @ %i\n", i, __LINE__);
									return data->parent->result.cast<arg>();
								}
							}
							else if constexpr (returnVal_is_shared_ptr) {
								//std::cout << GL::printf("ARG %i IS RECIEVING PARENT'S RETURNED SHARED_PTR @ %i\n", i, __LINE__);
								return data->parent->result.cast<typename GL::type_erasure::get_type<typename Parent::returnType>::type&>();
							}
							else {
								//std::cout << GL::printf("ARG %i IS RECIEVING PARENT'S RETURNED VALUE-TYPE @ %i\n", i, __LINE__);
								return data->parent->result.cast<typename Parent::returnType>();
							}
						}
						else {
							static_assert(false, "Job starts cannot evaluate arguments using this path");
						}
					}
					else {
						static_assert(false, "Wrong argument index in evaluation of argument in task");
					}
				};

				static void DoTask(impl::job_argument const& _args) {
					job* data = reinterpret_cast<job*>(_args.task_memory);

					if constexpr (this_is_job_start || (this_num_args == 0)) {
						if constexpr (this_returns_void) data->todo();
						else data->result = GL::any::instance(data->todo());
					}
					else {
						if constexpr (this_num_args == 1) {
							if constexpr (this_returns_void) data->todo(Argument<0>(_args));
							else data->result = GL::any::instance(data->todo(Argument<0>(_args)));
						}
						else if constexpr (this_num_args == 2) {
							if constexpr (this_returns_void) data->todo(Argument<0>(_args), Argument<1>(_args));
							else data->result = GL::any::instance(data->todo(Argument<0>(_args), Argument<1>(_args)));
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
				GL::atomic_vector< std::weak_ptr<job_base> > children;

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
						static constexpr bool arg_is_jobBase = std::is_same_v<std::decay_t<arg>, job_base>;
						static constexpr bool arg_is_any =
							std::is_same_v<std::decay_t<arg>, GL::any> ||
							std::is_same_v<std::decay_t <arg>, GL::atomic_any>;

						jobs* data = reinterpret_cast<jobs*>(_args.task_memory);
						if constexpr (!this_is_job_start) {
							static constexpr bool returnVal_is_any =
								std::is_same_v<std::decay_t<typename Parent::returnType>, GL::any> ||
								std::is_same_v<std::decay_t<typename Parent::returnType>, GL::atomic_any>;

							typedef typename GL::any::DataCaster::is_SharedPtr_class<typename Parent::returnType>::type isShared;
							typedef typename GL::any::DataCaster::is_stdSharedPtr_class<typename Parent::returnType>::type isStdShared;
							static constexpr bool is_shared_ptr{ isShared::value };
							static constexpr bool is_std_shared_ptr{ isStdShared::value };
							static constexpr bool returnVal_is_shared_ptr =
								isShared::value ||
								isStdShared::value;

							if constexpr (arg_is_any) {
								//std::cout << GL::printf("ARGs %i IS ANY @ %i\n", i, __LINE__);
								return data->parent->result;
							}
							else if constexpr (arg_is_jobBase) {
								//std::cout << GL::printf("ARGs %i IS JOB BASE @ %i\n", i, __LINE__);
								return *dynamic_cast<job_base*>(data->parent.get());
							}
							else if constexpr (returnVal_is_any) {
								if constexpr (std::is_convertible_v<typename Parent::returnType, arg>) {
									//std::cout << GL::printf("ARGs %i IS CONSTRUCTABLE FROM PARENT'S ANY RESULT @ %i\n", i, __LINE__);
									return data->parent->result.cast<typename Parent::returnType>();
								}
								else {
									//std::cout << GL::printf("ARGs %i IS NOT CONSTRUCTABLE FROM PARENT'S ANY RESULT -- !GUESSING THE TYPE! -- @ %i\n", i, __LINE__);
									return data->parent->result.cast<arg>();
								}
							}
							else if constexpr (returnVal_is_shared_ptr) {
								//std::cout << GL::printf("ARGs %i IS RECIEVING PARENT'S RETURNED SHARED_PTR @ %i\n", i, __LINE__);
								return data->parent->result.cast<typename GL::type_erasure::get_type<typename Parent::returnType>::type&>();
							}
							else {
								//std::cout << GL::printf("ARGs %i IS RECIEVING PARENT'S RETURNED VALUE-TYPE @ %i\n", i, __LINE__);
								return data->parent->result.cast<typename Parent::returnType>();
							}
						}
						else {
							static_assert(false, "Jobs starts cannot evaluate arguments using this path");
						}
					}
					else {
						static_assert(false, "Wrong argument index in evaluation of argument in task");
					}
				};

				static void DoTask(impl::job_argument const& _args) {
					if constexpr (this_num_args == 0) {
						if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo();
						else {
							if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(static_cast<jobs*>(_args.task_memory)->todo());
							else (void)static_cast<jobs*>(_args.task_memory)->todo();
						}
					}
					else {
						// first item must be the size_t
						if constexpr (!std::is_same_v<std::decay_t<typename std::tuple_element_t<0, typename functionType::arguments>>, size_t>) {
							if constexpr (!std::is_constructible_v<typename std::tuple_element_t<0, typename functionType::arguments>, size_t>) {
								static_assert("The [optional] first argument to a jobs task must be a size_t index or castable from a size_t index.");
							}
						}

						if constexpr (this_num_args == 1) {
							if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start);
							else {
								if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start));
								else (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start);
							}
						}
						else if constexpr (this_num_args == 2) {
							if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args));
							else {
								if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args)));
								else (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args));
							}
						}
						else if constexpr (this_num_args == 3) {
							if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args), Argument<2>(_args));
							else {
								if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args), Argument<2>(_args)));
								else (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args), Argument<2>(_args));
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

		namespace tasks {
			class AsyncJobBaseBase {
				template <typename return_t> friend class AsyncJobBase;

			protected:
				impl::dispatch_context
					ctx;
				std::shared_ptr<AsyncJobBaseBase>
					_parent;
				GL::atomic_any
					_result;
				GL::atomic_vector< std::weak_ptr<AsyncJobBaseBase> >
					_children;
				std::weak_ptr<AsyncJobBaseBase>
					_self;
			public:
				AsyncJobBaseBase(void (*callback)(void*), void* callback_data, std::shared_ptr<AsyncJobBaseBase>&& parent)
					: ctx{ 0, nullptr, callback, callback_data }
					, _parent{ std::forward<std::shared_ptr<AsyncJobBaseBase>>(parent) }
					, _result(nullptr)	
					, _children()
					, _self()
				{};
				AsyncJobBaseBase(AsyncJobBaseBase const&) = delete;
				AsyncJobBaseBase(AsyncJobBaseBase&&) noexcept = delete;
				AsyncJobBaseBase& operator=(AsyncJobBaseBase const&) = delete;
				AsyncJobBaseBase& operator=(AsyncJobBaseBase&&) noexcept = delete;
				virtual ~AsyncJobBaseBase() {
					this->wait();					
				};
				void wait() {
					if (this->_parent) this->_parent->wait();
					if (ctx.is_busy()) {
						impl::Wait(this->ctx, true);
					}
				};
				bool is_busy() {
					if (this->_parent && this->_parent->is_busy())
						return true;
					else 
						return ctx.is_busy();
				};
				virtual void dispatch() = 0;
				void set_self(std::weak_ptr<AsyncJobBaseBase> self) {
					this->_self = self;
					if (_parent) {
						_parent->_children.push_back(this->_self);
					}
				};
			};

			template <typename return_t>
			class AsyncJobBase : public AsyncJobBaseBase {
			protected:
				static constexpr bool this_returns_void = std::is_void_v<return_t>;

			public:
				AsyncJobBase(void (*callback)(void*), void* callback_data, std::shared_ptr<AsyncJobBaseBase>&& parent)
					: AsyncJobBaseBase(callback, callback_data, std::forward<std::shared_ptr<AsyncJobBaseBase>>(parent))
				{};
				AsyncJobBase(AsyncJobBase const&) = delete;
				AsyncJobBase(AsyncJobBase&&) noexcept = delete;
				AsyncJobBase& operator=(AsyncJobBase const&) = delete;
				AsyncJobBase& operator=(AsyncJobBase&&) noexcept = delete;
				virtual ~AsyncJobBase() {
					this->wait();
				};
				GL::atomic_any& get_any() {
					this->wait();
					if constexpr (!this_returns_void) {
						return _result;
					}
				};
				decltype(auto) get_ref() {
					this->wait();
					if constexpr (!this_returns_void) {
						if constexpr (std::is_same_v<GL::any, return_t> || std::is_same_v<GL::atomic_any, return_t>) {
							return get_any();
						}
						else {
							return _result.cast<return_t>();
						}
					}
				};
				decltype(auto) get() {
					this->wait();
					if constexpr (!this_returns_void) {
						if constexpr (std::is_same_v<GL::any, return_t> || std::is_same_v<GL::atomic_any, return_t>) {
							return get_any();
						}
						else {
							return (return_t)_result.cast<return_t>();
						}
					}
				};
				decltype(auto) get_shared() {
					this->wait();
					if constexpr (!this_returns_void) {
						if constexpr (!this_returns_void) {
							if constexpr (std::is_same_v<GL::any, return_t> || std::is_same_v<GL::atomic_any, return_t>) {
								return get_any();
							}
							else {
								return _result.cast<GL::shared_ptr<return_t>>();
							}
						}
					}
				};

				template<typename G, bool dispatch_multiple, typename ParentReturnType>
				class AsyncJob final : public AsyncJobBase<return_t> {
				public:
					static constexpr bool this_returns_void = std::is_void_v<return_t>;

				protected:
					G/* const& */
						function;
					size_t 
						offset;
					size_t 
						count;


				public:
					// called once the dispatched job ends
					static void Callback(void* _args) {
						if (auto* data = reinterpret_cast<AsyncJob*>(_args); data != nullptr) {
							for (std::weak_ptr<AsyncJobBaseBase>& child : data->_children) 
								if (std::shared_ptr<AsyncJobBaseBase> Child = child.lock())
									Child->dispatch(); // will only ever dispatch once. 							
						}
					};
					static decltype(auto) cast_return(GL::atomic_any const& rhs) {
						using function_t = impl::function_traits<decltype(std::function(reinterpret_cast<AsyncJob*>(0)->function))>;
						static constexpr size_t num_arg = std::tuple_size_v<typename function_t::arguments>;
						if constexpr (std::is_void_v<ParentReturnType>) return;
						else if constexpr (num_arg == 0) return;
						else if constexpr (dispatch_multiple && (num_arg == 1)) return;
						else {
							using function_arg_t = std::tuple_element_t<num_arg - 1, typename function_t::arguments>;
							static constexpr bool returned_any = std::is_same_v<GL::any, ParentReturnType> || std::is_same_v<GL::atomic_any, ParentReturnType>; // this means they are no help in determining the return type...
							if constexpr (returned_any) {
								if constexpr (std::is_same_v<std::decay_t<function_arg_t>, GL::any> || std::is_same_v<std::decay_t<function_arg_t>, GL::atomic_any>) {
									return rhs;
								}
								else {
									return rhs.cast<function_arg_t>();
								}
							}
							else {
								if constexpr (std::is_same_v<function_arg_t, std::shared_ptr<ParentReturnType>>) {
									return rhs.cast<std::shared_ptr<ParentReturnType>>();
								}
								else if constexpr (std::is_same_v<function_arg_t, GL::shared_ptr<ParentReturnType>>) {
									return rhs.cast<GL::shared_ptr<ParentReturnType>>();
								}
								else if constexpr (std::is_convertible_v<ParentReturnType&, function_arg_t>) {
									return rhs.cast<ParentReturnType&>();
								}
								else {
									return rhs.cast<ParentReturnType>();
								}
							}
						}
					};
					static void DoTask(impl::job_argument const& _args) {
						using function_t = impl::function_traits<decltype(std::function(reinterpret_cast<AsyncJob*>(_args.task_memory)->function))>;
						static constexpr size_t num_arg = std::tuple_size_v<typename function_t::arguments>;

						AsyncJob* data = reinterpret_cast<AsyncJob*>(_args.task_memory);
						if constexpr (std::is_void_v<ParentReturnType>) {
							if constexpr (dispatch_multiple) {
								if constexpr (this_returns_void) {
									if constexpr (num_arg == 0) data->function();
									else if constexpr (num_arg == 1) data->function(_args.job_index + data->offset);
								}
								else {
									if (_args.job_index == 0) {
										if constexpr (num_arg == 0) data->_result = GL::any::instance(data->function());
										else if constexpr (num_arg == 1) data->_result = GL::any::instance(data->function(_args.job_index + data->offset));
									}
									else {
										if constexpr (num_arg == 0) data->function();
										else if constexpr (num_arg == 1) data->function(_args.job_index + data->offset);
									}
								}
							}
							else {
								if constexpr (this_returns_void) data->function();
								else data->_result = GL::any::instance(data->function());
							}
						}
						else {
							if constexpr (dispatch_multiple) {
								if constexpr (this_returns_void) {
									if constexpr (num_arg == 0) data->function();
									else if constexpr (num_arg == 1) data->function(_args.job_index + data->offset);
									else if constexpr (num_arg == 2) data->function(_args.job_index + data->offset, cast_return(data->_parent->_result));
								}
								else {
									if (_args.job_index == 0) {
										if constexpr (num_arg == 0) data->_result = GL::any::instance(data->function());
										else if constexpr (num_arg == 1) data->_result = GL::any::instance(data->function(_args.job_index + data->offset));
										else if constexpr (num_arg == 2) data->_result = GL::any::instance(data->function(_args.job_index + data->offset, cast_return(data->_parent->_result)));
									}
									else {
										if constexpr (num_arg == 0) data->function();
										else if constexpr (num_arg == 1) data->function(_args.job_index + data->offset);
										else if constexpr (num_arg == 2) data->function(_args.job_index + data->offset, cast_return(data->_parent->_result));
									}
								}
							}
							else {
								if constexpr (num_arg == 0) {
									if constexpr (this_returns_void) data->function();
									else data->_result = GL::any::instance(data->function());
								}
								else if constexpr (num_arg == 1) {
									if constexpr (this_returns_void) data->function(cast_return(data->_parent->_result));
									else data->_result = GL::any::instance(data->function(cast_return(data->_parent->_result)));
								}
							}
						}
					};

				protected:
					void dispatch() override {
						if constexpr (!dispatch_multiple) {
							if (GL::interlocked::exchange(count, 0ull) > 0) {
								impl::DispatchOnce(this->ctx, &DoTask, reinterpret_cast<void*>(this));
							}
						}
						else {
							if (auto final_count = GL::interlocked::exchange(count, 0ull); final_count > 0) {
								impl::Dispatch(this->ctx, final_count, &DoTask, reinterpret_cast<void*>(this));
							}
						}
					};

				public:
					AsyncJob(G const& func, std::shared_ptr<AsyncJobBaseBase>&& parent)
						: AsyncJobBase<return_t>(&Callback, reinterpret_cast<void*>(this), std::forward<std::shared_ptr<AsyncJobBaseBase>>(parent))
						, function{ func }
						, offset{ 0 }
						, count{ 1 }
					{
						if (!this->is_busy()) dispatch();
					};
					AsyncJob(size_t start, size_t end, G const& func, std::shared_ptr<AsyncJobBaseBase>&& parent)
						: AsyncJobBase<return_t>(&Callback, reinterpret_cast<void*>(this), std::forward<std::shared_ptr<AsyncJobBaseBase>>(parent))
						, function{ func }
						, offset{ start }
						, count{ end - start }
					{						
						if (!this->is_busy()) dispatch();
					};
					AsyncJob(AsyncJob const&) = delete;
					AsyncJob(AsyncJob&&) noexcept = delete;
					AsyncJob& operator=(AsyncJob const&) = delete;
					AsyncJob& operator=(AsyncJob&&) noexcept = delete;
					virtual ~AsyncJob() {
						this->wait();
					};
				};

				template <typename G> decltype(auto) and_then(G const& func) {
					static constexpr bool and_then_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(func))>::arguments>;
					if constexpr (and_then_num_args == 0) {
						auto out = std::make_shared<AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type>::type>::template AsyncJob<G, false, void>>(func, this->_self.lock());
						out->set_self(std::dynamic_pointer_cast<AsyncJobBaseBase>(out));
						// return out;
						return std::dynamic_pointer_cast<AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type>::type>>(out);
					}
					else {
						auto out = std::make_shared < AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>::template AsyncJob<G, false, return_t>>(func, this->_self.lock());
						out->set_self(std::dynamic_pointer_cast<AsyncJobBaseBase>(out));
						// return out;
						return std::dynamic_pointer_cast<AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type>::type>>(out);
					}
				};
				template <typename G> decltype(auto) and_then(size_t start, size_t end, G const& func) {
					static constexpr bool and_then_num_args = std::tuple_size_v<typename impl::function_traits<decltype(std::function(func))>::arguments>;
					if constexpr (and_then_num_args == 0) {
						auto out = std::make_shared < AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>::template AsyncJob<G, true, void>>(start, end, func, this->_self.lock());
						out->set_self(std::dynamic_pointer_cast<AsyncJobBaseBase>(out));
						//return out;
					    return std::dynamic_pointer_cast<AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>>(out);
					}
					else {
						auto out = std::make_shared < AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>::template AsyncJob<G, true, return_t>>(start, end, func, this->_self.lock());
						out->set_self(std::dynamic_pointer_cast<AsyncJobBaseBase>(out));
						//return out;
						return std::dynamic_pointer_cast<AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>>(out);
					}
				};
			};
			template<typename G> decltype(auto) async(G const& func) {
				auto out = std::make_shared < AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>::template AsyncJob<G, false, void>>(func, nullptr);
				out->set_self(std::dynamic_pointer_cast<AsyncJobBaseBase>(out));
				//return out;
				return std::dynamic_pointer_cast<AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>>(out);
			};
			template<typename G> decltype(auto) async(size_t start, size_t end, G const& func) {
				auto out = std::make_shared < AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>::template AsyncJob<G, true, void>>(start, end, func, nullptr);
				out->set_self(std::dynamic_pointer_cast<AsyncJobBaseBase>(out));
				//return out;
				return std::dynamic_pointer_cast<AsyncJobBase<typename GL::type_erasure::get_type< typename impl::function_traits<decltype(std::function(func))>::result_type >::type>>(out);
			};
		};







		namespace impl {
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
				auto out = std::make_shared<impl::jobs<F, Parent>>(start, end, std::move(ToDo), std::move(parent));
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
			GL::any get_any() const noexcept {
				if (_state) {
					_state->wait();
					return _state->result;
				}
				return {};
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

			template<typename G>
			__forceinline decltype(auto) and_then(G const& func) {
				using function_t = impl::function_traits<decltype(std::function(func))>;				
				return future<typename function_t::result_type>(impl::task([_old_state = _state]() {
					_old_state->wait();
					if constexpr (!std::is_void_v<T>) {
						if constexpr (std::is_same_v<GL::any, std::decay_t<T>> || std::is_same_v<GL::atomic_any, std::decay_t<T>>) {
							return _old_state->result;
						}
						else {
							return _old_state->result.cast<GL::shared_ptr<T>>();
						}
					}
				})->and_then(func));
			};

			template<typename G>
			__forceinline decltype(auto) and_then(size_t start, size_t end, G const& func) {
				using function_t = impl::function_traits<decltype(std::function(func))>;
				return future<typename function_t::result_type>(impl::task([_old_state = _state]() {
					_old_state->wait();
					if constexpr (!std::is_void_v<T>) {
						if constexpr (std::is_same_v<GL::any, std::decay_t<T>> || std::is_same_v<GL::atomic_any, std::decay_t<T>>) {
							return _old_state->result;
						}
						else {
							return _old_state->result.cast<GL::shared_ptr<T>>();
						}
					}
				})->and_then(start, end, func));
			};
		};




		/* performs the job on the single-threaded job queue, intended for use as part of the job system for the GPU tasks. */
		template<typename G> decltype(auto) async(G const& func) {
			using function_t = impl::function_traits<decltype(std::function(func))>;
			return future<typename function_t::result_type>(impl::task(func));
		};

		/* performs the job on the single-threaded job queue, intended for use as part of the job system for the GPU tasks. */
		template<typename G> decltype(auto) async(size_t start, size_t end, G const& func) {
			using function_t = impl::function_traits<decltype(std::function(func))>;
			return future<typename function_t::result_type>(impl::task(start, end, func));
		};
	};

	namespace parallel {
		namespace single_threaded {
			class job_base {
			public:
				job_base() = default;
				job_base(job_base const&) = delete;
				job_base(job_base&&) = default;
				job_base& operator=(job_base const&) = delete;
				job_base& operator=(job_base&&) = default;
				virtual ~job_base() = default;

				GL::any result;

				// Returns true if the job was dispatched. Returns false if the job had already been dispatched previously. 
				virtual bool dispatch() = 0;
				// Will wait until job is and may help with parallel computing. If an exception is caught during the async operation, it will be re-thrown at this time. 
				virtual job_base& wait() = 0;
				// returns a pointer to the parent of this job, if one exists. Can be used to get the results of past jobs. 
				virtual job_base* parent_ptr() const = 0;

				template<typename F, typename Parent> friend class GL::parallel::impl::job;
				template<typename F, typename Parent> friend class GL::parallel::impl::jobs;
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

			/* parallel_for (auto i = start; i < end; i++){ todo(i); } */
			template<typename iteratorType, class F> void For(iteratorType start, iteratorType end, F const& ToDo) {
				if (end >= start) {
					using f_t = GL::parallel::impl::function_traits<decltype(std::function(ToDo))>;
					impl::dispatch_context ctx{ 0, nullptr, nullptr };
					struct IterData {
						const F& _to_do;
						const iteratorType& _start;
						const iteratorType& _end;
						static void DoTask(impl::job_argument const& _args) {
							const IterData& data = *static_cast<const IterData*>(_args.task_memory);
							for (iteratorType i = data._start; i < data._end; ++i)
								if constexpr (std::tuple_size_v<f_t::arguments> == 0)
									(void)data._to_do();
								else
									(void)data._to_do(i);
						};
					} data{ ToDo, start, end };
					impl::Dispatch(ctx, 1, &IterData::DoTask, static_cast<void*>(&data));
					impl::Wait(ctx);
				}
				else {
					For(end, start, ToDo);
				}
			};

			/* parallel_for (auto i = start; i < end; i++){ todo(i); }
			If the todo(i) returns anything, it will be collected into a vector at the end. */
			template<typename iteratorType, class F> void For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
				using f_t = GL::parallel::impl::function_traits<decltype(std::function(ToDo))>;
				impl::dispatch_context ctx{ 0, nullptr, nullptr };
				struct IterData {
					const F& _to_do;
					const iteratorType& _start;
					const iteratorType& _end;
					const iteratorType& _step;

					static void DoTask(impl::job_argument const& _args) {
						const IterData& data = *static_cast<const IterData*>(_args.task_memory);
						for (iteratorType i = data._start; i < data._end; i += data._step)
							if constexpr (std::tuple_size_v<f_t::arguments> == 0)
								(void)data._to_do();
							else
								(void)data._to_do(i);
					};
				} data{ ToDo, start, end, step };
				impl::Dispatch(ctx, 1, &IterData::DoTask, static_cast<void*>(&data));
				impl::Wait(ctx);
			};

			/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
			If the todo(*i) returns anything, it will be collected into a vector at the end. */
			template<typename containerType, typename F> void For_Each(containerType& container, F const& ToDo) {
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
			template<typename containerType, typename F> void For_Each(containerType const& container, F const& ToDo) {
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
				impl::dispatch_context ctx{ 0, nullptr, nullptr };
				struct IterData {
					const F& _while;
					const G& _to_do;

					static void DoTask(impl::job_argument const& _args) {
						const IterData& data = *static_cast<const IterData*>(_args.task_memory);
						while (data._while()) {
							data._to_do();
						}
					};
				} data{ WhileBoolean, Do };
				impl::Dispatch(ctx, 1, &IterData::DoTask, static_cast<void*>(&data));
				impl::Wait(ctx);
			};

			/* while (true){ Do(); if (Until()){ return; } } */
			template<typename F, typename G> decltype(auto) Until(F const& Do, G const& UntilBoolean) {
				impl::dispatch_context ctx{ 0, nullptr, nullptr };
				struct IterData {
					const F& _to_do;
					const G& _until;

					static void DoTask(impl::job_argument const& _args) {
						const IterData& data = *static_cast<const IterData*>(_args.task_memory);
						do {
							data._to_do();
						} while (data._until());
					};
				} data{ Do, UntilBoolean };
				impl::Dispatch(ctx, 1, &IterData::DoTask, static_cast<void*>(&data));
				impl::Wait(ctx);
			};

			/* for (int i = 0; i < numToDispatch; i++){ ToDo(i, SharedObject); } return SharedObject; */
			template<typename F, typename G> F Dispatch(size_t numToDispatch, F&& SharedObject, G const& ToDo) {
				F out{ std::forward<F>(SharedObject) };
				struct IterData {
					const G& _to_do;
					F& _obj;
					size_t numToDispatch;

					static void DoTask(impl::job_argument const& _args) {
						const IterData& data = *static_cast<const IterData*>(_args.task_memory);
						for (size_t i = 0; i < data.numToDispatch; ++i)
							data._to_do(i, data._obj);
					};
				} data{ ToDo, out, numToDispatch };
				impl::dispatch_context ctx{ 0, nullptr, nullptr };
				impl::Dispatch(
					ctx,
					1,
					&IterData::DoTask,
					reinterpret_cast<void*>(&data)
				);
				impl::Wait(ctx);
				return out;
			};

			/* for (int i = 0; i < numToDispatch; i++){ ToDo(i, SharedObject); } return SharedObject; */
			template<typename F, typename G> F& Dispatch(size_t numToDispatch, F& SharedObject, G const& ToDo) {
				struct IterData {
					const G& _to_do;
					F& _obj;
					size_t numToDispatch;

					static void DoTask(impl::job_argument const& _args) {
						const IterData& data = *static_cast<const IterData*>(_args.task_memory);
						for (size_t i = 0; i < data.numToDispatch; ++i)
							data._to_do(i, data._obj);
					};
				} data{ ToDo, SharedObject, numToDispatch };
				impl::dispatch_context ctx{ 0, nullptr, nullptr };
				impl::Dispatch(
					ctx,
					1,
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

			namespace impl {
				template<typename F, typename Parent>
				class job final : public job_base {
					friend class job_base;
					template<typename G, typename ParentB> friend class job;
					template<typename G, typename ParentB> friend class jobs;
				public:
					using functionType = GL::parallel::impl::function_traits<decltype(std::function(std::declval<F>()))>;
					using returnType = typename functionType::result_type;
					static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
					static constexpr size_t this_num_args = std::tuple_size_v<typename functionType::arguments>;
					static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

				public:
					std::weak_ptr<job> self;
					GL::atomic_vector< std::weak_ptr<job_base> > children;

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
							static constexpr bool arg_is_jobBase = std::is_same_v<std::decay_t<arg>, job_base>;
							static constexpr bool arg_is_any =
								std::is_same_v<std::decay_t<arg>, GL::any> ||
								std::is_same_v<std::decay_t <arg>, GL::atomic_any>;

							job* data = reinterpret_cast<job*>(_args.task_memory);
							if constexpr (!this_is_job_start) {
								static constexpr bool returnVal_is_any =
									std::is_same_v<std::decay_t<typename Parent::returnType>, GL::any> ||
									std::is_same_v<std::decay_t<typename Parent::returnType>, GL::atomic_any>;

								if constexpr (arg_is_any) {
									return data->parent->result;
								}
								else if constexpr (arg_is_jobBase) {
									auto p = std::static_pointer_cast<job_base>(data->parent);
									return *p;
								}
								else {
									if constexpr (returnVal_is_any) {
										if constexpr (std::is_convertible_v<typename Parent::returnType, arg>) {
											return data->parent->result.cast<typename Parent::returnType>();
										}
										else {
											return data->parent->result.cast<arg>();
										}
									}
									else {
										return data->parent->result.cast<typename Parent::returnType>();
									}
								}
							}
							else {
								static_assert(false, "Job starts cannot evaluate arguments using this path");
							}
						}
						else {
							static_assert(false, "Wrong argument index in evaluation of argument in task");
						}
					};

					static void DoTask(impl::job_argument const& _args) {
						job* data = reinterpret_cast<job*>(_args.task_memory);

						if constexpr (this_is_job_start || (this_num_args == 0)) {
							if constexpr (this_returns_void) data->todo();
							else data->result = GL::any::instance(data->todo());
						}
						else {
							if constexpr (this_num_args == 1) {
								if constexpr (this_returns_void) data->todo(Argument<0>(_args));
								else data->result = GL::any::instance(data->todo(Argument<0>(_args)));
							}
							else if constexpr (this_num_args == 2) {
								if constexpr (this_returns_void) data->todo(Argument<0>(_args), Argument<1>(_args));
								else data->result = GL::any::instance(data->todo(Argument<0>(_args), Argument<1>(_args)));
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
					using functionType = GL::parallel::impl::function_traits<decltype(std::function(std::declval<F>()))>;
					using returnType = typename functionType::result_type;
					static constexpr bool this_returns_void = std::is_same_v<returnType, void>;
					static constexpr size_t this_num_args = std::tuple_size_v<typename functionType::arguments>;
					static constexpr bool this_is_job_start = std::is_same_v<void, Parent>;

				public:
					std::weak_ptr<jobs> self;
					GL::atomic_vector< std::weak_ptr<job_base> > children;

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
							static constexpr bool arg_is_jobBase = std::is_same_v<std::decay_t<arg>, job_base>;
							static constexpr bool arg_is_any = 
								std::is_same_v<std::decay_t<arg>, GL::any> || 
								std::is_same_v<std::decay_t <arg>, GL::atomic_any>;

							jobs* data = reinterpret_cast<jobs*>(_args.task_memory);
							if constexpr (!this_is_job_start) {
								static constexpr bool returnVal_is_any = 
									std::is_same_v<std::decay_t<typename Parent::returnType>, GL::any> || 
									std::is_same_v<std::decay_t<typename Parent::returnType>, GL::atomic_any>;

								if constexpr (arg_is_any) {
									return data->parent->result;
								}
								else if constexpr (arg_is_jobBase) {
									auto p = std::static_pointer_cast<job_base>(data->parent);
									return *p;
								}
								else {
									if constexpr (returnVal_is_any) {
										if constexpr (std::is_convertible_v<typename Parent::returnType, arg>) {
											return data->parent->result.cast<typename Parent::returnType>();
										}
										else {
											return data->parent->result.cast<arg>();
										}
									}
									else {
										return data->parent->result.cast<typename Parent::returnType>();
									}
								}
							}
							else {
								static_assert(false, "Job starts cannot evaluate arguments using this path");
							}
						}
						else {
							static_assert(false, "Wrong argument index in evaluation of argument in task");
						}
					};

					static void DoTask(impl::job_argument const& _args) {
						if constexpr (this_num_args == 0) {
							if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo();
							else {
								if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(static_cast<jobs*>(_args.task_memory)->todo());
								else (void)static_cast<jobs*>(_args.task_memory)->todo();
							}
						}
						else {
							// first item must be the size_t
							if constexpr (!std::is_same_v<std::decay_t<typename std::tuple_element_t<0, typename functionType::arguments>>, size_t>) {
								if constexpr (!std::is_constructible_v<typename std::tuple_element_t<0, typename functionType::arguments>, size_t>) {
									static_assert("The [optional] first argument to a jobs task must be a size_t index or castable from a size_t index.");
								}
							}

							if constexpr (this_num_args == 1) {
								if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start);
								else {
									if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start));
									else (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start);
								}
							}
							else if constexpr (this_num_args == 2) {
								if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args));
								else {
									if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args)));
									else (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args));
								}
							}
							else if constexpr (this_num_args == 3) {
								if constexpr (this_returns_void) (void)static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args), Argument<2>(_args));
								else {
									if (_args.job_index == 0) static_cast<jobs*>(_args.task_memory)->result = GL::any::instance(
										static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args), Argument<2>(_args))
									);
									else (void)
										static_cast<jobs*>(_args.task_memory)->todo(_args.job_index + static_cast<jobs*>(_args.task_memory)->start, Argument<1>(_args), Argument<2>(_args));
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

			namespace impl {
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
					auto out = std::make_shared<impl::jobs<F, Parent>>(start, end, std::move(ToDo), std::move(parent));
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

				template <typename T>
				class single_threaded_job_impl {
				public:
					T gpu_task;
					using return_type = typename typename T::element_type::returnType;

					single_threaded_job_impl() = delete;
					single_threaded_job_impl(T const& job) : gpu_task(job) {
						static_assert(std::is_base_of_v<GL::parallel::single_threaded::job_base, typename T::element_type>, "GPU job must use the single-threaded task system");
					};
					single_threaded_job_impl(T&& job) : gpu_task(std::forward<T>(job)) {
						static_assert(std::is_base_of_v<GL::parallel::single_threaded::job_base, typename T::element_type>, "GPU job must use the single-threaded task system");
					};
					single_threaded_job_impl(single_threaded_job_impl const&) = default;
					single_threaded_job_impl(single_threaded_job_impl&&) noexcept = default;
					single_threaded_job_impl& operator=(single_threaded_job_impl const&) = default;
					single_threaded_job_impl& operator=(single_threaded_job_impl&&) noexcept = default;
					~single_threaded_job_impl() = default;


					/* Get the anticipated return type, without needing to wait for the result. */
					const auto& type() const {
						return GL::type_of<return_type>();
					};
					/* get a copy of the result of the task, waiting if necessary. */
					return_type get() const {
						gpu_task->wait();
						if constexpr (!std::is_void_v<return_type>) {
							return gpu_task->result.cast<return_type>();
						}
					};
					/* get a reference to the result of the task, waiting if necessary. Note: lifetime of return reference must not outlive the future<T> object. */
					decltype(auto) get_ref() const {
						gpu_task->wait();
						if constexpr (!std::is_void_v<return_type>) {
							return gpu_task->result.cast<return_type>();
						}
					};
					/* get a shared_pointer of the result of the task, waiting if necessary. */
					GL::shared_ptr<return_type> get_shared() const {
						gpu_task->wait();
						if constexpr (!std::is_void_v<return_type>) {
							return gpu_task->result.cast<GL::shared_ptr<return_type>>();
						}
					};
					/* Get the results */
					GL::any get_any() const {
						gpu_task->wait();
						if constexpr (!std::is_void_v<return_type>) {
							return gpu_task->result;
						}
					};
					/* Ensure the job is completed */
					void wait() const {
						gpu_task->wait();
					};

					template<typename G> decltype(auto) and_then(G&& ToDo) const {
						return single_threaded_job_impl<decltype(gpu_task->and_then(ToDo))>(gpu_task->and_then(ToDo));
					};
					template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo) const {
						return single_threaded_job_impl<decltype(gpu_task->and_then(start, end, ToDo))>(gpu_task->and_then(start, end, ToDo));
					};
				};
			};
			
			/* Async task with known return type, which can be used to wait on and get the results of any job. Can be safely shared if multiple places will need access to the result once available. */
			template <typename return_t> class future {
			protected:
				template <typename G> friend class future;
				static decltype(auto) function_return() {
					if constexpr (std::is_void_v<return_t>) {
						return;
					}
					else {
						return GL::any();
					}
				};
				static decltype(auto) determine_type() {
					if constexpr (std::is_void_v<return_t>) {
						return;
					}
					else {
						static return_t temp;
						////if constexpr (std::is_pod_v< return_t>) {                
						////    return temp;
						////}
						////else {
						return std::ref(temp).get();
						////}
					}
				};

			public:
				using return_type = typename decltype(GL::detail::function_signature(&determine_type))::Return_Type;
				
			protected:
				using wrap_type = typename decltype(GL::detail::function_signature(&function_return))::Return_Type;
				std::function<wrap_type(void)>
					gpu_task; // type-erasure for the task system. 

			public:
				future() = delete;
				template<typename G> future(G&& job) : gpu_task([Job = impl::single_threaded_job_impl(std::forward<G>(job))]() -> decltype(auto) {
					if constexpr (std::is_void_v<return_type>) { Job.wait(); }
					else { return Job.get_any(); }
				}) {};
				future(future const&) = default;
				future(future&&) noexcept = default;
				future& operator=(future const&) = default;
				future& operator=(future&&) noexcept = default;
				~future() = default;

				/* Get the anticipated return type, without needing to wait for the result. */
				static const auto& type()  {
					return GL::type_of<return_type>();
				};
				/* get a copy of the result of the task, waiting if necessary. */
				return_t get() const {
					if constexpr (std::is_void_v<return_type>) { gpu_task(); }
					else { return gpu_task().cast<return_type>(); }
				};
				/* get a reference to the result of the task, waiting if necessary. Note: lifetime of return reference must not outlive the future<T> object. */
				decltype(auto) get_ref() const {
					if constexpr (std::is_void_v<return_type>) { gpu_task(); }
					else { return gpu_task().cast<return_type>(); }
				};
				/* get a shared_pointer of the result of the task, waiting if necessary. */
				GL::shared_ptr<return_type> get_shared() const {
					if constexpr (std::is_void_v<return_type>) { gpu_task(); }
					else { return gpu_task().cast<GL::shared_ptr<return_type>>(); }
				};
				/* Get the results */
				GL::any get_any() const {
					if constexpr (std::is_void_v<return_type>) { gpu_task(); }
					else { return gpu_task(); }
				};
				/* return a new future who can take as input the return of this future */
				template<typename G> decltype(auto) and_then(G&& ToDo) const {
					auto out = impl::single_threaded_job_impl(GL::parallel::single_threaded::impl::task([task = gpu_task]() {
						if constexpr (std::is_void_v<return_type>) { const_cast<std::function<wrap_type(void)>&>(task)(); }
						else { return const_cast<std::function<wrap_type(void)>&>(task)(); }
					})).and_then(ToDo);
					return future<typename decltype(out)::return_type>(std::move(out));
				};
				/* return a new future who can take as input the return of this future */
				template<typename G> decltype(auto) and_then(size_t start, size_t end, G&& ToDo) const {
					auto out = impl::single_threaded_job_impl(GL::parallel::single_threaded::impl::task([task = gpu_task]() {
						if constexpr (std::is_void_v<return_type>) { const_cast<std::function<wrap_type(void)>&>(task)(); }
						else { return const_cast<std::function<wrap_type(void)>&>(task)(); }
					})).and_then(start, end, ToDo);
					return future<typename decltype(out)::return_type>(std::move(out));
				};
			};

			/* performs the job on the single-threaded job queue, intended for use as part of the job system for the GPU tasks. */
			template<typename G> decltype(auto) async(G const& func) {
				impl::single_threaded_job_impl out(GL::parallel::single_threaded::impl::task(func));
				return future<typename decltype(out)::return_type>(std::move(out));
			};

			/* performs the job on the single-threaded job queue, intended for use as part of the job system for the GPU tasks. */
			template<typename G> decltype(auto) async(size_t start, size_t end, G const& func) {
				impl::single_threaded_job_impl out(GL::parallel::single_threaded::impl::task(start, end, func));
				return future<typename decltype(out)::return_type>(std::move(out));
			};
		};

	};
};