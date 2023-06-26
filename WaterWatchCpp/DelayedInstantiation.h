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

template<typename T> class DelayedInstantiation {
public:
	constexpr DelayedInstantiation() : obj(nullptr) {};
	~DelayedInstantiation() {};

	T* operator->() { Ensure(); return obj.Get(); };
	T& operator*() { Ensure(); return *obj; };

private:
	void Ensure() {
		AUTO g = obj.Guard();
		auto* p = obj.UnsafeGet();
		if (!p) {
			obj.UnsafeSet(new T());
		}		
	};
	cweeSharedPtr<T> obj;

};