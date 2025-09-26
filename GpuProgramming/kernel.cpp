#include "strings.hpp"
#include "kernel.hpp" // note: unbalanced round brackets () are not allowed and string literals can't be arbitrarily long, so periodically interrupt with )+R(

string opencl_c_container(string const& typeName, bool floatingPoint) {
// ########################## begin of OpenCL C code ####################################################################
auto out = GL::string(R( 
	kernel void copy(global _type_ * A, global _type_ * B) {
		const uint n = get_global_id(0);
		A[n] = B[n];
	}
    kernel void copy_single(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] = B;
	}

	kernel void add(global _type_ * A, global _type_ * B, global _type_ * C) {
		const uint n = get_global_id(0);
		C[n] = A[n] + B[n];
	}
	kernel void add_inplace(global _type_* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] += B[n];
	}
	kernel void sub(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] - B[n];
	}
	kernel void sub_inplace(global _type_* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] -= B[n];
	}
	kernel void mult(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] * B[n];
	}
	kernel void mult_inplace(global _type_* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] *= B[n];
	}
	kernel void divide(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] / B[n];
	}
	kernel void divide_inplace(global _type_* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] /= B[n];
	}

	kernel void add_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] + B;
	}
	kernel void add_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] += B;
	}
	kernel void sub_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] - B;
	}
	kernel void sub_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] -= B;
	}
	kernel void mult_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] * B;
	}
	kernel void mult_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] *= B;
	}
	kernel void divide_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] / B;
	}
	kernel void divide_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] /= B;
	}

	kernel void from_char(global _type_* A, global char* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	}
	kernel void from_uchar(global _type_* A, global uchar* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	}
	kernel void from_ulong(global _type_* A, global ulong* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	}
	kernel void from_uint(global _type_* A, global uint* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	}
	kernel void from_long(global _type_* A, global long* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	}
	kernel void from_int(global _type_* A, global int* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	}
	kernel void from_float(global _type_* A, global float* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	}

	kernel void item_eq_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] == B) ? 1 : 0;
	}
	kernel void item_neq_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] != B) ? 1 : 0;
	}
	kernel void item_eq(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] == B[n]) ? 1 : 0;
	}
	kernel void item_neq(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] != B[n]) ? 1 : 0;
	}
));
if (floatingPoint) {
out = out + GL::string(R(
	kernel void power_single(global _type_ * A, _type_ B, global _type_ * C) {
		const uint n = get_global_id(0);
		C[n] = pow(A[n], B);
	}
	kernel void power_n_single(global _type_* A, int B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = pown(A[n], B);
	}
	kernel void power(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = pow(A[n], B[n]);
	}
	kernel void power_n(global _type_* A, global int* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = pown(A[n], B[n]);
	}
	kernel void square_root(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = native_sqrt(A[n]);
	}
	kernel void round(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		const _type_ H = ((_type_)1) / (_type_)2;
		C[n] = floor(A[n] + H);
	}
	kernel void flr(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = floor(A[n]);
	}
	kernel void ceil(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = floor(A[n] + 1);
	}
	kernel void mult_add(global _type_* A, global _type_* B, global _type_* C, global _type_* D) {
		const uint n = get_global_id(0);
		D[n] = fma(A[n], B[n], C[n]);
	}
	kernel void absolute(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = fabs(A[n]);
	}
	kernel void Cos(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = cos(A[n]);
	}
	kernel void Sin(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = sin(A[n]);
	}
	kernel void Tan(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = tan(A[n]);
	}
	kernel void aCos(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = acos(A[n]);
	}
	kernel void aSin(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = asin(A[n]);
	}
	kernel void aTan(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = atan(A[n]);
	}
	kernel void Cosh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = cosh(A[n]);
	}
	kernel void Sinh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = sinh(A[n]);
	}
	kernel void Tanh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = tanh(A[n]);
	}
	kernel void aCosh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = acosh(A[n]);
	}
	kernel void aSinh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = asinh(A[n]);
	}
	kernel void aTanh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = atanh(A[n]);
	}
	kernel void Exp(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = exp(A[n]);
	}
	kernel void Exp2(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = exp2(A[n]);
	}
	kernel void Exp10(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = exp10(A[n]);
	}
	kernel void Expm1(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = expm1(A[n]);
	}
	kernel void Mod(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = fmod(A[n], B[n]);
	}
	kernel void Mod_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = fmod(A[n], B);
	}

));
}
else {
out = out + GL::string(R(
	kernel void power_single(global _type_ * A, _type_ B, global _type_ * C) {
		const uint n = get_global_id(0);
		C[n] = pow((float)A[n], (float)B);
	}
	kernel void power_n_single(global _type_* A, int B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = pown((float)A[n], B);
	}
	kernel void power(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = pow((float)A[n], (float)B[n]);
	}
	kernel void power_n(global _type_* A, global int* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = pown((float)A[n], B[n]);
	}
	kernel void square_root(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = native_sqrt((float)A[n]);
	}
	kernel void round(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n];
	}
    kernel void flr(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n];
	}
	kernel void ceil(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n];
	}
	kernel void mult_add(global _type_* A, global _type_* B, global _type_* C, global _type_* D) {
		const uint n = get_global_id(0);
		D[n] = (A[n] * B[n]) + C[n];
	}
	kernel void absolute(global _type_ * A, global _type_ * C) {
		const uint n = get_global_id(0);
		C[n] = abs(A[n]);
	}
	kernel void Cos(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = cos((float)A[n]);
	}
	kernel void Sin(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = sin((float)A[n]);
	}
	kernel void Tan(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = tan((float)A[n]);
	}
	kernel void aCos(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = acos((float)A[n]);
	}
	kernel void aSin(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = asin((float)A[n]);
	}
	kernel void aTan(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = atan((float)A[n]);
	}
	kernel void Cosh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = cosh((float)A[n]);
	}
	kernel void Sinh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = sinh((float)A[n]);
	}
	kernel void Tanh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = tanh((float)A[n]);
	}
	kernel void aCosh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = acosh((float)A[n]);
	}
	kernel void aSinh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = asinh((float)A[n]);
	}
	kernel void aTanh(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = atanh((float)A[n]);
	}
	kernel void Exp(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = exp((float)A[n]);
	}
	kernel void Exp2(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = exp2((float)A[n]);
	}
	kernel void Exp10(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = exp10((float)A[n]);
	}
	kernel void Expm1(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = expm1((float)A[n]);
	}
	kernel void Mod(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] % B[n];
	}
	kernel void Mod_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] % B;
	}

));
}
return out.replace("_type_", GL::string(typeName)).to_string();
} // ############################################################### end of OpenCL C code #####################################################################