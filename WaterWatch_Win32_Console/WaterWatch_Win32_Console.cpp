/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#include "WaterWatch_Win32_Console.h"

#include "../FiberTasks/Fibers.h"
#include "../FiberTasks/TaskScheduler.h"
#include "../FiberTasks/WaitGroup.h"



namespace testing {
	namespace utilities {
		class typenames {
		public:
			template<typename T>
			struct identity { typedef T type; };

			class detail {
			public:
				using type_name_prober = void;
				template <typename T> static constexpr std::string_view wrapped_type_name() {
#ifdef __clang__
					return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
					return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
					return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
				};
				static constexpr std::size_t wrapped_type_name_prefix_length() { return wrapped_type_name<type_name_prober>().find(sv_type_name<type_name_prober>()); };
				static constexpr std::size_t wrapped_type_name_suffix_length() { return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - sv_type_name<type_name_prober>().length(); };
			};

			template <typename T>
			static constexpr std::string_view sv_type_name() {
				return sv_type_name(identity<T>());
			};

			template <typename T>
			static constexpr const char* type_name() {
				return type_name(identity<T>());
			};

		private:
			template <typename T>
			static constexpr std::string_view sv_type_name(identity<T>)
			{
				constexpr std::string_view wrapped_name = utilities::typenames::detail::wrapped_type_name<T>();
				constexpr auto prefix_length = utilities::typenames::detail::wrapped_type_name_prefix_length();
				constexpr auto suffix_length = utilities::typenames::detail::wrapped_type_name_suffix_length();
				constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
				return wrapped_name.substr(prefix_length, type_name_length);
			};

			template <typename T>
			static constexpr const char* type_name(identity<T>)
			{
				constexpr std::string_view wrapped_name = utilities::typenames::detail::wrapped_type_name<T>();
				constexpr auto prefix_length = utilities::typenames::detail::wrapped_type_name_prefix_length();
				constexpr auto suffix_length = utilities::typenames::detail::wrapped_type_name_suffix_length();
				constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
				return wrapped_name.substr(prefix_length, type_name_length).data();
			};

			static constexpr std::string_view sv_type_name(identity<void>) { return "void"; };
			static constexpr const char* type_name(identity<void>) { return "void"; };
		};
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };
	};
	namespace {
		template<class T> struct get_type { using type = T; };
		template<class T> struct get_type<std::shared_ptr<T>> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<std::shared_ptr<T>&> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<std::shared_ptr<T>*> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>&> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>*> { using type = typename get_type<T>::type; };
	};

	template <typename F = void()> class Function { 
	public:
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; static constexpr const size_t num_arguments = 0; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; static constexpr const size_t num_arguments = 0; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; static constexpr const size_t num_arguments = sizeof...(Args); };

		using Type = typename F;
		using ResultType = typename function_traits<std::function<Type>>::result_type;
		using Arguments = typename function_traits<std::function<Type>>::arguments;
		static constexpr const size_t NumArguments = function_traits<std::function<Type>>::num_arguments;
	
	private:
		template<int N, bool badIndex> struct function_destination_impl {
			using type = typename std::tuple_element<N, typename Arguments>::type;
		};
		template<int N> struct function_destination_impl<N, true> {
			using type = int;
		};

		template<int N> struct ArgumentType {
			static constexpr bool is_bad_index = N >= NumArguments;
			
			struct function_destination {
				using type = typename function_destination_impl<N, is_bad_index>::type;
				using underlying_type = typename testing::get_type<typename type>::type;

				static constexpr bool is_shared_ptr = !std::is_same<typename testing::get_type<type>::type, type>::value;
				static constexpr bool is_reference = std::is_reference<underlying_type>::value;
				static constexpr bool is_pointer = std::is_pointer<underlying_type>::value;
				static constexpr bool is_const = std::is_const<typename std::remove_pointer<typename std::remove_reference<underlying_type>::type>::type>::value;
			};
			struct parameter_pack {
			private:
				template<bool is_shared_ptr> struct underlying_type_impl {
					using type = typename std::remove_pointer<typename std::decay<typename function_destination::underlying_type>::type>::type;
					using package_type = type;
				};
				template<> struct underlying_type_impl<true> {
					using type = typename function_destination::underlying_type;
					using package_type = std::shared_ptr<type>;
				};

			public:
				using underlying_type = typename underlying_type_impl<function_destination::is_shared_ptr>::type;
				using shared_ptr_type = std::shared_ptr<underlying_type>;
				using unique_ptr_type = std::unique_ptr<underlying_type>;
				using type = typename underlying_type_impl<function_destination::is_shared_ptr>::package_type;

				static constexpr bool is_trivial = std::is_trivial<underlying_type>::value;
				static constexpr bool is_move_constructible = std::is_move_constructible<underlying_type>::value;
				static constexpr bool is_copy_constructible = std::is_copy_constructible<underlying_type>::value;
			};
		};
#define parameterPackFoundation std::tuple
#define getParam(N, O) std::get<N>(O)
//#define parameterPackFoundation cweeUnion
//#define getParam(N, O) O.get<N>()
		template <int N> struct ParameterPackImpl {
			using type = parameterPackFoundation<>;
		};
		template <> struct ParameterPackImpl<0> {
			using type = parameterPackFoundation<>;
		};
		template <> struct ParameterPackImpl<1> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<2> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<3> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<4> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<5> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<6> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<7> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<8> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<9> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<10> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<11> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<12> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<13> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<14> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type,
				typename ArgumentType<13>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<15> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type,
				typename ArgumentType<13>::parameter_pack::type,
				typename ArgumentType<14>::parameter_pack::type
			>;
		};
		template <> struct ParameterPackImpl<16> {
			using type = parameterPackFoundation<
				typename ArgumentType<0>::parameter_pack::type,
				typename ArgumentType<1>::parameter_pack::type,
				typename ArgumentType<2>::parameter_pack::type,
				typename ArgumentType<3>::parameter_pack::type,
				typename ArgumentType<4>::parameter_pack::type,
				typename ArgumentType<5>::parameter_pack::type,
				typename ArgumentType<6>::parameter_pack::type,
				typename ArgumentType<7>::parameter_pack::type,
				typename ArgumentType<8>::parameter_pack::type,
				typename ArgumentType<9>::parameter_pack::type,
				typename ArgumentType<10>::parameter_pack::type,
				typename ArgumentType<11>::parameter_pack::type,
				typename ArgumentType<12>::parameter_pack::type,
				typename ArgumentType<13>::parameter_pack::type,
				typename ArgumentType<14>::parameter_pack::type,
				typename ArgumentType<15>::parameter_pack::type
			>;
		};
#undef parameterPackFoundation
	public:
		std::function<typename F> function;
		typename ParameterPackImpl<NumArguments>::type parameter_pack;

		template<int N> auto& GetParameter() noexcept {
			return getParam(N, parameter_pack);
		};
		template<int N> const auto& GetParameter() const noexcept {
			return getParam(N, parameter_pack);
		};

	private:
		template <int N = 0> static void AddData(typename ParameterPackImpl<NumArguments>::type& d) {};
		template<int N = 0, typename T, typename... Targs> static void AddData(typename ParameterPackImpl<NumArguments>::type& d, T&& value, Targs && ... Fargs) {// recursive function		
			if constexpr (std::is_same<T, void>::value) {
				AddData<N + 1>(d, std::forward<Targs>(Fargs)...);
			}
			else {
				static constexpr bool desire_shared_ptr = !std::is_same<typename get_type<typename ArgumentType<N>::parameter_pack::type>::type, typename ArgumentType<N>::parameter_pack::type>::value;
				static constexpr bool got_shared_ptr = !std::is_same<typename get_type<T>::type, T>::value;

				if constexpr (desire_shared_ptr) {
					// we WANT a shared ptr. Did we get one? 
					if constexpr (got_shared_ptr) {
						getParam(N, d) = std::forward<T>(value);
					}
					else {
						getParam(N, d) = std::make_shared<typename ArgumentType<N>::parameter_pack::underlying_type>(std::forward<T>(value));
					}
				}
				else {
					// we DO NOT want a shared ptr. Did we get one?
					if constexpr (got_shared_ptr) {
						getParam(N, d) = *value;
					}
					else {
						getParam(N, d) = std::forward<T>(value);
					}
				}

				if constexpr (N + 1 < NumArguments) {
					AddData<N + 1>(d, std::forward<Targs>(Fargs)...);
				}
			}
		};
#undef getParam

    public:
		template <typename... Args>
		Function(std::function<F>&& function, Args && ... Fargs) noexcept : function(std::forward<std::function<F>>(function)), parameter_pack() {
			AddData(parameter_pack, std::forward<Args>(Fargs)...);
		};
		ResultType operator()() {
			if constexpr (NumArguments == 16) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>(), GetParameter<13>(), GetParameter<14>(),
					GetParameter<15>()
				);
			}
			else if constexpr (NumArguments == 15) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>(), GetParameter<13>(), GetParameter<14>()
				);
			}
			else if constexpr (NumArguments == 14) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>(), GetParameter<13>()
				);
			}
			else if constexpr (NumArguments == 13) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>(),
					GetParameter<12>()
				);
			}
			else if constexpr (NumArguments == 12) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>(), GetParameter<11>()
				);
			}
			else if constexpr (NumArguments == 11) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>(), GetParameter<10>()
				);
			}
			else if constexpr (NumArguments == 10) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>(),
					GetParameter<9>()
				);
			}
			else if constexpr (NumArguments == 9) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>(), GetParameter<8>()
				);
			}
			else if constexpr (NumArguments == 8) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>(), GetParameter<7>()
				);
			}
			else if constexpr (NumArguments == 7) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>(),
					GetParameter<6>()
				);
			}
			else if constexpr (NumArguments == 6) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>(), GetParameter<5>()
				);
			}
			else if constexpr (NumArguments == 5) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>(), GetParameter<4>()
				);
			}
			else if constexpr (NumArguments == 4) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>(),
					GetParameter<3>()
				);
			}
			else if constexpr (NumArguments == 3) {
				return function(
					GetParameter<0>(), GetParameter<1>(), GetParameter<2>()
				);
			}
			else if constexpr (NumArguments == 2) {
				return function(
					GetParameter<0>(), GetParameter<1>()
				);
			}
			else if constexpr (NumArguments == 1) {
				return function(
					GetParameter<0>()
				);
			}
			else if constexpr (NumArguments == 0) {
				return function();
			}
			else {
			    throw(std::runtime_error("The number of arguments and number of parameters did not match"));
			}
		}

		static constexpr size_t NumInputs() noexcept { return NumArguments; };
		static constexpr bool ReturnsNothing() {
			static constexpr bool returnsNothing = std::is_same<ResultType, void>::value;
			return returnsNothing;
		};
		const char* FunctionName() const {
			return function.target_type().name();
		};

	};

	namespace {
		class Action_Interface {
		public:
			explicit Action_Interface() {};
			explicit Action_Interface(Action_Interface const&) = delete;
			explicit Action_Interface(Action_Interface&&) = delete;
			Action_Interface& operator=(Action_Interface const&) = delete;
			Action_Interface& operator=(Action_Interface&&) = delete;
			virtual ~Action_Interface() noexcept {};

			virtual boost::typeindex::type_info const& type() const noexcept = 0;
			virtual const char* typeName() const noexcept = 0;
			virtual std::shared_ptr<Action_Interface> clone() const noexcept = 0;
			virtual Any& Invoke() noexcept = 0;
			virtual Any& ForceInvoke() noexcept = 0;
			virtual const char* FunctionName() const noexcept = 0;
			virtual const Any& Result() const noexcept = 0;
			virtual bool IsFinished() const noexcept = 0;
			virtual bool ReturnsNothing() const noexcept = 0;
		};
		template<typename ValueType> class Action_Impl final : public Action_Interface {
		private:
			using ReturnType = typename Function<ValueType>::ResultType;
			static constexpr bool returnsNothing{ std::is_same<ReturnType, void>::value };

		public:
			explicit Action_Impl() = delete;
			explicit Action_Impl(Function<ValueType> const& f) noexcept : data(f), result(), running(false), finished(false) {};
			explicit Action_Impl(Function<ValueType>&& f) noexcept : data(std::forward<Function<ValueType>>(f)), result(), running(false), finished(false) {};
			virtual ~Action_Impl() noexcept {};

			virtual boost::typeindex::type_info const& type() const noexcept final {
				return boost::typeindex::type_id<Function<ValueType>>().type_info();
			};
			virtual const char* typeName() const noexcept final {
				return boost::typeindex::type_id<Function<ValueType>>().type_info().name();
			};
			virtual std::shared_ptr<Action_Interface> clone() const noexcept final {
				return std::static_pointer_cast<Action_Interface>(std::make_shared<Action_Impl<ValueType>>(data));
			};
			virtual Any& Invoke() noexcept final {
				if (finished.load()) return result;
				bool compare(false);
				if (running.compare_exchange_strong(compare, true)) {
					if constexpr (returnsNothing) {
						data();
					}
					else {
						result = Any(data());
					}
					finished.store(true);
					running.store(false);
				}
				else {
					while (running.load()) {}
				}
				return result;
			};
			virtual Any& ForceInvoke() noexcept final {
				if constexpr (returnsNothing) {
					data();
				}
				else {
					result = Any(data());
				}
				return result;
			};
			virtual const char* FunctionName() const noexcept final {
				return data.FunctionName();
			};
			virtual const Any& Result() const noexcept final {
				return result;
			};
			virtual bool IsFinished() const noexcept final {
				return finished.load();
			};
			virtual bool ReturnsNothing()  const noexcept final {
				return data.ReturnsNothing();
			};

			Function<ValueType> data;
			Any result;
			std::atomic<bool> running;
			std::atomic<bool> finished;
		};
	};

	/* Wrapper for Function<> which allows for sharing and capture of lambdas, functions, etc. that can be evaluated at later times/date/threads/fibers. e.g:
	. auto f = Action([](int i, double x)->int{ return i+x; }, 10, 0.0);
	. int value = f.Invoke().cast();
	. assert(value == 10); */
	class Action {
	public: // structors
		/*! Init */ Action() noexcept : content(BasePtr()) {};
		/*! Copy */ Action(const Action& other) noexcept : content(BasePtr()) { std::shared_ptr<Action_Interface> c = other.content; content = c->clone(); };
		/*! Perfect forwarding */ Action(Action&& other) noexcept : content(std::move(other.content)) {};
		/*! Data Assignment */ template<typename ValueType> explicit Action(Function<ValueType>&& value) : content(ToPtr<ValueType>(std::forward< Function<ValueType>>(value))) {};
		/*! Direct instantiation2 */ template <typename F, typename... Args> Action(F&& function, Args &&... Fargs) : Action(Function(std::function(std::forward<F>(function)), std::forward<Args>(Fargs)...)) {};
		~Action() = default;

	public: // modifiers
		/*! Swap Data */ Action& swap(Action& rhs) noexcept {
			std::shared_ptr<Action_Interface> c1 = this->content;
			std::shared_ptr<Action_Interface> c2 = rhs.content;

			auto copy1 = c1->clone();
			auto copy2 = c2->clone();

			rhs.content = copy1;
			content = copy2;

			return *this;
		}
		/*! Copy Data */ Action& operator=(const Action& rhs) noexcept { Action(rhs).swap(*this); return *this; };
		/* Perfect forwarding of ValueType */ template <class ValueType> Action& operator=(const Function<ValueType>& rhs) noexcept { Action(rhs).swap(*this); return *this; };

	public: // queries
		explicit operator bool() { return !IsEmpty(); };
		explicit operator bool() const { return !IsEmpty(); };

		/*! Checks if the Action has been assigned something */
		bool IsEmpty() const noexcept { decltype(auto) c = content; return !c; };

		/*! Empties the Action and frees the memory. */
		void Clear() noexcept { Action().swap(*this); };

		template <typename ValueT> static constexpr const char* TypeNameOf() { return utilities::typenames::type_name<ValueT>(); };
		template <typename ValueT> static const boost::typeindex::type_info& TypeOf() { return boost::typeindex::type_id<ValueT>().type_info(); };

		const char* TypeName() const noexcept { std::shared_ptr<Action_Interface> c = content; if (c) return c->typeName(); return utilities::typenames::type_name<void>(); };
		const boost::typeindex::type_info& Type() const noexcept { std::shared_ptr<Action_Interface> c = content; return c ? c->type() : boost::typeindex::type_id<void>().type_info(); };

	private:
		template <class ValueType> static std::shared_ptr<Action_Interface> ToPtr(const Function<ValueType>& rhs) noexcept { return std::static_pointer_cast<Action_Interface>(std::make_shared<Action_Impl<ValueType>>(rhs)); };
		template <class ValueType> static std::shared_ptr<Action_Interface> ToPtr(Function<ValueType>&& rhs) noexcept { return std::static_pointer_cast<Action_Interface>(std::make_shared<Action_Impl<ValueType>>(std::forward<Function<ValueType>>(rhs))); };
		static std::shared_ptr<Action_Interface> BasePtr() noexcept {
			static thread_local auto base_ptr{ ToPtr(Function(std::function([]() {}))) };
			return base_ptr;
		};

	public:
		std::shared_ptr<Action_Interface> content;

	public:
		Any* operator()() noexcept {
			return Invoke();
		};
		Any* Invoke() noexcept {
			std::shared_ptr<Action_Interface> c = content; if (c) { return &c->Invoke(); }
			return nullptr;
		};
		Any* ForceInvoke() noexcept {
			std::shared_ptr<Action_Interface> c = content; if (c) { return &c->ForceInvoke(); }
			return nullptr;
		};
		const char* FunctionName() const {
			std::shared_ptr<Action_Interface> c = content;
			if (c)
			{
				return c->FunctionName();
			}
			return "No Function";
		};
		bool     IsFinished() const {
			std::shared_ptr<Action_Interface> c = content;
			if (c)
			{
				return c->IsFinished();
			}
			return false;
		};
		bool     ReturnsNothing() const {
			std::shared_ptr<Action_Interface> c = content;
			if (c)
			{
				return c->ReturnsNothing();
			}
			return true;
		};
		const Any* Result() const { if (this) { std::shared_ptr<Action_Interface> c = content; if (c) { return &c->Result(); } } return nullptr; };
	};
};


int Example::ExampleF(int numTasks, int numSubTasks) {
	// need a MUCH faster and lighter-weight job / action tool.
	{
		printf("SpeedTest (New Functions): ");

		Stopwatch sw; sw.Start();
		for (int i = 0; i < 1000; i++) {
			static auto f1 = [](int const& i, int const& j = 0)->double {
				return j + i;
			};
			static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
				return *i;
			};
			static auto f3 = [](cweeStr& i)->cweeStr {
				return i;
			};

			auto x1 = testing::Function(std::function(f1), 2, 2);
			auto x2 = testing::Function(std::function(f1), 4, 0);
			auto x3 = testing::Function(std::function(f1), 8, 0);
			if (x1() == x2() && x2() != x3()) {}
			else {
				throw(std::runtime_error("Something went wrong 1"));
			}

			auto y1 = testing::Function(std::function(f2), cweeStr("TEST"));
			auto y2 = testing::Function(std::function(f2), std::make_shared<cweeStr>("TEST"));
			auto y3 = testing::Function(std::function(f2), std::make_shared<cweeStr>("TESTING"));
			if (y1() == y2() && y2() != y3()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			}

			auto w1 = testing::Function(std::function(f3), cweeStr("TEST"));
			auto w2 = testing::Function(std::function(f3), std::make_shared<cweeStr>("TEST"));
			auto w3 = testing::Function(std::function(f3), std::make_shared<cweeStr>("TESTING"));
			if (w1() == w2() && w2() != w3()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			}
		}

		cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
		std::cout << timePassed.ToString() << std::endl;
	}
	{
		printf("SpeedTest (Old Functions): ");

		Stopwatch sw; sw.Start();
		for (int i = 0; i < 1000; i++) {
			static auto f1 = [](int const& i, int const& j = 0)->double {
				return j + i;
			};
			static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
				return *i;
			};
			static auto f3 = [](cweeStr& i)->cweeStr {
				return i;
			};

			auto x1 = Function(std::function(f1), 2, 2);
			auto x2 = Function(std::function(f1), 4, 0);
			auto x3 = Function(std::function(f1), 8, 0);
			if (x1.ForceInvoke().cast<double>() == x2.ForceInvoke().cast<double>() && x2.ForceInvoke().cast<double>() != x3.ForceInvoke().cast<double>()) {}
			else {
				throw(std::runtime_error("Something went wrong 1"));
			};

			auto y1 = Function(std::function(f2), cweeStr("TEST"));
			auto y2 = Function(std::function(f2), std::make_shared<cweeStr>("TEST"));
			auto y3 = Function(std::function(f2), std::make_shared<cweeStr>("TESTING"));
			if (y1.ForceInvoke().cast<cweeStr>() == y2.ForceInvoke().cast<cweeStr>() && y2.ForceInvoke().cast<cweeStr>() != y3.ForceInvoke().cast<cweeStr>()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			};

			auto w1 = Function(std::function(f3), cweeStr("TEST"));
			auto w2 = Function(std::function(f3), std::make_shared<cweeStr>("TEST"));
			auto w3 = Function(std::function(f3), std::make_shared<cweeStr>("TESTING"));
			if (w1.ForceInvoke().cast<cweeStr>() == w2.ForceInvoke().cast<cweeStr>() && w2.ForceInvoke().cast<cweeStr>() != w3.ForceInvoke().cast<cweeStr>()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			};
		}

		cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
		std::cout << timePassed.ToString() << std::endl;
	}

	{
		printf("SpeedTest (New Actions): ");

		Stopwatch sw; sw.Start();
		for (int i = 0; i < 1000; i++) {
			static auto f1 = [](int const& i, int const& j = 0)->double {
				return j + i;
			};
			static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
				return *i;
			};
			static auto f3 = [](cweeStr& i)->cweeStr {
				return i;
			};

			auto x1 = testing::Action(f1, 2, 2);
			auto x2 = testing::Action(f1, 4, 0);
			auto x3 = testing::Action(f1, 8, 0);
			if (x1.ForceInvoke()->cast<double>() == x2.ForceInvoke()->cast<double>() && x2.ForceInvoke()->cast<double>() != x3.ForceInvoke()->cast<double>()) {}
			else {
				throw(std::runtime_error("Something went wrong 1"));
			};

			auto y1 = testing::Action(f2, cweeStr("TEST"));
			auto y2 = testing::Action(f2, std::make_shared<cweeStr>("TEST"));
			auto y3 = testing::Action(f2, std::make_shared<cweeStr>("TESTING"));
			if (y1.ForceInvoke()->cast<cweeStr>() == y2.ForceInvoke()->cast<cweeStr>() && y2.ForceInvoke()->cast<cweeStr>() != y3.ForceInvoke()->cast<cweeStr>()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			};

			auto w1 = testing::Action(f3, cweeStr("TEST"));
			auto w2 = testing::Action(f3, std::make_shared<cweeStr>("TEST"));
			auto w3 = testing::Action(f3, std::make_shared<cweeStr>("TESTING"));
			if (w1.ForceInvoke()->cast<cweeStr>() == w2.ForceInvoke()->cast<cweeStr>() && w2.ForceInvoke()->cast<cweeStr>() != w3.ForceInvoke()->cast<cweeStr>()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			};
		}

		cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
		std::cout << timePassed.ToString() << std::endl;
	}
	{
		printf("SpeedTest (Old Actions): ");

		Stopwatch sw; sw.Start();
		for (int i = 0; i < 1000; i++) {
			static auto f1 = [](int const& i, int const& j = 0)->double {
				return j + i;
			};
			static auto f2 = [](std::shared_ptr<cweeStr> i)->cweeStr {
				return *i;
			};
			static auto f3 = [](cweeStr& i)->cweeStr {
				return i;
			};

			auto x1 = Action(f1, 2, 2);
			auto x2 = Action(f1, 4, 0);
			auto x3 = Action(f1, 8, 0);
			if (x1.ForceInvoke()->cast<double>() == x2.ForceInvoke()->cast<double>() && x2.ForceInvoke()->cast<double>() != x3.ForceInvoke()->cast<double>()) {}
			else {
				throw(std::runtime_error("Something went wrong 1"));
			};

			auto y1 = Action(f2, cweeStr("TEST"));
			auto y2 = Action(f2, std::make_shared<cweeStr>("TEST"));
			auto y3 = Action(f2, std::make_shared<cweeStr>("TESTING"));
			if (y1.ForceInvoke()->cast<cweeStr>() == y2.ForceInvoke()->cast<cweeStr>() && y2.ForceInvoke()->cast<cweeStr>() != y3.ForceInvoke()->cast<cweeStr>()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			};

			auto w1 = Action(f3, cweeStr("TEST"));
			auto w2 = Action(f3, std::make_shared<cweeStr>("TEST"));
			auto w3 = Action(f3, std::make_shared<cweeStr>("TESTING"));
			if (w1.ForceInvoke()->cast<cweeStr>() == w2.ForceInvoke()->cast<cweeStr>() && w2.ForceInvoke()->cast<cweeStr>() != w3.ForceInvoke()->cast<cweeStr>()) {}
			else {
				throw(std::runtime_error("Something went wrong 2"));
			};
		}

		cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
		std::cout << timePassed.ToString() << std::endl;
	}









	{
		fibers::TaskScheduler scheduler;
		scheduler.Init();

		for (int j = 1; j < 10; j += 2) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();
				
				for (int i = 0; i < numLoops; i++) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				cweeBalancedPattern pat;
				Stopwatch sw; sw.Start();				

				fibers::parallel::For(0, numLoops, [&pat, &j](int i) {
					for (int k = 0; k < j; k++)
						pat.AddValue(i + k, i + k);
				});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers, Idealized) : ");
			{
				cweeBalancedPattern pat;

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0,j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->AddValue(i + k, i + k);
					}
				};

				Stopwatch sw; sw.Start();
				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (Pattern) ");
			std::cout << j;
			printf(" (Fibers, Likely) : ");
			{
				cweeBalancedPattern pat;

				Stopwatch sw; sw.Start();

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->AddValue(i + k, i + k);
					}
				};

				
				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}
			printf("\n");
		}
		for (int j = 1; j < 10; j += 2) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				std::atomic<int> count;
				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++)
						count.fetch_add(k);
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				std::atomic<int> count;
				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&count, &j](int i) {
					for (int k = 0; k < j; k++)
						count.fetch_add(k);
					});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers, Idealized) : ");
			{
				std::atomic<int> pat;

				std::vector< cweeUnion<int, decltype(pat)*> > data(numLoops, cweeUnion<int, decltype(pat)*>(j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& j = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<0>();
					auto& p = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<1>();

					for (k = 0; k < j; k++) {
						p->fetch_add(k);
					}
				};

				Stopwatch sw; sw.Start();
				for (int i = 0; i < numLoops; i++) {
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (atomic int) ");
			std::cout << j;
			printf(" (Fibers, Likely) : ");
			{
				std::atomic<int> pat;

				Stopwatch sw; sw.Start();

				std::vector< cweeUnion<int, decltype(pat)*> > data(numLoops, cweeUnion<int, decltype(pat)*>(j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& j = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<0>();
					auto& p = static_cast<cweeUnion<int, decltype(pat)*>*>(arg)->get<1>();

					for (k = 0; k < j; k++) {
						p->fetch_add(k);
					}
				};

				
				for (int i = 0; i < numLoops; i++) {
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("\n");
		}
		for (int j = 1; j < 10; j += 2) {
			int numLoops = 400 * j * j;

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (No Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				int i, k;
				for (i = 0; i < numLoops; i++) {
					for (k = 0; k < j; k++) {
						vec[i] = cweeStr(k);
					}
				}

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers) : ");
			{
				std::vector<cweeStr> vec(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();
				fibers::parallel::For(0, numLoops, [&vec, &j](int i) {
					for (int k = 0; k < j; k++) {
						vec[i] = cweeStr(k);
					}
					});

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers, Idealized) : ");
			{
				std::vector<cweeStr> pat(numLoops, cweeStr("TEST"));

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->operator[](i) = cweeStr(k);
					}
				};

				Stopwatch sw; sw.Start();
				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("SpeedTest (vector overwriting) ");
			std::cout << j;
			printf(" (Fibers, Likely) : ");
			{
				std::vector<cweeStr> pat(numLoops, cweeStr("TEST"));

				Stopwatch sw; sw.Start();

				std::vector< cweeUnion<int, int, decltype(pat)*> > data(numLoops, cweeUnion<int, int, decltype(pat)*>(0, j, &pat));
				fibers::WaitGroup wg(&scheduler);

				static auto todo = [](void* arg) {
					int k;
					auto& i = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<0>();
					auto& j = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<1>();
					auto& p = static_cast<cweeUnion<int, int, decltype(pat)*>*>(arg)->get<2>();

					for (k = 0; k < j; k++) {
						p->operator[](i) = cweeStr(k);
					}
				};

				for (int i = 0; i < numLoops; i++) {
					data[i].get<0>() = i;
					scheduler.AddTask({ todo, static_cast<void*>(&data[i]) }, &wg);
				}
				wg.Wait();

				cweeUnitValues::second timePassed = sw.Stop() / 1000000000.0;
				std::cout << timePassed.ToString() << std::endl;
			}

			printf("\n");
		}

	}







	printf("Job 1\n");
	if (true) {
		fibers::parallel::For(0, numTasks * numSubTasks, [](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {}
		});
	}
	printf("Job 2\n");
	if (true) {
		cweeBalancedPattern pat;
		fibers::JobGroup group;
		for (int i = 0; i < numTasks * numSubTasks; i++) {
			auto job = fibers::Job([&pat](int j) {
				pat.AddUniqueValue(j, j);
			}, (int)i);
			group.Queue(job);
		}
		group.Wait();

		int sizeIs = pat.GetNumValues();
		if (sizeIs == 0) throw(std::runtime_error("Something went wrong"));
	}
	printf("Job 3\n");
	if (true) {
		fibers::parallel::For(0, numTasks * numSubTasks * 2, [](int j) {
			Stopwatch sw;
			sw.Start();
			while (sw.Stop() < 1000) {}
		});
	}
	printf("Job 4\n");
	if (true) {
		fibers::JobGroup group;
		//fibers::ftl_wrapper::TaskScheduler scheduler;
		for (int i = 0; i < numTasks; i++) {
			auto job = fibers::Job([]() {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}
			});
			group.Queue(job);
			//scheduler.AddTask(job);
		}
		group.Wait();
		//scheduler.Wait();
	}
	printf("Job 5\n");
	if (true) {
		std::atomic<int> numJobsDone;
		fibers::parallel::For(0, numTasks, [&numSubTasks, &numJobsDone](int i) {
			fibers::parallel::For(0, numSubTasks, [&numJobsDone](int j) {
				Stopwatch sw;
				sw.Start();
				while (sw.Stop() < 1000) {}

				numJobsDone.fetch_add(1);
			});
		});
		if (numJobsDone.load() != numTasks * numSubTasks) {
			printf("Job 5 failed");
		}
	}
	printf("Job 6\n");
	if (true) {
		cweeBalancedPattern pat;
		fibers::parallel::For(0, numTasks, [&numSubTasks, &pat](int i) {
			fibers::parallel::For(0, numSubTasks, [&pat, &i, &numSubTasks](int j) {
				pat.AddUniqueValue(i*numSubTasks + j, i* numSubTasks + j);
			});
		});
		if (pat.GetNumValues() != numSubTasks * numTasks) {
			auto err = cweeStr::printf("Pattern had %i values instead of %i", pat.GetNumValues(), numSubTasks * numTasks);
			printf(err);
		}
	}
	printf("Job 7a\n");
	if (true) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		fibers::containers::vector<int> list;

		fibers::parallel::For(0, numTasks, [&counter, &list, numSubTasks](int i) {
			fibers::parallel::For(0, numSubTasks, [&counter, &list](int j) {
				list.push_back(
					counter->fetch_add(1)
				); // do work
			});
		});
	}
	printf("Job 7b\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));

		fibers::JobGroup group;
		for (int i = 0; i < numTasks; ++i){
			group.Queue(fibers::Job([&numSubTasks, &counter](int i) {
				fibers::JobGroup group;
				for (int j = 0; j < numSubTasks; ++j) {
					group.Queue(fibers::Job([&counter](int j) {
						counter->fetch_add(1);
					}, (int)j));
				}
				group.Wait();
			}, (int)i));
		};
		group.Wait();
	}
	printf("Job 7c\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		auto job = cweeJob([&counter, &numTasks, &numSubTasks]() {
			fibers::JobGroup group;
			for (int i = 0; i < numTasks; ++i) {
				group.Queue(fibers::Job([&numSubTasks, &counter](int i) {
					fibers::JobGroup group;
					for (int j = 0; j < numSubTasks; ++j) {
						group.Queue(fibers::Job([&counter](int j) {
							counter->fetch_add(1);
						}, (int)j));
					}
					group.Wait();
				}, (int)i));
			};
			group.Wait();
		});
		job.AsyncInvoke();
		job.Await();
	}
	printf("Job 7d\n");
	if (false) {
		std::shared_ptr<std::atomic<int>> counter(new std::atomic<int>(0));
		fibers::containers::vector<int> list;
		auto job = cweeJob([&counter, &numTasks, &numSubTasks, &list]() {
			fibers::parallel::For(0, numTasks, [&counter, &list, numSubTasks](int i) {
				fibers::parallel::For(0, numSubTasks, [&counter, &list](int j) {
					list.push_back(
						counter->fetch_add(1)
					); // do work
				});
			});
		});
		job.AsyncInvoke();
		job.Await();
		return counter->load();
	}

	auto x = fibers::parallel::async([](int x) { return 100.0f; }, 10).wait_get(); // returns 100.0f
	size_t t = fibers::parallel::For(0, numTasks * numSubTasks, [](int i) { return i; }).size();

	printf("Loop done\n");

	return t;
};












#include "../ExcelInterop/Wrapper.h"

class Win32ConsoleSupport {
public:
	static void clearLine() {
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen;
		DWORD written;

		GetConsoleScreenBufferInfo(console, &screen);

		COORD pos = { 0, screen.dwCursorPosition.Y };

		FillConsoleOutputCharacterA(
			console, ' ', screen.dwSize.X * 1, pos, &written
		);
		FillConsoleOutputAttribute(
			console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
			screen.dwSize.X * 1, pos, &written
		);
		SetConsoleCursorPosition(console, pos);
	};
};

/* Linux style terminal for multithread friendly processing. */
static auto GetUserInput() {
	return fibers::parallel::async([]() {
		std::cout << std::endl; // new line, since we'll be refreshing constantly on the current line, then using carriage return to reset the origin.
		cweeStr temp;
		while (1) {
			// if the user hasn't typed anything, keep thinking.                
			Win32ConsoleSupport::clearLine();
			Win32ConsoleSupport::clearLine();
			std::cout << "\r" << temp; // print the current text here, so that it doesn't appear to flash on screen.
			while (!_kbhit()) {
				cweeSysThreadTools::Sys_Yield();
			}
			unsigned char character = _getch(); // grab user key press. 	
			// bool shift_pressed = GetAsyncKeyState(VK_SHIFT);

			if (character == '\n' || character == '\r') { // new line
				std::cout << std::endl;
				break;
			}
			if (character == '\b') { // backspace
				temp = temp.Left(cweeMath::max(0, temp.Length() - 1)); // remove one character
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if ((int)character == 27) { // escape
				temp.Clear();
				Win32ConsoleSupport::clearLine();
				continue;
			}
			if (character == 0 || character == 0xE0) {
				character = _getch();

				if (character == 72) { // up arrow		
					continue;
				}
				if (character == 80) { // down arrow	
					continue;
				}
				if (character == 75) { // left arrow	
					continue;
				}
				if (character == 77) { // right arrow	
					continue;
				}
				continue; // If another not-programmed key was pressed, skip. (i.e. Home button, PGUP, END, PGDOWN)
			}

			temp += (char)character; // all other keys, record to temp repo
		}
		return temp;
	});
};
static cweeStr UserMustSelectFile(fileType_t fileType = fileType_t::ANY_EXT) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().wait_get();// Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return fileSystem->getDataFolder() + "\\" + files[(int)reply];
	}
	else {
		return fileSystem->getDataFolder() + "\\" + reply.BestMatch(files);
	}
};
static cweeStr UserMustSelectFile(cweeStr fileType) {
	AUTO files = fileSystem->listFilesWithExtension(fileSystem->getDataFolder(), fileType);
	std::cout << "Select file by name or number: (number) \"name\"\n";
	int n = 0;
	for (auto& x : files) {
		std::cout << cweeStr::printf("\t(%i) \"%s\"\n", n++, x.c_str());
	}
	std::cout << std::endl;
	cweeStr reply = GetUserInput().wait_get();//Await().cast();
	if (reply.IsNumeric() && (int)reply < files.Num()) {
		return files[(int)reply];
	}
	else {
		return reply.BestMatch(files);
	}
};

/* Parallel thread to occasionally look for and process toast messages. Sleeps most of the time and wakes up to check for toasts. */
#if 1
static Timer parallel_toast_manager = Timer(0.1, Action(std::function([](cweeStr& title, cweeStr& content) {
	while (cweeToasts->tryGetToast(title, content)) std::cout << cweeStr::printf("\n/* WaterWatch Toast: \t\"%s\": \t\"%s\" */\n\n", title.c_str(), content.c_str());
}), cweeStr(), cweeStr()));
#endif

// Handle async or scripted AppRequests. 
#if 1
static Timer AppLayerRequestProcessor = Timer(0.01, Action(std::function([]() {
	std::pair<
		int, // ID
		cweeUnion<
			cweeStr, // Function Name
			cweeThreadedList<cweeStr>, // Arguments (optional)
			cweeSharedPtr<cweeStr> // result ptr?
		>
	> request = AppLayerRequests->DequeueRequest();
	if (request.first >= 0) {
#pragma warning(disable : 4573)			// the usage of 'cweeStr::Hash' requires the compiler to capture 'this' but the current default capture mode does not allow it
		cweeStr result;
		cweeStr& funcName = request.second.get<0>();
		cweeThreadedList<cweeStr>& args = request.second.get<1>();
		switch (static_cast<size_t>(funcName.Hash())) {
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFile")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SelectFolder")):
			result = UserMustSelectFile();
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SavePassword")): // server, username, password
			if (args.Num() >= 3) fileSystem->saveWindowsPassword(args[0], args[1], args[2]);
			else result = "Arguments required: 'account_name', 'user_name', 'password'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_LoadPassword")):
			if (args.Num() >= 2) result = fileSystem->retrieveWindowsPassword(args[0], args[1]);
			else result = "Arguments required: 'account_name', 'user_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_ThemeColor")):
			if (args.Num() >= 1) result = "[255,255,255,255]";
			else result = "Arguments required: 'color_name'";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetUserName")):
			result = "Win32 Project";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetMousePosition")):
			result = "[0,0]";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetClipboard")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_SaveSetting")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("OS_GetSetting")):
			result = "TBD";
			break;
		case static_cast<size_t>(cweeStr::Hash("Fiber")): 
		    {
				if (args.Num() >= 2) result = std::to_string(Example::ExampleF(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
				//if (args.Num() >= 2) result = std::to_string(Example::ExampleF(args[0].ReturnNumeric(), args[1].ReturnNumeric())).c_str();
				else result = "Arguments required: 'num_tasks', 'num_subtasks'";
			}
			break;
		default:
			// unknown function.
			result = "Function Not Found";
			break;
		}
		AppLayerRequests->FinishRequest(request.first, result); // let the app set the result and end this dequeue
#pragma warning(default : 4573)	
	}
})));
#endif

static cweeStr GetHeaderString() {
	cweeStr toRet = "Welcome to the WaterWatch Sample App.\n";
	toRet += "Data Directory = " + fileSystem->getDataFolder() + "\n";
	toRet += "Begin Scripting:\n\n";
	return toRet;
}

#define AddFunctionToLib(lib, name, todo, ...){ \
	auto varNames = cweeStr(#__VA_ARGS__).RemoveBetween("<", ">").ReplaceInline("*", " ").ReplaceInline("&", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").ReplaceInline("  ", " ").Split(",").Trim(' ').ReplaceInline("  ", " ").SplitAgain(" ").Trim(' ').GetEveryOtherVar(); \
	lib->add(fun([](__VA_ARGS__) { todo; }, varNames), #name);	\
}

int main() {
	using namespace cwee_units;
	
	std::cout << GetHeaderString() << std::endl;

	cweeStr prevLine = "";
	cweeStr command = "";
	std::shared_ptr<chaiscript::WaterWatch_ChaiScript> engine = std::make_shared<chaiscript::WaterWatch_ChaiScript>();
	while (true) {
		AUTO input = GetUserInput();
		cweeStr str = input.wait_get();//Await().cast();

		if (str == "Fiber") {
			int x = 1000;
			int y = 10;
			int z;
			while (true) {
				z = Example::ExampleF(y, x);
				if (y * x != z) {
					cweeStr err = cweeStr::printf("Something went wrong with the job system and a job returned %i instead of %i", z, y * x);
					throw(std::runtime_error(err.c_str()));
				}


				auto job = cweeJob(Example::ExampleF, (int)y, (int)x);
				job.AsyncInvoke();
				job.Await();

			}
		}
		if (str == "Exit")
		{
			return 0;
		}
		if (str == "Reset") { 
			engine = std::make_shared<chaiscript::WaterWatch_ChaiScript>();
			command = ""; 
			continue; 
		} 
		if (str == "" && prevLine == "") {
			Stopwatch sw;
			sw.Start();

			cweeStr result;
			try {
				AUTO bv = engine->eval(command.c_str());
				result = engine->to_string(bv).c_str();
			} catch (std::exception e) {
				result = e.what();
			}

			sw.Stop();
			std::cout << "Time Elapsed; " << (second_t)sw.Seconds_Passed() << std::endl;
			std::cout << "Current DateTime: " << ((cweeTime)fileSystem->getCurrentTime()).c_str() << std::endl;

			std::cout << result << std::endl;

			command = "";
		}

		command += str;
		command += "\n";
		prevLine = str;
	}
}