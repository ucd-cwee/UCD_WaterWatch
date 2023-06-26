#pragma hdrstop
#include "precompiled.h"

/*
========================
AssertFailed
========================
*/
bool AssertFailed(const char* file, int line, const char* expression) {	
	static volatile bool skipAllAssertions = false; // Set this to true to skip ALL assertions, including ones YOU CAUSE!
	if (skipAllAssertions) {
		return false;
	}
	return true;
}

