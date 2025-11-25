#include "../ScriptLanguageTesting/Strings.h"
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
	};
	kernel void sub(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] - B[n];
	};
	kernel void sub_inplace(global _type_* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] -= B[n];
	};
	kernel void mult(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] * B[n];
	};
	kernel void mult_inplace(global _type_* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] *= B[n];
	};
	kernel void divide(global _type_* A, global _type_* B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] / B[n];
	};
	kernel void divide_inplace(global _type_* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] /= B[n];
	};
	kernel void add_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] + B;
	};
	kernel void add_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] += B;
	};
	kernel void sub_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] - B;
	};

	kernel void sub_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] -= B;
	};
	kernel void mult_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] * B;
	};
	kernel void mult_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] *= B;
	};
	kernel void divide_single(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] / B;
	};
	kernel void divide_single_inv(global _type_* A, _type_ B, global _type_* C) {
		const uint n = get_global_id(0);
		C[n] = B / A[n];
	};
	kernel void divide_single_inplace(global _type_* A, _type_ B) {
		const uint n = get_global_id(0);
		A[n] /= B;
	};

	kernel void from_char(global _type_* A, global char* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	};
	kernel void from_uchar(global _type_* A, global uchar* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	};
	kernel void from_ulong(global _type_* A, global ulong* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	};
	kernel void from_uint(global _type_* A, global uint* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	};
	kernel void from_long(global _type_* A, global long* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	};
	kernel void from_int(global _type_* A, global int* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	};
	kernel void from_float(global _type_* A, global float* B) {
		const uint n = get_global_id(0);
		A[n] = (_type_)B[n];
	};

	kernel void item_eq_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] == B) ? 1 : 0;
	};
	kernel void item_neq_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] != B) ? 1 : 0;
	};
	kernel void item_eq(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] == B[n]) ? 1 : 0;
	};
	kernel void item_neq(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] != B[n]) ? 1 : 0;
	};
	kernel void item_not(global uint* A, global _type_* B) {
		const uint n = get_global_id(0);
		A[n] = !B[n];
	};
	kernel void item_ls_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] < B) ? 1 : 0;
	}
	kernel void item_lse_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] <= B) ? 1 : 0;
	}
	kernel void item_ls(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] < B[n]) ? 1 : 0;
	}
	kernel void item_lse(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] <= B[n]) ? 1 : 0;
	}
	kernel void item_gr_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] > B) ? 1 : 0;
	}
	kernel void item_gre_single(global _type_* A, _type_ B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] >= B) ? 1 : 0;
	}
	kernel void item_gr(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] > B[n]) ? 1 : 0;
	}
	kernel void item_gre(global _type_* A, global _type_* B, global uint* C) {
		const uint n = get_global_id(0);
		C[n] = (A[n] >= B[n]) ? 1 : 0;
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

	kernel void Transpose(global _type_* destination, global _type_* RHS, uint lenX, uint lenY) {
		const uint n = get_global_id(0);
		const uint source_X = (uint)floor((float)n / (float)lenY);
		const uint source_Y = n - (lenY * source_X);
		const uint source_N = source_Y * lenX + source_X;
		destination[n] = RHS[source_N];
	};
	kernel void make_square(global _type_* destination, global _type_* RHS, uint RHS_LenX, uint RHS_LenY, uint LenZ, uint Len) {
		const uint n = get_global_id(0);
		const uint Z = (uint)floor((float)n / (float)(Len * Len));
		const uint pos2 = n - Z * (Len * Len);
		const uint Y = (uint)floor((float)pos2 / (float)Len);
		const uint X = pos2 - Y * Len;

		if ((X < RHS_LenX) && (Y < RHS_LenY) && (Z < LenZ)) {
			destination[n] = RHS[(Z * RHS_LenY * RHS_LenX) + (Y * RHS_LenX) + X];
		}
		else {
			destination[n] = (_type_)0;
		}		
	};
	kernel void identity(global _type_* destination, uint LenX) {
		const uint n = get_global_id(0);
		const uint Y = (uint)floor((float)n / (float)LenX);
		const uint X = n - Y * LenX;

		if (X == Y) {
			destination[n] = (_type_)1;
		}
		else {
			destination[n] = (_type_)0;
		}
	};
	kernel void diagonal(global _type_* destination, global _type_* source, uint LenX) {
		const uint n = get_global_id(0);
		const uint Y = (uint)floor((float)n / (float)LenX);
		const uint X = n - Y * LenX;

		if (X == Y) {
			destination[X] = source[n];
		}
	};
	kernel void join_dim_0(global _type_* destination, global _type_* LHS, uint LHS_LenX, uint LenY, uint LenZ, global _type_* RHS, uint RHS_LenX) {
		const uint n = get_global_id(0);
		const uint Z = (uint)floor((float)n / ((float)(LenY) * (float)(LHS_LenX + RHS_LenX)));
		const uint pos2 = n - Z * ((LenY) * (LHS_LenX + RHS_LenX));
		const uint Y = (uint)floor((float)pos2 / (float)(LHS_LenX + RHS_LenX));
		uint X = pos2 - Y * (LHS_LenX + RHS_LenX);

		if (X < LHS_LenX) {
			destination[n] = LHS[(Z * LenY * LHS_LenX) + (Y * LHS_LenX) + X];
		}
		else {
			X -= LHS_LenX;
			destination[n] = RHS[(Z * LenY * RHS_LenX) + (Y * RHS_LenX) + X];
		}
	};
	kernel void join_dim_1(global _type_* destination, global _type_* LHS, uint LenX, uint LHS_LenY, uint LenZ, global _type_* RHS, uint RHS_LenY) {
		const uint n = get_global_id(0);
		const uint Z = (uint)floor((float)n / ((float)(LHS_LenY + RHS_LenY) * (float)LenX));
		const uint pos2 = n - Z * ((LHS_LenY + RHS_LenY) * LenX);
		uint Y = (uint)floor((float)pos2 / (float)LenX);
		const uint X = pos2 - Y * LenX;

		if (Y < LHS_LenY) {
			destination[n] = LHS[(Z * LHS_LenY * LenX) + (Y * LenX) + X];
		}
		else {
			Y -= LHS_LenY;
			destination[n] = RHS[(Z * RHS_LenY * LenX) + (Y * LenX) + X];
		}
	};
	kernel void join_dim_2(global _type_* destination, global _type_* LHS, uint LenX, uint LenY, uint LHS_LenZ, global _type_* RHS) {
		const uint n = get_global_id(0);
		uint Z = (uint)floor((float)n / ((float)(LenY) * (float)LenX));
		const uint pos2 = n - Z * ((LenY)*LenX);
		const uint Y = (uint)floor((float)pos2 / (float)LenX);
		const uint X = pos2 - Y * LenX;

		if (Z < LHS_LenZ) {
			destination[n] = LHS[(Z * LenY * LenX) + (Y * LenX) + X];
		}
		else {
			Z -= LHS_LenZ;
			destination[n] = RHS[(Z * LenY * LenX) + (Y * LenX) + X];
		}
	};
	kernel void reduce_sum(global _type_* input, global _type_* output, /*local _type_* scratch, */uint n) {
		uint global_id = get_global_id(0);
		uint local_id = get_local_id(0);
		uint group_size = get_local_size(0);
		local _type_ scratch[64];
		// Copy data from global to local memory
		scratch[local_id] = (global_id < n) ? input[global_id] : 0.0f;
		barrier(CLK_LOCAL_MEM_FENCE);

		// Perform reduction within the work-group
		for (uint s = group_size / 2; s > 0; s /= 2) {
			if (local_id < s) {
				scratch[local_id] += scratch[local_id + s];
			}
			barrier(CLK_LOCAL_MEM_FENCE);
		}

		// Write the work-group's partial sum to global memory
		if (local_id == 0) {
			output[get_group_id(0)] = scratch[0];
		}
	};

)) + GL::string(R(
	kernel void linear_between(global _type_ * A, _type_ Low, _type_ High, unsigned long Count) {
		const uint n = get_global_id(0);
		A[n] = (_type_)((float)(High - Low) * (float)((float)n / (float)Count)) + Low;
	};
	kernel void wrap_around(global _type_* C, global _type_* A, unsigned long Count) {
		const uint n = get_global_id(0);
		if (n > Count) {
			C[n] = A[n % Count];
		}
		else {
			C[n] = A[n];
		}
	};
	kernel void row_of(global _type_* destination, global _type_* LHS, uint RowN, uint LHS_LenX, uint LHS_LenY, uint LHS_LenZ) {
		const uint n = get_global_id(0);
		const uint destination_Y = (uint)floor((float)n / (float)(LHS_LenY));
		const uint destination_X = n - destination_Y * LHS_LenY;
		const uint source_Z = destination_Y;
		const uint source_Y = destination_X;
		const uint source_X = RowN;
		const uint source_n = (source_X + (LHS_LenX * source_Y) + ((LHS_LenX * LHS_LenY) * source_Z));
		destination[n] = LHS[source_n];
	};
	kernel void resample(global _type_* destination, global _type_* Source, global uint* Indexes) {
		const uint n = get_global_id(0);
		const uint I = Indexes[n];
		destination[n] = Source[I];
	};
	//kernel void Sign(global _type_* A, global _type_* C) {
	//	const uint n = get_global_id(0);
	//	C[n] = A[n] >= 0 ? (_type_)1 : (_type_)-1;
	//};
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
	kernel void matx_mult_2( // utilizes unrolling to improve performance.
		global _type_* destination,
		uint destination_lenX, uint destination_lenY,
		global _type_* LHS,
		uint LHS_lenX, const uint LHS_lenY,
		global _type_* RHS,
		uint RHS_lenX, uint RHS_lenY
	) {
		_type_ v = 0;
        $pragma unroll
		for (unsigned int index = 0; index < LHS_lenY; ++index) {
			const uint n = get_global_id(0);
			const uint destination_Y = (uint)floor((float)n / (float)destination_lenX);
			const uint destination_X = n - (destination_lenX * destination_Y);
			const uint LHS_X = destination_X; // row from LHS		
			const uint RHS_Y = destination_Y; // column from RHS
			const uint LHS_Y = index;
			const uint RHS_X = index;
			const uint LHS_n = index * LHS_lenX + LHS_X;
			const uint RHS_n = RHS_Y * RHS_lenX + index;
			v += LHS[LHS_n] * RHS[RHS_n];
		}
		destination[get_global_id(0)] = v;
	};	
	kernel void matx_mult( // does not utilize unrolling, which may be desireable if the unroll would exceed the capacity of the processor.
		global _type_* destination,
		uint destination_lenX, uint destination_lenY,
		global _type_* LHS,
		uint LHS_lenX, uint LHS_lenY,
		global _type_* RHS,
		uint RHS_lenX, uint RHS_lenY
	) {
		const uint n = get_global_id(0);
		const uint destination_Y = (uint)floor((float)n / (float)destination_lenX);
		const uint destination_X = n - (destination_lenX * destination_Y);

		// row from LHS
		const uint LHS_X = destination_X;
		// column from RHS
		const uint RHS_Y = destination_Y;

		_type_ v = 0;
		for (unsigned int index = 0; index < LHS_lenY; ++index) {
			const uint LHS_Y = index;
			const uint RHS_X = index;
			const uint LHS_n = LHS_Y * LHS_lenX + LHS_X;
			const uint RHS_n = RHS_Y * RHS_lenX + RHS_X;
			v += LHS[LHS_n] * RHS[RHS_n];
		}

		destination[n] = v;
	};
	kernel void reduce_max(global _type_* input, global _type_* output, /*local _type_* scratch, */uint n, _type_ minV) {
		uint global_id = get_global_id(0);
		uint local_id = get_local_id(0);
		uint group_size = get_local_size(0);
		local _type_ scratch[64];

		// Copy data from global to local memory
		scratch[local_id] = (global_id < n) ? input[global_id] : minV;
		barrier(CLK_LOCAL_MEM_FENCE);

		// Perform reduction within the work-group
		for (uint s = group_size / 2; s > 0; s /= 2) {
			if (local_id < s) {
				scratch[local_id] = fmax(scratch[local_id], scratch[local_id + s]);
			}
			barrier(CLK_LOCAL_MEM_FENCE);
		}

		// Write the work-group's partial sum to global memory
		if (local_id == 0) {
			output[get_group_id(0)] = scratch[0];
		}
	}
	kernel void reduce_min(global _type_* input, global _type_* output, /*local _type_* scratch, */uint n, _type_ maxV) {
		uint global_id = get_global_id(0);
		uint local_id = get_local_id(0);
		uint group_size = get_local_size(0);
		local _type_ scratch[64];

		// Copy data from global to local memory
		scratch[local_id] = (global_id < n) ? input[global_id] : maxV;
		barrier(CLK_LOCAL_MEM_FENCE);

		// Perform reduction within the work-group
		for (uint s = group_size / 2; s > 0; s /= 2) {
			if (local_id < s) {
				scratch[local_id] = fmin(scratch[local_id], scratch[local_id + s]);
			}
			barrier(CLK_LOCAL_MEM_FENCE);
		}

		// Write the work-group's partial sum to global memory
		if (local_id == 0) {
			output[get_group_id(0)] = scratch[0];
		}
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
	kernel void reduce_max(global _type_* input, global _type_* output, local _type_* scratch, uint n, _type_ minV) {
		uint global_id = get_global_id(0);
		uint local_id = get_local_id(0);
		uint group_size = get_local_size(0);

		// Copy data from global to local memory
		scratch[local_id] = (global_id < n) ? input[global_id] : minV;
		barrier(CLK_LOCAL_MEM_FENCE);

		// Perform reduction within the work-group
		for (uint s = group_size / 2; s > 0; s /= 2) {
			if (local_id < s) {
				scratch[local_id] = max(scratch[local_id], scratch[local_id + s]);
			}
			barrier(CLK_LOCAL_MEM_FENCE);
		}

		// Write the work-group's partial sum to global memory
		if (local_id == 0) {
			output[get_group_id(0)] = scratch[0];
		}
	}
	kernel void reduce_min(global _type_* input, global _type_* output, local _type_* scratch, uint n, _type_ maxV) {
		uint global_id = get_global_id(0);
		uint local_id = get_local_id(0);
		uint group_size = get_local_size(0);

		// Copy data from global to local memory
		scratch[local_id] = (global_id < n) ? input[global_id] : maxV;
		barrier(CLK_LOCAL_MEM_FENCE);

		// Perform reduction within the work-group
		for (uint s = group_size / 2; s > 0; s /= 2) {
			if (local_id < s) {
				scratch[local_id] = min(scratch[local_id], scratch[local_id + s]);
			}
			barrier(CLK_LOCAL_MEM_FENCE);
		}

		// Write the work-group's partial sum to global memory
		if (local_id == 0) {
			output[get_group_id(0)] = scratch[0];
		}
	}

));
}
return out.replace("_type_", GL::string(typeName)).to_string();
} // ############################################################### end of OpenCL C code #####################################################################