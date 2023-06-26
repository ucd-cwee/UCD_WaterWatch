#ifndef __CWEEANY_H__
#define __CWEEANY_H__

#include <boost/any.hpp>
#include <map>
#include <memory>
#include <type_traits>

#define useChaiscriptBoxedValue
#define preventForwarding

// static cweeSysMutex cweeAnyTypeComparisonLock;
#include "chaiscript_defines.hpp"
#include "dispatchkit/any.hpp"
#include "dispatchkit/type_info.hpp"

#if 0

#ifdef useChaiscriptBoxedValue
#include "chaiscript_defines.hpp"
#include "dispatchkit/any.hpp"
#include "dispatchkit/type_info.hpp"






class BoxedAny {
private:
	struct Data {
		// Data() noexcept : m_type(boost::typeindex::type_id<void>().type_info()), isConst(true) {};

		explicit Data(const boost::typeindex::type_info& t_type, bool _IsConst, bool _isSharedPtr) noexcept : m_type(t_type), isConst(_IsConst), isSharedPtr(_isSharedPtr) {};

		Data& operator=(const Data&) = delete;

		virtual ~Data() noexcept = default;

		virtual void* data() const noexcept = 0;
		virtual void* get_shared_data() const noexcept = 0;

		const boost::typeindex::type_info& type() const noexcept { return m_type; };
		bool IsConst() const noexcept { return isConst; };

		virtual cweeSharedPtr<Data> clone() const = 0;

		const boost::typeindex::type_info& m_type;

		bool isConst;

		bool isSharedPtr;
	};

	template<typename T>
	struct Data_Impl : Data {
		explicit Data_Impl(T t_type) : Data(boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>, false), m_data(std::move(t_type)) {};

		void* data() const noexcept override final { return &m_data; };
		void* get_shared_data() const noexcept override final { return &m_data; };

		cweeSharedPtr<Data> clone() const override final { make_cwee_shared< Data_Impl<T>>(m_data).CastReference<Data>(); };

		template<typename ToType>
		Data_Impl& operator=(const Data_Impl&) = delete;
		~Data_Impl() {}
		mutable T m_data;
	};

	template<typename T>
	struct Data_Shared_Impl : Data {
		explicit Data_Shared_Impl(cweeSharedPtr<T> t_type) : Data(boost::typeindex::type_id<cweeSharedPtr<T>>().type_info(), std::is_const_v<cweeSharedPtr<T>>, true), m_data(std::move(t_type)) {};

		void* data() const noexcept override final { return &m_data; };
		void* get_shared_data() const noexcept override final { return m_data.Get(); };

		cweeSharedPtr<Data> clone() const override final { make_cwee_shared< Data_Impl<T>>(m_data).CastReference<Data>(); };

		template<typename ToType>
		Data_Shared_Impl& operator=(const Data_Shared_Impl&) = delete;
		~Data_Shared_Impl() { m_data = nullptr; };
		mutable cweeSharedPtr<T> m_data;
	};

	cweeSharedPtr<Data> m_data;

public:
	const Data* get() const noexcept { if (m_data) return m_data.Get(); else return nullptr; };

	// construct/copy/destruct
	BoxedAny() noexcept : m_data(nullptr) { }; // = default;
	BoxedAny(const BoxedAny& t_any) : m_data(t_any.empty() ? nullptr : t_any.m_data->clone()) { };
	BoxedAny& operator=(const BoxedAny& t_any) { BoxedAny copy(t_any); swap(copy); return *this; };

	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<BoxedAny, std::decay_t<ValueType>>>>
	explicit BoxedAny(ValueType&& t_value) : m_data(make_cwee_shared<Data_Impl<std::decay_t<ValueType>>>(std::forward<ValueType>(t_value)).CastReference<Data>()) {};

	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<BoxedAny, std::decay_t<ValueType>>>>
	explicit BoxedAny(cweeSharedPtr<ValueType>&& t_value) : m_data(make_cwee_shared<Data_Shared_Impl<std::decay_t<ValueType>>>(std::forward<cweeSharedPtr<ValueType>>(t_value)).CastReference<Data>()) {}

	template<typename ToType> ToType& cast() const { AUTO m = m_data; return *static_cast<ToType*>(m->get_shared_data()); };

	template<typename ToType>
	ToType& force_cast() const { AUTO m = m_data; return *static_cast<ToType*>(m->get_shared_data()); };

	template<typename ToType>
	ToType& ptr_cast() const { AUTO m = m_data; return *static_cast<ToType*>(m->data()); };

	template<typename ToType>
	ToType& force_ptr_cast() const { AUTO m = m_data; return *static_cast<ToType*>(m->data()); };

	// modifiers
	BoxedAny& swap(BoxedAny& t_other) {
		m_data.Swap(t_other.m_data);

		return *this;
	}

	// queries
	bool empty() const noexcept { return !static_cast<bool>(m_data); }

	const boost::typeindex::type_info& type() const noexcept {
		AUTO m = m_data;
		if (m) {
			return m->type();
		}
		else {
			return boost::typeindex::type_id<void>().type_info();
		}
	}
};

namespace detail {
	template<typename T>
	class cweeCastHelper;
}

/// \brief A wrapper for holding any valid C++ type.
class cweeBoxedValue {
public:
	/// used for explicitly creating a "void" object
	struct Void_Type {
	};

private:
	/// structure which holds the internal state of a cweeBoxedValue
	/// \todo Get rid of Any and merge it with this, reducing an allocation in the process
	struct Data {
		Data(BoxedAny to, bool is_ref, const void* t_void_ptr, const void* t_ptr_ptr = nullptr)
			: m_obj(std::move(to))
			, m_data_ptr(const_cast<void*>(t_void_ptr))
			, m_ptr_ptr(const_cast<void*>(t_ptr_ptr))
			, m_is_ref(is_ref)
		{ };

		Data& operator=(const Data& rhs) = delete;
		Data& operator=(Data&& rhs) = delete;

		Data(const Data&) = delete;
		Data(Data&&) = delete;

		const boost::typeindex::type_info& Type() {
			return m_obj.type();
		};
		BoxedAny m_obj;
		void* m_data_ptr;
		void* m_ptr_ptr;
		bool m_is_ref;
	};

	struct Object_Data {
		static auto get(cweeBoxedValue::Void_Type) {
			return make_cwee_shared<Data>(BoxedAny(), false, nullptr, nullptr);
		}

		template<typename T>
		static auto get(const cweeSharedPtr<T>* obj) {
			return get(*obj);
		}

		template<typename T>
		static auto get(const cweeSharedPtr<T>& obj) {
			AUTO ba = BoxedAny(cweeSharedPtr<T>(obj));
			AUTO ptr_loc = ba.get()->data();
			return make_cwee_shared<Data>(std::move(ba), false, obj.get(), ptr_loc); // obj.get());
		}

		template<typename T>
		static auto get(T* t) {
			return get(std::ref(*t));
		}

		template<typename T>
		static auto get(const T* t) {
			return get(std::cref(*t));
		}

		template<typename T>
		static auto get(std::reference_wrapper<T> obj) {
			auto p = &obj.get();
			AUTO ba = BoxedAny(std::move(obj));
			return make_cwee_shared<Data>(std::move(ba), true, p, nullptr);
		}

		//template<typename T>
		//static auto get(cweeSharedPtr<T>&& obj) {
		//	auto ptr = obj.get();
		//	return make_cwee_shared<Data>( BoxedAny(make_cwee_shared<cweeSharedPtr<T>>(std::move(obj))), true, ptr);
		//}

		template<typename T>
		static auto get(T t) {
			auto p = make_cwee_shared<T>(std::move(t));
			auto ptr = p.get();
			AUTO ba = BoxedAny(std::move(p));
			AUTO ptr_loc = ba.get()->data();
			return make_cwee_shared<Data>(std::move(ba), false, ptr, ptr_loc);
		}

		static cweeSharedPtr<Data> get() {
			return make_cwee_shared<Data>(BoxedAny(), false, nullptr, nullptr);
		};
	};

public:
	template<typename T> static const T* verify_type_no_throw(const cweeBoxedValue& ob, const T* ptr) {
		if (ob.is_null())
			return nullptr;
		else
			return ptr;
	}
	template<typename T> static T* verify_type_no_throw(const cweeBoxedValue& ob, T* ptr) {
		if (ob.is_null())
			return nullptr;
		else
			return ptr;
	}
	template<typename T> static const T* verify_type(const cweeBoxedValue& ob, const T* ptr) {
		cweeStackTrace::GetTrace();
		return ptr;
	}
	template<typename T> static T* verify_type(const cweeBoxedValue& ob, T* ptr) {
		cweeStackTrace::GetTrace();
		return ptr;
	}

	/// Generic cweeCastHelper_Inner, for casting to any type
	template<typename Result>
	struct cweeCastHelper_Inner {
		static Result cast(const cweeBoxedValue& ob) {
			return *static_cast<const Result*>(verify_type(ob, ob.get_const_ptr()));
		}
	};

	template<typename Result> struct cweeCastHelper_Inner<const Result> : cweeCastHelper_Inner<Result> {};

	/// cweeCastHelper_Inner for casting to a const * type
	template<typename Result>
	struct cweeCastHelper_Inner<const Result*> {
		static const Result* cast(const cweeBoxedValue& ob) {
			return static_cast<const Result*>(verify_type_no_throw(ob, ob.get_const_ptr()));
		}
	};

	/// cweeCastHelper_Inner for casting to a * type
	template<typename Result>
	struct cweeCastHelper_Inner<Result*> {
		static Result* cast(const cweeBoxedValue& ob) {
			return static_cast<Result*>(verify_type_no_throw(ob, ob.get_ptr()));
		}
	};

	template<typename Result> struct cweeCastHelper_Inner<Result* const&> : public cweeCastHelper_Inner<Result*> {};

	template<typename Result> struct cweeCastHelper_Inner<const Result* const&> : public cweeCastHelper_Inner<const Result*> {};

	/// cweeCastHelper_Inner for casting to a & type
	template<typename Result>
	struct cweeCastHelper_Inner<const Result&> {
		static const Result& cast(const cweeBoxedValue& ob) {
			return *static_cast<const Result*>(verify_type(ob, ob.get_const_ptr()));
		}
	};

	/// cweeCastHelper_Inner for casting to a & type
	template<typename Result>
	struct cweeCastHelper_Inner<Result&> {
		static Result& cast(const cweeBoxedValue& ob) {
			return *static_cast<Result*>(verify_type(ob, ob.get_ptr()));
		}
	};

	/// cweeCastHelper_Inner for casting to a cweeSharedPtr<> type
	template<typename Result>
	struct cweeCastHelper_Inner<cweeSharedPtr<Result>> {
		static auto cast(const cweeBoxedValue& ob) {
			return *static_cast<cweeSharedPtr<Result>>(verify_type(ob, ob.get_ptr(true)));
			// return ob.get().ptr_cast<cweeSharedPtr<Result>>();
		}
	};

	/// cweeCastHelper_Inner for casting to a cweeSharedPtr<const> type
	template<typename Result>
	struct cweeCastHelper_Inner<cweeSharedPtr<const Result>> {
		static auto cast(const cweeBoxedValue& ob) {
			if (!ob.is_const()) {
				return static_cast<cweeSharedPtr<Result>>(verify_type(ob, ob.get_ptr(true)))->ConstReference();
				// return ob.get().ptr_cast<cweeSharedPtr<Result>>().ConstReference();
			}
			else {
				return *static_cast<cweeSharedPtr<const Result>>(verify_type(ob, ob.get_ptr(true)));
				// return ob.get().ptr_cast<cweeSharedPtr<const Result>>();
			}
		}
	};

	/// cweeCastHelper_Inner for casting to a const cweeSharedPtr<> & type
	template<typename Result> struct cweeCastHelper_Inner<const cweeSharedPtr<Result>> : cweeCastHelper_Inner<cweeSharedPtr<Result>> {};
	template<typename Result> struct cweeCastHelper_Inner<const cweeSharedPtr<Result>&> : cweeCastHelper_Inner<cweeSharedPtr<Result>> {};

	template<typename Result>
	struct cweeCastHelper_Inner<cweeSharedPtr<Result>&> {
		// static_assert(!std::is_const<Result>::value, "Non-const reference to cweeSharedPtr<const T> is not supported");
		static auto cast(const cweeBoxedValue& ob) {
			if (ob.is_pointer()) {
				return *static_cast<cweeSharedPtr<Result>>(verify_type(ob, ob.get_ptr(true)));
			}
			else {
				cweeSharedPtr<Result>& res = ob.get().ptr_cast<cweeSharedPtr<Result>>();
				return ob.pointer_sentinel(res);
			}
		}
	};

	/// cweeCastHelper_Inner for casting to a const cweeSharedPtr<const> & type
	template<typename Result> struct cweeCastHelper_Inner<const cweeSharedPtr<const Result>> : cweeCastHelper_Inner<cweeSharedPtr<const Result>> {};
	template<typename Result> struct cweeCastHelper_Inner<const cweeSharedPtr<const Result>&> : cweeCastHelper_Inner<cweeSharedPtr<const Result>> {};

	/// cweeCastHelper_Inner for casting to a cweeBoxedValue type
	template<>
	struct cweeCastHelper_Inner<cweeBoxedValue> {
		static cweeBoxedValue cast(const cweeBoxedValue& ob) { return ob; }
	};

	/// cweeCastHelper_Inner for casting to a cweeBoxedValue & type
	template<>
	struct cweeCastHelper_Inner<cweeBoxedValue&> {
		static std::reference_wrapper<cweeBoxedValue> cast(const cweeBoxedValue& ob) {
			return std::ref(const_cast<cweeBoxedValue&>(ob));
		}
	};

	/// cweeCastHelper_Inner for casting to a const cweeBoxedValue & type
	template<> struct cweeCastHelper_Inner<const cweeBoxedValue> : cweeCastHelper_Inner<cweeBoxedValue> {};

	template<> struct cweeCastHelper_Inner<const cweeBoxedValue&> : cweeCastHelper_Inner<cweeBoxedValue> {};

	/// cweeCastHelper_Inner for casting to a std::reference_wrapper type
	template<typename Result> struct cweeCastHelper_Inner<std::reference_wrapper<Result>> : cweeCastHelper_Inner<Result&> {};

	template<typename Result> struct cweeCastHelper_Inner<const std::reference_wrapper<Result>> : cweeCastHelper_Inner<Result&> {};

	template<typename Result> struct cweeCastHelper_Inner<const std::reference_wrapper<Result>&> : cweeCastHelper_Inner<Result&> {};

	template<typename Result> struct cweeCastHelper_Inner<std::reference_wrapper<const Result>> : cweeCastHelper_Inner<const Result&> {};

	template<typename Result> struct cweeCastHelper_Inner<const std::reference_wrapper<const Result>> : cweeCastHelper_Inner<const Result&> {};

	template<typename Result> struct cweeCastHelper_Inner<const std::reference_wrapper<const Result>&> : cweeCastHelper_Inner<const Result&> {};

	/// The exposed cweeCastHelper object that by default just calls the cweeCastHelper_Inner
	template<typename T>
	class cweeCastHelper {
	public:
		static decltype(auto) cast(const cweeBoxedValue& ob) {
			return (cweeCastHelper_Inner<T>::cast(ob));
		}
	};

public:
	/// Basic cweeBoxedValue copy constructor
	template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeBoxedValue, std::decay_t<T>>>>
	explicit cweeBoxedValue(const T& t) : m_data(Object_Data::get(t)) {};

	template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeBoxedValue, std::decay_t<T>>>>
	explicit cweeBoxedValue(T&& t) : m_data(Object_Data::get(std::forward<T>(t))) {};

	template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeBoxedValue, std::decay_t<T>>>>
	cweeBoxedValue& operator=(const T& t) {
		cweeBoxedValue(t).swap(*this);
		return *this;
	};

	template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeBoxedValue, std::decay_t<T>>>>
	cweeBoxedValue& operator=(T&& t) {
		cweeBoxedValue(std::forward<T>(t)).swap(*this);
		return *this;
	};

	template<typename T, typename = std::enable_if_t<!std::is_same_v<cweeBoxedValue, std::decay_t<T>>>>
	explicit operator T() const {
		{
			try {
				return detail::cweeCastHelper<T>::cast(*this);
			}
			catch (const chaiscript::detail::exception::bad_any_cast&) {

			}
		}
		throw(std::runtime_error(cweeStr::printf("Bad Cast From '%s' to '%s'", this->get_type_info().name(), boost::typeindex::type_id<T>().type_info().name()).c_str()));
	};

	template<typename T>
	T cast() const {
		{
			try {
				return cweeCastHelper<T>::cast(*this);
			}
			catch (const chaiscript::detail::exception::bad_any_cast&) {

			}
		}
		throw(std::runtime_error(cweeStr::printf("Bad Cast From '%s' to '%s'", this->get_type_info().name(), boost::typeindex::type_id<T>().type_info().name()).c_str()));
	};

	/// Unknown-type constructor
	cweeBoxedValue() : m_data(Object_Data::get()) {};
	cweeBoxedValue(const cweeBoxedValue& rhs) : m_data(rhs.m_data) {};
	cweeBoxedValue& operator=(const cweeBoxedValue& rhs) { m_data = rhs.m_data; return *this; };

	void swap(cweeBoxedValue& rhs) noexcept {
		AUTO x = m_data;
		m_data = rhs.m_data;
		rhs.m_data = x;
	};

	const boost::typeindex::type_info& get_type_info() const noexcept {
		AUTO m = m_data;
		if (m)
			return m->Type();
		else
			return boost::typeindex::type_id<void>().type_info();
	};

	/// return true if the object is uninitialized
	bool is_undef() const noexcept {
		AUTO m = m_data;
		if (m) {
			return m->Type() == boost::typeindex::type_id<void>().type_info();
		}
		return false;
	};

	/// return true if the object is uninitialized
	bool is_const() const noexcept {
		AUTO m = m_data;
		if (m) {
			decltype(auto) ptr = m->m_obj.get();
			if (ptr) {
				return ptr->IsConst();
			}
		}
		return false;
	};

	bool is_type(const boost::typeindex::type_info& ti) const noexcept {
		return get_type_info() == ti;
	};

	template<typename T>
	auto pointer_sentinel(cweeSharedPtr<T>& ptr) const noexcept {
		struct Sentinel {
			Sentinel(cweeSharedPtr<T>& t_ptr, Data& data)
				: m_ptr(t_ptr)
				, m_data(data) {
			}

			~Sentinel() {
				// save the pointer data
				const auto ptr_ = m_ptr.get().get();
				m_data.get().m_data_ptr = ptr_;
				m_data.get().m_const_data_ptr = ptr_;
			}

			//Sentinel& operator=(Sentinel&& s) = default;
			//Sentinel(Sentinel&& s) = default;

			operator cweeSharedPtr<T>& () const noexcept { return m_ptr.get(); }

			Sentinel& operator=(const Sentinel&) = delete;
			Sentinel(Sentinel&) = delete;

			std::reference_wrapper<cweeSharedPtr<T>> m_ptr;
			std::reference_wrapper<Data> m_data;
		};

		return Sentinel(ptr, *(m_data.get()));
	};

	bool is_null() const noexcept {
		AUTO m = m_data;
		if (!m) return true;
		return (m->m_data_ptr == nullptr);
	};

	const BoxedAny& get() const noexcept {
		AUTO m = m_data;
		if (!m) {
			m = Object_Data::get();
		}
		return m->m_obj;
	};

	bool is_ref() const noexcept {
		AUTO m = m_data;
		if (m)
			return m->m_is_ref;
		else
			return false;
	};

	bool is_pointer() const noexcept { return !is_ref(); };

	void* get_ptr(bool get_shared_ptr = false) const noexcept {
		AUTO m = m_data;
		if (m) {
			if (get_shared_ptr) {
				return m->m_ptr_ptr;
			}
			else {
				return m->m_data_ptr;
			}
		}
		else
			return nullptr;
	};

	const void* get_const_ptr(bool get_shared_ptr = false) const noexcept {
		return get_ptr(std::move(get_shared_ptr));
	};

	/// \returns true if the two cweeBoxedValues share the same internal type
	static bool type_match(const cweeBoxedValue& l, const cweeBoxedValue& r) noexcept { return l.get_type_info() == r.get_type_info(); };

protected:
	// necessary to avoid hitting the templated && constructor of cweeBoxedValue
	struct Internal_Construction {
	};

	cweeBoxedValue(cweeSharedPtr<Data> t_data, Internal_Construction) : m_data(std::move(t_data)) { };

	mutable cweeSharedPtr<Data> m_data;
};

#endif


class cweeAnyAutoCast;
/*!
Generic container that accepts any data and can be used to retrieve it again later (loses the strong type information). Requires casting on retreival.

cweeAny obj = 100.0;
double& out = (double&)obj;
double* x = obj;

void Foo(double x){}; void Foo(double& x){}; void Foo(double* x){};
Foo(obj);
*/
class cweeAny
{
public: // structors
	/*! Init */
	cweeAny() BOOST_NOEXCEPT : container() {}; // BOOST_CONSTEXPR  , mutex()

	/*! Copy */
	cweeAny(const cweeAny& rhs) : container() { // , mutex()
		Lock(); rhs.Lock();
		container = rhs.container;
		rhs.Unlock();  Unlock();
	};

	/*! Data Assignment */
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>>
	cweeAny(const ValueType& value) : container(value) {}; // , mutex()

	/*! Data Assignment */
	template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>>
	cweeAny(ValueType&& value) : container(std::forward<ValueType>(value)) {}; // , mutex()

	/*! Data Assignment */
	template<typename ValueType>
	cweeAny(const cweeSharedPtr<ValueType>& value) : container(cweeSharedPtr<ValueType>(value)) {}; // , mutex()

	/*! Data Assignment */
	template<typename ValueType>
	cweeAny(cweeSharedPtr<ValueType>&& value) : container(cweeSharedPtr<ValueType>(std::forward<cweeSharedPtr<ValueType>>(value))) {}; // , mutex()

	~cweeAny() BOOST_NOEXCEPT { };

public: // modifiers
	/*! Swap Data */
	cweeAny& swap(cweeAny& rhs) BOOST_NOEXCEPT {
		Lock(); rhs.Lock();
		container.swap(rhs.container);
		rhs.Unlock();  Unlock();

		return *this;
	};

	/*! Copy Data */
	cweeAny& operator=(const cweeAny& rhs) {
		cweeAny(rhs).swap(*this);
		return *this;
	};
	cweeAny& operator=(cweeAny&& rhs) {
		cweeAny(std::forward<cweeAny>(rhs)).swap(*this);
		return *this;
	};

	// Perfect forwarding of ValueType
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>>
	cweeAny& operator=(const ValueType& rhs) {
		Lock();
		cweeBoxedValue(rhs).swap(container);
		Unlock();

		return *this;
	};
	template <class ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>>
	cweeAny& operator=(ValueType&& rhs) {
		Lock();
		cweeBoxedValue(std::forward<ValueType>(rhs)).swap(container);
		Unlock();

		return *this;
	};

public: // queries
	cweeAnyAutoCast	cast() const;

	//template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> operator ValueType& () {
	//	return *cast<ValueType*>();
	//};
	//template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> operator ValueType& () const {
	//	return *cast<ValueType*>();
	//};
	//template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> operator ValueType* () {
	//	return cast<ValueType*>();
	//};
	//template<typename ValueType, typename = std::enable_if_t<!std::is_same_v<cweeAny, std::decay_t<ValueType>>>> operator ValueType* () const {
	//	return cast<ValueType*>();
	//};

	/*! Checks if the cweeAny has been assigned something */
	bool IsEmpty() const BOOST_NOEXCEPT {
		Lock();
		bool out(container.is_null());
		Unlock();

		return out;
	};

	/*! Empties the cweeAny and frees the memory. */
	void Clear() BOOST_NOEXCEPT {
		cweeAny().swap(*this);
	};

	template <typename ValueT> static const char* TypeNameOf() {
		return TypeOf<ValueT>().name();
	};
	template <typename ValueT> static const boost::typeindex::type_info& TypeOf() {
		return boost::typeindex::type_id<ValueT>().type_info();
	};

	const char* TypeName() const noexcept {
		return Type().name(); // may not be thread-safe?
	};
	const boost::typeindex::type_info& Type() const BOOST_NOEXCEPT {
		Lock();
		const boost::typeindex::type_info& out(container.get_type_info());
		Unlock();

		return out;
	};

public:
	template<typename VType>
	bool IsTypeOf() noexcept
	{
		decltype(auto) targetType = TypeOf<VType>();
		decltype(auto) thisType = Type();

		//cweeAnyTypeComparisonLock.Lock();
		bool out = (thisType == targetType);
		//cweeAnyTypeComparisonLock.Unlock();

		return out;
	};

	template<typename VType, typename = std::enable_if_t<!std::is_pointer<VType>::value>>
	typename  std::remove_reference<typename std::remove_pointer<VType>::type>::type& cast() noexcept
	{
		typename std::remove_reference<typename std::remove_pointer<VType>::type>::type* ptr = GetPtr<VType>();
		return std::ref(*ptr).get();
	};

	template<typename VType, typename = std::enable_if_t<std::is_pointer<VType>::value>>
	typename std::remove_reference<typename std::remove_pointer<VType>::type>::type* cast() noexcept
	{
		return GetPtr<VType>();
	};

	template<typename VType>
	decltype(auto) force_cast() noexcept
	{
		return this->cast<VType>();
	};

protected:
	cweeBoxedValue container;

private:
	void Lock() const {
		//mutex.Lock(); 
	};
	void Unlock() const {
		//mutex.Unlock(); 
	};
	//mutable cweeSysMutex   mutex;

	template<typename VType>
	decltype(auto) GetPtr() {
		Lock();
		decltype(auto) out(container.cast<typename std::remove_reference<typename std::remove_pointer<VType>::type>::type*>());
		Unlock();
		return out;
	};
};
class cweeAnyAutoCast {
public:
	constexpr cweeAnyAutoCast(const cweeAny* _parent) : parent(const_cast<cweeAny*>(_parent)) {};
	constexpr cweeAnyAutoCast(cweeAnyAutoCast&& other) : parent(other.parent) {};

	cweeAnyAutoCast() = delete;
	cweeAnyAutoCast(const cweeAnyAutoCast&) = delete;
	cweeAnyAutoCast& operator=(const cweeAnyAutoCast&) = delete;
	cweeAnyAutoCast& operator=(cweeAnyAutoCast&&) = delete;
	~cweeAnyAutoCast() {};

	template<typename ValueType, typename = std::enable_if<!std::is_same<cweeAny, std::decay<ValueType>>::value>> operator ValueType& () const {
		return *parent->cast<ValueType*>();
	};
	template<typename ValueType, typename = std::enable_if<!std::is_same<cweeAny, std::decay<ValueType>>::value>> operator ValueType* () const {
		return parent->cast<ValueType*>();
	};
	operator cweeAny* () const {
		return parent;
	};
	operator cweeAny& () const {
		return *parent;
	};
	cweeAny* parent;
};
INLINE cweeAnyAutoCast cweeAny::cast() const {
	cweeAnyAutoCast x(this);
	return x;
};

#else
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
	const boost::typeindex::type_info&	m_type;
	cweeSharedPtr<void>					m_ptr;

};
template<typename T> class cweeAnyData_Impl final : public cweeAnyData {
public:
	cweeAnyData_Impl() noexcept : cweeAnyData(nullptr, boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
	cweeAnyData_Impl(cweeSharedPtr<T> const& d) noexcept : cweeAnyData(cweeAnyData_Impl<T>::get_data(d), boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
	cweeAnyData_Impl(cweeSharedPtr<T>&& d) noexcept : cweeAnyData(cweeAnyData_Impl<T>::get_data(std::forward<cweeSharedPtr<T>>(d)), boost::typeindex::type_id<T>().type_info(), std::is_const_v<T>) {};
	~cweeAnyData_Impl() noexcept {};

	static cweeSharedPtr<void> get_data(const cweeSharedPtr<T>& data) {
		return cweeSharedPtr<void>(data, [](void* p) {
			return p;
		});
	};
	static cweeSharedPtr<void> get_data(cweeSharedPtr<T>&& data) {
		return cweeSharedPtr<void>(std::forward<cweeSharedPtr<T>>(data), [](void* p) {
			return p;
		});
	};
};
class cweeAnyAutoCast;

//#ifndef cweeSharedPtrWithSentinel 
//#define cweeSharedPtrWithSentinel cweeSharedPtr
//#endif

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

	template <typename T> operator cweeSharedPtr<T>() const noexcept{
		return parentCopy.cast<cweeSharedPtr<T>>();
	};
	template <typename T> operator cweeSharedPtr<T>*() const noexcept {
		return parentCopy.cast<cweeSharedPtr<T>*>();
	};

	template< bool cond, typename U > using resolvedType = typename std::enable_if< cond, U >::type;
	template< typename ValueTypeT, typename U = ValueTypeT&, typename = std::enable_if<!cweeAny::DataCaster::is_cweeSharedPtr_class<ValueTypeT>::type::value> >
	operator ValueTypeT& () const noexcept {
		return *parentCopy.cast<ValueTypeT*>();
	};
	template< typename ValueTypeT, typename U = ValueTypeT*, typename = std::enable_if<!cweeAny::DataCaster::is_cweeSharedPtr_class<ValueTypeT>::type::value> >
	operator ValueTypeT*() const noexcept {
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
#endif









#define HashType(_type_) cweeStr::hash(cweeAny::TypeNameOf<_type_>())
#define HashStr(_str_) cweeStr::hash(_str_)

class cweeJob;
template<typename T> struct count_arg;
template<typename R, typename ...Args> struct count_arg<std::function<R(Args...)>> { static constexpr const size_t value = sizeof...(Args); };
template <typename... Args> constexpr size_t sizeOfParameterPack(Args... Fargs) { return sizeof...(Args); }
template<class R> struct function_traits { using result_type = R; using arguments = std::tuple<>; };
template<class R> struct function_traits<std::function<R(void)>> { using result_type = R; using arguments = std::tuple<>; };
template<class R, class... Args> struct function_traits<std::function<R(Args...)>> { using result_type = R; using arguments = std::tuple<Args...>; };

template <typename F = void()> class cweeFunction {
public:
	typedef typename F Type;
	typedef typename std::function<F>::result_type ResultType;
	typedef typename function_traits<std::function<F>>::arguments Arguments;

	cweeFunction() noexcept
		: _function()
		, _data()
		, Result()
		, IsFinished(false)
		//, InvokeLock(0)
	{};

	template <typename... Args>
	cweeFunction(const std::function<F>& function, Args... Fargs) noexcept
		: _function(function)
		, Result()
		, IsFinished(false)
		, _data(GetData(Fargs...))
		//, InvokeLock(0)
	{};

private:
	static void AddData(std::vector<cweeAny>& d) { return; };
	template<typename T, typename... Targs> static void AddData(std::vector<cweeAny>& d, const T& value, Targs... Fargs) // recursive function
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
	template <typename... Args> std::vector<cweeAny> GetData(Args... Fargs) {
		constexpr size_t NumNeededInputs = NumInputs();
		constexpr size_t NumProvidedInputs = sizeof...(Args);
		static_assert(NumNeededInputs <= NumProvidedInputs, "Providing fewer inputs than required is unsupported. C++ Lambdas cannot support default arguments and therefore all arguments must be provided for.");

		std::vector<cweeAny> out;
		AddData(out, Fargs...);
		return std::move(out);
	};

public:
	cweeFunction(const cweeFunction& copy) noexcept
		: _function(copy._function)
		, _data(copy._data)
		, Result(copy.Result)
		, IsFinished(copy.IsFinished.load())
		//, InvokeLock(0)
	{};

	static cweeFunction Finished() {
		cweeFunction to_return;

		to_return.IsFinished.store(true);

		return to_return;
	};
	template <typename T> static cweeFunction Finished(const T& returnMe) {
		cweeFunction to_return;

		to_return.Result = returnMe;
		to_return.IsFinished.store(true);

		return to_return;
	};

	cweeAny& Invoke(int iterationNumber = 0) {
		DoJob(iterationNumber);
		return Result;
	};
	cweeAny& ForceInvoke(int iterationNumber = 0) {
		ForceDoJob(iterationNumber);
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

	cweeAny& GetResult() {
		return Result;
	};
	cweeAny& GetResult() const {
		return Result;
	};

private:
	void						DoJob(int iterationNumber = 0) {
		static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a cweeFunction without further specialization.");

		while (!InvokeTryLock()) {}

		if (!IsFinished.load()) {
			FixQueuedInputs();

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

			if (GetResult().IsTypeOf<cweeJob>()) {
				AUTO r = GetResult();
				cweeAnyAutoCast auto_r = r.cast();
				auto_r.operator cweeJob& ();

				cweeJob& j = GetResult().cast();
				if (iterationNumber < 20) {
					Result = j.Invoke(iterationNumber + 1);
				}
				else {
					Result.Clear();
					submitToast("C++ cweeJob Looping", FunctionName());
					// throw(std::exception(cweeStr("cweeJob Loop within ") + FunctionName()));
				}
				SetAsFinished();
			}
			else {
				SetAsFinished();
			}
		}
		InvokeUnlock();
	};
	void						ForceDoJob(int iterationNumber = 0) {
		static_assert(NumInputs() <= 16, "Cannot have more than 16 inputs for a cweeFunction without further specialization.");

		while (!InvokeTryLock()) {}

		if (true) {
			FixQueuedInputs();

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

			if (GetResult().IsTypeOf<cweeJob>()) {
				AUTO r = GetResult();
				cweeAnyAutoCast auto_r = r.cast();
				auto_r.operator cweeJob& ();

				cweeJob& j = GetResult().cast();
				if (iterationNumber < 20) {
					Result = j.Invoke(iterationNumber + 1);
				}
				else {
					Result.Clear();
					submitToast("C++ cweeJob Looping", FunctionName());
					// throw(std::exception(cweeStr("cweeJob Loop within ") + FunctionName()));
				}
				SetAsFinished();
			}
			else {
				SetAsFinished();
			}
		}
		InvokeUnlock();
	};

	void						FixQueuedInputs() {
		bool failed = true;
		while (failed) {
			failed = false;
			if constexpr (NumInputs() >= 1) {
				constexpr int i = 0;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 2) {
				constexpr int i = 1;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 3) {
				constexpr int i = 2;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 4) {
				constexpr int i = 3;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 5) {
				constexpr int i = 4;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 6) {
				constexpr int i = 5;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 7) {
				constexpr int i = 6;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 8) {
				constexpr int i = 7;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 9) {
				constexpr int i = 8;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 10) {
				constexpr int i = 9;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 11) {
				constexpr int i = 10;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 12) {
				constexpr int i = 11;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 13) {
				constexpr int i = 12;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 14) {
				constexpr int i = 13;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 15) {
				constexpr int i = 14;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}
			if constexpr (NumInputs() >= 16) {
				constexpr int i = 15;
				if (_data[i].IsTypeOf<cweeJob>() && !_data[i].IsTypeOf< typename std::decay_t<typename ::std::tuple_element<i, Arguments>::type> >()) {
					_data[i] = _data[i].cast<cweeJob&>().Await();
					failed = true;
				}
			}

			//for (int i = _data.size() - 1; i >= 0; i--) {
			//	if (_data[i].IsTypeOf<cweeJob>()) {
			//		_data[i] = _data[i].cast<cweeJob&>().Await(); // GetResult();
			//		failed = true;
			//	}
			//}
		}
	};
	void						DoJob_Internal_16() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast(), _data[15].cast());
		}
	};
	void						DoJob_Internal_15() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast(), _data[14].cast());
		}
	};
	void						DoJob_Internal_14() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast(), _data[13].cast());
		}
	};
	void						DoJob_Internal_13() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast(), _data[12].cast());
		}
	};
	void						DoJob_Internal_12() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast(), _data[11].cast());
		}
	};
	void						DoJob_Internal_11() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast(), _data[10].cast());
		}
	};
	void						DoJob_Internal_10() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast(), _data[9].cast());
		}
	};
	void						DoJob_Internal_9() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast(), _data[8].cast());
		}
	};
	void						DoJob_Internal_8() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast(), _data[7].cast());
		}
	};
	void						DoJob_Internal_7() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast(), _data[6].cast());
		}
	};
	void						DoJob_Internal_6() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast(), _data[5].cast());
		}
	};
	void						DoJob_Internal_5() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast(), _data[4].cast());
		}
	};
	void						DoJob_Internal_4() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast(), _data[3].cast());
		}
	};
	void						DoJob_Internal_3() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast(), _data[2].cast());
		}
	};
	void						DoJob_Internal_2() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast(), _data[1].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast(), _data[1].cast());
		}
	};
	void						DoJob_Internal_1() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function(_data[0].cast());
			cweeAny().swap(Result);
		}
		else {
			Result = _function(_data[0].cast());
		}
	};
	void						DoJob_Internal_0() {
		if constexpr (ReturnsNothing())
		{
			/*    */ _function();
			cweeAny().swap(Result);
		}
		else {
			Result = _function();
		}
	};

private:
	std::function<F>			_function;
	std::vector<cweeAny>		_data;
	void						SetAsFinished() {
		IsFinished.store(true);
	};
	// mutable cweeSysInterlockedInteger	InvokeLock;

	bool						InvokeTryLock() const {
		//if (InvokeLock.Increment() == 1) {
			return true;
		//}
		//else {
		//	InvokeLock.Decrement();
		//	return false;
		//}
	};
	void						InvokeUnlock() const {
		//InvokeLock.Decrement();
	};

public:
	mutable cweeAny				Result;
	mutable std::atomic<bool>	IsFinished;

};

class cweeAction_Interface {
public:
	explicit cweeAction_Interface() {};
	explicit cweeAction_Interface(cweeAction_Interface const&) = delete;
	explicit cweeAction_Interface(cweeAction_Interface&&) = delete;
	cweeAction_Interface& operator=(cweeAction_Interface const&) = delete;
	cweeAction_Interface& operator=(cweeAction_Interface&&) = delete;
	virtual ~cweeAction_Interface() noexcept {};

	virtual boost::typeindex::type_info const& type() const noexcept = 0;
	virtual const char* typeName() const noexcept = 0;
	virtual cweeSharedPtr<cweeAction_Interface> clone() const noexcept = 0;
	virtual cweeAny& Invoke(int iterationNumber = 0) noexcept = 0;
	virtual cweeAny& ForceInvoke(int iterationNumber = 0) noexcept = 0;
	virtual const char* FunctionName() const noexcept = 0;
	virtual cweeAny& Result() const noexcept = 0;
	virtual bool IsFinished() const noexcept = 0;
};
template<typename ValueType> class cweeAction_Impl final : public cweeAction_Interface {
public:
	explicit cweeAction_Impl() = delete;
	explicit cweeAction_Impl(cweeFunction<ValueType> const& f) noexcept : data(f) {};
	explicit cweeAction_Impl(cweeFunction<ValueType>&& f) noexcept : data(std::forward<cweeFunction<ValueType>>(f)) {};
	virtual ~cweeAction_Impl() noexcept {};

	virtual boost::typeindex::type_info const& type() const noexcept final {
		return boost::typeindex::type_id<cweeFunction<ValueType>>().type_info();
	};
	virtual const char* typeName() const noexcept final {
		return boost::typeindex::type_id<cweeFunction<ValueType>>().type_info().name();
	};
	virtual cweeSharedPtr<cweeAction_Interface> clone() const noexcept final {
		return make_cwee_shared<cweeAction_Impl<ValueType>>(data).CastReference<cweeAction_Interface>();
	};
	virtual cweeAny& Invoke(int iterationNumber = 0) noexcept final {
		return data.Invoke(iterationNumber);
	};
	virtual cweeAny& ForceInvoke(int iterationNumber = 0) noexcept final {
		return data.ForceInvoke(iterationNumber);
	};
	virtual const char* FunctionName() const noexcept final {
		return data.FunctionName();
	};
	virtual cweeAny& Result() const noexcept final {
		return data.GetResult();
	};
	virtual bool IsFinished() const noexcept final {
		return data.IsFinished.load();
	};

	cweeFunction<ValueType> data;
};

class cweeAction {
public: // structors
	/*! Init */ cweeAction() noexcept : content(BasePtr()) {};
	/*! Copy */ cweeAction(const cweeAction& other) noexcept : content(BasePtr()) { cweeSharedPtr<cweeAction_Interface> c = other.content; content = c->clone(); };
	/*! Data Assignment */ template<typename ValueType> explicit cweeAction(const cweeFunction<ValueType>& value) : content(ToPtr<ValueType>(value)) {};
	~cweeAction() noexcept {};

public: // modifiers
	/*! Swap Data */ cweeAction& swap(cweeAction& rhs) noexcept {
		cweeSharedPtr<cweeAction_Interface> c1 = this->content;
		cweeSharedPtr<cweeAction_Interface> c2 = rhs.content;

		auto copy1 = c1->clone();
		auto copy2 = c2->clone();

		rhs.content = copy1;
		content = copy2;

		return *this;
	}
	/*! Copy Data */ cweeAction& operator=(const cweeAction& rhs) noexcept { cweeAction(rhs).swap(*this); return *this; };
	/* Perfect forwarding of ValueType */ template <class ValueType> cweeAction& operator=(const cweeFunction<ValueType>& rhs) noexcept { cweeAction(rhs).swap(*this); return *this; };

public: // queries
	explicit operator bool() { return !IsEmpty(); };
	explicit operator bool() const { return !IsEmpty(); };

	/*! Checks if the cweeAction has been assigned something */
	bool IsEmpty() const noexcept { AUTO c = content; return !c; };

	/*! Empties the cweeAction and frees the memory. */
	void Clear() noexcept { cweeAction().swap(*this); };

	template <typename ValueT> static constexpr const char* TypeNameOf() { return typenames::type_name<ValueT>(); };
	template <typename ValueT> static const boost::typeindex::type_info& TypeOf() {	return boost::typeindex::type_id<ValueT>().type_info(); };

	const char* TypeName() const noexcept { cweeSharedPtr<cweeAction_Interface> c = content; if (c) return c->typeName(); return typenames::type_name<void>(); };
	const boost::typeindex::type_info& Type() const noexcept { cweeSharedPtr<cweeAction_Interface> c = content; return c ? c->type() : boost::typeindex::type_id<void>().type_info(); };

private:
	template <class ValueType> static cweeSharedPtr<cweeAction_Interface> ToPtr(const cweeFunction<ValueType>& rhs) noexcept { return make_cwee_shared<cweeAction_Impl<ValueType>>(rhs).CastReference<cweeAction_Interface>(); };
	template <class ValueType> static cweeSharedPtr<cweeAction_Interface> ToPtr(cweeFunction<ValueType>&& rhs) noexcept { return make_cwee_shared<cweeAction_Impl<ValueType>>(std::forward<cweeFunction<ValueType>>(rhs)).CastReference<cweeAction_Interface>(); };
	static cweeSharedPtr<cweeAction_Interface> BasePtr() noexcept { return ToPtr(cweeFunction<void()>()); };

public: 
	cweeSharedPtr<cweeAction_Interface> content;

public:
	cweeAny* Invoke(int iterationNumber = 0) noexcept { cweeSharedPtr<cweeAction_Interface> c = content; if (c) { return &c->Invoke(iterationNumber); } return nullptr; };
	cweeAny* ForceInvoke(int iterationNumber = 0) noexcept { cweeSharedPtr<cweeAction_Interface> c = content; if (c) { return &c->ForceInvoke(iterationNumber); } return nullptr; };
	const char* FunctionName() const {
		cweeSharedPtr<cweeAction_Interface> c = content;
		if (c)
		{
			return c->FunctionName();
		}
		return "No Function";
	};
	bool     IsFinished() const {
		cweeSharedPtr<cweeAction_Interface> c = content;
		if (c)
		{
			try {
				return c->IsFinished();
			}
			catch (...) {
				cweeStackTrace::GetTrace(true);
				return true;
			}
		}
		return false;
	};
	cweeAny* Result() const { cweeSharedPtr<cweeAction_Interface> c = content; if (c) { return &c->Result(); } return nullptr;	};
	static cweeAction Finished() { return cweeAction(cweeFunction<void()>::Finished());	};
	template <typename T> static cweeAction Finished(const T& returnMe) { return cweeAction(cweeFunction<T()>::Finished(returnMe)); };

};

class cweeBaseObj {
public:
	cweeBaseObj() : parent(), level() { };
	virtual				~cweeBaseObj() = default;
	virtual cweeStr		Type() const { return typeid(*this).name(); };
	virtual cweeStr		ToString() const { return ""; };
	virtual void		Deserialize(cweeStr& inbound) { Clear(); inbound.Clear(); };
	virtual cweeStr		Serialize() const { return ""; };
	virtual void		Update() { };
	virtual void		IncrementLevel() { level.Increment(); };
	virtual void		DecrementLevel() { level.Decrement(); };
	virtual int			GetLevel() const { return level.GetValue(); };
	virtual void		SetLevel(int v) { level.SetValue(v); };
	virtual void		Clear() {  };
	// copy operator
	cweeBaseObj& operator=(cweeBaseObj& obj) {
		Clear();
		return *this;
	};

	cweeSysInterlockedInteger	level;
	cweeAny						parent;
};

#endif