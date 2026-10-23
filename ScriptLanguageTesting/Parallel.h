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
			struct alignas(64) job_argument {
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
				void (*task)(job_argument&);
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
				void (*Task)(job_argument&),
				void* task_memory,
				size_t sharedmemory_size,
				void (*group_start_job)(void* const&), // callback func with memory for type T
				void (*group_end_job)(void* const&) // callback func with memory for type T
			) noexcept;
			void Dispatch(
				dispatch_context& ctx,
				size_t jobCount,
				void (*Task)(job_argument&),
				void* task_memory
			) noexcept;
			void DispatchOnce(
				dispatch_context& ctx,
				void (*Task)(job_argument&),
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

						static void DoTask(impl::job_argument& _args) {
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

						static void DoTask(impl::job_argument& _args) {
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

				static void DoTask(impl::job_argument& _args) {
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

				static void DoTask(impl::job_argument& _args) {
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

				static void DoTask(impl::job_argument& _args) {
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

				static void DoTask(impl::job_argument& _args) {
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

				static void DoTask(impl::job_argument& _args) {
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
					parallel::impl::dispatch_context* ctx;
					void (*task)(job_argument&);
					size_t group_id;
					size_t group_job_offset;
					size_t group_job_end;
					size_t group_memory_size;
					void (*group_start_job)(void* const&); // callback func with memory for type T
					void (*group_end_job)(void* const&); // callback func with memory for type T
					void* task_memory;
				};

				void Dispatch(
					parallel::impl::dispatch_context& ctx,
					size_t jobCount,
					void (*Task)(job_argument&),
					void* task_memory,
					size_t sharedmemory_size,
					void (*group_start_job)(void* const&), // callback func with memory for type T
					void (*group_end_job)(void* const&) // callback func with memory for type T
				) noexcept;
				void Dispatch(
					parallel::impl::dispatch_context& ctx,
					size_t jobCount,
					void (*Task)(job_argument&),
					void* task_memory
				) noexcept;
				void DispatchOnce(
					parallel::impl::dispatch_context& ctx,
					void (*Task)(job_argument&),
					void* task_memory
				) noexcept;
				void Wait(parallel::impl::dispatch_context& ctx, bool rethrow = true);
			};

			/* parallel_for (auto i = start; i < end; i++){ todo(i); } */
			template<typename iteratorType, class F> void For(iteratorType start, iteratorType end, F const& ToDo) {
				if (end >= start) {
					using f_t = GL::parallel::impl::function_traits<decltype(std::function(ToDo))>;
					parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
					struct IterData {
						const F& _to_do;
						const iteratorType& _start;
						const iteratorType& _end;
						static void DoTask(impl::job_argument& _args) {
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
				parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
				struct IterData {
					const F& _to_do;
					const iteratorType& _start;
					const iteratorType& _end;
					const iteratorType& _step;

					static void DoTask(impl::job_argument& _args) {
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

					static void DoTask(impl::job_argument& _args) {
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

				parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
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

					static void DoTask(impl::job_argument& _args) {
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

				parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
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
				parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
				struct IterData {
					const F& _while;
					const G& _to_do;

					static void DoTask(impl::job_argument& _args) {
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
				parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
				struct IterData {
					const F& _to_do;
					const G& _until;

					static void DoTask(impl::job_argument& _args) {
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

					static void DoTask(impl::job_argument& _args) {
						const IterData& data = *static_cast<const IterData*>(_args.task_memory);
						for (size_t i = 0; i < data.numToDispatch; ++i)
							data._to_do(i, data._obj);
					};
				} data{ ToDo, out, numToDispatch };
				parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
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

					static void DoTask(impl::job_argument& _args) {
						const IterData& data = *static_cast<const IterData*>(_args.task_memory);
						for (size_t i = 0; i < data.numToDispatch; ++i)
							data._to_do(i, data._obj);
					};
				} data{ ToDo, SharedObject, numToDispatch };
				parallel::impl::dispatch_context ctx{ 0, nullptr, nullptr };
				impl::Dispatch(
					ctx,
					1,
					&IterData::DoTask,
					reinterpret_cast<void*>(&data)
				);
				impl::Wait(ctx);
				return SharedObject;
			};
		};

	};
};