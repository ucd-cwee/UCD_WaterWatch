#include "../cweelib/precompiled.h"
#pragma hdrstop

/*
================================================================================================
	Signal
================================================================================================
*/
void Sys_SignalCreate(signalHandle_t& handle, bool manualReset) {
	handle = CreateEvent(NULL, manualReset, FALSE, NULL);
}
void Sys_SignalDestroy(signalHandle_t& handle) {
	CloseHandle(handle);
}
void Sys_SignalRaise(signalHandle_t& handle) {
	SetEvent(handle);
}
void Sys_SignalClear(signalHandle_t& handle) {
	// events are created as auto-reset so this should never be needed
	ResetEvent(handle);
}
bool Sys_SignalWait(signalHandle_t& handle, int timeout) {
	DWORD result = WaitForSingleObject(handle, timeout == cweeSysSignal::WAIT_INFINITE ? INFINITE : timeout);
	assert(result == WAIT_OBJECT_0 || (timeout != cweeSysSignal::WAIT_INFINITE && result == WAIT_TIMEOUT));
	return (result == WAIT_OBJECT_0);
}

/*
================================================================================================
	Mutex
================================================================================================*/
void Sys_MutexCreate(mutexHandle_t& handle) {
	InitializeCriticalSection(&handle);
}
void Sys_MutexDestroy(mutexHandle_t& handle) {
	DeleteCriticalSection(&handle);
}
bool Sys_MutexLock(mutexHandle_t& handle, bool blocking) {
	try {
		if (TryEnterCriticalSection(&handle) == 0) {
			if (!blocking) {
				return false;
			}

			try {
				EnterCriticalSection(&handle);
			}
			catch (...) {
				LeaveCriticalSection(&handle);
				return false;
			}
		}
		return true;
	}
	catch (...) {	
		return false;
	}
}
void Sys_MutexUnlock(mutexHandle_t& handle) {
	LeaveCriticalSection(&handle);
}

cweeSysThread::cweeSysThread() :
	name("newThread"),
	threadHandle(0),
	isWorker(false),
	isRunning(false),
	isTerminating(false),
	moreWorkToDo(false),
	signalWorkerDone(true) {
}

cweeSysThread::~cweeSysThread() {
	StopThread(true);
	if (threadHandle) {
		Sys_DestroyThread(threadHandle);
	}
}

bool cweeSysThread::StartThread(const char* name_, core_t core, xthreadPriority priority, int stackSize) {
	if (isRunning) {
		return false;
	}
	name = name_;
	isTerminating = false;
	if (threadHandle) {
		Sys_DestroyThread(threadHandle);
	}
	threadHandle = Sys_CreateThread((xthread_t)ThreadProc, this, priority, name, core, stackSize, false);
	isRunning = true;
	return true;
}

bool cweeSysThread::StartWorkerThread(const char* name_, core_t core, xthreadPriority priority, int stackSize) {
	if (isRunning) {
		return false;
	}
	isWorker = true;
	bool result = StartThread(name_, core, priority, stackSize);
	signalWorkerDone.Wait(cweeSysSignal::WAIT_INFINITE);
	return result;
}

void cweeSysThread::StopThread(bool wait) {
	if (!isRunning) {
		return;
	}
	if (isWorker) {
		signalMutex.Lock();
		moreWorkToDo = true;
		signalWorkerDone.Clear();
		isTerminating = true;
		signalMoreWorkToDo.Raise();
		signalMutex.Unlock();
	}
	else {
	isTerminating = true;
	}
	if (wait) {
		WaitForThread();
	}
}

void cweeSysThread::WaitForThread() {
	if (isWorker) {
		signalWorkerDone.Wait(cweeSysSignal::WAIT_INFINITE);
	}
	else if (isRunning) {
		Sys_DestroyThread(threadHandle);
		threadHandle = 0;
	}
}

bool cweeSysThread::IsWorkDone() {
	if (isWorker) {
		// a timeout of 0 will return immediately with true if signaled
		if (signalWorkerDone.Wait(0)) {
			return true;
		}
	}
	return false;
}

void cweeSysThread::SignalWork() {
	if (isWorker) {
		signalMutex.Lock();
		moreWorkToDo = true;
		signalWorkerDone.Clear();
		signalMoreWorkToDo.Raise();
		signalMutex.Unlock();
	}
}

int cweeSysThread::ThreadProc(cweeSysThread* thread) {
	int retVal = 0;
		try {
			if (thread->isWorker) {
				for (; ; ) {
					thread->signalMutex.Lock();
					if (thread->moreWorkToDo) {
						thread->moreWorkToDo = false;
						thread->signalMoreWorkToDo.Clear();
						thread->signalMutex.Unlock();
					}
					else {
						thread->signalWorkerDone.Raise();
						thread->signalMutex.Unlock();
						thread->signalMoreWorkToDo.Wait(cweeSysSignal::WAIT_INFINITE);
						continue;
					}

					if (thread->isTerminating) {
						break;
					}

					retVal = thread->Run();
				}
				thread->signalWorkerDone.Raise();
			}else {
				retVal = thread->Run();
			}
		}
		catch (...) {
		}
	thread->isRunning = false;
	return retVal;
}

int cweeSysThread::Run() {
	// The Run() is not pure virtual because on destruction of a derived class
	// the virtual function pointer will be set to NULL before the cweeSysThread
	// destructor actually stops the thread.
	return 0;
}