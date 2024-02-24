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
#include "SharedPtr.h"

template<typename T > class DelayedInstantiation {
public:
	DelayedInstantiation() : obj(nullptr), createFunc() {};
	DelayedInstantiation(std::function<T*()> create) : obj(nullptr), createFunc(create) {};
	~DelayedInstantiation() {};

	//T* operator->() { Ensure(); return obj.Get(); };
	T* operator->() { Ensure(); return obj.get(); };
	T& operator*() { Ensure(); return *obj; };

	void lock() {
		Lock.lock();
	};
	void unlock() {
		Lock.unlock();
	};
private:
	void Ensure() {
		if (!obj.get()) {
			Lock.lock();
			auto* p = obj.get();
			if (!p) {
				obj = std::shared_ptr<T>(createFunc());
			}
			Lock.unlock();
		}
	};
	std::shared_ptr<T> obj;
	cweeConstexprLock Lock;
	std::function<T*()> createFunc;
};