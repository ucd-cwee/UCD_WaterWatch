#ifndef DelayedInstantiation_H
#define DelayedInstantiation_H

#pragma region "INCLUDES"
#pragma hdrstop
#include "Precompiled.h"
#pragma endregion

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


#endif