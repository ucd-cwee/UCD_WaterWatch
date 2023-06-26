#pragma once
#include "Precompiled.h"

static cweeSharedPtr<int> thing = make_cwee_shared<int>(); // ALLOWED GLOBAL OBJ ("STATIC") IN INSTANTIATED FILES OUTSIDE OF CLASSES
class ImplTest {
public:
	void Init();
	void Shutdown();
};
class ImplTest2 {
public:
	void Init2();
	void Shutdown2();

	template <typename T = int>
	void Function2() { 
		/*
		Templated functions must be defined in the header file with all associated code available.
		I.e. C++ Cannot support delayed instantiation of template functions. There are hacks available (Using "INLINE" to force it) but can be complex and result in some strange comiling behavior. 
		Recommended that if a template is required, somehow isolate it and define the entire interface for it in a header-only file. 
		*/ 
	};
};