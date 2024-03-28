#pragma once

#pragma region Precompiled STL Headers
#pragma warning(disable : 4005)				// macro redefinition
#pragma warning(disable : 4010)				// single-line comment contains line-continuation character
#pragma warning(disable : 4018)				// singed / unsigned mismatch
#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4101)				// unreferenced local variable
#pragma warning(disable : 4127)				// conditional expression is constant
#pragma warning(disable : 4172)				// returning address of local variable or temporary
#pragma warning(disable : 4189)				// local variable is initialized but not referenced
#pragma warning(disable : 4238)				// nonstandard extension used: class rvalue used as lvalue
#pragma warning(disable : 4239)				// conversion from 'T' to 'T&'
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4251)				// needs to have dll-interface
#pragma warning(disable : 4267)				// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4273)				// inconsistent DLL linkage
#pragma warning(disable : 4297)				// function assumed not to throw but does
#pragma warning(disable : 4302)				// truncation from 'void *' to 'int'
#pragma warning(disable : 4305)				// truncating a literal from double to float
#pragma warning(disable : 4311)				// pointer truncation from 'void *' to 'int'
#pragma warning(disable : 4312)				// conversion from 'int' to 'void*' of greater size
#pragma warning(disable : 4390)				// ';' empty controlled statement
#pragma warning(disable : 4456)				// declaration hides previous local declaration
#pragma warning(disable : 4458)				// hides class member
#pragma warning(disable : 4459)				// hides global declaration
#pragma warning(disable : 4499)				// 'static': an explicit specialization cannot have a storage class
#pragma warning(disable : 4505)				// unreferenced local function has been removed
#pragma warning(disable : 4595)				// non-member operator new or delete functions may not be declared inline
#pragma warning(disable : 4701)				// potentially uninitialized local variable
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4715)				// not all control paths return a value
#pragma warning(disable : 4996)				// unsafe string operations
#pragma warning(disable : 6011)				// Dereferencing NULL ptr
#pragma warning(disable : 6385)				// Reading invalid data from buf
#pragma warning(disable : 26110)			// Caller failing to hold lock
#pragma warning(disable : 26439)			// This kind of function may not throw
#pragma warning(disable : 26450)			// Arithmetic overflow: using '<<'
#pragma warning(disable : 26451)			// Arithmetic overflow: using '*' on a 4 byte variable and casting to 8 bytes
#pragma warning(disable : 26495)			// uninitialized member variable type 6
#pragma warning(disable : 26498)			// Mark function constexpr if compile-time evaluation is desired
#pragma warning(disable : 26812)			// prefer enum class to enum
#pragma warning(disable : 28182)			// Dereferencing NULL pointer
#pragma warning(disable : 28251)			// Inconsistent annotation for 'new'
#define NOMINMAX
#define _CRT_FUNCTIONS_REQUIRED 1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include <ShlDisp.h>
#include <mutex>
#include <shared_mutex>
#include <synchapi.h>
#include <handleapi.h>
#include <ppl.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_set.h>
#include <boost/any.hpp>
#pragma endregion
#pragma region iterator_definition
#ifdef SETUP_STL_ITERATOR
#else
#define SETUP_STL_ITERATOR(ParentClass, IterType, StateType) typedef std::ptrdiff_t difference_type;											\
	typedef size_t size_type; typedef IterType value_type; typedef IterType* pointer; typedef const IterType* const_pointer;					\
	typedef IterType& reference;																												\
	typedef const IterType& const_reference;																									\
	class iterator {					\
	public: const ParentClass* ref;	mutable StateType state;			\
		iterator() : ref(nullptr), state() {};																									\
		iterator(const ParentClass* parent) : ref(parent), state() {};																			\
		iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };									\
		iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };									\
		difference_type operator-(iterator const& other) { return state.distance(other.state); };												\
		iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };							\
		iterator& operator--() { state.prev(ref); return *this; };																				\
		iterator operator--(int) { iterator retval = *this; --(*this); return retval; };														\
		iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };							\
		iterator& operator++() { state.next(ref); return *this; };																				\
		iterator operator++(int) { iterator retval = *this; ++(*this); return retval; };														\
		bool operator==(iterator const& other) const { return !(operator!=(other)); };															\
		bool operator!=(iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };									\
		reference operator*() { return const_cast<reference>(state.get(ref)); };																\
		pointer operator->() { return const_cast<pointer>(&state.get(ref)); };																	\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		iterator& begin() { state.begin(ref); return *this; };																					\
		iterator& end() { state.end(ref); return *this; };																						\
	};																													\
	iterator begin() { return iterator(this).begin(); };																						\
	iterator end() { return iterator(this).end(); };																							\
	class const_iterator {	\
	public: const ParentClass* ref;	mutable StateType state;																					\
		const_iterator() : ref(nullptr), state() {};																							\
		const_iterator(const ParentClass* parent) : ref(parent), state() {};																	\
		const_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };							\
		const_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };							\
		difference_type operator-(const_iterator const& other) { return state.distance(other.state); };											\
		const_iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };						\
		const_iterator& operator--() { state.prev(ref); return *this; };																		\
		const_iterator operator--(int) { const_iterator retval = *this; --(*this); return retval; };											\
		const_iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };						\
		const_iterator& operator++() { state.next(ref); return *this; };																		\
		const_iterator operator++(int) { const_iterator retval = *this; ++(*this); return retval; };											\
		bool operator==(const_iterator const& other) const { return !(operator!=(other)); };													\
		bool operator!=(const_iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };							\
		const_reference operator*() { return const_cast<reference>(state.get(ref)); };															\
		const_pointer operator->() { return const_cast<pointer>(&state.get(ref)); };															\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		const_iterator& begin() { state.begin(ref); return *this; };																			\
		const_iterator& end() { state.end(ref); return *this; };																				\
	};																												\
	const_iterator cbegin() const { return const_iterator(this).begin(); };																		\
	const_iterator cend() const { return const_iterator(this).end(); };																			\
	const_iterator begin() const { return cbegin(); };																							\
	const_iterator end() const { return cend(); };																								\
	class reverse_iterator {			\
	public: const ParentClass* ref;	mutable StateType state;																					\
		reverse_iterator() : ref(nullptr), state() {};																							\
		reverse_iterator(const ParentClass* parent) : ref(parent), state() {};																	\
		reverse_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };							\
		reverse_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };							\
		difference_type operator-(reverse_iterator const& other) { return state.distance(other.state); };										\
		reverse_iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };					\
		reverse_iterator& operator--() { state.next(ref); return *this; };																		\
		reverse_iterator operator--(int) { reverse_iterator retval = *this; --(*this); return retval; };										\
		reverse_iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };					\
		reverse_iterator& operator++() { state.prev(ref); return *this; };																		\
		reverse_iterator operator++(int) { reverse_iterator retval = *this; ++(*this); return retval; };										\
		bool operator==(reverse_iterator const& other) const { return !(operator!=(other)); };													\
		bool operator!=(reverse_iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };							\
		reference operator*() { return const_cast<reference>(state.get(ref)); };																\
		pointer operator->() { return const_cast<pointer>(&state.get(ref)); };																	\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		reverse_iterator& begin() { state.end(ref); state.prev(ref); return *this; };															\
		reverse_iterator& end() { state.begin(ref); state.prev(ref); return *this; };															\
	};																											\
	reverse_iterator rbegin() { return reverse_iterator(this).begin(); };																		\
	reverse_iterator rend() { return reverse_iterator(this).end(); };																			\
	class const_reverse_iterator {	\
	public: const ParentClass* ref;	mutable StateType state;																					\
		const_reverse_iterator() : ref(nullptr), state() {};																					\
		const_reverse_iterator(const ParentClass* parent) : ref(parent), state() {};															\
		const_reverse_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };					\
		const_reverse_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };					\
		difference_type operator-(const_reverse_iterator const& other) { return state.distance(other.state); };									\
		const_reverse_iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };				\
		const_reverse_iterator& operator--() { state.next(ref); return *this; };																\
		const_reverse_iterator operator--(int) { const_reverse_iterator retval = *this; --(*this); return retval; };							\
		const_reverse_iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };				\
		const_reverse_iterator& operator++() { state.prev(ref); return *this; };																\
		const_reverse_iterator operator++(int) { const_reverse_iterator retval = *this; ++(*this); return retval; };							\
		bool operator==(const_reverse_iterator const& other) const { return !(operator!=(other)); };											\
		bool operator!=(const_reverse_iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };					\
		const_reference operator*() { return const_cast<reference>(state.get(ref)); };															\
		const_pointer operator->() { return const_cast<pointer>(&state.get(ref)); };															\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		const_reverse_iterator& begin() { state.end(ref); state.prev(ref); return *this; };														\
		const_reverse_iterator& end() { state.begin(ref); state.prev(ref); return *this; };														\
	};																										\
	const_reverse_iterator rbegin() const { return const_reverse_iterator(this).begin(); };														\
	const_reverse_iterator rend() const { return const_reverse_iterator(this).end(); };															\
	const_reverse_iterator crbegin() const { return rbegin(); };																				\
	const_reverse_iterator crend() const { return rend(); };
#endif
#pragma endregion 


#include <functional>
#include <memory>
#include <utility>

// Finally is a pure virtual base class, implemented by the templated FinallyImpl.
class Finally { public:
	virtual ~Finally() = default;
};

// FinallyImpl implements a Finally.
// The template parameter F is the function type to be called when the finally is destructed. F must have the signature void().
template <typename F>
class FinallyImpl : public Finally { public:
	inline FinallyImpl(const F& func_) : func(func_) {};
	inline FinallyImpl(F&& func_) : func(std::move(func_)) {};
	inline FinallyImpl(FinallyImpl<F>&& other) : func(std::move(other.func)) { other.valid = false; };
	inline ~FinallyImpl() { if (valid) { func(); } };

private:
	FinallyImpl(const FinallyImpl<F>& other) = delete;
	FinallyImpl<F>& operator=(const FinallyImpl<F>& other) = delete;
	FinallyImpl<F>& operator=(FinallyImpl<F>&&) = delete;
	F func;
	bool valid = true;
};
template <typename F> inline [[nodiscard]] FinallyImpl<F> make_finally(F&& f) { return FinallyImpl<F>(std::forward<F>(f)); };
template <typename F> inline [[nodiscard]] std::shared_ptr<Finally> make_shared_finally(F&& f) { return std::make_shared<FinallyImpl<F>>(std::forward<F>(f)); };

#define FINALLY_CONCAT_(a, b) a##b
#define FINALLY_CONCAT(a, b) FINALLY_CONCAT_(a, b)

// defer() is a macro to defer execution of a statement until the surrounding
// scope is closed and is typically used to perform cleanup logic once a
// function returns.
//
// Note: Unlike golang's defer(), the defer statement is executed when the
// surrounding *scope* is closed, not necessarily the function.
//
// Example usage:
//
//  void sayHelloWorld()
//  {
//      defer(printf("world\n"));
//      printf("hello ");
//  }
//
#define defer(x) \
  decltype(auto) FINALLY_CONCAT(defer_, __LINE__) { make_finally([&] { x; }) }

#define return_defered(x) \
  return make_shared_finally([=] { x; })

namespace fibers {
	namespace synchronization {
		/* Simple atomic spin-lock that fairly synchronizes threads or fibers based on a ticket-queue system. */
		class TicketSpinLock {
		private:
			std::atomic<unsigned long> ticket;
			std::atomic<unsigned long> serving;

		public:
			TicketSpinLock() : ticket(0), serving(0) {};
			TicketSpinLock(TicketSpinLock const&) = default;
			TicketSpinLock(TicketSpinLock&&) = default;
			TicketSpinLock& operator=(TicketSpinLock const&) = default;
			TicketSpinLock& operator=(TicketSpinLock&&) = default;
			~TicketSpinLock() = default;

			void lock() {
				auto my_ticket = ticket.fetch_add(1, std::memory_order::memory_order_relaxed);
				while (my_ticket != serving.load(std::memory_order::memory_order_acquire)) {
					::SwitchToThread();
				}
			};
			void unlock() {
				serving.fetch_add(1, std::memory_order::memory_order_release);
			};
		};

	};
	namespace containers {
		/* Generic container that does not instantiate an object until actually needed. Useful for seperating consumption from creation. Once made, the container is nearly "free" in terms of speed. */
		template<typename T > class DelayedInstantiation {
		public:
			DelayedInstantiation(std::shared_ptr<T> const& o) : obj(o), Lock(std::make_shared<synchronization::TicketSpinLock>()), createFunc([]()->T* { return new T(); }), deleteFunc([](T* p)->void { delete p; }) {};
			DelayedInstantiation() : obj(nullptr), Lock(std::make_shared<synchronization::TicketSpinLock>()), createFunc([]()->T* { return new T(); }), deleteFunc([](T* p)->void { delete p; }) {};
			DelayedInstantiation(std::function<T* ()> create) : obj(nullptr), Lock(std::make_shared<synchronization::TicketSpinLock>()), createFunc(create), deleteFunc([](T* p)->void { delete p; }) {};
			DelayedInstantiation(std::function<T* ()> create, std::function<void(T*)> destroyFunc) : obj(nullptr), Lock(std::make_shared<synchronization::TicketSpinLock>()), createFunc(create), deleteFunc(destroyFunc) {};
			~DelayedInstantiation() {};

			//T* operator->() { Ensure(); return obj.Get(); };
			T* operator->() { Ensure(); return obj.get(); };
			T& operator*() { Ensure(); return *obj; };

		private:
			void Ensure() {
				if (!obj.get()) {
					lock();
					auto* p = obj.get();
					if (!p) {
						obj = std::shared_ptr<T>(createFunc(), deleteFunc);
					}
					unlock();
				}
			};
			void lock() {
				Lock->lock();
			};
			void unlock() {
				Lock->unlock();
			};
			std::shared_ptr<T> obj;
			std::shared_ptr<synchronization::TicketSpinLock> Lock;
			std::function<T* ()> createFunc;
			std::function<void(T*)> deleteFunc;
		};

	};
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

		class AnyData {
		public:
			AnyData(std::shared_ptr<void> const& t_ptr, const boost::typeindex::type_info& t_type, const bool t_const) noexcept : m_ptr(t_ptr), m_type(t_type), m_const(t_const) {};
			AnyData(AnyData const&) = default;
			AnyData(AnyData&&) = default;
			AnyData& operator=(AnyData const&) = default;
			AnyData& operator=(AnyData&&) = default;
			~AnyData() = default;

		public:
			template<typename ToType> std::shared_ptr<ToType> cast() const { return std::static_pointer_cast<ToType>(m_ptr); };
			void* ptr() const { return m_ptr.get(); };

			template<typename T> static std::shared_ptr<void> get_data(const std::shared_ptr<T>& data) {
				if constexpr (std::is_const< T >::value) {
					return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(data));
				}
				else {
					return std::static_pointer_cast<void>(data);
				}
			};
			template<typename T> static std::shared_ptr<void> get_data(std::shared_ptr<T>&& data) {
				return std::static_pointer_cast<void>(std::forward<std::shared_ptr<T>>(data));
			};

		public:
			std::shared_ptr<void>					m_ptr; // underlying shared ptr for the provided object. (e.g. std::shared_ptr<int>, etc.)
			const boost::typeindex::type_info&      m_type; // type information of the saved object
			const bool						        m_const; // whether or not the saved object is const

		};
	}
	
	class FunctionBase {
	public:
		virtual ~FunctionBase() = default;
		virtual void Invoke() = 0;
	};

	/*
	Wrapper for std::function that can capture input parameters for later evaluation. e.g:
	. auto f = Function(std::function([](int i, double x)->int{ return i+x; }), 10, 0.0);
	. int value = f.Invoke().cast();
	. assert(value == 10);
	*/
	template <typename F = void()> class Function final : public FunctionBase {
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
				using underlying_type = typename get_type<typename type>::type;

				static constexpr bool is_shared_ptr = !std::is_same<typename get_type<type>::type, type>::value;
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
		Function(std::function<F>&& function, Args && ... Fargs) noexcept : FunctionBase(), function(std::forward<std::function<F>>(function)), parameter_pack() {
			AddData(parameter_pack, std::forward<Args>(Fargs)...);
		};
		Function() = default;
		Function(Function const&) = default;
		Function(Function&&) = default;
		Function& operator=(Function const&) = default;
		Function& operator=(Function&&) = default;
		~Function() = default;

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
		void Invoke() override final { operator()(); };

		static constexpr size_t NumInputs() noexcept { return NumArguments; };
		static constexpr bool ReturnsNothing() {
			static constexpr bool returnsNothing = std::is_same<ResultType, void>::value;
			return returnsNothing;
		};
		const char* FunctionName() const {
			return function.target_type().name();
		};

	};

	/*! Supports forward-declaring a "cast" from an Any to the desired destination type. e.g: int& ref_int = any_obj.cast(); ... std::string str = any_obj.cast(); */
	class AnyAutoCast; /* forward decl */

	/*! Generic container that enables the containment and sharing of any data type to/from std::shared_ptrs */
	class Any {
	public:
		struct Object_Data {
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(const H<S>* obj) { return get(*obj); };
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(const H<S>& obj) {
				return std::shared_ptr<AnyData>(new AnyData(AnyData::get_data<S>(obj), boost::typeindex::type_id<S>().type_info(), std::is_const_v<S>));
			};
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(H<S>&& obj) {
				return std::shared_ptr<AnyData>(new AnyData(AnyData::get_data<S>(std::forward<H<S>>(obj)), boost::typeindex::type_id<S>().type_info(), std::is_const_v<S>));
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<fibers::AnyAutoCast, T>>> static decltype(auto) get(T* t) { return get(std::make_shared<T>(t)); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<fibers::AnyAutoCast, T>>> static decltype(auto) get(const T* t) { return get(*t); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<fibers::AnyAutoCast, T>>> static decltype(auto) get(const T& obj) { return get(std::make_shared<T>(obj)); };
			static decltype(auto) get(const fibers::AnyAutoCast& obj);
			static decltype(auto) get(const fibers::AnyAutoCast* t);
		};
		template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(const ValueType& r) { return Object_Data::get(r); };
		template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(ValueType&& r) { return Object_Data::get(std::forward<ValueType>(r)); };

	public: /*! Init */
		constexpr Any() noexcept : container(nullptr) {};
		constexpr Any(std::nullptr_t) noexcept : container(nullptr) {};
		Any(const Any& rhs) noexcept : container(rhs.container) {};
		Any(Any&& rhs) noexcept : container(std::move(rhs.container)) {};

	public: /*! Init w/ DATA ASSIGNMENT */
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType& value) noexcept : container(CreateContainer(value)) {};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType* value) noexcept : container(CreateContainer(value)) {};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType* value) noexcept : container(CreateContainer(value)) {};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType&& value) noexcept : container(CreateContainer(std::forward<ValueType>(value))) {};

	public: /*! Destroy */
		~Any() = default;

	public: /*! Data Assignment AFTER INIT */
		Any& swap(Any& rhs) noexcept {
			if (this == &rhs) { return *this; }
			container.swap(rhs.container);
			return *this;
		};
		Any& operator=(const Any& rhs) noexcept {
			container = rhs.container;
			return *this;
		};
		Any& operator=(Any&& rhs) noexcept {
			rhs.container.swap(container);
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
		void Clear() noexcept { container = nullptr; };

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
				auto* ptr = p->container.get();
				if (ptr) {
					return ptr->cast<VType>();
				}
				else {
					decltype(auto) q{ std::make_shared<VType>() };
					p->container = Any::CreateContainer(q);
					return q;
				}
			};
			template <class VType> static decltype(auto) DoCast_Shared_Sentinel(Any* p) noexcept {
				throw("Casting Any to  std::shared_ptr<T>* or  std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
			};
			template<typename VType> static decltype(auto) DoCast_Unshared(Any* p) noexcept {
				constexpr bool is_ptr = std::is_pointer_v<VType>;

				typedef typename std::remove_reference<typename std::remove_pointer<VType>::type>::type desiredT;
				auto* ptr = p->container.get();
				if (ptr) {
					if constexpr (is_ptr) {
						return static_cast<desiredT*>(ptr->ptr());
					}
					else {
						return *static_cast<desiredT*>(ptr->ptr());
					}
				}
				else {
					decltype(auto) q{ std::make_shared<desiredT>() };
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
				constexpr bool is_shared_ptr = isShared::value;
				constexpr bool is_ptr = std::is_pointer_v<T>;
				constexpr bool is_ref = std::is_reference_v<T>;
				if constexpr (is_shared_ptr) {
					typedef typename get_type<T>::type innertype;
					if constexpr (is_ptr) {
						throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
					}
					else if constexpr (is_ref) {
						throw("Casting Any to std::shared_ptr<T>* or std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
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
		decltype(auto) cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<Any*>(this)); };

		template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
		Any& cast() const noexcept { return *const_cast<Any*>(this); };

		template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<Any, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
		Any* cast() const noexcept { return const_cast<Any*>(this); };

		AnyAutoCast cast() const noexcept;

	public:
		mutable std::shared_ptr<AnyData> container;

	};

	/*! Supports forward-declaring a "cast" from an Any to the desired destination type. e.g: int& ref_int = any_obj.cast(); ... std::string str = any_obj.cast(); */
	class AnyAutoCast {
	public:
		AnyAutoCast(const Any* _parent) :
			parent(const_cast<Any*>(_parent)),
			parentCopy(*_parent)
		{};
		AnyAutoCast(AnyAutoCast&& other) :
			parent(std::move(other.parent)),
			parentCopy(std::move(other.parentCopy))
		{};

		AnyAutoCast() = delete;
		AnyAutoCast(const AnyAutoCast&) = delete;
		AnyAutoCast& operator=(const AnyAutoCast&) = delete;
		AnyAutoCast& operator=(AnyAutoCast&&) = delete;
		~AnyAutoCast() {};

		explicit operator Any& () const noexcept { return *parent; };
		explicit operator Any* () const noexcept { return parent; };

		template <typename T>
		operator std::shared_ptr<T>() const noexcept { return parentCopy.cast<std::shared_ptr<T>>(); };

		template <typename T>
		operator std::shared_ptr<T>* () const noexcept { return parentCopy.cast<std::shared_ptr<T>*>(); };

		template< bool cond, typename U >
		using resolvedType = typename std::enable_if< cond, U >::type;

		template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
		operator ValueTypeT& () const noexcept { return *parentCopy.cast<ValueTypeT*>(); };

		template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
		operator ValueTypeT* () const noexcept { return parentCopy.cast<ValueTypeT*>(); };

		Any* parent;
		Any parentCopy;
	};

	/*! Casts to whatever is on the left-hand-side, with specializations for references, pointers, values, and std::shared_ptrs. References and pointers are lifetime-sensitive. */
	__forceinline AnyAutoCast Any::cast() const noexcept { return AnyAutoCast(this); };
	__forceinline decltype(auto) Any::Object_Data::get(const AnyAutoCast& obj) { Any* t = const_cast<Any*>(obj.parent); std::shared_ptr<AnyData> out; if (t) { out = t->container; } return out; };
	__forceinline decltype(auto) Any::Object_Data::get(const AnyAutoCast* t) { return get(*t); };

	namespace {
		class Action_Interface {
		public:
			virtual ~Action_Interface() = default;

			virtual boost::typeindex::type_info const& type() const noexcept { return boost::typeindex::type_id<void>().type_info(); };
			virtual const char* typeName() const noexcept { return ""; };
			virtual Action_Interface* clone() const noexcept {
				return new Action_Interface();
			};
			virtual fibers::Any& Invoke() noexcept { static fibers::Any staticTemp{}; return staticTemp; };
			virtual fibers::Any& ForceInvoke() noexcept { static fibers::Any staticTemp{}; return staticTemp; };
			virtual const char* FunctionName() const noexcept { return ""; };
			virtual const fibers::Any& Result() const noexcept { static fibers::Any staticTemp{}; return staticTemp; };
			virtual bool IsFinished() const noexcept { return true; };
			virtual bool ReturnsNothing() const noexcept { return true; };
			virtual std::function<void(Action_Interface*)>& deleter() const noexcept {
				static auto deleteFunc{ std::function<void(Action_Interface*)>([](Action_Interface* p) {
					if (p) {
						delete p;
					}
				}) };
				return deleteFunc;
			};
		};
		template<typename ValueType> class Action_Impl final : public Action_Interface {
		private:
			using ReturnType = typename Function<ValueType>::ResultType;
			static constexpr bool returnsNothing{ std::is_same<ReturnType, void>::value };

		public:
			Action_Impl() : Action_Interface() {};
			Action_Impl(Function<ValueType> const& f) noexcept : Action_Interface(), data(f), result(), running(false), finished(false) {};
			Action_Impl(Function<ValueType>&& f) noexcept : Action_Interface(), data(std::forward<Function<ValueType>>(f)), result(), running(false), finished(false) {};

			boost::typeindex::type_info const& type() const noexcept override final {
				return boost::typeindex::type_id<Function<ValueType>>().type_info();
			};
			const char* typeName() const noexcept final {
				return boost::typeindex::type_id<Function<ValueType>>().type_info().name();
			};
			Action_Interface* clone() const noexcept override final {
				return dynamic_cast<Action_Interface*>(new Action_Impl<ValueType>(data));
			};
			fibers::Any& Invoke() noexcept override final {
				if (finished.load()) return result;
				bool compare(false);
				if (running.compare_exchange_strong(compare, true)) {
					if constexpr (returnsNothing) {
						data();
					}
					else {
						result = fibers::Any(data());
					}
					finished.store(true);
					running.store(false);
				}
				else {
					while (running.load()) {}
				}
				return result;
			};
			fibers::Any& ForceInvoke() noexcept override final {
				if constexpr (returnsNothing) {
					data();
				}
				else {
					result = fibers::Any(data());
				}
				return result;
			};
			const char* FunctionName() const noexcept override final {
				return data.FunctionName();
			};
			const fibers::Any& Result() const noexcept override final {
				return result;
			};
			bool IsFinished() const noexcept override final {
				return finished.load();
			};
			bool ReturnsNothing() const noexcept override final {
				return data.ReturnsNothing();
			};
			std::function<void(Action_Interface*)>& deleter() const noexcept override final {
				static auto deleteFunc{ std::function<void(Action_Interface*)>([](Action_Interface* p) {
					if (p) {
						auto* p2 = dynamic_cast<Action_Impl<ValueType>*>(p);
						if (p2) {
							delete p2;
						}
						else {
							delete p;
						}
					}
				}) };
				return deleteFunc;
			};

			Function<ValueType> data;
			fibers::Any result;
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
		/*! Init */ Action() noexcept : content(nullptr) {};
		/*! Copy */ Action(const Action& other) noexcept : content(other.content ? other.content->clone() : nullptr) {};
		/*! Perfect forwarding */ Action(Action&& other) noexcept : content(std::move(other.content)) {};
		/*! Data Assignment */ template<typename ValueType> explicit Action(Function<ValueType>&& value) : content(ToPtr<ValueType>(std::forward< Function<ValueType>>(value))) {};
		/*! Direct instantiation2 */ template <typename F, typename... Args> Action(F&& function, Args &&... Fargs) : Action(Function(std::function(std::forward<F>(function)), std::forward<Args>(Fargs)...)) {};
		~Action() {
			if (content) {
				auto& deleter = content->deleter();
				deleter(content);
			}
		};

	public: // modifiers
		/*! Copy Data */ Action& operator=(const Action& rhs) = delete;
		/*! Move Data */ Action& operator=(Action&& rhs) = delete;

	public: // queries
		explicit operator bool() { return !IsEmpty(); };
		explicit operator bool() const { return !IsEmpty(); };

		/*! Checks if the Action has been assigned something */
		bool IsEmpty() const noexcept { return !content; };

		template <typename ValueT> static constexpr const char* TypeNameOf() { return utilities::typenames::type_name<ValueT>(); };
		template <typename ValueT> static const boost::typeindex::type_info& TypeOf() { return boost::typeindex::type_id<ValueT>().type_info(); };

		const char* TypeName() const noexcept {
			static const char* staticVal{ "" };
			if (content) return content->typeName();
			return staticVal;
		};
		const boost::typeindex::type_info& Type() const noexcept {
			static const boost::typeindex::type_info& staticVal{ boost::typeindex::type_id<void>().type_info() };
			if (content) return content->type();
			return staticVal;
		};

	private:
		template <class ValueType> static Action_Interface* ToPtr(Function<ValueType>&& rhs) noexcept {
			return dynamic_cast<Action_Interface*>(new Action_Impl<ValueType>(std::forward<Function<ValueType>>(rhs)));
		};

	public:
		Action_Interface* content;

	public:
		fibers::Any& operator()() {
			return Invoke();
		};
		fibers::Any& Invoke() {
			static fibers::Any staticVal{};
			if (content) return content->Invoke();
			return staticVal;
		};
		fibers::Any& ForceInvoke() {
			static fibers::Any staticVal{};
			if (content) return content->ForceInvoke();
			return staticVal;
		};
		const char* FunctionName() const {
			static const char* staticVal{ "" };
			if (content) return content->FunctionName();
			return staticVal;
		};
		bool     IsFinished() const {
			if (content) return content->IsFinished();
			return true;
		};
		bool     ReturnsNothing() const {
			if (content) return content->ReturnsNothing();
			return true;
		};
		const fibers::Any& Result() const {
			static fibers::Any staticVal{};
			if (content) return content->Result();
			return staticVal;
		};
	};

};
