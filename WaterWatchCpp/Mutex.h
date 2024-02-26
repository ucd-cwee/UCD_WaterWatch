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

#if 0
class cweeSharedMutex {
public:
	class cweeSharedMutexLifetimeGuard {
	public:
		constexpr explicit operator bool() const { return (bool)owner; };
		explicit cweeSharedMutexLifetimeGuard(const cweeSharedPtr<std::shared_mutex>& mut) noexcept : owner(mut), shared_guard(*mut) {};
		~cweeSharedMutexLifetimeGuard() noexcept {  };
		explicit cweeSharedMutexLifetimeGuard() = delete;
		explicit cweeSharedMutexLifetimeGuard(const cweeSharedMutexLifetimeGuard& other) = delete;
		explicit cweeSharedMutexLifetimeGuard(cweeSharedMutexLifetimeGuard&& other) = delete;
		cweeSharedMutexLifetimeGuard& operator=(const cweeSharedMutexLifetimeGuard&) = delete;
		cweeSharedMutexLifetimeGuard& operator=(cweeSharedMutexLifetimeGuard&&) = delete;
	protected:
		cweeSharedPtr<std::shared_mutex> owner;
		std::unique_lock<std::shared_mutex> shared_guard;
	};
	class cweeSharedMutexSharedLifetimeGuard {
	public:
		constexpr explicit operator bool() const { return (bool)owner; };
		explicit cweeSharedMutexSharedLifetimeGuard(const cweeSharedPtr<std::shared_mutex>& mut) noexcept : owner(mut), shared_guard(*mut) {};
		~cweeSharedMutexSharedLifetimeGuard() noexcept {  };
		explicit cweeSharedMutexSharedLifetimeGuard() = delete;
		explicit cweeSharedMutexSharedLifetimeGuard(const cweeSharedMutexSharedLifetimeGuard& other) = delete;
		explicit cweeSharedMutexSharedLifetimeGuard(cweeSharedMutexSharedLifetimeGuard&& other) = delete;
		cweeSharedMutexSharedLifetimeGuard& operator=(const cweeSharedMutexSharedLifetimeGuard&) = delete;
		cweeSharedMutexSharedLifetimeGuard& operator=(cweeSharedMutexSharedLifetimeGuard&&) = delete;
	protected:
		cweeSharedPtr<std::shared_mutex> owner;
		std::shared_lock<std::shared_mutex> shared_guard;
	};

public:
	cweeSharedMutex() : Handle(new std::shared_mutex()) {};
	cweeSharedMutex(const cweeSharedMutex& other) : Handle(new std::shared_mutex()) {};
	cweeSharedMutex(cweeSharedMutex&& other) : Handle(new std::shared_mutex()) {};
	cweeSharedMutex& operator=(const cweeSharedMutex& s) { return *this; };
	cweeSharedMutex& operator=(cweeSharedMutex&& s) { return *this; };
	~cweeSharedMutex() {};

	NODISCARD cweeSharedMutexLifetimeGuard	Guard() const { return cweeSharedMutexLifetimeGuard(Handle); };
	NODISCARD cweeSharedMutexSharedLifetimeGuard SharedGuard() const { return cweeSharedMutexSharedLifetimeGuard(Handle); };
	bool			Lock() const { 
		Handle->lock();
		return true;
	};
	void			Unlock() const { 
		Handle->unlock();
	};
	bool			SharedLock() const {
		Handle->lock_shared();
		return true;
	};
	void			SharedUnlock() const {
		Handle->unlock_shared();
	};

protected:
	cweeSharedPtr<std::shared_mutex> Handle;

};
#endif

class cweeSysSignalImpl {
public:
	static constexpr int WAIT_INFINITE = -1;
	cweeSysSignalImpl(bool manualReset = false) noexcept { cweeSysThreadTools::Sys_SignalCreate(Handle, manualReset); };
	~cweeSysSignalImpl()	noexcept { cweeSysThreadTools::Sys_SignalDestroy(Handle); };
	/* Raise an event. (Bool = true). Repeated raising is OK. */
	void	Raise() noexcept { cweeSysThreadTools::Sys_SignalRaise(Handle); }
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() noexcept { cweeSysThreadTools::Sys_SignalClear(Handle); }
	/* Waits till the event was called. (Bool == true ? return : continue). Waiting on a cweeSysSignal will put a thread to sleep until the thread is awoken by the Raise() from another thread.
	Wait returns true if the object is in a signalled state and returns false if the wait timed out. Wait also clears the signalled state when the signalled state is reached within the time out period.*/
	bool	Wait(int timeout = cweeSysSignalImpl::WAIT_INFINITE) noexcept { return cweeSysThreadTools::Sys_SignalWait(Handle, timeout); }

	cweeSysSignalImpl(const cweeSysSignalImpl&) = delete;
	cweeSysSignalImpl(cweeSysSignalImpl&&) = delete;
	cweeSysSignalImpl& operator=(cweeSysSignalImpl const&) = delete;
	cweeSysSignalImpl& operator=(cweeSysSignalImpl&&) = delete;

protected:
	signalHandle_t		Handle;
};
class cweeSysSignal {
public:
	static constexpr int	WAIT_INFINITE = -1;
	cweeSysSignal(bool manualReset = false) : Handle(new cweeSysSignalImpl(manualReset)) {};
	cweeSysSignal(const cweeSysSignal& other) : Handle(other.Handle) {};
	cweeSysSignal(cweeSysSignal&& other) : Handle(other.Handle) {};
	cweeSysSignal& operator=(const cweeSysSignal& s) { Handle = s.Handle; return *this; };
	cweeSysSignal& operator=(cweeSysSignal&& s) { Handle = s.Handle; return *this; };
	~cweeSysSignal() {};
	/* Raise an event. (Bool = true). Repeated raising is OK. */
	void	Raise() noexcept {
		cweeSharedPtr<cweeSysSignalImpl> h = Handle;
		h->Raise();
	};
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() noexcept {
		cweeSharedPtr<cweeSysSignalImpl> h = Handle;
		h->Clear();
	};
	/* Waits till the event was called. (Bool == true ? return : continue). Waiting on a cweeSysSignal will put a thread to sleep until the thread is awoken by the Raise() from another thread.
	Wait returns true if the object is in a signalled state and returns false if the wait timed out. Wait also clears the signalled state when the signalled state is reached within the time out period.*/
	bool	Wait(int timeout = cweeSysSignal::WAIT_INFINITE) noexcept {
		cweeSharedPtr<cweeSysSignalImpl> h = Handle;
		AUTO p = h.Get();
		return p->Wait(timeout);
	};
protected:
	mutable cweeSharedPtr<cweeSysSignalImpl> Handle;
};