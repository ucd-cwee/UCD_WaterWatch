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
#include "SharedPtr.h"

typedef LockedObject<std::vector<std::function<void()>>> cweeSharedPtrWithSentinelDestroyerListType;
class Details_InterfaceWithSentinel {
protected:
	Details_InterfaceWithSentinel() noexcept {};
	explicit Details_InterfaceWithSentinel(Details_InterfaceWithSentinel const&) = delete;
	explicit Details_InterfaceWithSentinel(Details_InterfaceWithSentinel&&) = delete;
	Details_InterfaceWithSentinel& operator=(Details_InterfaceWithSentinel const&) = delete;
	Details_InterfaceWithSentinel& operator=(Details_InterfaceWithSentinel&&) = delete;
public:
	virtual ~Details_InterfaceWithSentinel() noexcept {};
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual void add_on_destroy(const std::function<void()>& func) const noexcept = 0;
	virtual cweeSharedPtrWithSentinelDestroyerListType& destroyers() const noexcept = 0;
	virtual std::function<void(Details_InterfaceWithSentinel*)> Details_InterfaceWithSentinel_Deleter() const noexcept = 0;
};
/* Interface for simply allowing the sharing of a virtual pointer */
class cweeSharedPtrWithSentinel_Data_Interface {
protected:
	cweeSharedPtrWithSentinel_Data_Interface() noexcept {};
	virtual ~cweeSharedPtrWithSentinel_Data_Interface() noexcept {};
	explicit cweeSharedPtrWithSentinel_Data_Interface(cweeSharedPtrWithSentinel_Data_Interface const& other) noexcept = delete;
	explicit cweeSharedPtrWithSentinel_Data_Interface(cweeSharedPtrWithSentinel_Data_Interface&& other) noexcept = delete;
	cweeSharedPtrWithSentinel_Data_Interface& operator=(cweeSharedPtrWithSentinel_Data_Interface const& other) = delete;
	cweeSharedPtrWithSentinel_Data_Interface& operator=(cweeSharedPtrWithSentinel_Data_Interface&& other) = delete;
public:
	virtual void* source() const noexcept = 0;
	virtual int	use_count() const noexcept = 0;
	virtual void increment() const noexcept = 0;
	virtual int decrement() const noexcept = 0;
	virtual void add_on_destroy(const std::function<void()>& func) const noexcept = 0;
	virtual cweeSysInterlockedPointer<Details_InterfaceWithSentinel>& ptr() const noexcept = 0;
};
template <typename type> class cweeSharedPtrWithSentinel {
#pragma region Class Defs
public:
	using swappablePtr = cweeSysInterlockedPointer<type>;
	class details final : public Details_InterfaceWithSentinel {
	public:
		explicit details() = delete;
		~details() noexcept {};
		explicit details(details const&) = delete;
		explicit details(details&&) = delete;
		details& operator=(details const&) = delete;
		details& operator=(details&&) = delete;
		explicit details(type* _source, std::function<void(type*)> _destroy) noexcept :
			p(_source), count(1), deleter(std::move(_destroy)) {};
		void* source() const noexcept final { return static_cast<void*>(p); };
		int	use_count() const noexcept final {
			return count.GetValue();
		};
		void increment() const noexcept final {
			count.Increment();
		};
		int decrement() const noexcept final {
			int i = count.Decrement();
			if (i == 0) {
				deleter(p.Set(nullptr));
			}
			return i;
		};
		void add_on_destroy(const std::function<void()>& func) const noexcept  final {
			auto& ptr = m_destroyers;
			ptr.Lock();
			ptr->push_back(func);
			ptr.Unlock();
		};
		cweeSharedPtrWithSentinelDestroyerListType& destroyers() const noexcept final {
			return m_destroyers;
		};
		std::function<void(Details_InterfaceWithSentinel*)> Details_InterfaceWithSentinel_Deleter() const noexcept final {
			return [](Details_InterfaceWithSentinel* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtrWithSentinel<type>::details*>(p);
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
		mutable cweeSharedPtrWithSentinelDestroyerListType	m_destroyers;
	};
	class details_no_destructor final : public Details_InterfaceWithSentinel {
	public:
		explicit details_no_destructor() = delete;
		~details_no_destructor() noexcept {};
		explicit details_no_destructor(details_no_destructor const&) = delete;
		explicit details_no_destructor(details_no_destructor&&) = delete;
		details_no_destructor& operator=(details_no_destructor const&) = delete;
		details_no_destructor& operator=(details_no_destructor&&) = delete;
		explicit details_no_destructor(type* _source) noexcept :
			p(_source), count(1) {};
		void* source() const noexcept final {
			return (void*)p.Get();
		};
		int	use_count() const noexcept final {
			return count.GetValue();
		};
		void increment() const noexcept final {
			count.Increment();
		};
		int decrement() const noexcept final {
			int i = count.Decrement();
			if (i == 0) {
				AUTO P = p.Set(nullptr);
				if (P) {
					delete P;
				}
			}
			return i;
		};
		void add_on_destroy(const std::function<void()>& func) const noexcept  final {
			auto& ptr = m_destroyers;
			ptr.Lock();
			ptr->push_back(func);
			ptr.Unlock();
		};
		cweeSharedPtrWithSentinelDestroyerListType& destroyers() const noexcept final {
			return m_destroyers;
		};
		std::function<void(Details_InterfaceWithSentinel*)> Details_InterfaceWithSentinel_Deleter() const noexcept final {
			return [](Details_InterfaceWithSentinel* p) {
				if (p) {
					AUTO q = dynamic_cast<cweeSharedPtrWithSentinel<type>::details_no_destructor*>(p);
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
		mutable cweeSharedPtrWithSentinelDestroyerListType	m_destroyers;
	};
	class cweeSharedPtrWithSentinel_DataImpl final : public cweeSharedPtrWithSentinel_Data_Interface {
	public:
		explicit cweeSharedPtrWithSentinel_DataImpl() = delete;
		~cweeSharedPtrWithSentinel_DataImpl() noexcept { decrement(); };
		explicit cweeSharedPtrWithSentinel_DataImpl(cweeSharedPtrWithSentinel_DataImpl const&) = delete;
		explicit cweeSharedPtrWithSentinel_DataImpl(cweeSharedPtrWithSentinel_DataImpl&&) = delete;
		cweeSharedPtrWithSentinel_DataImpl& operator=(cweeSharedPtrWithSentinel_DataImpl const&) = delete;
		cweeSharedPtrWithSentinel_DataImpl& operator=(cweeSharedPtrWithSentinel_DataImpl&&) = delete;

		explicit cweeSharedPtrWithSentinel_DataImpl(const cweeSharedPtrWithSentinel_Data_Interface& other, std::function<type* (void*)> _getter) noexcept :
			det(other.ptr()), m_getter(std::move(_getter)) {
			increment();
		};

		explicit cweeSharedPtrWithSentinel_DataImpl(type* _source, std::function<void(type*)> _on_destroy, std::function<type* (void*)> _getter) noexcept :
			det(new details(_source, std::move(_on_destroy))), m_getter(std::move(_getter)) {};
		explicit cweeSharedPtrWithSentinel_DataImpl(type* _source, std::function<type* (void*)> _getter) noexcept :
			det(new details_no_destructor(_source)), m_getter(std::move(_getter)) {};

		void* source() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				return p->source();
			}
			return nullptr;
		};
		int	use_count() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				return p->use_count();
			}
			return 0;
		};
		void increment() const noexcept final {
			AUTO p = det.Get();
			if (p) {
				p->increment();
			}
		};
		int decrement() const noexcept final {
			int i = -1;
			AUTO p = det.Get();
			if (p) {
				i = p->decrement();
				if (i == 0) {
					{
						AUTO p2 = det.Set(nullptr);
						if (p2 != nullptr) {
							auto& destroyersP = p2->destroyers();							
							{
								AUTO g = destroyersP.Guard();
								for (auto& dest : *destroyersP) {
									dest();
								}
							}
							AUTO func = p2->Details_InterfaceWithSentinel_Deleter();
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
		void add_on_destroy(const std::function<void()>& func) const noexcept final {
			AUTO p = this->det.Get();
			if (p) {
				p->add_on_destroy(func);
			}
			else {
				func();
			}
		};

		cweeSysInterlockedPointer<Details_InterfaceWithSentinel>& ptr() const noexcept final { return det; };

	public:
		std::function<type* (void*)>							m_getter;
		mutable cweeSysInterlockedPointer<Details_InterfaceWithSentinel>	det;
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
	mutable cweeSysInterlockedPointer<cweeSharedPtrWithSentinel_DataImpl>			m_data;
#pragma endregion 
#pragma region Create or Destroy
public:
	/*! Create an empty pointer (nullptr) */
	constexpr cweeSharedPtrWithSentinel() noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a nullptr */
	constexpr cweeSharedPtrWithSentinel(std::nullptr_t) noexcept : mutex(0), m_data(nullptr) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtrWithSentinel(PtrType source) noexcept : mutex(0), m_data(InitData(source)) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtrWithSentinel(PtrType source, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(source, std::move(destroy))) {};

	/*! Instantiate a shared pointer by handing over a "new pointer()" to be managed, shared, and ultimately deleted by the shared pointer. */
	cweeSharedPtrWithSentinel(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitData(source, std::move(destroy), std::move(_getter))) {};

	/*! instantiate with a constructor and destructor */
	cweeSharedPtrWithSentinel(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept : mutex(0), m_data(InitData(std::move(create), std::move(destroy))) {};

	/*! instantiate from another ptr */
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel const& samePtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr)) {};
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel>(other))) {};

	/*! instantiate from another ptr with complex "get" instructions */
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel const& samePtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(samePtr, std::move(_getter))) {};
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel>(other), std::move(_getter))) {};

	/*! instantiate from another ptr with a different Type, using basic cast operations */
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
	cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T> const& similarPtr) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, [](void* p) constexpr -> PtrType {
		return static_cast<PtrType>((T*)p); // dynamic_cast
		})) {};
		template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
		cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T>&& other) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel<T>>(other), [](void* p) constexpr -> PtrType {
			return static_cast<PtrType>((T*)p); // dynamic_cast
			})) {};

			/*! instantiate from another ptr with a different Type with complex "get" instructions */
			template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
			cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T> const& similarPtr, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(similarPtr, std::move(_getter))) {};
			template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> >
			cweeSharedPtrWithSentinel(cweeSharedPtrWithSentinel<T>&& other, std::function<PtrType(void*)> _getter) noexcept : mutex(0), m_data(InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel<T>>(other), std::move(_getter))) {};

			template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
			cweeSharedPtrWithSentinel(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type> const& source) noexcept : mutex(0), m_data(
#ifdef allowcweeSharedPtrWithSentinelCaptureByValue
				new cweeSharedPtrWithSentinel_DataImpl(source, std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; }))
#else
				InitData(new Type(source))
#endif
			) {};

			template <typename Q = type, typename = std::enable_if_t<!std::is_same_v<Q, void>> >
			cweeSharedPtrWithSentinel(std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>&& source) noexcept : mutex(0), m_data(
#ifdef allowcweeSharedPtrWithSentinelCaptureByValue
				new cweeSharedPtrWithSentinel_DataImpl(std::forward<std::decay_t<typename std::remove_reference<typename std::remove_pointer<Q>::type>::type>>(source), std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; }))
#else
				InitData(new Type(std::forward<Type>(source)))
#endif
			) {};

			/*! Destroy this instance of the shared pointer and potentially delete the data */
			~cweeSharedPtrWithSentinel() noexcept {
				ClearData();
			};
#pragma endregion
#pragma region Internal Support Functions
public:
	PtrType Get() const {
		PtrType out(nullptr);

		Lock();
		auto* d = m_data.Get();
		if (d) {
			out = d->m_getter(d->source());
		}
		Unlock();
		return out;
	};
	template <typename T> cweeSharedPtrWithSentinel<T>* UnsafeSentinel(const cweeSharedPtrWithSentinel<T>& reference) const noexcept {
		AUTO p = new cweeSharedPtrWithSentinel<T>(reference);
		this->UnsafeDoOnThisScopeLoss([p]() {
			delete p;
			});
		return p;
	};
	template <typename T> cweeSharedPtr<T>* UnsafeSentinel(const cweeSharedPtr<T>& reference) const noexcept {
		AUTO p = new cweeSharedPtr<T>(reference);
		this->UnsafeDoOnThisScopeLoss([p]() {
			delete p;
			});
		return p;
	};

private:
	void UnsafeDoOnThisScopeLoss(std::function<void()> todo_after_delete) const noexcept {
		auto* d = m_data.Get();
		if (d) {
			d->add_on_destroy(std::move(todo_after_delete));
		}
		else {
			todo_after_delete();
		}
	};

public:
	void Lock() const noexcept {
		while (mutex.Increment() != 1) {
			mutex.Decrement();
		}
	};
	void Unlock() const noexcept {
		mutex.Decrement();
	};
	PtrType UnsafeGet() const noexcept {
		PtrType out(nullptr);
		auto* d = m_data.Get();
		if (d) {
			out = d->m_getter(d->source());
		}
		return out;
	};
	template <typename... Args> void UnsafeSet(Args... Fargs) {
		UnsafeSetData(cweeSharedPtrWithSentinel<type>::InitData(Fargs...));
	};
protected:
	template <typename T> static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T> const& Ptr) noexcept {
		return InitDataFromAnotherPtr(Ptr, [](void* p) constexpr -> PtrType { return (PtrType)p; });
	};
	template <typename T> static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T> const& Ptr, std::function<PtrType(void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<T>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Get();
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T>&& Ptr) noexcept {
		return InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel<T>>(Ptr), [](void* p) constexpr -> PtrType { return (PtrType)p; });
	};
	template <typename T, typename = std::enable_if_t<!std::is_same_v<Type, T>> > static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel<T>&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<T>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};
	static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel&& Ptr) noexcept {
		cweeSharedPtrWithSentinel_DataImpl* p;
		Ptr.Lock();
		p = Ptr.m_data.Set(nullptr);
		Ptr.Unlock();
		return p;
	};
	static cweeSharedPtrWithSentinel_DataImpl* InitDataFromAnotherPtr(cweeSharedPtrWithSentinel&& Ptr, std::function<type* (void*)> from) noexcept {
		Ptr.Lock();
		typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl* p = Ptr.m_data.Set(nullptr);
		if (p) {
			AUTO d = new cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl(*p, std::move(from));
			Ptr.Unlock();
			delete p;
			return d;
		}
		Ptr.Unlock();
		return nullptr;
	};

	static AUTO InitData(PtrType source, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtrWithSentinel_DataImpl(source, std::move(from));
	};
	static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept {
		return new cweeSharedPtrWithSentinel_DataImpl(source, std::move(destroy), std::move(from));
	};
	static AUTO InitData(PtrType source) noexcept { return InitData(source, std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	static AUTO InitData(PtrType source, std::function<void(PtrType)> destroy) noexcept { return InitData(source, std::move(destroy), std::function<PtrType(void*)>([](void* p) constexpr -> PtrType {return (PtrType)p; })); };
	static AUTO InitData(std::function<PtrType()> create) noexcept { return InitData(create()); };
	static AUTO InitData(std::function<PtrType()> create, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(from)); };
	static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy) noexcept { return InitData(create(), std::move(destroy)); };
	static AUTO InitData(std::function<PtrType()> create, std::function<void(PtrType)> destroy, std::function<PtrType(void*)> from) noexcept { return InitData(create(), std::move(destroy), std::move(from)); };

	void SetData(cweeSharedPtrWithSentinel_DataImpl* p) const noexcept {
		Lock();
		AUTO d = m_data.Set(p);
		Unlock();
		if (d) {
			delete d;
		}
	};
	void UnsafeSetData(cweeSharedPtrWithSentinel_DataImpl* p) const noexcept {
		AUTO d = m_data.Set(p);
		if (d) {
			delete d;
		}
	};
	void ClearData() const noexcept { SetData(nullptr); };

#pragma endregion 
#pragma region Boolean Operators
public:
	constexpr explicit operator bool() const { return m_data.Get(); };
	friend bool operator==(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() == b.Get(); };
	friend bool operator!=(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() != b.Get(); };
	friend bool operator<(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() < b.Get(); };
	friend bool operator<=(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() <= b.Get(); };
	friend bool operator>(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() > b.Get(); };
	friend bool operator>=(const cweeSharedPtrWithSentinel& a, const cweeSharedPtrWithSentinel& b) noexcept { return a.Get() >= b.Get(); };
	friend bool operator==(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() == nullptr; };
	friend bool operator!=(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() != nullptr; };
	friend bool operator<(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() < nullptr; };
	friend bool operator<=(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() <= nullptr; };
	friend bool operator>(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() > nullptr; };
	friend bool operator>=(const cweeSharedPtrWithSentinel& a, std::nullptr_t) noexcept { return a.m_data.Get() >= nullptr; };
	friend bool operator==(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr == a.m_data.Get(); };
	friend bool operator!=(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr != a.m_data.Get(); };
	friend bool operator<(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr < a.m_data.Get(); };
	friend bool operator<=(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr <= a.m_data.Get(); };
	friend bool operator>(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr > a.m_data.Get(); };
	friend bool operator>=(std::nullptr_t, const cweeSharedPtrWithSentinel& a) noexcept { return nullptr >= a.m_data.Get(); };
#pragma endregion
#pragma region Ptr Casting
public:
	/*! Create a shared pointer from this one with an added const component to the underlying managed object type */
	cweeSharedPtrWithSentinel<typename std::add_const<Type>::type> ConstReference() const noexcept {
		typedef typename std::add_const<Type>::type newT;
		cweeSharedPtrWithSentinel<newT> out;
		typedef typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl oldDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::cweeSharedPtrWithSentinel_DataImpl newDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::PtrType newP;
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
	cweeSharedPtrWithSentinel<typename std::remove_const<Type>::type> RemoveConstReference() const noexcept {
		typedef typename std::remove_const<Type>::type newT;
		cweeSharedPtrWithSentinel<newT> out;
		typedef typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl oldDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::cweeSharedPtrWithSentinel_DataImpl newDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::PtrType newP;
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
	template<typename astype>  cweeSharedPtrWithSentinel<typename std::remove_const<astype>::type> CastReference() const noexcept {
		typedef typename std::remove_const<astype>::type newT;
		cweeSharedPtrWithSentinel<newT> out;
		typedef typename cweeSharedPtrWithSentinel<type>::cweeSharedPtrWithSentinel_DataImpl oldDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::cweeSharedPtrWithSentinel_DataImpl newDI;
		typedef typename cweeSharedPtrWithSentinel<newT>::PtrType newP;
		std::function<newP(void*)> _getter = [](void* p) {
			return dynamic_cast<newP>((typename std::remove_const<PtrType>::type)(p));
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
	PtrType operator->() const noexcept { return Get(); };

	template<typename Q = type> typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator[](std::ptrdiff_t idx) const
	{
		AUTO x = Get();
		if (x) return x[idx];
		throw(std::exception("cweeSharedPtrWithSentinel was null with indexed access"));
	};

	template<typename Q = type> typename std::enable_if<!std::is_same<Q, void>::value, Q&>::type operator*() const
	{
		AUTO x = Get();
		if (x) return *x;
		throw(std::exception("cweeSharedPtrWithSentinel was null with reference access"));
	};

	template<typename Q = type> typename std::enable_if<std::is_same<Q, void>::value, void>::type operator[](std::ptrdiff_t idx) const { throw(std::exception("cweeSharedPtrWithSentinel was null with indexed access")); };

	template<typename Q = type> typename std::enable_if<std::is_same<Q, void>::value, void>::type operator*() const { throw(std::exception("cweeSharedPtrWithSentinel was null with reference access")); };

#pragma endregion
#pragma region Assignment Operators
public:
	/*! Share ownership */
	cweeSharedPtrWithSentinel& operator=(const cweeSharedPtrWithSentinel& other) noexcept {
		if (other == *this) { return *this; }

		AUTO p = InitDataFromAnotherPtr(other);
		SetData(p);

		return *this;
	};

	/*! Take ownership */
	cweeSharedPtrWithSentinel& operator=(cweeSharedPtrWithSentinel&& other) noexcept {
		if (other == *this) { return *this; }

		other.Lock();
		AUTO p = other.m_data.Set(nullptr); // InitDataFromAnotherPtr(std::forward<cweeSharedPtrWithSentinel>(other));
		other.Unlock();
		SetData(p);

		return *this;
	};
#pragma endregion
#pragma region Swap Operators
public:
	cweeSharedPtrWithSentinel& Swap(cweeSharedPtrWithSentinel& other) {
		cweeSharedPtrWithSentinel t = *this;
		*this = other;
		other = t;
		return *this;
	};
#pragma endregion
#pragma region "std::shared_ptr" Compatability Methods
public:
	/*! Get access to the underlying data in a thread-safe way. (The underlying data isn't necessarily thread-safe, but accessing the pointer itself has no data race.) */
	PtrType get() const noexcept { return Get(); };
	/*! Empty the current data pointer and free the data. */
	void reset() { cweeSharedPtrWithSentinel().Swap(*this); };
	/*! Exchange the current data with the provided new data. */
	template< class Y > void reset(Y* ptr) { cweeSharedPtrWithSentinel(ptr).Swap(*this); };
	/*! Swap shared pointers */
	void swap(cweeSharedPtrWithSentinel& r) { Swap(r); };
	/*! Number of shared pointers with access to the underlying data, including this one. */
	long use_count() const noexcept {
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
	class cweeSharedPtrWithSentinelGuard {
	public:
		constexpr explicit operator bool() const { return m_p; };
		cweeSharedPtrWithSentinelGuard() = delete;
		cweeSharedPtrWithSentinelGuard(cweeSharedPtrWithSentinelGuard&&) = delete;
		cweeSharedPtrWithSentinelGuard(cweeSharedPtrWithSentinelGuard const&) = delete;
		cweeSharedPtrWithSentinelGuard& operator=(cweeSharedPtrWithSentinelGuard const&) = delete;
		cweeSharedPtrWithSentinelGuard& operator=(cweeSharedPtrWithSentinelGuard&&) = delete;
		explicit cweeSharedPtrWithSentinelGuard(cweeSharedPtrWithSentinel* p_p) noexcept : m_p(p_p) { p_p->Lock(); }
		~cweeSharedPtrWithSentinelGuard() noexcept { m_p->Unlock(); };
	private:
		cweeSharedPtrWithSentinel* m_p;
	};
	[[nodiscard]] cweeSharedPtrWithSentinelGuard Guard() { return cweeSharedPtrWithSentinelGuard(this); };
#pragma endregion
};