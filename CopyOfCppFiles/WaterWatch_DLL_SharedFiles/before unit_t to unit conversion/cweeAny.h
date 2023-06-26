#pragma once

#include "Precompiled.h"
#include <boost/any.hpp>
#include <map>
#include <memory>
#include <type_traits>

#define useChaiscriptBoxedValue
#define preventForwarding

template<class T> struct extract_type { using type = T; };
template<class T> struct extract_type<cweeSharedPtr<T>> { using type = typename extract_type<T>::type; };
template<class T> struct extract_type<cweeSharedPtr<T>&> { using type = typename extract_type<T>::type; };
template<class T> struct extract_type<cweeSharedPtr<T>*> { using type = typename extract_type<T>::type; };
template<class T> struct extract_type<const cweeSharedPtr<T>> { using type = typename extract_type<T>::type; };
template<class T> struct extract_type<const cweeSharedPtr<T>&> { using type = typename extract_type<T>::type; };
template<class T> struct extract_type<const cweeSharedPtr<T>*> { using type = typename extract_type<T>::type; };

class cweeAnyData {
public:
	cweeAnyData(cweeSharedPtr<void> const& t_ptr, const boost::typeindex::type_info& t_type, bool t_const) noexcept : m_ptr(t_ptr), m_type(t_type), m_const(t_const) {};
	virtual ~cweeAnyData() noexcept {};

public:
	template<typename ToType> cweeSharedPtr<ToType> cast() const {
		return cweeSharedPtr<ToType>(m_ptr, [](void* p) {
			return static_cast<ToType*>(p);
		});
	};

public:
	const bool							m_const;
	const boost::typeindex::type_info& m_type;
	cweeSharedPtr<void>					m_ptr;

};
template<typename T> class cweeAnyData_Impl final : public cweeAnyData {
public:
	cweeAnyData_Impl() noexcept : cweeAnyData(nullptr, boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
	cweeAnyData_Impl(cweeSharedPtr<T> const& d) noexcept : cweeAnyData(cweeAnyData_Impl<T>::get_data(d), boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
	cweeAnyData_Impl(cweeSharedPtr<T>&& d) noexcept : cweeAnyData(cweeAnyData_Impl<T>::get_data(std::forward<cweeSharedPtr<T>>(d)), boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
	~cweeAnyData_Impl() noexcept {};

	static cweeSharedPtr<void> get_data(const cweeSharedPtr<T>& data) { return cweeSharedPtr<void>(data, [](void* p) { return p; }); };
	static cweeSharedPtr<void> get_data(cweeSharedPtr<T>&& data) { return cweeSharedPtr<void>(std::forward<cweeSharedPtr<T>>(data), [](void* p) { return p; }); };
};
class cweeAnyAutoCast; // forward

/*! Generic container that enables the containment and sharing of any data type to/from cweeSharedPtrs */
class cweeAny {
public:
	struct Object_Data {
		template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, cweeSharedPtr<S>>>> static AUTO get(const H<S>* obj) { return get(*obj); };
		template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, cweeSharedPtr<S>>>> static AUTO get(const H<S>& obj) {
			return cweeSharedPtrWithSentinel<cweeAnyData_Impl<S>>(new cweeAnyData_Impl<S>(obj)).CastReference<cweeAnyData>();
		};
		template <template<class> class H, class S, typename = std::enable_if_t<std::is_same_v<H<S>, cweeSharedPtr<S>>>> static AUTO get(H<S>&& obj) {
			return cweeSharedPtrWithSentinel<cweeAnyData_Impl<S>>(new cweeAnyData_Impl<S>(std::forward<H<S>>(obj))).CastReference<cweeAnyData>();
		};
		template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeAnyAutoCast, T>>> static AUTO get(T* t) { cweeSharedPtr<T> sp = make_cwee_shared<T>(t); return get(sp); };
		template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeAnyAutoCast, T>>> static AUTO get(const T* t) { return get(*t); };
		template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeAnyAutoCast, T>>> static AUTO get(const T& obj) { cweeSharedPtr<T> sp = make_cwee_shared<T>(obj); return get(sp); };
		static AUTO get(const cweeAnyAutoCast& obj);
		static AUTO get(const cweeAnyAutoCast* t);
	};
	template<typename ValueType> static cweeSharedPtrWithSentinel<cweeAnyData> CreateContainer(const ValueType& r) { return Object_Data::get(r); };
	template<typename ValueType> static cweeSharedPtrWithSentinel<cweeAnyData> CreateContainer(ValueType&& r) { return Object_Data::get(std::forward<ValueType>(r)); };

public: /*! Init */
	constexpr cweeAny() noexcept : container(nullptr) {};
	constexpr cweeAny(std::nullptr_t) noexcept : container(nullptr) {};
	cweeAny(const cweeAny& rhs) noexcept : container(rhs.container) {};
	cweeAny(cweeAny&& rhs) noexcept : container(rhs.container) { rhs.container = nullptr; };

public: /*! Init w/ DATA ASSIGNMENT */
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny(const ValueType& value) noexcept : container(CreateContainer(value)) {};
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny(const ValueType* value) noexcept : container(CreateContainer(value)) {};
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny(ValueType* value) noexcept : container(CreateContainer(value)) {};
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny(ValueType&& value) noexcept : container(CreateContainer(std::forward<ValueType>(value))) {};

public: /*! Destroy */
	~cweeAny() noexcept { container = nullptr; };

public: /*! Data Assignment AFTER INIT */
	cweeAny& swap(cweeAny& rhs) noexcept {
		if (this == &rhs) { return *this; }
		container.swap(rhs.container);
		return *this;
	};
	cweeAny& operator=(const cweeAny& rhs) noexcept {
		cweeAny(rhs).swap(*this);
		return *this;
	};
	cweeAny& operator=(cweeAny&& rhs) noexcept {
		cweeAny(std::forward<cweeAny>(rhs)).swap(*this);
		return *this;
	};

	cweeAny& operator=(std::nullptr_t) noexcept { Clear(); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny& operator=(const ValueType& rhs) noexcept { CreateContainer(rhs).swap(container); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny& operator=(const ValueType* rhs) noexcept { CreateContainer(rhs).swap(container); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny& operator=(ValueType* rhs) noexcept { CreateContainer(rhs).swap(container); return *this; };
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> cweeAny& operator=(ValueType&& rhs) noexcept { CreateContainer(std::forward<ValueType>(rhs)).swap(container); return *this; };

public:
	/*! Checks if the cweeAny has been assigned something */
	bool IsEmpty() const noexcept { return (bool)container; };

	/*! Empties the cweeAny and frees the memory. */
	void Clear() noexcept { cweeAny().swap(*this); };

	template <typename ValueT> static const char* TypeNameOf() { return TypeOf<ValueT>().name(); };
	template <typename ValueT> static const boost::typeindex::type_info& TypeOf() { return boost::typeindex::type_id<ValueT>().type_info(); };

	const char* TypeName() const noexcept { return Type().name(); };
	const boost::typeindex::type_info& Type() const noexcept {
		cweeSharedPtrWithSentinel<cweeAnyData> m = container;
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
	constexpr explicit operator bool() const { return (bool)container; };
	friend bool operator==(const cweeAny& a, const cweeAny& b) noexcept { return a.container == b.container; };
	friend bool operator!=(const cweeAny& a, const cweeAny& b) noexcept { return a.container != b.container; };
	friend bool operator<(const cweeAny& a, const cweeAny& b) noexcept { return a.container < b.container; };
	friend bool operator<=(const cweeAny& a, const cweeAny& b) noexcept { return a.container <= b.container; };
	friend bool operator>(const cweeAny& a, const cweeAny& b) noexcept { return a.container > b.container; };
	friend bool operator>=(const cweeAny& a, const cweeAny& b) noexcept { return a.container >= b.container; };
	friend bool operator==(const cweeAny& a, std::nullptr_t) noexcept { return a.container == nullptr; };
	friend bool operator!=(const cweeAny& a, std::nullptr_t) noexcept { return a.container != nullptr; };
	friend bool operator<(const cweeAny& a, std::nullptr_t) noexcept { return a.container < nullptr; };
	friend bool operator<=(const cweeAny& a, std::nullptr_t) noexcept { return a.container <= nullptr; };
	friend bool operator>(const cweeAny& a, std::nullptr_t) noexcept { return a.container > nullptr; };
	friend bool operator>=(const cweeAny& a, std::nullptr_t) noexcept { return a.container >= nullptr; };
	friend bool operator==(std::nullptr_t, const cweeAny& a) noexcept { return nullptr == a.container; };
	friend bool operator!=(std::nullptr_t, const cweeAny& a) noexcept { return nullptr != a.container; };
	friend bool operator<(std::nullptr_t, const cweeAny& a) noexcept { return nullptr < a.container; };
	friend bool operator<=(std::nullptr_t, const cweeAny& a) noexcept { return nullptr <= a.container; };
	friend bool operator>(std::nullptr_t, const cweeAny& a) noexcept { return nullptr > a.container; };
	friend bool operator>=(std::nullptr_t, const cweeAny& a) noexcept { return nullptr >= a.container; };
#pragma endregion

public:
	class DataCaster {
	public:
		template<typename T> struct is_cweeSharedPtr_class { using type = std::false_type; };
		template<typename T> struct is_cweeSharedPtr_class<cweeSharedPtr<T>> { using type = std::true_type; };
		template<typename T> struct is_cweeSharedPtr_class<cweeSharedPtr<T>&> { using type = std::true_type; };
		template<typename T> struct is_cweeSharedPtr_class<cweeSharedPtr<T>*> { using type = std::true_type; };
		template<typename T> struct is_cweeSharedPtr_class<const cweeSharedPtr<T>> { using type = std::true_type; };
		template<typename T> struct is_cweeSharedPtr_class<const cweeSharedPtr<T>&> { using type = std::true_type; };
		template<typename T> struct is_cweeSharedPtr_class<const cweeSharedPtr<T>*> { using type = std::true_type; };
		template<typename T> struct is_cweeSharedPtr_class<cweeSharedPtr<T>&&> { using type = std::true_type; };

	private:
		template <class VType> static decltype(auto) DoCast_Shared(cweeAny* p) noexcept {
			cweeSharedPtrWithSentinel<cweeAnyData> m = p->container;
			AUTO ptr = m.Get();
			if (ptr) {
				return ptr->cast<VType>();
			}
			else {
				AUTO q = make_cwee_shared<VType>();
				p->container = cweeAny::CreateContainer(q);
				return q;
			}
		};

		template <class VType> static decltype(auto) DoCast_Shared_Sentinel(cweeAny* p) noexcept {
#ifdef cweeSharedPtrWithSentinel 
			static_assert(false, "Casting cweeAny to  cweeSharedPtr<T>* or  cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
			throw("Casting cweeAny to  cweeSharedPtr<T>* or  cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
			cweeSharedPtr<VType>* finalOut = nullptr;
#else

			cweeSharedPtr<VType> out;

			cweeSharedPtrWithSentinel<cweeAnyData> m = p->container;
			AUTO ptr = m.Get();
			if (ptr) {
				out = ptr->cast<VType>();
			}
			else {
				out = make_cwee_shared<VType>();
				p->container = cweeAny::CreateContainer(out);
			}

			p->container.Lock();
			AUTO finalOut = p->container.UnsafeSentinel<VType>(out); // cweeSharedPtr<VType>* const& finalOut
			p->container.Unlock();
#endif
			return finalOut;
		};

		template<typename VType> static decltype(auto) DoCast_Unshared(cweeAny* p) noexcept {
			constexpr bool is_ptr = std::is_pointer_v<VType>;
			constexpr bool is_ref = std::is_reference_v<VType>;

			typedef typename std::remove_reference<typename std::remove_pointer<VType>::type>::type desiredT;
			cweeSharedPtrWithSentinel<cweeAnyData> m = p->container;
			if (m) {
				cweeSharedPtr<desiredT> ptr = m->cast<desiredT>();
				if constexpr (is_ptr) {
					return ptr.Get();
				}
				else {
					return *ptr.Get();
				}
			}
			else {
				cweeSharedPtr<desiredT> q = make_cwee_shared<desiredT>();
				p->container = cweeAny::CreateContainer(q);
				if constexpr (is_ptr) {
					return q.Get();
				}
				else {
					return *q.Get();
				}
			}
		};

	public:
		template<typename T> static decltype(auto) DoCast(cweeAny* p) noexcept {
			typedef is_cweeSharedPtr_class<T>::type isShared;
			constexpr bool is_cwee_shared_ptr = isShared::value;
			constexpr bool is_ptr = std::is_pointer_v<T>;
			constexpr bool is_ref = std::is_reference_v<T>;
			if constexpr (is_cwee_shared_ptr) {
				typedef typename extract_type<T>::type innertype;
				if constexpr (is_ptr) {
#ifdef cweeSharedPtrWithSentinel 
					static_assert(false, "Casting cweeAny to cweeSharedPtr<T>* or  cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
					throw("Casting cweeAny to cweeSharedPtr<T>* or cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
#else
					return DoCast_Shared_Sentinel<innertype>(p);
#endif

				}
				else if constexpr (is_ref) {
#ifdef cweeSharedPtrWithSentinel 
					static_assert(false, "Casting cweeAny to cweeSharedPtr<T>* or  cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
					throw("Casting cweeAny to cweeSharedPtr<T>* or cweeSharedPtr<T>& is not recommended due to lifetime management concerns. Suggest changing cast to cweeSharedPtr<T>.");
#else
					return *DoCast_Shared_Sentinel<innertype>(p);
#endif
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

	template<typename VType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
	AUTO cast() const noexcept { return DataCaster::DoCast<VType>(const_cast<cweeAny*>(this)); };

	template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value&& std::is_same_v<cweeAny, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
	cweeAny& cast() const noexcept { return *const_cast<cweeAny*>(this); };

	template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value&& std::is_same_v<cweeAny, std::decay_t<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type>>>>
	cweeAny* cast() const noexcept { return const_cast<cweeAny*>(this); };

	cweeAnyAutoCast cast() const noexcept;

public:
	mutable cweeSharedPtrWithSentinel<cweeAnyData> container;
};
class cweeAnyAutoCast {
public:
	cweeAnyAutoCast(const cweeAny* _parent) : parent(const_cast<cweeAny*>(_parent)), parentCopy(*_parent) {};
	cweeAnyAutoCast(cweeAnyAutoCast&& other) : parent(std::move(other.parent)), parentCopy(std::move(other.parentCopy)) {};

	cweeAnyAutoCast() = delete;
	cweeAnyAutoCast(const cweeAnyAutoCast&) = delete;
	cweeAnyAutoCast& operator=(const cweeAnyAutoCast&) = delete;
	cweeAnyAutoCast& operator=(cweeAnyAutoCast&&) = delete;
	~cweeAnyAutoCast() {};

	explicit operator cweeAny& () const noexcept { return *parent; };
	explicit operator cweeAny* () const noexcept { return parent; };

	template <typename T> operator cweeSharedPtr<T>() const noexcept {
		return parentCopy.cast<cweeSharedPtr<T>>();
	};
	template <typename T> operator cweeSharedPtr<T>* () const noexcept {
		return parentCopy.cast<cweeSharedPtr<T>*>();
	};

	template< bool cond, typename U > using resolvedType = typename std::enable_if< cond, U >::type;
	template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!cweeAny::DataCaster::is_cweeSharedPtr_class<ValueTypeT>::type::value> >
	operator ValueTypeT& () const noexcept {
		return *parentCopy.cast<ValueTypeT*>();
	};
	template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!cweeAny::DataCaster::is_cweeSharedPtr_class<ValueTypeT>::type::value> >
	operator ValueTypeT* () const noexcept {
		return parentCopy.cast<ValueTypeT*>();
	};

	cweeAny* parent;
	cweeAny parentCopy; /* Allows for the following type of interaction:
						cweeStr foo() const noexcept {
							return cweeJob([](float R){ return R + 1; }, 100.0f).Invoke().cast(); // without the parentCopy, the cweeAny may go out of scope before the cweeAnyAutoCast and will caush an exception.
						}
						*/

	void TESTING()
	{
		cweeAnyAutoCast caster = cweeAny().cast();
		cweeAny any1 = caster;
		cweeAny& any2 = caster;
		cweeAny* any3 = caster;

		cweeStr a = caster;
		cweeStr& b = caster;
		cweeStr* c = caster;

		cweeSharedPtr<cweeStr> p1 = caster;
		cweeSharedPtr<cweeAny> p2 = caster;
		cweeSharedPtr<cweeAny>& p3 = caster;
		cweeSharedPtr<cweeAny>* p4 = caster;
	};
};
INLINE cweeAnyAutoCast cweeAny::cast() const noexcept { return cweeAnyAutoCast(this); };
INLINE AUTO cweeAny::Object_Data::get(const cweeAnyAutoCast& obj) { cweeAny* t = const_cast<cweeAny*>(obj.parent); cweeSharedPtrWithSentinel<cweeAnyData> out; if (t) { out = t->container; } return out; };
INLINE AUTO cweeAny::Object_Data::get(const cweeAnyAutoCast* t) { return get(*t); };

// #define HashType(_type_) cweeStr::hash(cweeAny::TypeNameOf<_type_>())
#define HashType(_type_) cweeStr::hash(typenames::type_name<_type_>())
#define HashStr(_str_) cweeStr::hash(_str_)