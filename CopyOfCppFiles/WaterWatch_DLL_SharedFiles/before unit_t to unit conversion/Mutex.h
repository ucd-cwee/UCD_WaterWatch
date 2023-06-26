#ifndef __CWEE_MUTEX_H__
#define __CWEE_MUTEX_H__

#pragma hdrstop
#include "Precompiled.h"

/* MUTEX COLLECTION */
#if 1
class cweeSysMutex {
public:
	using Handle_t = cweeSysMutexImpl;
	using Phandle = cweeSharedPtr<Handle_t>;
	
	class cweeSysMutexLifetimeGuard {
	public:
		constexpr explicit operator bool() const { return (bool)owner; };
		explicit cweeSysMutexLifetimeGuard(const Phandle& mut) noexcept : owner(mut) { owner->Lock(); };
		~cweeSysMutexLifetimeGuard() noexcept { owner->Unlock(); };
		explicit cweeSysMutexLifetimeGuard() = delete;
		explicit cweeSysMutexLifetimeGuard(const cweeSysMutexLifetimeGuard& other) = delete;
		explicit cweeSysMutexLifetimeGuard(cweeSysMutexLifetimeGuard&& other) = delete;
		cweeSysMutexLifetimeGuard& operator=(const cweeSysMutexLifetimeGuard&) = delete;
		cweeSysMutexLifetimeGuard& operator=(cweeSysMutexLifetimeGuard&&) = delete;
	protected:
		Phandle owner;
	};

public:
	cweeSysMutex() : Handle(new Handle_t()) {};
	cweeSysMutex(const cweeSysMutex& other) : Handle(new Handle_t()) {};
	cweeSysMutex(cweeSysMutex&& other) : Handle(new Handle_t()) {};
	cweeSysMutex& operator=(const cweeSysMutex& s) { return *this; };
	cweeSysMutex& operator=(cweeSysMutex&& s) { return *this; };
	~cweeSysMutex() {};

	NODISCARD cweeSysMutexLifetimeGuard	Guard() const { return cweeSysMutexLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const { return Handle->Lock(blocking); };
	void			Unlock() const { Handle->Unlock(); };

protected:
	mutable Phandle Handle;

};
#else
class cweeSysMutex {
public:
	using Handle_t = cweeSysMutexImpl;
	using Phandle = std::shared_ptr<Handle_t>;

	class cweeSysMutexLifetimeGuard {
	public:
		constexpr explicit operator bool() const { return true; };
		explicit cweeSysMutexLifetimeGuard(const Phandle& mut) noexcept : owner(mut) { owner->Lock(); };
		~cweeSysMutexLifetimeGuard() noexcept { owner->Unlock(); };
		explicit cweeSysMutexLifetimeGuard() = delete;
		explicit cweeSysMutexLifetimeGuard(const cweeSysMutexLifetimeGuard& other) = delete;
		explicit cweeSysMutexLifetimeGuard(cweeSysMutexLifetimeGuard&& other) = delete;
		cweeSysMutexLifetimeGuard& operator=(const cweeSysMutexLifetimeGuard&) = delete;
		cweeSysMutexLifetimeGuard& operator=(cweeSysMutexLifetimeGuard&&) = delete;
	protected:
		Phandle owner;
	};

public:
	cweeSysMutex() : Handle(std::make_shared<Handle_t>()) {};
	cweeSysMutex(const cweeSysMutex& other) : Handle(std::make_shared<Handle_t>()) {};
	cweeSysMutex(cweeSysMutex&& other) : Handle(std::make_shared<Handle_t>()) {};
	cweeSysMutex& operator=(const cweeSysMutex& s) { return *this; };
	cweeSysMutex& operator=(cweeSysMutex&& s) { return *this; };
	~cweeSysMutex() {};

	NODISCARD cweeSysMutexLifetimeGuard	Guard() const { return cweeSysMutexLifetimeGuard(Handle); };
	bool			Lock(bool blocking = true) const { return Handle->Lock(blocking); };
	void			Unlock() const { Handle->Unlock(); };

protected:
	mutable Phandle Handle;

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
		return h->Raise();
	};
	/* Clears an event. (Bool = false) Prefer to Wait rather than clear. */
	void	Clear() noexcept {
		cweeSharedPtr<cweeSysSignalImpl> h = Handle;
		return h->Clear();
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

#endif