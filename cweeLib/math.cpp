/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/

#include "../cweeLib/precompiled.h"

#pragma hdrstop
float const		cweeMath::AngRad = 0.017453292519943299f;
float const		cweeMath::G = 9.81f;
float const		cweeMath::PI = 3.14159265358979323846f;
float const		cweeMath::TWO_PI = 2.0f * PI;
float const		cweeMath::HALF_PI = 0.5f * PI;
float const		cweeMath::ONEFOURTH_PI = 0.25f * PI;
float const		cweeMath::E = 2.71828182845904523536f;
float const		cweeMath::SQRT_TWO = 1.41421356237309504880f;
float const		cweeMath::SQRT_THREE = 1.73205080756887729352f;
float const		cweeMath::SQRT_1OVER2 = 0.70710678118654752440f;
float const		cweeMath::SQRT_1OVER3 = 0.57735026918962576450f;
float const		cweeMath::EPSILON = 1.192092896e-07f;
float const		cweeMath::INF = 1e30f;
dword			cweeMath::iSqrt[SQRT_TABLE_SIZE];		// inverse square root lookup table

void cweeMath::Init(void) {
	// Build out the iSqrt table for look-up
	union _flint fi, fo;
	for (int i = 0; i < SQRT_TABLE_SIZE; i++) {
		fi.i = ((EXP_BIAS - 1) << EXP_POS) | (i << LOOKUP_POS);
		fo.f = (float)cweeMath::RSqrt((fi.f));
		cweeMath::iSqrt[i] = ((dword)(((fo.i + (1 << (SEED_POS - 2))) >> SEED_POS) & 0xFF)) << SEED_POS;
	}
	cweeMath::iSqrt[SQRT_TABLE_SIZE / 2] = ((dword)(0xFF)) << (SEED_POS);
}

