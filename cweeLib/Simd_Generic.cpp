/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/
#include "precompiled.h"
#pragma hdrstop

#include "Simd_Generic.h"

//===============================================================
//
//	Generic implementation of cweeSIMDProcessor
//
//===============================================================

#define UNROLL1(Y) { int _IX; for (_IX=0;_IX<count;_IX++) {Y(_IX);} }
#define UNROLL2(Y) { int _IX, _NM = count&0xfffffffe; for (_IX=0;_IX<_NM;_IX+=2){Y(_IX+0);Y(_IX+1);} if (_IX < count) {Y(_IX);}}
#define UNROLL4(Y) { int _IX, _NM = count&0xfffffffc; for (_IX=0;_IX<_NM;_IX+=4){Y(_IX+0);Y(_IX+1);Y(_IX+2);Y(_IX+3);}for(;_IX<count;_IX++){Y(_IX);}}
#define UNROLL8(Y) { int _IX, _NM = count&0xfffffff8; for (_IX=0;_IX<_NM;_IX+=8){Y(_IX+0);Y(_IX+1);Y(_IX+2);Y(_IX+3);Y(_IX+4);Y(_IX+5);Y(_IX+6);Y(_IX+7);} _NM = count&0xfffffffe; for(;_IX<_NM;_IX+=2){Y(_IX); Y(_IX+1);} if (_IX < count) {Y(_IX);} }

#define NODEFAULT	default: __assume(0)

/*
============
cweeSIMD_Generic::GetName
============
*/
const char* cweeSIMD_Generic::GetName(void) const {
	return "generic code";
}

/*
============
cweeSIMD_Generic::Add

  dst[i] = constant + src[i];
============
*/
void VPCALL cweeSIMD_Generic::Add(float* dst, const float constant, const float* src, const int count) {
#define OPER(X) dst[(X)] = src[(X)] + constant;
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Add

  dst[i] = src0[i] + src1[i];
============
*/
void VPCALL cweeSIMD_Generic::Add(float* dst, const float* src0, const float* src1, const int count) {
#define OPER(X) dst[(X)] = src0[(X)] + src1[(X)];
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Sub

  dst[i] = constant - src[i];
============
*/
void VPCALL cweeSIMD_Generic::Sub(float* dst, const float constant, const float* src, const int count) {
	double c = constant;
#define OPER(X) dst[(X)] = c - src[(X)];
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Sub

  dst[i] = src0[i] - src1[i];
============
*/
void VPCALL cweeSIMD_Generic::Sub(float* dst, const float* src0, const float* src1, const int count) {
#define OPER(X) dst[(X)] = src0[(X)] - src1[(X)];
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Mul

  dst[i] = constant * src[i];
============
*/
void VPCALL cweeSIMD_Generic::Mul(float* dst, const float constant, const float* src0, const int count) {
	double c = constant;
#define OPER(X) (dst[(X)] = (c * src0[(X)]))
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Mul

  dst[i] = src0[i] * src1[i];
============
*/
void VPCALL cweeSIMD_Generic::Mul(float* dst, const float* src0, const float* src1, const int count) {
#define OPER(X) (dst[(X)] = src0[(X)] * src1[(X)])
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Div

  dst[i] = constant / divisor[i];
============
*/
void VPCALL cweeSIMD_Generic::Div(float* dst, const float constant, const float* divisor, const int count) {
	double c = constant;
#define OPER(X) (dst[(X)] = (c / divisor[(X)]))
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Div

  dst[i] = src0[i] / src1[i];
============
*/
void VPCALL cweeSIMD_Generic::Div(float* dst, const float* src0, const float* src1, const int count) {
#define OPER(X) (dst[(X)] = src0[(X)] / src1[(X)])
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::MulAdd

  dst[i] += constant * src[i];
============
*/
void VPCALL cweeSIMD_Generic::MulAdd(float* dst, const float constant, const float* src, const int count) {
	double c = constant;
#define OPER(X) (dst[(X)] += c * src[(X)])
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::MulAdd

  dst[i] += src0[i] * src1[i];
============
*/
void VPCALL cweeSIMD_Generic::MulAdd(float* dst, const float* src0, const float* src1, const int count) {
#define OPER(X) (dst[(X)] += src0[(X)] * src1[(X)])
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::MulSub

  dst[i] -= constant * src[i];
============
*/
void VPCALL cweeSIMD_Generic::MulSub(float* dst, const float constant, const float* src, const int count) {
	double c = constant;
#define OPER(X) (dst[(X)] -= c * src[(X)])
	UNROLL4(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::MulSub

  dst[i] -= src0[i] * src1[i];
============
*/
void VPCALL cweeSIMD_Generic::MulSub(float* dst, const float* src0, const float* src1, const int count) {
#define OPER(X) (dst[(X)] -= src0[(X)] * src1[(X)])
	UNROLL4(OPER)
#undef OPER
}

/*
================
cweeSIMD_Generic::Memcpy
================
*/
void VPCALL cweeSIMD_Generic::Memcpy(void* dst, const void* src, const int count) {
	memcpy(dst, src, count);
}

/*
================
cweeSIMD_Generic::Memset
================
*/
void VPCALL cweeSIMD_Generic::Memset(void* dst, const int val, const int count) {
	memset(dst, val, count);
}

/*
============
cweeSIMD_Generic::Zero16
============
*/
void VPCALL cweeSIMD_Generic::Zero16(float* dst, const int count) {
	memset(dst, 0, count * sizeof(float));
}

/*
============
cweeSIMD_Generic::Negate16
============
*/
void VPCALL cweeSIMD_Generic::Negate16(float* dst, const int count) {
	unsigned int* ptr = reinterpret_cast<unsigned int*>(dst);
#define OPER(X) ptr[(X)] ^= ( 1 << 31 )		// IEEE 32 bits float sign bit
	UNROLL1(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Copy16
============
*/
void VPCALL cweeSIMD_Generic::Copy16(float* dst, const float* src, const int count) {
#define OPER(X) dst[(X)] = src[(X)]
	UNROLL1(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Add16
============
*/
void VPCALL cweeSIMD_Generic::Add16(float* dst, const float* src1, const float* src2, const int count) {
#define OPER(X) dst[(X)] = src1[(X)] + src2[(X)]
	UNROLL1(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Sub16
============
*/
void VPCALL cweeSIMD_Generic::Sub16(float* dst, const float* src1, const float* src2, const int count) {
#define OPER(X) dst[(X)] = src1[(X)] - src2[(X)]
	UNROLL1(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::Mul16
============
*/
void VPCALL cweeSIMD_Generic::Mul16(float* dst, const float* src1, const float constant, const int count) {
#define OPER(X) dst[(X)] = src1[(X)] * constant
	UNROLL1(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::AddAssign16
============
*/
void VPCALL cweeSIMD_Generic::AddAssign16(float* dst, const float* src, const int count) {
#define OPER(X) dst[(X)] += src[(X)]
	UNROLL1(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::SubAssign16
============
*/
void VPCALL cweeSIMD_Generic::SubAssign16(float* dst, const float* src, const int count) {
#define OPER(X) dst[(X)] -= src[(X)]
	UNROLL1(OPER)
#undef OPER
}

/*
============
cweeSIMD_Generic::MulAssign16
============
*/
void VPCALL cweeSIMD_Generic::MulAssign16(float* dst, const float constant, const int count) {
#define OPER(X) dst[(X)] *= constant
	UNROLL1(OPER)
#undef OPER
}
