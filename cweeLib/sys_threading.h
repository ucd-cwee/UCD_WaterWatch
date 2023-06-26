
#ifndef __SYS_THREADING_H__
#define __SYS_THREADING_H__

typedef CRITICAL_SECTION		mutexHandle_t;
typedef HANDLE					signalHandle_t;
typedef LONG					interlockedInt_t;

enum core_t {
	CORE_ANY = -1,
	CORE_0A,
	CORE_0B,
	CORE_1A,
	CORE_1B,
	CORE_2A,
	CORE_2B
};

typedef unsigned int (*xthread_t)(void*);

enum xthreadPriority {
	THREAD_LOWEST,
	THREAD_BELOW_NORMAL,
	THREAD_NORMAL,
	THREAD_ABOVE_NORMAL,
	THREAD_HIGHEST
};

#define DEFAULT_THREAD_STACK_SIZE		( 256 * 1024 )
uintptr_t			Sys_GetCurrentThreadID();
uintptr_t			Sys_CreateThread(xthread_t function, void* parms, xthreadPriority priority,	const char* name, core_t core, int stackSize = DEFAULT_THREAD_STACK_SIZE,	bool suspended = false);
void				Sys_WaitForThread(uintptr_t threadHandle);
void				Sys_WaitForThread( uintptr_t threadHandle );
void				Sys_ForgetThread(uintptr_t threadHandle);
void				Sys_DestroyThread(uintptr_t threadHandle);

void				Sys_SignalCreate(signalHandle_t& handle, bool manualReset);
void				Sys_SignalDestroy(signalHandle_t& handle);
void				Sys_SignalRaise(signalHandle_t& handle);
void				Sys_SignalClear(signalHandle_t& handle);
bool				Sys_SignalWait(signalHandle_t& handle, int timeout);

void				Sys_MutexCreate(mutexHandle_t& handle);
void				Sys_MutexDestroy(mutexHandle_t& handle);
bool				Sys_MutexLock(mutexHandle_t& handle, bool blocking);
void				Sys_MutexUnlock(mutexHandle_t& handle);

interlockedInt_t	Sys_InterlockedIncrement(interlockedInt_t& value);
interlockedInt_t	Sys_InterlockedDecrement(interlockedInt_t& value);

interlockedInt_t	Sys_InterlockedAdd(interlockedInt_t& value, interlockedInt_t i);
interlockedInt_t	Sys_InterlockedSub(interlockedInt_t& value, interlockedInt_t i);

interlockedInt_t	Sys_InterlockedExchange(interlockedInt_t& value, interlockedInt_t exchange);
interlockedInt_t	Sys_InterlockedCompareExchange(interlockedInt_t& value, interlockedInt_t comparand, interlockedInt_t exchange);

template <typename type> type Sys_InterlockedIncrementV(type& value);
template <typename type> type Sys_InterlockedDecrementV(type& value);
template <typename type> type Sys_InterlockedAddV(type& value, const type& i);
template <typename type> type Sys_InterlockedSubV(type& value, const type& i);
template <typename type> type Sys_InterlockedExchangeV(type& value, const type& exchange);
template <typename type> type Sys_InterlockedCompareExchangeV(type& value, const type& comparand, const type& exchange);

void				Sys_Yield();
bool				Sys_TryYield();

void* Sys_InterlockedExchangePointer(void*& ptr, void* exchange);
void* Sys_InterlockedCompareExchangePointer(void*& ptr, void* comparand, void* exchange);

#endif
