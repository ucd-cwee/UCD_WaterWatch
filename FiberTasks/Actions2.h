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
#include <type_traits>
#include <functional>
#include <memory>
#include <utility>
#include <map>

// Type_Info and defer(...)
namespace GoodLang {
	class Any;

	// Finally is a pure virtual base class, implemented by the templated FinallyImpl.
	class Finally {
	public:
		virtual ~Finally() = default;
	};
	// FinallyImpl implements a Finally.
	// The template parameter F is the function type to be called when the finally is destructed. F must have the signature void().

	template <typename F>
	class FinallyImpl : public Finally {
	public:
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

	template <typename F> __forceinline [[nodiscard]] FinallyImpl<F> make_finally(F&& f) { return FinallyImpl<F>(std::forward<F>(f)); };
	template <typename F> __forceinline [[nodiscard]] std::shared_ptr<Finally> make_shared_finally(F&& f) { return std::make_shared<FinallyImpl<F>>(std::forward<F>(f)); };

#define FINALLY_CONCAT_(a, b) a##b
#define FINALLY_CONCAT(a, b) FINALLY_CONCAT_(a, b)

	// defer() is a macro to defer execution of a statement until the surrounding scope is closed and is typically used to perform cleanup logic once a function returns.
	// . .
	// Note: Unlike golang's defer(), the defer statement is executed when the surrounding *scope* is closed, not necessarily the function.
	// . .
	// Example usage:
	// . .
	// void sayHelloWorld() {
	//		defer(printf("world\n"));
	//      printf("hello ");
	// }
#define defer(x) decltype(auto) FINALLY_CONCAT(defer_, __LINE__) { make_finally([&] { x; }) }

	namespace impl {
		template<typename T> static const auto& TypeId() {
			static auto typeIdOfT{ boost::typeindex::type_id<T>() };
			return typeIdOfT.type_info();
		};
		using underlying_type_info = decltype(TypeId<void>());
	};
	namespace details {
		template<typename T>
		struct Bare_Type {
			using type = typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type;
		};

		inline static void hash_combine(std::size_t& seed) { };
		template <typename T, typename... Rest>
		inline static void hash_combine(std::size_t& seed, T&& v, Rest &&... rest) {
			std::hash<T> hasher{};
			seed ^= hasher(std::forward<T>(v)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			hash_combine(seed, std::forward<Rest>(rest)...);
		};
		template <typename T, typename... Rest>
		inline static void hash_combine(std::size_t& seed, T const& v, Rest const&... rest) {
			std::hash<T> hasher{};
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			hash_combine(seed, rest...);
		};

	};

	// Type_Info records the type of either built-in or scripted, runtime types
	//class BuiltIn_Type_Info; class Scripted_Type_Info;
	class Type_Info {
	public:
		//friend class BuiltIn_Type_Info;
		//friend class Scripted_Type_Info;
	protected:
		virtual size_t GetHashImpl() const {
			return impl::TypeId<void>().hash_code();
		};
		void CacheHash() noexcept {
			const_cast<size_t&>(uniqueHash) = this->GetHashImpl();
			details::hash_combine(const_cast<size_t&>(uniqueHash), (size_t)is_const(), (size_t)is_ref());
		};

	public:
		size_t GetHash() const {
			return uniqueHash;
		};

		Type_Info() noexcept
			: uniqueHash(GetHashImpl())
			, isConst(false)
			, isVoid(true)
			, isRef(false)
		{};
		Type_Info(bool t_is_const, bool t_is_void, bool t_is_ref, bool t_is_any) noexcept
			: uniqueHash(GetHashImpl())
			, isConst(t_is_const)
			, isVoid(t_is_void)
			, isRef(t_is_ref)
			, isAny(t_is_any)
		{};
		Type_Info(Type_Info const&) = delete;
		Type_Info(Type_Info&&) = delete;
		Type_Info& operator=(Type_Info const&) = delete;
		Type_Info& operator=(Type_Info&&) = delete;
		virtual ~Type_Info() = default;

		// Returns true if the types are similar enough to be casted
		static bool CanCast(Type_Info const& from, Type_Info const& to) {
			if (from.GetHashImpl() == to.GetHashImpl()) { // underlying matches
				// anything can convert into const T&
				if (to.is_const() && to.is_ref()) return true;

				// const T cannot be cast to T
				if (!to.is_const() && from.is_const()) return false;

				// T cannot be cast to T&
				if (!from.is_ref() && to.is_ref()) return false;

				return true;
			}
			return false;
		};
		// Returns true if the types are similar enough to be casted
		bool CanCast(Type_Info const& to) const {
			return CanCast(*this, to);
		};

		//// Operators
		friend bool operator==(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() == b.GetHash();
		};
		friend bool operator!=(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() != b.GetHash();
		};
		friend bool operator<(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() < b.GetHash();
		};
		friend bool operator<=(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() <= b.GetHash();
		};
		friend bool operator>(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() > b.GetHash();
		};
		friend bool operator>=(const Type_Info& a, const Type_Info& b) noexcept {
			return a.GetHash() >= b.GetHash();
		};

		// Query
		bool is_const() const noexcept { return isConst; };
		bool is_void() const noexcept { return isVoid; };
		bool is_ref() const noexcept { return isRef; };
		bool is_any() const noexcept { return isAny; };
		virtual std::string name() const noexcept { return impl::TypeId<void>().name(); };
		virtual std::weak_ptr<Type_Info> MakeBase() const { return std::weak_ptr<Type_Info>(); };
		virtual std::weak_ptr<Type_Info> MakeConst() const { return std::weak_ptr<Type_Info>(); };
		virtual std::weak_ptr<Type_Info> MakeRef() const { return std::weak_ptr<Type_Info>(); };
		virtual std::weak_ptr<Type_Info> MakeConstRef() const { return std::weak_ptr<Type_Info>(); };
		virtual std::weak_ptr<Type_Info> RemoveConst() const { return std::weak_ptr<Type_Info>(); };
		virtual std::weak_ptr<Type_Info> RemoveRef() const { return std::weak_ptr<Type_Info>(); };
	
		const size_t uniqueHash;
	private:
		bool isConst;
		bool isVoid;
		bool isRef;
		bool isAny;
		
	private: // flags
		//static const int is_const_flag = 0;
		//static const int is_void_flag = 1;
		//static const int is_ref_flag = 2;
	};

	template <typename T>
	class BuiltIn_Type_Info final : public Type_Info {
	protected:
		virtual size_t GetHashImpl() const override {
			return this->m_type_info.hash_code();
		};

	public:

		BuiltIn_Type_Info() noexcept
			: Type_Info(false, true, false, false)
			, m_type_info(impl::TypeId<void>())
		{
			this->CacheHash();
		};
		BuiltIn_Type_Info(impl::underlying_type_info t_ti, bool t_is_const = false, bool t_is_ref = false) noexcept
			: Type_Info(t_is_const, false, t_is_ref, std::is_same<typename std::decay_t<T>, Any>::value)
			, m_type_info(t_ti)
		{
			this->CacheHash();
		};
		BuiltIn_Type_Info(BuiltIn_Type_Info const&) = delete;
		BuiltIn_Type_Info(BuiltIn_Type_Info&&) = delete;
		BuiltIn_Type_Info& operator=(BuiltIn_Type_Info const&) = delete;
		BuiltIn_Type_Info& operator=(BuiltIn_Type_Info&&) = delete;
		virtual ~BuiltIn_Type_Info() = default;

		virtual std::string name() const noexcept override {
			if (is_const()) {
				if (is_ref()) {
					return std::string("const ") + std::string(m_type_info.name()) + "&";
				}
				else {
					return std::string("const ") + std::string(m_type_info.name());
				}
			}
			else {
				if (is_ref()) {
					return std::string(m_type_info.name()) + "&";
				}
				else {
					return m_type_info.name();
				}
			}
		};
		impl::underlying_type_info type_info() const noexcept {
			return m_type_info;
		};
		virtual std::weak_ptr<Type_Info> MakeBase() const { 
			using baseType = typename std::decay_t<T>;
			static auto out{ std::make_shared<BuiltIn_Type_Info<baseType>>(
				impl::TypeId<baseType>(),
				false,
				false
			) };
			return out;
		};
		virtual std::weak_ptr<Type_Info> MakeConst() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			using baseType = typename std::decay_t<T>;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
					 impl::TypeId<void>(),
					 false,
					 false
				) };
				return out;
			}
			else {
				if constexpr (thisIsRef) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType&>>(
						 impl::TypeId<const baseType&>(),
						 true,
						 thisIsRef
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType>>(
						 impl::TypeId<const baseType>(),
						 true,
						 thisIsRef
					) };
					return out;
				}
			}
		};
		virtual std::weak_ptr<Type_Info> MakeRef() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			using baseType = typename std::decay_t<T>;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
					 impl::TypeId<void>(),
					 false,
					 false
				) };
				return out;
			}
			else {
				if constexpr (thisIsConst) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType&>>(
						 impl::TypeId<const baseType&>(),
						 thisIsConst,
						 true
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType&>>(
						 impl::TypeId<baseType&>(),
						 thisIsConst,
						 true
					) };
					return out;
				}
			}
		};
		virtual std::weak_ptr<Type_Info> MakeConstRef() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			using baseType = typename std::decay_t<T>;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
					 impl::TypeId<void>(),
					 false,
					 false
				) };
				return out;
			}
			else {
				static auto out{ std::make_shared<BuiltIn_Type_Info<baseType&>>(
					 impl::TypeId<baseType&>(),
					 true,
					 true
				) };
				return out;
			}
		};
		virtual std::weak_ptr<Type_Info> RemoveConst() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			using baseType = typename std::decay_t<T>;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
					 impl::TypeId<void>(),
					 false,
					 false
				) };
				return out;
			}
			else {
				if constexpr (thisIsRef) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType&>>(
						 impl::TypeId<baseType&>(),
						 false,
						 thisIsRef
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType>>(
						 impl::TypeId<baseType>(),
						 false,
						 thisIsRef
					) };
					return out;
				}
			}
		};
		virtual std::weak_ptr<Type_Info> RemoveRef() const override {
			constexpr bool thisIsConst = std::is_const<T>::value;
			constexpr bool thisIsRef = std::is_reference<T>::value;
			using baseType = typename std::decay_t<T>;

			if constexpr (std::is_same< baseType, void>::value) {
				static auto out{ std::make_shared<BuiltIn_Type_Info<void>>(
					 impl::TypeId<void>(),
					 false,
					 false
				) };
				return out;
			}
			else {
				if constexpr (thisIsConst) {
					static auto out{ std::make_shared<BuiltIn_Type_Info<const baseType>>(
						 impl::TypeId<const baseType>(),
						 thisIsConst,
						 false
					) };
					return out;
				}
				else {
					static auto out{ std::make_shared<BuiltIn_Type_Info<baseType>>(
						 impl::TypeId<baseType>(),
						 thisIsConst,
						 false
					) };
					return out;
				}
			}
		};

	private:
		impl::underlying_type_info m_type_info;

	};

	class Scripted_Type_Info final : public Type_Info {
	protected:
		virtual size_t GetHashImpl() const override {
			return this->m_uniqueHash;
		};

	public:
		Scripted_Type_Info() noexcept
			: Type_Info(false, true, false, false)
			, m_full_name("")
			, m_qualified_namespace("")
			, m_name("")
			, m_uniqueHash(impl::TypeId<void>().hash_code())
		{
			this->CacheHash();
		};
		Scripted_Type_Info(const std::string& t_namespace, const std::string& t_name, bool t_is_const = false, bool t_is_ref = false) noexcept
			: Type_Info(t_is_const, false, t_is_ref, false)
			, m_full_name(t_namespace + "::" + t_name)
			, m_qualified_namespace(t_namespace)
			, m_name(t_name)
			, m_uniqueHash(std::hash<std::string>()(t_namespace + "::" + t_name))
		{
			this->CacheHash();
		};
		Scripted_Type_Info(Scripted_Type_Info const&) = delete;
		Scripted_Type_Info(Scripted_Type_Info&&) = delete;
		Scripted_Type_Info& operator=(Scripted_Type_Info const&) = delete;
		Scripted_Type_Info& operator=(Scripted_Type_Info&&) = delete;
		virtual ~Scripted_Type_Info() = default;

		virtual std::string name() const noexcept override {
			if (is_const()) {
				if (is_ref()) {
					return std::string("const ") + m_name + "&";
				}
				else {
					return std::string("const ") + m_name;
				}
			}
			else {
				if (is_ref()) {
					return m_name + "&";
				}
				else {
					return m_name;
				}
			}

			return m_full_name;
		};
		void SetSelf(std::shared_ptr<Scripted_Type_Info>& t_self) {
			m_self = t_self;
		};
		virtual std::weak_ptr<Type_Info> MakeBase() const {
			return MakeDuplicate(false, false);
		};
		virtual std::weak_ptr<Type_Info> MakeConst() const override {
			return MakeDuplicate(true, is_ref());
		};
		virtual std::weak_ptr<Type_Info> MakeRef() const override {
			return MakeDuplicate(is_const(), true);
		};
		virtual std::weak_ptr<Type_Info> MakeConstRef() const override {
			return MakeDuplicate(true, true);
		};
		virtual std::weak_ptr<Type_Info> RemoveConst() const override {
			return MakeDuplicate(false, is_ref());
		};
		virtual std::weak_ptr<Type_Info> RemoveRef() const override {
			return MakeDuplicate(is_const(), false);
		};

	protected:
		std::string m_full_name; // namespace::name
		std::string m_qualified_namespace; // namespace
		std::string m_name; // name
		size_t m_uniqueHash; // std::hash<std::string>()(m_full_name)
		
		std::weak_ptr<Scripted_Type_Info> m_self;
		std::weak_ptr<Scripted_Type_Info> m_parent;
		mutable std::shared_mutex m_children_mut;
		mutable std::unordered_map<size_t, std::shared_ptr<Scripted_Type_Info>> m_children;
		std::weak_ptr<Type_Info> MakeDuplicate(bool targetConst, bool targetRef) const {
			size_t targetHash = this->GetHashImpl();
			details::hash_combine(targetHash, (size_t)targetConst, (size_t)targetRef);

			if (this->GetHash() == targetHash) {
				return m_self;
			}
			else if (auto parentPtr = m_parent.lock()) {
				return parentPtr->MakeDuplicate(targetConst, targetRef);
			}
			else {
				if (1) {
					auto locked{ std::shared_lock(m_children_mut) };
					auto p = m_children.find(targetHash);
					if (p != m_children.end()) {
						return std::dynamic_pointer_cast<Type_Info>(p->second);
					}
				}

				if (1) {
					auto locked{ std::unique_lock(m_children_mut) };
					auto p = m_children.find(targetHash);
					if (p != m_children.end()) {
						return std::dynamic_pointer_cast<Type_Info>(p->second);
					}
					else {
						auto out = std::make_shared<Scripted_Type_Info>(m_qualified_namespace, m_name, targetConst, targetRef);
						out->SetSelf(out);
						out->m_parent = this->m_self;

						this->m_children.insert({ targetHash, out });
						return std::dynamic_pointer_cast<Type_Info>(out);
					}
				}
			}
		};

	};
};

// Type_Info std::hash
namespace std {
	template <> struct hash<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& k) const {
			return k.GetHash();
		};
	};
	template <> struct hash<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& k) const {
			if (k) {
				return k->GetHash();
			}
			else {
				return GoodLang::impl::TypeId<void>().hash_code();
			}
		};
	};
	template <> struct hash<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& k) const {
			if (auto p = k.lock()) {
				return p->GetHash();
			}
			else {
				return GoodLang::impl::TypeId<void>().hash_code();
			}
		};
	};
};

// GetHash
namespace GoodLang {
	template <typename T> auto& GetHash() {
		static auto hasher{ std::hash<typename details::Bare_Type<T>::type>{} };
		return hasher;
	};
	template <typename T> size_t GetHash(const T& a) {
		return GetHash<T>()(a);
	};
};

// Type_Info std::less, std::greater, std::equal_to
namespace std {
	template <> struct less<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& lhs, const GoodLang::Type_Info& rhs) const {
			return GoodLang::GetHash(lhs) < GoodLang::GetHash(rhs);
		};
	};
	template <> struct less<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& lhs, const std::shared_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) < GoodLang::GetHash(rhs);
		};
	};
	template <> struct less<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& lhs, const std::weak_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) < GoodLang::GetHash(rhs);
		};
	};

	template <> struct greater<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& lhs, const GoodLang::Type_Info& rhs) const {
			return GoodLang::GetHash(lhs) > GoodLang::GetHash(rhs);
		};
	};
	template <> struct greater<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& lhs, const std::shared_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) > GoodLang::GetHash(rhs);
		};
	};
	template <> struct greater<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& lhs, const std::weak_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) > GoodLang::GetHash(rhs);
		};
	};

	template <> struct equal_to<GoodLang::Type_Info> {
		std::size_t operator()(const GoodLang::Type_Info& lhs, const GoodLang::Type_Info& rhs) const {
			return GoodLang::GetHash(lhs) == GoodLang::GetHash(rhs);
		};
	};
	template <> struct equal_to<std::shared_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::Type_Info>& lhs, const std::shared_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) == GoodLang::GetHash(rhs);
		};
	};
	template <> struct equal_to<std::weak_ptr<GoodLang::Type_Info>> {
		std::size_t operator()(const std::weak_ptr<GoodLang::Type_Info>& lhs, const std::weak_ptr<GoodLang::Type_Info>& rhs) const {
			return GoodLang::GetHash(lhs) == GoodLang::GetHash(rhs);
		};
	};


};

// user_type & user_type_shared
namespace GoodLang {
	namespace details {
		/// Helper used to create a Type_Info object
		template<typename T>
		struct Get_Type_Info {
			static std::shared_ptr<BuiltIn_Type_Info<T>> get() noexcept {
				return std::make_shared<BuiltIn_Type_Info<T>>(
					impl::TypeId<T>(),
					std::is_const<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value,
					std::is_reference<typename std::remove_pointer<T>::type>::value
					);
			}
		};

		template<typename T> struct Get_Type_Info<std::shared_ptr<T>> : Get_Type_Info<T> {};

		template<typename T> struct Get_Type_Info<std::shared_ptr<T>&> : Get_Type_Info<std::shared_ptr<T>> {};

		template<typename T> struct Get_Type_Info<const std::shared_ptr<T>&> : Get_Type_Info<T> {};
	} // namespace detail

	/// \brief Creates a Type_Info object representing the templated type
	/// \tparam T Type of object to get a Type_Info for
	/// \return Type_Info for T
	///
	/// \b Example:
	/// \code
	/// chaiscript::Type_Info ti = chaiscript::user_type<int>();
	/// \endcode
	template<typename T> std::weak_ptr<Type_Info> user_type_shared() noexcept {
		static std::shared_ptr<Type_Info> out{ std::dynamic_pointer_cast<Type_Info>(details::Get_Type_Info<T>::get()) };
		return out;
	};

	/// \brief Creates a Type_Info object representing the templated type
	/// \tparam T Type of object to get a Type_Info for
	/// \return Type_Info for T
	///
	/// \b Example:
	/// \code
	/// chaiscript::Type_Info ti = chaiscript::user_type<int>();
	/// \endcode
	template<typename T> const Type_Info& user_type() noexcept {
		static std::shared_ptr<Type_Info> out{ user_type_shared<T>().lock() };
		static const Type_Info& toReturn{ *out };
		return toReturn;
	};

	/// \brief Creates a Type_Info object representing the type passed in
	/// \tparam T Type of object to get a Type_Info for, derived from the passed in parameter
	/// \return Type_Info for T
	///
	/// \b Example:
	/// \code
	/// int i;
	/// chaiscript::Type_Info ti = chaiscript::user_type(i);
	/// \endcode
	template<typename T> const Type_Info& user_type(const T& /*t*/) noexcept {
		return user_type<T>();
	};
};

__forceinline bool operator==(std::weak_ptr<GoodLang::Type_Info> const& a, std::weak_ptr<GoodLang::Type_Info> const& b) {
	return GetHash(a) == GetHash(b);
};
__forceinline bool operator!=(std::weak_ptr<GoodLang::Type_Info> const& a, std::weak_ptr<GoodLang::Type_Info> const& b) {
	return !operator==(a, b);
};
__forceinline bool operator==(std::weak_ptr<GoodLang::Type_Info> const& a, GoodLang::Type_Info const& b) {
	return GetHash(a) == GetHash(b);
};
__forceinline bool operator!=(std::weak_ptr<GoodLang::Type_Info> const& a, GoodLang::Type_Info const& b) {
	return !operator==(a, b);
};
__forceinline bool operator==(GoodLang::Type_Info const& b, std::weak_ptr<GoodLang::Type_Info> const& a) {
	return GetHash(a) == GetHash(b);
};
__forceinline bool operator!=(GoodLang::Type_Info const& b, std::weak_ptr<GoodLang::Type_Info> const& a) {
	return !operator==(a, b);
};

// Any, AnyAutoCast, DynamicObject, exceptions
namespace GoodLang {
	namespace exception {
		/// \brief Thrown in the event that an Any cannot be cast to the desired type
		/// It is used internally during function dispatch.
		class bad_any_cast : public std::bad_cast {
		public:
			/// \brief Description of what error occurred
			const char* what() const noexcept override { return exc.c_str(); }

			bad_any_cast() :
				bad_cast(std::bad_cast::__construct_from_string_literal("Bad Any Cast")),
				from(user_type<void>()),
				to(user_type<void>()),
				exc("Bad Any Cast")
			{};
			bad_any_cast(GoodLang::Type_Info const& from_m, GoodLang::Type_Info const& to_m, int lineNumber) :
				bad_cast(std::bad_cast::__construct_from_string_literal((std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber)).c_str())),
				from(from_m),
				to(to_m),
				exc(std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber))
			{};
			bad_any_cast(std::weak_ptr<GoodLang::Type_Info> from_m, std::weak_ptr<GoodLang::Type_Info> to_m, int lineNumber) :
				bad_cast(std::bad_cast::__construct_from_string_literal((std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber)).c_str())),
				from(TypeFromPtr(from_m)),
				to(TypeFromPtr(to_m)),
				exc(std::string("Bad Any Cast From \"") + NameFromType(from_m) + "\" To \"" + NameFromType(to_m) + "\" at line " + std::to_string(lineNumber))
			{};
		private:
			std::string exc;
			GoodLang::Type_Info const& from;
			GoodLang::Type_Info const& to;

			static std::string NameFromType(GoodLang::Type_Info const& x) { return x.name(); };
			static std::string NameFromType(std::weak_ptr<GoodLang::Type_Info> const& x) { if (auto y = x.lock()) return y->name(); else return user_type<void>().name(); };
			static GoodLang::Type_Info const& TypeFromPtr(std::weak_ptr<GoodLang::Type_Info> const& x) { if (auto y = x.lock()) return *y; else return user_type<void>(); };
		};

		/**
		* Exception thrown when there is a mismatch in number of
		* parameters during Proxy_Function execution
		*/
		struct arity_error : std::range_error {
			arity_error(int t_got, int t_expected)
				: std::range_error(
					t_expected >= 0 ?
					Units::printf("Arity mismatch: function requires %i parameters, but only %i were provided", t_expected, t_got)
					: std::string("Function was not found")
				)
				, got(t_got)
				, expected(t_expected) {
			}

			arity_error(const arity_error&) = default;

			~arity_error() noexcept override = default;

			int got;
			int expected;
		};

		/**
		* Exception thrown when there is a mismatch in number of
		* parameters during Proxy_Function execution
		*/
		struct not_found_error : std::runtime_error {
			not_found_error(const std::string& triedToFind)
				: std::runtime_error(
					Units::printf("Could not find \"%s\"", triedToFind.c_str())
				), m_triedToFind(triedToFind)
			{}
			not_found_error(const not_found_error&) = default;
			~not_found_error() noexcept override = default;

			std::string m_triedToFind;
		};
	}; // namespace exception

	namespace details {
		template<class T> struct get_type { using type = T; };
		template<class T> struct get_type<std::shared_ptr<T>> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<std::shared_ptr<T>&> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<std::shared_ptr<T>*> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>&> { using type = typename get_type<T>::type; };
		template<class T> struct get_type<const std::shared_ptr<T>*> { using type = typename get_type<T>::type; };
	};

	class AnyData {
	public:
		AnyData() noexcept = default;
		AnyData(AnyData const&) = default;
		AnyData(AnyData&&) = default;
		AnyData& operator=(AnyData const&) = default;
		AnyData& operator=(AnyData&&) = default;
		virtual ~AnyData() = default;

	public:
		// user must set the ptr to the AnyData object, so that it is aware of itself
		void SetSelf(std::shared_ptr< AnyData>& t_self) {
			m_self = t_self;
			typeHash = GetHash(GetType());
		};

	public:
		size_t GetTypeHash() const { return typeHash; };
		virtual bool CanCast(Type_Info const& to_type) const { return false; };
		virtual Type_Info const& GetType() const { return user_type<void>(); };
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const { return user_type_shared<void>(); };
		virtual void* ptr() const { return nullptr; };
		virtual std::shared_ptr<void> shared_ptr() const { return nullptr; };
		template<typename ToType> std::shared_ptr<ToType> cast_shared() const {
			Type_Info const& to_type{ user_type<ToType>() };

			if (CanCast(to_type)) {
				return std::static_pointer_cast<ToType>(shared_ptr());
			}
			else {
				return nullptr;
			}
		};
		template<typename ToType> ToType* cast() const {
			Type_Info const& to_type{ user_type<ToType>() };

			if (CanCast(to_type)) {
				return static_cast<ToType*>(ptr());
			}
			else {
				return nullptr;
			}
		};
		void ThrowIfNot(Type_Info const& type) const {
			if (!CanCast(type)) {
				throw exception::bad_any_cast(GetType(), type, __LINE__);
			}
		};
		template<typename T> static std::shared_ptr<void> get_data(const std::shared_ptr<T>& data) {
			if constexpr (std::is_const< T >::value) {
				return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(data));
			}
			else {
				return std::static_pointer_cast<void>(data);
			}
		};
		template<typename T> static std::shared_ptr<void> get_data(std::shared_ptr<T>&& data) {
			if constexpr (std::is_const< T >::value) {
				return std::const_pointer_cast<void>(std::static_pointer_cast<const void>(std::forward<std::shared_ptr<T>>(data)));
			}
			else {
				return std::static_pointer_cast<void>(std::forward<std::shared_ptr<T>>(data));
			}
			// return std::static_pointer_cast<void>(std::const_pointer_cast<std::remove_const_t<T>>(std::forward<std::shared_ptr<T>>(data)));
		};

	protected:
		std::weak_ptr< AnyData> m_self;
		size_t typeHash;
	};

	template <typename T>
	class AnyData_Instanced final : public AnyData {
	public:
		AnyData_Instanced() noexcept = default;
		AnyData_Instanced(T t_obj) noexcept
			: AnyData()
			, m_obj{ std::move(t_obj) }
		{ };
		AnyData_Instanced(AnyData_Instanced const&) = default;
		AnyData_Instanced(AnyData_Instanced&&) = default;
		AnyData_Instanced& operator=(AnyData_Instanced const&) = default;
		AnyData_Instanced& operator=(AnyData_Instanced&&) = default;
		virtual ~AnyData_Instanced() = default;

		virtual bool CanCast(Type_Info const& to_type) const override { return GetType().CanCast(to_type); };
		virtual Type_Info const& GetType() const override { return user_type<T>(); };
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const override { return user_type_shared<T>(); };
		virtual void* ptr() const override { return static_cast<void*>(&const_cast<std::remove_const_t<T>&>(m_obj)); };
		virtual std::shared_ptr<void> shared_ptr() const override {
			if (auto p = m_self.lock()) {
				auto P = std::shared_ptr<T>(&const_cast<std::remove_const_t<T>&>(m_obj), [PTR = p](T* toDelete) { (void)PTR->ptr(); /* ThrowIfNot(user_type<T>());*/ });
				return std::static_pointer_cast<void>(std::const_pointer_cast<std::remove_const_t<T>>(P));
			}
			return nullptr;
		};

	private:
		T m_obj;

	};

	template <typename T>
	class AnyData_Shared final : public AnyData {
	public:
		AnyData_Shared() noexcept = default;
		AnyData_Shared(std::shared_ptr<T> const& t_obj) noexcept
			: AnyData()
			, m_obj(t_obj)
		{ };
		AnyData_Shared(std::shared_ptr<T>&& t_obj) noexcept
			: AnyData()
			, m_obj(std::forward<std::shared_ptr<T>>(t_obj))
		{ };
		AnyData_Shared(AnyData_Shared const&) = default;
		AnyData_Shared(AnyData_Shared&&) = default;
		AnyData_Shared& operator=(AnyData_Shared const&) = default;
		AnyData_Shared& operator=(AnyData_Shared&&) = default;
		virtual ~AnyData_Shared() = default;

		virtual bool CanCast(Type_Info const& to_type) const override { return GetType().CanCast(to_type); };
		virtual Type_Info const& GetType() const override { return user_type<T>(); };
		virtual std::weak_ptr<Type_Info> const& GetTypeShared() const override { return user_type_shared<T>(); };
		virtual void* ptr() const override { return m_obj.get(); };
		virtual std::shared_ptr<void> shared_ptr() const override { return std::static_pointer_cast<void>(m_obj); };

	private:
		std::shared_ptr<T> m_obj;

	};

	namespace details {
		class AnyAutoCast; /* forward decl */
	};
	
	// serves as an instance of a customizable class
	class DynamicObject {
	public:
		DynamicObject() = default;
		DynamicObject(std::weak_ptr< Type_Info > const& type)
			: m_classType(type)
			, m_objects()
		{};
		DynamicObject(DynamicObject const&) = default;
		DynamicObject(DynamicObject&&) = default;
		DynamicObject& operator=(DynamicObject const&) = default;
		DynamicObject& operator=(DynamicObject&&) = default;
		~DynamicObject() = default;

		std::weak_ptr< Type_Info >
			m_classType;
		concurrency::concurrent_unordered_map<std::string, std::shared_ptr<Any>>
			m_objects;
	};

	/*! Generic container that enables the containment and sharing of any data type to/from std::shared_ptrs */
	class Any {
	public:
		struct Object_Data {
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(const H<S>* obj) { return get(*obj); };
			template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, std::shared_ptr<S>>>> static decltype(auto) get(H<S> obj) {
				if (obj) {
					if constexpr (std::is_same<Any, S>::value) {
						return obj->container;
					}
					else {
						std::shared_ptr<AnyData> instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Shared<S>>(std::move(obj)));
						instanced_any->SetSelf(instanced_any);
						return instanced_any;
					}
				}
				else {
					return std::shared_ptr<AnyData>();
				}
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static decltype(auto) get(T* t) { return get(std::make_shared<T>(t)); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static decltype(auto) get(const T* t) { return get(*t); };
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static decltype(auto) get(const T& obj) {
				return get((std::decay_t<T>)obj);
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<GoodLang::details::AnyAutoCast, T>>> static decltype(auto) get(T&& obj) {
				std::shared_ptr<AnyData> instanced_any = std::dynamic_pointer_cast<GoodLang::AnyData>(std::make_shared<GoodLang::AnyData_Instanced<std::decay_t<T>>>(std::forward<T>(obj)));
				instanced_any->SetSelf(instanced_any);
				return instanced_any;
			};

			static decltype(auto) get(const GoodLang::details::AnyAutoCast& obj);
			static decltype(auto) get(const GoodLang::details::AnyAutoCast* t);
		};
		template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(const ValueType& r) { return Object_Data::get(r); };
		template<typename ValueType> static std::shared_ptr<AnyData> CreateContainer(ValueType&& r) { return Object_Data::get(std::forward<ValueType>(r)); };

	public: /*! Init */
		Any() noexcept
			: container(nullptr)
			, mut()
		{};
		Any(std::nullptr_t) noexcept
			: container(nullptr)
			, mut()
		{};
		Any(const Any& rhs) noexcept
			: container()
			, mut()
		{
			auto locked2{ std::shared_lock(rhs.mut) };
			container = rhs.container;
		};
		Any(Any&& rhs) noexcept
			: container(std::move(rhs.container))
			, mut()
		{};

	public: /*! Init w/ DATA ASSIGNMENT */
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType& value) noexcept
			: container(CreateContainer(value))
			, mut()
		{};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(const ValueType* value) noexcept
			: container(CreateContainer(value))
			, mut()
		{};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType* value) noexcept
			: container(CreateContainer(value))
			, mut()
		{};
		template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any(ValueType&& value) noexcept
			: container(CreateContainer(std::forward<ValueType>(value)))
			, mut()
		{};

	public: /*! Destroy */
		~Any() = default; // { Clear(); };

	public: /*! Data Assignment AFTER INIT */
		Any& swap(Any& rhs) noexcept {
			if (this == &rhs) { return *this; }

			auto locked{ std::unique_lock(mut) };
			auto locked2{ std::unique_lock(rhs.mut) };

			container.swap(rhs.container);
			return *this;
		};
		Any& operator=(const Any& rhs) noexcept {
			if (this == &rhs) { return *this; }

			auto locked{ std::unique_lock(mut) };
			auto locked2{ std::shared_lock(rhs.mut) };

			container = rhs.container;
			return *this;
		};
		Any& operator=(Any&& rhs) noexcept {
			auto locked{ std::unique_lock(mut) };
			auto locked2{ std::shared_lock(rhs.mut) };

			container = rhs.container;
			return *this;
		};
		Any& operator=(std::nullptr_t) noexcept { Clear(); return *this; };

		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(const ValueType& rhs) noexcept {
			auto newContainer = CreateContainer(rhs);
			mut.lock();
			container.swap(newContainer);
			mut.unlock();
			return *this;
		};
		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(const ValueType* rhs) noexcept {
			auto newContainer = CreateContainer(rhs);
			mut.lock();
			container.swap(newContainer);
			mut.unlock();
			return *this;
		};
		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(ValueType* rhs) noexcept {
			auto newContainer = CreateContainer(rhs);
			mut.lock();
			container.swap(newContainer);
			mut.unlock();
			return *this;
		};
		template <class ValueType, typename = std::enable_if_t<!std::is_same_v<Any, std::decay_t<ValueType>>>> Any& operator=(ValueType&& rhs) noexcept {
			auto newContainer = CreateContainer(std::forward<ValueType>(rhs));			
			mut.lock();
			container.swap(newContainer);
			mut.unlock();			
			return *this;
		};

	public:
		/*! Checks if the Any has been assigned something */
		bool IsEmpty() const noexcept {
			auto locked{ std::shared_lock(mut) };
			return (bool)container;
		};

		/*! Empties the Any and frees the memory. */
		void Clear() noexcept {
			auto locked{ std::unique_lock(mut) };
			container = nullptr;
		};

		template <typename ValueT> static const char* TypeNameOf() { return TypeOf<ValueT>().name(); };
		template <typename ValueT> static Type_Info TypeOf() { return user_type<ValueT>(); };

		std::string TypeName() const noexcept {
			if (auto p = Type().lock()) {
				return p->name();
			}
			else {
				return user_type<void>().name();
			}
		};
		std::weak_ptr<Type_Info> Type() const noexcept {
			static auto DynamicTypeHash{ GetHash(user_type<DynamicObject>()) };
			auto locked{ std::shared_lock(mut) };
			if (std::shared_ptr<AnyData>& m = container) {
				if (m->GetTypeHash() == DynamicTypeHash) {
					if (auto p2 = m->cast< DynamicObject>()) {
						return p2->m_classType;
					}
				}
				return m->GetTypeShared();
			}
			else {
				return user_type_shared<void>();
			}
		};
		size_t TypeHash() const noexcept {
			static auto DynamicTypeHash{ GetHash(user_type<DynamicObject>()) };

			auto locked{ std::shared_lock(mut) };
			if (std::shared_ptr<AnyData>& m = container) {
				if (m->GetTypeHash() == DynamicTypeHash) {
					if (auto p2 = m->cast< DynamicObject>()) {
						if (auto p3 = p2->m_classType.lock()) {
							return p3->GetHash();
						}
					}
				}
				return m->GetTypeHash();
			}
			else {
				static auto SharedT{ GetHash(user_type<void>()) };
				return SharedT;
			}
		};
		bool IsTypeOf(std::weak_ptr<Type_Info> const& targetType) const noexcept {
			static auto hasher{ std::hash<std::weak_ptr<Type_Info>>() };
			return TypeHash() == hasher(targetType);
		};
		bool IsTypeOf(Type_Info const& targetType) const noexcept {
			static auto hasher{ std::hash<Type_Info>() };
			return TypeHash() == hasher(targetType);
		};
		template<typename VType> bool IsTypeOf() const noexcept {
			return IsTypeOf(user_type<typename std::decay_t<VType>>());
		};

#pragma region Boolean Operators
	public:
		explicit operator bool() const { auto locked{ std::shared_lock(mut) }; return (bool)container; };
		friend bool operator==(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container == b.container; };
		friend bool operator!=(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container != b.container; };
		friend bool operator<(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container < b.container; };
		friend bool operator<=(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container <= b.container; };
		friend bool operator>(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container > b.container; };
		friend bool operator>=(const Any& a, const Any& b) noexcept { auto locked{ std::shared_lock(a.mut) }; auto locked2{ std::shared_lock(b.mut) }; return a.container >= b.container; };
		friend bool operator==(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container == nullptr; };
		friend bool operator!=(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container != nullptr; };
		friend bool operator<(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container < nullptr; };
		friend bool operator<=(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container <= nullptr; };
		friend bool operator>(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container > nullptr; };
		friend bool operator>=(const Any& a, std::nullptr_t) noexcept { auto locked{ std::shared_lock(a.mut) }; return a.container >= nullptr; };
		friend bool operator==(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr == a.container; };
		friend bool operator!=(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr != a.container; };
		friend bool operator<(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr < a.container; };
		friend bool operator<=(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr <= a.container; };
		friend bool operator>(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr > a.container; };
		friend bool operator>=(std::nullptr_t, const Any& a) noexcept { auto locked{ std::shared_lock(a.mut) }; return nullptr >= a.container; };
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
				auto locked{ std::shared_lock(p->mut) };
				if (p->container) {
					return p->container->cast_shared<VType>();
				}
				else {
					return std::shared_ptr<VType>{ nullptr };
				}
			};
			template <class VType> static decltype(auto) DoCast_Shared_Sentinel(Any* p) noexcept {
				throw("Casting Any to  std::shared_ptr<T>* or  std::shared_ptr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to std::shared_ptr<T>.");
			};
			template<typename VType> static decltype(auto) DoCast_Unshared(Any* p) noexcept {
				constexpr bool is_ptr = std::is_pointer_v<VType>;

				auto locked{ std::shared_lock(p->mut) };
				if (p->container) {
					if constexpr (is_ptr) {
						return p->container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
					}
					else {
						return *p->container->cast< typename std::remove_reference<typename std::remove_pointer<VType>::type>::type >();
					}
				}
				else {
					if constexpr (is_ptr) {
						return (typename std::remove_reference<typename std::remove_pointer<VType>::type>::type *)nullptr;
					}
					else {
						throw exception::bad_any_cast(p->Type(), user_type_shared<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>(), __LINE__);
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
					typedef typename details::get_type<T>::type innertype;
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

		details::AnyAutoCast cast() const noexcept;

		std::shared_ptr<AnyData> impl() const {
			auto locked{ std::shared_lock(mut) };
			return container;
		};
	private:
		mutable std::shared_ptr<AnyData> container;
		mutable std::shared_mutex mut;
	};

	namespace details {
		/*! Supports forward-declaring a "cast" from an Any to the desired destination type. e.g: int& ref_int = any_obj.cast(); ... std::string str = any_obj.cast(); */
		class AnyAutoCast {
		public:
			AnyAutoCast(const Any* _parent) 
				: parent(const_cast<Any*>(_parent))
				//, parentCopy(_parent->impl())
			{};
			AnyAutoCast(AnyAutoCast&& other) 
				: parent(std::move(other.parent))
				//, parentCopy(std::move(other.parentCopy))
			{};

			AnyAutoCast() = delete;
			AnyAutoCast(const AnyAutoCast&) = delete;
			AnyAutoCast& operator=(const AnyAutoCast&) = delete;
			AnyAutoCast& operator=(AnyAutoCast&&) = delete;
			~AnyAutoCast() {};

			explicit operator Any& () const noexcept { return *parent; };
			explicit operator Any* () const noexcept { return parent; };

			template <typename T>
			operator std::shared_ptr<T>() const noexcept { return parent->cast<std::shared_ptr<T>>(); };

			template <typename T>
			operator std::shared_ptr<T>* () const noexcept { return parent->cast<std::shared_ptr<T>*>(); };

			template< bool cond, typename U >
			using resolvedType = typename std::enable_if< cond, U >::type;

			template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
			operator ValueTypeT& () const noexcept { return parent->cast<ValueTypeT&>(); };

			template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!Any::DataCaster::is_SharedPtr_class<ValueTypeT>::type::value> >
			operator ValueTypeT* () const noexcept { return parent->cast<ValueTypeT*>(); };

			Any* parent;
			// std::shared_ptr<AnyData> parentCopy;
		};
	};

	/*! Casts to whatever is on the left-hand-side, with specializations for references, pointers, values, and std::shared_ptrs. References and pointers are lifetime-sensitive. */
	__forceinline details::AnyAutoCast Any::cast() const noexcept { return details::AnyAutoCast(this); };
	__forceinline decltype(auto) Any::Object_Data::get(const details::AnyAutoCast& obj) {
		Any* t = const_cast<Any*>(obj.parent);
		if (t) {
			auto locked{ std::shared_lock(t->mut) };
			return t->container;
		}
		return std::shared_ptr<AnyData>{ nullptr };
	};
	__forceinline decltype(auto) Any::Object_Data::get(const details::AnyAutoCast* t) { return get(*t); };

};

// Type_Conversion_Base, its impl's, & TypeConverter wrapper
namespace GoodLang {
	namespace details {
		// Tuning parameter. Should be larger than the slowest conversion time. Large values encourages fewer conversions. Smaller values encourages faster conversions.
		static constexpr auto TypeConversionBaselineCost = 1000.0;
		static constexpr auto TypeConversionWorstCaseCost = 1000000000000.0;
		class Type_Conversion_Base {
		public:
			// Converts From -> To, and places the result into the "From" container. Useful for faster conversions.
			virtual void convert_in_place(Any& from) const = 0;
			// From -> To
			virtual Any convert(const Any& from) const = 0;
			// To -> From
			virtual Any convert_down(const Any& to) const = 0;

			// returns the actual time (in nanoseconds) to perform the conversion
			virtual double cost() const noexcept { return 0; };

			// to type
			const std::weak_ptr<Type_Info>& to() const noexcept { return m_to; }

			// from type
			const std::weak_ptr<Type_Info>& from() const noexcept { return m_from; }

			// is bidirectional?
			virtual bool bidir() const noexcept { return true; }

			// is polymorphic conversion?
			virtual bool polymorphic() const noexcept { return false; }

			virtual std::string print() const noexcept { return std::string(" ... ") + this->to().lock()->name(); };

			virtual ~Type_Conversion_Base() = default;

			virtual bool IsDaisyChained() const { return false; };

			virtual size_t NumConversions() const { return 1; }
		protected:
			Type_Conversion_Base(std::weak_ptr<Type_Info> t_to, std::weak_ptr<Type_Info> t_from) : m_to(t_to), m_from(t_from) {}

		private:
			std::weak_ptr<Type_Info> m_to;
			std::weak_ptr<Type_Info> m_from;
		};

		template<class Callable>
		class Custom_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			using ReturnType = typename fibers::utilities::function_traits< typename decltype(std::function(std::declval<Callable>())) >::result_type;
			using InputType = typename /*std::decay_t<*/std::tuple_element_t<0, typename fibers::utilities::function_traits< typename decltype(std::function(std::declval<Callable>())) >::arguments>/*>*/;

		public:
			Custom_Type_Conversion_Impl(Callable t_func)
				: Type_Conversion_Base(
					user_type_shared<ReturnType>(),
					user_type_shared<InputType>()
				)
				, m_func(std::move(t_func))
				, m_cost(std::nullopt)
			{};
			Custom_Type_Conversion_Impl(Callable t_func, std::weak_ptr<Type_Info> inboundType, std::weak_ptr<Type_Info> outboundType, std::optional<double> Cost = std::nullopt)
				: Type_Conversion_Base(
					outboundType,
					inboundType
				)
				, m_func(std::move(t_func))
				, m_cost(std::move(Cost))
			{};

			// To -> From
			Any convert_down(const Any&) const override {
				throw std::runtime_error("Custom_Type_Conversion_Impl is not bidirectional.");
			};

			// From -> To
			void convert_in_place(Any& t_from) const override {
				if constexpr (std::is_convertible<decltype(t_from), InputType>::value) {
					t_from = m_func(t_from);
				}
				else {
					t_from = m_func(t_from.cast());
				}
			};

			// From -> To
			Any convert(const Any& t_from) const override {
				if constexpr (std::is_convertible<decltype(t_from), InputType>::value) {
					return m_func(t_from);
				}
				else {
					return m_func(t_from.cast());
				}
			};

			bool bidir() const noexcept override { return false; }

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override {
				if (m_cost.has_value()) {
					if (m_cost.value() == 0) {
						return 0;// m_cost.value();
					}
					else {
						return TypeConversionBaselineCost;// + m_cost.value();
					}
				}
				else {
					//static double actualCost{ -1 };
					//static std::decay_t<InputType> inputObj{};
					//if (actualCost < 0) {
					//	double temp{ 0 };
					//	for (int i = 0; i < 10; i++) {
					//		auto startT = clock_ns();
					//		(void)(m_func(inputObj));
					//		temp += (double)(clock_ns() - startT) / 100.0;
					//	}
					//	actualCost = TypeConversionBaselineCost + temp / 10.0;
					//}
					//return actualCost;

					return TypeConversionBaselineCost;
				}
			};

		private:
			Callable m_func;
			std::optional<double> m_cost;
		};

		class DaisyChained_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			DaisyChained_Type_Conversion_Impl(std::vector<std::shared_ptr<Type_Conversion_Base>> const& t_converters)
				: Type_Conversion_Base(
					t_converters[t_converters.size() - 1]->to(),
					t_converters[0]->from()
				)
				, m_converters(t_converters)
				, m_cost(0) /*TypeConversionBaselineCost*/
			{
				//for (auto& converter : t_converters) {
				//	m_converters.push_back(converter);
				//}

				for (auto& converter : t_converters) {
					m_cost += converter->cost();
				}
			};

			// To -> From
			Any convert_down(const Any&) const override {
				throw std::runtime_error("DaisyChained_Type_Conversion_Impl is not bidirectional.");
			};

			// From -> To
			void convert_in_place(Any& t_from) const override {
				for (auto& converter : m_converters) {
					if (auto& p = converter/*.lock()*/) {
						p->convert_in_place(t_from);
					}
					else {
						throw exception::bad_any_cast(this->from(), this->to(), __LINE__);
					}
				}
			};

			// From -> To
			Any convert(const Any& t_from) const override {
				Any out = t_from;
				for (auto& converter : m_converters) {
					if (auto& p = converter/*.lock()*/) {
						p->convert_in_place(out);
					}
					else {
						throw exception::bad_any_cast(this->from(), this->to(), __LINE__);
					}
				}
				return out;
			};

			bool bidir() const noexcept override { return false; }

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override {
				return m_cost;
			};

			virtual std::string print() const noexcept override {
				std::string out;
				for (auto& converter : m_converters) {
					out += converter->print();
				}
				return out;
			};
			virtual bool IsDaisyChained() const override { return true; };
			virtual size_t NumConversions() const override { return m_converters.size(); }
		private:
			std::vector<std::shared_ptr<Type_Conversion_Base>> m_converters;
			double m_cost;
		};

		namespace impl {
			template <class From, class To, class = void>
			struct is_explicitly_convertible_to_impl : std::false_type {};

			template <class From, class To>
			struct is_explicitly_convertible_to_impl<From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>> : std::true_type {};

			template <class From, class To>
			struct is_explicitly_convertible_to : is_explicitly_convertible_to_impl<From, To> {};

			template <class From, class To>
			inline constexpr bool is_explicitly_convertible_to_v = is_explicitly_convertible_to<From, To>::value;

		};

		template<typename From, typename To>
		class Static_Type_Conversion_Impl : public Type_Conversion_Base {
		private:
			constexpr static bool is_bidir = impl::is_explicitly_convertible_to<To, From>::value;

		public:
			Static_Type_Conversion_Impl()
				: Type_Conversion_Base(user_type_shared<To>(), user_type_shared<From>())
			{};

			// To -> From
			Any convert_down(const Any& t_to) const override {
				if constexpr (is_bidir) {
					return (From)(t_to.cast<To const&>());
				}
				else {
					throw std::runtime_error("Static_Type_Conversion_Impl was not bidirectional.");
				}
			};

			// From -> To
			void convert_in_place(Any& t_from) const override {
				t_from = (To)(t_from.cast<From const&>());
			};

			// From -> To
			Any convert(const Any& t_from) const override {
				return (To)(t_from.cast<From const&>());
			};

			bool bidir() const noexcept override { return is_bidir; }

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override {
				//static double actualCost{ -1 };
				//static std::decay_t<From> inputObj{};
				//if (actualCost < 0) {
				//	double temp{ 0 };
				//	for (int i = 0; i < 10; i++) {
				//		auto startT = clock_ns();
				//		(void)((To)(inputObj));
				//		temp += (double)(clock_ns() - startT) / 100.0;
				//	}
				//	actualCost = TypeConversionBaselineCost + temp / 10.0;
				//}
				return TypeConversionBaselineCost;//  actualCost;
			};
		};

		template<typename ChildType, typename BaseType>
		class Dynamic_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			Dynamic_Type_Conversion_Impl()
				: Type_Conversion_Base(user_type_shared<BaseType>(), user_type_shared<ChildType>())
			{};

			// BaseType -> ChildType
			Any convert_down(const Any& t_to) const override {
				throw std::runtime_error("Dynamic_Type_Conversion_Impl is never bidirectional (Base -> Child). Only may cast from (Child -> Base).");
			};

			// ChildType -> BaseType
			void convert_in_place(Any& t_from) const override {
				std::shared_ptr<ChildType> ptr{ t_from.cast<std::shared_ptr<ChildType>>() };
				t_from = std::dynamic_pointer_cast<BaseType>(ptr);
			};

			// ChildType -> BaseType
			Any convert(const Any& t_from) const override {
				std::shared_ptr<ChildType> ptr{ t_from.cast<std::shared_ptr<ChildType>>() };
				return std::dynamic_pointer_cast<BaseType>(ptr);
			};

			bool bidir() const noexcept override { return false; }

			double cost() const noexcept override { return 0; /* Assumes that dynamic casts are free */ };
		};

		// Create a wrapped user-defined function to cast provided types. Must have one (and only one) argument in the function. Argument may be an Any and do wild stuff.
		template<class Callable> __forceinline std::shared_ptr<Type_Conversion_Base> MakeConversionFunc(Callable func) {
			using CallableTypeAsStdFunc = decltype(std::function(std::declval<Callable>()));
			using CallableArguments = typename fibers::utilities::function_traits< CallableTypeAsStdFunc >::arguments;
			if constexpr (std::tuple_size_v< CallableArguments > != 1) {
				return nullptr;
			}
			else {
				using From = typename /*std::decay_t<*/std::tuple_element_t<0, CallableArguments>/*>*/;
				using To = typename fibers::utilities::function_traits< CallableTypeAsStdFunc >::result_type;
				constexpr static bool is_convertable = impl::is_explicitly_convertible_to<From, To>::value;
				constexpr static bool is_bidir_convertable = impl::is_explicitly_convertible_to<To, From>::value;
				constexpr static bool is_polymorphic = std::is_base_of<To, From>::value;

				return std::shared_ptr< Type_Conversion_Base >(new Custom_Type_Conversion_Impl(std::move(func)));
			}

		};

		// Create a function to cast from "From" to "To". Supports static or dynamic (polymorphic) casting. 
		template<typename From, typename To> __forceinline std::shared_ptr<Type_Conversion_Base> MakeConversionFunc() {
			constexpr static bool is_convertable = impl::is_explicitly_convertible_to<From, To>::value;
			constexpr static bool is_bidir_convertable = impl::is_explicitly_convertible_to<To, From>::value;
			constexpr static bool is_polymorphic = std::is_base_of<To, From>::value;

			if constexpr (is_polymorphic) {
				return std::shared_ptr<Type_Conversion_Base >(new Dynamic_Type_Conversion_Impl<From, To>());
			}
			else {
				if constexpr (is_convertable) {
					return std::shared_ptr< Type_Conversion_Base >(new Static_Type_Conversion_Impl<From, To>());
				}
				else {
					return std::shared_ptr< Type_Conversion_Base >(new Custom_Type_Conversion_Impl([](From const& i) -> To { return To(i); }));
				}
			}
		};
	};

	// Tree that manages a complex graph network of conversion opportunities. 
	// It's task is to organize those conversions, find the minimium or best conversion paths, and then cache the results. 
	// Best, most thread-safe use is to pre-populate the tree with converters before use. 
	// Performance-wise, it caches all potential conversions for each new type all at once, so beware small hick-ups in timing due to this. 
	// Can be fixed by pre-fetching all (or most) of the conversions you plan to use. 
	class TypeConverter {
	private:
		class UniformCostSearchNode {
		public:
			UniformCostSearchNode() = default;
			UniformCostSearchNode(std::shared_ptr<Type_Info> const& a, double const& b, std::vector<std::weak_ptr<Type_Info>> const& c)
				: thisVertexType(a)
				, distanceFromTarget(b)
				, bestPath(c)
			{};
			UniformCostSearchNode(UniformCostSearchNode&&) = default;
			UniformCostSearchNode(UniformCostSearchNode const&) = default;
			UniformCostSearchNode& operator=(UniformCostSearchNode&&) = default;
			UniformCostSearchNode& operator=(UniformCostSearchNode const&) = default;
			~UniformCostSearchNode() = default;
		public:
			std::shared_ptr<Type_Info> thisVertexType;
			double distanceFromTarget; // if not known, then we can simply guess. 
			std::vector<std::weak_ptr<Type_Info>> bestPath;

		public:
			bool operator()(const UniformCostSearchNode* a, const UniformCostSearchNode* b) const {
				return (a->bestPath.size() + 1) > (b->bestPath.size() + 1) || (a->distanceFromTarget > b->distanceFromTarget);
			};
			bool operator()(const std::shared_ptr<UniformCostSearchNode>& a, const std::shared_ptr<UniformCostSearchNode>& b) const {
				return (a->bestPath.size() + 1) > (b->bestPath.size() + 1) || (a->distanceFromTarget > b->distanceFromTarget);
			};
		};

	public:
		using TypeConverterFunc = std::shared_ptr< details::Type_Conversion_Base >;

		TypeConverter() = default;
		TypeConverter(TypeConverter const& rhs) {
			auto locked2{ std::shared_lock(const_cast<TypeConverter&>(rhs).AllConversionsMut) };
			AllConversions = rhs.AllConversions;
		};
		TypeConverter(TypeConverter&& rhs) {
			auto locked2{ std::unique_lock(rhs.AllConversionsMut) };
			AllConversions = std::move(rhs.AllConversions);
		};
		TypeConverter& operator=(TypeConverter const& rhs) {
			auto locked1{ std::unique_lock(AllConversionsMut) };
			auto locked2{ std::shared_lock(const_cast<TypeConverter&>(rhs).AllConversionsMut) };
			AllConversions = rhs.AllConversions;
		};
		TypeConverter& operator=(TypeConverter&& rhs) {
			auto locked1{ std::unique_lock(AllConversionsMut) };
			auto locked2{ std::unique_lock(rhs.AllConversionsMut) };
			AllConversions = std::move(rhs.AllConversions);
		};
		~TypeConverter() = default;

	public:
		std::string print() {
			std::string out;
			auto locked{ std::shared_lock(AllConversionsMut) };
			for (auto& conv : AllConversions) {
				out += (conv.first->name() + " (" + std::to_string(conv.first->GetHash()) + "): \n");
				for (auto& conv2 : conv.second) {
					out += (std::string("\t -> ") + conv2.first->name() + " (" + std::to_string(conv2.first->GetHash()) + ") " + " cost(" + std::to_string(conv2.second->cost()) + ") path(" + conv2.second->print() + ")\n");
				}
			}
			return out;
		};

	private:
		// All conversions, will include "real" and cached conversions.
		//using conversionTreeType = concurrency::concurrent_unordered_map< std::shared_ptr<Type_Info>, // From
		//	concurrency::concurrent_unordered_map< std::shared_ptr<Type_Info>, // To
		//	    TypeConverterFunc // Function
		//	>
		//>;
		using conversionTreeType = concurrency::concurrent_unordered_map< std::shared_ptr<Type_Info>, // From
			concurrency::concurrent_unordered_map< std::shared_ptr<Type_Info>, // To
			TypeConverterFunc // Function
			>
		>;
		conversionTreeType AllConversions;
		std::shared_mutex AllConversionsMut;

		// may return nullptr
		TypeConverterFunc GetExistingConverter(std::weak_ptr<Type_Info> const& From, std::weak_ptr<Type_Info> const& To) {
			auto locked{ std::shared_lock(AllConversionsMut) };
			return AllConversions[From.lock()][To.lock()];
		};
		// may return nullptr if it could not be built
		TypeConverterFunc GetOrBuildConverter(std::weak_ptr<Type_Info> const& From, std::weak_ptr<Type_Info> const& To, bool forceBuild = false) {
			// Solves the Uniform Cost Search Algorithm to determine the shortest path for "From" to "To", puts the path in "Out", and returns true. 
			// If no path is possible, returns false.
			static auto CreateConversionPaths{ [](fibers::utilities::Allocator< UniformCostSearchNode>& alloc, conversionTreeType& AllConversions, std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To) {
				// create the shortest paths from "From" to all possible vertices. 

				std::unordered_map<std::shared_ptr<Type_Info>, UniformCostSearchNode*> vertices;

				if (1) {
					// create an empty vertex set
					std::priority_queue< UniformCostSearchNode*, std::vector<UniformCostSearchNode*>, UniformCostSearchNode > vertexSet;

					// Add the source vertex into the set
					vertexSet.push(alloc.Alloc(From, 0.0, std::vector<std::weak_ptr<Type_Info>>{}));

					// is the vertex set empty?
					while (vertexSet.size() != 0) {
						// extract the vertex with the smallest distance value from the set
						auto smallestDistanceNode = vertexSet.top();
						vertexSet.pop();

						// for each neighbor of the extracted vertex... 
						for (auto& connection : AllConversions[smallestDistanceNode->thisVertexType]) {
							if (connection.second) {
								// do not use daisy-chained functions as candidates for new ones, since it can be harder to determine the actual conversion chain length
								if (connection.second->IsDaisyChained()) continue;

								// Is the neighbor already in the vertex set? 
								auto& toType = connection.first;
								auto& toVertex = vertices[toType];
								if (!toVertex) {
									// Instance it before we start working with it on an as-needed basis
									auto path = std::vector<std::weak_ptr<Type_Info>>(smallestDistanceNode->bestPath);
									path.push_back(toType);
									toVertex = alloc.Alloc(toType, std::numeric_limits<double>::infinity(), std::vector<std::weak_ptr<Type_Info>>{ toType });
								}

								// calculate distance value for the neighbor vertex
								double conversionCost = connection.second->cost();
								if ((toVertex->bestPath.size() + 1) > (smallestDistanceNode->bestPath.size() + 1)) {
									toVertex->distanceFromTarget = (smallestDistanceNode->distanceFromTarget + conversionCost);

									toVertex->bestPath = smallestDistanceNode->bestPath;
									toVertex->bestPath.push_back(toVertex->thisVertexType);

									vertexSet.push(toVertex);
								}
								else if (toVertex->distanceFromTarget > (smallestDistanceNode->distanceFromTarget + conversionCost)) {
									toVertex->distanceFromTarget = (smallestDistanceNode->distanceFromTarget + conversionCost);

									toVertex->bestPath = smallestDistanceNode->bestPath;
									toVertex->bestPath.push_back(toVertex->thisVertexType);

									vertexSet.push(toVertex);
								}
							}
						}
					}
				}

				return vertices;
			} };

			if (!forceBuild) {
				if (auto ptr = GetExistingConverter(From, To)) {
					return ptr;
				}
			}

			// Add conversion for From to a large variety of types...
			if (1) {
				fibers::utilities::Allocator< UniformCostSearchNode> alloc;
				AllConversionsMut.lock_shared();
				auto conversions{ CreateConversionPaths(alloc, AllConversions, From.lock(), To.lock()) };

				// All of these are for "From"...
				for (auto& conversion : conversions) {
					auto& ToType = conversion.first; // To...

					auto& cost = conversion.second->distanceFromTarget; // cost
					auto& path = conversion.second->bestPath; // conversion path
					//std::vector<std::shared_ptr< details::Type_Conversion_Base >>& conversionPath = conversion.second->bestPathConverters;

					if (path.size() >= 1) {
						TypeConverterFunc converterPtr = AllConversions[From.lock()][ToType];
						if ((converterPtr && (converterPtr->NumConversions() < path.size())) || (converterPtr && (converterPtr->cost() <= cost))) {
							continue;
						}
						else {
							// make new function, get hard lock, insert	
							if (1) {
								// make a new converter function
								TypeConverterFunc newConverter; {
									// convert the "type path" to a actual daisy-chains of weak_ptrs to converter functions
									std::vector<std::shared_ptr<details::Type_Conversion_Base>> functors; {
										std::weak_ptr<Type_Info> currentNodeType = From;

										for (auto& nextNodeType : path) {
											auto& func = AllConversions[currentNodeType.lock()][nextNodeType.lock()];
											if (!func) { // something went wrong -- this conversion has failed.
												continue;
											}
											else {
												functors.push_back(func);
												currentNodeType = nextNodeType;
											}
										}

										if (ToType != currentNodeType) {
											// this failed -- unclear why, but it happened. 
											continue;
										}
									}
									if (functors.size() > 1) {
										newConverter = std::shared_ptr< details::Type_Conversion_Base >(new details::DaisyChained_Type_Conversion_Impl(functors));
									}
									else {
										continue; // do nothing, assuming either the conversion failed or the shorter version was obviously already in the list.
									}
								}

								AllConversionsMut.unlock_shared();
								// insert it (requires hard lock)
								if (newConverter) {
									auto locked{ std::unique_lock(AllConversionsMut) };
									converterPtr = AllConversions[From.lock()][ToType];
									if ((converterPtr && (converterPtr->NumConversions() < path.size())) || (converterPtr && (converterPtr->cost() <= cost))) {}

									//if (converterPtr && (converterPtr->cost() <= cost)) {}
									else {
										AllConversions[From.lock()][ToType] = newConverter;
									}
								}
								AllConversionsMut.lock_shared();
							}
						}
					}
				}
				AllConversionsMut.unlock_shared();
			}

			// try and get our target type back ... 
			return GetExistingConverter(From, To);
		};
	private:
		// Base -> const Base
		// Base -> Base&
		// Base -> const Base&
		// const Base -> const Base&
		// Base& -> const Base&
		void AddDefaultConverters(std::weak_ptr<Type_Info> const& Type) {
			if (auto ptr = Type.lock()) {
				auto baseType = ptr->MakeBase().lock();
				if (baseType) {
					auto refType = baseType->MakeRef().lock();
					auto constType = baseType->MakeConst().lock();
					if (refType && constType) {
						auto constRefType = constType->MakeRef().lock();
						if (constRefType) {
							// Base -> const Base
							if (1) {
								if (auto func = std::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
									return x;
									}, baseType, constType, 0.0))) {
									auto locked{ std::unique_lock(AllConversionsMut) };
									AllConversions[func->from().lock()][func->to().lock()] = func;
								}
							}
							// Base -> Base&
							if (1) {
								if (auto func = std::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
									return x;
									}, baseType, refType, 0.0))) {
									auto locked{ std::unique_lock(AllConversionsMut) };
									AllConversions[func->from().lock()][func->to().lock()] = func;
								}
							}
							// Base -> const Base&
							if (1) {
								if (auto func = std::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
									return x;
									}, baseType, constRefType, 0.0))) {
									auto locked{ std::unique_lock(AllConversionsMut) };
									AllConversions[func->from().lock()][func->to().lock()] = func;
								}
							}
							// const Base -> const Base&
							if (1) {
								if (auto func = std::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
									return x;
									}, constType, constRefType, 0.0))) {
									auto locked{ std::unique_lock(AllConversionsMut) };
									AllConversions[func->from().lock()][func->to().lock()] = func;
								}
							}
							// Base& -> const Base&
							if (1) {
								if (auto func = std::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
									return x;
									}, refType, constRefType, 0.0))) {
									auto locked{ std::unique_lock(AllConversionsMut) };
									AllConversions[func->from().lock()][func->to().lock()] = func;
								}
							}

						}
					}
				}
			}
		};

	public:
		// if does not exists, will add it. If exists, overwrites if the converter is better-performance.
		template<typename From_t, typename To_t> void AddConverter() {
			// forward
			if (1) {
				auto func = details::MakeConversionFunc < const typename std::decay_t<From_t>&, typename std::decay_t<To_t> >();
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto locked{ std::unique_lock(AllConversionsMut) };
						TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
						converterPtr = func;
					}
					AddDefaultConverters(From);
				}
			}
			// backwards (may fail, which is OK)
			if (1) {
				auto func = details::MakeConversionFunc<const typename std::decay_t<To_t>&, typename std::decay_t<From_t>>();
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto locked{ std::unique_lock(AllConversionsMut) };
						TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
						converterPtr = func;
					}
					AddDefaultConverters(From);
				}
			}

			// Add "contructor" class for const Type& -> Type, if possible
			if constexpr (std::is_copy_constructible<typename std::decay_t<From_t>>::value) {
				auto func = details::MakeConversionFunc([](const typename std::decay_t<From_t>& f) -> typename std::decay_t<From_t> { return f; });
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto locked{ std::unique_lock(AllConversionsMut) };
						TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
						converterPtr = func;
					}
				}
			}
			if constexpr (std::is_copy_constructible<typename std::decay_t<To_t>>::value) {
				auto func = details::MakeConversionFunc([](const typename std::decay_t<To_t>& f) -> typename std::decay_t<To_t> { return f; });
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto locked{ std::unique_lock(AllConversionsMut) };
						TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
						converterPtr = func;
					}
				}
			}

		};
		// if does not exists, will add it. If exists, overwrites if the converter is better-performance.
		template<class Callable> void AddConverter(Callable Func) {
			using CallableTypeAsStdFunc = decltype(std::function(std::declval<Callable>()));
			using CallableArguments = typename fibers::utilities::function_traits< CallableTypeAsStdFunc >::arguments;
			if constexpr (std::tuple_size_v< CallableArguments > != 1) {
				return;
			}
			else {
				using From_t = typename details::get_type<typename /*std::decay_t<*/std::tuple_element_t<0, CallableArguments>/*>*/>;
				using To_t = typename details::get_type<typename fibers::utilities::function_traits< CallableTypeAsStdFunc >::result_type>;

				auto func = details::MakeConversionFunc(std::move(Func));
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto locked{ std::unique_lock(AllConversionsMut) };
						TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
						converterPtr = func;
					}
					AddDefaultConverters(From);
					AddDefaultConverters(To);
				}

				// Add "contructor" class for const Type& -> Type, if possible
				if (std::is_same< From_t, Any>::value || std::is_same< To_t, Any>::value) {}
				else {
					// Add "contructor" class for const Type& -> Type, if possible
					if constexpr (std::is_copy_constructible<typename std::decay_t<From_t>>::value) {
						auto func = details::MakeConversionFunc([](const typename std::decay_t<From_t>& f) -> typename std::decay_t<From_t> { return f; });
						if (func) {
							auto& From = func->from();
							auto& To = func->to();
							if (1) {
								auto locked{ std::unique_lock(AllConversionsMut) };
								TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
								converterPtr = func;
							}
						}
					}
					if constexpr (std::is_copy_constructible<typename std::decay_t<To_t>>::value) {
						auto func = details::MakeConversionFunc([](const typename std::decay_t<To_t>& f) -> typename std::decay_t<To_t> { return f; });
						if (func) {
							auto& From = func->from();
							auto& To = func->to();
							if (1) {
								auto locked{ std::unique_lock(AllConversionsMut) };
								TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
								converterPtr = func;
							}
						}
					}


				}
			}
		};
		// if does not exists, will add it. If exists, overwrites if the converter is better-performance.
		template<typename From_t> void AddConverter() {
			// Add "contructor" class for const Type& -> Type, if possible
			if constexpr (std::is_copy_constructible<typename std::decay_t<From_t>>::value) {
				auto func = details::MakeConversionFunc([](const typename std::decay_t<From_t>& f) -> typename std::decay_t<From_t> { return f; });
				if (func) {
					auto& From = func->from();
					auto& To = func->to();
					if (1) {
						auto locked{ std::unique_lock(AllConversionsMut) };
						TypeConverterFunc& converterPtr = AllConversions[From.lock()][To.lock()];
						converterPtr = func;
					}
				}
			}
		};

		// Find or make converter to accomplish the request
		TypeConverterFunc FindConverter(std::weak_ptr<Type_Info> const& From, std::weak_ptr<Type_Info> const& To, bool forceBuild = false) {
			return GetOrBuildConverter(From, To, forceBuild);
		};
		// Find or make converter to accomplish the request
		template<typename From_t, typename To_t> TypeConverterFunc FindConverter(bool forceBuild = false) {
			return FindConverter(user_type_shared<From_t>(), user_type_shared<To_t>(), forceBuild);
		};

		// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
		Any Convert(Any const& from, std::weak_ptr<Type_Info> const& To) {
			if (To.lock()->is_any()) {
				return from;
			}
			else if (auto f = FindConverter(from.Type(), To)) {
				return f->convert(from);
			}
			else return from;
		};
		// will throw an error if the conversion was impossible.
		template<typename To_t> typename std::remove_reference_t<To_t> Convert(Any const& from) {
			static auto to_type { user_type_shared<To_t>().lock() };
			if (auto f = FindConverter(from.Type(), to_type)) {
				Any temp = f->convert(from);
				return temp.cast<To_t>();
			}
			else if (from.IsTypeOf<To_t>()) {
				return from.cast<To_t>();
			}
			else if (to_type->is_any()) {
				return from.cast();
			}
			else {
				throw exception::bad_any_cast(from.Type(), user_type_shared<To_t>(), __LINE__);
			}
		};

		// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
		double ConversionCost(Any const& from, std::weak_ptr<Type_Info> const& To) {
			if (auto f = FindConverter(from.Type(), To)) {
				return f->cost();
			}
			else if (from.IsTypeOf(To)) {
				return 0;
			}
			else if (To.lock()->is_any()) {
				return 0;
			}
			else {
				return std::numeric_limits<double>::max();
			}
		};
		// will throw an error if the conversion was impossible.
		template<typename To_t> double ConversionCost(Any const& from) {
			static auto to_type{ user_type_shared<To_t>().lock() };
			if (auto f = FindConverter(from.Type(), to_type)) {
				return f->cost();
			}
			else if (from.IsTypeOf<To_t>()) {
				return 0;
			}
			else if (to_type->is_any()) {
				return 0;
			}
			else {
				return std::numeric_limits<double>::max();
			}
		};

		// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
		bool Converts(Any const& from, std::weak_ptr<Type_Info> const& To) {
			if (auto f = FindConverter(from.Type(), To)) {
				return true;
			}
			else if (from.IsTypeOf(To)) {
				return true;
			}
			else if (To.lock()->is_any()) {
				return true;
			}
			else {
				return false;
			}
		};
		// will throw an error if the conversion was impossible.
		template<typename To_t> bool Converts(Any const& from) {
			static auto to_type{ user_type_shared<To_t>().lock() };
			if (auto f = FindConverter(from.Type(), to_type)) {
				return true;
			}
			else if (from.IsTypeOf<To_t>()) {
				return true;
			}
			else if (to_type->is_any()) {
				return true;
			}
			else {
				return false;
			}
		};
	};
};

// FunctionSignature, FunctionArgs, & ParamTypes
namespace GoodLang {
	// A collection or list of parameter types. May be a list of types for input into a function, the argument types of the function, or a simple list of types.
	// The types are hashed together to generate a unique hash for this list that can be used to quickly compare them.
	class ParamTypes {
	public:
		static size_t CalculateHash() {
			size_t out{ 37 };
			return out;
		};
		static size_t CalculateHash(std::vector<std::weak_ptr<Type_Info>> const& t_types) {
			size_t out{ 37 };
			for (auto& x : t_types)
				details::hash_combine(out, GetHash(x));
			return out;
		};

	public:
		ParamTypes()
			: uniquehash{ CalculateHash() }
			, m_types{ nullptr }
		{};
		ParamTypes(std::vector<std::weak_ptr<Type_Info>> const& t_types)
			: uniquehash{ CalculateHash(t_types) }
			, m_types(std::make_shared<std::vector<std::weak_ptr<Type_Info>>>(t_types))
		{
			for (auto& type : *m_types) if (auto ptr = type.lock()) if (ptr->MakeBase() == user_type_shared<Any>()) {
				isTemplate = true;
				break;
			}
		};
		ParamTypes(std::vector<std::weak_ptr<Type_Info>>&& t_types)
			: uniquehash{}
			, m_types(std::make_shared<std::vector<std::weak_ptr<Type_Info>>>(std::forward<std::vector<std::weak_ptr<Type_Info>>>(t_types)))
		{
			uniquehash = CalculateHash(*m_types);
			for (auto& type : *m_types) if (auto ptr = type.lock()) if (ptr->MakeBase() == user_type_shared<Any>()) {
				isTemplate = true;
				break;
			}
		};
		ParamTypes(std::vector<Any> const& params)
			: uniquehash{}
			, m_types(std::make_shared<std::vector<std::weak_ptr<Type_Info>>>(params.size(), std::weak_ptr<Type_Info>()))
		{
			for (int i = params.size() - 1; i >= 0; i--) m_types->at(i) = params[i].Type();
			uniquehash = CalculateHash(*m_types);
			for (auto& type : *m_types) if (auto ptr = type.lock()) if (ptr->MakeBase() == user_type_shared<Any>()) {
				isTemplate = true; 
				break;
			}			
		};
		ParamTypes(ParamTypes const&) = default;
		ParamTypes(ParamTypes&&) = default;
		ParamTypes& operator=(ParamTypes const&) = default;
		ParamTypes& operator=(ParamTypes&&) = default;
		~ParamTypes() = default;

		const std::weak_ptr<Type_Info>& operator[](const std::size_t t_i) const noexcept { return m_types->operator[](t_i); };
		std::weak_ptr<Type_Info>& operator[](const std::size_t t_i) noexcept { return m_types->operator[](t_i); };

		friend bool operator==(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash == b.uniquehash;
		};
		friend bool operator!=(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash != b.uniquehash;
		};
		friend bool operator>(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash > b.uniquehash;
		};
		friend bool operator<(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash < b.uniquehash;
		};
		friend bool operator>=(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash >= b.uniquehash;
		};
		friend bool operator<=(ParamTypes const& a, ParamTypes const& b) {
			return a.uniquehash <= b.uniquehash;
		};

		auto begin() const noexcept {
			if (m_types) return m_types->begin();
			else return std::vector<std::weak_ptr<Type_Info>>::iterator();
		}
		auto end() const noexcept {
			if (m_types) return m_types->end();
			else return std::vector<std::weak_ptr<Type_Info>>::iterator();
		}
		size_t size() const noexcept {
			if (m_types) return m_types->size();
			else return 0;
		}
		bool empty() const noexcept { return size() == 0; }
		size_t hash() const noexcept { return uniquehash; };
		bool CanCast(ParamTypes const& to) const {
			if (uniquehash == to.uniquehash) { // exact match
				return true;
			}
			else {
				long long i = to.size() - 1;
				if (i < 0) return true;
				if (i >= size()) return false;
				else {
					auto& toVector = *to.m_types;
					auto& fromVector = *m_types;
					for (; i >= 0; i--) {
						if (auto fromType = fromVector[i].lock()) {
							if (auto toType = toVector[i].lock()) {
								if (!fromType->CanCast(*toType)) {
									return false;
								}
							}
						}
					}
					return true;
				}
			}
		};
		bool IsTemplate() const { return isTemplate; };

	private:
		std::shared_ptr<std::vector<std::weak_ptr<Type_Info>>> m_types;
		size_t uniquehash;
		bool isTemplate{ false };
	};

	// A combination of ParamTypes (list of types) and variable names. Variable names do NOT impact the "uniqueness" of a list of function arguments. e.g;
	// { int:"a" } == { int:"b" }
	class FunctionArgs {
	private:
		static std::vector<std::string> DefaultVariableNames(size_t n) {
			auto out = std::vector<std::string>(n, "Param");
			for (int i = 0; i < n; i++) {
				out[i].append(std::to_string(i));
			}
			return out;
		};
		static std::vector<std::string> DefaultVariableNames(size_t n, std::vector<std::string> const& paramNames) {
			auto out = std::vector<std::string>(n, "Param");
			for (int i = 0; i < n; i++) {
				if (paramNames.size() > i) {
					out[i] = paramNames[i];
				}
				else {
					out[i].append(std::to_string(i));
				}
			}
			return out;
		};

	public:
		FunctionArgs()
			: m_names{}
			, m_types{}
		{};
		FunctionArgs(ParamTypes const& t_types)
			: m_names{ DefaultVariableNames(t_types.size()) }
			, m_types{ t_types }
		{};
		FunctionArgs(ParamTypes const& t_types, std::vector<std::string> const& paramNames)
			: m_names{ DefaultVariableNames(t_types.size(), paramNames) }
			, m_types{ t_types }
		{};
		FunctionArgs(std::vector<std::pair<std::weak_ptr<Type_Info>, std::string>> const& TypesAndNames)
			: m_names{}
			, m_types{}
		{
			std::vector<std::weak_ptr<Type_Info>> types;
			for (int i = 0; i < TypesAndNames.size(); i++) {
				m_names.push_back(TypesAndNames[i].second);
				types.push_back(TypesAndNames[i].first);
			}
			m_types = ParamTypes(types);
		};
		FunctionArgs(FunctionArgs const&) = default;
		FunctionArgs(FunctionArgs&&) = default;
		FunctionArgs& operator=(FunctionArgs const&) = default;
		FunctionArgs& operator=(FunctionArgs&&) = default;
		~FunctionArgs() = default;

		auto& Type(int i) const {
			return m_types[i];
		};
		auto& Name(int i) const {
			return m_names[i];
		};
		auto& Types() const {
			return m_types;
		};
		auto& Names() const {
			return m_names;
		};

		friend bool operator==(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types == b.m_types;
		};
		friend bool operator!=(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types != b.m_types;
		};
		friend bool operator>(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types > b.m_types;
		};
		friend bool operator<(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types < b.m_types;
		};
		friend bool operator>=(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types >= b.m_types;
		};
		friend bool operator<=(FunctionArgs const& a, FunctionArgs const& b) {
			return a.m_types <= b.m_types;
		};

		size_t size() const noexcept {
			return m_types.size();
		}
		bool empty() const noexcept { return m_types.empty(); }
		size_t hash() const noexcept { return m_types.hash(); };

		auto begin() const noexcept {
			return m_types.begin();
		};
		auto end() const noexcept {
			return m_types.end();
		};

		bool CanCastTo(ParamTypes const& to) const {
			return m_types.CanCast(to);
		};
		bool CanCastFrom(ParamTypes const& from) const {
			return from.CanCast(m_types);
		};
		bool CanCastTo(FunctionArgs const& to) const {
			return m_types.CanCast(to.m_types);
		};
		bool CanCastFrom(FunctionArgs const& from) const {
			return from.m_types.CanCast(m_types);
		};
		bool IsTemplate() const { return m_types.IsTemplate(); };
	private:
		ParamTypes m_types;
		std::vector<std::string> m_names;
	};

	// A combination of FunctionArgs (argument types and names), return type, and function name. 
	// Function names DO impact the "uniqueness" of a function signature.
	// Return types do NOT impact the "uniqueness" of a function signature.
	class FunctionSignature {
	public:
		static size_t CalculateHash(FunctionArgs const& arguments, std::string const& qualified_name) {
			size_t out{ 37 };
			details::hash_combine(out, arguments.hash());
			details::hash_combine(out, std::hash<std::string>()(qualified_name));
			return out;
		};
		static size_t CalculateHash(ParamTypes const& arguments, std::string const& qualified_name) {
			size_t out{ 37 };
			details::hash_combine(out, arguments.hash());
			details::hash_combine(out, std::hash<std::string>()(qualified_name));
			return out;
		};

	public:
		FunctionSignature()
			: m_qualified_name("::")
		{
			uniqueHash = CalculateHash(m_arguments, m_qualified_name);
		};
		FunctionSignature(
			std::weak_ptr<Type_Info> returnType
			, FunctionArgs const& args
			, std::string const& Namespace = ""
			, std::string const& name = "")
			: m_arguments(args)
			, m_namespace(Namespace)
			, m_name(name)
			, m_qualified_name(Namespace + "::" + name)
			, m_returnType(returnType)
			, uniqueHash(CalculateHash(args, Namespace + "::" + name))
		{};
		FunctionSignature(FunctionSignature const&) = default;
		FunctionSignature(FunctionSignature&&) = default;
		FunctionSignature& operator=(FunctionSignature const&) = default;
		FunctionSignature& operator=(FunctionSignature&&) = default;
		~FunctionSignature() = default;

		const std::weak_ptr<Type_Info>& Returns() const { return m_returnType; };
		const FunctionArgs& Arguments() const { return m_arguments; };
		const std::string& Name() const { return m_name; };
		const std::string& QualifiedName() const { return m_qualified_name; };
		size_t hash() const noexcept { return uniqueHash; };
		bool IsTemplate() const { return m_arguments.IsTemplate(); };

		friend bool operator==(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash == b.uniqueHash;
		};
		friend bool operator!=(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash != b.uniqueHash;
		};
		friend bool operator>(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash > b.uniqueHash;
		};
		friend bool operator<(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash < b.uniqueHash;
		};
		friend bool operator>=(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash >= b.uniqueHash;
		};
		friend bool operator<=(FunctionSignature const& a, FunctionSignature const& b) {
			return a.uniqueHash <= b.uniqueHash;
		};

	private:
		FunctionArgs m_arguments; // Use this for the hash.
		std::string m_namespace;
		std::string m_name;
		std::string m_qualified_name; // m_namespace::m_name. Use this for the hash.
		std::weak_ptr<Type_Info> m_returnType; // not used for the hash.
		size_t uniqueHash;
	};

};

// FunctionSignature, FunctionArgs, & ParamTypes std::hash, std::less, std::greater, std::equal_to
namespace std {
	template <> struct hash<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& lhs, const GoodLang::ParamTypes& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& lhs, const GoodLang::ParamTypes& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::ParamTypes> {
		std::size_t operator()(const GoodLang::ParamTypes& lhs, const GoodLang::ParamTypes& rhs) const {
			return lhs == rhs;
		};
	};

	template <> struct hash<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& lhs, const GoodLang::FunctionArgs& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& lhs, const GoodLang::FunctionArgs& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::FunctionArgs> {
		std::size_t operator()(const GoodLang::FunctionArgs& lhs, const GoodLang::FunctionArgs& rhs) const {
			return lhs == rhs;
		};
	};

	template <> struct hash<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& lhs, const GoodLang::FunctionSignature& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& lhs, const GoodLang::FunctionSignature& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::FunctionSignature> {
		std::size_t operator()(const GoodLang::FunctionSignature& lhs, const GoodLang::FunctionSignature& rhs) const {
			return lhs == rhs;
		};
	};
};

// Proxy_Function_Base 
namespace GoodLang {
	namespace details {
		/**
		 * Pure virtual base class for all Proxy_Function implementations
		 * Proxy_Functions are a type erasure of type-safe C++ function calls.
		 * At runtime parameter types are expected to be tested against passed in types.
		 * Dispatch_Engine only knows how to work with Proxy_Function, no other
		 * function classes.
		*/
		class Proxy_Function_Base {
		private:
			static double conversion_cost(std::vector<Any> const& t_from, ParamTypes const& t_to, TypeConverter& t_conversions) {
				double out{ 0 };

				// Quick return if the types exactly match.
				if (t_to.size() > t_from.size()) return std::numeric_limits<double>::max();
				// if (t_to.hash() == t_from.hash()) { return 0; } // exact match -- no conversions will happen
				
				size_t i = 0;
				for (; i < t_to.size(); ++i) {
					double conversionCost = t_conversions.ConversionCost(t_from[i], t_to[i]);
					if (conversionCost == std::numeric_limits<double>::max()) {
						return std::numeric_limits<double>::max();
					}
					else {
						out += conversionCost;
					}
				}
				for (; i < t_from.size(); ++i) {
					out += details::TypeConversionWorstCaseCost; // large penalty for not using the provided type(s).
				}
				return out;
			};
			static std::vector<Any> convert(std::vector<Any> const& t_from, ParamTypes const& t_to, TypeConverter& t_conversions) {
				std::vector<Any> out;

				if (t_to.size() > t_from.size()) throw exception::arity_error(t_to.size(), t_to.size());

				out.resize(t_to.size());

				size_t i = 0;
				for (; i < t_to.size(); ++i) {
					out[i] = t_conversions.Convert(t_from[i], t_to[i]);
				}

				return out;
			};

		protected:
			GoodLang::FunctionSignature m_signature;

		public:
			virtual ~Proxy_Function_Base() = default;

			size_t hash() const {
				return m_signature.hash();
			};
			const GoodLang::FunctionSignature& GetSignature() const {
				return m_signature;
			};
			size_t NumArguments() const {
				return m_signature.Arguments().size();
			};
			const auto& Argument(size_t N) const noexcept { return m_signature.Arguments().Type(N); };
			const auto& Arguments() const noexcept { return m_signature.Arguments(); };
			const auto& Returns() const noexcept { return m_signature.Returns(); };

			// Symbolic "cost" to perform the conversion. Not meant to be precise, but meant to be relative for comparison with other converters.
			double conversion_cost(std::vector<Any> const& t_params, TypeConverter& t_conversions) const {
				return Proxy_Function_Base::conversion_cost(t_params, Arguments().Types(), t_conversions);
			};

			// Does want conversions -- ensure types match if possible.
			Any operator()(const std::vector<Any>& params, TypeConverter& t_conversions) const {
				if (params.size() >= NumArguments()) {
					return do_call(convert(params, t_conversions));
				}
				throw exception::arity_error(static_cast<int>(params.size()), NumArguments());
			};

			//// Does not want conversions -- straight call.
			//Any operator()(const std::vector<Any>& params) const {
			//	if (params.size() >= NumArguments()) {
			//		return do_call(params);
			//	}
			//	throw exception::arity_error(static_cast<int>(params.size()), NumArguments());
			//};

			//// Does not want conversions -- straight call.
			//Any operator()(Any& param) const {
			//	if (1 >= NumArguments()) {
			//		return do_call({ param });
			//	}
			//	throw exception::arity_error(1, NumArguments());
			//};

			friend bool operator==(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature == rhs.m_signature; // same signature
			};
			friend bool operator!=(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature != rhs.m_signature; // same signature
			};
			friend bool operator>(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature > rhs.m_signature; // same signature
			};
			friend bool operator>=(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature >= rhs.m_signature; // same signature
			};
			friend bool operator<(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature < rhs.m_signature; // same signature
			};
			friend bool operator<=(const Proxy_Function_Base& lhs, const Proxy_Function_Base& rhs) noexcept {
				return lhs.m_signature <= rhs.m_signature; // same signature
			};

		protected:
			// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
			std::vector<Any> convert(std::vector<Any> const& t_params, TypeConverter& t_conversions) const {
				return Proxy_Function_Base::convert(t_params, m_signature.Arguments().Types(), t_conversions);
			};

		protected:
			virtual Any do_call(std::vector<Any> const&) const = 0;

			Proxy_Function_Base(GoodLang::FunctionSignature const& p_signature)
				: m_signature(p_signature)
			{}
		};
	};
};

// Proxy_Function_Base std::hash, std::less, std::greater, std::equal_to
namespace std {
	template <> struct hash<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& k) const {
			return k.hash();
		};
	};
	template <> struct less<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& lhs, const GoodLang::details::Proxy_Function_Base& rhs) const {
			return lhs < rhs;
		};
	};
	template <> struct greater<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& lhs, const GoodLang::details::Proxy_Function_Base& rhs) const {
			return lhs > rhs;
		};
	};
	template <> struct equal_to<GoodLang::details::Proxy_Function_Base> {
		std::size_t operator()(const GoodLang::details::Proxy_Function_Base& lhs, const GoodLang::details::Proxy_Function_Base& rhs) const {
			return lhs == rhs;
		};
	};

	template <> struct hash<std::shared_ptr<GoodLang::details::Proxy_Function_Base>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& k) const {
			if (k) return k->hash();
			else return 0;
		};
	};
	template <> struct less<std::shared_ptr<std::shared_ptr<GoodLang::details::Proxy_Function_Base>>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& lhs, const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& rhs) const {
			if (lhs && rhs) return *lhs < *rhs;
			else return 0;
		};
	};
	template <> struct greater<std::shared_ptr<GoodLang::details::Proxy_Function_Base>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& lhs, const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& rhs) const {
			if (lhs && rhs) return *lhs > *rhs;
			else return 0;
		};
	};
	template <> struct equal_to<std::shared_ptr<GoodLang::details::Proxy_Function_Base>> {
		std::size_t operator()(const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& lhs, const std::shared_ptr<GoodLang::details::Proxy_Function_Base>& rhs) const {
			if (lhs && rhs) return *lhs == *rhs;
			else return 0;
		};
	};
};

// Proxy Function typedef, make_callable(...), and call(...)
namespace GoodLang {
	/// \brief Common typedef used for passing of any registered function in ChaiScript
	using Proxy_Function = std::shared_ptr<details::Proxy_Function_Base>;

	namespace details {
		/**
		 * Use to call function objects
		*/
		template <class Callable>
		class Explicit_Function_Impl : public Proxy_Function_Base {
		protected:
			static GoodLang::FunctionSignature CreateSignature() {
				using argType = typename fibers::utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::arguments;
				using returnType = typename fibers::utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };

				std::vector<std::weak_ptr<Type_Info>> types(numArgs, std::weak_ptr<Type_Info>());
#define argT(NN) if constexpr (numArgs > NN) { types[NN] = user_type_shared<typename std::tuple_element_t<NN, argType>>(); }
				argT(0);
				argT(1);
				argT(2);
				argT(3);
				argT(4);
				argT(5);
				argT(6);
				argT(7);
				argT(8);
				argT(9);
				argT(10);
				argT(11);
				argT(12);
				argT(13);
				argT(14);
				argT(15);
#undef argT
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<returnType>(), args, "", "");
			};

		public:
			Explicit_Function_Impl(Callable F_p)
				: Proxy_Function_Base(CreateSignature())
				, F_m(std::move(F_p))
			{};
			virtual ~Explicit_Function_Impl() = default;

		protected:
			virtual Any do_call(std::vector<Any> const& r) const override {
				using argType = typename fibers::utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::arguments;
				using returnType = typename fibers::utilities::function_traits<decltype(std::function(std::declval<Callable>()))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };

				if constexpr (std::is_reference< returnType>::value) {
					// if the return type is a reference, the parent(s) should be protected by carrying them along. 
					using refAsBaseType = typename std::remove_reference_t< returnType>;
					using ptrType = std::shared_ptr<refAsBaseType>;
					ptrType out;
					std::vector<Any> parents = r;
					if constexpr (numArgs == 16) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 15) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 14) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 13) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 12) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 11) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 10) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 9) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 8) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 7) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 6) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 5) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 4) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 3) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 2) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 1) {
						out = ptrType(&F_m(
							r[0].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs <= 0) {
					    out = ptrType(&F_m(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					return out;
				}
				else {
					// best-case, normal operation
					if constexpr (std::is_same_v<returnType, void>) {
						static Any temp;
						if constexpr (numArgs == 16) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							F_m();
						}
						return temp;
					}
					else {

						if constexpr (numArgs == 16) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							return F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							return F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							return F_m();
						}
					}
				}
			};
			Callable F_m;
		};

		/**
		 * Use to call member objects:
		 * struct Test{ public: std::string attr; }
		 * var& func = fibers::details::Attribute_Access_Impl(&Test::attr);
		 * assert(func(Test{ "STR" }).cast<std::string>() == "STR");
		*/
		template <typename T, class Class>
		class Attribute_Access_Impl : public Proxy_Function_Base {
		protected:
			using ReturnType = typename std::decay_t<typename details::get_type<T>::type>;
			static GoodLang::FunctionSignature CreateSignature() {
				std::vector<std::weak_ptr<Type_Info>> types = { user_type_shared<Class&>() };
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<ReturnType&>(), args, "", "");
			};

		public:
			using actualT = typename std::decay_t<typename details::get_type<T>::type>;
			Attribute_Access_Impl(T Class::* t_attr)
				: Proxy_Function_Base(CreateSignature())
				, m_attr(t_attr)
			{};
			virtual ~Attribute_Access_Impl() = default;

		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < 1) throw(exception::arity_error(0, 1));
				return do_call_impl(r[0].cast<std::shared_ptr<Class>>());
			};

			auto& do_call_impl_impl(Class* o) const {
				return o->*m_attr;
			};

			Any do_call_impl(std::shared_ptr<Class> o) const {
				if constexpr (std::is_same_v<void, T>) {
					// void? Return void.
					return Any();
				}
				else if constexpr (std::is_same<Any, actualT>::value) { // typename std::remove_reference_t<T>
					// Any? Return reference to the underlying value, NOT a reference to the Any.
					return do_call_impl_impl(o.get());
				}
				else if constexpr (std::is_pointer<T>::value) {
					// Pointer? Wrap it as a shared pointer.
					using Type = typename std::remove_pointer<T>::type;
					decltype(auto) ptr = do_call_impl_impl(o.get());
					if (ptr) {
						return std::shared_ptr<Type>(ptr, [=](Type*) { (void)o.get(); /* do nothing */ });
					}
					else {
						return Any();
					}
				}
				else {
					// Reference? Wrap it as a shared pointer.
					using Type = typename std::remove_reference<T>::type;
					return std::shared_ptr<Type>(&(do_call_impl_impl(o.get())), [=](Type*) { (void)o.get(); /* do nothing */ });
				}
			};

			T Class::* m_attr;
		};

		template <typename T, class Class>
		class Const_Attribute_Access_Impl : public Proxy_Function_Base {
		protected:
			using ReturnType = typename std::decay_t<typename details::get_type<T>::type>;

			static GoodLang::FunctionSignature CreateSignature() {
				std::vector<std::weak_ptr<Type_Info>> types = { user_type_shared<const Class&>() };
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<const ReturnType&>(), args, "", "");
			};

		public:
			using actualT = typename std::decay_t<typename details::get_type<T>::type>;
			Const_Attribute_Access_Impl(T Class::* t_attr)
				: Proxy_Function_Base(CreateSignature())
				, m_attr(t_attr)
			{};
			virtual ~Const_Attribute_Access_Impl() = default;

		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < 1) throw(exception::arity_error(0, 1));
				return do_call_impl(r[0].cast<std::shared_ptr<Class>>());
			};

			auto& do_call_impl_impl(Class* o) const {
				return o->*m_attr;
			};

			Any do_call_impl(std::shared_ptr<Class> o) const {
				if constexpr (std::is_same_v<void, T>) {
					// void? Return void.
					return Any();
				}
				else if constexpr (std::is_same<Any, actualT>::value) { // typename std::remove_reference_t<T>
					// Any? Return reference to the underlying value, NOT a reference to the Any.
					return do_call_impl_impl(o.get());
				}
				else if constexpr (std::is_pointer<T>::value) {
					// Pointer? Wrap it as a shared pointer.
					using Type = typename std::remove_pointer<T>::type;
					decltype(auto) ptr = do_call_impl_impl(o.get());
					if (ptr) {
						return std::shared_ptr<Type>(ptr, [=](Type*) { (void)o.get(); /* do nothing */ });
					}
					else {
						return Any();
					}
				}
				else {
					// Reference? Wrap it as a shared pointer.
					using Type = typename std::remove_reference<T>::type;
					return std::shared_ptr<Type>(&(do_call_impl_impl(o.get())), [=](Type*) { (void)o.get(); /* do nothing */ });
				}
			};

			T Class::* m_attr;
		};

		namespace detail {
			/**
			 * Use to call member functions:
			 * struct Test{ public: std::string attr(){ return "TEST"; }; }
			 * var& func = fibers::details::Attribute_Access_Impl(&Test::attr);
			 * assert(func(Test{}).cast<std::string>() == "TEST");
			*/
			template <typename R, typename Class, typename... T>
			class VolatileConst_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				using actualT = typename std::decay_t<typename details::get_type<R>::type>;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<const Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				VolatileConst_Member_Function_Impl(R(Class::* f)(T...) volatile const)
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~VolatileConst_Member_Function_Impl() = default;

			protected:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					using returnType = R;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						using refAsBaseType = typename std::remove_reference_t< returnType>;
						using ptrType = std::shared_ptr<refAsBaseType>;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};

				R(Class::* m_attr)(T...) volatile const;
			};
			template <typename R, typename Class, typename... T>
			class Volatile_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				using actualT = typename std::decay_t<typename details::get_type<R>::type>;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				Volatile_Member_Function_Impl(R(Class::* f)(T...) volatile)
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~Volatile_Member_Function_Impl() = default;

			protected:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					using returnType = R;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						using refAsBaseType = typename std::remove_reference_t< returnType>;
						using ptrType = std::shared_ptr<refAsBaseType>;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};

				R(Class::* m_attr)(T...) volatile;
			};
			template <typename R, typename Class, typename... T>
			class Const_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				using actualT = typename std::decay_t<typename details::get_type<R>::type>;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<const Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				Const_Member_Function_Impl(R(Class::* f)(T...) const)
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~Const_Member_Function_Impl() = default;

			protected:
				virtual Any do_call(std::vector<Any> const& r) const override {
					using returnType = R;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						using refAsBaseType = typename std::remove_reference_t< returnType>;
						using ptrType = std::shared_ptr<refAsBaseType>;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
								), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
									);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
									);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
									);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
									);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
									);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
									);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
									);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
									);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
									);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
									);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
									);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
									);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
									);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
									);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
									);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
									);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};
				R(Class::* m_attr)(T...) const;
			};
			template <typename R, typename Class, typename... T>
			class Default_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				using actualT = typename std::decay_t<typename details::get_type<R>::type>;
				static GoodLang::FunctionSignature CreateSignature() {
					std::vector<std::weak_ptr<Type_Info>> types(numArgs + 1, std::weak_ptr<Type_Info>());
					types[0] = user_type_shared<Class&>();
#define argT(NN) if constexpr (numArgs > NN) { types[NN+1] = user_type_shared<typename std::tuple_element_t<NN, argType>>(); }
					argT(0);
					argT(1);
					argT(2);
					argT(3);
					argT(4);
					argT(5);
					argT(6);
					argT(7);
					argT(8);
					argT(9);
					argT(10);
					argT(11);
					argT(12);
					argT(13);
					argT(14);
					argT(15);
#undef argT
					GoodLang::ParamTypes params(types);
					GoodLang::FunctionArgs args(params);
					return GoodLang::FunctionSignature(user_type_shared<typename details::get_type<R>::type>(), args, "", "");
				};

			public:
				Default_Member_Function_Impl(R(Class::* f)(T...))
					: Proxy_Function_Base(CreateSignature())
					, m_attr(std::move(f)) {};
				virtual ~Default_Member_Function_Impl() = default;

			private:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					using returnType = R;
					if constexpr (std::is_reference< returnType>::value) {
						// if the return type is a reference, the parent(s) should be protected by carrying them along. 
						using refAsBaseType = typename std::remove_reference_t< returnType>;
						using ptrType = std::shared_ptr<refAsBaseType>;
						ptrType out;
						std::vector<Any> parents = r;
						if constexpr (numArgs == 16) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 15) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast(), r[15].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 14) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast(), r[14].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 13) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
								r[13].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 12) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 11) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast(), r[11].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 10) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast(), r[10].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 9) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
								r[9].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 8) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 7) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast(), r[7].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 6) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast(), r[6].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 5) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
								r[5].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 4) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 3) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast(), r[3].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 2) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast(), r[2].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs == 1) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(
								r[1].cast()
							), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						else if constexpr (numArgs <= 0) {
							out = ptrType(&(r[0].cast<Class*>()->*m_attr)(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
						}
						return out;
					}
					else {
						// best-case, normal operation
						if constexpr (std::is_same_v<returnType, void>) {
							static Any temp;
							if constexpr (numArgs == 16) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								);
							}
							else if constexpr (numArgs == 15) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
								);
							}
							else if constexpr (numArgs == 14) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
								);
							}
							else if constexpr (numArgs == 13) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
								);
							}
							else if constexpr (numArgs == 12) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								);
							}
							else if constexpr (numArgs == 11) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
								);
							}
							else if constexpr (numArgs == 10) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
								);
							}
							else if constexpr (numArgs == 9) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
								);
							}
							else if constexpr (numArgs == 8) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								);
							}
							else if constexpr (numArgs == 7) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
								);
							}
							else if constexpr (numArgs == 6) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
								);
							}
							else if constexpr (numArgs == 5) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
								);
							}
							else if constexpr (numArgs == 4) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								);
							}
							else if constexpr (numArgs == 3) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
								);
							}
							else if constexpr (numArgs == 2) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
								);
							}
							else if constexpr (numArgs == 1) {
								(r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
								);
							}
							else if constexpr (numArgs <= 0) {
								(r[0].cast<Class*>()->*m_attr)();
							}
							return temp;
						}
						else {
							if constexpr (numArgs == 16) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast(), r[16].cast()
								);
							}
							else if constexpr (numArgs == 15) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast(), r[15].cast()
								);
							}
							else if constexpr (numArgs == 14) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast(), r[14].cast()
								);
							}
							else if constexpr (numArgs == 13) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast(),
									r[13].cast()
								);
							}
							else if constexpr (numArgs == 12) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast(), r[12].cast()
								);
							}
							else if constexpr (numArgs == 11) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast(), r[11].cast()
								);
							}
							else if constexpr (numArgs == 10) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast(), r[10].cast()
								);
							}
							else if constexpr (numArgs == 9) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast(),
									r[9].cast()
								);
							}
							else if constexpr (numArgs == 8) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast(), r[8].cast()
								);
							}
							else if constexpr (numArgs == 7) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast(), r[7].cast()
								);
							}
							else if constexpr (numArgs == 6) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast(), r[6].cast()
								);
							}
							else if constexpr (numArgs == 5) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast(),
									r[5].cast()
								);
							}
							else if constexpr (numArgs == 4) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast(), r[4].cast()
								);
							}
							else if constexpr (numArgs == 3) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast(), r[3].cast()
								);
							}
							else if constexpr (numArgs == 2) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast(), r[2].cast()
								);
							}
							else if constexpr (numArgs == 1) {
								return (r[0].cast<Class*>()->*m_attr)(
									r[1].cast()
								);
							}
							else if constexpr (numArgs <= 0) {
								return (r[0].cast<Class*>()->*m_attr)();
							}
						}
					}
				};

				R(Class::* m_attr)(T...);
			};

		};

		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile const) {
			auto* function_impl = new detail::VolatileConst_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile) {
			auto* function_impl = new detail::Volatile_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) const) {
			auto* function_impl = new detail::Const_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...)) {
			auto* function_impl = new detail::Default_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};

		/**
		 * Use to call member functions:
		 * struct Test{ public: std::string attr(){ return "TEST"; }; }
		 * var& func = fibers::details::Attribute_Access_Impl(&Test::attr);
		 * assert(func(Test{}).cast<std::string>() == "TEST");
		*/
		template <typename R, typename... T>
		class Static_Function_Impl : public Proxy_Function_Base {
		public:
			using argType = std::tuple<R, T...>;
			static constexpr auto numArgs = std::tuple_size_v<argType> -1;
			static GoodLang::FunctionSignature CreateSignature() {
				std::vector<std::weak_ptr<Type_Info>> types(numArgs, std::weak_ptr<Type_Info>());
#define argT(NN) if constexpr (numArgs > NN) { types[NN] = user_type_shared<typename std::tuple_element_t<NN, argType>>(); }
				argT(0);
				argT(1);
				argT(2);
				argT(3);
				argT(4);
				argT(5);
				argT(6);
				argT(7);
				argT(8);
				argT(9);
				argT(10);
				argT(11);
				argT(12);
				argT(13);
				argT(14);
				argT(15);
#undef argT
				GoodLang::ParamTypes params(types);
				GoodLang::FunctionArgs args(params);
				return GoodLang::FunctionSignature(user_type_shared<R>(), args, "", "");
			};

		public:
			Static_Function_Impl(R(*f)(T...))
				: Proxy_Function_Base(CreateSignature())
				, F_m(std::move(f)) {};
			virtual ~Static_Function_Impl() = default;

		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < numArgs) throw(exception::arity_error(r.size(), numArgs));
				return do_call_impl(r);
			};

			Any do_call_impl(std::vector<Any> const& r) const {
				if constexpr (std::is_reference< R>::value) {
					// if the return type is a reference, the parent(s) should be protected by carrying them along. 
					using refAsBaseType = typename std::remove_reference_t< R>;
					using ptrType = std::shared_ptr<refAsBaseType>;
					ptrType out;
					std::vector<Any> parents = r;
					if constexpr (numArgs == 16) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 15) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 14) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 13) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 12) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 11) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 10) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 9) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 8) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 7) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 6) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 5) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 4) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 3) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 2) {
						out = ptrType(&F_m(
							r[0].cast(), r[1].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs == 1) {
						out = ptrType(&F_m(
							r[0].cast()
						), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					else if constexpr (numArgs <= 0) {
						out = ptrType(&F_m(), [parents](refAsBaseType*) { if (parents.size() < numArgs) { std::cout << "ERR" << std::endl; } });
					}
					return out;
				}
				else {
					// best-case, normal operation
					if constexpr (std::is_same_v<R, void>) {
						static Any temp;
						if constexpr (numArgs == 16) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							F_m();
						}
						return temp;
					}
					else {

						if constexpr (numArgs == 16) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
						}
						else if constexpr (numArgs == 15) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast(), r[14].cast()
							);
						}
						else if constexpr (numArgs == 14) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast(), r[13].cast()
							);
						}
						else if constexpr (numArgs == 13) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
								r[12].cast()
							);
						}
						else if constexpr (numArgs == 12) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
						}
						else if constexpr (numArgs == 11) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast(), r[10].cast()
							);
						}
						else if constexpr (numArgs == 10) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast(), r[9].cast()
							);
						}
						else if constexpr (numArgs == 9) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
								r[8].cast()
							);
						}
						else if constexpr (numArgs == 8) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
						}
						else if constexpr (numArgs == 7) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast(), r[6].cast()
							);
						}
						else if constexpr (numArgs == 6) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast(), r[5].cast()
							);
						}
						else if constexpr (numArgs == 5) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
								r[4].cast()
							);
						}
						else if constexpr (numArgs == 4) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
						}
						else if constexpr (numArgs == 3) {
							return F_m(
								r[0].cast(), r[1].cast(), r[2].cast()
							);
						}
						else if constexpr (numArgs == 2) {
							return F_m(
								r[0].cast(), r[1].cast()
							);
						}
						else if constexpr (numArgs == 1) {
							return F_m(
								r[0].cast()
							);
						}
						else if constexpr (numArgs <= 0) {
							return F_m();
						}
					}
				}
			};

			R(*F_m)(T...);
		};

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
				using argType = std::tuple<Param...>;
				static constexpr auto numArgs = std::tuple_size_v<argType>;
			};

			template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false>
			struct Function_Signature {
				using Param_Types = Params;
				using Class_Type = Class;
				using Return_Type = Ret;

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
				->Function_Signature<Ret, void, Function_Params<Param...>>; // static function

			// no reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...))
				->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const)
				->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

			// & reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile&)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const&)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...)&)
				->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const&)
				->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

			// && reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile&&)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const&&)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...)&&)
				->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const&&)
				->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class>
			Function_Signature(Ret Class::* f)
				->Function_Signature<Ret, Class, Function_Params<Class&>, true, true>; // member object

			// primary template handles types that have no nested ::type member:
			template<class, class = std::void_t<>>
			struct has_call_operator : std::false_type {};

			// specialization recognizes types that do have a nested ::type member:
			template<class T>
			struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

			template<typename Func>
			auto function_signature(const Func& f) {
				if constexpr (has_call_operator<Func>::value) {
					return Function_Signature<
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Return_Type,
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Class_Type,
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Param_Types,
						false,
						false,
						true
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

		} // namespace chaiscript::dispatch::detail

	};

	// Convert nearly any function or function pointer to a callable, generic proxy function. 
	template<typename Func> Proxy_Function make_callable(Func&& func, TypeConverter& tree) {
		using function_header = decltype(details::detail::function_signature(func));

		static constexpr const bool is_static_member_function = function_header::is_static_member_function;
		static constexpr const bool is_member = function_header::is_member;
		static constexpr const bool is_object = function_header::is_object;
		static constexpr const bool is_member_object = function_header::is_member_object;

		tree.AddConverter< typename function_header::Class_Type>();
		tree.AddConverter< typename function_header::Return_Type>();
#define argT(NN) if constexpr (function_header::Param_Types::numArgs > NN) { tree.AddConverter< typename std::tuple_element_t<NN, function_header::Param_Types::argType> >(); }
		argT(0);
		argT(1);
		argT(2);
		argT(3);
		argT(4);
		argT(5);
		argT(6);
		argT(7);
		argT(8);
		argT(9);
		argT(10);
		argT(11);
		argT(12);
		argT(13);
		argT(14);
		argT(15);
#undef argT

		if constexpr (is_object) {
			// function objects, e.g. auto x = [](){};
			auto* function_impl = new details::Explicit_Function_Impl(std::forward<Func>(func));
			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member_object) {
			// member objects, e.g. return object.member;
			auto* function_impl = new details::Attribute_Access_Impl(std::forward<Func>(func));
			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member && !is_member_object) {
			// member functions, e.g. return object.member();
			return details::Member_Function_Impl(std::forward<Func>(func));
		}
		else if constexpr (is_static_member_function) {
			// static function pointers, e.g. static foo(){};
			auto* function_impl = new details::Static_Function_Impl(std::forward<Func>(func));
			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else {
			throw std::runtime_error("Did not handle conversion of provided function to a PROXY_FUNCTION.");
		}
	};

	// Call a generic, proxy function using a vector of inputs (may be empty), which will be converted as necessary to the expected types. 
	__forceinline Any call(Proxy_Function callable, std::vector<Any> const& inputs, TypeConverter& conversionTree) {
		if (callable) {
			return callable->operator()(inputs, conversionTree);
		}
		else {
			throw exception::arity_error(inputs.size(), -1);
		}
	};

	//// Call a generic, proxy function using a vector of inputs (may be empty), which will be converted as necessary to the expected types. 
	//__forceinline Any call(Proxy_Function callable, std::vector<Any> const& inputs) {
	//	if (callable) {
	//		return callable->operator()(inputs);
	//	}
	//	else {
	//		throw exception::arity_error(inputs.size(), -1);
	//	}
	//};

	//// Call a generic, proxy function using a vector of inputs (may be empty), which will be converted as necessary to the expected types. 
	//__forceinline Any call(Proxy_Function callable, Any const& input) {
	//	if (callable) {
	//		return callable->operator()({ input });
	//	}
	//	else {
	//		throw exception::arity_error(1, -1);
	//	}
	//};
};

// Functions wrapper
namespace GoodLang {
	class Function {
	public:
		Function() = default;
		Function(Proxy_Function const& a) 
			: m_function(a) 
		{};
		Function(Proxy_Function const& a, bool isExplicit, bool isCached = false)
			: m_function(a)
			, m_isEplicit(isExplicit)
			, m_isCached(isCached)
		{};
		Function(Function&&) = default;
		Function(Function const&) = default;
		Function& operator=(Function&&) = default;
		Function& operator=(Function const&) = default;
		~Function() = default;

		Proxy_Function m_function{ nullptr };
		bool m_isEplicit{ false };
		bool m_isCached{ false };
	};

	/*
	// If a function is namespaced in a class, that means it's a free function in the namespace of _CLASS_NAME_, whose first parameter is to be that class type.
	def _CLASS_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _FUNCTION_NAME_( _CLASS_NAME_(),  ... );
		or
		return _CLASS_NAME_()._FUNCTION_NAME_(...);

	// If a function is namespaced in a class but noted as "static", that means it is a free-function, within the namespace of _CLASS_NAME_.
	def static _CLASS_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _CLASS_NAME_::_FUNCTION_NAME_( ... );

	// If a function is declared but not namespaced, then it is assumed to be a free function declared in the current namespace
	def _FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _FUNCTION_NAME_( ... );

	// if a function is namespaced in a namespace, then it is assumed to be a free function
	def _NAMESPACE_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info {};
		e.g.
		return _NAMESPACE_NAME_::_FUNCTION_NAME_();
		or
		return (...)._FUNCTION_NAME_(...);

	namespace { // e.g. global namespace
		class _CLASS_NAME_ {
			def _FUNCTION_NAME_(...) -> void {};
				e.g.
				return ::_CLASS_NAME_()._FUNCTION_NAME_( ... );
				or
				return ::_CLASS_NAME_::_FUNCTION_NAME_( ::_CLASS_NAME_(), ...);

			def static _FUNCTION_NAME_(...) -> void {};
				e.g.
				return ::_CLASS_NAME_()._FUNCTION_NAME_( ... );
				or
				return ::_CLASS_NAME_::_FUNCTION_NAME_( ... );

			def _CLASS_NAME_() -> _CLASS_NAME_ {}; // EXCEPTION TO THE ABOVE RULES. IF THE FUNCTION NAME MATCHES THE CLASS NAME, THEN IT'S A FUNCTION ADDED TO THE PARENT NAMESPACE
				e.g.
				return ::_CLASS_NAME_();

			def _CLASS_NAME_(...) -> _CLASS_NAME_ {}; // EXCEPTION TO THE ABOVE RULES. IF THE FUNCTION NAME MATCHES THE CLASS NAME, THEN IT'S A FUNCTION ADDED TO THE PARENT NAMESPACE
				e.g.
				return ::_CLASS_NAME_(...);

			Type_Info _PARAMETER_NAME_; // parameter within the class, accessible from an instance of it.
				e.g.
				return ::_CLASS_NAME_()._PARAMETER_NAME_;
				or
				return ::_CLASS_NAME_::_PARAMETER_NAME_( ::_CLASS_NAME_() );

			static Type_Info _VARIABLE_NAME_; // static variable associated to a class, accessible from an instance of it or from the class name.
				e.g.
				return ::_CLASS_NAME_()._VARIABLE_NAME_;
				or
				return ::_CLASS_NAME_::_VARIABLE_NAME_;

			class _CLASS_NAME_ { ... } // may declare classes within a class
			namespace _NAMESPACE_NAME_ { ... } // may declare namespaces within a class

		};
	};

	namespace { // e.g. global namespace
		namespace _NAMESPACE_NAME_ {
			def _FUNCTION_NAME_() -> void {}; // IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_FUNCTION_NAME_();

			def _FUNCTION_NAME_(...) -> void {}; // IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_FUNCTION_NAME_(...);

			Type_Info _VARIABLE_NAME_; // global variable within the namespace, accessible outside of it. IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_VARIABLE_NAME_;

			class _CLASS_NAME_ { ... } // may declare classes within a namespace
			namespace _NAMESPACE_NAME_ { ... } // may declare namespaces within a namespace


		};
	};

	// If a function is delcared in a namespace, how that namespace begins determines the ownership behavior
	namespace _NAMESPACE_NAME_ { // e.g. current parent namespace
		def _FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_NAMESPACE_NAME_::_FUNCTION_NAME_(...); // incorporates the _NAMESPACE_NAME_

		def ::_FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_FUNCTION_NAME_(...); // ignores the _NAMESPACE_NAME_

		def ::_NEW_NAMESPACE_NAME_::_FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_NEW_NAMESPACE_NAME_::_FUNCTION_NAME_(...); // ignores the _NAMESPACE_NAME_
	};

	// A possible use of that feature would be extend global functions:
	namespace _CLASS_NAME_ { // e.g. current parent namespace
		def ::to_string() ->  void {}; // non-static implies the first parameter must be a _CLASS_NAME_
			e.g.
			_CLASS_NAME_().to_string();
			or
			to_string(_CLASS_NAME_());

		def static ::to_json(_CLASS_NAME_ a) -> JSON {};
			e.g.
			to_json(_CLASS_NAME_()); // as a static, global function
			or
			_CLASS_NAME_().to_json; // as a static function called on a class_name object

		def static ::`==`(_CLASS_NAME_ a, _CLASS_NAME_ b) -> bool {};
			e.g.
			_CLASS_NAME_() == _CLASS_NAME_();
			or
			`==`( _CLASS_NAME_(), _CLASS_NAME_() );
	};



	*/
	class Functions {
	public:
		Functions() = default;
		Functions(Functions const& rhs) {
			auto locked2{ std::shared_lock(rhs.m_mut) };
			m_functions = rhs.m_functions;
		};
		Functions(Functions && rhs) {
			auto locked2{ std::unique_lock(rhs.m_mut) };
			m_functions = std::move(rhs.m_functions);
		};
		Functions& operator=(Functions const& rhs) {
			auto locked{ std::unique_lock(m_mut) };
			auto locked2{ std::shared_lock(rhs.m_mut) };
			m_functions = rhs.m_functions;
		};
		Functions& operator=(Functions&& rhs) {
			auto locked{ std::unique_lock(rhs.m_mut) };
			auto locked2{ std::unique_lock(rhs.m_mut) };
			m_functions = std::move(rhs.m_functions);
		};
		~Functions() = default;

	public:
		using FunctionPtr = std::shared_ptr<Function>;
		using FunctionSort = std::unordered_map< ParamTypes, FunctionPtr>; // key may NOT be the function's underlying params, but just params that were previously searched... 
		using FunctionMap = std::unordered_map< std::string, FunctionSort >;

		FunctionMap m_functions;
		mutable std::shared_mutex m_mut{};

	private:
		FunctionPtr at_unsafe(std::string const& key, ParamTypes const& params) const {
			auto functionMapPtr = m_functions.find(key);
			if (functionMapPtr != m_functions.end()) {
				auto FunctionSortPtr = functionMapPtr->second.find(params);
				if (FunctionSortPtr != functionMapPtr->second.end()) {
					return FunctionSortPtr->second;
				}
			}
			return nullptr;
		};
	public:
		FunctionPtr operator()(std::string const& key, ParamTypes const& params) const {
			auto locked{ std::shared_lock(m_mut) };
			return at_unsafe(key, params);
		};
		FunctionPtr at(std::string const& key, ParamTypes const& params) const {
			return operator()(key, params);
		};

		FunctionPtr emplace(std::string const& key, ParamTypes const& params, Function const& func, bool replaceIfAlreadyExists = false) {
			auto locked{ std::unique_lock(m_mut) };
			auto& ptr = m_functions[key][params];
			if (!ptr || (ptr && replaceIfAlreadyExists)) 
				ptr = std::make_shared<Function>(func);
			return ptr;
		};
		FunctionPtr emplace(std::string const& key, Function const& func, bool replaceIfAlreadyExists = false) {
			auto locked{ std::unique_lock(m_mut) };
			auto& ptr = m_functions[key][func.m_function->Arguments().Types()];
			if (!ptr || (ptr && replaceIfAlreadyExists))
				ptr = std::make_shared<Function>(func);
			return ptr;
		};

		/* Given a function name and call parameters, will attempt to find an exact-match function, variadic instantiation, or convertable function call, or return nullptr. */
		Proxy_Function BuildMatch(std::string const& functionName, std::vector<Any> const& params, TypeConverter& m_typeConverters, bool AllowTemplateInstantiation = true, bool AllowTypeConversion = true) {
			if (auto func = at(functionName, params)) {
				// cache (or actual) found
				return func->m_function;
			}
			else {
				// Three sorted groups of candidates. 
				// Group 1 = exact matches, Group 2 = type conversions, Group 3 = template functions
				std::map< size_t, std::array<std::map<double, FunctionPtr, std::less<double>>, 3>, std::greater<size_t>>
					candidates;

				// Create candidates.
				{
					auto locked{ std::shared_lock(m_mut) }; // LOCKED
					for (auto& function : m_functions[functionName]) {
						if (!function.second) continue;
						if (!function.second->m_function) continue;
						if (function.second->m_isCached) continue; // ignoring pre-cached functions. Only interested in "true" functions. 
						bool isTemplateFunc = function.second->m_function->GetSignature().IsTemplate();
						bool isExplicitFunc = function.second->m_isEplicit;

						auto conversionCost = function.second->m_function->conversion_cost(params, m_typeConverters);
						if (conversionCost == std::numeric_limits<double>::max()) continue;							

						if (isTemplateFunc) {
							if (AllowTemplateInstantiation) {
								candidates[function.second->m_function->NumArguments()][2][conversionCost] = function.second;
							}
						}
						else {
							if (conversionCost == 0) {
								candidates[function.second->m_function->NumArguments()][0][conversionCost] = function.second;
							}
							else if (AllowTypeConversion && !isExplicitFunc) {
								candidates[function.second->m_function->NumArguments()][1][conversionCost] = function.second;
							}
						}
					}					
				}

				// Get the "cheapest" or fastest conversion option available at this scope, with the largest number of arguments, in order of group (e.g. preference).
				for (auto& numParams : candidates) {
					for (auto& preference_order : numParams.second) {
						for (auto& candidate : preference_order) {
							if (candidate.first == std::numeric_limits<double>::max()) continue;
							if (!candidate.second) continue;

							ParamTypes ParamTypesToCache{ params };
							Function FunctionToCache{ candidate.second->m_function };
							FunctionToCache.m_isCached = true;
							// if someone already beat us to it, it should return the "current" value
							if (auto func = this->emplace(functionName, ParamTypesToCache, FunctionToCache, false)) {
								return func->m_function;
							}
						}
					}
				}				
			}
			return nullptr;
		};
		Any Call(std::string const& functionName, std::vector<Any> const& params, TypeConverter& m_typeConverters) {
			if (auto f = BuildMatch(functionName, params, m_typeConverters)) {
				return f->operator()(params, m_typeConverters);
			}
			else {
				std::string params_str;
				for (auto& p : params) {
					std::string className = p.TypeName(); {
						//if (auto classPtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(p.Type()))) {
						//	className = classPtr->GetName();
						//}
					}

					if (params_str.empty()) {
						params_str += className;
					}
					else {
						params_str += ", ";
						params_str += className;
					}
				}
				throw exception::not_found_error(Units::printf("`%s`(%s)", functionName.c_str(), params_str.c_str()));
			}
		};

	};
};

namespace GoodLang {
	class Scope;
	class Namespace;
	class Class;
	class Global;

	class Scope {
	public:
		friend class Namespace;
		friend class Class;
		friend class Global;

		Scope(std::shared_ptr<Scope> const& parent)
			: p_UniqueName(" _ _ _ _ _ _")
			, p_self()
			, p_parent(parent)
			, p_namespace()
			, p_library()
			, p_using()
		{
			static auto randN{ [](double min, double max) -> double { return (((double)std::rand() / (double)RAND_MAX) * (max - min)) + min; } };
			for (int i = 0; i < 12; i++) {
				if (i < 2)
					p_UniqueName[i] = (char)(int)randN((int)('A'), (int)('Z'));
				else if (i < 6)
					p_UniqueName[i] = (char)(int)randN((int)('0'), (int)('9'));
				else if (i < 8)
					p_UniqueName[i] = (char)(int)randN((int)('A'), (int)('Z'));
				else if (i < 12)
					p_UniqueName[i] = (char)(int)randN((int)('0'), (int)('9'));
			};

			if (auto p = p_parent.lock()) { p_namespace = p->GetNamespaceImpl(); }
			else { p_namespace = std::dynamic_pointer_cast<Namespace>(p_self.lock()); }

			if (auto p = p_parent.lock()) { p_library = p->GetLibraryImpl(); }
			else { p_library = std::dynamic_pointer_cast<Global>(p_self.lock()); }

			qualifiedNamespaceWithQualifiers = GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = GetQualifiedNamespaceImpl();
		};
		Scope(std::shared_ptr<Namespace> const& parent) : Scope(std::dynamic_pointer_cast<Scope>(parent)) {};
		Scope(std::shared_ptr<Class> const& parent) : Scope(std::dynamic_pointer_cast<Scope>(parent)) {};
		Scope(std::shared_ptr<Global> const& parent) : Scope(std::dynamic_pointer_cast<Scope>(parent)) {};

		Scope(Scope const&) = default;
		Scope(Scope&&) = default;
		Scope& operator=(Scope const&) = default;
		Scope& operator=(Scope&&) = default;
		virtual ~Scope() = default;

	private:
		std::string // randomly generated, truly unique name. 
			p_UniqueName;
	public:
		class Hasher {
		public:
			Hasher() = default;
			~Hasher() = default;
			Hasher(Hasher const&) = default;
			Hasher(Hasher&&) = default;
			Hasher& operator=(Hasher const&) = default;
			Hasher& operator=(Hasher&&) = default;

			size_t operator()(std::weak_ptr<Scope> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(ptr.lock());
			};
			size_t operator()(std::weak_ptr<Namespace> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(ptr.lock()));
			};
			size_t operator()(std::weak_ptr<Class> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(ptr.lock()));
			};
			size_t operator()(std::weak_ptr<Global> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(ptr.lock()));
			};
			size_t operator()(std::weak_ptr<TypeConverter> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<TypeConverter>>()(ptr.lock());
			};

			size_t operator()(std::shared_ptr<Scope> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(p);
			};
			size_t operator()(std::shared_ptr<Namespace> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(p));
			};
			size_t operator()(std::shared_ptr<Class> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(p));
			};
			size_t operator()(std::shared_ptr<Global> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope>>()(std::dynamic_pointer_cast<Scope>(p));
			};
		};

	private:
		std::string GetQualifiedNamespaceImpl(bool GetUniqueQualifier = false) const {
			std::string path = "::";

			auto parent = p_parent.lock();

			if (!parent) {
				path = "::";
			}
			else {
				auto name{ GetName() };
				if (GetUniqueQualifier) {
					if (name == "") {
						name = p_UniqueName;
					}
					else {
						name = p_UniqueName + "_" + name;
					}
				}
				if (!name.empty()) {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier) + name + "::";
				}
				else {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier);
				}
			}

			while (path.find("::::") != std::string::npos) {
				size_t start_pos = 0;
				while ((start_pos = path.find("::::", start_pos)) != std::string::npos) {
					path = path.replace(start_pos, 4, "::");
					start_pos += 2; // In case 'to' contains 'from', like replacing 'x' with 'yx'
				}
			}

			return path;
		};

	private:
		std::weak_ptr<Scope> // shared_pointer to itself. MUST be set immediately after creating the Scope/Class/Namespace/Global.
			p_self{};
		std::string
			qualifiedNamespaceWithQualifiers{};
		std::string
			qualifiedNamespaceWithoutQualifiers{};

	public:
		void SetSelf(std::shared_ptr<Scope>& p) { p_self = p; };
		virtual bool IsClass() const { return false; };
		virtual bool IsNamespace() const { return false; };
		virtual std::string GetName() const { return ""; };
		std::string const& GetQualifiedNamespace(bool GetUniqueQualifier = false) const {
			if (GetUniqueQualifier) {
				return qualifiedNamespaceWithQualifiers;
			}
			else {
				return qualifiedNamespaceWithoutQualifiers;
			}
		};

	private:
		std::weak_ptr<Scope> // parent scope, for navigation. Could be anything, or null.
			p_parent{};
		std::weak_ptr<Namespace> // parent's parent's ... parent's scope. The logical result of asking for p_parent on repeat until you get the first Namespace type. 
			p_namespace{};
		// if Namespace or Class or Global, returns self. Otherwise, returns the parent's Namespace. 
		std::weak_ptr<Namespace> GetNamespaceImpl() const {
			if (auto p = std::dynamic_pointer_cast<Namespace>(p_self.lock())) {
				return p;
			}
			else {
				return p_namespace;
			}
		};

	public:
		// if Namespace or Class or Global, returns self. Otherwise, returns the parent's Namespace. 
		std::shared_ptr<Namespace> GetNamespace() const { return p_namespace.lock(); };

	private:
		fibers::containers::Map<std::string, std::shared_ptr<Any>>
			p_objects; // scopes of all types may declare objects. Namespace objects may be global objects, but still. 

	public:
		// try and find the object with the requested key.
		std::shared_ptr<Any> GetObj(std::string const& name) const {
			return p_objects.at_or(name, nullptr);
		};
		// Returns true if successful. Returns false is replaceIfExisting==false and the object already existed on the Scope.
		bool AddObj(std::string const& name, std::shared_ptr<Any> const& obj, bool replaceIfExisting = true) {
			return p_objects.emplace(name, obj, replaceIfExisting);
		};
		// Returns true if successful.
		bool EraseObj(std::string const& name) {
			return p_objects.erase(name);
		};
		// Returns true if successful.
		bool EraseObj(std::shared_ptr<Any> const& Obj) {
			std::string key;
			bool doErasure = false;
			for (auto& obj : p_objects) {
				if (obj && (obj->second == Obj)) {
					key = obj->first;
					doErasure = true;
					break;
				}
			}
			if (doErasure) return EraseObj(key);
			else return false;
		};

	private:
		std::weak_ptr<Global> // parent's parent's ... parent's scope. The logical result of asking for p_parent on repeat until you get the end. 
			p_library{};
		// if Global, returns self. Otherwise, returns the parent's Library. 
		std::weak_ptr<Global> GetLibraryImpl() const {
			if (auto p = std::dynamic_pointer_cast<Global>(p_self.lock())) {
				return p;
			}
			else {
				return p_library;
			}
		};

	public:
		// if Global, returns self. Otherwise, returns the parent's Library. 
		std::shared_ptr<Global> GetLibrary() const {
			if (auto p = p_library.lock())
				return p;
			else if (auto p = std::dynamic_pointer_cast<Global>(p_self.lock()))
				return p;
			else
				return nullptr;
		};

	public:
		fibers::containers::Map< size_t, std::weak_ptr<Namespace>> // allows this scope to use the children of other scopes as if they were their own.
			p_using;
		// the Library should know about our "using" list
		virtual bool RecordUsing(std::shared_ptr<Namespace> ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->RecordUsing(ptr, overrideIfExists);
			}
			return false;
		};

	public:
		// allows this scope to use the children of other scopes as if they were their own.
		bool AddUsing(std::weak_ptr<Namespace> namespacePtr) {
			if (auto p = std::dynamic_pointer_cast<Scope>(namespacePtr.lock())) {
				if (p_using.emplace(Hasher()(p), namespacePtr)) {
					if (p->IsNamespace()) {
						// if this "namespacePtr" belongs to our same library, then we do not care. We only care to track other libraries being used.
						if (p->GetLibrary() != this->GetLibrary()) {
							(void)RecordUsing(std::dynamic_pointer_cast<Namespace>(p));
						}
					}
					return true;
				}
			}
			return false;
		};

	private:
		fibers::containers::Map<std::string,
			std::shared_ptr<
			fibers::containers::Map<size_t, std::shared_ptr<Namespace>>
			>
		> // children namespaces - may be classes or namespaces. By this design, imported namespaces may be "unloaded" on scope unloading, which is on purpose.
			p_children;
		// the Library should know about our "Class" list
		virtual bool RecordClass(std::shared_ptr<Class> ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->RecordClass(ptr, overrideIfExists);
			}
			return false;
		};

	public:
		bool AddChild(std::shared_ptr<Namespace> NamespacePtr) {
			if (auto p = std::dynamic_pointer_cast<Scope>(NamespacePtr)) {
				auto name = p->GetName();
				auto ptr = p_children.get_or_insert(name, std::make_shared<fibers::containers::Map<size_t, std::shared_ptr<Namespace>>>());
				if (ptr->emplace(Hasher()(NamespacePtr), NamespacePtr)) {
					if (p->IsClass()) {
						(void)RecordClass(std::dynamic_pointer_cast<Class>(p));
					}
					return true;
				}
			}
			return false;
		};

	private:
		virtual void RemoveStaleReferences() {
			// p_using
			while (true) {
				size_t toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_using) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_using.erase(toRemove);
				}
				else {
					break;
				}
			}
		};

		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (1) {
					if (auto p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
					checkedSelf.emplace(parentPtr);
					if (1) {
						if (auto p = std::dynamic_pointer_cast<Scope>(parentPtr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
				parentPtr = parentPtr->p_parent.lock();
			}

			// Test my children themselves.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (1) {
									if (auto p = std::dynamic_pointer_cast<Scope>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};


		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace>& bestMatch,
			std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (this->IsNamespace()) {
					if (auto p = std::dynamic_pointer_cast<Namespace>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
					checkedSelf.emplace(parentPtr);
					if (parentPtr->IsNamespace()) {
						if (auto p = std::dynamic_pointer_cast<Namespace>(parentPtr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
				parentPtr = parentPtr->p_parent.lock();
			}

			// Test my children themselves.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (ptr->IsNamespace()) {
									if (auto p = std::dynamic_pointer_cast<Namespace>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};

	protected:
		virtual std::weak_ptr<Type_Info> GetClassType() const { return user_type_shared<void>(); };

	public:
		virtual bool AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists = true) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->AddFunction(name, function, overrideIfAlreadyExists);
			}
			else {
				return false;
			}
		};

	public:
		virtual std::shared_ptr< Functions > GetFunctions() const {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunctions();
			}
			else {
				return nullptr;
			}
		};
		virtual std::shared_ptr< Functions::FunctionSort > GetFunctions(std::string const& name) const {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunctions(name);
			}
			else {
				return nullptr;
			}
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunction(name, params);
			}
			else {
				return {};
			}
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params, TypeConverter& tree) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(GetNamespace())) {
				return namespacePtr->GetFunction(name, params, tree);
			}
			else {
				return {};
			}
		};

	public:
		std::shared_ptr<Scope> FindNearestScopeWhere(std::function<bool(std::shared_ptr<Scope> const&)> const& func) const {
			std::shared_ptr<Scope> out;
			if (TryFindNearestScopeWhere(out, func)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		std::shared_ptr<Namespace> FindNearestNamespaceWhere(std::function<bool(std::shared_ptr<Namespace> const&)> const& func) const {
			std::shared_ptr<Namespace> out;
			if (TryFindNearestNamespaceWhere(out, func)) {
				return out;
			}
			else {
				return nullptr;
			}
		};

	private:
	#define useCachedData
	#ifdef useCachedData
		inline static void hash_combine(std::size_t& seed) { };
		template <typename T, typename... Rest>
		inline static void hash_combine(std::size_t& seed, T&& v, Rest &&... rest) {
			std::hash<T> hasher{};
			seed ^= hasher(std::forward<T>(v)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			hash_combine(seed, std::forward<Rest>(rest)...);
		};
		template <typename T, typename... Rest>
		inline static void hash_combine(std::size_t& seed, T const& v, Rest const&... rest) {
			std::hash<T> hasher{};
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			hash_combine(seed, rest...);
		};

		using CacheContainer = std::pair < std::shared_mutex, concurrency::concurrent_unordered_map<size_t, std::weak_ptr<void>>>;
		using TemplatedCacheContainer = std::pair<std::shared_mutex, concurrency::concurrent_unordered_map<size_t, std::shared_ptr<CacheContainer>>>; // organizes multiple caches for several purposes...
		using VersionedCacheContainer = std::pair<std::shared_mutex, concurrency::concurrent_unordered_map<size_t, std::shared_ptr<TemplatedCacheContainer>>>; // use this mutex to delete entire caches once the version is out-of-date
		std::shared_ptr<VersionedCacheContainer>
			SearchCache{ std::make_shared<VersionedCacheContainer>() };

		std::shared_ptr<TemplatedCacheContainer> GetVersionedCacheContainer(std::shared_ptr<TypeConverter> const& tree)const {
			auto version = std::hash<std::shared_ptr<TypeConverter>>()(tree);
			while (SearchCache) {
				// test if exists
				if (1) {
					auto locked{ std::shared_lock(SearchCache->first) };
					auto f = SearchCache->second.find(version);
					if (f != SearchCache->second.end()) {
						return f->second;
					}
				}

				// didn't exist yet
				if (1) {
					auto locked{ std::unique_lock(SearchCache->first) };
					auto f = SearchCache->second.find(version);
					if (f != SearchCache->second.end()) {
						return f->second;
					}
					else {
						SearchCache->second.insert(std::pair<size_t, std::shared_ptr<TemplatedCacheContainer>>{ version, std::make_shared<TemplatedCacheContainer>() });
					}
				}
			}
			return nullptr;
		};
		template<size_t CacheID> std::shared_ptr<CacheContainer> GetCacheContainer(std::shared_ptr<TypeConverter> const& tree)const {
			if (auto version_container = GetVersionedCacheContainer(tree)) {
				// test if exists
				if (1) {
					auto locked{ std::shared_lock(version_container->first) };
					auto f = version_container->second.find(CacheID);
					if (f != version_container->second.end()) {
						return f->second;
					}
				}

				// didn't exist yet
				if (1) {
					auto locked{ std::unique_lock(version_container->first) };
					auto f = version_container->second.find(CacheID);
					if (f != version_container->second.end()) {
						return f->second;
					}
					else {
						version_container->second.insert(std::pair<size_t, std::shared_ptr<CacheContainer>>{ CacheID, std::make_shared<CacheContainer>() });
					}
				}
			}
			return nullptr;
		};
		template<size_t CacheID, typename T, typename... Rest> bool TryGetCached(std::shared_ptr<TypeConverter> const& tree, std::shared_ptr<T>& out, Rest const&... rest) const {
			if (std::shared_ptr<CacheContainer> cache_container = GetCacheContainer<CacheID>(tree)) {
				size_t hash = 0;
				hash_combine(hash, rest...);

				// test if exists
				if (1) {
					auto locked{ std::shared_lock(cache_container->first) };
					auto f = cache_container->second.find(hash);
					if (f != cache_container->second.end()) {
						out = std::static_pointer_cast<T>(f->second.lock());
						return true;
					}
				}
			}
			out = nullptr;
			return false;
		};
		template<size_t CacheID, typename... Rest> void InsertCached(std::shared_ptr<TypeConverter> const& tree, std::shared_ptr<void> const& obj, Rest const&... rest) const {
			if (std::shared_ptr<CacheContainer> cache_container = GetCacheContainer<CacheID>(tree)) {
				size_t hash = 0;
				hash_combine(hash, rest...);

				// didn't exist yet
				if (1) {
					auto locked{ std::unique_lock(cache_container->first) };
					auto f = cache_container->second.find(hash);
					if (f != cache_container->second.end()) {
						// do nothing?
						f->second = obj;
					}
					else {
						cache_container->second.insert(std::pair<size_t, std::weak_ptr<void>>{ hash, obj });
					}
				}
			}
		};
		template<size_t CacheID, typename... Rest> void InsertCachedIfNotExist(std::shared_ptr<TypeConverter> const& tree, std::shared_ptr<void> const& obj, Rest const&... rest) const {
			if (std::shared_ptr<CacheContainer> cache_container = GetCacheContainer<CacheID>(tree)) {
				size_t hash = 0;
				hash_combine(hash, rest...);

				// didn't exist yet
				if (1) {
					auto locked{ std::unique_lock(cache_container->first) };
					auto f = cache_container->second.find(hash);
					if (f != cache_container->second.end()) {
						// do nothing?
						// f->second = obj;
					}
					else {
						cache_container->second.insert(std::pair<size_t, std::weak_ptr<void>>{ hash, obj });
					}
				}
			}
		};
	#endif
	public:
		std::shared_ptr<Namespace> FindNamespace(std::string QualifiedOrUnqualifiedNamespaceName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };

			QualifiedOrUnqualifiedNamespaceName = fixNamespace(QualifiedOrUnqualifiedNamespaceName);

			if (QualifiedOrUnqualifiedNamespaceName == "" || QualifiedOrUnqualifiedNamespaceName == "::") { return std::dynamic_pointer_cast<Namespace>(this->GetLibrary()); }

			std::shared_ptr<Namespace> out;

	#ifdef useCachedData
			// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
			if (!this->IsNamespace()) {
				if (this->p_using.size() == 0) {
					if (this->p_children.size() == 0) {
						if (auto p = this->p_parent.lock()) {
							return p->FindNamespace(QualifiedOrUnqualifiedNamespaceName);
						}
					}
				}
			}

			auto tree = this->GetTypeConverterTree();
			if (TryGetCached<1>(tree, out, QualifiedOrUnqualifiedNamespaceName)) {
				return out;
			}
	#endif

			long long len = QualifiedOrUnqualifiedNamespaceName.length();
			if (TryFindNearestNamespaceWhere(out, [&len, tryFind = QualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace> const& namespacePtr)->bool {
				std::string const& qualifiedName = std::dynamic_pointer_cast<Scope>(namespacePtr)->GetQualifiedNamespace();
				if ((qualifiedName.size() >= 2) && (qualifiedName.substr(qualifiedName.length() - 2) == "::")) {
					// remove "::" from end
					std::string QualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2);
					while (QualifiedName.size() >= 2 && (QualifiedName.substr(QualifiedName.length() - 2) == "::")) QualifiedName = QualifiedName.substr(0, QualifiedName.length() - 2);

					long long QualifiedNameLen = QualifiedName.length();
					auto F = QualifiedName.find(tryFind);
					if ((F != std::string::npos) && (F == (QualifiedNameLen - len))) return true;
					return false;
				}
				else {
					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);
					if ((F != std::string::npos) && (F == (qualifiedNameLen - len))) return true;
					return false;
				}
				})) {
	#ifdef useCachedData
				InsertCached<1>(tree, out, QualifiedOrUnqualifiedNamespaceName);
	#endif
				return out;
			}		
			return nullptr;
		};
		std::shared_ptr<Class> FindClass(std::string const& QualifiedOrUnqualifiedNamespaceName) const {
			return std::dynamic_pointer_cast<Class>(FindNamespace(QualifiedOrUnqualifiedNamespaceName));
		};

	public:
		std::shared_ptr<Class> FindClass(std::weak_ptr<Type_Info> const& typeInfo) const {
			std::shared_ptr<Namespace> out;

	#ifdef useCachedData
			auto tree = this->GetTypeConverterTree();
			{
				std::shared_ptr<Class> out2;
				if (TryGetCached<2>(tree, out2, typeInfo)) {
					return out2;
				}
			}
	#endif

			if (TryFindNearestNamespaceWhere(out, [tryFind = typeInfo](std::shared_ptr<Namespace> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
					if (ptr->IsClass()) {
						if (tryFind == ptr->GetClassType()) {
							return true;
						}
					}
				}
				return false;
				})) {
	#ifdef useCachedData
				InsertCached<2>(tree, std::dynamic_pointer_cast<Class>(out), typeInfo);
	#endif				
				return std::dynamic_pointer_cast<Class>(out);
			}
			else {
	#ifdef useCachedData
				InsertCached<2>(tree, nullptr, typeInfo);
	#endif				
				return nullptr;
			}
		};

		std::shared_ptr<Scope> FindScopeWithObj(std::string objName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			objName = fixNamespace(objName);

	#ifdef useCachedData
			// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
			if (!this->IsNamespace()) {
				if (this->p_using.size() == 0) {
					if (this->p_children.size() == 0) {
						if (auto objFound = GetObj(objName)) {
							return p_self.lock();
						}
						if (auto p = this->p_parent.lock()) {
							return p->FindScopeWithObj(objName);
						}
					}
				}
			}

			auto tree = this->GetTypeConverterTree();
			{
				std::shared_ptr<Scope> out;

				if (TryGetCached<3>(tree, out, objName)) {
					if (out->TryFindNearestScopeWhere(out, [&objName](std::shared_ptr<Scope> const& namespacePtr)->bool {
						if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
							if (auto objFound = ptr->GetObj(objName)) {
								return true;
							}
						}
						return false;
						})) {
						return out;
					}
				}
			}
	#endif

			auto lastOfColons = objName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				std::shared_ptr<Scope> out;
				if (TryFindNearestScopeWhere(out, [&objName](std::shared_ptr<Scope> const& namespacePtr)->bool {
					if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
						if (auto objFound = ptr->GetObj(objName)) {
							return true;
						}
					}
					return false;
					})) {
					InsertCached<3>(tree, out, objName);
					return out;
				}
				else {
					InsertCached<3>(tree, nullptr, objName);
					return nullptr;
				}
			}
			else {
				std::string Namespace = objName.substr(0, lastOfColons - 1);
				objName = objName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
					auto out = namespacePtr->FindScopeWithObj(objName);
					InsertCached<3>(tree, out, objName);
					return out;
				}
				else {
					InsertCached<3>(tree, nullptr, objName);
					return nullptr;
				}
			}
		};
		std::shared_ptr<Any> FindObj(std::string objName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			objName = fixNamespace(objName);

			auto lastOfColons = objName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(FindScopeWithObj(objName))) {
					return ptr->GetObj(objName);
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = objName.substr(0, lastOfColons - 1);
				objName = objName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
					return namespacePtr->FindObj(objName);
				}
				else {
					return nullptr;
				}
			}
		};

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				std::shared_ptr<Namespace> out;
				if (TryFindNearestNamespaceWhere(out, [&functionName](std::shared_ptr<Namespace> const& namespacePtr)->bool {
					if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
						if (auto objFound = ptr->GetFunctions(functionName)) {
							return true;
						}
					}
					return false;
					})) {
					return out;
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
					return namespacePtr->FindNamespaceWithFunction(functionName);
				}
				else {
					return nullptr;
				}
			}
		};
		std::shared_ptr< Functions::FunctionSort > FindFunctions(std::string functionName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(FindNamespaceWithFunction(functionName))) {
					return ptr->GetFunctions(functionName);
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);

				if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
					return namespacePtr->FindFunctions(functionName);
				}
				else {
					return nullptr;
				}
			}
		};

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree) {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				std::shared_ptr<Namespace> out;
				if (TryFindNearestNamespaceWhere(out, [&functionName, &params, &tree](std::shared_ptr<Namespace> const& namespacePtr)->bool {
					if (auto ptr = std::dynamic_pointer_cast<Scope>(namespacePtr)) {
						if (auto func = ptr->GetFunction(functionName, params, tree)) {
							return true;
						}
					}
					return false;
					})) {
					return out;
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
					return namespacePtr->FindNamespaceWithFunction(functionName, params, tree);
				}
				else {
					return nullptr;
				}
			}
		};
		Proxy_Function FindFunction(std::string functionName, std::vector<Any> const& params, TypeConverter& tree) {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				if (auto ptr = std::dynamic_pointer_cast<Scope>(FindNamespaceWithFunction(functionName, params, tree))) {
					return ptr->GetFunction(functionName, params, tree);
				}
				else {
					return {};
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);

				if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(Namespace))) {
					return namespacePtr->FindFunction(functionName, params, tree);
				}
				else {
					return {};
				}
			}
		};

		virtual std::shared_ptr<TypeConverter> GetTypeConverterTree() const {
			if (auto p = std::dynamic_pointer_cast<Scope>(GetLibrary())) {
				return p->GetTypeConverterTree();
			}
			else {
				return nullptr;
			}
		};

		std::shared_ptr<Namespace> FindNamespaceWithFunction(std::string functionName, std::vector<Any> const& params) {
			return FindNamespaceWithFunction(functionName, params, *GetTypeConverterTree());
		};
		Proxy_Function FindFunction(std::string functionName, std::vector<Any> const& params) {
			auto tree = GetTypeConverterTree();
			return FindFunction(functionName, params, *tree);
		};

		std::vector<std::shared_ptr<Scope>> GetScopesForObjectSearch() const {
			std::vector<std::shared_ptr<Scope>> out;
			// will loop over all available scopes in the order we like
			(void)FindNearestScopeWhere([&](std::shared_ptr<Scope> const& ptr) -> bool {
				out.push_back(ptr);
				return false;
				});
			return out;
		};

	private:
		bool TryFindFunctionImpl(std::string const& functionName, std::vector<Any>  const& params, std::shared_ptr<TypeConverter> const& m_conversionTree, Proxy_Function& out) const {
			if (!m_conversionTree) return false;
	#ifdef useCachedData
			if (TryGetCached<0>(m_conversionTree, out, functionName, ParamTypes(params))) {
				return (bool)out;
			}
			defer(InsertCachedIfNotExist<0>(m_conversionTree, out, functionName, params));
	#endif
			//auto tree_hash = Hasher()(std::weak_ptr<TypeConverter>(m_conversionTree));
			//auto cache1 = GetCache(FindFunctionCache, tree_hash);
			//defer(EraseAllCacheExcept(FindFunctionCache, tree_hash));
			//auto cache2 = GetCache(GetCache(FindFunctionCache, tree_hash), functionName);
			//if (TryGetCache(cache2, params, out)) 
			//	return (bool)out;
			//defer(EmplaceCache(cache2, params, out));

			size_t lastOfColons{ 0 };
			if ((lastOfColons = functionName.find_last_of("::")) == std::string::npos) {
				std::shared_ptr<Scope> firstParamScopePtr{ nullptr };
				std::shared_ptr<Scope> constructorScopePtr{ nullptr };

				// FIRST SEARCH DOES ALLOW FOR CONVERSIONS, BUT NO TEMPLATES
				if (1) {
					std::multimap<double, Proxy_Function> sort;

					// FIRST, WE CHECK TO SEE IF THE DESIRED FUNCTION IS AVAILABLE FROM THE CLASS OF THE FIRST PARAM (e.g. to_string(Units::foot()) would search the Units::foot class before anything else)				
					{
						auto firstParam = params.begin();
						if (firstParam != params.end()) {
							firstParamScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(firstParam->Type()));
						}
					}
					// While we normally try to minimize the conversion cost, 
					if (firstParamScopePtr) {
						if (auto ptr = firstParamScopePtr->GetFunctions()) {
							if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
								// The function is available and requires (potentially) conversion of other parameters. 
								out = func;
								// EmplaceCache(cache2, params, out);
								return true;								
							}
						}
					}

					// try to find the function from nearby scopes... 
					(void)FindNearestNamespaceWhere([&sort, &functionName, &params, &m_conversionTree](std::shared_ptr<Namespace> const& namespace_ptr) -> bool {
						if (auto scope = std::dynamic_pointer_cast<Scope>(namespace_ptr)) {
							if (auto ptr = scope->GetFunctions()) {
								if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
									sort.emplace(func->conversion_cost(params, *m_conversionTree), func);
									// return true; // found a nearby match? Give up? Add a give-up criteria? 
								}
							}
						}
						return false;
					});

					// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, BUT NO TEMPLATES)
					if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
						// Is there a pre-defined constructor that this could work with?
						if (auto functions = constructorScopePtr->GetFunctions()) {
							if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
								sort.emplace(func->conversion_cost(params, *m_conversionTree), func);
							}
						}
					}
					for (auto& s : sort) {
						if (s.first != std::numeric_limits<double>::max()) {
							out = s.second;
							// EmplaceCache(cache2, params, out);
							return true;
						}
					}
				}

				// SECOND SEARCH ALLOWS FOR TEMPLATE FUNCTIONS
				if (1) {
					std::multimap<double, Proxy_Function> sort;
					for (auto& scope : GetScopesForObjectSearch()) {
						if (scope) {
							if (auto ptr = scope->GetFunctions()) {
								if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
									sort.emplace(func->conversion_cost(params, *m_conversionTree), func);
								}
							}
						}
					}
					if (firstParamScopePtr) {
						for (auto& scope : firstParamScopePtr->GetScopesForObjectSearch()) {
							if (scope) {
								if (auto ptr = scope->GetFunctions()) {
									if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
										sort.emplace(func->conversion_cost(params, *m_conversionTree), func);
									}
								}
							}
						}
					}
					// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, AND TEMPLATES)
					if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
						// Is there a pre-defined constructor that this could work with?
						if (auto functions = constructorScopePtr->GetFunctions()) {
							if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
								sort.emplace(func->conversion_cost(params, *m_conversionTree), func);
							}
						}
					}
					for (auto& s : sort) {
						if (s.first != std::numeric_limits<double>::max()) {
							out = s.second;
							// EmplaceCache(cache2, params, out);
							return true;
						}
					}
				}

				// IF ALL SEARCHES FAILED, PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS
				if (1) {
					if (constructorScopePtr) {
						if (auto functions = constructorScopePtr->GetFunctions()) {
							if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
								if (func->conversion_cost(params, *m_conversionTree) != std::numeric_limits<double>::max()) {
									out = func;
									// EmplaceCache(cache2, params, out);
									return true;
								}
							}
						}
					}
				}
			}
			else {
				std::string functionNameActual{ functionName.substr(lastOfColons + 1) };
				std::string scopeName{ functionName.substr(0, lastOfColons - 1) };

				if (auto namespacePtr = std::dynamic_pointer_cast<Scope>(FindNamespace(scopeName))) {
					if (namespacePtr->TryFindFunctionImpl(functionNameActual, params, m_conversionTree, out)) {
						// EmplaceCache(cache2, params, out);
						return true;
					}
					else {
						// EmplaceCache(cache2, params, out);
						return false;
					}
				}
			}

			// EmplaceCache(cache2, params, out);
			return false;
		};

	public:
		Proxy_Function BuildFunction(std::string const& functionName, std::vector<Any> const& params, std::shared_ptr<TypeConverter> const& m_conversionTree) const {
			// note: if this scope is a non-namespace, has no "Using" statements, and has no children, then we can potentially speed-up the process by calling this function on the parent. The parent will do the caching, speeding up that parent scope (hopefully)
			if (!this->IsNamespace()) {
				if (this->p_using.size() == 0) {
					if (this->p_children.size() == 0) {
						if (auto p = this->p_parent.lock()) {
							return p->BuildFunction(functionName, params, m_conversionTree);
						}
					}
				}
			}

			Proxy_Function out{ nullptr };
			if (TryFindFunctionImpl(functionName, params, m_conversionTree, out)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		Proxy_Function BuildFunction(std::string const& functionName, std::vector<Any> const& params) const {
			return BuildFunction(functionName, params, GetTypeConverterTree());
		};
		Any CallFunction(std::string const& functionName, std::vector<Any> const& params) const {
			auto tree = GetTypeConverterTree(); // builds and caches the tree. Updates the tree only if the situation has changed (new functions, new classes, or new Using statements)
			if (tree) {
				if (auto func = BuildFunction(functionName, params, tree)) {
					return call(func, params, *tree);
				}
				else {
					// function was not found with the given params
					std::string params_str;
					for (auto& p : params) {
						std::string className = p.TypeName(); {
							if (auto classPtr = std::dynamic_pointer_cast<Scope>(this->FindClass(p.Type()))) {
								className = classPtr->GetName();
							}
						}

						if (params_str.empty()) {
							params_str += className;
						}
						else {
							params_str += ", ";
							params_str += className;
						}
					}

					throw exception::not_found_error(Units::printf("`%s`(%s)", functionName.c_str(), params_str.c_str()));
				}
			}
			throw std::runtime_error("Scope was invalid");
		};

		template <typename T>
		T Cast(Any const& from) const {
			auto ToType = user_type<T>();
			auto FromType = from.Type();

			// see if we can convert (fastest option)
			auto Tree = this->GetTypeConverterTree();
			if (Tree && Tree->Converts(FromType, ToType)) {
				return Tree->Convert(from, ToType).cast<T>();
			}

			auto ToClass = std::dynamic_pointer_cast<Scope>(this->FindClass(user_type<T>()));
			if (ToClass) {
				// see if he can convert (fastest option)
				auto Tree2 = ToClass->GetTypeConverterTree();
				if (Tree2 && Tree2->Converts(FromType, ToType)) {
					return Tree2->Convert(from, ToType).cast<T>();
				}

				// search for a function that can do it
				if (1) {
					std::vector<Any> params = { from };

					// call a functor from our scope
					try {
						return this->CallFunction(ToClass->GetName(), params).cast<T>();
					}
					catch (exception::not_found_error) {}

					// call a functor from their scope
					try {
						return ToClass->CallFunction(ToClass->GetName(), params).cast<T>();
					}
					catch (exception::not_found_error) {}
				}

				// Failure
				throw exception::not_found_error(ToClass->GetName());
			}

			// Failure
			throw exception::not_found_error(user_type<T>().lock()->name());
		};

	};

	class Namespace : public Scope {
	public:
		friend class Class;
		friend class Global;

		Namespace(std::shared_ptr<Scope> const& parent, std::string const& Name)
			: Scope(parent)
			, p_Name(Name)
		{
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		Namespace(std::shared_ptr<Namespace> const& parent, std::string const& Name) : Namespace(std::dynamic_pointer_cast<Scope>(parent), Name) {};
		Namespace(std::shared_ptr<Class> const& parent, std::string const& Name) : Namespace(std::dynamic_pointer_cast<Scope>(parent), Name) {};
		Namespace(std::shared_ptr<Global> const& parent, std::string const& Name) : Namespace(std::dynamic_pointer_cast<Scope>(parent), Name) {};

		virtual ~Namespace() {};
		void SetSelf(std::shared_ptr<Namespace>& p) { this->p_self = std::dynamic_pointer_cast<Scope>(p); };
		virtual bool IsClass() const override { return false; };
		virtual bool IsNamespace() const override { return true; };
		virtual std::string GetName() const override { return p_Name; };

	private:
		std::string  // e.g. "", or "_NAMESPACE_NAME_", or "_CLASS_NAME_"
			p_Name;
	public:

	private:
		fibers::containers::Map<std::string, std::weak_ptr<Class>> // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate are not allowed.
			p_postfixes;

	public:


	private:
		std::shared_ptr<Functions> // functions. (e.g. `==` or `to_string`). Duplicate names are expected. 
			p_functions{ std::make_shared<Functions>() };

	private:
		virtual void RemoveStaleReferences() override {
			// p_using
			while (true) {
				size_t toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_using) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_using.erase(toRemove);
				}
				else {
					break;
				}
			}

			// p_postfixes
			while (true) {
				std::string toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_postfixes) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_postfixes.erase(toRemove);
				}
				else {
					break;
				}
			}

			// p_functions


		};
		virtual bool RecordFunction(std::string const& Name, Function ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Namespace>(GetLibrary())) {
				return p->RecordFunction(Name, ptr, overrideIfExists);
			}
			return false;
		};
	public:
		virtual bool AddFunction(std::string const& name, Function const& function, bool overrideIfAlreadyExists = true) override {
			
			defer(this->RecordFunction(name, function, overrideIfAlreadyExists));
			if (this->IsClass() && (this->GetName() == name) && (function.m_function->Arguments().size() <= 1)) {
				if (function.m_function->Arguments().size() == 1) {
					if (auto inputClass = std::dynamic_pointer_cast<Scope>(this->FindClass(function.m_function->Arguments().Types()[0]))) {
						try {
							auto tree = this->GetTypeConverterTree();
							if (auto func = inputClass->GetFunction(inputClass->GetName(), {}, *tree)) {								
								if (auto inputParamImpl = func->operator()({}, *tree)) {
									return (bool)p_functions->emplace(name, function, overrideIfAlreadyExists);
								}								
							}
						}
						catch (...) {}

					}
				}
				return (bool)p_functions->emplace(name, function, overrideIfAlreadyExists);
			}
			return (bool)p_functions->emplace(name, function, overrideIfAlreadyExists);

		};

	public:

		virtual std::shared_ptr< Functions > GetFunctions() const override {
			return p_functions;
		};
		virtual std::shared_ptr< Functions::FunctionSort > GetFunctions(std::string const& name) const override {
			// movable shared lock
			auto locked{ std::make_shared< std::shared_lock<std::shared_mutex> >(p_functions->m_mut) };
			auto f = p_functions->m_functions.find(name);
			if (f != p_functions->m_functions.end()) {				
				return std::shared_ptr< Functions::FunctionSort >(&f->second, [lockedCopy = locked](Functions::FunctionSort*) { if (!lockedCopy) { std::cout << "ERR" << std::endl; }; });
			}
			return nullptr;
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params, TypeConverter& tree) override {
			return p_functions->BuildMatch(name, params, tree);
		};
		virtual Proxy_Function GetFunction(std::string const& name, std::vector<Any> const& params) override {
			auto tree = GetTypeConverterTree();
			return GetFunction(name, params, *tree);
		};

	};

	class Class final : public Namespace {
	public:
		friend class Global;

		Class(
			std::shared_ptr<Scope> const& parent
			, std::string const& Name
			, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock()
			, std::weak_ptr<Class> inheritance = std::weak_ptr<Class>() // e.g. this class derives from another Class
		)
			: Namespace(parent, Name)
			, DerivedFrom(inheritance)
		{
			if (auto ptr = std::dynamic_pointer_cast<Namespace>(DerivedFrom.lock())) {
				this->AddUsing(ptr);
			}

			if ((!type) || (type->is_void())) {
				ClassType = std::dynamic_pointer_cast<Type_Info>(std::make_shared<Scripted_Type_Info>(this->p_UniqueName, Name, false, false));
			}
			else {
				ClassType = type;
			}

			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};

		Class(std::shared_ptr<Namespace> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Class> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};
		Class(std::shared_ptr<Global> const& parent, std::string const& Name, std::shared_ptr<Type_Info> type = user_type_shared<void>().lock(), std::weak_ptr<Class> inheritance = std::weak_ptr<Class>())
			: Class(std::dynamic_pointer_cast<Scope>(parent), Name, type, inheritance) {};

		virtual ~Class() {};
		void SetSelf(std::shared_ptr<Class>& p) { this->p_self = std::dynamic_pointer_cast<Scope>(p); };
		virtual bool IsClass() const override { return true; };

	private:
		std::weak_ptr<Class>
			DerivedFrom; // e.g. this class derives from another Class
		std::shared_ptr<Type_Info>
			ClassType;

	public:
		virtual std::weak_ptr<Type_Info> GetClassType() const override { return ClassType; };

	private:
		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope>& bestMatch,
			std::function<bool(std::shared_ptr<Scope> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (1) {
					if (auto p = std::dynamic_pointer_cast<Scope>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test my inherited namespace.
			if (auto p = std::dynamic_pointer_cast<Scope>(DerivedFrom.lock())) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}


			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
					checkedSelf.emplace(parentPtr);
					if (1) {
						if (auto p = std::dynamic_pointer_cast<Scope>(parentPtr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
				parentPtr = parentPtr->p_parent.lock();
			}

			// Test my children themselves.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (1) {
									if (auto p = std::dynamic_pointer_cast<Scope>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};

		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace>& bestMatch,
			std::function<bool(std::shared_ptr<Namespace> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (this->IsNamespace()) {
					if (auto p = std::dynamic_pointer_cast<Namespace>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test my inherited namespace.
			if (auto p = std::dynamic_pointer_cast<Scope>(DerivedFrom.lock())) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test all of my parents 
			if (1) {
				auto parentPtr = this->p_parent.lock();
				while (parentPtr) {
					if (!checkedSelf.count(parentPtr) >= 1) {
						checkedSelf.emplace(parentPtr);
						if (parentPtr->IsNamespace()) {
							if (auto p = std::dynamic_pointer_cast<Namespace>(parentPtr)) {
								if (func(p)) {
									bestMatch = p;
									return true;
								}
							}
						}
					}
					else {
						break; // we've checked this before! Quick, get out. 
					}
					parentPtr = parentPtr->p_parent.lock();
				}
			}

			// test my children themselves
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (ptr->IsNamespace()) {
									if (auto p = std::dynamic_pointer_cast<Namespace>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope>(innerChildNamespace->second)) {
								if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};

	};

#if 0
	namespace UnitsLibrary {
		template<typename T>
		static void AddUnit(std::shared_ptr<scripting::Namespace> const& std_namespace, std::shared_ptr<scripting::Class> const& value_namespace) {
			std::string UnitName = T().UnitName();
			auto foot_namespace{ std::make_shared<scripting::Class>(std_namespace, UnitName, scripting::user_type<T>(), value_namespace) };
			foot_namespace->SetSelf(foot_namespace);
			std_namespace->AddChild(foot_namespace);
			{
				// Constructors
				// foot()
				foot_namespace->AddFunction(UnitName, scripting::make_callable([]() -> T { return T{}; }));
				// foot(double)
				//foot_namespace->AddFunction(UnitName, scripting::make_callable([](double const& o)->T { return o; }));
				// foot(value)
				foot_namespace->AddFunction(UnitName, scripting::make_callable([](Units::value const& makeCopy) -> T { return makeCopy; }));
				// foot() = value();
				foot_namespace->AddFunction("=", scripting::make_callable([](scripting::Any const& a, Units::value const& b) -> scripting::Any { T& out = a.cast(); out = b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<Units::value>() } }));

				// Comparisons & operators
				//foot_namespace->AddFunction("==", scripting::make_callable([](T const& x, T const& y) -> bool { return x == y; }));
				//foot_namespace->AddFunction("!=", scripting::make_callable([](T const& x, T const& y) -> bool { return x != y; }));
				//foot_namespace->AddFunction("<", scripting::make_callable([](T const& x, T const& y) -> bool { return x < y; }));
				//foot_namespace->AddFunction(">", scripting::make_callable([](T const& x, T const& y) -> bool { return x > y; }));
				//foot_namespace->AddFunction("<=", scripting::make_callable([](T const& x, T const& y) -> bool { return x <= y; }));
				//foot_namespace->AddFunction(">=", scripting::make_callable([](T const& x, T const& y) -> bool { return x >= y; }));
				////foot_namespace->AddFunction("+", scripting::make_callable([](T const& x, T const& y) -> T { return x + y; }));
				////foot_namespace->AddFunction("-", scripting::make_callable([](T const& x, T const& y) -> T { return x - y; }));
				////foot_namespace->AddFunction("*", scripting::make_callable([](T const& x, T const& y) -> T { return x * y; }));
				////foot_namespace->AddFunction("/", scripting::make_callable([](T const& x, T const& y) -> T { return x / y; }));
				//foot_namespace->AddFunction("+=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out += b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));
				//foot_namespace->AddFunction("-=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out -= b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));
				//foot_namespace->AddFunction("*=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out *= b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));
				//foot_namespace->AddFunction("/=", scripting::make_callable([](scripting::Any const& a, T const& b) -> scripting::Any { T& out = a.cast(); out /= b; return a; }), scripting::Param_Types({ {std::string("a"), scripting::user_type<T>() }, {std::string("b"), scripting::user_type<T>() } }));

				// value(foot)
				value_namespace->AddFunction(value_namespace->GetName(), scripting::make_callable([](scripting::Any const& from) -> std::shared_ptr<Units::value> {
					return std::dynamic_pointer_cast<Units::value>(from.cast<std::shared_ptr<T>>());
					}), scripting::Param_Types({ { "from", foot_namespace->GetClassType() } }));
			}
		};

		class UnitsLibrary {
		public:
			static void Part1(std::shared_ptr<scripting::Namespace> const& std_namespace, std::shared_ptr<scripting::Class> const& value_namespace);
			static void Part2(std::shared_ptr<scripting::Namespace> const& std_namespace, std::shared_ptr<scripting::Class> const& value_namespace);
			static void Part3(std::shared_ptr<scripting::Namespace> const& std_namespace, std::shared_ptr<scripting::Class> const& value_namespace);
		};

	}
#endif

	class Global final : public Namespace {
	public:
		Global()
			: Namespace(std::shared_ptr<Scope>(), "")
		{
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		virtual ~Global() {};
		void SetSelf(std::shared_ptr<Global>& p) { this->p_self = std::dynamic_pointer_cast<Scope>(p); };

		std::vector<std::weak_ptr<Class>> GetClasses() const {
			std::vector<std::weak_ptr<Class>> out;
			out.reserve(Classes.size() + 16);
			static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

			bool DoCleanup = false;

			size_t hash{ 0 };
			for (auto& x : Classes) {
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.push_back(x->second);
					}
				}
			}

			if (DoCleanup) {
				//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global*>(this)->RemoveStaleReferences();
			}

			return out;
		};
		std::vector<std::weak_ptr<Namespace>> GetUsing() const {
			std::vector<std::weak_ptr<Namespace>> out;
			out.reserve(Classes.size() + 16);
			static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

			bool DoCleanup = false;

			size_t hash{ 0 };
			for (auto& x : Usings) {
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.push_back(x->second);
					}
				}
			}

			if (DoCleanup) {
				//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global*>(this)->RemoveStaleReferences();
			}

			return out;
		};

	public:
#if 0
		void AddBuiltIns() {
			auto defineBuiltInType = [this](auto typeImpl, std::string const& Name) -> void {
				// make it a class
				std::shared_ptr<Class> classPtr; {
					classPtr.reset(new Class(this->p_self.lock(), Name, user_type<decltype(typeImpl)>()));
				}
				classPtr->SetSelf(classPtr);
				this->AddChild(classPtr);

				// add converters
				classPtr->AddFunction(Name, make_callable([](bool from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](int from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](float from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](double from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](size_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](fibers::containers::number < double > from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](signed char from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned char from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char16_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char32_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](wchar_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](short from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned short from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned int from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned long from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long long from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long double from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));

				// Constructors
				classPtr->AddFunction(Name, make_callable([]() ->  decltype(typeImpl) { return  decltype(typeImpl){}; }));
				classPtr->AddFunction(Name, make_callable([](decltype(typeImpl) const& makeCopy) ->  decltype(typeImpl) { return makeCopy; }));
				Type_Info thisType = user_type<decltype(typeImpl)>();
				std::vector<std::pair<std::string, Type_Info>> temp; {
					std::pair<std::string, Type_Info> tempPair{ std::string(), Type_Info() };
					tempPair.first = "o";
					tempPair.second = thisType;
					temp.push_back(tempPair);
					temp.push_back(tempPair);
				}
				classPtr->AddFunction("=", make_callable([](Any const& a, decltype(typeImpl) const& b) -> Any { decltype(typeImpl)& x = a.cast(); x = b; return a; }), Param_Types(temp));

				// Comparisons
				classPtr->AddFunction("==", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x != y; }));
				classPtr->AddFunction("<", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x < y; }));
				classPtr->AddFunction("<=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x <= y; }));
				classPtr->AddFunction(">", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x > y; }));
				classPtr->AddFunction(">=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x >= y; }));
				classPtr->AddFunction("+", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x + y; }));
				classPtr->AddFunction("-", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x - y; }));
				classPtr->AddFunction("*", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x * y; }));
				classPtr->AddFunction("/", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { if (y == 0) return std::numeric_limits<decltype(typeImpl)>::max(); else return x / y; }));
				classPtr->AddFunction("+=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x += y; }));
				classPtr->AddFunction("-=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x -= y; }));
				classPtr->AddFunction("*=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x *= y; }));
				classPtr->AddFunction("/=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { if (y == 0) x = std::numeric_limits<decltype(typeImpl)>::max(); else x /= y; }));

				// Functions
				classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::max(); }));
				classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::lowest(); }));
				classPtr->AddFunction("to_string", make_callable([](decltype(typeImpl) const& o) -> std::string { return std::to_string(o); }));

			};

			// Built-in types
			if (1) {
	#define DefineBuiltInType(V) defineBuiltInType(##V(0), #V);
				DefineBuiltInType(bool);
				DefineBuiltInType(char);
				DefineBuiltInType(int);
				DefineBuiltInType(long);
				DefineBuiltInType(float);
				DefineBuiltInType(double);
				DefineBuiltInType(size_t);
				defineBuiltInType(fibers::containers::number<double>(), "Number");
				DefineBuiltInType(char16_t);
				DefineBuiltInType(char32_t);
				DefineBuiltInType(wchar_t);
				DefineBuiltInType(short);
				defineBuiltInType(unsigned char(0), "uchar");
				defineBuiltInType(unsigned short(0), "ushort");
				defineBuiltInType(unsigned int(0), "uint");
				defineBuiltInType(unsigned long(0), "ulong");
				defineBuiltInType(long long(0), "llong");
				defineBuiltInType(long double(), "ldouble");
	#undef DefineBuiltInType

				// String
				if (1) {
					// make it a class
					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), "string", user_type<std::string>()));
					}
					classPtr->SetSelf(classPtr);
					this->AddChild(classPtr);

					// add converters
					classPtr->AddFunction("string", make_callable([](bool from) -> std::string { if (from) return "true"; else return "false"; }));
					classPtr->AddFunction("string", make_callable([](char from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](int from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](float from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](double from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](size_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](fibers::containers::number < double > from) -> std::string { return std::to_string(from.load()); }));
					classPtr->AddFunction("string", make_callable([](signed char from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned char from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](char16_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](char32_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](wchar_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](short from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned short from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned int from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned long from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long long from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long double from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([self = classPtr->p_self](fibers::Type_Info from)->std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						return from.name();
					}));

					// Constructors
					classPtr->AddFunction("string", make_callable([]() -> std::string { return std::string{}; }));
					classPtr->AddFunction("string", make_callable([](std::string const& makeCopy) -> std::string { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out = b; return a; }), Param_Types({ {std::string("a"), user_type<std::string>() }, {std::string("b"), user_type<std::string>() } }));

					// Comparisons
					classPtr->AddFunction("==", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x != y; }));
					classPtr->AddFunction("<", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x < y; }));
					classPtr->AddFunction("<=", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x <= y; }));
					classPtr->AddFunction(">", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x > y; }));
					classPtr->AddFunction(">=", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x >= y; }));
					classPtr->AddFunction("+", scripting::make_callable([](std::string const& x, std::string const& y) -> std::string { return x + y; }));
					classPtr->AddFunction("+=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out += b; return a; }), Param_Types({ {std::string("a"), user_type<std::string>() }, {std::string("b"), user_type<std::string>() } }));

					// Functions
					classPtr->AddFunction("length", make_callable([](std::string const& a) -> size_t { return a.length(); }));
					classPtr->AddFunction("size", make_callable([](std::string const& a) -> size_t { return a.size(); }));
					classPtr->AddFunction("[]", make_callable([](std::string const& a, size_t index) -> char { return a[index]; }));
					classPtr->AddFunction("front", make_callable([](std::string const& a) -> char { return a.front(); }));
					classPtr->AddFunction("back", make_callable([](std::string const& a) -> char { return a.back(); }));
					classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind) -> size_t { return a.find(toFind); }));
					classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind, size_t startPos) -> size_t { return a.find(toFind, startPos); }));
					classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off) -> std::string { return x.substr(Off); })/*, { "input", "Off" }*/);
					classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off, size_t Count) -> std::string { return x.substr(Off, Count); })/*, { "input", "Off", "Count" }*/);
					classPtr->AddFunction("to_string", make_callable([](std::string const& o) -> std::string { return o; }));

					// Objects or Constants
					classPtr->AddObj("npos", std::make_shared<Any>(std::string::npos));
				}

				// Types
				if (1) {
					// make it a class
					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), "Type_Info", user_type<fibers::Type_Info>()));
					}
					classPtr->p_self = classPtr;
					this->AddChild(classPtr);

					// Constructors
					classPtr->AddFunction("Type_Info", make_callable([]() -> std::weak_ptr<Type_Info> { return std::weak_ptr<Type_Info>{}; }));
					classPtr->AddFunction("Type_Info", make_callable([](std::weak_ptr<Type_Info> const& makeCopy) -> std::weak_ptr<Type_Info> { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, std::weak_ptr<Type_Info> const& b) -> Any { std::weak_ptr<Type_Info>& out = a.cast(); out = b; return a; }), Param_Types({ {std::string("a"), user_type<std::weak_ptr<Type_Info>>() }, {std::string("b"), user_type<std::weak_ptr<Type_Info>>() } }));

					// Comparisons
					classPtr->AddFunction("==", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x != y; }));
					classPtr->AddFunction("<", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x < y; }));
					classPtr->AddFunction("<=", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x <= y; }));
					classPtr->AddFunction(">", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x > y; }));
					classPtr->AddFunction(">=", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x >= y; }));

					// Functions
					classPtr->AddFunction("to_string", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)->std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						return from.name();
					}));
					classPtr->AddFunction("name", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)->std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						return from.name();
					}));
					classPtr->AddFunction("cpp_name", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)->std::string {
						return from.name();
					}));
					classPtr->AddFunction("is_undef", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)-> bool {
						return from.is_undef();
					}));
					classPtr->AddFunction("is_void", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)-> bool {
						return from.is_void();
					}));
				}

				// Units
				if (1) {
					auto selfPtr = this->p_self.lock();
					auto std_namespace{ std::make_shared<Namespace>(selfPtr, "Units") };
					std_namespace->SetSelf(std_namespace);
					this->AddChild(std_namespace);

					// the "std" namespace imports the "string" namespace...
					{
						auto value_namespace{ std::make_shared<Class>(std_namespace, "value", scripting::user_type<Units::value>()) };
						value_namespace->SetSelf(value_namespace);
						std_namespace->AddChild(value_namespace);

						// which has the following types groups... 
						{
							// value -> double
							if (auto p = std_namespace->FindClass(user_type<double>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> double { return o(); }));
							}
							// value -> float
							if (auto p = std_namespace->FindClass(user_type<float>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> float { return o(); }));
							}
							// value -> int
							if (auto p = std_namespace->FindClass(user_type<int>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> int { return o(); }));
							}
							// value -> string
							if (auto p = std_namespace->FindClass(user_type<std::string>())) {
								p->AddFunction(p->GetName(), make_callable([](Units::value const& o) -> std::string { return o.ToString(); }));
							}

							value_namespace->AddFunction("abbreviation", make_callable([](Units::value const& x)->std::string {
								return x.Abbreviation();
								}));
							value_namespace->AddFunction("name", make_callable([](Units::value const& x)->std::string {
								return x.UnitName();
								}));
							value_namespace->AddFunction("to_string", make_callable([](Units::value const& x)->std::string {
								return x.ToString();
								}));

							// Constructors
							value_namespace->AddFunction("value", make_callable([]() -> Units::value { return Units::value{}; }));
							value_namespace->AddFunction("value", make_callable([](Units::value const& makeCopy) -> Units::value { return makeCopy; }));
							value_namespace->AddFunction("value", make_callable([](int const& o)->Units::value { return o; }));
							value_namespace->AddFunction("value", make_callable([](float const& o)->Units::value { return o; }));
							value_namespace->AddFunction("value", make_callable([](double const& o)->Units::value { return o; }));
							value_namespace->AddFunction("=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out = b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));

							// Comparisons & operators
							value_namespace->AddFunction("==", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x == y; }));
							value_namespace->AddFunction("!=", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x != y; }));
							value_namespace->AddFunction("<", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x < y; }));
							value_namespace->AddFunction(">", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x > y; }));
							value_namespace->AddFunction("<=", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x <= y; }));
							value_namespace->AddFunction(">=", scripting::make_callable([](Units::value const& x, Units::value const& y) -> bool { return x >= y; }));
							value_namespace->AddFunction("+", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x + y; }));
							value_namespace->AddFunction("-", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x - y; }));
							value_namespace->AddFunction("*", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x * y; }));
							value_namespace->AddFunction("/", scripting::make_callable([](Units::value const& x, Units::value const& y) -> Units::value { return x / y; }));
							value_namespace->AddFunction("+=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out += b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
							value_namespace->AddFunction("-=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out -= b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
							value_namespace->AddFunction("*=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out *= b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
							value_namespace->AddFunction("/=", make_callable([](Any const& a, Units::value const& b) -> Any { Units::value& out = a.cast(); out /= b; return a; }), Param_Types({ {std::string("a"), user_type<Units::value>() }, {std::string("b"), user_type<Units::value>() } }));
						}

						if (1) {
							scripting::UnitsLibrary::UnitsLibrary::Part1(std_namespace, value_namespace);
							scripting::UnitsLibrary::UnitsLibrary::Part2(std_namespace, value_namespace);
							scripting::UnitsLibrary::UnitsLibrary::Part3(std_namespace, value_namespace);
						}
					}

				}

				// DateTime
				if (1) {
					using thisType = DateTime;
					std::string thisTypeName = "DateTime";

					// make it a class
					std::shared_ptr<Class> classPtr; {
						classPtr.reset(new Class(this->p_self.lock(), thisTypeName, user_type<thisType>()));
					}
					classPtr->SetSelf(classPtr);
					this->AddChild(classPtr);
					scripting::Type_Info thisTypeInfo = classPtr->ClassType;

					// Constructors
					classPtr->AddFunction(thisTypeName, make_callable([]() -> thisType { return thisType{}; }));
					classPtr->AddFunction(thisTypeName, make_callable([](thisType const& makeCopy) -> thisType { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, thisType const& b) -> Any { thisType& out = a.cast(); out = b; return a; }), Param_Types({ {"a", thisTypeInfo }, {"b", thisTypeInfo } }));
					classPtr->AddFunction(thisTypeName, make_callable([](Units::second const& from) -> thisType { return from; }));
					classPtr->AddFunction(thisTypeName, make_callable([](std::string const& from) -> thisType { return thisType(from); }), true, true); // explicit = cannot be used for conversion trees, and must be called directly with exact type match, e.g. DateTime("string")

					// Converters
					if (auto p = this->FindClass(user_type<std::string>())) {
						p->AddFunction(p->GetName(), make_callable([](thisType const& from) -> std::string { return from.c_str(); }));
					}
					if (auto p = this->FindClass(user_type<Units::second>())) {
						p->AddFunction(p->GetName(), make_callable([](thisType const& from) -> Units::second { return (Units::second)from; }));
					}

					// Comparisons
					classPtr->AddFunction("==", scripting::make_callable([](thisType const& x, thisType const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", scripting::make_callable([](thisType const& x, thisType const& y) -> bool { return x != y; }));
					classPtr->AddFunction("<", scripting::make_callable([](thisType const& x, thisType const& y) -> bool { return x < y; }));
					classPtr->AddFunction("<=", scripting::make_callable([](thisType const& x, thisType const& y) -> bool { return x <= y; }));
					classPtr->AddFunction(">", scripting::make_callable([](thisType const& x, thisType const& y) -> bool { return x > y; }));
					classPtr->AddFunction(">=", scripting::make_callable([](thisType const& x, thisType const& y) -> bool { return x >= y; }));

					// Operators
					classPtr->AddFunction("+", scripting::make_callable([](thisType const& x, thisType const& y) -> thisType { return x + y; }));
					classPtr->AddFunction("-", scripting::make_callable([](thisType const& x, thisType const& y) -> thisType { return x - y; }));
					classPtr->AddFunction("*", scripting::make_callable([](thisType const& x, thisType const& y) -> thisType { return x * y; }));
					classPtr->AddFunction("/", scripting::make_callable([](thisType const& x, thisType const& y) -> thisType { if (y == 0) return (Units::second)(std::numeric_limits<Units::second>::max()); else return x / y; }));
					classPtr->AddFunction("+=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x += b; return a; }), Param_Types({ {"obj",thisTypeInfo }, { "toOperateWith",user_type<Units::second>()} }));
					classPtr->AddFunction("-=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x -= b; return a; }), Param_Types({ {"obj",thisTypeInfo }, { "toOperateWith",user_type<Units::second>()} }));
					classPtr->AddFunction("*=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); x *= b; return a; }), Param_Types({ {"obj",thisTypeInfo }, { "toOperateWith",user_type<Units::second>()} }));
					classPtr->AddFunction("/=", make_callable([](Any const& a, Units::second const& b) -> Any { thisType& x = a.cast(); if (b != 0) x /= b; else x = (Units::second)(std::numeric_limits<Units::second>::max()); return a; }), Param_Types({ {"obj",thisTypeInfo }, { "toOperateWith",user_type<Units::second>()} }));

					// Functions
					classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<thisType>::max(); }));
					classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<thisType>::lowest(); }));
					classPtr->AddFunction("to_string", make_callable([](thisType const& o) -> std::string { return o.c_str(); }));
					classPtr->AddFunction("Epoch", make_callable(&DateTime::Epoch));
					// classPtr->AddFunction("Now", make_callable(&DateTime::Now));
					classPtr->AddFunction("Now", make_callable([]() -> DateTime { return DateTime::Now(); }));
					classPtr->AddFunction("tm_fractionalsec", make_callable(&DateTime::tm_fractionalsec));
					classPtr->AddFunction("tm_sec", make_callable(&DateTime::tm_sec));
					classPtr->AddFunction("tm_min", make_callable(&DateTime::tm_min));
					classPtr->AddFunction("tm_hour", make_callable(&DateTime::tm_hour));
					classPtr->AddFunction("tm_mday", make_callable(&DateTime::tm_mday));
					classPtr->AddFunction("tm_mon", make_callable(&DateTime::tm_mon));
					classPtr->AddFunction("tm_yday", make_callable(&DateTime::tm_yday));
					classPtr->AddFunction("tm_wday", make_callable(&DateTime::tm_wday));
					classPtr->AddFunction("tm_year", make_callable(&DateTime::tm_year));
					classPtr->AddFunction("load", make_callable(&DateTime::load));
					classPtr->AddFunction("getNumDaysInSameMonth", make_callable(&DateTime::getNumDaysInSameMonth));
					classPtr->AddFunction("make_time", make_callable([]() { return DateTime::make_time(); }));
					classPtr->AddFunction("make_time", make_callable([](int year) { return DateTime::make_time(year); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month) { return DateTime::make_time(year, month); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day) { return DateTime::make_time(year, month, day); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour) { return DateTime::make_time(year, month, day, hour); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute) { return DateTime::make_time(year, month, day, hour, minute); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute, float second) { return DateTime::make_time(year, month, day, hour, minute, second); }));
					classPtr->AddFunction("make_time", make_callable([](int year, int month, int day, int hour, int minute, float second, bool useLocalTime) { return DateTime::make_time(year, month, day, hour, minute, second, useLocalTime); }));
					classPtr->AddFunction("ToStartOfMonth", make_callable([](DateTime from) -> DateTime { return from.ToStartOfMonth(); }));
					classPtr->AddFunction("ToEndOfMonth", make_callable([](DateTime from) -> DateTime { return from.ToEndOfMonth(); }));
					classPtr->AddFunction("ToStartOfDay", make_callable([](DateTime from) -> DateTime { return from.ToStartOfDay(); }));
					classPtr->AddFunction("ToEndOfDay", make_callable([](DateTime from) -> DateTime { return from.ToEndOfDay(); }));
					classPtr->AddFunction("ToStartOfHour", make_callable([](DateTime from) -> DateTime { return from.ToStartOfHour(); }));
					classPtr->AddFunction("ToEndOfHour", make_callable([](DateTime from) -> DateTime { return from.ToEndOfHour(); }));
					classPtr->AddFunction("ToStartOfMinute", make_callable([](DateTime from) -> DateTime { return from.ToStartOfMinute(); }));
					classPtr->AddFunction("ToEndOfMinute", make_callable([](DateTime from) -> DateTime { return from.ToEndOfMinute(); }));

					// Parameters
					classPtr->AddFunction("time", make_callable(&DateTime::time));
				}

			}

			// Built-In functions
			if (1) {
				// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
				this->AddFunction("Type", make_callable([](Any const& obj) -> fibers::Type_Info {
					if (auto p = obj.Type().lock())
						return *p;
					else
						return fibers::user_type<void>();
					}));

				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_string", make_callable([](Any const& x) -> std::string {
					return Units::printf("`%s`", x.TypeName());
					}));
			}
		};
#endif

	private:
		void GetClasses(std::unordered_map<size_t, std::weak_ptr<Class>>& out) const {
			bool DoCleanup = false;
			static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };

			size_t hash{ 0 };
			for (auto& x : Classes) {
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.insert({ hash, x->second });
					}
				}
			}

			if (DoCleanup) {
				const_cast<Global*>(this)->RemoveStaleReferences();
			}
		};
		void GetAllAvailableClassesImpl(
			std::unordered_map<size_t, std::weak_ptr<Class>>& out,
			std::unordered_map<size_t, std::weak_ptr<Scope>>& uniqueLibraries
		) const {
			auto hashed{ Scope::Hasher()(this->p_self) };
			if (uniqueLibraries.count(hashed) > 0) return;
			uniqueLibraries.insert({ hashed, this->p_self });

			this->GetClasses(out);
			static auto badHash{ Scope::Hasher()(std::weak_ptr<Scope>()) };
			size_t hash{ 0 };
			bool DoCleanup = false;
			for (auto& x : Usings) {
				if (x) {
					hash = Scope::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						if (auto ptr = x->second.lock()) {
							if (auto p2 = ptr->GetLibrary()) {
								p2->GetAllAvailableClassesImpl(out, uniqueLibraries);
							}
						}
					}
				}
			}
			if (DoCleanup) {
				//const_cast<Global*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global*>(this)->RemoveStaleReferences();
			}
		};

	private:
		// Searches for all classes that are defined in the current and "used" libraries. 
		std::unordered_map<size_t, std::weak_ptr<Class>> GetAllAvailableClassesImpl() const {
			thread_local static std::unordered_map<size_t, std::weak_ptr<Class>> allClasses{};
			thread_local static std::unordered_map<size_t, std::weak_ptr<Scope>> uniqueList{};
			defer(allClasses.clear());
			defer(uniqueList.clear());

			GetAllAvailableClassesImpl(allClasses, uniqueList);
			return allClasses;
		};
	public:
		std::shared_ptr<std::unordered_map<size_t, std::weak_ptr<Class>>> GetAllAvailableClasses() const {
			auto oldVersion = CachedClassListVersion.load();
			if (oldVersion != RecordVersion) {
				auto guard{ std::unique_lock(const_cast<Global*>(this)->CachedClassListMutex) };
				if (const_cast<Global*>(this)->CachedClassListVersion.CompareExchange(oldVersion, RecordVersion)) {
					return const_cast<Global*>(this)->CachedClassList = std::make_shared<std::unordered_map<size_t, std::weak_ptr<Class>>>(GetAllAvailableClassesImpl());
				}
			}

			if (1) {
				auto guard{ std::shared_lock(const_cast<Global*>(this)->CachedClassListMutex) };
				return CachedClassList;
			}
		};
	public:
		// Creates a tree of type-converter functions using the classes found with GetAllAvailableClasses()
		void CreateTypeConverterTree(std::shared_ptr<TypeConverter>& out/*, bool printOut = false*/) const {
			if (auto classes = GetAllAvailableClasses()) {
				for (auto& FoundClass : *classes) {
					if (auto p = FoundClass.second.lock()) {
						auto& outputType = p->ClassType; //  FoundClass.first;
						// Type Conversions are identical to Constructors with one input type. Therefore ...
						auto className = p->GetName();
						// ... find all constructors ...
						if (auto constructors = p->GetFunctions(className)) {
							for (auto& constructor : *constructors) {
								if (constructor.second) {
									// ... whose inputs are size of 1 ...
									if (constructor.second->m_function->NumArguments() == 1) {
										auto& inputType = constructor.second->m_function->Arguments().Types()[0];

										if (!constructor.second->m_isEplicit) { 
											// TO-DO, add Type Specification to the AddConverter function for the explicit path


											/*out->AddConverter([func = constructor.second->m_function](Any const& input) -> Any {
												return func->operator()({}, )
											}, inputType, outputType);


											out->AddConverter([func = constructor->second, this](Any const& input)->Any {
												return func->first->operator()(const_cast<Any&>(input));
											}, inputType, outputType);*/


											if (constructor->second->cost.has_value()) {
												out->AddConverter([func = constructor->second, this](Any const& input)->Any {
													return func->first->operator()(const_cast<Any&>(input));
												}, inputType, outputType, constructor->second->cost.value());
											}
											else {
												out->AddConverter([func = constructor->second, this](Any const& input)->Any {
													return func->first->operator()(const_cast<Any&>(input));
												}, inputType, outputType);
											}
										}

										//if (printOut) {
										//	auto inputTypeName = inputType.lock()->name();
										//	auto outputTypeName = outputType.lock()->name();
										//	std::cout << Units::printf("%s( %s )\n", outputTypeName, inputTypeName);
										//}
									}
								}
							}
						}
					}
				}
			}
		};

	public:
		virtual std::shared_ptr<TypeConverter> GetTypeConverterTree() const override {
			auto oldVersion = CachedTypeConverterTreeVersion.load();
			if (oldVersion != RecordVersion) {
				auto guard{ std::unique_lock(const_cast<Global*>(this)->CachedTypeConverterTreeMutex) };
				if (const_cast<Global*>(this)->CachedTypeConverterTreeVersion.CompareExchange(oldVersion, RecordVersion)) {
					auto tree = std::make_shared<TypeConverter>();
					CreateTypeConverterTree(tree);
					return const_cast<Global*>(this)->CachedTypeConverterTree = tree;
				}
			}

			if (1) {
				auto guard{ std::shared_lock(const_cast<Global*>(this)->CachedTypeConverterTreeMutex) };
				return CachedTypeConverterTree;
			}
		};

	private:
		fibers::containers::Map<size_t, std::weak_ptr<Class>> // collection of all classes that are added as "children" of this library
			Classes;
		fibers::containers::Map<size_t, std::weak_ptr<Namespace>> // collection of all namespaces that are added being "used" by this library
			Usings;
		fibers::containers::Map<size_t, std::pair<std::string, std::weak_ptr<details::Proxy_Function_Base>>> // collection of all namespaces that are added being "used" by this library
			Functions;

		std::shared_ptr<TypeConverter>
			CachedTypeConverterTree{ std::make_shared<TypeConverter>() };
		fibers::containers::number<unsigned __int64>
			CachedTypeConverterTreeVersion{ 0 };
		std::shared_mutex // fibers::synchronization::shared_mutex<fibers::synchronization::mutex>
			CachedTypeConverterTreeMutex{};

		std::shared_ptr<std::unordered_map<size_t, std::weak_ptr<Class>>>
			CachedClassList{ std::make_shared<std::unordered_map<size_t, std::weak_ptr<Class>>>() };
		fibers::containers::number<unsigned __int64>
			CachedClassListVersion{ 0 };
		std::shared_mutex // fibers::synchronization::shared_mutex<fibers::synchronization::mutex>
			CachedClassListMutex{};

		//fibers::containers::number<unsigned int> 
			//CleanupRequested{ 0 };
		fibers::containers::number<unsigned __int64>
			CleanupVersion{ 0 };
		fibers::containers::number<unsigned __int64>
			RecordVersion{ 0 };
		fibers::containers::number<unsigned __int64>
			LibraryVersion{ 0 };

		virtual void RemoveStaleReferences() override {
			auto oldVersion = CleanupVersion.load();
			if (oldVersion != RecordVersion) {
				if (CleanupVersion.CompareExchange(oldVersion, RecordVersion)) {
					// p_using
					while (true) {
						size_t toRemove{};
						bool doRemoval = false;
						for (auto& ref : p_using) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove = ref->first;
									doRemoval = true;
									break;
								}
							}
						}
						if (doRemoval) {
							p_using.erase(toRemove);
						}
						else {
							break;
						}
					}

					// p_postfixes
					while (true) {
						std::string toRemove{};
						bool doRemoval = false;
						for (auto& ref : p_postfixes) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove = ref->first;
									doRemoval = true;
									break;
								}
							}
						}
						if (doRemoval) {
							p_postfixes.erase(toRemove);
						}
						else {
							break;
						}
					}

					// Classes
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Classes) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Classes.erase(toRemove);
						toRemove.clear();
					}

					// Usings
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Usings) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Usings.erase(toRemove);
						toRemove.clear();
					}

					// Functions
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Functions) {
							if (ref) {
								if (ref->second.second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Functions.erase(toRemove);
						toRemove.clear();
					}
				}
			}
		};

		virtual bool RecordClass(std::shared_ptr<Class> ptr, bool overrideIfExists = true) override {
			if (Classes.emplace(Scope::Hasher()(ptr), ptr, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};

		virtual bool RecordUsing(std::shared_ptr<Namespace> ptr, bool overrideIfExists = true) override {
			if (Usings.emplace(Scope::Hasher()(ptr), ptr, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};

		virtual bool RecordFunction(std::string const& Name, Function const& ptr, bool overrideIfExists = true) override {
			if (Functions.emplace(std::hash<Proxy_Function>()(ptr), { Name, ptr }, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};

	};
};






