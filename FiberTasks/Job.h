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

namespace fibers {
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
			AnyData(std::shared_ptr<void> const& t_ptr, const boost::typeindex::type_info& t_type, bool t_const) noexcept :
				m_ptr(t_ptr),
				m_type(t_type),
				m_const(t_const)
			{};
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

	class AnyAutoCast; /* forward */

	/*! Generic container that enables the containment and sharing of any data type to/from std::shared_ptrs */
	class Any {
	public:
		struct Object_Data {
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(const H<S>* obj) { return get(*obj); };
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(const H<S>& obj) {
				return std::static_pointer_cast<AnyData>(std::shared_ptr<AnyData_Impl<S>>(new AnyData_Impl<S>(obj)));
			};
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(H<S>&& obj) {
				return std::static_pointer_cast<AnyData>(std::shared_ptr<AnyData_Impl<S>>(new AnyData_Impl<S>(std::forward<H<S>>(obj))));
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<AnyAutoCast, T>>> static decltype(auto) get(T* t) { std::shared_ptr<T> sp = std::make_shared<T>(t); return get(sp); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<AnyAutoCast, T>>> static decltype(auto) get(const T* t) { return get(*t); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<AnyAutoCast, T>>> static decltype(auto) get(const T& obj) { std::shared_ptr<T> sp = std::make_shared<T>(obj); return get(sp); };
			static decltype(auto) get(const AnyAutoCast& obj);
			static decltype(auto) get(const AnyAutoCast* t);
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
				decltype(auto) ptr = m.get();
				if (ptr) {
					return ptr->cast<VType>();
				}
				else {
					decltype(auto) q = std::make_shared<VType>();
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

	__forceinline AnyAutoCast Any::cast() const noexcept { return AnyAutoCast(this); };
	__forceinline decltype(auto) Any::Object_Data::get(const AnyAutoCast& obj) { Any* t = const_cast<Any*>(obj.parent); std::shared_ptr<AnyData> out; if (t) { out = t->container; } return out; };
	__forceinline decltype(auto) Any::Object_Data::get(const AnyAutoCast* t) { return get(*t); };

	template <typename F = void()> class Function {
	public:
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };

		typedef F Type;
		typedef typename std::function<F>::result_type ResultType;
		typedef typename function_traits<std::function<F>>::arguments Arguments;

		Function() noexcept
			: _function()
			, _data()
			, Result()
			, IsFinished(0)
		{};

		template <typename... Args>
		Function(const std::function<F>& function, Args... Fargs) noexcept
			: _function(function)
			, _data(GetData(Fargs...))
			, Result()
			, IsFinished(0)
		{};

	private:
		static void AddData(std::vector<Any>& d) { return; };
		template<typename T, typename... Targs> static void AddData(std::vector<Any>& d, const T& value, Targs... Fargs) // recursive function
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
		template <typename... Args> std::vector<Any> GetData(Args... Fargs) {
			constexpr size_t NumNeededInputs = NumInputs();
			constexpr size_t NumProvidedInputs = sizeof...(Args);
			static_assert(NumNeededInputs <= NumProvidedInputs, "Providing fewer inputs than required is unsupported. C++ Lambdas cannot support default arguments and therefore all arguments must be provided for.");

			std::vector<Any> out;
			AddData(out, Fargs...);
			return out;
		};

	public:
		Function(const Function& copy) noexcept
			: _function(copy._function)
			, _data(copy._data)
			, Result(copy.Result)
			, IsFinished(copy.IsFinished.load())
		{};

		static Function Finished() {
			Function to_return;

			to_return.IsFinished.fetch_add(1);

			return to_return;
		};
		template <typename T> static Function Finished(const T& returnMe) {
			Function to_return;

			to_return.Result = returnMe;
			to_return.IsFinished.fetch_add(1);

			return to_return;
		};

		Any& Invoke() {
			DoJob();
			return Result;
		};
		Any& ForceInvoke() {
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

		Any& GetResult() {
			return Result;
		};
		Any& GetResult() const {
			return Result;
		};

	private:
		void						DoJob() {
			static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a Function without further specialization.");

			if ((IsFinished.fetch_add(1) + 1) == 1) {
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
			}
			else {
				IsFinished.fetch_sub(1);
			}
		};
		void						ForceDoJob() {
			static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a Function without further specialization.");

			IsFinished.fetch_add(1);
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
			}
		};

		void						DoJob_Internal_16() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
			}
		};
		void						DoJob_Internal_15() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
			}
		};
		void						DoJob_Internal_14() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
			}
		};
		void						DoJob_Internal_13() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
			}
		};
		void						DoJob_Internal_12() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
			}
		};
		void						DoJob_Internal_11() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
			}
		};
		void						DoJob_Internal_10() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
			}
		};
		void						DoJob_Internal_9() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
			}
		};
		void						DoJob_Internal_8() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
			}
		};
		void						DoJob_Internal_7() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
			}
		};
		void						DoJob_Internal_6() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
			}
		};
		void						DoJob_Internal_5() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
			}
		};
		void						DoJob_Internal_4() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
			}
		};
		void						DoJob_Internal_3() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
			}
		};
		void						DoJob_Internal_2() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast(), _data[1].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast(), _data[1].cast());
			}
		};
		void						DoJob_Internal_1() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function(_data[0].cast());
				Any().swap(Result);
			}
			else {
				Result = _function(_data[0].cast());
			}
		};
		void						DoJob_Internal_0() {
			if constexpr (ReturnsNothing())
			{
				/*    */ _function();
				Any().swap(Result);
			}
			else {
				Result = _function();
			}
		};

	private:
		std::function<F>			_function;
		std::vector<Any>		_data;

	public:
		mutable Any				  Result;
		mutable std::atomic<long> IsFinished;

	};
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
		virtual Any& Result() const noexcept = 0;
		virtual bool IsFinished() const noexcept = 0;
		virtual bool ReturnsNothing() const noexcept = 0;
	};
	template<typename ValueType> class Action_Impl final : public Action_Interface {
	public:
		explicit Action_Impl() = delete;
		explicit Action_Impl(Function<ValueType> const& f) noexcept : data(f) {};
		explicit Action_Impl(Function<ValueType>&& f) noexcept : data(std::forward<Function<ValueType>>(f)) {};
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
			return data.Invoke();
		};
		virtual Any& ForceInvoke() noexcept final {
			return data.ForceInvoke();
		};
		virtual const char* FunctionName() const noexcept final {
			return data.FunctionName();
		};
		virtual Any& Result() const noexcept final {
			return data.GetResult();
		};
		virtual bool IsFinished() const noexcept final {
			return data.IsFinished.load() > 0;
		};
		virtual bool ReturnsNothing()  const noexcept final {
			return data.ReturnsNothing();
		};

		Function<ValueType> data;
	};

	class Action {
	public: // structors
		/*! Init */ Action() noexcept : content(BasePtr()) {};
		/*! Copy */ Action(const Action& other) noexcept : content(BasePtr()) { std::shared_ptr<Action_Interface> c = other.content; content = c->clone(); };
		/*! Data Assignment */ template<typename ValueType> explicit Action(const Function<ValueType>& value) : content(ToPtr<ValueType>(value)) {};
		// /*! Direct instantiation */ template <typename F, typename... Args> Action(const std::function<F>& function, Args... Fargs) : Action(Function(function, Fargs...)) {};
		/*! Direct instantiation2 */ template <typename F, typename... Args> Action(const F& function, Args... Fargs) : Action(Function(std::function(function), Fargs...)) {};
		~Action() noexcept { content = nullptr; };

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
		static std::shared_ptr<Action_Interface> BasePtr() noexcept { return ToPtr(Function<void()>()); };

	public:
		std::shared_ptr<Action_Interface> content;

	public:
		Any* Invoke() noexcept { std::shared_ptr<Action_Interface> c = content; if (c) { return &c->Invoke(); } return nullptr; };
		Any* ForceInvoke() noexcept { std::shared_ptr<Action_Interface> c = content; if (c) { return &c->ForceInvoke(); } return nullptr; };
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

		Any* Result() const { std::shared_ptr<Action_Interface> c = content; if (c) { return &c->Result(); } return nullptr; };
		static Action Finished() { return Action(Function<void()>::Finished()); };
		template <typename T> static Action Finished(const T& returnMe) { return Action(Function<T()>::Finished(returnMe)); };

	};
};
