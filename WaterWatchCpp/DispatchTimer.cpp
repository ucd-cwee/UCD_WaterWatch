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
#include "DispatchTimer.h"
#include "InterlockedValues.h"

namespace {
	template<typename T> static cweeSharedPtr<void> cast_to_void(const cweeSharedPtr<T>& data) { return cweeSharedPtr<void>(data, [](void* p) { return p; }); };
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
};

DispatchTimer::DispatchTimer(u64 millisecondsBetweenDispatch, cweeJob const& queuedActivity) : data(cast_to_void(make_cwee_shared<DispatchTimerImpl>(millisecondsBetweenDispatch, queuedActivity))) {};