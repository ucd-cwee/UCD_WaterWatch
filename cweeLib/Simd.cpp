/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "Simd_Generic.h"


cweeSIMDProcessor*		processor			= NULL;				// pointer to SIMD processor
cweeSIMDProcessor*		generic				= NULL;				// pointer to generic SIMD implementation
cweeSIMDProcessor*		SIMDProcessor		= NULL;

/*
================
cweeSIMD::Init
================
*/
void cweeSIMD::Init(void) {
	generic = new cweeSIMD_Generic;
	generic->cpuid = CPUID_GENERIC;
	processor = NULL;
	SIMDProcessor = generic;
}

/*
================
cweeSIMD::Shutdown
================
*/
void cweeSIMD::Shutdown(void) {
	if (processor != generic) {
		delete processor;
	}
	delete generic;
	generic = NULL;
	processor = NULL;
	SIMDProcessor = NULL;
}