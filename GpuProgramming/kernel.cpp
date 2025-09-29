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

	global atomic_int __rand_counter = ATOMIC_VAR_INIT(123456789);
	uint __rand() {
		uint x, y;
		for (;;) {
			x = y = atomic_load(&__rand_counter);
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 5;
			if (atom_cmpxchg(&__rand_counter, y, x) == y) break;
		}
		return x;
	};

	kernel void transpose(global _type_* destination, global _type_* RHS, global uint* lengths) {
		const uint n = get_global_id(0);
		const uint Z = n / (lengths[4] * lengths[3]);
		const uint pos2 = n - Z * (lengths[4] * lengths[3]);
		const uint Y = pos2 / lengths[3];
		const uint X = pos2 - Y * lengths[3];

		destination[n] = RHS[(Z * lengths[1] * lengths[0]) + (X * lengths[0]) + Y];
	};
	kernel void make_square(global _type_* destination, global _type_* RHS, global uint* lengths) {
		const uint n = get_global_id(0);
		const uint Z = n / (lengths[4] * lengths[3]);
		const uint pos2 = n - Z * (lengths[4] * lengths[3]);
		const uint Y = pos2 / lengths[3];
		const uint X = pos2 - Y * lengths[3];

		if ((X < lengths[0]) && (Y < lengths[1]) && (Z < lengths[2])) {
			destination[n] = RHS[(Z * lengths[1] * lengths[0]) + (Y * lengths[0]) + X];
		}
		else {
			destination[n] = 0;
		}		
	};

	kernel void join_dim_0(global _type_* destination, global _type_* LHS, global _type_* RHS, global uint* lengths) {
		const uint n = get_global_id(0);
		const uint Z = n / (lengths[7] * lengths[6]);
		const uint pos2 = n - Z * (lengths[7] * lengths[6]);
		const uint Y = pos2 / lengths[6];
		uint X = pos2 - Y * lengths[6];

		if ((X < lengths[0])) {
			destination[n] = LHS[(Z * lengths[1] * lengths[0]) + (Y * lengths[0]) + X];
		}
		else {
			X -= lengths[0];
			destination[n] = RHS[(Z * lengths[4] * lengths[3]) + (Y * lengths[3]) + X];
		}
	}
	kernel void join_dim_1(global _type_* destination, global _type_* LHS, global _type_* RHS, global uint* lengths) {
		const uint n = get_global_id(0);
		const uint Z = n / (lengths[7] * lengths[6]);
		const uint pos2 = n - Z * (lengths[7] * lengths[6]);
		uint Y = pos2 / lengths[6];
		const uint X = pos2 - Y * lengths[6];

		if (Y < lengths[1]) {
			destination[n] = LHS[(Z * lengths[1] * lengths[0]) + (Y * lengths[0]) + X];
		}
		else {
			Y -= lengths[1];
			destination[n] = RHS[(Z * lengths[4] * lengths[3]) + (Y * lengths[3]) + X];
		}
	}
	kernel void join_dim_2(global _type_* destination, global _type_* LHS, global _type_* RHS, global uint* lengths) {
		const uint n = get_global_id(0);
		uint Z = n / (lengths[7] * lengths[6]);
		const uint pos2 = n - Z * (lengths[7] * lengths[6]);
		const uint Y = pos2 / lengths[6];
		const uint X = pos2 - Y * lengths[6];

		if ((Z < lengths[2])) {
			destination[n] = LHS[(Z * lengths[1] * lengths[0]) + (Y * lengths[0]) + X];
		}
		else {
			Z -= lengths[2];
			destination[n] = RHS[(Z * lengths[4] * lengths[3]) + (Y * lengths[3]) + X];
		}
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
	kernel void Max(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = fmax(A[n], B[n]);
	}
	kernel void Max_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = fmax(A[n], B);
	}
	kernel void Min(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = fmin(A[n], B[n]);
	}
	kernel void Min_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = fmin(A[n], B);
	}
	kernel void Lgamma(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = lgamma(A[n]);
	}
	kernel void Log(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log(A[n]);
	}
	kernel void Log2(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log2(A[n]);
	}
	kernel void Log10(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log10(A[n]);
	}
	kernel void Log1p(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log1p(A[n]);
	}
	kernel void Rand(global _type_* A) {
		const uint n = get_global_id(0);
		A[n] = (_type_)__rand() / ((uint)~((uint)0));
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
	kernel void Max(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = max(A[n], B[n]);
	}
	kernel void Max_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = max(A[n], B);
	}
	kernel void Min(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = min(A[n], B[n]);
	}
	kernel void Min_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = min(A[n], B);
	}
	kernel void Lgamma(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = lgamma((float)A[n]);
	}
	kernel void Log(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log((float)A[n]);
	}
	kernel void Log2(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log2((float)A[n]);
	}
	kernel void Log10(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log10((float)A[n]);
	}
	kernel void Log1p(global _type_* A, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = log1p((float)A[n]);
	}
	kernel void Rand(global _type_* A) {
		const uint n = get_global_id(0);
		A[n] = __rand();
	}

));
}
return out.replace("_type_", GL::string(typeName)).to_string();
} // ############################################################### end of OpenCL C code #####################################################################