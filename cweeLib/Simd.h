/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/
#ifndef __MATH_SIMD_H__
#define __MATH_SIMD_H__

/*
===============================================================================
	Single Instruction Multiple Data (SIMD)
	For optimal use data should be aligned on a 16 byte boundary.
	All cweeSIMDProcessor routines are thread safe.
===============================================================================
*/

class cweeSIMD {
public:
	static void			Init(void);
	static void			InitProcessor(const char* module, bool forceGeneric);
	static void			Shutdown(void);
};

#define VPCALL __fastcall
//class cweeVec3;
class vec3;
const int MIXBUFFER_SAMPLES = 4096;


class cweeSIMDProcessor {
public:
	cweeSIMDProcessor(void) { cpuid = CPUID_NONE; }

	cpuid_t							cpuid;

	virtual const char* VPCALL		GetName(void) const = 0;

	virtual void VPCALL				Add(float* dst, const float constant, const float* src, const int count) = 0;
	virtual void VPCALL				Add(float* dst, const float* src0, const float* src1, const int count) = 0;
	virtual void VPCALL				Sub(float* dst, const float constant, const float* src, const int count) = 0;
	virtual void VPCALL				Sub(float* dst, const float* src0, const float* src1, const int count) = 0;
	virtual void VPCALL				Mul(float* dst, const float constant, const float* src, const int count) = 0;
	virtual void VPCALL				Mul(float* dst, const float* src0, const float* src1, const int count) = 0;
	virtual void VPCALL				Div(float* dst, const float constant, const float* src, const int count) = 0;
	virtual void VPCALL				Div(float* dst, const float* src0, const float* src1, const int count) = 0;
	virtual void VPCALL				MulAdd(float* dst, const float constant, const float* src, const int count) = 0;
	virtual void VPCALL				MulAdd(float* dst, const float* src0, const float* src1, const int count) = 0;
	virtual void VPCALL				MulSub(float* dst, const float constant, const float* src, const int count) = 0;
	virtual void VPCALL				MulSub(float* dst, const float* src0, const float* src1, const int count) = 0;

	virtual void VPCALL				Memcpy(void* dst, const void* src, const int count) = 0;
	virtual void VPCALL				Memset(void* dst, const int val, const int count) = 0;

	// these assume 16 byte aligned and 16 byte padded memory
	virtual void VPCALL				Zero16(float* dst, const int count) = 0;
	virtual void VPCALL				Negate16(float* dst, const int count) = 0;
	virtual void VPCALL				Copy16(float* dst, const float* src, const int count) = 0;
	virtual void VPCALL				Add16(float* dst, const float* src1, const float* src2, const int count) = 0;
	virtual void VPCALL				Sub16(float* dst, const float* src1, const float* src2, const int count) = 0;
	virtual void VPCALL				Mul16(float* dst, const float* src1, const float constant, const int count) = 0;
	virtual void VPCALL				AddAssign16(float* dst, const float* src, const int count) = 0;
	virtual void VPCALL				SubAssign16(float* dst, const float* src, const int count) = 0;
	virtual void VPCALL				MulAssign16(float* dst, const float constant, const int count) = 0;
};

// pointer to SIMD processor
extern cweeSIMDProcessor* SIMDProcessor;

#endif