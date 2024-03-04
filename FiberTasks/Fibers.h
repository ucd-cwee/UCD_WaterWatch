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

namespace fibers{
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

		class Hardware {
		public:
			static int GetNumCpuCores();
			static float GetPercentCpuLoad();
		};


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

	namespace utilities {
		template<typename T> struct count_arg;
		template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
		template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
		template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
		template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
		template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };
	};

	namespace containers {
		template<typename T > class DelayedInstantiation {
		public:
			DelayedInstantiation() : obj(nullptr), Lock(0), createFunc() {};
			DelayedInstantiation(std::function<T* ()> create) : obj(nullptr), createFunc(create) {};
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
						obj = std::shared_ptr<T>(createFunc());
					}
					unlock();
				}
			};
			void lock() {
				while ((Lock.fetch_add(1) + 1) != 1) Lock.fetch_sub(1);
			};
			void unlock() {
				Lock.fetch_sub(1);
			};
			std::shared_ptr<T> obj;
			std::atomic<long> Lock;
			std::function<T* ()> createFunc;
		};


		/* Fiber- and thread-safe vector. Objects are stored and returned as std::shared_ptr. Growth, iterations, and push_back operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire vector. */
		template<typename _Ty>
		class vector {
		protected:
			using underlying = typename concurrency::concurrent_vector<std::shared_ptr<_Ty>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const vector* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const vector* ref) { ++pos; }
				inline void end(const vector* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(vector* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const vector* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const vector* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(vector, typename underlying::value_type, it_state);

			vector() : data(new underlying()) {};
			explicit vector(size_type _N) : data(new underlying(_N)) {};
			vector(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			vector(vector const& r) : data(new underlying()) {
				operator=(r);
			}
			vector(vector&& r) = default;
			vector& operator=(vector const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					vector out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			vector& operator=(vector&& r) = default;
			~vector() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(std::make_shared<_Ty>(_Item)); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::make_shared<_Ty>(std::forward<_Ty>(_Item))); };
			decltype(auto) push_back(std::shared_ptr<_Ty> const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(std::shared_ptr<_Ty>&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			std::shared_ptr<_Ty> operator[](size_type _Index) { return data->operator[](_Index); };
			std::shared_ptr<_Ty> operator[](size_type _Index) const { return data->operator[](_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) { return data->at(_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
			decltype(auto) erase(const_iterator _Where) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) clear() {
				vector out;
				data.swap(out.data);
			};
			decltype(auto) swap(vector& r) {
				data.swap(r.data);
			};
		};

		/* Fiber- and thread-safe vector that cannot erase elements. Objects are stored as-is, and therefore the array must outlive the underlying objects. All operations are concurrent except for 'operator=(array)'.
		Higher performance is expected with the array than the vector, but the user needs to be slighly more careful with their timing to ensure the array remaings alive until access is complete. */
		template<typename _Ty>
		class array {
		protected:
			using underlying = typename concurrency::concurrent_vector<_Ty>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const array* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const array* ref) { ++pos; }
				inline void end(const array* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(array* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const array* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const array* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(array, typename underlying::value_type, it_state);

			array() : data(new underlying()) {};
			explicit array(size_type _N) : data(new underlying(_N)) {};
			array(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			array(array const& r) : data(new underlying()) {
				operator=(r);
			}
			array(array&& r) = default;
			array& operator=(array const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					array out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			array& operator=(array&& r) = default;
			~array() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			decltype(auto) operator[](size_type _Index) { return data->operator[](_Index); };
			decltype(auto) operator[](size_type _Index) const { return data->operator[](_Index); };
			decltype(auto) at(size_type _Index) { return data->at(_Index); };
			decltype(auto) at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
		};

		/* Fiber- and thread-safe map / dictionary. Objects are stored and returned as std::shared_ptr. Growth, iterations, and insert/emplace operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire map. */
		template<typename _Key_type, typename _Element_type>
		class unordered_map {
		protected:
			using underlying = typename concurrency::concurrent_unordered_map<_Key_type, std::shared_ptr<_Element_type>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;

				inline void begin(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const unordered_map* ref) { ++pos; }
				inline void end(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(unordered_map* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				inline void prev(const unordered_map* ref) { --pos; }
				inline const typename underlying::value_type& get(const unordered_map* ref) const { return *pos; }
			};
			SETUP_STL_ITERATOR(unordered_map, typename underlying::value_type, it_state);

			typedef typename underlying::key_type key_type;
			typedef typename underlying::mapped_type mapped_type;
			typedef typename underlying::key_equal key_equal;
			typedef typename underlying::hasher hasher;
			typedef iterator local_iterator;
			typedef const_iterator const_local_iterator;

			unordered_map() : data(new underlying()) {};
			explicit unordered_map(size_type _N) : data(new underlying(_N)) {};
			unordered_map(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			template<class _InputIterator> unordered_map(_InputIterator _Begin, _InputIterator _End) : data(new underlying(_Begin, _End)) {};
			unordered_map(unordered_map const& r) : data(new underlying()) {
				this->operator=(r);
			};
			unordered_map(unordered_map&& r) = default;
			unordered_map& operator=(unordered_map const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					unordered_map out;
					for (auto& x : *r.data) out[x.first] = x.second;
					out.data.swap(data);
				}
				return *this;
			};
			unordered_map& operator=(unordered_map&& r) = default;

			decltype(auto) insert(const value_type& _Value) { return data->insert(_Value); };
			decltype(auto) insert(const_iterator _Where, const value_type& _Value) { return data->insert(_Where, _Value); };
			template<class _Iterator> decltype(auto) insert(_Iterator _First, _Iterator _Last) { return data->insert(_First, _Last); };
			template<class _Valty> decltype(auto) insert(_Valty&& _Value) { return data->insert(_Value); };
			template<class _Valty> decltype(auto) insert(const_iterator _Where, _Valty&& _Value) { return data->insert(_Where, _Value); };
			decltype(auto) hash_function() const { return data->hash_function(); };
			decltype(auto) key_eq() const { return data->key_eq(); };
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) const {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) { return data->at(_Keyval); };
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) const { return data->at(_Keyval); };
			decltype(auto) front() { return data->front(); };
			decltype(auto) front() const { return data->front(); };
			decltype(auto) back() { return data->back(); };
			decltype(auto) back() const { return data->back(); };
			decltype(auto) erase(const_iterator _Where) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const key_type& _Keyval) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter->first == _Keyval) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) unsafe_erase(const key_type& _Keyval) {
				data->unsafe_erase(_Keyval);
			};
			decltype(auto) count(const key_type& _Keyval) const { return data->count(_Keyval); };
			decltype(auto) emplace(const key_type& _Keyval, const std::shared_ptr<_Element_type>& _Value) { return insert(value_type(_Keyval, _Value)); };
			decltype(auto) emplace(const key_type& _Keyval, std::shared_ptr<_Element_type>&& _Value) { return insert(value_type(_Keyval, std::forward<typename underlying::value_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, const _Element_type& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, _Element_type&& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(std::forward<typename underlying::value_type>(_Value)))); };
			decltype(auto) clear() {
				unordered_map out;
				data.swap(out.data);
			};
		};

		template<typename _Key_type> using unordered_set = concurrency::concurrent_unordered_set<_Key_type>; /* Wrapper To-Do */
		template<typename _Value_type> using queue = concurrency::concurrent_queue<_Value_type>; /* Wrapper To-Do */
    };

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup;

	/*!
	Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wraper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	Job([](){ return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, allowing you to continue work on this thread.
	*/
	class Job {
		friend JobGroup;
	protected:
		mutable std::shared_ptr<Action> impl;

	public:
		static Job Finished() {
			decltype(auto) toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished());
			return toReturn;
		};
		template <typename T> static Job Finished(const T& returnMe) {
			decltype(auto) toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished(returnMe));
			return toReturn;
		};

		Job() : impl(std::make_shared<Action>()) {};
		Job(const Job& other) = default;
		Job(Job&& other) = default;
		Job& operator=(const Job& other) = default;
		Job& operator=(Job&& other) = default;
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<T>> && !std::is_same_v<Any, std::decay_t<T>> >>
		explicit Job(T function, Args... Fargs) : impl(new Action(function, Fargs...)) {};

	public:
		/* Do the task immediately, without using any thread/fiber tools. Does not do the task if it has been previously performed. */
		Any Invoke() noexcept {
			Any out;
			auto* p = impl->Invoke();
			if (p) out = *p;
			return out;
		};

		/* Do the task immediately, without using any thread/fiber tools, whether or not it has been performed before. */
		Any ForceInvoke() noexcept {
			Any out;
			auto* p = impl->ForceInvoke();
			if (p) out = *p;
			return out;
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. Does not do the task if it has been previously performed. */
		JobGroup AsyncInvoke();

		/* Add the task to a thread / fiber, and retrieve an awaiter group, whether or not it has been performed before. */
		JobGroup AsyncForceInvoke();

		/* Queue job, and return tool to await the result */
		void DelayedInvoke(double milliseconds_delay);

		/* Queue job, and return tool to await the result */
		void DelayedForceInvoke(double milliseconds_delay);

		/* Queues the job onto a new thread to take place in the future */
		void AsyncDelayedInvoke(double milliseconds_delay);

		/* Queues the job onto a new thread to take place in the future */
		void AsyncDelayedForceInvoke(double milliseconds_delay);

		/* Returns the potential name of the static function, if one was provided. */
		const char* FunctionName() const {
			return impl->FunctionName();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any GetResult() {
			return Invoke();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any operator()() {
			return Invoke();
		};

		/* Checks if the job has been completed before */
		bool IsFinished() const {
			return impl->IsFinished();
		};

		bool ReturnsNothing() const {
			return impl->ReturnsNothing();
		};
	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup {
		friend Job;
	private:
		class JobGroupImpl {
		public:
			std::shared_ptr<void> waitGroup;
			fibers::containers::vector<Job> jobs;

			JobGroupImpl() : waitGroup(nullptr), jobs() {};
			JobGroupImpl(std::shared_ptr<void> wg) : waitGroup(wg), jobs() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;

			void Queue(Job const& job);
			void ForceQueue(Job const& job); 
			void Queue(std::vector<Job> const& listOfJobs);
			void ForceQueue(std::vector<Job> const& listOfJobs);
			void Wait();
			~JobGroupImpl() {
				// Wait(); // any jobs queued with this waiter will want to talk w/it when finished -- we must wait to ensure they go out of scope before we do.
			};
		};

	public:
		JobGroup();
		JobGroup(Job const& job);

		// The waiter should not be passed around. Ideally we want to follow Fiber job logic, e.g. splitting jobs quickly and 
		// then finishing them in the same job that started them, continuing like the split never happened.
		JobGroup(JobGroup const&) = delete;
		JobGroup(JobGroup&&) = delete;
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() {};

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			impl->Queue(job);
			return *this;
		};
		/* Queue job, and return tool to await the result */
		JobGroup& ForceQueue(Job const& job) {
			impl->ForceQueue(job);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			impl->Queue(listOfJobs);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& ForceQueue(std::vector<Job> const& listOfJobs) {
			impl->ForceQueue(listOfJobs);
			return *this;
		};

		/* Await all jobs in this group, and get the return values (which may be empty) for each job */
		std::vector<Any> Wait_Get() {
			impl->Wait();

			typename decltype(JobGroupImpl::jobs) out;
			out.swap(impl->jobs);

			if (out.size() > 0) {
				std::vector<Any> any;
				for (auto& job : out) {
					if (job) {
						any.push_back(job->GetResult());
					}
				}
				return any;
			}
			else {
				return std::vector<Any>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
			impl->jobs.clear();
		};

	private:
		std::unique_ptr<JobGroupImpl> impl;

	};

	namespace synchronization {
		/* Fiber mutex */
		class mutex {
		public:
			mutex();
			mutex(const mutex& other);
			mutex(mutex&& other);
			mutex& operator=(const mutex& s) { return *this; };
			mutex& operator=(mutex&& s) { return *this; };
			~mutex() {};

			[[nodiscard]] std::lock_guard<mutex>	guard() noexcept;
			void			lock() noexcept;
			void			unlock() noexcept;
			bool            try_lock() noexcept;

		protected:
			std::shared_ptr<void> Handle;

		};

		/* Wrapper for non-atomic objects to allow for threads to lock them for exclusive access */
		template< typename type, typename MutexType = fibers::synchronization::mutex>
		class interlocked {
		public:
			class ExclusiveObject {
			public:
				constexpr ExclusiveObject(const interlocked<type>& mut) : owner(const_cast<interlocked<type>&>(mut)) { this->owner.Lock(); };
				~ExclusiveObject() { this->owner.Unlock(); };

				ExclusiveObject() = delete;
				ExclusiveObject(const ExclusiveObject& other) = delete;
				ExclusiveObject(ExclusiveObject&& other) = delete;
				ExclusiveObject& operator=(const ExclusiveObject& other) = delete;
				ExclusiveObject& operator=(ExclusiveObject&& other) = delete;

				type& operator=(const type& a) { data() = a; return data(); };
				type& operator=(type&& a) { data() = a; return data(); };
				type& operator*() const { return data(); };
				type* operator->() const { return &data(); };

			protected:
				type& data() const { return owner.UnsafeRead(); };
				interlocked<type>& owner;
			};

		public: // construction and destruction
			typedef type Type;

			interlocked() : data() {};
			interlocked(const type& other) : data(other) {};
			interlocked(type&& other) : data(std::forward<type>(other)) {};
			interlocked(const interlocked& other) : data() { this->Copy(other); };
			interlocked(interlocked&& other) : data() { this->Copy(std::forward<interlocked>(other)); };
			~interlocked() {};

		public: // copy and clear
			interlocked<type>& operator=(const interlocked<type>& other) {
				this->Copy(other);
				return *this;
			};
			interlocked<type>& operator=(interlocked<type>&& other) {
				this->Copy(std::forward<interlocked<type>>(other));
				return *this;
			};
			void Copy(const interlocked<type>& copy) {
				if (&copy == this) return;
				lock.Lock();
				copy.Lock();
				data = copy.data;
				copy.Unlock();
				lock.Unlock();
			};
			void Copy(interlocked<type>&& copy) {
				if (&copy == this) return;
				lock.Lock();
				copy.Lock();
				data = copy.data;
				copy.Unlock();
				lock.Unlock();
			};
			void Clear() {
				lock.Lock();
				data = type();
				lock.Unlock();
			};

		public: // read and swap
			type Read() const {
				decltype(auto) g = Guard();
				return data;
			};
			void Swap(const type& replacement) {
				decltype(auto) g = Guard();
				data = replacement;
			};
			interlocked<type>& operator=(const type& other) {
				Swap(other);
				return *this;
			};

			operator type() const { return Read(); };
			type* operator->() const { return &data; };

		public: // lock, unlock, and direct edit
			ExclusiveObject GetExclusive() const { return ExclusiveObject(*this); };

			[[nodiscard]] decltype(auto) Guard() const { return lock.Guard(); };
			void Lock() const { lock.Lock(); };
			void Unlock() const { lock.Unlock(); };
			type& UnsafeRead() const { return data; };

		private:
			mutable type data;
			MutexType lock;

		};

		/* Tool that allows fibers to busy-work until a signla is raised by another fiber or thread. */
		class signal {
		public:
			signal(bool manualReset = true);
			signal(const signal&) = default;
			signal(signal&&) = default;
			signal& operator=(const signal&) = default;
			signal& operator=(signal&&) = default;
			~signal() {};

		public:
			/* Raise (set to one) the signal. Free & fast. */
			void	Raise() noexcept;
			/* Clear (zero-out) the signal. Free & fast. */
			void	Clear() noexcept;
			/* Busy-waits for the signal to be raised. */
			void	Wait() noexcept;
			/* Tests if the signal has been raised. Free & fast. */
			bool	TryWait() noexcept;

		private:
			std::shared_ptr<void> impl;

		};

		/* Read-Write mutex that allows multiple readers and one writer to cooperatively access an underlying object. Very fast for 100% reading operations, as (effectively) no locking actually happens. */
		class shared_mutex {
		private:
			mutex    mut_;
			std::condition_variable_any gate1_;
			std::condition_variable_any gate2_;
			unsigned state_;

			static const unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
			static const unsigned n_readers_ = ~write_entered_;

		public:

			shared_mutex() : state_(0) {}

			// Exclusive/Writer ownership
			void lock() {
				std::unique_lock<mutex> lk(mut_);
				while (state_ & write_entered_) gate1_.wait(lk);
				state_ |= write_entered_;
				while (state_ & n_readers_) gate2_.wait(lk);
			};
			// Exclusive/Writer ownership
			bool try_lock() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock);
				if (lk.owns_lock() && state_ == 0)
				{
					state_ = write_entered_;
					return true;
				}
				return false;
			};
			// Exclusive/Writer ownership
			void unlock() {
				{
					std::scoped_lock<mutex> _(mut_);
					state_ = 0;
				}
				gate1_.notify_all();
			};

			// Shared/Reader ownership
			void lock_shared() {
				std::unique_lock<mutex> lk(mut_);
				while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_)
					gate1_.wait(lk);
				unsigned num_readers = (state_ & n_readers_) + 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
			};
			// Shared/Reader ownership
			bool try_lock_shared() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock);
				unsigned num_readers = state_ & n_readers_;
				if (lk.owns_lock() && !(state_ & write_entered_) && num_readers != n_readers_)
				{
					++num_readers;
					state_ &= ~n_readers_;
					state_ |= num_readers;
					return true;
				}
				return false;
			};
			// Shared/Reader ownership
			void unlock_shared() {
				std::scoped_lock<mutex> _(mut_);
				unsigned num_readers = (state_ & n_readers_) - 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
				if (state_ & write_entered_)
				{
					if (num_readers == 0)
						gate2_.notify_one();
				}
				else
				{
					if (num_readers == n_readers_ - 1)
						gate1_.notify_one();
				}
			};

			[[nodiscard]] decltype(auto) Write_Guard() noexcept { return std::lock_guard(*this); };
			[[nodiscard]] decltype(auto) Read_Guard() noexcept { return std::shared_lock(*this); };
		};
	};


	namespace ftl_wrapper {
		class TaskScheduler {
		public:
			TaskScheduler();
			TaskScheduler(TaskScheduler const&) = delete;
			TaskScheduler(TaskScheduler&&) noexcept = delete;
			TaskScheduler& operator=(TaskScheduler const&) = delete;
			TaskScheduler& operator=(TaskScheduler&&) noexcept = delete;
			~TaskScheduler() = default;

		private:
			std::shared_ptr<void> m_TaskScheduler;
			std::shared_ptr<void> m_WaitGroup;

		public:
			void AddTask(Job const& task);
			void Wait();

		};


	};

	namespace parallel {
		class Timer {
		public:
			Timer() : data(nullptr) {};
			Timer(long double millisecondsBetweenDispatch, Job const& queuedActivity);
			Timer(Timer const&) = default;
			Timer(Timer&&) = default;
			Timer& operator=(Timer const&) = default;
			Timer& operator=(Timer&&) = default;
			~Timer() {};

		private:
			std::shared_ptr<void> data;
		};



		/* parallel_for (auto i = start; i < end; i++){ todo(i); } */
		template<typename iteratorType, typename F>
		std::vector<Any> For(iteratorType start, iteratorType end, F&& ToDo) {
			ftl_wrapper::TaskScheduler scheduler;

			auto todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;
			for (iteratorType iter = start; iter < end; iter++) {
				scheduler.AddTask(fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter));

				// jobs.push_back(fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter));
			}
			//JobGroup group;
			//group.Queue(jobs);

			//if constexpr (retNo) { group.Wait(); return std::vector<Any>(); }
			//else return group.Wait_Get();

			scheduler.Wait();
			return std::vector<Any>();
		};

		/* parallel_for (auto i = start; i < end; i += step){ todo(i); } */
		template<typename iteratorType, typename F>
		std::vector<Any> For(iteratorType start, iteratorType end, iteratorType step, F&& ToDo) {
			auto todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;
			for (iteratorType iter = start; iter < end; iter += step) {
				jobs.push_back(fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); } */
		template<typename containerType, typename F>
		std::vector<Any> ForEach(containerType const& container, F&& ToDo) {
			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;

			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs.push_back(fibers::Job([todo](typename containerType::const_iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(const_cast<typename containerType::value_type*>(&*T), [](typename containerType::value_type*) {})).cast()); }, (typename containerType::const_iterator)(iter)));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};

		/* parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); } */
		template<typename containerType, typename F>
		std::vector<Any> ForEach(containerType& container, F&& ToDo) {
			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			std::vector<fibers::Job> jobs;

			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs.push_back(fibers::Job([todo](typename containerType::iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(&*T, [](typename containerType::value_type*) {})).cast()); }, (typename containerType::iterator)(iter)));
			}

			JobGroup group;
			group.Queue(jobs);

			if constexpr (retNo) group.Wait();
			else return group.Wait_Get();
		};

		/* Generic form of a future<T>, which can be used to wait on and get the results of any job. */
		class promise {
		protected:
			std::shared_ptr< std::atomic<JobGroup*>> shared_state;
			std::shared_ptr<std::atomic<Any*>> result;

		public:
			promise() : shared_state(nullptr), result(nullptr) {};
			promise(Job const& job) :
				shared_state(std::shared_ptr<std::atomic<JobGroup*>>(new std::atomic<JobGroup*>(new JobGroup(job)), [](std::atomic<JobGroup*>* anyP) { if (anyP) { auto* p = anyP->exchange(nullptr); if (p) { delete p; } delete anyP; } })),
				result(std::shared_ptr<std::atomic<Any*>>(new std::atomic<Any*>(), [](std::atomic<Any*>* anyP) { if (anyP) { auto* p = anyP->exchange(nullptr); if (p) { delete p; } delete anyP; } })) {};
			promise(promise const&) = default;
			promise(promise&&) = default;
			promise& operator=(promise const&) = default;
			promise& operator=(promise&&) = default;
			virtual ~promise() {};

			bool valid() const noexcept {
				return (bool)shared_state;
			};
			void wait() {
				if (shared_state) {
					auto p = shared_state->exchange(nullptr);
					if (p) {
						Any* p2 = result->exchange(new Any(p->Wait_Get()[0]));
						if (p2) {
							delete p2;
						}
						delete p;
					}
				}
				while (result && !result->load()) {
					YieldProcessor();
				}
			};

			Any get_any() const noexcept {
				if (result) {
					Any* p = result->load();
					if (p) {
						return Any(*p);
					}
				}
				return Any();
			};
			Any wait_get_any() {
				wait();
				return get_any();
			};
		};

		/* A secondary type tag used to identify if a template type is a future<T> type. */
		class future_type {
		public:
			virtual ~future_type() {};
		};

		/* Specialized form of a promise, which can be used to handle type-casting for lambdas automatically, while still being useful for waiting on and getting the results of any job. 
		Note: Only the first thread that "waits" on a future<T> assists the thread pool. More waiters != more jobs, and therefore additional waiters are spin-locking.
		Recommended that only the thread (or consuming thread) that scheduled the future<T> object should wait for it. */
		template <typename T> class future final : public promise/*, public future_type*/ {
		public:
			future() : promise()/*, future_type()*/ {};
			future(Job const& job) : promise(job)/*, future_type()*/ {};
			future(promise const& p_promise) : promise(p_promise)/*, future_type()*/ {};
			future(future const&) = default;
			future(future&&) = default;
			future& operator=(future const&) = default;
			future& operator=(future&&) = default;
			virtual ~future() {};

			/* Cast-down to a generic promise that erases the information on the return type. Useful for sharing tasks between libraries where type info itself cannot be shared. */
			promise as_promise() const { return promise(reinterpret_cast<const promise&>(*this)); };

			/* get a copy of the result of the task. must have already waited. */
			decltype(auto) get() {
				if (result) {
					Any* p = result->load();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						} else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get();
							//}
							//else {
								return static_cast<T>(p->cast<T>());
							//}
						}						
					}
				}
				throw(std::runtime_error("future was empty"));
			};
			/* get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. must have already waited. */
			decltype(auto) get_ref() {
				if (result) {
					Any* p = result->load();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get_ref();
							//}
							//else {
								return static_cast<T&>(p->cast<T&>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};
			/* get a shared_pointer of the result of the task. must have already waited. */
			decltype(auto) get_shared() {
				if (result) {
					Any* p = result->load();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get_shared();
							//}
							//else {
								return static_cast<std::shared_ptr<T>>(p->cast<std::shared_ptr<T>>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};

			/* wait to get a copy of the result of the task. Repeated waiting is OK. */
			decltype(auto) wait_get() {
				wait();
				return get();
			};
			/* wait to get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. Repeated waiting is OK. */
			decltype(auto) wait_get_ref() {
				wait();
				return get_ref();
			};
			/* wait to get a shared_pointer of the result of the task. Repeated waiting is OK. */
			decltype(auto) wait_get_shared() {
				wait();
				return get_shared();
			};
		};
		
		/* returns a future<T> object for awaiting the results of the job. */
		template < typename F, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<F>> && !std::is_same_v<Any, std::decay_t<F>> >>
		__forceinline static decltype(auto) async(F function, Args... Fargs) {
			return future<typename utilities::function_traits<decltype(std::function(function))>::result_type>(Job(function, Fargs...));
		};
	};



};
