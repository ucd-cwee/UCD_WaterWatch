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

/* ATOMIC INTEGER COLLECTION */
template <typename type> class cweeSysInterlockedValue {
public:
	cweeSysInterlockedValue() : value((type)0) {};
	cweeSysInterlockedValue(const type& a) : value(a) {};
	cweeSysInterlockedValue(const cweeSysInterlockedValue& other) : value(other.GetValue()) {};
	cweeSysInterlockedValue& operator=(const cweeSysInterlockedValue& other) { SetValue(other.GetValue()); return *this; };
	cweeSysInterlockedValue& operator=(const type& newSource) { SetValue(newSource); return *this; };

	operator type() { return GetValue(); };
	operator type() const { return GetValue(); };

	friend cweeSysInterlockedValue operator+(const type& i, const cweeSysInterlockedValue& b) { cweeSysInterlockedValue out(b); out.Add(i); return out; };
	friend cweeSysInterlockedValue operator+(const cweeSysInterlockedValue& b, const type& i) { cweeSysInterlockedValue out(b); out.Add(i); return out; };
	friend cweeSysInterlockedValue operator-(const type& i, const cweeSysInterlockedValue& b) { cweeSysInterlockedValue out(i); out.Add(-b.GetValue()); return out; };
	friend cweeSysInterlockedValue operator-(const cweeSysInterlockedValue& b, const type& i) { cweeSysInterlockedValue out(b); out.Add(-i); return out; };
	friend cweeSysInterlockedValue operator/(const type& i, const cweeSysInterlockedValue& b) { return i / b.GetValue(); };
	friend cweeSysInterlockedValue operator/(const cweeSysInterlockedValue& b, const type& i) { return b.GetValue() / i; };

	cweeSysInterlockedValue& operator+=(int i) { Add(i); return *this; };
	cweeSysInterlockedValue& operator-=(int i) { Add(-i); return *this; };

	cweeSysInterlockedValue& operator+=(const cweeSysInterlockedValue& i) { Add(i.GetValue()); return *this; };
	cweeSysInterlockedValue& operator-=(const cweeSysInterlockedValue& i) { Add(-i.GetValue()); return *this; };

	bool operator==(const type& i) { return i == GetValue(); };
	bool operator!=(const type& i) { return i != GetValue(); };
	bool operator==(const cweeSysInterlockedValue& i) { return i.GetValue() == GetValue(); };
	bool operator!=(const cweeSysInterlockedValue& i) { return i.GetValue() != GetValue(); };

	friend bool operator<=(const type& i, const cweeSysInterlockedValue& b) { return i <= b.GetValue(); };
	friend bool operator<=(const cweeSysInterlockedValue& b, const type& i) { return i > b.GetValue(); };
	friend bool operator>=(const type& i, const cweeSysInterlockedValue& b) { return i >= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedValue& b, const type& i) { return i < b.GetValue(); };
	friend bool operator>(const type& i, const cweeSysInterlockedValue& b) { return i > b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedValue& b, const type& i) { return i <= b.GetValue(); };
	friend bool operator<(const type& i, const cweeSysInterlockedValue& b) { return i < b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedValue& b, const type& i) { return i >= b.GetValue(); };

	friend bool operator<=(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() <= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() >= b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() > b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedValue& i, const cweeSysInterlockedValue& b) { return i.GetValue() < b.GetValue(); };

	type					Increment() { return cweeSysThreadTools::Sys_InterlockedIncrementV(value); } // atomically increments the integer and returns the new value
	type					Decrement() { return cweeSysThreadTools::Sys_InterlockedDecrementV(value); } // atomically decrements the integer and returns the new value
	type					Add(const type& v) { return cweeSysThreadTools::Sys_InterlockedAddV(value, (type)v); } // atomically adds a value to the integer and returns the new value
	type					Sub(const type& v) { return cweeSysThreadTools::Sys_InterlockedSubV(value, (type)v); } // atomically subtracts a value from the integer and returns the new value
	type					GetValue() const { return value; } // returns the current value of the integer
	void				    SetValue(const type& v) { value = v; };

private:
	type	            value;
};

class cweeSysInterlockedInteger {
public:
	constexpr cweeSysInterlockedInteger() noexcept : value(0) {};
	constexpr cweeSysInterlockedInteger(int a) noexcept : value(a) {};
	cweeSysInterlockedInteger(const cweeSysInterlockedInteger& other) : value(other.GetValue()) {};
	cweeSysInterlockedInteger& operator=(const cweeSysInterlockedInteger& other) { SetValue(other.GetValue()); return *this; };
	cweeSysInterlockedInteger& operator=(int newSource) { SetValue(newSource); return *this; };

	explicit operator int() { return GetValue(); };
	explicit operator int() const { return GetValue(); };

	explicit operator bool() { if (GetValue() == 0) return false; else return true; };
	explicit operator bool() const { if (GetValue() == 0) return false; else return true; };

	friend cweeSysInterlockedInteger operator+(int i, const cweeSysInterlockedInteger& b) { cweeSysInterlockedInteger out(b); out.Add(i); return out; };
	friend cweeSysInterlockedInteger operator+(const cweeSysInterlockedInteger& b, int i) { cweeSysInterlockedInteger out(b); out.Add(i); return out; };
	friend cweeSysInterlockedInteger operator-(int i, const cweeSysInterlockedInteger& b) { cweeSysInterlockedInteger out(i); out.Add(-b.GetValue()); return out; };
	friend cweeSysInterlockedInteger operator-(const cweeSysInterlockedInteger& b, int i) { cweeSysInterlockedInteger out(b); out.Add(-i); return out; };
	friend cweeSysInterlockedInteger operator/(int i, const cweeSysInterlockedInteger& b) { return i / b.GetValue(); };
	friend cweeSysInterlockedInteger operator/(const cweeSysInterlockedInteger& b, int i) { return b.GetValue() / i; };

	friend bool operator<=(int i, const cweeSysInterlockedInteger& b) { return i <= b.GetValue(); };
	friend bool operator<=(const cweeSysInterlockedInteger& b, int i) { return i > b.GetValue(); };
	friend bool operator>=(int i, const cweeSysInterlockedInteger& b) { return i >= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedInteger& b, int i) { return i < b.GetValue(); };
	friend bool operator>(int i, const cweeSysInterlockedInteger& b) { return i > b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedInteger& b, int i) { return i <= b.GetValue(); };
	friend bool operator<(int i, const cweeSysInterlockedInteger& b) { return i < b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedInteger& b, int i) { return i >= b.GetValue(); };

	friend bool operator<=(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() <= b.GetValue(); };
	friend bool operator>=(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() >= b.GetValue(); };
	friend bool operator>(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() > b.GetValue(); };
	friend bool operator<(const cweeSysInterlockedInteger& i, const cweeSysInterlockedInteger& b) { return i.GetValue() < b.GetValue(); };

	cweeSysInterlockedInteger& operator+=(int i) { Add(i); return *this; };
	cweeSysInterlockedInteger& operator-=(int i) { Add(-i); return *this; };

	cweeSysInterlockedInteger& operator+=(const cweeSysInterlockedInteger& i) { Add(i.GetValue()); return *this; };
	cweeSysInterlockedInteger& operator-=(const cweeSysInterlockedInteger& i) { Add(-i.GetValue()); return *this; };

	bool operator==(int i) { return i == GetValue(); };
	bool operator!=(int i) { return i != GetValue(); };
	bool operator==(const cweeSysInterlockedInteger& i) { return i.GetValue() == GetValue(); };
	bool operator!=(const cweeSysInterlockedInteger& i) { return i.GetValue() != GetValue(); };

	int					Increment() { return cweeSysThreadTools::Sys_InterlockedIncrement(value); } // atomically increments the integer and returns the new value
	int					Decrement() { return cweeSysThreadTools::Sys_InterlockedDecrement(value); } // atomically decrements the integer and returns the new value
	int					Add(int v) { return cweeSysThreadTools::Sys_InterlockedAdd(value, (interlockedInt_t)v); } // atomically adds a value to the integer and returns the new value
	int					Sub(int v) { return cweeSysThreadTools::Sys_InterlockedSub(value, (interlockedInt_t)v); } // atomically subtracts a value from the integer and returns the new value
	int					GetValue() const { return cweeSysThreadTools::Sys_InterlockedAdd(const_cast<interlockedInt_t&>(value), 0); } // returns the current value of the integer
	void				SetValue(int v) { cweeSysThreadTools::Sys_InterlockedAdd(value, (interlockedInt_t)(v - GetValue())); };

	bool				TryIncrementTo(int n) {
		if (Increment() == n) {
			return true;
		}
		Decrement();
		return false;
	};

private:
	interlockedInt_t	value;
};

/* ATOMIC POINTER COLLECTION */
template< typename T > class cweeSysInterlockedPointer {
public:
	constexpr cweeSysInterlockedPointer() noexcept : ptr(nullptr) {}
	constexpr cweeSysInterlockedPointer(std::nullptr_t) noexcept : ptr(nullptr) {}
	constexpr cweeSysInterlockedPointer(T* newSource) noexcept : ptr(newSource) {}
	constexpr cweeSysInterlockedPointer(const cweeSysInterlockedPointer& other) : ptr(other.Get()) {};
	cweeSysInterlockedPointer& operator=(const cweeSysInterlockedPointer& other) { Set(other.Get()); return *this; };
	cweeSysInterlockedPointer& operator=(T* newSource) { Set(newSource); return *this; };
	~cweeSysInterlockedPointer() { ptr = nullptr; };

	explicit operator bool() { return ptr; };
	explicit operator bool() const { return ptr; };

	constexpr operator T* () noexcept { return ptr; };
	constexpr operator const T* () const noexcept { return ptr; };

	/* atomically sets the pointerand returns the previous pointer value */
	T* Set(T* newPtr) { return (T*)cweeSysThreadTools::Sys_InterlockedExchangePointer((void*&)ptr, (void*)newPtr); };
	/* atomically sets the pointer to 'newPtr' only if the previous pointer is equal to 'comparePtr' */
	T* CompareExchange(T* comparePtr, T* newPtr) { return (T*)cweeSysThreadTools::Sys_InterlockedCompareExchangePointer((void*&)ptr, (void*)comparePtr, (void*)newPtr); };

	constexpr T* operator->() noexcept { return Get(); };
	constexpr const T* operator->() const noexcept { return Get(); };
	constexpr T* Get() noexcept { return ptr; };
	constexpr T* Get() const noexcept { return ptr; };

private:
	T* ptr;
};

class cweeSysMutexImpl {
public:
	cweeSysMutexImpl() noexcept { cweeSysThreadTools::Sys_MutexCreate(Handle); };
	~cweeSysMutexImpl()	noexcept { cweeSysThreadTools::Sys_MutexDestroy(Handle); };
	bool Lock(bool blocking = true) noexcept { return cweeSysThreadTools::Sys_MutexLock(Handle, blocking); };
	void Unlock() noexcept { cweeSysThreadTools::Sys_MutexUnlock(Handle); };

	cweeSysMutexImpl(const cweeSysMutexImpl&) = delete;
	cweeSysMutexImpl(cweeSysMutexImpl&&) = delete;
	cweeSysMutexImpl& operator=(cweeSysMutexImpl const&) = delete;
	cweeSysMutexImpl& operator=(cweeSysMutexImpl&&) = delete;

protected:
	mutexHandle_t Handle;
};

class cweeConstexprLock {
public:
	class cweeConstexprLockLifetimeGuard {
	public:
		constexpr explicit operator bool() const noexcept { return true; };
		explicit cweeConstexprLockLifetimeGuard(cweeSysInterlockedInteger& mut) noexcept : owner(mut) { while (owner.Increment() != 1) { owner.Decrement(); } };
		~cweeConstexprLockLifetimeGuard() noexcept { owner.Decrement();	};
		explicit cweeConstexprLockLifetimeGuard() = delete;
		explicit cweeConstexprLockLifetimeGuard(const cweeSysInterlockedInteger& other) = delete;
		explicit cweeConstexprLockLifetimeGuard(cweeSysInterlockedInteger&& other) = delete;
		cweeConstexprLockLifetimeGuard& operator=(const cweeSysInterlockedInteger&) = delete;
		cweeConstexprLockLifetimeGuard& operator=(cweeSysInterlockedInteger&&) = delete;
	protected:
		cweeSysInterlockedInteger& owner;
	};

public:
	constexpr cweeConstexprLock() noexcept : Handle(0) {};
	constexpr cweeConstexprLock(const cweeConstexprLock& other) noexcept : Handle(0) {};
	constexpr cweeConstexprLock(cweeConstexprLock&& other) noexcept : Handle(0) {};
	constexpr cweeConstexprLock& operator=(const cweeConstexprLock& s) noexcept { return *this; };
	constexpr cweeConstexprLock& operator=(cweeConstexprLock&& s) noexcept { return *this; };
	~cweeConstexprLock() {};

	NODISCARD		cweeConstexprLockLifetimeGuard	Guard() const noexcept { return cweeConstexprLockLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const noexcept {
		if (blocking) {
			while (Handle.Increment() != 1) Handle.Decrement();

			return true;
		}
		else {
			if (Handle.Increment() != 1) {
				Handle.Decrement();
				return false;
			}
			return true;
		}
	};
	void			Unlock() const noexcept {
		Handle.Decrement();
	};
	void            lock() const { Lock(); }
	void            unlock() const { Unlock(); }
protected:
	mutable cweeSysInterlockedInteger Handle;

};

template <typename T>
class LockedObject {
public:
	LockedObject() noexcept : obj(), lock() {};
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject(T&& o) noexcept : obj(std::forward<T>(o)), lock() {};
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject(T const& o) noexcept : obj(o), lock() {};
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject& operator=(T const& o) { Lock(); obj = o; Unlock(); };
	template<typename = std::enable_if_t<!std::is_same_v<LockedObject, std::decay_t<typename std::remove_reference<typename std::remove_pointer<T>::type>::type>>>>
	LockedObject& operator=(T&& o) { Lock(); obj = std::forward<T>(o); Unlock(); };

	LockedObject(LockedObject&& o) noexcept : obj(o.obj), lock(o.lock) {};
	LockedObject(LockedObject const& o) noexcept : obj(o.obj), lock(o.lock) {};
	LockedObject& operator=(LockedObject const& o) { if (this != &o) { Lock(); o.Lock(); obj = o.obj; o.Unlock(); Unlock(); } return *this; };
	LockedObject& operator=(LockedObject&& o) { if (this != &o) { Lock(); o.Lock(); obj = o.obj; o.Unlock(); Unlock(); } return *this; };

	~LockedObject() noexcept {};

	T* get() const noexcept { return &obj; };
	T* operator->() const noexcept { return get(); };
	T& operator*() const noexcept { return *get(); };
	void Lock() const noexcept { lock.Lock(); };
	void Unlock() const noexcept { lock.Unlock(); };
	NODISCARD AUTO Guard() const noexcept { return lock.Guard(); };

private:
	mutable T	obj;
	mutable cweeConstexprLock lock;
};

