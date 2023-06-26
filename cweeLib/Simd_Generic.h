/*
=======================================================================
Energy Demand Management System for Water Distribution Systems
Developed by the Center for Water-Energy Efficiency, 2019 - 2020
=======================================================================
*/
#ifndef __MATH_SIMD_GENERIC_H__
#define __MATH_SIMD_GENERIC_H__



/*
===============================================================================

	Generic implementation of cweeSIMDProcessor

===============================================================================
*/

class cweeSIMD_Generic : public cweeSIMDProcessor {
public:
	virtual const char* VPCALL GetName(void) const;

	virtual void VPCALL Add(float* dst, const float constant, const float* src, const int count);
	virtual void VPCALL Add(float* dst, const float* src0, const float* src1, const int count);
	virtual void VPCALL Sub(float* dst, const float constant, const float* src, const int count);
	virtual void VPCALL Sub(float* dst, const float* src0, const float* src1, const int count);
	virtual void VPCALL Mul(float* dst, const float constant, const float* src, const int count);
	virtual void VPCALL Mul(float* dst, const float* src0, const float* src1, const int count);
	virtual void VPCALL Div(float* dst, const float constant, const float* src, const int count);
	virtual void VPCALL Div(float* dst, const float* src0, const float* src1, const int count);
	virtual void VPCALL MulAdd(float* dst, const float constant, const float* src, const int count);
	virtual void VPCALL MulAdd(float* dst, const float* src0, const float* src1, const int count);
	virtual void VPCALL MulSub(float* dst, const float constant, const float* src, const int count);
	virtual void VPCALL MulSub(float* dst, const float* src0, const float* src1, const int count);

	virtual void VPCALL Memcpy(void* dst, const void* src, const int count);
	virtual void VPCALL Memset(void* dst, const int val, const int count);

	virtual void VPCALL Zero16(float* dst, const int count);
	virtual void VPCALL Negate16(float* dst, const int count);
	virtual void VPCALL Copy16(float* dst, const float* src, const int count);
	virtual void VPCALL Add16(float* dst, const float* src1, const float* src2, const int count);
	virtual void VPCALL Sub16(float* dst, const float* src1, const float* src2, const int count);
	virtual void VPCALL Mul16(float* dst, const float* src1, const float constant, const int count);
	virtual void VPCALL AddAssign16(float* dst, const float* src, const int count);
	virtual void VPCALL SubAssign16(float* dst, const float* src, const int count);
	virtual void VPCALL MulAssign16(float* dst, const float constant, const int count);
};


#endif 