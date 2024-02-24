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

