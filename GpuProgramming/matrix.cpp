#pragma once

#include <cstdarg>
#include <type_traits>
#include <ShlDisp.h>
#include <winnt.h>
#include <thread>
#include <execution>
#include <memory>
#include <boost/math/distributions/students_t.hpp>

#include "matrix.h"
#include "opencl.hpp"
#include "../ScriptLanguageTesting/Strings.h"
#include "../ScriptLanguageTesting/thread_object.h"
#include "../ScriptLanguageTesting/atomic_allocator.h"
#include "../ScriptLanguageTesting/atomic_maps.h"
#include "../ScriptLanguageTesting/atomic_stack.h"
#include "../ScriptLanguageTesting/ticket_dispensor.h"
#include "../ScriptLanguageTesting/atomic_tree.h"
#include "../ScriptLanguageTesting/stopwatch.h"
#include "../ScriptLanguageTesting/util.h"

class mem_matrix; // linear array of bytes that manages a pointer to GPU and/or CPU memory, as well as GPU events (queued GPU jobs). GPU jobs are awaited if attempting to be destroyed. 
static void* Mem_Alloc(const size_t& size) {
    if (!size) return nullptr; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16);
};
static void  Mem_Free(void* ptr) {
    if (ptr) ::_aligned_free(ptr);
};

namespace parallel {
    template<typename Type = unsigned int> class sequence {
    private:
        Type min;
        Type max;
        Type step;

        static std::tuple<Type, Type, Type> DetermineSteps(Type N0, Type N1, Type Step) {
            if (Step >= 0) {
                // want to go from small to large
                if (N1 >= N0) {
                    return { N0, N1, Step };
                }
                else {
                    return { N1, N0, Step };
                }
            }
            else {
                // want to go from large to small
                if (N1 >= N0) {
                    return { N1, N0, Step };
                }
                else {
                    return { N0, N1, Step };
                }
            }
        };

    public:
        sequence(Type N0, Type N1, Type Step) {
            std::tie(min, max, step) = DetermineSteps(std::move(N0), std::move(N1), std::move(Step));
        };
        sequence() : sequence(0, 0, 1) {};
        sequence(Type N) : sequence(0, N, 1) {};
        sequence(Type N0, Type N1) : sequence(N0, N1, 1) {};

        class Iterator { // : public std::iterator<std::random_access_iterator_tag, Type>
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = Type;
            using difference_type = ptrdiff_t;
            using pointer = Type*;
            using reference = Type&;

            Iterator() : _ptr(0), _min(0), _step(1) {}
            Iterator(Type rhs, Type min, Type step) : _ptr(rhs), _min(min), _step(step) {}
            Iterator(const Iterator& rhs) : _ptr(rhs._ptr), _min(rhs._min), _step(rhs._step) {}

            inline Iterator& operator+=(difference_type rhs) { _ptr += static_cast<Type>(rhs) * _step; return *this; }
            inline Iterator& operator-=(difference_type rhs) { _ptr -= static_cast<Type>(rhs) * _step; return *this; }
            inline Type& operator*() { return _ptr; }
            inline Type* operator->() { return &_ptr; }
            inline Type operator[](difference_type rhs) { return static_cast<Type>(_min + static_cast<Type>(rhs) * _step); }
            inline const Type& operator*() const { return _ptr; }
            inline const Type* operator->() const { return &_ptr; }
            inline const Type operator[](difference_type rhs) const { return static_cast<Type>(_min + static_cast<Type>(rhs) * _step); }

            inline Iterator& operator++() { _ptr += _step; return *this; }
            inline Iterator& operator--() { _ptr -= _step; return *this; }
            inline Iterator operator++(int) { Iterator tmp(*this); _ptr += _step; return tmp; }
            inline Iterator operator--(int) { Iterator tmp(*this); _ptr -= _step; return tmp; }
            inline difference_type operator-(const Iterator& rhs) const { return (_ptr - rhs._ptr) / _step; }
            inline Iterator operator+(difference_type rhs) const { return Iterator(_ptr + static_cast<Type>(rhs) * _step, _min, _step); }
            inline Iterator operator-(difference_type rhs) const { return Iterator(_ptr - static_cast<Type>(rhs) * _step, _min, _step); }
            friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { return Iterator((static_cast<Type>(lhs) * rhs._step) + rhs._ptr, rhs._min, rhs._step); }
            friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { return Iterator((static_cast<Type>(lhs) * rhs._step) - rhs._ptr, rhs._min, rhs._step); }

            inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
            inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
            inline bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
            inline bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
            inline bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
            inline bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

        protected:
            Type _min;
            Type _ptr;
            Type _step;
        };

        using iterator = Iterator;
        using const_iterator = iterator;

        auto begin() { return Iterator(min, min, step); };
        auto end() { return Iterator(max, min, step); };
        auto cbegin() const { return iterator(min, min, step); };
        auto cend() const { return iterator(max, min, step); };
        auto begin() const { return iterator(min, min, step); };
        auto end() const { return iterator(max, min, step); };
    };
    template<typename iteratorType, class F> decltype(auto) Std_For(iteratorType start, iteratorType end, F const& ToDo) {
        sequence<iteratorType> seq(start, end); // 0..999
        std::exception_ptr* e{ nullptr };

        std::for_each(
            std::execution::par,
            seq.begin(),
            seq.end(),
            [&](auto& x) { // copies are safer, and the resulting code will be as quick.
                try {
                    if (!e) ToDo(x);
                }
                catch (std::runtime_error& err) {
                    std::cout << err.what() << std::endl;
                    if (!e) {
                        auto ptr = new std::exception_ptr(std::current_exception());
                        if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
                            delete ptr;
                        }
                    }
                }
                catch (...) {
                    if (!e) {
                        auto ptr = new std::exception_ptr(std::current_exception());
                        if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&e), ptr, nullptr) != nullptr) {
                            delete ptr;
                        }
                    }
                }
            }
        );
        if (e) {
            std::exception_ptr copy{ *e };
            delete e;
            std::rethrow_exception(std::move(copy));
        }
    };
};

// #define CL_HPP_CL_1_2_DEFAULT_BUILD

class opencl_impl {
public:
    template<typename T>
    static constexpr decltype(auto) type_name() {
        if constexpr (std::is_same_v<T, char>) return "char";
        else if constexpr (std::is_same_v<T, unsigned char>) return "uchar";
        else if constexpr (std::is_same_v<T, int>) return "int";
        else if constexpr (std::is_same_v<T, unsigned int>) return "uint";
        else if constexpr (std::is_same_v<T, long>) return "long";
        else if constexpr (std::is_same_v<T, unsigned long>) return "ulong";
        else if constexpr (std::is_same_v<T, float>) return "float";
        else if constexpr (std::is_same_v<T, double>) return "double";
        else if constexpr (std::is_same_v<T, void>) return "";
        else static_assert("Not all numeric types are supported by GPU calculations.");
    }

    template<typename T>
    static std::string create_kernel() {
        GL::string out;
#define R(...) GL::string(" "#__VA_ARGS__" ")
        out = out + R(
        kernel void copy_type_(global _type_ * A, global _type_ * B) {
            const uint n = get_global_id(0);
            A[n] = B[n];
        };

        kernel void copy_slice_type_(global _type_ * A, global _type_ * B, uint offset) {
            const uint n = get_global_id(0);
            A[n] = B[n + offset];
        };

        kernel void subsample_1D_type_(global _type_ * destination, global _type_ * Source, global float* Indexes) {
            // assumes that positions are floating-point indices, e.g. 2.5 means halfway between index 2 and 3
            const uint n = get_global_id(0);
            const float I = Indexes[n];
            const float lhs = floor(I);
            const float rhs = floor(I + 1);
            const float result = ((float)(Source[(uint)lhs]) * (rhs - I)) + ((float)(Source[(uint)rhs]) * (I - lhs));
            destination[n] = (_type_)result;
        };
        kernel void binomial_search_smallest_gre_type_(global uint * destination, global _type_ * Source, global _type_ * Find, uint srcelX) {
            const uint n = get_global_id(0);
            // assumes goal is to locate the index of Find[n] in the sorted list of Source(0,srcelX] that is greater than or equal to Find[n]
            const _type_ time = Find[n];

            // use binary search to find the index for the given time
            long len = (long)srcelX;
            long mid = len;
            long offset = 0;
            long res = 0;
            _type_ sample = 0;
            while (mid > 0) {
                mid = len >> 1;
                // OPTIMIZED ORDERING
                sample = Source[offset + mid];
                if (time >= sample)
                {
                    offset += mid;
                    len -= mid;
                    res = 1;
                    if (time == sample) {
                        destination[n] = (uint)(offset);
                        return;
                    }
                }
                else
                {
                    len -= mid;
                    res = 0;
                }
            }
            destination[n] = (uint)(offset + res);
        };
        kernel void subsample_pat_type_(global _type_ * destination, global _type_ * SourceY, global _type_ * SourceX, global _type_ * FindX, uint srcelX) {
            const uint n = get_global_id(0);
            // assumes goal is to locate the index of Find[n] in the sorted list of Source(0,srcelX] that is greater than or equal to Find[n]
            const _type_ time = FindX[n];
            uint index_1 = 0;

            // use binary search to find the index for the given time
            long len = (long)srcelX;
            long mid = len;
            long offset = 0;
            long res = 0;
            _type_ sample;
            while (mid > 0) {
                mid = len >> 1;
                // OPTIMIZED ORDERING
                sample = SourceX[offset + mid];
                if (time >= sample)
                {
                    offset += mid;
                    len -= mid;
                    res = 1;
                    if (time == sample) {
                        destination[n] = SourceY[(uint)offset];
                        return;
                    }
                }
                else
                {
                    len -= mid;
                    res = 0;
                }
            }
            index_1 = (uint)(offset + res);

            if (index_1 <= 0) {
                // the value we requested exists before our dataset... return the earliest value
                destination[n] = SourceY[0];
            }
            else if (index_1 >= (srcelX - 1)) {
                // the value we requested exists after our dataset... return the latest value
                destination[n] = SourceY[srcelX - 1];
            }
            else {
                uint index_0 = index_1 - 1;
                uint index_2 = index_1 + 1;
                uint index_3 = index_1 + 2;
                float s = (float)(time - SourceX[index_0]) / (float)(SourceX[index_1] - SourceX[index_0]);
                if (index_1 == 1) {
                    // we are between indices 0 and 1... do a linear interp.
                    destination[n] = (_type_)(((float)SourceY[index_0] * (1.0f - s)) + ((float)SourceY[index_1] * s));
                }
                else if (index_1 == (srcelX - 2)) {
                    // we are between the final two indices ... do a linear interp.
                    destination[n] = (_type_)(((float)SourceY[index_0] * (1.0f - s)) + ((float)SourceY[index_1] * s));
                }
                else {
                    // we should have enough data to do our analysis.
                    float4 bvals = (float4)(
                        ((2.0f - s) * s - 1.0f) * s * 0.5f,				// -0.5f s * s * s + s * s - 0.5f * s
                        (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f,		// 1.5f * s * s * s - 2.5f * s * s + 1.0f
                        ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f,		// -1.5f * s * s * s - 2.0f * s * s + 0.5f s
                        ((s - 1.0f) * s * s) * 0.5f						// 0.5f * s * s * s - 0.5f * s * s
                        );
                    destination[n] = (_type_)(
                        (bvals[0] * (float)SourceY[index_0]) +
                        (bvals[1] * (float)SourceY[index_1]) +
                        (bvals[2] * (float)SourceY[index_2]) +
                        (bvals[3] * (float)SourceY[index_3])
                        );
                }
            }
        };

        //kernel void subsample_2D_type_(global _type_* destination, global _type_* Source, global float2* Indexes) {
        //    // assumes that Indexes positions are 2-D floating-point indices, e.g. [2.5, 2.5] means halfway between index 2 and 3 in both the x and y axis
        //    const uint n = get_global_id(0);
        //    const float I = Indexes[n];
        //    const float lhs = floor(I);
        //    const float rhs = floor(I + 1);
        //    const float result = ((float)(Source[(uint)lhs](*(rhs - I)) + ((float)(Source[(uint)rhs]) * (I - lhs));
        //    destination[n] = (_type_)result;
        //};
        kernel void copy_resize_type_(global _type_ * A, global _type_ * B, uint destlX, uint destlY, uint destlZ, uint srcelX, uint srcelY, uint srcelZ) {
            const uint n = get_global_id(0);
            const uint Z = (uint)floor((float)n / ((float)(destlY) * (float)(destlX)));
            const uint pos2 = n - Z * destlY * destlX;
            const uint Y = (uint)floor((float)pos2 / (float)(destlX));
            const uint X = pos2 - Y * destlX;
            const uint srce_N = (Z * srcelX * srcelY) + (Y * srcelX) + X;

            if (X >= 0 && X < srcelX) {
                if (Y >= 0 && Y < srcelY) {
                    if (Z >= 0 && Z < srcelZ) {
                        A[n] = B[srce_N];
                    }
                    else {
                        A[n] = 0;
                    }
                }
                else {
                    A[n] = 0;
                }
            }
            else {
                A[n] = 0;
            }
        };
        kernel void copy_resize_stretch_type_(global _type_ * A, global _type_ * B, uint destlX, uint destlY, uint destlZ, uint srcelX, uint srcelY, uint srcelZ) {
            const uint n = get_global_id(0);
            const uint Z = (uint)floor((float)n / ((float)(destlY) * (float)(destlX)));
            const uint pos2 = n - Z * destlY * destlX;
            const uint Y = (uint)floor((float)pos2 / (float)(destlX));
            const uint X = pos2 - Y * destlX;

            if ((srcelX < destlX || srcelY < destlY) && (destlZ == 1) && (srcelZ == 1)) {
                // desination is expanding the source image. Destination should bilinear blend from the available samples.
                float4 p = (float4)((float)X * (float)srcelX / (float)destlX, (float)Y * (float)srcelY / (float)destlY, 1, 0);
                float4 p1 = (float4)(floor(p[0]), floor(p[1]), 0, 0);
                float4 p2 = (float4)(floor(p[0] + 1), floor(p[1]), 0, 0);
                float4 p3 = (float4)(floor(p[0]), floor(p[1] + 1), 0, 0);
                float4 p4 = (float4)(floor(p[0] + 1), floor(p[1] + 1), 0, 0);

                // p1-p4
                // p2-p3
                // p3-p2
                // p4-p1

                float a1 = fabs((p1[0] - p[0]) * (p1[1] - p[1]) * (p1[2] - p[2]));
                float a2 = fabs((p2[0] - p[0]) * (p2[1] - p[1]) * (p2[2] - p[2]));
                float a3 = fabs((p3[0] - p[0]) * (p3[1] - p[1]) * (p3[2] - p[2]));
                float a4 = fabs((p4[0] - p[0]) * (p4[1] - p[1]) * (p4[2] - p[2]));

                //a1 = distance(p1, p);
                //a2 = distance(p2, p);
                //a3 = distance(p3, p);
                //a4 = distance(p4, p);

                p1[3] = (float)(B[((uint)p1[1] * srcelX) + (uint)p1[0]]);
                p2[3] = (float)(B[((uint)p2[1] * srcelX) + (uint)p2[0]]);
                p3[3] = (float)(B[((uint)p3[1] * srcelX) + (uint)p3[0]]);
                p4[3] = (float)(B[((uint)p4[1] * srcelX) + (uint)p4[0]]);

                p[3] += p1[3] * a4;
                p[3] += p2[3] * a3;
                p[3] += p3[3] * a2;
                p[3] += p4[3] * a1;

                A[n] = (_type_)(p[3]);
            }
            else {
                // desintation is shrinking the source image. Assume the user already performed a blur and simply drop pixels.
                uint srceX = fmin(srcelX - 1, floor(((float)X * (float)srcelX / (float)(destlX)) + 0.5));
                uint srceY = fmin(srcelY - 1, floor(((float)Y * (float)srcelY / (float)(destlY)) + 0.5));
                uint srceZ = fmin(srcelZ - 1, floor(((float)Z * (float)srcelZ / (float)(destlX)) + 0.5));
                uint srce_N = (srceZ * srcelX * srcelY) + (srceY * srcelX) + srceX;
                A[n] = B[srce_N];
            }
        };

        kernel void copy_single_type_(global _type_ * A, _type_ B) {
            const uint n = get_global_id(0);
            A[n] = B;
        };
        kernel void add_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] + B[n];
        };
        kernel void add_inplace_type_(global _type_ * A, global _type_ * B) {
            const uint n = get_global_id(0);
            A[n] += B[n];
        };
        kernel void sub_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] - B[n];
        };
        kernel void sub_inplace_type_(global _type_ * A, global _type_ * B) {
            const uint n = get_global_id(0);
            A[n] -= B[n];
        };
        kernel void mult_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] * B[n];
        };
        kernel void mult_inplace_type_(global _type_ * A, global _type_ * B) {
            const uint n = get_global_id(0);
            A[n] *= B[n];
        };
        kernel void divide_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] / B[n];
        };
        kernel void divide_inplace_type_(global _type_ * A, global _type_ * B) {
            const uint n = get_global_id(0);
            A[n] /= B[n];
        };
        kernel void add_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] + B;
        };
        kernel void add_single_inplace_type_(global _type_ * A, _type_ B) {
            const uint n = get_global_id(0);
            A[n] += B;
        };
        kernel void sub_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] - B;
        };
        kernel void sub_single_inv_type_(global _type_ * A, _type_ B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = B - A[n];
        };
        kernel void sub_single_inplace_type_(global _type_ * A, _type_ B) {
            const uint n = get_global_id(0);
            A[n] -= B;
        };
        kernel void mult_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] * B;
        };
        kernel void mult_single_inplace_type_(global _type_ * A, _type_ B) {
            const uint n = get_global_id(0);
            A[n] *= B;
        };
        kernel void divide_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = A[n] / B;
        };
        kernel void divide_single_inv_type_(global _type_ * A, _type_ B, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = B / A[n];
        };
        kernel void divide_single_inplace_type_(global _type_ * A, _type_ B) {
            const uint n = get_global_id(0);
            A[n] /= B;
        };

        kernel void from_char_type_(global _type_ * A, global char* B) {
            const uint n = get_global_id(0);
            A[n] = (_type_)B[n];
        };
        kernel void from_uchar_type_(global _type_ * A, global uchar * B) {
            const uint n = get_global_id(0);
            A[n] = (_type_)B[n];
        };
        kernel void from_ulong_type_(global _type_ * A, global ulong * B) {
            const uint n = get_global_id(0);
            A[n] = (_type_)B[n];
        };
        kernel void from_uint_type_(global _type_ * A, global uint * B) {
            const uint n = get_global_id(0);
            A[n] = (_type_)B[n];
        };
        kernel void from_long_type_(global _type_ * A, global long* B) {
            const uint n = get_global_id(0);
            A[n] = (_type_)B[n];
        };
        kernel void from_int_type_(global _type_ * A, global int* B) {
            const uint n = get_global_id(0);
            A[n] = (_type_)B[n];
        };
        kernel void from_float_type_(global _type_ * A, global float* B) {
            const uint n = get_global_id(0);
            A[n] = (_type_)B[n];
        };

        kernel void Cos_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = cos((float)A[n]);
        }
        kernel void Sin_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = sin((float)A[n]);
        }
        kernel void Tan_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = tan((float)A[n]);
        }
        kernel void aCos_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = acos((float)A[n]);
        }
        kernel void aSin_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = asin((float)A[n]);
        }
        kernel void aTan_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = atan((float)A[n]);
        }
        kernel void Cosh_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = cosh((float)A[n]);
        }
        kernel void Sinh_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = sinh((float)A[n]);
        }
        kernel void Tanh_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = tanh((float)A[n]);
        }
        kernel void aCosh_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = acosh((float)A[n]);
        }
        kernel void aSinh_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = asinh((float)A[n]);
        }
        kernel void aTanh_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = atanh((float)A[n]);
        }
        kernel void Exp_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = exp((float)A[n]);
        }
        kernel void Exp2_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = exp2((float)A[n]);
        }
        kernel void Exp10_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = exp10((float)A[n]);
        }
        kernel void Expm1_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = expm1((float)A[n]);
        }
        kernel void Lgamma_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = lgamma((float)A[n]);
        }
        kernel void Log_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = log((float)A[n]);
        }
        kernel void Log2_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = log2((float)A[n]);
        }
        kernel void Log10_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = log10((float)A[n]);
        }
        kernel void Log1p_type_(global _type_ * A, global _type_ * C) {
            const uint n = get_global_id(0);
            C[n] = log1p((float)A[n]);
        }
        );
        out = out + R(
            kernel void item_eq_single_type_(global _type_ * A, _type_ B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] == B) ? 1 : 0;
        };
        kernel void item_neq_single_type_(global _type_ * A, _type_ B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] != B) ? 1 : 0;
        };
        kernel void item_eq_type_(global _type_ * A, global _type_ * B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] == B[n]) ? 1 : 0;
        };
        kernel void item_neq_type_(global _type_ * A, global _type_ * B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] != B[n]) ? 1 : 0;
        };
        kernel void item_not_type_(global uint * A, global _type_ * B) {
            const uint n = get_global_id(0);
            A[n] = !B[n];
        };
        kernel void item_ls_single_type_(global _type_ * A, _type_ B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] < B) ? 1 : 0;
        }
        kernel void item_lse_single_type_(global _type_ * A, _type_ B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] <= B) ? 1 : 0;
        }
        kernel void item_ls_type_(global _type_ * A, global _type_ * B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] < B[n]) ? 1 : 0;
        }
        kernel void item_lse_type_(global _type_ * A, global _type_ * B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] <= B[n]) ? 1 : 0;
        }
        kernel void item_gr_single_type_(global _type_ * A, _type_ B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] > B) ? 1 : 0;
        }
        kernel void item_gre_single_type_(global _type_ * A, _type_ B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] >= B) ? 1 : 0;
        }
        kernel void item_gr_type_(global _type_ * A, global _type_ * B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] > B[n]) ? 1 : 0;
        }
        kernel void item_gre_type_(global _type_ * A, global _type_ * B, global uint * C) {
            const uint n = get_global_id(0);
            C[n] = (A[n] >= B[n]) ? 1 : 0;
        }
        kernel void join_dim_0_type_(global _type_ * destination, global _type_ * LHS, uint LHS_LenX, uint LenY, uint LenZ, global _type_ * RHS, uint RHS_LenX) {
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
        kernel void join_dim_1_type_(global _type_ * destination, global _type_ * LHS, uint LenX, uint LHS_LenY, uint LenZ, global _type_ * RHS, uint RHS_LenY) {
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
        kernel void join_dim_2_type_(global _type_ * destination, global _type_ * LHS, uint LenX, uint LenY, uint LHS_LenZ, global _type_ * RHS) {
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
        kernel void Transpose_type_(global _type_ * destination, global _type_ * RHS, uint lenX, uint lenY) {
            const uint n = get_global_id(0);
            const uint source_X = (uint)floor((float)n / (float)lenY);
            const uint source_Y = n - (lenY * source_X);
            const uint source_N = source_Y * lenX + source_X;
            destination[n] = RHS[source_N];
        };
        kernel void make_square_type_(global _type_ * destination, global _type_ * RHS, uint RHS_LenX, uint RHS_LenY, uint LenZ, uint Len) {
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
        kernel void identity_type_(global _type_ * destination, uint LenX) {
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
        kernel void diagonal_type_(global _type_ * destination, global _type_ * source, uint LenX) {
            const uint n = get_global_id(0);
            const uint Y = (uint)floor((float)n / (float)LenX);
            const uint X = n - Y * LenX;

            if (X == Y) {
                destination[X] = source[n];
            }
        };
        kernel void reduce_sum_type_(global _type_ * input, global _type_ * output, uint n) {
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
        kernel void linear_between_type_(global _type_ * A, _type_ Low, _type_ High, uint Count) {
            const uint n = get_global_id(0);
            A[n] = (_type_)((float)(High - Low) * (float)((float)n / (float)Count)) + Low;
        };
        kernel void wrap_around_type_(global _type_ * C, global _type_ * A, uint Count) {
            const uint n = get_global_id(0);
            if (n > Count) {
                C[n] = A[n % Count];
            }
            else {
                C[n] = A[n];
            }
        };
        kernel void resample_type_(global _type_ * destination, global _type_ * Source, global uint * Indexes) {
            const uint n = get_global_id(0);
            const uint I = Indexes[n];
            destination[n] = Source[I];
        };
        kernel void row_of_type_(global _type_ * destination, global _type_ * LHS, uint RowN, uint LHS_LenX, uint LHS_LenY, uint LHS_LenZ) {
            const uint n = get_global_id(0);
            const uint destination_Y = (uint)floor((float)n / (float)(LHS_LenY));
            const uint destination_X = n - destination_Y * LHS_LenY;
            const uint source_Z = destination_Y;
            const uint source_Y = destination_X;
            const uint source_X = RowN;
            const uint source_n = (source_X + (LHS_LenX * source_Y) + ((LHS_LenX * LHS_LenY) * source_Z));
            destination[n] = LHS[source_n];
        };

        kernel void convolve_type_(global _type_ * A, global _type_ * B, global _type_ * K, uint lX, uint lY, uint kX, uint kY, float kTot) {
            const int n = (int)get_global_id(0);
            const int Y = (int)floor((float)n / (float)lX);
            const int X = (int)n - Y * (int)lX;
            float kernel_captured = 0;

            const int kW = (int)floor((float)(kX - 1) / 2.0f);
            const int kH = (int)floor((float)(kY - 1) / 2.0f);

            float result = 0.0f;
            for (int offset_x = -kW; offset_x <= kW; ++offset_x) {
                const int x = X + offset_x;
                if (x < 0) { continue; }
                if (x >= lX) { continue; }

                for (int offset_y = -kH; offset_y <= kH; ++offset_y) {
                    const int y = Y + offset_y;
                    if (y < 0) { continue; }
                    if (y >= lY) { continue; }

                    const uint b_i = (y * lX) + x;
                    const uint k_i = (offset_y + kH) * kX + (offset_x + kW);

                    result += (float)B[b_i] * (float)K[k_i];

                    kernel_captured += (float)K[k_i];
                }
            }
            if (kernel_captured > 0) {
                result *= (kTot / kernel_captured);
            }
            A[n] = (_type_)result;
        };

        kernel void ASCII_type_(global char* A, global _type_ * B, _type_ minValue, _type_ maxValue, global char* ramp, unsigned int ramp_length) {
            // convert from the input to ASCII based on the intensity of the incoming values
            const uint n = (uint)get_global_id(0);
            const float delta = (float)maxValue - (float)minValue;
            if (delta > 0) {
                if (n % 2 == 0) {
                    const uint index = fmin(floor(((((float)(B[n] - minValue) /*+ 0.5f*/) / delta) * (float)ramp_length) + 0.5), ramp_length - 1);
                    A[n] = ramp[index];
                }
                else {
                    const uint index = fmin(floor((((float)(B[n] - minValue) / delta) * (float)ramp_length) + 0.5), ramp_length - 1);
                    A[n] = ramp[index];
                }
            }
            else {
                A[n] = ramp[0];
            }
        }

        );
        if constexpr (std::is_floating_point_v<T>) {
            out = out + R(
            global atomic_int __rand_global_counter = ATOMIC_VAR_INIT(123456789);
            uint __rand_global() {
                uint x, y;
                for (;;) {
                    x = y = atomic_load(&__rand_global_counter);
                    x ^= x << 13;
                    x ^= x >> 17;
                    x ^= x << 5;
                    // atomic_xchg(&__rand_global_counter, x);
                    if (atom_cmpxchg(&__rand_global_counter, y, x) == y) break;
                }
                return x;
            };
            kernel void Rand_type_(global _type_ * A) {
                const uint n = get_global_id(0);
                const uint lS = get_local_id(0);
                local uint __rand_counter;
                if (lS == 0) {
                    __rand_counter = __rand_global();
                }
                barrier(CLK_LOCAL_MEM_FENCE);

                uint __this_rand_counter = __rand_counter;

                for (int i = 0; i < lS; ++i) {
                    __this_rand_counter ^= __this_rand_counter << 17;
                    __this_rand_counter ^= __this_rand_counter >> 19;
                    __this_rand_counter ^= __this_rand_counter << 7;
                }
                A[n] = (_type_)__this_rand_counter / ((uint)~((uint)0));
            };
            kernel void power_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = pow(A[n], B);
            };
            kernel void power_n_single_type_(global _type_ * A, int B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = pown(A[n], B);
            };
            kernel void power_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = pow(A[n], B[n]);
            };
            kernel void power_n_type_(global _type_ * A, global int* B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = pown(A[n], B[n]);
            };
            kernel void square_root_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = native_sqrt(A[n]);
            };
            kernel void round_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                const _type_ H = ((_type_)1) / (_type_)2;
                C[n] = floor(A[n] + H);
            };
            kernel void flr_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = floor(A[n]);
            };
            kernel void ceil_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = floor(A[n] + 1);
            };
            kernel void mult_add_type_(global _type_ * A, global _type_ * B, global _type_ * C, global _type_ * D) {
                const uint n = get_global_id(0);
                D[n] = fma(A[n], B[n], C[n]);
            };
            kernel void absolute_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fabs(A[n]);
            };
            kernel void Mod_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmod(A[n], B[n]);
            }
            kernel void Mod_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmod(A[n], B);
            }
            kernel void Max_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmax(A[n], B[n]);
            }
            kernel void Max_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmax(A[n], B);
            }
            kernel void Min_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmin(A[n], B[n]);
            }
            kernel void Min_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmin(A[n], B);
            }
            kernel void reduce_max_type_(global _type_ * input, global _type_ * output, uint n, _type_ minV) {
                const uint global_id = get_global_id(0);
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
                    output[global_id] = scratch[0];
                }
            }
            kernel void reduce_min_type_(global _type_ * input, global _type_ * output, uint n, _type_ maxV) {
                const uint global_id = get_global_id(0);
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
                    output[global_id] = scratch[0];
                }
            }
            );
            out = out + R(
                kernel void item_AND_type_(global uint * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                A[n] = B[n] && C[n];
            };
            kernel void item_OR_type_(global uint * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                A[n] = B[n] || C[n];
            };
            kernel void item_AND_single_type_(global uint * A, global _type_ * B, _type_ C) {
                const uint n = get_global_id(0);
                A[n] = B[n] && C;
            };
            kernel void item_OR_single_type_(global uint * A, global _type_ * B, _type_ C) {
                const uint n = get_global_id(0);
                A[n] = B[n] || C;
            };
            kernel void guassian_type_(global _type_ * A, uint lX, uint lY) {
                const int n = (int)get_global_id(0);
                const int Y = (int)floor((float)n / (float)lY);
                const int X = (int)n - Y * (int)lY;
                const _type_ x = ((_type_)X + 0.5) - ((_type_)lX / 2.0);
                const _type_ y = ((_type_)Y + 0.5) - ((_type_)lY / 2.0);
                float SigmaX = fmax(0.5, ((((_type_)lX - 1.0) / 2.0) * 0.5));
                float SigmaY = fmax(0.5, ((((_type_)lY - 1.0) / 2.0) * 0.5));
                A[n] = exp(-(((x * x) / (2 * SigmaX * SigmaX)) + ((y * y) / (2 * SigmaY * SigmaY)))) / (2.0 * 3.141592653589793238462643383279502884197169399375105820974944 * SigmaX * SigmaY);
                // A[n] = (1.0 / (2.0 * 3.141592653589793238462643383279502884197169399375105820974944)) * exp(-(x*x + y*y) / 2.0);
            };

            );
        }
        else {
            out = out + R(
            kernel void power_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = pow((float)A[n], (float)B);
            }
            kernel void power_n_single_type_(global _type_ * A, int B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = (_type_)pown((float)A[n], B);
            }
            kernel void power_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = (_type_)pow((float)A[n], (float)B[n]);
            }
            kernel void power_n_type_(global _type_ * A, global int* B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = (_type_)pown((float)A[n], B[n]);
            }
            kernel void square_root_type_(global _type_ * A, global float* C) {
                const uint n = get_global_id(0);
                C[n] = native_sqrt((float)A[n]);
            }
            kernel void round_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = A[n];
            }
            kernel void mult_add_type_(global _type_ * A, global _type_ * B, global _type_ * C, global _type_ * D) {
                const uint n = get_global_id(0);
                D[n] = (A[n] * B[n]) + C[n];
            }
            kernel void absolute_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = abs(A[n]);
            }
            kernel void Mod_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = A[n] % B[n];
            }
            kernel void Mod_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = A[n] % B;
            }
            kernel void Max_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = max(A[n], B[n]);
            }
            kernel void Max_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = max(A[n], B);
            }
            kernel void Min_type_(global _type_ * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = min(A[n], B[n]);
            }
            kernel void Min_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = min(A[n], B);
            }
            kernel void reduce_max_type_(global _type_ * input, global _type_ * output, uint n, _type_ minV) {
                const uint global_id = get_global_id(0);
                uint local_id = get_local_id(0);
                uint group_size = get_local_size(0);
                local _type_ scratch[64];

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
                    output[global_id] = scratch[0];
                }
            }
            kernel void reduce_min_type_(global _type_ * input, global _type_ * output, uint n, _type_ maxV) {
                const uint global_id = get_global_id(0);
                uint local_id = get_local_id(0);
                uint group_size = get_local_size(0);
                local _type_ scratch[64];

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
                    output[global_id] = scratch[0];
                }
            }
            kernel void item_AND_type_(global uint * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                A[n] = B[n] && C[n];
            };
            kernel void item_OR_type_(global uint * A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                A[n] = B[n] || C[n];
            };
            kernel void item_AND_single_type_(global uint * A, global _type_ * B, _type_ C) {
                const uint n = get_global_id(0);
                A[n] = B[n] && C;
            };
            kernel void item_OR_single_type_(global uint * A, global _type_ * B, _type_ C) {
                const uint n = get_global_id(0);
                A[n] = B[n] || C;
            };

            );
        }

#undef R        
        return out.replace("_type_", GL::string(type_name<T>())).to_string();
    };

    static std::string create_kernels() {
        std::string out =
            create_kernel<char>() + "\n" +
            create_kernel<unsigned char>() + "\n" +
            create_kernel<int>() + "\n" +
            create_kernel<unsigned int>() + "\n" +
            create_kernel<long>() + "\n" +
            create_kernel<unsigned long>() + "\n" +
            create_kernel<float>() + "\n";

        return out;
    };

    static Device& get_device() {
        static Device device(select_device_with_most_flops(), create_kernels());
        return device;
    };
};

template <typename T> class Lockable {
private:
    T obj;
    std::shared_mutex mut;

public:
    template <typename... U> Lockable(const U&... args) : obj(args...), mut() {};
    Lockable(Lockable const&) = delete;
    Lockable(Lockable&&) = delete;
    Lockable& operator=(Lockable const&) = delete;
    Lockable& operator=(Lockable&&) = delete;
    ~Lockable() = default;

    class Locked {
        std::scoped_lock<std::shared_mutex> locked;

    public:
        T& obj;

        Locked(std::shared_mutex const& l, const T& o) : locked(const_cast<std::shared_mutex&>(l)), obj{ const_cast<T&>(o) } {};
        Locked(Locked const&) = delete;
        Locked(Locked&&) = delete;
        Locked& operator=(Locked const&) = delete;
        Locked& operator=(Locked&&) = delete;
        ~Locked() = default;
    };
    class cLocked {
        std::shared_lock<std::shared_mutex> locked;

    public:
        const T& obj;

        cLocked(std::shared_mutex const& l, const T& o) : locked(const_cast<std::shared_mutex&>(l)), obj{ o } {};
        cLocked(cLocked const&) = delete;
        cLocked(cLocked&&) = delete;
        cLocked& operator=(cLocked const&) = delete;
        cLocked& operator=(cLocked&&) = delete;
        ~cLocked() = default;
    };

    cLocked get() const {
        return cLocked(mut, obj);
    };
    Locked get() {
        return Locked(mut, obj);
    };
};

// Compiles OpenCL code and makes it available through its assigned device.
class Program {
public:
    class ProgramImpl {
    private:
        static std::string combine(std::vector<std::string> const& opencl_code) {
            std::string out;
            if (opencl_code.size() > 0) {
                out = opencl_code[0];
                for (int i = 1; i < opencl_code.size(); i++) {
                    out += "\n";
                    out += opencl_code[i];
                }
            }
            return out;
        };
        static cl::Program::Sources make_kernel_code(Device_Info const& info, const std::string& opencl_c_code) {
            return cl::Program::Sources{ enable_device_capabilities(info) + "\n" + opencl_c_code };
        };

    public:
        Lockable<cl::Program> cl_program;
        bool initialized = false;

    public:
        // Instantiates and compiles the program.
        ProgramImpl(Device_Info const& info, std::vector<std::string> const& opencl_code)
            : cl_program(info.cl_context, make_kernel_code(info, GL::string(combine(opencl_code)).replace("; ", ";\n").to_string()))
        {
            const std::string build_options
                = std::string("-cl-std=CL") + info.opencl_c_version + std::string(" -cl-finite-math-only -cl-no-signed-zeros -cl-mad-enable") + (info.patch_intel_gpu_above_4gb ? " -cl-intel-greater-than-4GB-buffer-required" : "");
            int error
                = cl_program.get().obj.build(info.cl_device, (build_options + " -w").c_str());
            if (error) {
                print_warning(cl_program.get().obj.getBuildInfo<CL_PROGRAM_BUILD_LOG>(info.cl_device)); // print build log

                auto splits = GL::string(combine(opencl_code)).replace("; ", ";\n").split("\n");
                int l = 1;
                for (auto& split : splits) {
                    std::cout << (GL::string(std::to_string(l++)) + "\t" + split) << std::endl;
                }
            }

            initialized = true;
        }
        ProgramImpl() = delete;
        ProgramImpl(ProgramImpl const&) = delete;
        ProgramImpl(ProgramImpl&&) = delete;
        ProgramImpl& operator=(ProgramImpl const&) = delete;
        ProgramImpl& operator=(ProgramImpl&&) = delete;
        ~ProgramImpl() = default;
    };

private:
    static std::string enable_device_capabilities(Device_Info const& info) {
        return // enable FP64/FP16 capabilities if available
            std::string(info.patch_nvidia_fp16 ? "\n #define cl_khr_fp16" : "") + // Nvidia Pascal and newer GPUs with driver>=520.00 don't report cl_khr_fp16, but do support basic FP16 arithmetic
            std::string(info.patch_legacy_gpu_fma ? "\n #define fma(a, b, c) ((a)*(b)+(c))" : "") + // some old GPUs have terrible fma performance, so replace with a*b+c
            std::string(info.nvidia_compute_capability ? "\n #define cl_nv_compute_capability " + to_string(info.nvidia_compute_capability) : "") + // allows querying Nvidia compute capability for inline PTX
            std::string(info.is_dp4a_capable == 0u ? "\n #undef __opencl_c_integer_dot_product_input_4x8bit\n #undef __opencl_c_integer_dot_product_input_4x8bit_packed" : "") + // patch false dp4a reporting on Intel
            "\n #define cl_workgroup_size " + to_string(WORKGROUP_SIZE) + "u"
            "\n #ifdef cl_khr_fp64"
            "\n #pragma OPENCL EXTENSION cl_khr_fp64 : enable" // make sure cl_khr_fp64 extension is enabled
            "\n #endif"
            "\n #ifdef cl_khr_fp16"
            "\n #pragma OPENCL EXTENSION cl_khr_fp16 : enable" // make sure cl_khr_fp16 extension is enabled
            "\n #endif"
            "\n #ifdef cl_khr_int64_base_atomics"
            "\n #pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable" // make sure cl_khr_int64_base_atomics extension is enabled
            "\n #endif";
    };

public:
    Device_Info
        info;
    ProgramImpl
        program;
    Lockable<cl::CommandQueue>
        queue;
    bool
        initialized = false;

    class kernel_list {
    public:
        GL::bTree<struct _cl_kernel, size_t, 10> functions;

        void push_back(GL::string const& name, cl_kernel new_kernel) {
            if (auto* node = functions.NodeFind(name.hash())) {
                ::clReleaseKernel(node->object);
                node->object = new_kernel;
            }
            else {
                functions.Add(new_kernel, name.hash());
            }
        };
        cl_kernel operator[](GL::string const& name) const {
            if (auto* node = functions.NodeFind(name.hash())) {
                return node->object;
            }
            else {
                return nullptr;
            }
        }

        kernel_list() {};
        ~kernel_list() {
            if (auto* n = functions.GetRoot()) {
                while (n) {
                    if (n->object) {
                        ::clReleaseKernel(n->object);
                    }
                    n = functions.GetNextLeaf(n);
                }
            }
        };

    };

    kernel_list functions;


     Program(std::vector<std::string> const& opencl_c_code)
        : info(select_device_with_most_flops(get_devices(false)))
        , program(info, opencl_c_code)
        , queue(info.cl_context, info.cl_device)
        , initialized(true)
    {}
    Program() = delete;
    Program(Program const&) = delete;
    Program(Program&&) = delete;
    Program& operator=(Program const&) = delete;
    Program& operator=(Program&&) = delete;
    ~Program() = default;

    //inline void barrier(const std::pair<Event*, Event*> event_waitlist = { nullptr, nullptr }, Event* event_returned = nullptr) { cl_queue.enqueueBarrierWithWaitList(event_waitlist, event_returned); }
    //inline void finish_queue() { cl_queue.finish(); }
    inline cl::Context get_cl_context() const { return info.cl_context; }
    //inline cl::Program get_cl_program() const { return cl_program; }
    //inline cl::CommandQueue get_cl_queue() const { return cl_queue; }

};
class opencl {
public:
    static Program& get_program() {
        static Program out({ opencl_impl::create_kernels() });
        return out;
    };
};

struct static_mem_matrix { mem_matrix* ptr; };
class mem_matrix {
    template <typename G> friend class matrix;
public:
    static auto& program() { return opencl::get_program(); };
    
    using dynamic_cpu_allocator = GL::parallel_dynamic_allocator<void*>;
    static dynamic_cpu_allocator& cpu_allocator() { 
        static dynamic_cpu_allocator out(
            [](unsigned long long length) -> void* {
                return Mem_Alloc(length);
            }, // _alloc_block
            [](void*& parent_block) -> void {
                Mem_Free(parent_block);
                parent_block = nullptr;
            } // _free_block
        );
        return out; 
    };

    using dynamic_gpu_allocator = GL::/*parallel_*/dynamic_allocator<cl_mem>;
    static dynamic_gpu_allocator& gpu_allocator() {
        static dynamic_gpu_allocator out(
            [](unsigned long long length) -> cl_mem {
                cl_int err = CL_SUCCESS;
                cl_mem buf = ::clCreateBuffer(program().get_cl_context().get(), CL_MEM_READ_WRITE | ((int)program().info.patch_intel_gpu_above_4gb << 23), length, nullptr, &err);
                if ((err != CL_SUCCESS) || !buf) {
                    ::clReleaseMemObject(buf);
                    return nullptr;
                }
                else {
                    return buf;
                }
            }, // _alloc_block
            [](cl_mem& parent_block) -> void {
                ::clReleaseMemObject(parent_block);
                parent_block = nullptr;
            } // _free_block            
        );
        return out;
    };
    
public:
    template <typename T>
    struct helper {
        // using block_type = dynamic_allocator<void*>::dynamic_block;
        using block_type = dynamic_cpu_allocator::dynamic_block;
        struct array_delete { // default deleter for unique_ptr to array of unknown size
            constexpr array_delete() noexcept = default;
            array_delete(const array_delete&) noexcept {}
            void operator()(T* p) const noexcept { // delete a pointer
                static_assert(0 < sizeof(T), "can't delete an incomplete type");
                auto* ptr = (void*)((::byte*)p - sizeof(block_type));
                auto* alloced = *(block_type**)(::byte*)(ptr);
                unsigned int N = (alloced->length - sizeof(block_type)) / sizeof(T);
                if constexpr (!std::is_pod_v<T>) for (unsigned int i = 0; i < N; ++i) (&p[i])->~T();         
                cpu_allocator().Free(alloced);
            }
        };
        __declspec(noinline) static T* create(unsigned int N) {
            auto* ptr = cpu_allocator().Alloc((sizeof(T) * N) + sizeof(block_type));
            *(block_type**)(::byte*)(ptr->sub_buffer) = ptr;
            T* out = (T*)(void*)((::byte*)ptr->sub_buffer + sizeof(block_type));
            if constexpr (!std::is_pod_v<T>) for (unsigned int i = 0; i < N; ++i) new (&out[i]) T;            
            return out;
        };
        template <typename... U> static T* create_single(U&&... args) {
            unsigned int N = 1;
            auto* ptr = cpu_allocator().Alloc((sizeof(T) * N) + sizeof(block_type));
            *(block_type**)(::byte*)(ptr->sub_buffer) = ptr;
            T* out = (T*)(void*)((::byte*)ptr->sub_buffer + sizeof(block_type));
            if constexpr (!std::is_pod_v<T> || (sizeof...(args) > 0)) new (&out[0]) T(std::move(args)...);
            return out;
        };
    };

    template <typename T>
    static auto make_unique(unsigned int N) {
        return std::unique_ptr<T[], helper<T>::array_delete>(helper<T>::create(N));
    };

    template <typename T, typename... U>
    static auto make_unique_single(U&&... args) {
        return std::unique_ptr<T, helper<T>::array_delete>(helper<T>::create_single(std::move(args)...));
    };

    template <typename T>
    static auto make_shared(unsigned int N) {
        struct helper {
             static void destroy(T* p) noexcept { // delete a pointer
                static_assert(0 < sizeof(T), "can't delete an incomplete type");
                auto* ptr = (void*)((::byte*)p - sizeof(dynamic_cpu_allocator::dynamic_block*));
                auto* alloced = *(dynamic_cpu_allocator::dynamic_block**)(::byte*)(ptr);
                unsigned int N = (alloced->length - sizeof(dynamic_cpu_allocator::dynamic_block*)) / sizeof(T);
                if constexpr (!std::is_pod_v<T>) {
                    for (unsigned int i = 0; i < N; ++i)
                        (&p[i])->~T();
                }
                cpu_allocator().Free(alloced);
            };
             static T* create(unsigned int N) {
                //N = ((N + (WORKGROUP_SIZE - 1)) / WORKGROUP_SIZE) * WORKGROUP_SIZE;
                auto* ptr = cpu_allocator().Alloc((sizeof(T) * N) + sizeof(dynamic_cpu_allocator::dynamic_block*));

                *(dynamic_cpu_allocator::dynamic_block**)(::byte*)(ptr->sub_buffer) = ptr;
                T* out = (T*)(void*)((::byte*)ptr->sub_buffer + sizeof(dynamic_cpu_allocator::dynamic_block*));
                if constexpr (std::is_pod_v<T>) {
                    // best-case scenario!
                    std::memset(out, 0, (sizeof(T) * N));
                }
                else {
                    // need to actually initialize the array...
                    for (unsigned int i = 0; i < N; ++i) {
                        new (&out[i]) T;
                    }
                }
                return out;
            };
        };
        return std::shared_ptr<T[]>(helper::create(N), &helper::destroy);
    };

public:
    class gpu_event {
    public:
        cl_event _ev;

    public:
        gpu_event()
            : _ev{ nullptr } 
        {};
         gpu_event(cl_event ev) // assumes the event comes in fresh & retained already.
            : _ev{ev} 
        {};
         gpu_event(gpu_event const& rhs) // copies
            : _ev{ rhs._ev } 
        {
            if (_ev) ::clRetainEvent(_ev);
        };
         gpu_event(gpu_event && rhs) noexcept // moves
            : _ev{ rhs._ev } 
        {
            rhs._ev = nullptr;
        };
         gpu_event& operator=(gpu_event const& rhs) { // copies        
            if (_ev) {
                ::clReleaseEvent(_ev);
            }
            _ev = rhs._ev;
            if (_ev) ::clRetainEvent(_ev);
            return *this;
        };
         gpu_event& operator=(gpu_event&& rhs) noexcept { // moves        
            if (_ev != nullptr) {
                ::clReleaseEvent(_ev);
            }
            _ev = rhs._ev;
            rhs._ev = nullptr;
            return *this;
        };
         ~gpu_event() {
            if (_ev != nullptr) {
                ::clReleaseEvent(_ev);
            }
        };

    };

    // vector of events which does not shrink -- attempts to re-use the vector whenever possible. 
    class event_list {
    public:
        std::unique_ptr< gpu_event[], mem_matrix::helper<gpu_event>::array_delete>
            items; // leverages the CPU allocator for speed of allocation
        long long 
            len;
        long long 
            reservation;

        void clear() {
            if (len == 0) return;
            if (items) {
                cl_int eventStatus = 0;
                cl_int err = 0;
#if 0
                for (long long L = 0; L < len; ++L) {
                    GL::stopwatch sw;
                    do {
                        err = ::clGetEventInfo(items[L], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(eventStatus), &eventStatus, nullptr);
                        check_for_errors(err);
                        if ((eventStatus > CL_QUEUED) || (eventStatus < CL_COMPLETE) || (sw.check() > 1.0l)) {
                            ::clFlush(program().queue.get().obj.get());
                            ::clFinish(program().queue.get().obj.get());
                        }
                        if (eventStatus == CL_QUEUED || eventStatus == CL_SUBMITTED) std::this_thread::yield();
                    } while (eventStatus != CL_COMPLETE);
                    ::clReleaseEvent(items[L]);
                }
#else
                ::clWaitForEvents(len, &items[0]._ev);
#endif
                len = 0;
            }
        };
        void push_back(gpu_event const& rhs) {
            if (!rhs._ev) return;
            if (reservation == 0) reserve(4);

            if (len >= 64) {
                clear();
            }
            if (len >= reservation) {
                reservation = std::min<long long>(64, reservation * 2);
                auto new_items = make_unique<gpu_event>(reservation);      
                for (int i = 0; i < len; ++i) new_items[i] = std::move(items[i]);
                items = std::move(new_items);
            }
            items[len++] = rhs;
        };
        size_t size() const { return len; };
        size_t capacity() const { return reservation; };
        void reserve(size_t n) {
            n = std::min<long long>(64, n);
            if (reservation < n) {
                reservation = n;
                auto new_items = make_unique<gpu_event>(reservation);
                for (int i = 0; i < len; ++i) new_items[i] = std::move(items[i]);
                items = std::move(new_items);
            }
        };
        gpu_event& operator[](size_t n) { return items[n]; };
        const gpu_event& operator[](size_t n) const { return items[n]; };

        event_list()
            : items{}
            , len{ 0 }
            , reservation{ 0 }
        {};
        event_list(event_list const&) = delete;
        event_list(event_list&& rhs) noexcept
            : items{ std::move(rhs.items) }
            , len{ std::move(rhs.len) }
            , reservation{ std::move(rhs.reservation) }
        {
            rhs.items = nullptr;
            rhs.len = 0;
            rhs.reservation = 0;
        };
        event_list& operator=(event_list const&) = delete;
        event_list& operator=(event_list&& rhs) noexcept {
            this->clear();
            this->items = nullptr;
            this->items = std::move(rhs.items);
            this->len = std::move(rhs.len);
            this->reservation = std::move(rhs.reservation);
            rhs.items = nullptr;
            rhs.len = 0;
            rhs.reservation = 0;
            return *this;
        };
        ~event_list() {
            clear();
            this->items = nullptr;
            len = 0;
            reservation = 0;
        };
    };
    dynamic_gpu_allocator::unique_ptr
        gpu_memory;
    dynamic_cpu_allocator::unique_ptr
        cpu_memory;
    unsigned long long
        len_bytes;
    mutable event_list
        events;

    // immediate
    bool add_event(gpu_event const& ev) const {
        if (!ev._ev) return false;

        // ensure we are not double-adding the event.
        for (int i = 0; i < events.size(); ++i) {
            if (events[i]._ev == ev._ev) {
                return false;
            }
        }
        events.push_back(ev);
        return true;
    };
    // immediate
    void wait_for_events() {
        events.clear(); // waits for events in addition to clearing
    };
    void read_from_device() {
        if (gpu_memory && cpu_memory) {
            events.clear();
            cl_event evP;            
            cl_int err =
                ::clEnqueueReadBuffer(
                    program().queue.get().obj.get(), gpu_memory->sub_buffer, true, 0, len_bytes, cpu_memory->sub_buffer,
                    0, nullptr, &evP
                );
            check_for_errors(err);
            gpu_event ev(evP);

#if 0            
            cl_int eventStatus = 0;
            GL::stopwatch sw;
            do {
                err = ::clGetEventInfo(ev, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(eventStatus), &eventStatus, nullptr);
                check_for_errors(err);
                if ((eventStatus > CL_QUEUED) || (eventStatus < CL_COMPLETE) || (sw.check() > 1.0l)) {
                    ::clFlush(program().queue.get().obj.get());
                    ::clFinish(program().queue.get().obj.get());
                }
                if (eventStatus == CL_QUEUED || eventStatus == CL_SUBMITTED) std::this_thread::yield();
            } while (eventStatus != CL_COMPLETE);
            check_for_errors(err);
            ::clReleaseEvent(ev);  
#else
            err = ::clWaitForEvents(1, &ev._ev);
            check_for_errors(err);
#endif
        }        
    };
    void write_to_device() {
        if (gpu_memory && cpu_memory) {
            events.clear();

            
            cl_event evP;
            cl_int err =
                ::clEnqueueWriteBuffer(
                    program().queue.get().obj.get(), gpu_memory->sub_buffer, false, 0, len_bytes, cpu_memory->sub_buffer,
                    0, nullptr, &evP
                );
            gpu_event ev(evP);


            check_for_errors(err);
            err = ::clWaitForEvents(1, &ev._ev);
            check_for_errors(err);
        }
    };
    // immediate
    void ensure_host_mem_exists() {
        if (!cpu_memory) {
            cpu_memory = cpu_allocator().make_unique(len_bytes);
        }
    };
    // immediate
    void ensure_device_mem_exists() {
        if (!gpu_memory) {
            gpu_memory = gpu_allocator().make_unique(len_bytes);
        }
    };

    mem_matrix(unsigned long long bytes, const bool allocate_cpu = true, const bool allocate_gpu = true)
        : len_bytes{ bytes }
        , cpu_memory{ allocate_cpu ? cpu_allocator().make_unique(bytes) : nullptr }
        , gpu_memory{ allocate_gpu ? gpu_allocator().make_unique(bytes) : nullptr }
        , events{}
    {};
    mem_matrix()
        : len_bytes{ 0 }
        , cpu_memory{ nullptr }
        , gpu_memory{ nullptr }
        , events{}
    {};
    mem_matrix(mem_matrix const&) = delete;
    mem_matrix(mem_matrix&& rhs) noexcept
        : len_bytes{ std::move(rhs.len_bytes) }
        , cpu_memory{ std::move(rhs.cpu_memory) }
        , gpu_memory{ std::move(rhs.gpu_memory) }
        , events{ std::move(rhs.events) }
    {
        rhs.len_bytes = 0;
        rhs.cpu_memory = nullptr;
        rhs.gpu_memory = nullptr;
        rhs.events.clear();
    };
    mem_matrix& operator=(mem_matrix const&) = delete;
    mem_matrix& operator=(mem_matrix&& rhs) noexcept {
        events.clear();

        len_bytes = std::move(rhs.len_bytes);
        cpu_memory = std::move(rhs.cpu_memory);
        gpu_memory = std::move(rhs.gpu_memory);
        events = std::move(rhs.events);
        rhs.len_bytes = 0;
        rhs.cpu_memory = nullptr;
        rhs.gpu_memory = nullptr;
        rhs.events.clear();
        return *this;
    };
    ~mem_matrix() {
        events.clear();
        cpu_memory = nullptr;
        gpu_memory = nullptr;
        len_bytes = 0;
    };

    template <typename T = void> T* cpu_data() {
        if (cpu_memory) {
            ensure_host_mem_exists();
            return reinterpret_cast<T*>(cpu_memory->sub_buffer);
        }
        else {
            return nullptr;
        }
    };
    cl_mem gpu_data() {
        if (gpu_memory) {
            ensure_device_mem_exists();
            return gpu_memory->sub_buffer;
        }
        else {
            return nullptr;
        }
    };
    static auto& get_program() {
        return program();
    };

private:
    static void append_events(int& count, gpu_event const& ev) {}
    template<class T, class... U> static void append_events(int& count, gpu_event const& ev, const T& parameter, const U&... parameters) {
        if constexpr (std::is_same_v<T, mem_matrix>) {
            if (parameter.add_event(ev)) {
                ++count;
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<mem_matrix>>) {
            if (parameter) {
                if (parameter->add_event(ev)) {
                    ++count;
                }
            }
        }
        else if constexpr (std::is_same_v<T, std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>>) {
            if (parameter) {
                if (parameter->add_event(ev)) {
                    ++count;
                }
            }
        }
        append_events(count, ev, parameters...);
    };
    static void check_for_errors(const int error) {
        if (error == -48) print_error("There is no OpenCL kernel in the OpenCL C code! Check spelling!");
        if (error<-48 && error>-53) print_error("Parameters for OpenCL kernel don't match between C++ and OpenCL C!");
        if (error == -54) print_error("Workgrop size " + to_string(WORKGROUP_SIZE) + " for OpenCL kernel is invalid!");
        if (error != 0) print_error("OpenCL kernel failed with error code " + to_string(error) + "!");
    };
    template<typename T> static void link_parameter(cl_kernel const& cl_kernel, const uint position, const T& constant) {
        check_for_errors(::clSetKernelArg(cl_kernel, position, sizeof(T), (void*)&constant));
    };
    static void link_parameters(cl_kernel const& cl_kernel, std::array<gpu_event, 512>& waitlist, int& waitlistSize, const uint starting_position) { }
    template<template<class> typename G, typename T, class... U>
     static void link_parameters(cl_kernel const& cl_kernel, std::array<gpu_event, 512>& waitlist, int& waitlistSize, const uint starting_position, const G<T>& parameter, const U&... parameters) {
        if constexpr (std::is_same_v<G<T>, std::shared_ptr<T>>) {
            check_for_errors(::clSetKernelArgSVMPointer(cl_kernel, starting_position, parameter.get()));
        }
        else {
            link_parameter(cl_kernel, starting_position, parameter);
        }
        link_parameters(cl_kernel, waitlist, waitlistSize, starting_position + 1u, parameters...);
    };
    template<class G, class... U>
     static void link_parameters(cl_kernel const& cl_kernel, std::array<gpu_event, 512>& waitlist, int& waitlistSize, const uint starting_position, const G& parameter, const U&... parameters) {
        if constexpr (std::is_same_v<G, mem_matrix>) {
            if (parameter.gpu_memory) {
                link_parameter(cl_kernel, starting_position, parameter.gpu_memory->sub_buffer);
                cl_int eventStatus = 0; cl_int err = 0;
                for (size_t i = 0; i < parameter.events.size(); ++i) {
                    size_t j = 0;

                    if (!parameter->events[i]._ev) continue;
                        //err = ::clGetEventInfo(parameter->events[i]._ev, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(eventStatus), &eventStatus, nullptr);
                        //check_for_errors(err);
                        //if (eventStatus == CL_COMPLETE) {
                        //    continue;
                        //}
                    for (j = 0; j < waitlistSize; ++j) if (waitlist[j] == parameter.events[i]) break;                    
                    if (j == waitlistSize) waitlist[waitlistSize++] = parameter.events[i];
                    if (waitlistSize >= 512) throw std::runtime_error("Too many events are queued for completion -- increase the size of the queue list");
                }
            }
            else {                
                throw std::runtime_error(GL::printf("Parameter %i was empty in call to GPU-accelerated function", (int)starting_position).to_string());
            }
        }
        else if constexpr (std::is_same_v<G, std::unique_ptr<mem_matrix>>) {
            if (parameter && parameter->gpu_memory) {
                link_parameter(cl_kernel, starting_position, parameter->gpu_memory->sub_buffer);
                cl_int eventStatus = 0; cl_int err = 0;
                for (size_t i = 0; i < parameter->events.size(); ++i) {
                    size_t j = 0;

                    if (!parameter->events[i]._ev) continue;
                        //err = ::clGetEventInfo(parameter->events[i]._ev, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(eventStatus), &eventStatus, nullptr);
                        //check_for_errors(err);
                        //if (eventStatus == CL_COMPLETE) {
                        //    continue;
                        //}
                    for (j = 0; j < waitlistSize; ++j) if (waitlist[j]._ev == parameter->events[i]._ev) break;                    
                    if (j == waitlistSize) waitlist[waitlistSize++] = parameter->events[i];
                    if (waitlistSize >= 512) throw std::runtime_error("Too many events are queued for completion -- increase the size of the queue list");                    
                }
            }
            else throw std::runtime_error(GL::printf("Parameter %i was empty in call to GPU-accelerated function", (int)starting_position).to_string());            
        }
        else if constexpr (std::is_same_v<G, std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>>) {
            if (parameter && parameter->gpu_memory) {
                link_parameter(cl_kernel, starting_position, parameter->gpu_memory->sub_buffer);
                cl_int eventStatus = 0; cl_int err = 0;
                for (size_t i = 0; i < parameter->events.size(); ++i) {
                    size_t j = 0;

                    if (!parameter->events[i]._ev) continue;
                        //err = ::clGetEventInfo(parameter->events[i]._ev, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(eventStatus), &eventStatus, nullptr);
                        //check_for_errors(err);
                        //if (eventStatus == CL_COMPLETE) {
                        //    continue;
                        //}
                    for (j = 0; j < waitlistSize; ++j) if (waitlist[j]._ev == parameter->events[i]._ev) break;                    
                    if (j == waitlistSize) waitlist[waitlistSize++] = parameter->events[i];
                    if (waitlistSize >= 512) throw std::runtime_error("Too many events are queued for completion -- increase the size of the queue list");                    
                }
            }
            else throw std::runtime_error(GL::printf("Parameter %i was empty in call to GPU-accelerated function", (int)starting_position).to_string());            
        }
        else if constexpr (std::is_same_v<G, static_mem_matrix>) {
            if (parameter.ptr && parameter.ptr->gpu_memory) {
                link_parameter(cl_kernel, starting_position, parameter.ptr->gpu_memory->sub_buffer);
            }
            else {
                throw std::runtime_error(GL::printf("Parameter %i was empty in call to GPU-accelerated function", (int)starting_position).to_string());
            }
        }
        else {
            link_parameter(cl_kernel, starting_position, parameter);
        }
        link_parameters(cl_kernel, waitlist, waitlistSize, starting_position + 1u, parameters...);
    };

public:
    // for a given name and count (with optional params, including either "mem_matrix const&" or POD-types) it will queue a GPU kernel for completion. 
    template<class... T> static void queue_gpu_work(GL::string const& name, unsigned long long count, const T&... parameters) {
        static std::array<gpu_event, 512>
            waitlist{};
        auto& prog
            = mem_matrix::program();
        auto kernel
            = prog.functions[name];
        cl_int err
            = 0;
        
        cl::NDRange
            cl_range_global = cl::NDRange(((count + WORKGROUP_SIZE - 1ull) / WORKGROUP_SIZE) * WORKGROUP_SIZE),
            cl_range_local = cl::NDRange(WORKGROUP_SIZE);
        int waitlist_size 
            = 0;

        if (!kernel) {
            prog.functions.push_back(name, ::clCreateKernel(prog.program.cl_program.get().obj.get(), name.c_str().data(), &err));
            check_for_errors(err);
            kernel = prog.functions[name];
        }
        link_parameters(kernel, waitlist, waitlist_size, 0u, parameters...); // expand variadic template to link kernel parameters
        cl_event evP;
        err = ::clEnqueueNDRangeKernel(
            prog.queue.get().obj.get(), kernel, (cl_uint)cl_range_global.dimensions(),
            nullptr, (const size_t*)cl_range_global,
            cl_range_local.dimensions() != 0 ? (const size_t*)cl_range_local : nullptr,
            (cl_int)waitlist_size,
            (cl_event*)((waitlist_size > 0) ? &waitlist[0]._ev : nullptr),
            &evP);
        gpu_event
            tmp(evP);

        check_for_errors(err);
        int C = 0;
        append_events(C, tmp, parameters...);
        if (C == 0) { // makes copies for each reference to a mem_matrix
            event_list list;
            for (int i = 0; i < waitlist_size; ++i) {
                list.push_back(waitlist[i]);
            }
            list.clear();
            list.push_back(tmp);
            list.clear();
        }
    };
};

// implement the GPU-accelerated interface for matrix<T>
namespace GL {
    namespace GPU {
        template <typename T, size_t capacity>
        class deferred_deletion_support {
        private:
            GL::thread_object_no_default<std::pair<size_t, std::array<T, capacity>>> vecs;
        public:
             void push_back(T&& arg) {
                std::pair<size_t, std::array<T, capacity>>& PR = *vecs;
                // size_t pos = PR.first++ % capacity;
                size_t pos = PR.first++ & (capacity - 1); // faster than %, must be power of two
                auto& vec = PR.second;
                if (pos > capacity) pos = 0;
                vec[pos] = nullptr;
                vec[pos] = std::move(arg);
            };
        };

        template <typename T> static std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>& mem(matrix<T> const& rhs) {
            return reinterpret_cast<std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>&>(const_cast<matrix<T>&>(rhs).internal_memory());
        };
        static unsigned int WorkgroupAdjustment(unsigned int N) { 
            return ((N + (WORKGROUP_SIZE - 1)) / WORKGROUP_SIZE) * WORKGROUP_SIZE; 
        };
        static auto& mem_free_helper() {
            class wrap {
            public:
                std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete> mem;
                wrap() = default;
                wrap(std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>&& rhs) : mem{ std::move(rhs) } {};
                wrap(wrap const&) = delete;
                wrap(wrap&&) = default;
                wrap& operator=(wrap const&) = delete;
                wrap& operator=(wrap&&) = default;
                ~wrap() = default;
            };
            static GL::atomic_epoch_allocator<wrap, GL::atomic_allocator<wrap, 128, false, false>, 32> allocator{};
            return allocator;
        }
        static void mem_free(std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>& mem) {
            if (mem) {
                if (/*mem->cpu_memory || */mem->gpu_memory) {
                    static deferred_deletion_support< std::unique_ptr<mem_matrix, mem_matrix::helper< mem_matrix>::array_delete>, 256> helper;
                    helper.push_back(std::move(mem));

                    //auto* p = mem_free_helper().Alloc(std::move(mem));
                    //mem_free_helper().ProtectCurrentEpoch_Fast(); // increments the epoch and protects the next free() call
                    //mem_free_helper().Free(p);
                }
            }
            mem = nullptr;            
        };

        template <typename T> matrix<T>::matrix(GL::GPU::dimensions d)
            : dim{ d.ensure() }
            , memory(nullptr)
        {
            mem(*this) = mem_matrix::make_unique_single< mem_matrix>(sizeof(T) * WorkgroupAdjustment(d.X * d.Y * d.Z), false, true);
        };
        template <typename T> matrix<T>::matrix(unsigned int X, unsigned int Y, unsigned int Z, bool cpu_only)
            : dim{ GL::GPU::dimensions{ X, Y, Z }.ensure() }
            , memory(nullptr)
        {
            mem(*this) = mem_matrix::make_unique_single< mem_matrix>(sizeof(T) * WorkgroupAdjustment(X * Y * Z), cpu_only, !cpu_only);
        };
        template <typename T> matrix<T>::matrix()
            : dim{ 0, 0, 0 }
            , memory(nullptr)
        {}
        template <typename T> matrix<T>::matrix(matrix&& rhs) noexcept : dim{ std::move(rhs.dim) }, memory(nullptr) {
            mem(*this) = std::move(mem(rhs));
            mem(rhs) = nullptr;
        };
        template <typename T> matrix<T>::matrix(matrix const& rhs)
            : dim{ rhs.dim }
            , memory(nullptr)
        {
            static auto func_name{ GL::string("copy") + GL::string(opencl_impl::type_name<T>()) };

            mem(*this) = mem_matrix::make_unique_single< mem_matrix>(sizeof(T) * WorkgroupAdjustment(rhs.dim.count()), (bool)mem(rhs)->cpu_memory, (bool)mem(rhs)->gpu_memory);
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), mem(rhs)
            );
        };
        template <typename T> matrix<T>& matrix<T>::operator=(matrix const& rhs) {
            static auto func_name{ GL::string("copy") + GL::string(opencl_impl::type_name<T>()) };

            dim = rhs.dim;
            auto mem2 = mem_matrix::make_unique_single< mem_matrix>(sizeof(T) * WorkgroupAdjustment(rhs.dim.count()), false, true);
            mem_matrix::queue_gpu_work(func_name,
                rhs.dim.count(),
                mem2, mem(rhs)
            );
            mem_free(mem(*this));
            mem(*this) = std::move(mem2);
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator=(matrix&& rhs) noexcept {
            mem_free(mem(*this));
            dim = rhs.dim;
            mem(*this) = std::move(mem(rhs));
            mem(rhs) = nullptr;
            return *this;
        };
        template <typename T> matrix<T>::~matrix() {
            mem_free(mem(*this));
        };
        template <typename T> unsigned int matrix<T>::size() const {
            return dim.count();
        };
        template <typename T> unsigned int matrix<T>::size(unsigned int d) const {
            switch (d) {
            case 0: return dim.X;
            case 1: return dim.Y;
            case 2: return dim.Z;
            default: return 0;
            }
        };
        template <typename T> matrix<T>::reader::reader(matrix<T>& copy, GL::GPU::dimensions const& D) : data{ nullptr }, dim(D) {
            static auto func_name{ GL::string("copy_slice") + GL::string(opencl_impl::type_name<T>()) };

            if (mem(copy)->gpu_memory) {
                if (mem_matrix::get_program().info.svm_memory_allowed) {
                    data = std::shared_ptr<T[]>(
                        (T*)::clSVMAlloc(mem(copy)->program().get_cl_context().get(), CL_MEM_READ_WRITE,
                            sizeof(T) * WorkgroupAdjustment(dim.count())
                            , WORKGROUP_SIZE),
                        [](T* p) {
                            ::clSVMFree(mem_matrix::program().get_cl_context().get(), p);
                        }
                    );
                    mem_matrix::queue_gpu_work(func_name,
                        dim.count(),
                        data, mem(copy), (unsigned int)0
                    );
                    mem(copy)->events.clear();
                }
                else {
                    mem(copy)->ensure_host_mem_exists();
                    mem(copy)->read_from_device();
                    data = std::shared_ptr<T[]>(mem(copy)->cpu_data<T>(), [](T*) { /* do nothing */ });
                }
            }
            else {
                mem(copy)->ensure_host_mem_exists();
                mem(copy)->read_from_device();
                data = std::shared_ptr<T[]>(mem(copy)->cpu_data<T>(), [](T*) { /* do nothing */ });
            }
        };
        template <typename T> matrix<T>::reader::reader(reader&& rhs) noexcept : data(rhs.data), dim(rhs.dim) {};
        template <typename T> matrix<T>::reader::operator bool() const {
            return data.get() ? true : false;
        };
        template <typename T> T const& matrix<T>::reader::operator[](unsigned int X) const {
            return data[X];
        };
        template <typename T> T const& matrix<T>::reader::operator()(unsigned int X, unsigned int Y, unsigned int Z) const {
            return data[(Z * dim.X * dim.Y) + (Y * dim.X) + X];
        };
        template <typename T> std::shared_ptr<T[]> matrix<T>::reader::get() const {
            return data;
        };
        template <typename T> matrix<T>::writer::writer(matrix<T>& copy, GL::GPU::dimensions const& D, bool cpu_only) : gpu_cpu_data{ nullptr }, cpu_data{ nullptr }, data(&copy), dim(D), _cpu_only(cpu_only) {
            static auto func_name{ GL::string("copy_slice") + GL::string(opencl_impl::type_name<T>()) };

            if (_cpu_only) {
                mem(*data)->ensure_host_mem_exists();
                mem(*data)->read_from_device();
                cpu_data = mem(*data)->cpu_data<T>();
            }
            else {
                if (mem_matrix::get_program().info.svm_memory_allowed) {
                    gpu_cpu_data = std::shared_ptr<T[]>(
                        (T*)::clSVMAlloc(mem(copy)->program().get_cl_context().get(), CL_MEM_READ_WRITE,
                            sizeof(T) * WorkgroupAdjustment(dim.count())
                            , WORKGROUP_SIZE),
                        [](T* p) {
                            ::clSVMFree(mem_matrix::program().get_cl_context().get(), p);
                        }
                    );
                    mem_matrix::queue_gpu_work(func_name,
                        dim.count(),
                        gpu_cpu_data, mem(copy), (unsigned int)0
                    );
                    mem(copy)->events.clear();
                }
                else {
                    mem(*data)->ensure_host_mem_exists();
                    mem(*data)->read_from_device();
                    cpu_data = mem(*data)->cpu_data<T>();
                }
            }
        };
        template <typename T> matrix<T>::writer::writer(writer&& rhs) noexcept : gpu_cpu_data{ rhs.gpu_cpu_data }, cpu_data{ rhs.cpu_data }, data(rhs.data), dim(rhs.dim), _cpu_only(rhs._cpu_only) {
            rhs.gpu_cpu_data = nullptr;
            rhs.data = nullptr;
            rhs.cpu_data = nullptr;
            rhs.dim = GL::GPU::dimensions{ 0,0,0 };
            rhs._cpu_only = false;
        };
        template <typename T> matrix<T>::writer::~writer() {
            static auto func_name{ GL::string("copy_slice") + GL::string(opencl_impl::type_name<T>()) };
            if (data) {
                if (_cpu_only && cpu_data) mem(*data)->write_to_device();
                else {
                    if (mem_matrix::get_program().info.svm_memory_allowed) {
                        mem_matrix::queue_gpu_work(func_name,
                            dim.count(),
                            mem(*data), gpu_cpu_data, (unsigned int)0
                        );
                        mem(*data)->events.clear();
                    }
                    else {
                        mem(*data)->write_to_device();
                    }


                }
            }
        };
        template <typename T> matrix<T>::writer::operator bool() const {
            if (_cpu_only) {
                return cpu_data;
            }
            else {
                if (mem_matrix::get_program().info.svm_memory_allowed) {
                    return gpu_cpu_data.get() ? true : false;
                }
                else {
                    return cpu_data;
                }

            }
        };
        template <typename T> T& matrix<T>::writer::operator[](unsigned int X) const {
            if (_cpu_only) {
                return cpu_data[X];
            }
            else {
                if (mem_matrix::get_program().info.svm_memory_allowed) {
                    return gpu_cpu_data[X];
                }
                else {
                    return cpu_data[X];
                }
            }
        };
        template <typename T> T& matrix<T>::writer::operator()(unsigned int X, unsigned int Y, unsigned int Z) const {
            if (_cpu_only) {
                return cpu_data[(Z * dim.X * dim.Y) + (Y * dim.X) + X];
            }
            else {
                if (mem_matrix::get_program().info.svm_memory_allowed) {
                    return gpu_cpu_data[(Z * dim.X * dim.Y) + (Y * dim.X) + X];
                }
                else {
                    return cpu_data[(Z * dim.X * dim.Y) + (Y * dim.X) + X];
                }
            }
        };
        template <typename T> typename matrix<T>::reader  matrix<T>::read() const {
            return reader(const_cast<matrix<T>&>(*this), dim);
        };
        template <typename T> typename matrix<T>::writer  matrix<T>::write(bool cpu_only) {
            return writer(const_cast<matrix<T>&>(*this), dim, cpu_only);
        };
        template <typename T> matrix<T>& matrix<T>::operator=(T rhs) {
            static auto func_name{ GL::string("copy_single") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), rhs
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator+=(T rhs) {
            static auto func_name{ GL::string("add_single_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), rhs
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator-=(T rhs) {
            static auto func_name{ GL::string("sub_single_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), rhs
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator*=(T rhs) {
            static auto func_name{ GL::string("mult_single_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), rhs
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator/=(T rhs) {
            static auto func_name{ GL::string("divide_single_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), rhs
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator+=(matrix const& rhs) {
            static auto func_name{ GL::string("add_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), mem(rhs)
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator-=(matrix const& rhs) {
            static auto func_name{ GL::string("sub_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), mem(rhs)
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator*=(matrix const& rhs) {
            static auto func_name{ GL::string("mult_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), mem(rhs)
            );
            return *this;
        };
        template <typename T> matrix<T>& matrix<T>::operator/=(matrix const& rhs) {
            static auto func_name{ GL::string("divide_inplace") + GL::string(opencl_impl::type_name<T>()) };
            mem_matrix::queue_gpu_work(func_name,
                dim.count(),
                mem(*this), mem(rhs)
            );
            return *this;
        };

        template <typename U> GL::GPU::matrix<U> operator+(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("add") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator-(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("sub") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator*(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("mult") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator/(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("divide") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator+(GL::GPU::matrix<U> const& lhs, U rhs) {
            static auto func_name{ GL::string("add_single") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator-(GL::GPU::matrix<U> const& lhs, U rhs) {
            static auto func_name{ GL::string("sub_single") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator*(GL::GPU::matrix<U> const& lhs, U rhs) {
            static auto func_name{ GL::string("mult_single") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator/(GL::GPU::matrix<U> const& lhs, U rhs) {
            static auto func_name{ GL::string("divide_single") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator+(U rhs, GL::GPU::matrix<U> const& lhs) {
            static auto func_name{ GL::string("add_single") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator-(U rhs, GL::GPU::matrix<U> const& lhs) {
            static auto func_name{ GL::string("sub_single_inv") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator*(U rhs, GL::GPU::matrix<U> const& lhs) {
            static auto func_name{ GL::string("mult_single") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator/(U rhs, GL::GPU::matrix<U> const& lhs) {
            static auto func_name{ GL::string("divide_single_inv") + GL::string(opencl_impl::type_name<U>()) };

            auto out = GL::GPU::matrix<U>(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), rhs, mem(out)
            );
            return out;
        };

        template <typename T> template<typename G> matrix<G> matrix<T>::cast() const {
            if constexpr (std::is_same_v<G, T>) {
                return *this; // makes an implicit copy
            }
            else {
                static GL::string CastFunc{ GL::string("from_") + GL::string(opencl_impl::type_name<T>()) + GL::string(opencl_impl::type_name<G>()) }; // from_int
                auto out = matrix<G>(this->dim);
                mem_matrix::queue_gpu_work(CastFunc,
                    out.size(),
                    mem(out), mem(*this)
                );
                return out;
            }
        };
        template <typename T> matrix<T> matrix<T>::random(unsigned int X, unsigned int Y, unsigned int Z) { 
            static auto func_name{ GL::string("Rand") + GL::string(opencl_impl::type_name<T>()) };
            if constexpr (std::is_floating_point_v<T>) {
#if 1 // should result in better quality, but is performed on the CPU rather than the GPU. 
                matrix out(X, Y, Z);
                if (auto w = out.write()) {
                    for (int i = 0; i < out.size(); ++i) {
                        w[i] = GL::util::rand_fast(0, 1);
                    }
                }
                return out;
#else // perform the random on the GPU directly, which requires contention and locking
                matrix out(X, Y, Z);
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(out)
                );
                return out;
#endif
            }
            else {
                return (matrix<float>::random(X, Y, Z) * (float)std::numeric_limits<T>::max()).cast<T>();
            }
        };
        template <typename T> matrix<T> matrix<T>::random_between(T lower, T upper, unsigned int X, unsigned int Y, unsigned int Z) {
            static auto func_name{ GL::string("Rand") + GL::string(opencl_impl::type_name<T>()) };
            if constexpr (std::is_floating_point_v<T>) {
#if 1 // should result in better quality, but is performed on the CPU rather than the GPU. 
                matrix out(X, Y, Z);
                if (auto w = out.write()) {
                    for (unsigned int i = 0; i < out.size(); ++i) {
                        w[i] = (float)GL::util::rand_fast(lower, upper);
                    }
                }
                return out;
#else // perform the random on the GPU directly, which requires contention and locking
                matrix out(GL::GPU::dimensions{ X, Y, Z });
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(out)
                );
                out *= (upper - lower);
                out += lower;
                return out;
#endif
            }
            else {
                return matrix<float>::random_between((float)lower, (float)upper, X, Y, Z).cast<T>();
            }
        };
        template <typename T> matrix<T> matrix<T>::identity(unsigned int width) {
            static auto func_name{ GL::string("identity") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(GL::GPU::dimensions{ width, width, 1 });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), width
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::linear(T low, T high, unsigned int lenX, unsigned int lenY, unsigned int lenZ) {
            static auto func_name{ GL::string("linear_between") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(GL::GPU::dimensions{ lenX, lenY, lenZ });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), low, high, out.size()
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::constant(T value, unsigned int lenX, unsigned int lenY, unsigned int lenZ) {
            static auto func_name{ GL::string("copy_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(GL::GPU::dimensions{ lenX, lenY, lenZ });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), value
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::from_vector(const std::vector<T>& parameters) {
            unsigned int count = parameters.size();
            matrix out(GL::GPU::dimensions{ count, 1, 1 });
            count = 0;
            if (auto W = out.write())
                std::memcpy((T*)(&W[0]), (T*)(&parameters[0]), parameters.size() * sizeof(T));
            return out;
        };
        template <typename T> matrix<T> matrix<T>::from_vector(const std::vector<T>& parameters, unsigned int LenX) {
            unsigned int count = parameters.size();
            matrix out(GL::GPU::dimensions{ LenX, count / LenX, 1 });
            count = 0;
            if (auto W = out.write())
                std::memcpy((T*)(&W[0]), (T*)(&parameters[0]), parameters.size() * sizeof(T));
            return out;
        };
        template <typename T> matrix<T> matrix<T>::pown(matrix<int> const& rhs) const {
            static auto func_name{ GL::string("power_n") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem<int>(rhs), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::pow(matrix const& rhs) const {
            static auto func_name{ GL::string("power") + GL::string(opencl_impl::type_name<T>()) };

            if constexpr (std::is_same_v<T, int>) return pown(rhs);
            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(rhs), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::pown(int rhs) const {
            static auto func_name{ GL::string("power_n_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::pow(T rhs) const {
            static auto func_name{ GL::string("power_single") + GL::string(opencl_impl::type_name<T>()) };

            if constexpr (std::is_same_v<T, int>) return pown(rhs);
            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template <typename T> matrix<float> matrix<T>::sqrt() const {
            static auto func_name{ GL::string("square_root") + GL::string(opencl_impl::type_name<float>()) };

            matrix<float> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::round() const {
            static auto func_name{ GL::string("round") + GL::string(opencl_impl::type_name<T>()) };

            if constexpr (std::is_floating_point_v<T>) {
                matrix out(this->dim);
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(*this), mem(out)
                );
                return out;
            }
            else {
                return *this;
            }
        };
        template <typename T> matrix<T> matrix<T>::ceil() const {
            static auto func_name{ GL::string("ceil") + GL::string(opencl_impl::type_name<T>()) };

            if constexpr (std::is_floating_point_v<T>) {
                matrix out(this->dim);
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(*this), mem(out)
                );
                return out;
            }
            else {
                return *this;
            }
        };
        template <typename T> matrix<T> matrix<T>::floor() const {
            static auto func_name{ GL::string("flr") + GL::string(opencl_impl::type_name<T>()) };

            if constexpr (std::is_floating_point_v<T>) {
                matrix out(this->dim);
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(*this), mem(out)
                );
                return out;
            }
            else {
                return *this;
            }
        };
        template <typename T> matrix<T> matrix<T>::fma(matrix const& multiply, matrix const& add) const {
            static auto func_name{ GL::string("mult_add") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(multiply), mem(add), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::abs() const {
            static auto func_name{ GL::string("absolute") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::cos() const {
            static auto func_name{ GL::string("Cos") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::sin() const {
            static auto func_name{ GL::string("Sin") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::tan() const {
            static auto func_name{ GL::string("Tan") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::acos() const {
            static auto func_name{ GL::string("aCos") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::asin() const {
            static auto func_name{ GL::string("aSin") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::atan() const {
            static auto func_name{ GL::string("aTan") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::cosh() const {
            static auto func_name{ GL::string("Cosh") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::sinh() const {
            static auto func_name{ GL::string("Sinh") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::tanh() const {
            static auto func_name{ GL::string("Tanh") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::acosh() const {
            static auto func_name{ GL::string("aCosh") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::asinh() const {
            static auto func_name{ GL::string("aSinh") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::atanh() const {
            static auto func_name{ GL::string("aTanh") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::exp() const {
            static auto func_name{ GL::string("Exp") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::exp2() const {
            static auto func_name{ GL::string("Exp2") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::exp10() const {
            static auto func_name{ GL::string("Exp10") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::expm1() const {
            static auto func_name{ GL::string("Expm1") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::lgamma() const {
            static auto func_name{ GL::string("Lgamma") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::log() const {
            static auto func_name{ GL::string("Log") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::log2() const {
            static auto func_name{ GL::string("Log2") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::log10() const {
            static auto func_name{ GL::string("Log10") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::log1p() const {
            static auto func_name{ GL::string("Log1p") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(out)
            );
            return out;
        };
        template <typename U> GL::GPU::matrix<U> operator%(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            return lhs.mod(rhs);
        };
        template <typename U> GL::GPU::matrix<U> operator%(GL::GPU::matrix<U> const& lhs, U rhs) {
            return lhs.mod(rhs);
        };
        template <typename T> matrix<T> matrix<T>::mod(T rhs) const {
            static auto func_name{ GL::string("Mod_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::mod(matrix const& rhs) const {
            static auto func_name{ GL::string("Mod") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename T> matrix<T> matrix<T>::max(matrix const& rhs) const {
            static auto func_name{ GL::string("Max") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename T> matrix<T> matrix<T>::max(T rhs) const {
            static auto func_name{ GL::string("Max_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template<typename T> matrix<T> matrix<T>::min(matrix const& rhs) const {
            static auto func_name{ GL::string("Min") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename T> matrix<T> matrix<T>::min(T rhs) const {
            static auto func_name{ GL::string("Min_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator!() const {
            static auto func_name{ GL::string("item_not") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator==(T rhs) const {
            static auto func_name{ GL::string("item_eq_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator!=(T rhs) const {
            static auto func_name{ GL::string("item_neq_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator<(T rhs) const {
            static auto func_name{ GL::string("item_ls_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator<=(T rhs) const {
            static auto func_name{ GL::string("item_lse_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator>(T rhs) const {
            static auto func_name{ GL::string("item_gr_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator>=(T rhs) const {
            static auto func_name{ GL::string("item_gre_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(*this), rhs, mem(out)
            );
            return out;
        };

        template<typename U> static GL::GPU::matrix<unsigned int> operator==(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("item_eq") + GL::string(opencl_impl::type_name<U>()) };
            
            GL::GPU::matrix<unsigned int> out(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename U> static GL::GPU::matrix<unsigned int> operator!=(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("item_neq") + GL::string(opencl_impl::type_name<U>()) };

            GL::GPU::matrix<unsigned int> out(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename U> static GL::GPU::matrix<unsigned int> operator<(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("item_ls") + GL::string(opencl_impl::type_name<U>()) };

            GL::GPU::matrix<unsigned int> out(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename U> static GL::GPU::matrix<unsigned int> operator<=(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("item_lse") + GL::string(opencl_impl::type_name<U>()) };

            GL::GPU::matrix<unsigned int> out(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename U> static GL::GPU::matrix<unsigned int> operator>(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("item_gr") + GL::string(opencl_impl::type_name<U>()) };

            GL::GPU::matrix<unsigned int> out(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };
        template<typename U> static GL::GPU::matrix<unsigned int> operator>=(GL::GPU::matrix<U> const& lhs, GL::GPU::matrix<U> const& rhs) {
            static auto func_name{ GL::string("item_gre") + GL::string(opencl_impl::type_name<U>()) };

            GL::GPU::matrix<unsigned int> out(lhs.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(lhs), mem(rhs), mem(out)
            );
            return out;
        };

        template<typename T> matrix<unsigned int> matrix<T>::operator&&(T rhs) const {
            static auto func_name{ GL::string("item_AND_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), rhs
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator&&(matrix const& rhs) const {
            static auto func_name{ GL::string("item_AND") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), mem(rhs)
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator||(T rhs) const {
            static auto func_name{ GL::string("item_OR_single") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), rhs
            );
            return out;
        };
        template<typename T> matrix<unsigned int> matrix<T>::operator||(matrix const& rhs) const {
            static auto func_name{ GL::string("item_OR") + GL::string(opencl_impl::type_name<T>()) };

            matrix<unsigned int> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), mem(rhs)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::join(unsigned int jdim, matrix const& first) const {
            // All dimensions except join dimension must be equal
            for (unsigned int I = 0; I < 3; ++I) {
                if (I == jdim) continue;
                if (this->size(I) != first.size(I)) {
                    return matrix();
                }
            }

            // Compute output dims
            unsigned int
                NewX = this->size(0) + first.size(0) * (jdim == 0),
                NewY = this->size(1) + first.size(1) * (jdim == 1),
                NewZ = this->size(2) + first.size(2) * (jdim == 2);

            matrix out(GL::GPU::dimensions{ NewX, NewY, NewZ });
            if (jdim == 0) {
                static auto func_name{ GL::string("join_dim_0") + GL::string(opencl_impl::type_name<T>()) };
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(out), mem(*this), (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, mem(first), (unsigned int)first.dim.X
                );
            }
            else if (jdim == 1) {
                static auto func_name{ GL::string("join_dim_1") + GL::string(opencl_impl::type_name<T>()) };
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(out), mem(*this), (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, mem(first), (unsigned int)first.dim.Y
                );
            }
            else {
                static auto func_name{ GL::string("join_dim_2") + GL::string(opencl_impl::type_name<T>()) };
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(out), mem(*this), (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, mem(first)
                );
            }

            return out;
        };
        template <typename T> matrix<T> matrix<T>::transpose() const {
            static auto func_name{ GL::string("Transpose") + GL::string(opencl_impl::type_name<T>()) };

            // matrix must be 2-D
            if (this->dim.num_dimensions() == 0) return matrix();
            else if (this->dim.num_dimensions() > 2) return matrix();

            matrix out(GL::GPU::dimensions{ this->dim.Y, this->dim.X, 1 });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), (unsigned int)dim.X, (unsigned int)dim.Y
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::make_square() const {
            static auto func_name{ GL::string("make_square") + GL::string(opencl_impl::type_name<T>()) };
            unsigned int len = std::max<unsigned int>(dim.X, dim.Y);
            matrix out(GL::GPU::dimensions{ len, len, 1 });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, (unsigned int)len
            );
            return out;
        }
        template <typename T> matrix<T> matrix<T>::diagonal() const {
            static auto func_name{ GL::string("diagonal") + GL::string(opencl_impl::type_name<T>()) };

            if (this->dim.num_dimensions() == 0) return matrix();
            else if (this->dim.num_dimensions() == 1) return *this;
            else if (this->dim.num_dimensions() > 2) return matrix();
            matrix out(GL::GPU::dimensions{ std::min<unsigned int>(this->dim.X, this->dim.Y), 1, 1 });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), dim.X
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::row(unsigned int rowN) const {
            static auto func_name{ GL::string("row_of") + GL::string(opencl_impl::type_name<T>()) };
            matrix out(GL::GPU::dimensions{ dim.Y, dim.Z, 1 });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), (unsigned int)rowN, (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::grow_by_wrapping(unsigned int new_length) const {
            static auto func_name{ GL::string("wrap_around") + GL::string(opencl_impl::type_name<T>()) };
            if (this->dim.num_dimensions() == 1) {
                matrix out(GL::GPU::dimensions{ new_length, 1, 1 });
                mem_matrix::queue_gpu_work(func_name,
                    out.size(),
                    mem(out), mem(*this), (unsigned int)this->size()
                );
                return out;
            }
            else {
                // ??
                throw std::runtime_error("Cannot grow a matrix by wrapping -- yet. Depends on how we want to grow it? Y-axis growth is off, but X-axis growth makes sense with wrapping");
            }
        };
        template <typename T> matrix<T> matrix<T>::resample(matrix<unsigned int> const& sample_indices) const {
            static auto func_name{ GL::string("resample") + GL::string(opencl_impl::type_name<T>()) };
            matrix out(sample_indices.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), mem(sample_indices)
            );
            return out;
        };
        template <typename T> float matrix<T>::determinant() const {
            if constexpr (!std::is_same_v<float, T>) return {};
            else {
                if (this->dim.X != this->dim.Y) {
                    return 1;
                }
                unsigned int dimension = this->dim.X;

                if (dimension == 0) {
                    return 1;
                }
                else if (dimension == 1) {
                    auto R = this->read();
                    return R(0);
                }
                else if (dimension == 2) {
                    auto R = this->read();
                    return R(0, 0) * R(1, 1) - R(0, 1) * R(1, 0);
                }
                else {
                    float result = 0;
                    int sign = 1;

                    if (auto R = this->read()) {
                        matrix subVect(dimension - 1, dimension - 1, 1, true);
                        for (unsigned int i = 0; i < dimension; ++i) {
                            if (auto W = subVect.write(true)) {
                                // build a sub-matrix
                                for (unsigned int m = 1; m < dimension; m++) {
                                    unsigned int z = 0;
                                    for (unsigned int n = 0; n < dimension; n++) {
                                        if (n != i) {
                                            W(m - 1, z) = R(m, n);
                                            z++;
                                        }
                                    }
                                }
                            }
                            //recursive call
                            result += sign * R(0, i) * subVect.determinant();
                            sign = -sign;
                        }
                    }
                    return result;
                }
            }
        }
        template <typename T> matrix<float> matrix<T>::cofactor() const {
            if constexpr (!std::is_same_v<float, T>) return {};
            else {
                if (this->dim.X != this->dim.Y) {
                    return make_square().cofactor();
                }
                unsigned int dimension = this->dim.X;
                matrix solution(GL::GPU::dimensions{ dimension, dimension, 1 });
                if (auto W1 = solution.write()) { // creates a shared space on the CPU/GPU, does the work there, and copies it over to the GPU once completed on the CPU side.
                    matrix subVect(dimension - 1, dimension - 1, 1, true);
                    if (auto R = this->read()) {
                        for (unsigned int i = 0; i < dimension; i++) {
                            for (unsigned int j = 0; j < dimension; j++) {
                                int p = 0;
                                if (auto W = subVect.write(true)) {
                                    for (unsigned int x = 0; x < dimension; x++) {
                                        if (x == i) continue;
                                        int q = 0;

                                        for (unsigned int y = 0; y < dimension; y++) {
                                            if (y == j) continue;
                                            W(p, q) = R(x, y);
                                            q++;
                                        }
                                        p++;
                                    }
                                }
                                W1(i, j) = (T)(std::pow<long double>(-1.0l, (long double)(i + j)) * (long double)subVect.determinant());
                            }
                        }
                    }
                }
                return solution;
            }
        };
        template <typename T> matrix<float> matrix<T>::adjoint() const {
            if constexpr (!std::is_same_v<float, T>) return {};
            else {
                return cofactor().transpose();
            }
        };
        template <typename T> matrix<float> matrix<T>::inverse() const {
            if constexpr (!std::is_same_v<float, T>) return {};
            else {
                return adjoint() / std::abs(determinant());
            }
        };
        template <typename T> matrix<float> matrix<T>::matrix_multiply(matrix const& rhs) const {
            if constexpr (!std::is_same_v<float, T>) return matrix<float>{};
            else {
                if (this->dim.Y == rhs.dim.X) {
                    // only useful for dim-2 matrices. 
                    unsigned int final_num_rows = this->dim.X;
                    unsigned int final_num_cols = rhs.dim.Y;

                    matrix out(GL::GPU::dimensions{ final_num_rows, final_num_cols, 1 });
                    auto R_lhs = this->read();
                    auto R_rhs = rhs.read();
                    if (auto W = out.write()) {
                        if (R_lhs && R_rhs && W) {
                            auto N = out.dim.count();
                            for (unsigned int n = 0; n < N; ++n){
                            //parallel::Std_For<unsigned int>(0, N, [&](unsigned int n) {
                                T v = (T)0;
                                const unsigned int destination_Y = (unsigned int)std::floor((long double)n / (long double)final_num_rows);
                                const unsigned int destination_X = n - (final_num_rows * destination_Y);
                                for (unsigned int index = 0; index < this->dim.Y; ++index) {
                                    v += R_lhs(destination_X, index) * R_rhs(index, destination_Y);
                                }
                                W[n] = v;
                            }//);
                        }
                    }
                    return out;
                }
                else if (this->dim.Y > rhs.dim.X) {
                    return matrix_multiply(matrix(rhs).join(0, matrix(GL::GPU::dimensions{ this->dim.Y - rhs.dim.X, rhs.dim.Y, rhs.dim.Z }) = 1));
                }
                else /*if (this->LenY < rhs.LenX)*/ {
                    // To-Do: need to set final column in joining array to 1?
                    return matrix(*this).join(1, matrix(GL::GPU::dimensions{ this->dim.X, rhs.dim.X - this->dim.Y, this->dim.Z }) = 0).matrix_multiply(rhs);
                }
            }
        };
        template <typename T> bool matrix<T>::is_colinear() const {
            if constexpr (!std::is_same_v<float, T>) return {};
            return std::abs(this->transpose().matrix_multiply(*this).determinant()) == 0;
        };
        template <typename T> T matrix<T>::sum() const {
            //static auto func_name{ GL::string("reduce_sum") + GL::string(opencl_impl::type_name<T>()) };
            //if (this->size() > 512) {
            //    matrix Temp(GL::GPU::dimensions{ (unsigned int)std::ceil((long double)(this->size()) / (long double)64), 1, 1 });
            //    mem_matrix::queue_gpu_work(func_name,
            //        this->size(),
            //        mem(*this), mem(Temp), (unsigned int)this->size()
            //    );
            //    {
            //        auto N = Temp.size();
            //        T out2 = (T)0;
            //        if (auto R = Temp.read()) {
            //            for (unsigned int n = 0; n < N; ++n) {
            //                out2 += R(n);
            //            }
            //        }
            //        return out2;
            //    }
            //}
            //else {
                auto N = this->size();
                T out = (T)0;
                if (auto R = this->read()) {
                    for (unsigned int n = 0; n < N; ++n) {
                        out += R(n);
                    }
                }
                return out;
            // }
        };
        template <typename T> T matrix<T>::avg() const {
            return (T)((long double)sum() / (long double)this->size());
        };
        template <typename T> T matrix<T>::max() const {
            //static auto func_name{ GL::string("reduce_max") + GL::string(opencl_impl::type_name<T>()) };
            //if (this->size() > 512) {
            //    matrix out(GL::GPU::dimensions{ (unsigned int)std::ceil((long double)(this->size()) / (long double)64), 1, 1 });
            //    mem_matrix::queue_gpu_work(func_name,
            //        this->size(),
            //        mem(*this), mem(out), (unsigned int)this->size(), std::numeric_limits<T>::lowest()
            //    );
            //    {
            //        auto N = out.size();
            //        T out2 = std::numeric_limits<T>::lowest();
            //        if (auto R = out.read()) {
            //            for (unsigned int n = 0; n < N; ++n) {
            //                out2 = std::max(out2, R(n));
            //            }
            //        }
            //        return out2;
            //    }
            //}
            //else {
                auto N = this->size();
                T out = std::numeric_limits<T>::lowest();
                if (auto R = this->read()) {
                    for (unsigned int n = 0; n < N; ++n) {
                        out = std::max(out, R(n));
                    }
                }
                return out;
            //}
        };
        template <typename T> T matrix<T>::min() const {
            //static auto func_name{ GL::string("reduce_min") + GL::string(opencl_impl::type_name<T>()) };
            //if (this->size() > 512) {
            //    matrix out(GL::GPU::dimensions{ (unsigned int)std::ceil((long double)(this->size()) / (long double)64), 1, 1 });
            //    mem_matrix::queue_gpu_work(func_name,
            //        this->size(),
            //        mem(*this), mem(out), (unsigned int)this->size(), std::numeric_limits<T>::max()
            //    );
            //    {
            //        auto N = out.size();
            //        T out2 = std::numeric_limits<T>::max();
            //        if (auto R = out.read()) {
            //            for (unsigned int n = 0; n < N; ++n) {
            //                out2 = std::min(out2, R(n));
            //            }
            //        }
            //        return out2;
            //    }
            //}
            //else {
                auto N = this->size();
                T out = std::numeric_limits<T>::max();
                if (auto R = this->read()) {
                    for (unsigned int n = 0; n < N; ++n) {
                        out = std::min(out, R(n));
                    }
                }
                return out;
            //}
        };
        template <typename T> matrix<T> matrix<T>::convolve(matrix_kernel<T> const& K) const {
            static auto func_name{ GL::string("convolve") + GL::string(opencl_impl::type_name<T>()) };
            if (this->dim.num_dimensions() == 2) {
                matrix out(this->dim);
                float kernel_tot = (float)K.sum;
                mem_matrix::queue_gpu_work(func_name,
                    this->size(),
                    mem(out), mem(*this), mem(*K.mat), this->size(0), this->size(1), K.mat->size(0), K.mat->size(1), kernel_tot
                );
                return out;
            }
            else {
                return *this;
            }
        };
        template <typename T> matrix<T> matrix<T>::convolve(static_matrix_kernel<T> const& K) const {
            static auto func_name{ GL::string("convolve") + GL::string(opencl_impl::type_name<T>()) };
            if (this->dim.num_dimensions() == 2) {
                matrix out(this->dim);
                float kernel_tot = (float)K.ptr->sum;
                mem_matrix::queue_gpu_work(func_name,
                    this->size(),
                    mem(out), mem(*this), static_mem_matrix{ &(*mem(*K.ptr->mat)) }, this->size(0), this->size(1), K.ptr->mat->size(0), K.ptr->mat->size(1), kernel_tot
                );
                return out;
            }
            else {
                return *this;
            }
        };
        template <typename T> matrix_kernel<float> matrix<T>::guassian_kernel(unsigned int X, unsigned int Y) {
            static auto func_name{ GL::string("guassian") + GL::string(opencl_impl::type_name<float>()) };
            matrix<float> out(X, Y, 1);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem<float>(out), out.size(0), out.size(1)
            );
            float V = out.sum();
            if (V == 0)
                out = 1.0f;
            else
                out *= 1.0f / V;
            mem<float>(out)->wait_for_events();
            return matrix_kernel<float>(std::move(out));
        };
        template <typename T> matrix<char> matrix<T>::ASCII() const {
            static auto func_name{ GL::string("ASCII") + GL::string(opencl_impl::type_name<T>()) };
            static std::vector<char> chars = []() {
                std::vector<char> chars{
                    'Q', '&', '@', '$', 'B', 'M', 'W', '8', 'h', 'k', '%', '#', '0', 'O', 'b', 'd', 'p', 'q', 'w', 'm', 'Z',
                    'C', 'U', 'X', 'I', 'a', 'o', 'z', 'c', 'f', 'Y', 'v', 'u', 'n', 'x', 'r', 'L', 'J', 'j', 't', '|',
                    '[', ']', '(', ')', '/', '\\', '1', '{', '}', 'i', 'l', '<', '>', '?', '*', '+', '~', '!',
                    '\"', '^', ';', ':', '_', '-', ',', '\'', '.', '`', ' '
                };
                std::reverse(chars.begin(), chars.end());
                return chars;
            }();
            static auto ramp{ matrix<char>::from_vector(chars) };
            mem(ramp)->wait_for_events();
            auto thisMinV = this->min();
            auto thisMaxV = this->max();

            matrix<char> out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), thisMinV, thisMaxV, static_mem_matrix{ mem(ramp).get() }, ramp.size()
            );
            return out;
            //}
        };
        template <typename T> matrix<T> matrix<T>::resize(unsigned int X, unsigned int Y, unsigned Z) const {
            static auto func_name{ GL::string("copy_resize") + GL::string(opencl_impl::type_name<T>()) };
            auto out = matrix(GL::GPU::dimensions{ X, Y, Z });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), X, Y, Z, this->size(0), this->size(1), this->size(2)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::resize_stretch(unsigned int X, unsigned int Y, unsigned Z) const {
            static auto func_name{ GL::string("copy_resize_stretch") + GL::string(opencl_impl::type_name<T>()) };
            auto out = matrix(GL::GPU::dimensions{ X, Y, Z });
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), X, Y, Z, this->size(0), this->size(1), this->size(2)
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::subsample_1D(matrix<float> const& FloatingPointIndexes) const {
            static auto func_name{ GL::string("subsample_1D") + GL::string(opencl_impl::type_name<T>()) };
            matrix out(FloatingPointIndexes.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem<T>(out), mem<T>(*this), mem<float>(FloatingPointIndexes)
            );
            return out;
        };
        template <typename T> matrix<unsigned int> matrix<T>::binomial_search_smallest_gre(matrix const& find) const {
            static auto func_name{ GL::string("binomial_search_smallest_gre") + GL::string(opencl_impl::type_name<T>()) };
            matrix<unsigned int> out(find.dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(*this), mem(find), this->dim.X
            );
            return out;
        };
        template <typename T> matrix<T> matrix<T>::subsample_pat(matrix const& X, matrix const& Y) const {
            static auto func_name{ GL::string("subsample_pat") + GL::string(opencl_impl::type_name<T>()) };
            matrix out(this->dim);
            mem_matrix::queue_gpu_work(func_name,
                out.size(),
                mem(out), mem(Y), mem(X), mem(*this), X.dim.X
            );
            return out;
        };
        template <typename T> matrix<float> matrix<T>::halfsize() const {
            static std::vector<float> kernel{
                (1.0f - (0.341f * 2.0f)) / 2.0f,
                0.341f * 2.0f,
                (1.0f - (0.341f * 2.0f)) / 2.0f
            };
            static matrix_kernel<float> kernel1(matrix<float>::from_vector(kernel, kernel.size())); // x = 3, y = 1
            static matrix_kernel<float> kernel2(matrix<float>::from_vector(kernel, 1)); // x = 1, y = 3
            if constexpr (std::is_same_v<float, T>) {
                return convolve(static_matrix_kernel<float>{ &kernel1 }).convolve(static_matrix_kernel<float>{ &kernel2 }).resize_stretch((unsigned int)std::floorf(((float)size(0) / 2.0f) + 0.5f), (unsigned int)std::floorf(((float)size(1) / 2.0f) + 0.5f), size(2));
            }
            else {
                return cast<float>().convolve(static_matrix_kernel<float>{ &kernel1 }).convolve(static_matrix_kernel<float>{ &kernel2 }).resize_stretch((unsigned int)std::floorf(((float)size(0) / 2.0f) + 0.5f), (unsigned int)std::floorf(((float)size(1) / 2.0f) + 0.5f), size(2));
            }

        };
        template <typename T> matrix<float> matrix<T>::quartersize() const {
            static std::vector<float> kernel{
                (0.136f / 2.0f),
                (0.341f / 2.0f) + (.186f / 2.0f),
                0.341f,
                (0.341f / 2.0f) + (.186f / 2.0f),
                (0.136f / 2.0f)
            };
            matrix_kernel<float> kernel1(matrix<float>::from_vector(kernel, kernel.size())); // x = 5, y = 1
            matrix_kernel<float> kernel2(matrix<float>::from_vector(kernel, 1)); // x = 1, y = 5     
            return cast<float>().convolve(kernel1).convolve(kernel2).resize_stretch(std::floorf(((float)size(0) / 4.0f) + 0.5f), std::floorf(((float)size(1) / 4.0f) + 0.5f), size(2));
        };
        template <typename T> matrix<T> matrix<T>::doublesize() const {
            return resize_stretch(size(0) * 2, size(1) * 2, size(2));
        };
        template <typename T> matrix<T> matrix<T>::quadruplesize() const {
            return resize_stretch(size(0) * 4, size(1) * 4, size(2));
        };
        template <typename T> std::string&& matrix<T>::resize(std::string&& rhs, unsigned int len, const char def) {
            if (rhs.length() != len) {
                rhs.resize(len, def);
            }
            return std::forward<std::string>(rhs);
        };
        template <typename T> std::string matrix<T>::to_string_impl(reader const& R, unsigned int x) const {
            if constexpr (std::is_same_v<char, T> || std::is_same_v<unsigned char, T>) {
                auto c = R(x);
                if ((c >= 32) && (c <= 126))
                    return std::string(1, c);
                else
                    return " ";
            }
            else {
                return std::to_string(R(x));
            }
        };
        template <typename T> std::string matrix<T>::to_string_impl(reader const& R, unsigned int x, unsigned int y) const {
            if constexpr (std::is_same_v<char, T> || std::is_same_v<unsigned char, T>) {
                auto c = R(x, y);
                if ((c >= 32) && (c <= 126))
                    return std::string(1, c);
                else
                    return " ";
            }
            else {
                return std::to_string(R(x, y));
            }
        };
        template <typename T> std::string matrix<T>::to_string_impl(reader const& R, unsigned int x, unsigned int y, unsigned int z) const {
            if constexpr (std::is_same_v<char, T> || std::is_same_v<unsigned char, T>) {
                auto c = R(x, y, z);
                if ((c >= 32) && (c <= 126))
                    return std::string(1, c);
                else
                    return " ";
            }
            else {
                return std::to_string(R(x, y, z));
            }
        };
        template <typename T> std::vector<unsigned int> matrix<T>::evaluate_column_sizes(reader const& R, std::vector<std::string> column_titles) const {
            std::vector<unsigned int> out;
            out.resize(this->dim.Y);

            for (unsigned int i = 0; i < out.size(); ++i) {
                if (i < column_titles.size())
                    out[i] = (unsigned int)column_titles[i].size();
                else
                    out[i] = 0u;
            }

            // only tests the first and last 10 rows of each column
            for (unsigned int ColN = 0; ColN < this->dim.Y; ++ColN) {
                for (unsigned int RowN = 0; RowN < this->dim.X && (RowN < 10); ++RowN) {
                    out[ColN] = std::max<unsigned int>(out[ColN], (unsigned int)to_string_impl(R, RowN, ColN).size());
                }
                if (this->dim.X > 10) {
                    for (unsigned int RowN = this->dim.X - 10; RowN < this->dim.X; ++RowN) {
                        out[ColN] = std::max<unsigned int>(out[ColN], (unsigned int)to_string_impl(R, RowN, ColN).size());
                    }
                }
            }

            return out;
        };
        template <typename T> std::string matrix<T>::to_string(std::vector<std::string> column_titles, bool doNotSkip) const {
            reader R = this->read();
            std::string column_spacer = " ";
            std::string out;

            if constexpr (std::is_same_v<char, T>) {
                out.reserve((dim.X + 1) * ((dim.Y * 2) + 1) * 2);
            }

            if (this->dim.num_dimensions() == 0) return out;
            else if (this->dim.num_dimensions() == 1) {
                auto col_sizes = evaluate_column_sizes(R, column_titles);

                unsigned int n = 0;
                for (; (n < this->size()) && (n < 1); ++n) {
                    out += resize(to_string_impl(R, n), col_sizes[0], ' ');
                }
                if (!doNotSkip && (this->size() >= 21)) {
                    for (; (n < this->size()) && (n < 10); ++n) {
                        out += "\n";
                        out += resize(to_string_impl(R, n), col_sizes[0], ' ');
                    }
                    out += "\n...";
                    for (n = this->size() - 10; n < this->size(); ++n) {
                        out += "\n";
                        out += resize(to_string_impl(R, n), col_sizes[0], ' ');
                    }
                }
                else {
                    for (; n < this->size(); ++n) {
                        out += "\n";
                        out += resize(to_string_impl(R, n), col_sizes[0], ' ');
                    }
                }
                if (column_titles.size() > 0) {
                    out = resize(std::string(column_titles[0]), col_sizes[0], ' ') + "\n" + out;
                }
            }
            else if (this->dim.num_dimensions() == 2) {
                auto col_sizes = evaluate_column_sizes(R, column_titles);

                unsigned int n = 0;
                for (; (n < this->dim.X) && (n < 1); ++n) {
                    unsigned int y = 0;
                    for (; (y < this->dim.Y) && (y < 1); ++y) {
                        out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                    }
                    for (; y < this->dim.Y; ++y) {
                        out += column_spacer;
                        out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                    }
                }
                if (!doNotSkip && (this->dim.X >= 21)) {
                    for (; (n < this->dim.X) && (n < 10); ++n) {
                        out += "\n";
                        unsigned int y = 0;
                        for (; (y < this->dim.Y) && (y < 1); ++y) {
                            out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                        }
                        for (; y < this->dim.Y; ++y) {
                            out += column_spacer;
                            out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                        }
                    }
                    out += "\n...";
                    for (n = this->dim.X - 10; n < this->dim.X; ++n) {
                        out += "\n";
                        unsigned int y = 0;
                        for (; (y < this->dim.Y) && (y < 1); ++y) {
                            out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                        }
                        for (; y < this->dim.Y; ++y) {
                            out += column_spacer;
                            out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                        }
                    }
                }
                else {
                    for (; n < this->dim.X; ++n) {
                        out += "\n";
                        unsigned int y = 0;
                        for (; (y < this->dim.Y) && (y < 1); ++y) {
                            out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                        }
                        for (; y < this->dim.Y; ++y) {
                            out += column_spacer;
                            out += resize(to_string_impl(R, n, y), col_sizes[y], ' ');
                        }
                    }
                }

                if (column_titles.size() > 0) {
                    std::string temp = column_titles[0];
                    for (unsigned int i = 1; i < column_titles.size(); ++i) {
                        temp += column_spacer;
                        temp += resize(std::string(column_titles[i]), col_sizes[i], ' ');
                    }
                    out = temp + "\n" + out;
                }
            }
            else if (this->dim.num_dimensions() == 3) {
                out = "3 dims";
            }
            return out;
        };
        template <typename U> static std::ostream& operator<<(std::ostream& os, GL::GPU::matrix<U> const& obj) {
            os << obj.to_string();
            return os;
        };
        template <typename T> std::shared_ptr<T[]> matrix<T>::slice(size_t offset, size_t length) const {
            length = std::min<size_t>(length, this->size() - offset);
            if ((offset == 0) && (length == this->size())) {
                return read().get();
            }
            else {
                static auto func_name{ GL::string("copy_slice") + GL::string(opencl_impl::type_name<T>()) };

                size_t alignment = WORKGROUP_SIZE;
                std::shared_ptr<T[]> slice;
                if (!mem_matrix::get_program().info.svm_memory_allowed) {
                    matrix<T> copier(GL::GPU::dimensions{ (unsigned int)length, 1u, 1u });
                    mem_matrix::queue_gpu_work(func_name,
                        length,
                        mem(copier), mem(*this), (unsigned int)offset
                    );
                    slice = std::shared_ptr<T[]>(new T[length], [](T* p) { delete[] p; });
                    if (auto r = copier.read()) {
                        for (int i = 0; i < length; ++i)
                            slice[i] = r[i];
                    }
                }
                else {
                    slice = std::shared_ptr<T[]>(
                        (T*)::clSVMAlloc(mem(*this)->program().get_cl_context().get(), CL_MEM_READ_WRITE,
                            sizeof(T) * (((length + (alignment - length)) / alignment) * alignment)
                            , alignment),
                        [](T* p) {
                            ::clSVMFree(mem_matrix::program().get_cl_context().get(), p);
                        }
                    );
                    mem_matrix::queue_gpu_work(func_name,
                        length,
                        slice, mem(*this), (unsigned int)offset
                    );
                    mem(*this)->events.clear();
                }
                return slice;
            }
        };
        template <typename T> T matrix<T>::operator[](unsigned int n) const {
            auto ptr = slice(n, 1);
            return ptr[0];
        };
        template <typename T> T matrix<T>::operator()(unsigned int x, unsigned int y, unsigned int z) const {
            return operator[](x + (y * dim.X) + (z * dim.Y * dim.X));
        };
        template <typename T> matrix<T> matrix<T>::linear_regressions::solve_for_weights(matrix const& measurements, matrix const& features) {
            if constexpr (!std::is_same_v<float, T>) {
                return matrix<float>::linear_regressions::solve_for_weights(measurements.cast<float>(), features.cast<float>()).cast<T>();
            }
            else {
                return (features.transpose().matrix_multiply(features)).inverse().matrix_multiply(features.transpose()).matrix_multiply(measurements);
            }
        };
        template <typename T> matrix<T> matrix<T>::linear_regressions::predict(matrix const& features, matrix const& weights) {
            if constexpr (!std::is_same_v<float, T>) {
                return features.matrix_multiply(weights).cast<T>();
            }
            else {
                return features.matrix_multiply(weights);
            }
        };
        template <typename T> matrix<T> matrix<T>::linear_regressions::standard_error(matrix const& measurements, matrix const& features, matrix const& weights) {
            if constexpr (std::is_same_v<float, T>) {
                auto prediction = predict(features, weights);
                return ((((measurements - prediction).pow(2.0f).sum() / std::max<float>(1.0f, static_cast<float>((float)features.size(0)) - 2.0f)) * (features.transpose().matrix_multiply(features)).inverse()).pow(0.5)).diagonal();
            }
            else {
                return matrix<float>::linear_regressions::standard_error(measurements.cast<float>(), features.cast<float>(), weights.cast<float>()).cast<T>();
            }
        };
        template <typename T> matrix<T> matrix<T>::linear_regressions::standard_deviation(matrix const& measurements, matrix const& features, matrix const& weights) {
            return standard_error(measurements, features, weights) * (T)std::sqrt(measurements.size(0));
        };
        template <typename T> matrix<T> matrix<T>::linear_regressions::t_statistic(matrix const& weights, matrix const& std_err) {
            return weights / std_err;
        };
        template <typename T> matrix<T> matrix<T>::linear_regressions::p_value(matrix const& features, matrix const& t_stat) {
            if constexpr (std::is_same_v<float, T>) {
                boost::math::students_t dist(features.size(0) - features.size(1)); // n - k - 1, but should include the intercept in the features list already
                matrix out(GL::GPU::dimensions{ t_stat.size(0), 1, 1 });
                unsigned int N = out.size();
                if (auto R = t_stat.read()) {
                    if (auto W = out.write()) {
                        for (unsigned int i = 0; i < N; ++i) {
                            W[i] = (1.0f - (float)boost::math::cdf(dist, R[i])) + (float)boost::math::cdf(dist, -R[i]);
                            if ((W[i] > 1.0f) || (W[i] < 0.0f))
                                W[i] = (1.0f - (float)boost::math::cdf(dist, -R[i])) + (float)boost::math::cdf(dist, R[i]);
                        }
                    }
                }
                return out;
            }
            else {
                return matrix<float>::linear_regressions::p_value(features.cast<float>(), t_stat.cast<float>()).cast<T>();
            }
        };
        template <typename T> matrix<T> matrix<T>::linear_regressions::build_features(matrix const& current_best) {
            return current_best;
        };
    };
};

// pre-compilation of all functions to be used by matrix<T>; This is necessary is we want to keep the matrix<T> implimentation seperate but still guarrantee its compilation. 
namespace GL {
    namespace GPU {
        template <typename Foo> void for_each_type(Foo const& todo) {
            todo(GL::GPU::matrix<char>());
            todo(GL::GPU::matrix<unsigned char>());
            todo(GL::GPU::matrix<int>());
            todo(GL::GPU::matrix<unsigned int>());
            todo(GL::GPU::matrix<long>());
            todo(GL::GPU::matrix<unsigned long>());
            todo(GL::GPU::matrix<float>());
        };
        template <typename T> void do_precompile(GL::GPU::matrix<T> const&) {
            // matrix<T>::maximum_allocation_size();
            matrix<T> pre1(GL::GPU::dimensions{ 1, 1, 1 });
            matrix<T> pre2(1, 1, 1, false);
            matrix<T> pre3(1, 1, 1, true);
            if (1) {
                matrix<T> a1{};
                matrix<T> a2{ a1 };
                matrix<T> a3{ std::move(a2) };
                a2 = a1;
                a2 = std::move(a3);
            }
            matrix<T>(10).size();
            matrix<T>(10).size(1);
            if (auto r = matrix<T>(10).read()) {
                r[0];
                r(0);
                r.get();
            }
            if (auto r = matrix<T>(10).write()) {
                r[0];
                r(0);
            }
            if (1) {
                matrix<T> a;
                a = T{ 0 };
                a += T{ 0 };
                a -= T{ 0 };
                a *= T{ 1 };
                a /= T{ 1 };
                a += matrix<T>(10);
                a -= matrix<T>(10);
                a *= matrix<T>(10);
                a /= matrix<T>(10);
            }
            if (1) {
                matrix<T>(10) + matrix<T>(10);
                matrix<T>(10) - matrix<T>(10);
                matrix<T>(10) * matrix<T>(10);
                matrix<T>(10) / matrix<T>(10);

                matrix<T>(10) + T{ 0 };
                matrix<T>(10) - T{ 0 };
                matrix<T>(10) * T{ 1 };
                matrix<T>(10) / T{ 1 };

                T{ 0 } + matrix<T>(10);
                T{ 0 } - matrix<T>(10);
                T{ 1 } * matrix<T>(10);
                T{ 1 } / matrix<T>(10);
            }
            for_each_type([](auto const& to) {
                matrix<T>(10).cast<typename std::decay_t<decltype(to)>::type>();
                // matrix<typename decltype(to)::type>(10).cast<T>();
                });
            matrix<T>::random(10);
            matrix<T>::random_between(T{ 0 }, T{ 1 }, 10);
            matrix<T>::identity(10);
            matrix<T>::linear(T{ 0 }, T{ 1 }, 10);
            matrix<T>::constant(T{ 0 }, 10);
            matrix<T>::from_vector({ T{0} });
            matrix<T>::from_vector({ T{0} }, 1);
            matrix<T>(10).pown(matrix<int>(10));
            matrix<T>(10).pow(matrix<T>(10));
            matrix<T>(10).pown(10);
            matrix<T>(10).pow(T{ 1 });
            matrix<T>(10).sqrt();
            matrix<T>(10).round();
            matrix<T>(10).ceil();
            matrix<T>(10).floor();
            matrix<T>(10).fma(matrix<T>(10), matrix<T>(10));
            matrix<T>(10).abs();
            matrix<T>(10).cos();
            matrix<T>(10).sin();
            matrix<T>(10).tan();
            matrix<T>(10).acos();
            matrix<T>(10).asin();
            matrix<T>(10).atan();
            matrix<T>(10).cosh();
            matrix<T>(10).sinh();
            matrix<T>(10).tanh();
            matrix<T>(10).acosh();
            matrix<T>(10).asinh();
            matrix<T>(10).atanh();
            matrix<T>(10).exp();
            matrix<T>(10).exp2();
            matrix<T>(10).exp10();
            matrix<T>(10).expm1();
            matrix<T>(10).lgamma();
            matrix<T>(10).log();
            matrix<T>(10).log2();
            matrix<T>(10).log10();
            matrix<T>(10).log1p();
            matrix<T>(10).mod(T{ 0 });
            matrix<T>(10).mod(matrix<T>(10));
            matrix<T>(10) % matrix<T>(10);
            matrix<T>(10) % T { 1 };
            matrix<T>(10).max(matrix<T>(10));
            matrix<T>(10).max(T{ 0 });
            matrix<T>(10).min(matrix<T>(10));
            matrix<T>(10).min(T{ 0 });

            !matrix<T>(10);
            matrix<T>(10) == T{ 0 };
            matrix<T>(10) != T{ 0 };
            matrix<T>(10) < T{ 0 };
            matrix<T>(10) <= T{ 0 };
            matrix<T>(10) > T{ 0 };
            matrix<T>(10) >= T{ 0 };
            matrix<T>(10) == matrix<T>(10);
            matrix<T>(10) != matrix<T>(10);
            matrix<T>(10) < matrix<T>(10);
            matrix<T>(10) <= matrix<T>(10);
            matrix<T>(10) > matrix<T>(10);
            matrix<T>(10) >= matrix<T>(10);

            matrix<T>(10) && T { 0 };
            matrix<T>(10) && matrix<T>(10);
            matrix<T>(10) || T{ 0 };
            matrix<T>(10) || matrix<T>(10);

            matrix<T>(10).join(1, matrix<T>(10));
            matrix<T>(10).transpose();
            matrix<T>(10).make_square();
            matrix<T>(10).diagonal();
            matrix<T>(10).row(0);
            matrix<T>(10).grow_by_wrapping(10);
            matrix<T>(10).resample(matrix<unsigned int>(10));
            matrix<T>(10).determinant();
            matrix<T>(10).cofactor();

            matrix<T>(10).adjoint();
            matrix<T>(10).inverse();
            matrix<T>(1, 10).matrix_multiply(matrix<T>(10, 1));
            matrix<T>(10).is_colinear();
            matrix<T>(10).sum();
            matrix<T>(10).avg();
            matrix<T>(10).max();
            matrix<T>(10).min();

            matrix<T>(10).convolve(GL::GPU::matrix_kernel<T>());
            matrix<T>(10).convolve(GL::GPU::static_matrix_kernel<T>());
            matrix<T>::guassian_kernel(3, 3);

            matrix<T>(10).ASCII();
            matrix<T>(10).resize(10, 1, 1);
            matrix<T>(10).resize_stretch(10, 1, 1);
            matrix<T>(10).subsample_1D(matrix<float>(10));
            matrix<T>(10).binomial_search_smallest_gre(matrix<T>(10));
            matrix<T>(10).subsample_pat(matrix<T>(10), matrix<T>(10));
            matrix<T>(10).halfsize();
            matrix<T>(10).quartersize();
            matrix<T>(10).doublesize();
            matrix<T>(10).quadruplesize();

            std::cout << matrix<T>(10) << std::endl;

            matrix<T>(10).slice();
            matrix<T>(10)[0];
            matrix<T>(10)(0);

            matrix<T>::linear_regressions::solve_for_weights(matrix<T>(10), matrix<T>(10));
            matrix<T>::linear_regressions::predict(matrix<T>(10), matrix<T>(10));
            matrix<T>::linear_regressions::standard_error(matrix<T>(10), matrix<T>(10), matrix<T>(10));
            matrix<T>::linear_regressions::standard_deviation(matrix<T>(10), matrix<T>(10), matrix<T>(10));
            matrix<T>::linear_regressions::t_statistic(matrix<T>(10), matrix<T>(10));
            matrix<T>::linear_regressions::p_value(matrix<T>(10), matrix<T>(10));
            matrix<T>::linear_regressions::build_features(matrix<T>(10));

        }
        void pre_compile() {
            if (GL::util::get_current_epoch() > 0) return;
            for_each_type([](auto const& with) {
                do_precompile(with);
            });
        };
    };

    void* 
        arena_memory_pool::malloc_bytes(unsigned int bytes) {
        // return Mem_Alloc(bytes); // significantly slower
        return (void*)mem_matrix::helper<char>::create(bytes / sizeof(char));
    };
    void 
        arena_memory_pool::free(void* p) {
        //Mem_Free(p); // significantly slower
        mem_matrix::helper<char>::array_delete()((char*)p);
    };
    std::string
        arena_memory_pool::debug() {
        return GL::printf("gpu(%i / %i) cpu(%i / %i)", (int)mem_matrix::gpu_allocator().size(), (int)1, (int)mem_matrix::cpu_allocator().size(), (int)mem_matrix::cpu_allocator().num_threads()).to_string();
    };
};