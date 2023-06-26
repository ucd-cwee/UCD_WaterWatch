#pragma once
#include "Precompiled.h"

class DispatchTimer {
public:
	class DispatchTimerImpl {
	public:
		DispatchTimerImpl(u64 millisecondsBetweenDispatch, cweeJob const& queuedActivity) : handle(), stop(new cweeSysInterlockedInteger(0)) {
			cweeUnion< u64, cweeJob, cweeSharedPtr<cweeSysInterlockedInteger>>* data = new cweeUnion<u64, cweeJob, cweeSharedPtr<cweeSysInterlockedInteger>>(millisecondsBetweenDispatch, queuedActivity, stop);
			handle = cweeSysThreadTools::Sys_CreateThread((xthread_t)([](void* _anon_ptr) -> unsigned int {
				if (_anon_ptr != nullptr) {
					cweeUnion< u64, cweeJob, cweeSharedPtr<cweeSysInterlockedInteger>>* T = static_cast<cweeUnion< u64, cweeJob, cweeSharedPtr<cweeSysInterlockedInteger>>*>(_anon_ptr);
					while (1) {
						if (T->get<2>()->GetValue() >= 1) { break; }

						::Sleep(T->get<0>());

						if (T->get<2>()->GetValue() >= 1) { break; }

						T->get<1>().ForceInvoke();
					}
					delete T;
				}
				return 0;
				}), (void*)data, 1024);
		};
		~DispatchTimerImpl() { stop->Increment(); cweeSysThreadTools::Sys_DestroyThread(handle); };

	private:
		cweeSharedPtr<cweeSysInterlockedInteger> stop;
		uintptr_t handle;

	};

	DispatchTimer() : data(nullptr) {};
	DispatchTimer(u64 millisecondsBetweenDispatch, cweeJob const& queuedActivity) : data(make_cwee_shared<DispatchTimerImpl>(millisecondsBetweenDispatch, queuedActivity)) {};
	~DispatchTimer() {};

private:
	cweeSharedPtr<DispatchTimerImpl> data;

};