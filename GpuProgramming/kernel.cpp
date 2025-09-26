#include "kernel.hpp" // note: unbalanced round brackets () are not allowed and string literals can't be arbitrarily long, so periodically interrupt with )+R(

//GL::string templated_algorithms(GL::string TypeT){
//	return GL::string(R( // ########################## begin of OpenCL C code ####################################################################
//
//	kernel void add(global _Type_* A, global _Type_* B, global _Type_* C) {
//		const uint n = get_global_id(0);
//		C[n] = A[n] + B[n];
//	}
//)).replace("_Type_", TypeT); } // ############################################################### end of OpenCL C code #####################################################################
//


string opencl_c_container() {
	return R( // ########################## begin of OpenCL C code ####################################################################

	kernel void add(global float* A, global float* B, global float* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] + B[n];
	}
	kernel void add_inplace(global float* A, global float* B) {
		const uint n = get_global_id(0);
		A[n] += B[n];
	}

	kernel void mult(global float* A, global float* B, global float* C) {
		const uint n = get_global_id(0);
		C[n] = A[n] * B[n];
	}
	kernel void mult_inplace(global float* A, global float* B) {
		const uint n = get_global_id(0);
		A[n] *= B[n];
	}


);} // ############################################################### end of OpenCL C code #####################################################################