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

#pragma once
#include "Precompiled.h"
#include "InterlockedValues.h"

#define allowCweeSharedPtrCaptureByValue
class Details_Interface {
protected:
	Details_Interface() noexcept {};
	explicit Details_Interface(Details_Interface const&) = delete;
	explicit Details_Interface(Details_Interface&&) = delete;
	Details_Interface& operator=(Details_Interface const&) = delete;
	Details_Interface& operator=(Details_Interface&&) = delete;

public:
	virtual ~Details_Interface() noexcept {};
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept = 0;

};
/* Interface for simply allowing the sharing of a virtual pointer */
class cweeSharedPtr_Data_Interface {
protected:
	cweeSharedPtr_Data_Interface() noexcept {};
	virtual ~cweeSharedPtr_Data_Interface() noexcept {};
	explicit cweeSharedPtr_Data_Interface(cweeSharedPtr_Data_Interface const& other) noexcept = delete;
	explicit cweeSharedPtr_Data_Interface(cweeSharedPtr_Data_Interface&& other) noexcept = delete;
	cweeSharedPtr_Data_Interface& operator=(cweeSharedPtr_Data_Interface const& other) = delete;
	cweeSharedPtr_Data_Interface& operator=(cweeSharedPtr_Data_Interface&& other) = delete;
public:
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual cweeSysInterlockedPointer<Details_Interface>& ptr() const noexcept = 0;
};
template <typename type> class cweeSharedPtr {
#pragma region Class Defs
public:
	using swappablePtr = cweeSysInterlockedPointer<type>;
#ifdef allowCweeSharedPtrCaptureByValue
	class details_withData final : public Details_Interface {
	public:
		~details_withData() noexcept {};
		explicit details_withData(details_withData const&) = delete;
		explicit details_withData(details_withData&&) = delete;
		details_withData& operator=(details_withData const&) = delete;
		details_withData& operator=(details_withData&&) = delete;

		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> > INLINE explicit details_withData() noexcept : count(1), d() {};
		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> > INLINE explicit details_withData(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>&& obj) noexcept : count(1), d(std::forward<type>(obj)) {};
		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> > INLINE explicit details_withData(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>const& obj) noexcept : count(1), d(obj) {};

		INLINE void* source() const noexcept final { return const_cast<void*>(static_cast<const void*>(&d)); };
		INLINE int	use_count() const noexcept final { return count.GetValue(); };
		INLINE void increment() const noexcept final { count.Increment(); };
		INLINE int decrement() const noexcept final { return count.Decrement(); };
		INLINE std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept final {
			return [](Details_Interface* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtr<type>::details_withData*>(p);
					if (q) {
						delete q;
					}
					else {
						delete p; // likely a  leak
					}
				}
			};
		};

	private:
		mutable cweeSysInterlockedInteger count;
		type d;
	};
#endif
	class details final : public Details_Interface {
	public:
		explicit details() = delete;
		~details() noexcept {};
		explicit details(details const&) = delete;
		explicit details(details&&) = delete;
		details& operator=(details const&) = delete;
		details& operator=(details&&) = delete;
		INLINE explicit details(type* _source, std::function<void(type*)> _destroy) noexcept :
			p(_source), count(1), deleter(std::move(_destroy)) {};
		INLINE void* source() const noexcept final { return static_cast<void*>(p.Get()); };
		INLINE int	use_count() const noexcept final {
			return count.GetValue();
		};
		INLINE void increment() const noexcept final {
			count.Increment();
		};
		INLINE int decrement() const noexcept final {
			int i = count.Decrement();
			if (i == 0) {
				deleter(p.Set(nullptr));
			}
			return i;
		};
		INLINE std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept final {
			return [](Details_Interface* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtr<type>::details*>(p);
					if (q) {
						delete q;
					}
					else {
						delete p; // likely a  leak
					}
				}
			};
		};
	private:
		mutable swappablePtr p;
		mutable cweeSysInterlockedInteger count;
		mutable std::function<void(type*)> deleter;
	};
	class details_no_destructor final : public Details_Interface {
	public:
		explicit details_no_destructor() = delete;
		~details_no_destructor() noexcept {};
		explicit details_no_destructor(details_no_destructor const&) = delete;
		explicit details_no_destructor(details_no_destructor&&) = delete;
		details_no_destructor& operator=(details_no_destructor const&) = delete;
		details_no_destructor& operator=(details_no_destructor&&) = delete;
		INLINE explicit details_no_destructor(type* _source) noexcept :
			p(_source), count(1) {};
		INLINE void* source() const noexcept final {
			return static_cast<void*>(p.Get());
		};
		INLINE int	use_count() const noexcept final {
			return count.GetValue();
		};
		INLINE void increment() const noexcept final {
			count.Increment();
		};
		INLINE int decrement() const noexcept final {
			int i = count.Decrement();
			if (i == 0) {
				AUTO P = p.Set(nullptr);
				if (P) {
					delete P;
				}
			}
			return i;
		};
		INLINE std::function<void(Details_Interface*)> Details_Interface_Deleter() const noexcept final {
			return [](Details_Interface* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtr<type>::details_no_destructor*>(p);
					if (q) {
						delete q;
					}
					else {
						delete p; // likely a  leak
					}
				}
			};
		};
	private:
		mutable swappablePtr p;
		mutable cweeSysInterlockedInteger count;
	};
	class cweeSharedPtr_DataImpl final : public cweeSharedPtr_Data_Interface {
	public:
		explicit cweeSharedPtr_DataImpl() = delete;
		~cweeSharedPtr_DataImpl() noexcept { 
			decrement(); 
			m_ptr = nullptr; 
		};
		explicit cweeSharedPtr_DataImpl(cweeSharedPtr_DataImpl const&) = delete;
		explicit cweeSharedPtr_DataImpl(cweeSharedPtr_DataImpl&&) = delete;
		cweeSharedPtr_DataImpl& operator=(cweeSharedPtr_DataImpl const&) = delete;
		cweeSharedPtr_DataImpl& operator=(cweeSharedPtr_DataImpl&&) = delete;

		INLINE explicit cweeSharedPtr_DataImpl(const cweeSharedPtr_Data_Interface& other, type* _ptr) noexcept : det(other.ptr()), m_ptr(_ptr) {
			increment();
		};

		INLINE explicit cweeSharedPtr_DataImpl(const cweeSharedPtr_Data_Interface& other, std::function<type* (void*)> _getter) noexcept : det(other.ptr()) {
			m_ptr = _getter(other.source());
			increment(); 
		};

		INLINE explicit cweeSharedPtr_DataImpl(type* _source, std::function<void(type*)> _on_destroy, std::function<type* (void*)> _getter) noexcept :
			det(new details(_source, std::move(_on_destroy))) 
		{
			m_ptr = _source;
		};
		INLINE explicit cweeSharedPtr_DataImpl(type* _source, std::function<type* (void*)> _getter) noexcept : det(new details_no_destructor(_source)) {
			m_ptr = _source;
		};

#ifdef allowCweeSharedPtrCaptureByValue
		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
		INLINE explicit cweeSharedPtr_DataImpl(std::function<type* (void*)> _getter) noexcept : det(new details_withData()) {
			m_ptr = _getter(source());
		};

		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
		INLINE explicit cweeSharedPtr_DataImpl(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>&& _obj, std::function<type* (void*)> _getter) noexcept : det(new details_withData(std::forward<type>(_obj))) {
			m_ptr = _getter(source());
		};

		template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
		INLINE explicit cweeSharedPtr_DataImpl(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>const& _obj, std::function<type* (void*)> _getter) noexcept : det(new details_withData(_obj)) {
			m_ptr = _getter(source());
		};
#endif
		INLINE void* source() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				return p->source();
			}
			return nullptr;
		};
		INLINE int	use_count() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				return p->use_count();
			}
			return 0;
		};
		INLINE void increment() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				p->increment();
			}
		};
		INLINE int decrement() const noexcept final {
			int i = -1;
			AUTO p = det.Get();
			if (p) {
				i = p->decrement();
				if (i == 0) {
					{
						AUTO p2 = det.Set(nullptr);
						m_ptr = nullptr;
						if (p2 != nullptr) {
							AUTO func = p2->Details_Interface_Deleter();
							func(p2);
						}
						else {
							delete p; // likely a leak
						}
					}
				}
			}
			return i;
		};
		INLINE cweeSysInterlockedPointer<Details_Interface>& ptr() const noexcept final { return det; };

	public:
		mutable cweeSysInterlockedPointer<Details_Interface>	det;
		mutable type*											m_ptr;
	};
#pragma endregion 
#pragma region Type Defs
public:
	typedef type						Type;
	typedef type* PtrType;
#pragma endregion 
#pragma region Data Members
public:
	mutable cweeSysInterlockedInteger									mutex;
	mutable cweeSysInterlockedPointer<cweeSharedPtr_DataImpl>			m_data;
#pragma endregion 
#pragma region Create or Destroy
public:
	/*! Create an empty pointer (nullptr) */
	constexpr cweeSharedPtr() noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a nullptr */
	constexpr cweeSharedPtr(std::nullptr_t) noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	INLINE cweeSharedPtr(PtrType source) noexcept : mutex(0), m_data(InitData(source)) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	INLINE cweeSharedPtr(PtrType source, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(source, std::move(destroy))) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	INLINE cweeSharedPtr(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitData(source, std::move(destroy), std::move(_getter))) {};

	/*! instantiate with a constructor and destructor */
	INLINE cweeSharedPtr(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(std::move(create), std::move(destroy))) {};

	/*! instantiate from another ptr */
	cweeSharedPtr(cweeSharedPtr const& samePtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr)) {};
	cweeSharedPtr(cweeSharedPtr&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr>(other))) {};

	/*! instantiate from another ptr with complex "get" instructions */
	INLINE cweeSharedPtr(cweeSharedPtr const& samePtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr, std::move(_getter))) {};
	INLINE cweeSharedPtr(cweeSharedPtr&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr>(other), std::move(_getter))) {};

	/*! instantiate from another ptr with a different Type, using basic cast operations */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	INLINE cweeSharedPtr(cweeSharedPtr<T> const& similarPtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, [](void* p) constexpr -> PtrType {
		if constexpr (std::is_polymorphic<Type>::value && std::is_polymorphic<T>::value && (std::is_base_of<T, Type>::value || std::is_base_of<Type, T>::value)) {
			return dynamic_cast<PtrType>((T*)p);
		}
		else {
			return static_cast<PtrType>((T*)p);
		}
		// return static_cast<PtrType>((T*)p); // dynamic_cast
	})) {};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	INLINE cweeSharedPtr(cweeSharedPtr<T>&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr<T>>(other), [](void* p) constexpr -> PtrType {
		if constexpr (std::is_polymorphic<Type>::value && std::is_polymorphic<T>::value && (std::is_base_of<T, Type>::value || std::is_base_of<Type, T>::value)) {
			return dynamic_cast<PtrType>((T*)p);
		}
		else {
			return static_cast<PtrType>((T*)p);
		}
		// return static_cast<PtrType>((T*)p); // dynamic_cast
	})) {};

	/*! instantiate from another ptr with a different Type with complex "get" instructions */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	INLINE cweeSharedPtr(cweeSharedPtr<T> const& similarPtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, std::move(_getter))) {};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	INLINE cweeSharedPtr(cweeSharedPtr<T>&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtr<T>>(other), std::move(_getter))) {};

	template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
	INLINE cweeSharedPtr(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type> const& source) noexcept : mutex(0), m_data(
#ifdef allowCweeSharedPtrCaptureByValue
		new cweeSharedPtr_DataImpl(source, std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; }))
#else
		InitData(new Type(source))
#endif
	) {};

	template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
	INLINE cweeSharedPtr(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>&& source) noexcept : mutex(0), m_data(
#ifdef allowCweeSharedPtrCaptureByValue
		new cweeSharedPtr_DataImpl(std::forward<std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>>(source), std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; }))
#else
		InitData(new Type(std::forward<Type>(source)))
#endif
	) {};

#ifdef allowCweeSharedPtrCaptureByValue
	template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
	INLINE static cweeSharedPtr<Q> InstantiateInline() {
		AUTO toReturn = cweeSharedPtr<Q>();
		toReturn.UnsafeSetData(
			new cweeSharedPtr_DataImpl(
				std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })
			)
		);
		return toReturn;
	};
#endif

	/*! Destroy this instance of the shared pointer and potentially delete the data */
	~cweeSharedPtr() noexcept {
		ClearData();
	};
#pragma endregion
#pragma region Internal Support Functions
public:
	INLINE PtrType Get() const {
		PtrType out { nullptr };
		Lock();
		if (m_data) {
			out = m_data->m_ptr;
		}
		Unlock();
		return out;
	};

public:
	INLINE void Lock() const noexcept { while (mutex.Increment() != 1) mutex.Decrement(); };
	INLINE void Unlock() const noexcept { mutex.Decrement(); };
	PtrType UnsafeGet() const noexcept {
		if (m_data) {
			return m_data->m_ptr;
		}
		return nullptr;
	};
	template <typename... Args> void UnsafeSet(Args... Fargs) { UnsafeSetData(cweeSharedPtr<type>::InitData(Fargs...)); };
protected:
	template <typename T> static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T> const& Ptr) noexcept {
		cweeSharedPtr<type>::cweeSharedPtr_DataImpl* out = nullptr;
		Ptr.Lock();
		typename cweeSharedPtr<T>::cweeSharedPtr_DataImpl* p = Ptr.m_data.Get();
		if (p) {
			out = new cweeSharedPtr<type>::cweeSharedPtr_DataImpl(*p, p->m_ptr);
		}
		Ptr.Unlock();
		return out;
	};
	template <typename T> INLINE static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T> const& Ptr, std::function<PtrType(void*)> from) noexcept {
		cweeSharedPtr<type>::cweeSharedPtr_DataImpl* out = nullptr;
		Ptr.Lock();
		typename cweeSharedPtr<T>::cweeSharedPtr_DataImpl* p = Ptr.m_data.Get();
		if (p) {
			out = new cweeSharedPtr<type>::cweeSharedPtr_DataImpl(*p, std::move(from));
		}
		Ptr.Unlock();
		return out;
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > INLINE static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T>&& Ptr) noexcept {
		return InitDataFromAnotherPtr(std::forward<cweeSharedPtr<T>>(Ptr), [](void* p) constexpr -> PtrType { return (PtrType)p; });
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > INLINE static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr<T>&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtr<T>::cweeSharedPtr_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtr<type>::cweeSharedPtr_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	
	
	static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr&& Ptr) noexcept {
		//cweeSharedPtr_DataImpl* p;
		// Ptr.Lock(); // lock is unecessary since no other resource should be able to access this pointer
		return /* p = */ Ptr.m_data.Set(nullptr);
		// Ptr.Unlock();
		// return p;
	};
	static cweeSharedPtr_DataImpl* InitDataFromAnotherPtr(cweeSharedPtr&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtr<type>::cweeSharedPtr_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};

	INLINE static AUTO InitData(PtrType source, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtr_DataImpl(source, std::move(from));
	};
	INLINE static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtr_DataImpl(source, std::move(destroy), std::move(from));
	};
	INLINE static AUTO InitData(PtrType source) noexcept { return InitData(source, std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	INLINE static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy) noexcept { return InitData(source, std::move(destroy), std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	INLINE static AUTO InitData(std::function<PtrType()> create) noexcept { return InitData(create()); };
	INLINE static AUTO InitData(std::function<PtrType()> create, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(from)); };
	INLINE static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept { return InitData(create(), std::move(destroy)); };
	INLINE static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(destroy), std::move(from)); };

	INLINE void SetData(cweeSharedPtr_DataImpl* p) const noexcept {
		Lock();
		AUTO d = m_data.Set(p);
		Unlock();
		if (d) {
			delete d;
		}
	};
	INLINE void UnsafeSetData(cweeSharedPtr_DataImpl* p) const noexcept {
		AUTO d = m_data.Set(p);
		if (d) {
			delete d;
		}
	};
	INLINE void ClearData() const noexcept { SetData(nullptr); };

#pragma endregion 
#pragma region Boolean Operators
public:
	INLINE constexpr explicit operator bool() const { return m_data.Get(); };
	INLINE friend bool operator==(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() == b.Get(); };
	INLINE friend bool operator!=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() != b.Get(); };
	INLINE friend bool operator<(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() < b.Get(); };
	INLINE friend bool operator<=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() <= b.Get(); };
	INLINE friend bool operator>(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() > b.Get(); };
	INLINE friend bool operator>=(const cweeSharedPtr& a, const cweeSharedPtr& b) noexcept { return a.Get() >= b.Get(); };
	INLINE friend bool operator==(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() == nullptr; };
	INLINE friend bool operator!=(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() != nullptr; };
	INLINE friend bool operator<(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() < nullptr; };
	INLINE friend bool operator<=(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() <= nullptr; };
	INLINE friend bool operator>(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() > nullptr; };
	INLINE friend bool operator>=(const cweeSharedPtr& a, std::nullptr_t) noexcept { return a.m_data.Get() >= nullptr; };
	INLINE friend bool operator==(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr == a.m_data.Get(); };
	INLINE friend bool operator!=(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr != a.m_data.Get(); };
	INLINE friend bool operator<(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr < a.m_data.Get(); };
	INLINE friend bool operator<=(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr <= a.m_data.Get(); };
	INLINE friend bool operator>(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr > a.m_data.Get(); };
	INLINE friend bool operator>=(std::nullptr_t, const cweeSharedPtr& a) noexcept { return nullptr >= a.m_data.Get(); };
#pragma endregion
#pragma region Ptr Casting
public:
	/*! Create a shared pointer from this one with an added const component to the underlying managed object type */
	INLINE cweeSharedPtr<typename std::add_const<Type>::type> ConstReference() const noexcept {
		typedef typename std::add_const<Type>::type newT;
		cweeSharedPtr<newT> out;
		typedef typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl oldDI;
		typedef typename cweeSharedPtr<newT>::cweeSharedPtr_DataImpl newDI;
		typedef typename cweeSharedPtr<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return (newP)(p);
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
	/*! Create a shared pointer from this one with an removed const component to the underlying managed object type */
	INLINE cweeSharedPtr<typename std::remove_const<Type>::type> RemoveConstReference() const noexcept {
		typedef typename std::remove_const<Type>::type newT;
		cweeSharedPtr<newT> out;
		typedef typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl oldDI;
		typedef typename cweeSharedPtr<newT>::cweeSharedPtr_DataImpl newDI;
		typedef typename cweeSharedPtr<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return (newP)(p);
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
	/*! Create a shared pointer from this one that casts the underlying type to another underlying type (i.e. for derived types) */
	template<typename astype> INLINE cweeSharedPtr<typename std::remove_const<astype>::type> CastReference() const noexcept {
		using newT = typename std::remove_const<astype>::type;
		cweeSharedPtr<newT> out;
		using oldDI = typename cweeSharedPtr<type>::cweeSharedPtr_DataImpl;
		using newDI = typename cweeSharedPtr<newT>::cweeSharedPtr_DataImpl;
		using  newP = typename cweeSharedPtr<newT>::PtrType;

		cweeSharedPtr<type> copy = *this;
		std::function<newP(void*)> _getter = [copy](void* p) {
			return dynamic_cast<newP>(copy.Get());
		};

		newDI* d_p = nullptr;
		{
			this->Lock();
			oldDI* p = this->m_data.Get();
			if (p) {
				d_p = new newDI(*p, _getter);
			}
			this->Unlock();
		}
		out.m_data.Set(d_p);

		return out;
	};
#pragma endregion
#pragma region Data Access
public:
	/*! Pointer Access (never throws) (no race conditions) */
	INLINE PtrType operator->() const noexcept { return Get(); };

	template<typename Q = type> INLINE typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator[](std::ptrdiff_t idx) const
	{
		AUTO x = Get();
		if (x) return x[idx];
		throw(std::runtime_error(std::string("cweeSharedPtr was null with indexed access")));
	};

	template<typename Q = type> INLINE typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator*() const
	{
		AUTO x = Get();
		if (x) return *x;
		throw(std::runtime_error(std::string("cweeSharedPtr was null with reference access")));
	};

	template<typename Q = type> INLINE typename std::enable_if<std::is_same<Q, void>::value, void>::type operator[](std::ptrdiff_t idx) const { throw(std::exception("cweeSharedPtr was null with indexed access")); };

	template<typename Q = type> INLINE typename std::enable_if<std::is_same<Q, void>::value, void>::type operator*() const { throw(std::exception("cweeSharedPtr was null with reference access")); };

#pragma endregion
#pragma region Assignment Operators
public:
	/*! Share ownership */
	INLINE cweeSharedPtr& operator=(const cweeSharedPtr& other) noexcept {
		if (other == *this) { return *this; }

		AUTO p = InitDataFromAnotherPtr(other);
		SetData(p);

		return *this;
	};

	/*! Take ownership */
	INLINE cweeSharedPtr& operator=(cweeSharedPtr&& other) noexcept {
		if (other == *this) { return *this; }

		other.Lock();
		AUTO p = other.m_data.Set(nullptr); 
		other.Unlock();
		SetData(p);

		return *this;
	};
#pragma endregion
#pragma region Swap Operators
public:
	INLINE cweeSharedPtr& Swap(cweeSharedPtr& other) {
		cweeSharedPtr t = *this;
		*this = other;
		other = t;
		return *this;
	};
#pragma endregion
#pragma region "std::shared_ptr" Compatability Methods
public:
	/*! Get access to the underlying data in a thread-safe way. (The underlying data isn't necessarily thread-safe, but accessing the pointer itself has no data race.) */
	INLINE PtrType get() const noexcept { return Get(); };
	/*! Empty the current data pointer and free the data. */
	INLINE void reset() { cweeSharedPtr().Swap(*this); };
	/*! Exchange the current data with the provided new data. */
	template< class Y > INLINE void reset(Y* ptr) { cweeSharedPtr(ptr).Swap(*this); };
	/*! Swap shared pointers */
	INLINE void swap(cweeSharedPtr& r) { Swap(r); };
	/*! Number of shared pointers with access to the underlying data, including this one. */
	INLINE long use_count() const noexcept {
		long out(0);
		Lock();
		AUTO d = m_data.Get();
		if (d) {
			out = d->use_count();
		}
		Unlock();
		return out;
	};
#pragma endregion
#pragma region Guard
public:
	class cweeSharedPtrGuard {
	public:
		constexpr explicit operator bool() const { return m_p; };
		cweeSharedPtrGuard() = delete;
		cweeSharedPtrGuard(cweeSharedPtrGuard&&) = delete;
		cweeSharedPtrGuard(cweeSharedPtrGuard const&) = delete;
		cweeSharedPtrGuard& operator=(cweeSharedPtrGuard const&) = delete;
		cweeSharedPtrGuard& operator=(cweeSharedPtrGuard&&) = delete;
		explicit cweeSharedPtrGuard(const cweeSharedPtr* p_p) noexcept : m_p(p_p) { p_p->Lock(); }
		~cweeSharedPtrGuard() noexcept { m_p->Unlock(); };
	private:
		const cweeSharedPtr* m_p;
	};
	[[nodiscard]] cweeSharedPtrGuard Guard() const noexcept { return cweeSharedPtrGuard(this); };
#pragma endregion
};

template <typename type> INLINE cweeSharedPtr<type> make_cwee_shared() {
#ifdef allowCweeSharedPtrCaptureByValue
	if constexpr (std::is_copy_constructible<type>::value) {
		return cweeSharedPtr<type>::InstantiateInline();
	}
	else {
		return cweeSharedPtr<type>(new type());
	}
#else
	return cweeSharedPtr<type>(new type());
#endif
};
template <typename type> INLINE cweeSharedPtr<type> make_cwee_shared(type* p) { return cweeSharedPtr<type>(p); };
template<typename type, typename t1, typename = std::enable_if_t<!std::is_same_v<type*, t1>>> INLINE cweeSharedPtr<type> make_cwee_shared(t1&& d) { return cweeSharedPtr<type>(new type(std::forward<t1>(d))); };
template <typename type, typename t1, typename = std::enable_if_t<!std::is_same_v<type*, t1>>, class ...Args> INLINE cweeSharedPtr<type> make_cwee_shared(t1&& d, Args&&... Fargs) { return cweeSharedPtr<type>(new type(std::forward<t1>(d), std::forward<Args>(Fargs)...)); };
