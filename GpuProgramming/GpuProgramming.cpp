// GpuProgramming.cpp : Defines the functions for the static library.

#include <cstdarg>
#include <type_traits>
#include <tuple>
#include <ShlDisp.h>
#include <winnt.h>
#include <thread>
#include <execution>
#include <vector>
#include <iostream>
#include <map>
#include <stdint.h>
#include <chrono>
#include <ShlDisp.h>
#include <winnt.h>
#include <string>
#include <memory>
#include <iostream>
#include <set>
#include <boost/math/distributions/students_t.hpp>
#include "dynamic_allocator.h"

#pragma region "Convenience implementation of CPU parallel computing for the conditions where GPU parallel compute is not available or not convenient."
namespace parallel {
    /// <summary>
    /// Iterator that steps through a list, without needing to instance the whole list. 
    /// </summary>
    /// <typeparam name="Type"></typeparam>
    template<typename Type = unsigned int>
    class sequence {
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

    /* parallel_for (auto i = start; i < end; i++){ todo(i); }
    If the todo(i) returns anything, it will be collected into a vector at the end. */
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
#pragma endregion

#pragma region "Includes and Defines"
#include "GpuProgramming.h"
#define CL_HPP_ENABLE_EXCEPTIONS
#include "../arrayfire/include/CL/opencl.hpp"
#include "opencl.hpp"
#include "strings.hpp"

#define print(a) std::cout << a << std::endl
#define EXPECT_EQ(a, b) if (a != b){ std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }
#define EXPECT_NE(a, b) if (a == b){ std::cout << "FAILURE AT LINE " << __LINE__ << std::endl; }
#pragma endregion

#pragma once
#pragma hdrstop


namespace GL {
    namespace clock {
        // seconds since boot
        __forceinline static long long s() {
            return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        };
        // milliseconds since boot
        __forceinline static long long ms() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        };
        // microseconds since boot
        __forceinline static long long us() {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        };
        // nanoseconds since boot
        __forceinline static long long ns() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        };
    };
    class stopwatch {
    public:
        stopwatch() : t0(clock::ns()), t1(0) {};
        // resets the timer to start "now"
        long long reset() {
            InterlockedExchange64(reinterpret_cast<volatile long long*>(&t0), clock::ns());
            return t0;
        };
        // stops the timer and returns the time passed since in seconds
        long double stop() {
            InterlockedExchange64(reinterpret_cast<volatile long long*>(&t1), clock::ns());
            return static_cast<long double>(t1 - t0) / 1000000000.0;
        };
        // does not stop the timer, but does return the time passed since in seconds
        long double check() const {
            if (t1 < t0) InterlockedExchange64(reinterpret_cast<volatile long long*>(&const_cast<stopwatch*>(this)->t1), clock::ns()); // const_cast<stopwatch*>(this)->t1 = clock::ns();
            return static_cast<long double>(t1 - t0) / 1000000000.0;
        };

        std::shared_ptr<void> debug_timer() {
            return std::static_pointer_cast<void>(std::shared_ptr<int>(new int(0), [startTime = this->reset(), this](int* p) -> void {
                auto stopTime = this->stop();
                std::string to_print = std::to_string(stopTime) + " s\n";
                std::cout << to_print;
                delete p;
            }));
        };

        template <size_t N>
        __forceinline std::shared_ptr<void> debug_timer(const char(&additional_message_content)[N]) {
            return std::static_pointer_cast<void>(std::shared_ptr<int>(new int(0), [startTime = this->reset(), this, additional_message = additional_message_content](int* p) -> void {
                auto stopTime = this->stop();
                if constexpr (N == 0) {
                    std::string to_print = std::to_string(stopTime) + " s\n";
                    std::cout << to_print;
                }
                else {
                    std::string to_print = std::string(additional_message) + ": " + std::to_string(stopTime) + " s\n";
                    std::cout << to_print;
                }
                delete p;
            }));
        };

        template<typename T>
        __forceinline std::shared_ptr<void> debug_timer(T const& additional_message_content) {
            return std::static_pointer_cast<void>(std::shared_ptr<int>(new int(0), [startTime = this->reset(), this, additional_message = std::to_string(additional_message_content)](int* p) -> void {
                auto stopTime = this->stop();
                if (additional_message.empty()) {
                    std::string to_print = std::to_string(stopTime) + " s\n";
                    std::cout << to_print;
                }
                else {
                    std::string to_print = additional_message + ": " + std::to_string(stopTime) + " s\n";
                    std::cout << to_print;
                }
                delete p;
            }));
        };

    private:
        long long t0;
        long long t1;
    };
};

#pragma region OPEN CL ARRAY
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
        kernel void row_of_type_(global _type_* destination, global _type_* LHS, uint RowN, uint LHS_LenX, uint LHS_LenY, uint LHS_LenZ) {
            const uint n = get_global_id(0);
            const uint destination_Y = (uint)floor((float)n / (float)(LHS_LenY));
            const uint destination_X = n - destination_Y * LHS_LenY;
            const uint source_Z = destination_Y;
            const uint source_Y = destination_X;
            const uint source_X = RowN;
            const uint source_n = (source_X + (LHS_LenX * source_Y) + ((LHS_LenX * LHS_LenY) * source_Z));
            destination[n] = LHS[source_n];
        };

        kernel void convolve_type_(global _type_* A, global _type_* B, global _type_* K, uint lX, uint lY, uint kX, uint kY, float kTot) {
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
                result = result * kTot / kernel_captured;
            }
            A[n] = (_type_)result;
        };

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
                    if (atom_cmpxchg(&__rand_global_counter, y, x) == y) break;
                }
                return x;
            };
            kernel void Rand_type_(global _type_ * A) {
                const uint n = get_global_id(0);
                const uint lS = get_local_id(0);

                local uint __rand_counter;
                if (get_local_id(0) == 0) {
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
            kernel void power_type_(global _type_* A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = pow(A[n], B[n]);
            };
            kernel void power_n_type_(global _type_* A, global int* B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = pown(A[n], B[n]);
            };
            kernel void square_root_type_(global _type_ * A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = native_sqrt(A[n]);
            };
            kernel void round_type_(global _type_* A, global _type_ * C) {
                const uint n = get_global_id(0);
                const _type_ H = ((_type_)1) / (_type_)2;
                C[n] = floor(A[n] + H);
            };
            kernel void flr_type_(global _type_* A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = floor(A[n]);
            };
            kernel void ceil_type_(global _type_* A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = floor(A[n] + 1);
            };
            kernel void mult_add_type_(global _type_* A, global _type_ * B, global _type_ * C, global _type_ * D) {
                const uint n = get_global_id(0);
                D[n] = fma(A[n], B[n], C[n]);
            };
            kernel void absolute_type_(global _type_* A, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fabs(A[n]);
            };
            kernel void Mod_type_(global _type_* A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmod(A[n], B[n]);
            }
            kernel void Mod_single_type_(global _type_ * A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmod(A[n], B);
            }
            kernel void Max_type_(global _type_* A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmax(A[n], B[n]);
            }
            kernel void Max_single_type_(global _type_* A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmax(A[n], B);
            }
            kernel void Min_type_(global _type_* A, global _type_ * B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmin(A[n], B[n]);
            }
            kernel void Min_single_type_(global _type_* A, _type_ B, global _type_ * C) {
                const uint n = get_global_id(0);
                C[n] = fmin(A[n], B);
            }
            kernel void reduce_max_type_(global _type_* input, global _type_* output, uint n, _type_ minV) {
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
            kernel void reduce_min_type_(global _type_* input, global _type_* output, uint n, _type_ maxV) {
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
            );
            out = out + R(

            kernel void guassian_type_(global _type_* A, uint lX, uint lY) {
                const int n = (int)get_global_id(0);
                const int Y = (int)floor((float)n / (float)lY);
                const int X = (int)n - Y * (int)lY;

                const _type_ x = ((_type_)X + 0.5) - ((_type_)lX / 2.0);
                const _type_ y = ((_type_)Y + 0.5) - ((_type_)lY / 2.0);

                A[n] = (1.0 / (2.0 * 3.141592653589793238462643383279502884197169399375105820974944)) * exp(-(x * x + y * y) / 2.0);
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
                        scratch[local_id] = max(scratch[local_id], scratch[local_id + s]);
                    }
                    barrier(CLK_LOCAL_MEM_FENCE);
                }

                // Write the work-group's partial sum to global memory
                if (local_id == 0) {
                    output[get_group_id(0)] = scratch[0];
                }
            }
            kernel void reduce_min_type_(global _type_ * input, global _type_ * output, uint n, _type_ maxV) {
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
                        scratch[local_id] = min(scratch[local_id], scratch[local_id + s]);
                    }
                    barrier(CLK_LOCAL_MEM_FENCE);
                }

                // Write the work-group's partial sum to global memory
                if (local_id == 0) {
                    output[get_group_id(0)] = scratch[0];
                }
            }
            kernel void item_AND_type_(global _type_* A, global _type_* B, global _type_* C) {
                const uint n = get_global_id(0);
                A[n] = B[n] && C[n];
            };
            kernel void item_OR_type_(global _type_* A, global _type_* B, global _type_* C) {
                const uint n = get_global_id(0);
                A[n] = B[n] || C[n];
            };
            kernel void item_AND_single_type_(global _type_* A, global _type_* B, _type_ C) {
                const uint n = get_global_id(0);
                A[n] = B[n] && C;
            };
            kernel void item_OR_single_type_(global _type_* A, global _type_* B, _type_ C) {
                const uint n = get_global_id(0);
                A[n] = B[n] || C;
            };

            );
        }
#undef R        
        return out.replace("_type_", GL::string(type_name<T>())).to_string();
    };

    static std::string create_kernels() {
        return
            create_kernel<char>() + "\n" +
            create_kernel<unsigned char>() + "\n" +
            create_kernel<int>() + "\n" +
            create_kernel<unsigned int>() + "\n" +
            create_kernel<long>() + "\n" +
            create_kernel<unsigned long>() + "\n" +
            create_kernel<float>() + "\n";
    };

    static Device& get_device() {
        static Device device(select_device_with_most_flops(), create_kernels());
        return device;
    };
};
#pragma endregion 

#include "dynamic_allocator.h"
#include "../ScriptLanguageTesting/thread_object.h"
#include "../ScriptLanguageTesting/atomic_allocator.h"
#include "../ScriptLanguageTesting/atomic_maps.h"
#include "../ScriptLanguageTesting/atomic_stack.h"
#include <concurrent_unordered_map.h>

namespace GL {
    template <typename T> class Lockable {
    private:
        T obj;
        std::mutex mut;

    public:
        template <typename... U> Lockable(const U&... args) : obj(args...), mut() {};
        Lockable(Lockable const&) = delete;
        Lockable(Lockable&&) = delete;
        Lockable& operator=(Lockable const&) = delete;
        Lockable& operator=(Lockable&&) = delete;
        ~Lockable() = default;

        class Locked {
            std::scoped_lock<std::mutex> locked;

        public:
            T& obj;

            Locked(std::mutex const& l, const T& o) : locked(const_cast<std::mutex&>(l)), obj{ const_cast<T&>(o) } {};
            Locked(Locked const&) = delete;
            Locked(Locked&&) = delete;
            Locked& operator=(Locked const&) = delete;
            Locked& operator=(Locked&&) = delete;
            ~Locked() = default;
        };
        class cLocked {
            std::scoped_lock<std::mutex> locked;

        public:
            const T& obj;

            cLocked(std::mutex const& l, const T& o) : locked(const_cast<std::mutex&>(l)), obj{ o } {};
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

    template <typename T, unsigned int minAllocCount = (sizeof(T) << 8), unsigned int padding_bytes = 0>
    class dynamic_allocator {         
        struct dynamic_block {
            dynamic_block*
                prev;
            dynamic_block*
                next;
            long long
                generated_epoch;
            unsigned int 
                num;
            unsigned int
                original_allocation; // garbage value if is_base_block() returns false
            bool 
                is_free;
            bool
                is_available;           
            char 
                Padding[padding_bytes];

            bool is_base_block() const {
                return (prev == nullptr);
            };
            dynamic_block* get_base_block() {
                if (prev) return prev->get_base_block();
                else return this;
            };
            bool is_split() const {
                if (next || prev) return true;
                else return false;
            };
        };
        cweeBTree< dynamic_block, unsigned int, 8>
            free_tree{};
        unsigned int 
            total_allocations{ 0 };
        __declspec(noinline) bool try_combine(dynamic_block* lhs) {
            if (lhs) {
                if (lhs->is_free && lhs->next) {
                    if (lhs->next->is_available && lhs->next->is_free) {
                        // we are free, and the next pointer is free
                        lhs->next->is_available = false;
                        lhs->num += (sizeof(dynamic_block) + (lhs->next->num * sizeof(T))) / sizeof(T);
                        if (lhs->next->next) {
                            lhs->next->next->prev = lhs;
                        }

                        // lhs->next is no longer valid and should be removed from the list
                        auto tree_node = free_tree.NodeFindSmallestLargerEqual(lhs->next->num);
                        while (tree_node) {
                            if (tree_node->object) {
                                if (tree_node->key == lhs->next->num) {
                                    if (tree_node->object == lhs->next) {
                                        free_tree.Remove(tree_node);
                                        break;
                                    }
                                }
                            }
                            tree_node = free_tree.GetNextLeaf(tree_node);
                        }

                        lhs->next = lhs->next->next;

                        if (!lhs->next && !lhs->prev) {
                            lhs->num = lhs->original_allocation;
                        }

                        // try to combine again!
                        (void)try_combine(lhs);

                        return true;
                    }
                }
                //if (lhs->is_free && lhs->prev) {
                //    if (lhs->prev->is_available && lhs->prev->is_free) {
                //        // we are free, and the prev pointer is free
                //        return try_combine(lhs->prev);
                //    }
                //}
            }
            return false;
        };
        std::mutex 
            mut;

    public:
        T* Alloc(unsigned int N) {
            if (N == 0) return nullptr;

            std::scoped_lock locked(mut);

            dynamic_block* free_block = nullptr;
            // try to get a free block
            if (1) {
                auto* tree_node = free_tree.NodeFindSmallestLargerEqual(N);
                while (tree_node) {
                    if (tree_node->object) {
                        if (tree_node->key >= N) {
                            if (tree_node->object->is_available) {
                                free_block = tree_node->object;
                                free_block->get_base_block()->generated_epoch = GL::util::get_current_epoch();
                                free_tree.Remove(tree_node);
                                break;
                            }
                        }
                    }
                    tree_node = free_tree.GetNextLeaf(tree_node);
                }                
            }
            // otherwise make a block
            if (!free_block) { // allocate a new buffer to fit the requested size
                unsigned int alloc_count = CONST_MAX(minAllocCount, ((N > minAllocCount) ? (N + (N % minAllocCount)) : N) );
                free_block = (dynamic_block*)Mem_Alloc(sizeof(dynamic_block) + sizeof(T) * alloc_count);
                if (!free_block) return nullptr;
                free_block->prev = nullptr;
                free_block->next = nullptr;
                free_block->num = alloc_count;
                free_block->original_allocation = alloc_count;
                free_block->is_available = true;
                free_block->generated_epoch = GL::util::get_current_epoch();
                total_allocations += free_block->original_allocation;
            }
            // split the block if too large
            if (free_block && (free_block->num > N)) {
                // we got a buffer of size enough. But, if it is too large, we can share it with another, smaller allocation later.
                long long free_block_size = sizeof(T) * (free_block->num - N);
                long long remaining_N = ((free_block_size - (long long)sizeof(dynamic_block)) / (long long)sizeof(T));
                if (remaining_N > 0) {
                    dynamic_block* child_block = (dynamic_block*)(((::byte*)free_block) + sizeof(dynamic_block) + sizeof(T) * N);
                    child_block->prev = free_block;
                    child_block->next = free_block->next;
                    child_block->num = remaining_N;
                    child_block->is_free = true;
                    child_block->is_available = true;
                    free_block->next = child_block;
                    if (child_block->next) child_block->next->prev = child_block;
                    free_block->num = N;

                    free_tree.Add(child_block, child_block->num);
                }
            }
            // return the result
            free_block->is_free = false;
            return (T*)(((::byte*)free_block) + sizeof(dynamic_block));
        };
        __declspec(noinline) void Free(T* ptr) {
            if (!ptr) return;

            std::scoped_lock locked(mut);

            dynamic_block* free_block = (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));
            free_block->is_free = true;
            try_combine(free_block);
            if (free_block->is_base_block() && !free_block->is_split() && ((total_allocations - free_block->original_allocation) > 0)) {
                long long curr_epoch = GL::util::get_current_epoch();

                // has enough time passed to warrant this?
                if ((curr_epoch - free_block->generated_epoch) > 1000) {
                    total_allocations -= free_block->original_allocation;
                    Mem_Free(free_block);
                }
                else {
                    free_block->generated_epoch = curr_epoch;
                    free_tree.Add(free_block, free_block->num);
                }

                // review the free tree and see if anyone is expired...
                if (auto* tree_node = free_tree.GetRoot()) {
                    while (tree_node) {
                        if (tree_node->object) {
                            if (try_combine(tree_node->object)) break;
                            if (!tree_node->object->is_split() && tree_node->object->is_base_block()) {
                                if ((curr_epoch - tree_node->object->generated_epoch) > 10) {
                                    total_allocations -= tree_node->object->original_allocation;
                                    Mem_Free(tree_node->object);

                                    free_tree.Remove(tree_node);
                                    break;
                                }
                            }                        
                        }
                        tree_node = free_tree.GetNextLeaf(tree_node);
                    }
                }
            }
            else {
                free_tree.Add(free_block, free_block->num);
            }
        };
        __declspec(noinline) ~dynamic_allocator() {
            std::vector< dynamic_block* > blocks;
            if (auto* tree_node = free_tree.GetRoot()) {
                while (tree_node) {
                    if (tree_node->object) {
                        if (tree_node->object->is_base_block()) {
                            blocks.push_back(tree_node->object);
                        }
                    }
                    tree_node = free_tree.GetNextLeaf(tree_node);
                }
            }
            for (auto& free_block : blocks) {
                total_allocations -= free_block->original_allocation;
                Mem_Free(free_block);
            }
            EXPECT_EQ(total_allocations, 0);
        };
        static char* get_padded_content(T* ptr) {
            if (!ptr) return nullptr;
            dynamic_block* free_block = (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));
            return &free_block->Padding[0];
        };

        class unique_ptr {
            T* data;
            dynamic_allocator* parent;

        public:
            explicit unique_ptr(T* d, dynamic_allocator* p) : data{ d }, parent{ p } {};
            unique_ptr() : data{ nullptr }, parent{ nullptr } {};
            unique_ptr(unique_ptr const&) = delete;
            unique_ptr(unique_ptr && rhs) noexcept : data{ rhs.data }, parent{ rhs.parent } {
                rhs.data = nullptr;
                rhs.parent = nullptr;
            };
            unique_ptr& operator=(unique_ptr const&) = delete;
            unique_ptr& operator=(unique_ptr&& rhs) noexcept {
                if (data && parent) { parent->Free(data); }
                data = rhs.data;
                parent = rhs.parent;
                rhs.data = nullptr;
                rhs.parent = nullptr;
            };
            ~unique_ptr() {
                if (data && parent) { parent->Free(data); }
            };

            const T* operator->() const {
                return data;
            };
            T* operator->() {
                return data;
            };
            const T& operator*() const {
                return data;
            };
            T& operator*() {
                return data;
            };
            const T* get() const {
                return data;
            };
            T* get() {
                return data;
            };

            T& operator[](unsigned int N) {
                return data[N];
            };
            const T& operator[](unsigned int N) const {
                return data[N];
            };

        };

        unique_ptr make_unique(unsigned int N) {
            return unique_ptr(this->Alloc(N), this);
        };
        std::shared_ptr<T[]> make_shared(unsigned int N) {
            return std::shared_ptr<T[]>(this->Alloc(N), [this](T* p) {
                this->Free(p);
            });
        };

    };

    // Thread-safe, lock-free, high-performance page-based allocator with LIFO functionality for memory re-use. Optimized for heavy multithreading. 
    template <typename _type_, size_t minAllocCount = (sizeof(_type_) << 8)>
    class parallel_dynamic_allocator {
    private:
        thread_object_no_default<dynamic_allocator<_type_, minAllocCount, sizeof(size_t) / sizeof(char)>>
            TLS;

    public:
        parallel_dynamic_allocator() = default;
        ~parallel_dynamic_allocator() = default;

        _type_* Alloc(unsigned int N) {
            const auto threadID = GL::util::get_thread_id();
            _type_* out = TLS->Alloc(N);
            auto& thread_id = reinterpret_cast<size_t&>(*dynamic_allocator<_type_, minAllocCount, sizeof(size_t) / sizeof(char)>::get_padded_content(out));
            thread_id = threadID;
            return out;
        };
        __declspec(noinline) void Free(_type_* t) {
            if (t) {
                auto& thread_id = reinterpret_cast<size_t&>(*dynamic_allocator<_type_, minAllocCount, sizeof(size_t) / sizeof(char)>::get_padded_content(t));
                TLS[thread_id].Free(t);
            }
        };

    };
};
namespace GL {
    namespace GPU {
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
                    : cl_program(info.cl_context, make_kernel_code(info, combine(opencl_code)))
                {
                    const std::string build_options
                        = "-cl-std=CL" + info.opencl_c_version + " -cl-finite-math-only -cl-no-signed-zeros -cl-mad-enable" + (info.patch_intel_gpu_above_4gb ? " -cl-intel-greater-than-4GB-buffer-required" : "");
                    int error
                        = cl_program.get().obj.build(info.cl_device, (build_options + " -w").c_str());
                    if (error)
                        print_warning(cl_program.get().obj.getBuildInfo<CL_PROGRAM_BUILD_LOG>(info.cl_device)); // print build log

                    initialized = true;
                }
                ProgramImpl() = default;
                ProgramImpl(ProgramImpl const&) = delete;
                ProgramImpl(ProgramImpl&&) = delete;
                ProgramImpl& operator=(ProgramImpl const&) = delete;
                ProgramImpl& operator=(ProgramImpl&&) = delete;
                ~ProgramImpl() = default;

                void try_initialize(Device_Info const& info, std::vector<std::string> const& opencl_code) {
                    if (!initialized) {
                        cl_program.get().obj = cl::Program(info.cl_context, make_kernel_code(info, combine(opencl_code)));
                        const std::string build_options
                            = "-cl-std=CL" + info.opencl_c_version + " -cl-finite-math-only -cl-no-signed-zeros -cl-mad-enable" + (info.patch_intel_gpu_above_4gb ? " -cl-intel-greater-than-4GB-buffer-required" : "");
                        int error
                            = cl_program.get().obj.build(info.cl_device, (build_options + " -w").c_str());
                        if (error)
                            print_warning(cl_program.get().obj.getBuildInfo<CL_PROGRAM_BUILD_LOG>(info.cl_device)); // print build log

                        initialized = true;
                    }
                }

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
            bool initialized = false;

            Program(std::vector<std::string> const& opencl_c_code)
                : info(select_device_with_most_flops(get_devices(false)))
                , program(info, opencl_c_code)
                , queue(info.cl_context, info.cl_device)
                , initialized(true)
            {}
            Program()
                : info(select_device_with_most_flops(get_devices(false)))
                , program()
                , queue()
                , initialized(false)
            {}
            Program(Program const&) = delete;
            Program(Program&&) = delete;
            Program& operator=(Program const&) = delete;
            Program& operator=(Program&&) = delete;
            ~Program() = default;

            template <typename F>
            void try_initialize(F const& opencl_c_code) {
                if (!initialized) {
                    program.try_initialize(info, opencl_c_code());
                    queue.get().obj = cl::CommandQueue(info.cl_context, info.cl_device);
                    initialized = true;
                }
            };


            //inline void barrier(const std::pair<Event*, Event*> event_waitlist = { nullptr, nullptr }, Event* event_returned = nullptr) { cl_queue.enqueueBarrierWithWaitList(event_waitlist, event_returned); }
            //inline void finish_queue() { cl_queue.finish(); }
            inline cl::Context get_cl_context() const { return info.cl_context; }
            //inline cl::Program get_cl_program() const { return cl_program; }
            //inline cl::CommandQueue get_cl_queue() const { return cl_queue; }

        };
        // static parallel_dynamic_allocator<char, 8 << 12, 8 << 2> shared_mem_allocator;
        static parallel_dynamic_allocator<char> shared_mem_allocator;

        class opencl {
        public:
            static GL::GPU::Program& get_program() {
                static GL::GPU::Program out({ opencl_impl::create_kernels() });
                return out;
            };
        };

        template<typename T> class Memory {
        private:
            ulong N = 0ull; // buffer length
            uint d = 1u; // buffer dimensions
            bool host_buffer_exists = false;
            bool device_buffer_exists = false;
            bool external_host_buffer = false; // Memory object has been created with an externally supplied host buffer/pointer
            bool is_zero_copy = false; // if possible (device is CPU or iGPU), and if allowed by user, use zero-copy buffer: host+device buffers are fused into one
            T* host_buffer = nullptr; // host buffer
            std::unique_ptr<T[]> host_buffer_unaligned = nullptr; // unaligned host buffer (only required for zero-copy to align host_buffer)
            cl::Buffer device_buffer; // device buffer
            Program* device = nullptr; // pointer to linked Program            

        private:
            void initialize_auxiliary_pointers() {
                /********/ x = s0 = host_buffer; /******/ if (d > 0x4u) s4 = host_buffer + N * 0x4ull; if (d > 0x8u) s8 = host_buffer + N * 0x8ull; if (d > 0xCu) sC = host_buffer + N * 0xCull;
                if (d > 0x1u) y = s1 = host_buffer + N; /****/ if (d > 0x5u) s5 = host_buffer + N * 0x5ull; if (d > 0x9u) s9 = host_buffer + N * 0x9ull; if (d > 0xDu) sD = host_buffer + N * 0xDull;
                if (d > 0x2u) z = s2 = host_buffer + N * 0x2ull; if (d > 0x6u) s6 = host_buffer + N * 0x6ull; if (d > 0xAu) sA = host_buffer + N * 0xAull; if (d > 0xEu) sE = host_buffer + N * 0xEull;
                if (d > 0x3u) w = s3 = host_buffer + N * 0x3ull; if (d > 0x7u) s7 = host_buffer + N * 0x7ull; if (d > 0xBu) sB = host_buffer + N * 0xBull; if (d > 0xFu) sF = host_buffer + N * 0xFull;
            }
            inline void allocate_host_buffer(const bool allocate_host, const bool allow_zero_copy) {
                if (allocate_host) {
                    const ulong alignment = allow_zero_copy && device->info.uses_ram ? 4096ull : 64ull; // host_buffer must be aligned to 4096 Bytes for CL_MEM_USE_HOST_PTR, and to 64 Bytes for optimal enqueueReadBuffer performance on modern CPUs
                    const ulong padding = allow_zero_copy && device->info.uses_ram ? 64ull : 0ull; // for CL_MEM_USE_HOST_PTR, 64 Bytes padding is required because device_buffer capacity in this case must be a multiple of 64 Bytes
                    const ulong alloc_size = N * (ulong)d + (alignment + padding) / sizeof(T);
                    const ulong alloc_char_size = alloc_size * sizeof(T) / sizeof(char);
                    host_buffer_unaligned = std::make_unique<T[]>(alloc_size);
                    //host_buffer_unaligned = (T*)shared_mem_allocator.Alloc(alloc_char_size);
                    host_buffer = (T*)((((ulong)host_buffer_unaligned.get() + alignment - 1ull) / alignment) * alignment); // align host_buffer by fine-tuning pointer to be a multiple of alignment
                    initialize_auxiliary_pointers();
                    host_buffer_exists = true;
                }
            }
            inline void allocate_device_buffer(const bool allocate_device, const bool allow_zero_copy) {
                if (allocate_device) {
                    device->info.memory_used += (uint)(capacity() / 1048576ull); // track device memory usage
                    if (device->info.memory_used > device->info.memory) print_error("Program \"" + device->info.name + "\" does not have enough memory. Allocating another " + to_string((uint)(capacity() / 1048576ull)) + " MB would use a total of " + to_string(device->info.memory_used) + " MB / " + to_string(device->info.memory) + " MB.");
                    int error = 0;
                    is_zero_copy = allow_zero_copy && host_buffer_exists && device->info.uses_ram && (!external_host_buffer || ((ulong)host_buffer % 4096ull == 0ull && capacity() % 64ull == 0ull));
                    device_buffer = cl::Buffer( // if(is_zero_copy) { don't allocate extra memory on CPUs/iGPUs } else { allocate VRAM on GPUs }
                        device->get_cl_context(),
                        CL_MEM_READ_WRITE | ((int)is_zero_copy * CL_MEM_USE_HOST_PTR) | ((int)device->info.patch_intel_gpu_above_4gb << 23), // for Intel GPUs set flag CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL = (1<<23)
                        is_zero_copy ? ((capacity() + 63ull) / 64ull) * 64ull : capacity(), // device_buffer capacity must be a multiple of 64 Bytes for CL_MEM_USE_HOST_PTR
                        is_zero_copy ? (void*)host_buffer : nullptr,
                        &error
                    );
                    if (error == -61) print_error("Memory size is too large at " + to_string((uint)(capacity() / 1048576ull)) + " MB. Program \"" + device->info.name + "\" accepts a maximum buffer size of " + to_string(device->info.max_global_buffer) + " MB.");
                    else if (error) print_error("Program buffer allocation failed with error code " + to_string(error) + ".");
                    device_buffer_exists = true;
                }
            }
        public:
            std::vector<cl::Event> jobs_that_reference_me;

            T* x = nullptr, * y = nullptr, * z = nullptr, * w = nullptr; // host buffer auxiliary pointers for multi-dimensional array access (array of structures)
            T* s0 = nullptr, * s1 = nullptr, * s2 = nullptr, * s3 = nullptr, * s4 = nullptr, * s5 = nullptr, * s6 = nullptr, * s7 = nullptr, * s8 = nullptr, * s9 = nullptr, * sA = nullptr, * sB = nullptr, * sC = nullptr, * sD = nullptr, * sE = nullptr, * sF = nullptr;
            Memory(const ulong N, const uint dimensions = 1u, const bool allocate_host = true, const bool allocate_device = true, const T value = (T)0, const bool allow_zero_copy = true)
                : device(&opencl::get_program())
            {
                if (N * (ulong)dimensions == 0ull) print_error("Memory size must be larger than 0.");
                this->N = N;
                this->d = dimensions;
                allocate_host_buffer(allocate_host, allow_zero_copy); // allocate host_buffer first
                allocate_device_buffer(allocate_device, allow_zero_copy); // allocate device_buffer second
                reset(value);
            }
            Memory(const ulong N, const uint dimensions, T* const host_buffer, const bool allocate_device = true, const bool allow_zero_copy = true)
                : device(&opencl::get_program())
            {
                if (N * (ulong)dimensions == 0ull) print_error("Memory size must be larger than 0.");
                this->N = N;
                this->d = dimensions;
                this->host_buffer = host_buffer;
                initialize_auxiliary_pointers();
                host_buffer_exists = true;
                external_host_buffer = true;
                allocate_device_buffer(allocate_device, allow_zero_copy);
                write_to_device();
            }
            Memory() {} // default constructor
            Memory(Memory const&) = delete;
            Memory(Memory&&) = delete;
            Memory& operator=(Memory const&) = delete;
            Memory& operator=(Memory&&) = delete;
            ~Memory() {
                if (jobs_that_reference_me.size() > 0) {
                    cl::Event::waitForEvents({ &jobs_that_reference_me[0], &jobs_that_reference_me[0] + jobs_that_reference_me.size() });
                }
                delete_buffers();
            };

            inline void add_host_buffer() { // makes only sense if there is no host buffer yet but an existing device buffer
                if (!host_buffer_exists && device_buffer_exists) {
                    const ulong alloc_char_size = N * (ulong)d * sizeof(T) / sizeof(char);
                    host_buffer_unaligned = std::make_unique<T[]>(N * (ulong)d); //  (T*)shared_mem_allocator.Alloc(alloc_char_size); // 
                    host_buffer = host_buffer_unaligned.get();
                    initialize_auxiliary_pointers();
                    read_from_device();
                    host_buffer_exists = true;
                }
            }
            inline void add_device_buffer(const bool allow_zero_copy = true) { // makes only sense if there is no device buffer yet but an existing host buffer
                if (!device_buffer_exists && host_buffer_exists) {
                    allocate_device_buffer(true, allow_zero_copy);
                    write_to_device();
                }
            }
            inline void delete_host_buffer() {
                host_buffer_exists = false;
                if (!external_host_buffer) {
                    host_buffer = nullptr;
                    host_buffer_unaligned = nullptr;
                    // shared_mem_allocator.Free((char*)host_buffer_unaligned);
                }
                if (!device_buffer_exists) {
                    N = 0ull;
                    d = 1u;
                }
            }
            inline void delete_device_buffer() {
                if (device_buffer_exists) device->info.memory_used -= (uint)(capacity() / 1048576ull); // track device memory usage
                device_buffer_exists = false;
                device_buffer = nullptr;
                if (!host_buffer_exists) {
                    N = 0ull;
                    d = 1u;
                }
            }
            inline void delete_buffers() {
                delete_device_buffer();
                delete_host_buffer();
            }
            inline void reset(const T value = (T)0) {
                if (host_buffer_exists) std::fill(host_buffer, host_buffer + range(), value); // faster than "for(ulong i=0ull; i<range(); i++) host_buffer[i] = value;"
                write_to_device(); // enqueueFillBuffer is broken for large buffers on Nvidia GPUs!
            }
            inline const ulong length() const { return N; }
            inline const uint dimensions() const { return d; }
            inline const ulong range() const { return N * (ulong)d; }
            inline const ulong capacity() const { return N * (ulong)d * sizeof(T); } // returns capacity of the buffer in Bytes
            inline T* const data() { return host_buffer; }
            inline const T* const data() const { return host_buffer; }
            inline T* const operator()() { return host_buffer; }
            inline const T* const operator()() const { return host_buffer; }
            inline T& operator[](const ulong i) { return host_buffer[i]; }
            inline const T& operator[](const ulong i) const { return host_buffer[i]; }
            inline const T operator()(const ulong i) const { return host_buffer[i]; }
            inline const T operator()(const ulong i, const uint dimension) const { return host_buffer[i + (ulong)dimension * N]; } // array of structures

            void wait() {
                if (jobs_that_reference_me.size() > 0) {
                    cl::Event::waitForEvents(std::pair<Event*, Event*>{ &jobs_that_reference_me[0], & jobs_that_reference_me[0] + jobs_that_reference_me.size() });
                    jobs_that_reference_me.clear();
                }
            };
            inline void read_from_device() {
                if (host_buffer_exists && device_buffer_exists && !is_zero_copy) {
                    cl::Event event_returned;
                    device->queue.get().obj.enqueueReadBuffer(device_buffer, false, 0ull, capacity(), (void*)host_buffer,
                        jobs_that_reference_me.size() > 0 ? std::pair<Event*, Event*>{ &jobs_that_reference_me[0], & jobs_that_reference_me[0] + jobs_that_reference_me.size() } : std::pair<Event*, Event*>{ nullptr, nullptr }
                    , & event_returned);
                    jobs_that_reference_me.push_back(event_returned);
                }
            };
            inline void write_to_device() {
                if (host_buffer_exists && device_buffer_exists && !is_zero_copy) {
                    cl::Event event_returned;
                    device->queue.get().obj.enqueueWriteBuffer(device_buffer, false, 0ull, capacity(), (void*)host_buffer,
                        jobs_that_reference_me.size() > 0 ? std::pair<Event*, Event*>{ &jobs_that_reference_me[0], & jobs_that_reference_me[0] + jobs_that_reference_me.size() } : std::pair<Event*, Event*>{ nullptr, nullptr }
                    , & event_returned);
                    jobs_that_reference_me.push_back(event_returned);
                }
            };
            inline const cl::Buffer& get_cl_buffer() const {
                return device_buffer;
            };
        };

        class Function {
        public:
            std::string
                name = "";
            const Program*
                cl_program = nullptr;
            cl::Kernel
                cl_kernel;

        public:
            Function()
                : name(""), cl_program(nullptr), cl_kernel()
            {}
            Function(const std::string& Name)
                : name(Name), cl_program(&opencl::get_program()), cl_kernel(cl_program->program.cl_program.get().obj, name.c_str())
            {}
            Function(Function&&) = default;
            Function& operator=(Function&&) = default;
            Function(Function const&) = delete;
            Function& operator=(Function const&) = delete;
            ~Function() = default;


        private:
            void check_for_errors(const int error) const {
                if (error == -48) print_error("There is no OpenCL kernel with name \"" + name + "(...)\" in the OpenCL C code! Check spelling!");
                if (error<-48 && error>-53) print_error("Parameters for OpenCL kernel \"" + name + "(...)\" don't match between C++ and OpenCL C!");
                if (error == -54) print_error("Workgrop size " + to_string(WORKGROUP_SIZE) + " for OpenCL kernel \"" + name + "(...)\" is invalid!");
                if (error != 0) print_error("OpenCL kernel \"" + name + "(...)\" failed with error code " + to_string(error) + "!");
            }
            template<typename T> void link_parameter(const uint position, const T& constant) {
                check_for_errors(cl_kernel.setArg(position, sizeof(T), (void*)&constant));
            }
            template<> void link_parameter<cl::Buffer>(const uint position, const cl::Buffer& memory) {
                check_for_errors(cl_kernel.setArg(position, memory));
            }
            void link_parameters(unsigned int& number_of_parameters, std::set<cl::Event>& waitlist, const uint starting_position) {
                number_of_parameters = max(number_of_parameters, starting_position);
            }
            template<template<class> typename G, typename T, class... U> void link_parameters(unsigned int& number_of_parameters, std::set<cl::Event>& waitlist, const uint starting_position, const G<T>& parameter, const U&... parameters) {
                if constexpr (std::is_same_v<G<T>, std::shared_ptr<T>>) {
                    link_parameters(number_of_parameters, waitlist, starting_position, *parameter, parameters...);
                }
                else if constexpr (std::is_same_v<G<T>, GL::GPU::Memory<T>>) {
                    waitlist.insert(parameter.jobs_that_reference_me.begin(), parameter.jobs_that_reference_me.end());

                    link_parameter(starting_position, parameter.get_cl_buffer());
                    link_parameters(number_of_parameters, waitlist, starting_position + 1u, parameters...);
                }
                else {
                    link_parameter(starting_position, parameter);
                    link_parameters(number_of_parameters, waitlist, starting_position + 1u, parameters...);
                }
            }
            template<class T, class... U> void link_parameters(unsigned int& number_of_parameters, std::set<cl::Event>& waitlist, const uint starting_position, const T& parameter, const U&... parameters) {
                link_parameter(starting_position, parameter);
                link_parameters(number_of_parameters, waitlist, starting_position + 1u, parameters...);
            }

        public:
            static void append_events(cl::Event const& _event) {}
            template<template<class> typename G, typename T, class... U> static void append_events(cl::Event const& _event, const G<T>& parameter, const U&... parameters) {
                if constexpr (std::is_same_v<G<T>, std::shared_ptr<T>>) {
                    append_events(_event, *parameter, parameters...);
                }
                else if constexpr (std::is_same_v<G<T>, GL::GPU::Memory<T>>) {
                    const_cast<G<T>&>(parameter).jobs_that_reference_me.push_back(_event);
                    append_events(_event, parameters...);
                }
                else {
                    append_events(_event, parameters...);
                }
            }
            template<class T, class... U> static void append_events(cl::Event const& _event, const T& parameter, const U&... parameters) {
                append_events(_event, parameters...);
            }

        public:
            template<class... T> cl::Event operator()(unsigned int N, const T&... parameters) {
                unsigned int
                    number_of_parameters = 0u;
                std::set<cl::Event>
                    waitlist_set;
                std::vector<cl::Event>
                    waitlist;

                link_parameters(number_of_parameters, waitlist_set, 0u, parameters...); // expand variadic template to link kernel parameters
                waitlist.insert(waitlist.end(), waitlist_set.begin(), waitlist_set.end());

                cl::NDRange
                    cl_range_global = cl::NDRange(((N + WORKGROUP_SIZE - 1ull) / WORKGROUP_SIZE) * WORKGROUP_SIZE),
                    cl_range_local = cl::NDRange(WORKGROUP_SIZE);

                cl::Event out;
                check_for_errors(cl_program->queue.get().obj.enqueueNDRangeKernel(cl_kernel, cl::NullRange, cl_range_global, cl_range_local, waitlist.size() > 0 ? std::pair<Event*, Event*>{ &waitlist[0], & waitlist[0] + waitlist.size() } : std::pair<Event*, Event*>{ nullptr, nullptr }, & out));
                append_events(out, parameters...);
                return out;
            };



        };

        struct dimensions {
            unsigned int X;
            unsigned int Y;
            unsigned int Z;
            unsigned int num_dimensions() const {
                return std::max<unsigned int>(1u, (unsigned int)(X > 1u) + (unsigned int)(Y > 1u) + (unsigned int)(Z > 1u));
            };
            unsigned int count() const {
                return X * Y * Z;
            };
        };

        template<typename T> class gpu_array {
            template <typename G> friend class gpu_array;
        public:
            std::shared_ptr<Memory<T>> data;
            dimensions dim;

        public:
            using type = T;
            class reader {
                Memory<T>* data;
                dimensions dim;

            public:
                reader(Memory<T>& copy, dimensions const& D) : data(&copy), dim(D) {
                    data->add_host_buffer();
                    data->read_from_device();
                    data->wait();
                };
                reader(reader const&) = default;
                reader(reader&&) = default;
                reader& operator=(reader const&) = default;
                reader& operator=(reader&&) = default;
                ~reader() = default;
                operator bool() const {
                    return data->length() > 0;
                };
                T const& operator[](unsigned int X) const {
                    return data->operator[](X);
                };
                T const& operator()(unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const {
                    return data->operator[]((Z* dim.X* dim.Y) + (Y * dim.X) + X);
                };
            };
            class writer {
                Memory<T>* data;
                dimensions dim;
                bool _cpu_only = false;

            public:
                writer(Memory<T>& copy, dimensions const& D, bool cpu_only = false) : data(&copy), dim(D), _cpu_only(cpu_only) {
                    data->add_host_buffer();
                    data->read_from_device();
                    data->wait();
                };
                writer(writer const&) = delete;
                writer(writer&& rhs) : data(std::move(rhs.data)), dim(std::move(rhs.dim)), _cpu_only(rhs._cpu_only) {
                    rhs.data = nullptr;
                };
                writer& operator=(writer const&) = delete;
                writer& operator=(writer&& rhs) {
                    if (data) { data->write_to_device(); }

                    data = std::move(rhs.data);
                    dim = std::move(rhs.dim);
                    rhs.data = nullptr;

                    return *this;
                };
                ~writer() {
                    if (data) {
                        data->write_to_device();
                    }
                };
                operator bool() const {
                    return data->length() > 0;
                };
                T& operator[](unsigned int X) const {
                    return data->operator[](X);
                };
                T& operator()(unsigned int X, unsigned int Y = 0, unsigned int Z = 0) const {
                    return data->operator[]((Z* dim.X* dim.Y) + (Y * dim.X) + X);
                };
            };

        public:
            template<class... P> static inline void work(gpu_array& destination, unsigned int count, const std::string& name, const P&... parameters) {
                GL::GPU::Function kernel(name + opencl_impl::type_name<T>());
                if (destination.data) {
                    destination.data->jobs_that_reference_me.push_back(kernel(count, parameters...));
                }
                else {
                    GL::GPU::Function::append_events(kernel(count, parameters...), parameters...);
                }
            };

        public:
            gpu_array() : data(nullptr), dim{ 0,0,0 } {}
            explicit gpu_array(dimensions const& D, bool cpu_only = false)
                : data(std::make_shared<Memory<T>>(D.count(), 1, cpu_only, !cpu_only)), dim{ D }
            {};
            explicit gpu_array(unsigned int X, unsigned int Y = 1, unsigned int Z = 1)
                : data(std::make_shared<Memory<T>>(X* Y* Z, 1, false, true)), dim{ X, Y, Z }
            {}
            gpu_array(gpu_array const&) = delete;
            gpu_array(gpu_array&& rhs) noexcept : data(std::move(rhs.data)), dim{ rhs.dim }
            {
                rhs.data = nullptr;
            }
            gpu_array& operator=(gpu_array const&) = delete;
            gpu_array& operator=(gpu_array&& rhs) noexcept {
                dim = rhs.dim;
                data = std::move(rhs.data);
                rhs.data = nullptr;
                return *this;
            };
            ~gpu_array() {
                data = nullptr;
            };


        public:
            reader read() const {
                return reader(*data, dim);
            };
            writer write(bool cpu_only = false) {
                return writer(*data, dim, cpu_only);
            };
            unsigned int size() const {
                return dim.count();
            }
            unsigned int size(unsigned int D) const {
                if (D == 0) return dim.X;
                if (D == 1) return dim.Y;
                if (D == 2) return dim.Z;
                else throw std::runtime_error("Array does not support more than 3 dimensions yet");
            }

            gpu_array& operator=(T rhs) {
                gpu_array::work(*this, this->size(), "copy_single", data, rhs);
                return *this;
            };
            gpu_array& operator+=(T rhs) {
                gpu_array::work(*this, this->size(), "add_single_inplace", data, rhs);
                return *this;
            };
            gpu_array& operator-=(T rhs) {
                gpu_array::work(*this, this->size(), "sub_single_inplace", data, rhs);
                return *this;
            };
            gpu_array& operator*=(T rhs) {
                gpu_array::work(*this, this->size(), "mult_single_inplace", data, rhs);
                return *this;
            };
            gpu_array& operator/=(T rhs) {
                gpu_array::work(*this, this->size(), "divide_single_inplace", data, rhs);
                return *this;
            };
            gpu_array& operator+=(gpu_array const& rhs) {
                gpu_array::work(*this, this->size(), "add_inplace", data, rhs.data);
                return *this;
            };
            gpu_array& operator-=(gpu_array const& rhs) {
                gpu_array::work(*this, this->size(), "sub_inplace", data, rhs.data);
                return *this;
            };
            gpu_array& operator*=(gpu_array const& rhs) {
                gpu_array::work(*this, this->size(), "mult_inplace", data, rhs.data);
                return *this;
            };
            gpu_array& operator/=(gpu_array const& rhs) {
                gpu_array::work(*this, this->size(), "divide_inplace", data, rhs.data);
                return *this;
            };

            friend gpu_array operator+(gpu_array const& lhs, gpu_array const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "add", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array operator-(gpu_array const& lhs, gpu_array const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "sub", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array operator*(gpu_array const& lhs, gpu_array const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "mult", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array operator/(gpu_array const& lhs, gpu_array const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "divide", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array operator+(gpu_array const& lhs, T const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "add_single", lhs.data, rhs, out.data);
                return out;
            };
            friend gpu_array operator-(gpu_array const& lhs, T const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "sub_single", lhs.data, rhs, out.data);
                return out;
            };
            friend gpu_array operator*(gpu_array const& lhs, T const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "mult_single", lhs.data, rhs, out.data);
                return out;
            };
            friend gpu_array operator/(gpu_array const& lhs, T const& rhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "divide_single", lhs.data, rhs, out.data);
                return out;
            };
            friend gpu_array operator+(T const& rhs, gpu_array const& lhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "add_single", lhs.data, rhs, out.data);
                return out;
            };
            friend gpu_array operator-(T const& rhs, gpu_array const& lhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "sub_single_inv", lhs.data, rhs, out.data);
                return out;
            };
            friend gpu_array operator*(T const& rhs, gpu_array const& lhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "mult_single", lhs.data, rhs, out.data);
                return out;
            };
            friend gpu_array operator/(T const& rhs, gpu_array const& lhs) {
                auto out = gpu_array(lhs.dim);
                gpu_array::work(out, out.size(), "divide_single_inv", lhs.data, rhs, out.data);
                return out;
            };
            gpu_array copy() const {
                auto out = gpu_array(dim);
                gpu_array::work(out, out.size(), "copy", out.data, data);
                return out;
            };
            // cast from the current type to the requested type. E.g. from int to float, or char to unsigned long, etc.
            template<typename G> gpu_array<G> cast() const {
                if constexpr (std::is_same_v<G, T>) {
                    return copy();
                }
                else {
                    static std::string CastFunc{ std::string("from_") + opencl_impl::type_name<T>() }; // from_int
                    auto out = gpu_array<G>(this->dim);
                    gpu_array<G>::work(out, out.dim.count(), CastFunc, out.data, data);
                    return out;
                }
            };

            // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
            static gpu_array random(unsigned int X, unsigned int Y = 1, unsigned int Z = 1) {
                if constexpr (std::is_floating_point_v<T>) {
                    gpu_array out(dimensions{ X, Y, Z });
                    gpu_array::work(out, out.size(), "Rand", out.data);
                    return out;
                }
                else {
                    return (gpu_array<float>::random(X, Y, Z) * (float)std::numeric_limits<T>::max()).cast<T>();
                }
            };
            // returns a random number in the range of (lower, upper]
            static gpu_array random_between(T lower, T upper, unsigned int X, unsigned int Y = 1, unsigned int Z = 1) {
                if constexpr (std::is_floating_point_v<T>) {
                    gpu_array out(dimensions{ X, Y, Z });
                    gpu_array::work(out, out.size(), "Rand", out.data);
                    out *= (upper - lower);
                    out += lower;
                    return out;
                }
                else {
                    return gpu_array<float>::random_between((float)lower, (float)upper, X, Y, Z).cast<T>();
                }
            };
            // Returns a square 2-d matrix whose values are 1.0 along the diagonal, and 0.0 elsewhere.
            static gpu_array identity(unsigned int width) {
                gpu_array out(dimensions{ width, width, 1 });
                gpu_array::work(out, out.size(), "identity", out.data, (unsigned int)width);
                return out;
            };
            // Returns a matrix with all values linearly increasing from the low value to the high value based on their index. 
            static gpu_array linear(T low, T high, unsigned int lenX, unsigned int lenY = 1, unsigned int lenZ = 1) {
                int num_dim = std::max<int>(1, ((int)(lenZ > 1) + (int)(lenY > 1) + (int)(lenX > 1)));

                gpu_array out(dimensions{ lenX, lenY, lenZ });
                gpu_array::work(out, out.size(), "linear_between", out.data, low, high, (unsigned int)out.size());
                return out;
            };
            // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
            template <typename P> static gpu_array from_vector(const P& parameters) {
                unsigned int count = 0;
                for (auto& x : parameters) {
                    ++count;
                }
                gpu_array out(dimensions{ count, 1, 1 });
                count = 0;
                if (auto W = out.write()) {
                    for (auto& x : parameters) {
                        W[count++] = static_cast<T>(x);
                    }
                }
                return out;
            };
            // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
            template <typename P> static gpu_array from_vector(const P& parameters, unsigned int LenX) {
                unsigned int count = 0;
                for (auto& x : parameters) {
                    ++count;
                }
                gpu_array out(dimensions{ LenX, count / LenX, 1 });
                count = 0;
                if (auto W = out.write()) {
                    for (auto& x : parameters) {
                        W[count++] = static_cast<T>(x);
                    }
                }
                return out;
            };
            // Returns a matrix with all values equal to the provided value
            static gpu_array constant(T value, unsigned int lenX, unsigned int lenY = 1, unsigned int lenZ = 1) {
                gpu_array out(dimensions{ lenX, lenY, lenZ });
                gpu_array::work(out, out.size(), "copy_single", out.data, value);
                return out;
            };

            // specialization of POW for integer powers
            gpu_array pown(gpu_array<int> const& rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "power_n", data, rhs.data, out.data);
                return out;
            };
            // power of 
            gpu_array pow(gpu_array const& rhs) const {
                if constexpr (std::is_same_v<T, int>) return pown(rhs);
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "power", data, rhs.data, out.data);
                return out;
            };
            // specialization of POW for integer powers
            gpu_array pown(int rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "power_n_single", data, rhs, out.data);
                return out;
            };
            // power of 
            gpu_array pow(T rhs) const {
                if constexpr (std::is_same_v<T, int>) return pown(rhs);
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "power_single", data, rhs, out.data);
                return out;
            };
            // sqrt
            gpu_array<float> sqrt() const {
                gpu_array<float> out(this->dim);
                gpu_array<float>::work(out, out.size(), "square_root", data, out.data);
                return out;
            };
            // round to nearest whole number
            gpu_array round() const {
                if constexpr (std::is_floating_point_v<T>) {
                    gpu_array out(this->dim);
                    gpu_array::work(out, out.size(), "round", data, out.data);
                    return out;
                }
                else {
                    return copy();
                }
            };
            // round to higher integer
            gpu_array ceil() const {
                if constexpr (std::is_floating_point_v<T>) {
                    gpu_array out(this->dim);
                    gpu_array::work(out, out.size(), "ceil", data, out.data);
                    return out;
                }
                else {
                    return copy();
                }
            };
            // round to lower integer
            gpu_array floor() const {
                if constexpr (std::is_floating_point_v<T>) {
                    gpu_array out(this->dim);
                    gpu_array::work(out, out.size(), "flr", data, out.data);
                    return out;
                }
                else {
                    return copy();
                }
            };
            // return (this * multiply) + add;
            gpu_array fma(gpu_array const& multiply, gpu_array const& add) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "mult_add", data, multiply.data, add.data, out.data);
                return out;
            };
            // absolute value
            gpu_array abs() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "absolute", data, out.data);
                return out;
            };

            gpu_array cos() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Cos", data, out.data);
                return out;
            };
            gpu_array sin() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Sin", data, out.data);
                return out;
            };
            gpu_array tan() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Tan", data, out.data);
                return out;
            };
            gpu_array acos() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "aCos", data, out.data);
                return out;
            };
            gpu_array asin() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "aSin", data, out.data);
                return out;
            };
            gpu_array atan() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "aTan", data, out.data);
                return out;
            };
            gpu_array cosh() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Cosh", data, out.data);
                return out;
            };
            gpu_array sinh() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Sinh", data, out.data);
                return out;
            };
            gpu_array tanh() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Tanh", data, out.data);
                return out;
            };
            gpu_array acosh() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "aCosh", data, out.data);
                return out;
            };
            gpu_array asinh() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "aSinh", data, out.data);
                return out;
            };
            gpu_array atanh() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "aTanh", data, out.data);
                return out;
            };
            // e^x
            gpu_array exp() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Exp", data, out.data);
                return out;
            };
            // 2^x
            gpu_array exp2() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Exp2", data, out.data);
                return out;
            };
            // 10^x
            gpu_array exp10() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Exp10", data, out.data);
                return out;
            };
            // e^x-1
            gpu_array expm1() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Expm1", data, out.data);
                return out;
            };
            // log gamma function
            gpu_array lgamma() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Lgamma", data, out.data);
                return out;
            };
            // ln(x)
            gpu_array log() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Log", data, out.data);
                return out;
            };
            // log_2(x)
            gpu_array log2() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Log2", data, out.data);
                return out;
            };
            // log_10(x)
            gpu_array log10() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Log10", data, out.data);
                return out;
            };
            // ln(1+x)
            gpu_array log1p() const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Log1p", data, out.data);
                return out;
            };
            // return this % rhs
            gpu_array mod(T rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Mod_single", data, rhs, out.data);
                return out;
            };
            // return this % rhs
            gpu_array mod(gpu_array const& rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Mod", data, rhs.data, out.data);
                return out;
            };
            friend gpu_array operator%(gpu_array const& lhs, gpu_array const& rhs) {
                return lhs.mod(rhs);
            };
            friend gpu_array operator%(gpu_array const& lhs, T rhs) {
                return lhs.mod(rhs);
            };
            // returns the max of the two arrays (item-by-item, as an array)
            gpu_array max(gpu_array const& rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Max", data, rhs.data, out.data);
                return out;
            };
            // returns the max of the two arrays (item-by-item, as an array)
            gpu_array max(T rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Max_single", data, rhs, out.data);
                return out;
            };
            // returns the min of the two arrays (item-by-item, as an array)
            gpu_array min(gpu_array const& rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Min", data, rhs.data, out.data);
                return out;
            };
            // returns the min of the two arrays (item-by-item, as an array)
            gpu_array min(T rhs) const {
                gpu_array out(this->dim);
                gpu_array::work(out, out.size(), "Min_single", data, rhs, out.data);
                return out;
            };
            gpu_array<unsigned int> operator!() const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_not", out.data, data);
                return out;
            };
            gpu_array<unsigned int> operator==(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_eq_single", data, rhs, out.data);
                return out;
            };
            gpu_array<unsigned int> operator!=(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_neq_single", data, rhs, out.data);
                return out;
            };
            gpu_array<unsigned int> operator<(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_ls_single", data, rhs, out.data);
                return out;
            };
            gpu_array<unsigned int> operator<=(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_lse_single", data, rhs, out.data);
                return out;
            };
            gpu_array<unsigned int> operator>(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_gr_single", data, rhs, out.data);
                return out;
            };
            gpu_array<unsigned int> operator>=(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_gre_single", data, rhs, out.data);
                return out;
            };
            friend gpu_array<unsigned int> operator==(gpu_array const& lhs, gpu_array const& rhs) {
                gpu_array<unsigned int> out(lhs.dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_eq", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array<unsigned int> operator!=(gpu_array const& lhs, gpu_array const& rhs) {
                gpu_array<unsigned int> out(lhs.dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_neq", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array<unsigned int> operator<(gpu_array const& lhs, gpu_array const& rhs) {
                gpu_array<unsigned int> out(lhs.dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_ls", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array<unsigned int> operator<=(gpu_array const& lhs, gpu_array const& rhs) {
                gpu_array<unsigned int> out(lhs.dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_lse", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array<unsigned int> operator>(gpu_array const& lhs, gpu_array const& rhs) {
                gpu_array<unsigned int> out(lhs.dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_gr", lhs.data, rhs.data, out.data);
                return out;
            };
            friend gpu_array<unsigned int> operator>=(gpu_array const& lhs, gpu_array const& rhs) {
                gpu_array<unsigned int> out(lhs.dim);   
                gpu_array<unsigned int>::work(out, out.size(), "item_gre", lhs.data, rhs.data, out.data);
                return out;
            };

            gpu_array<unsigned int> operator&&(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_AND_single", out.data, data, rhs);
                return out;
            };
            gpu_array<unsigned int> operator&&(gpu_array const& rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_AND", out.data, data, rhs.data);
                return out;
            };
            gpu_array<unsigned int> operator||(T rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_OR_single", out.data, data, rhs);
                return out;
            };
            gpu_array<unsigned int> operator||(gpu_array const& rhs) const {
                gpu_array<unsigned int> out(this->dim);
                gpu_array<unsigned int>::work(out, out.size(), "item_OR", out.data, data, rhs.data);
                return out;
            };
            // joins two matrices along one of the dimensions.
            gpu_array join(unsigned int jdim, gpu_array const& first) const {
                // All dimensions except join dimension must be equal
                for (unsigned int I = 0; I < 3; ++I) {
                    if (I == jdim) continue;
                    if (this->size(I) != first.size(I)) {
                        return gpu_array();
                    }
                }

                // Compute output dims
                unsigned int
                    NewX = this->size(0) + first.size(0) * (jdim == 0),
                    NewY = this->size(1) + first.size(1) * (jdim == 1),
                    NewZ = this->size(2) + first.size(2) * (jdim == 2);

                gpu_array out(dimensions{ NewX, NewY, NewZ });
                if (jdim == 0) {
                    gpu_array::work(out, out.size(), "join_dim_0", out.data, this->data, (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, first.data, (unsigned int)first.dim.X);
                }
                else if (jdim == 1) {
                    gpu_array::work(out, out.size(), "join_dim_1", out.data, this->data, (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, first.data, (unsigned int)first.dim.Y);
                }
                else {
                    gpu_array::work(out, out.size(), "join_dim_2", out.data, this->data, (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, first.data);
                }

                return out;
            };
            // transpose a 2-D matrix along its diagonal. Does not support transposition of 3-D matrices. 
            gpu_array transpose() const {
                // matrix must be 2-D
                if (this->dim.num_dimensions() == 0) return gpu_array();
                else if (this->dim.num_dimensions() > 2) return gpu_array();

                gpu_array out(dimensions{ this->dim.Y, this->dim.X, 1 });
                gpu_array::work(out, out.size(), "Transpose", out.data, data, (unsigned int)dim.X, (unsigned int)dim.Y);
                return out;
            };
            // pad a matrix with zeros to make its X and Y components square. Used for calculating the inverse. 
            gpu_array make_square() const {
                unsigned int len = std::max<unsigned int>(dim.X, dim.Y);

                gpu_array out(dimensions{ len, len, 1 });
                gpu_array::work(out, out.size(), "make_square", out.data, data, (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z, (unsigned int)len);

                return out;
            }
            // extracts the diagonal of a 2-D matrix as a 1-D array
            gpu_array diagonal() const {
                if (this->dim.num_dimensions() == 0) return gpu_array();
                else if (this->dim.num_dimensions() == 1) return this->copy();
                else if (this->dim.num_dimensions() > 2) return gpu_array();
                gpu_array out(dimensions{ std::min<unsigned int>(this->dim.X, this->dim.Y), 1, 1 });
                gpu_array::work(out, this->size(), "diagonal", out.data, data, dim.X);
                return out;
            };
            // extract a row from this 2-D matrix as a 1-D array
            gpu_array row(unsigned int rowN) const {
                gpu_array out(dimensions{ dim.Y, dim.Z, 1 });
                gpu_array::work(out, out.size(), "row_of", out.data, data, (unsigned int)rowN, (unsigned int)dim.X, (unsigned int)dim.Y, (unsigned int)dim.Z);
                return out;
            };
            // grow a matrix by wrapping the new values around to the start. Only works for a 1-D vector. 
            gpu_array grow_by_wrapping(unsigned int new_length) const {
                if (this->dim.num_dimensions() == 1) {
                    gpu_array out(dimensions{ new_length, 1, 1 });
                    gpu_array::work(out, out.size(), "wrap_around", out.data, data, (unsigned int)this->size());
                    return out;
                }
                else {
                    // ??
                    throw std::runtime_error("Cannot grow a matrix by wrapping -- yet. Depends on how we want to grow it? Y-axis growth is off, but X-axis growth makes sense with wrapping");
                }
            };
            // create a new array by sampling this array at the provided indices. E.g. This = [5,4,3,2,1,0]
            // Indices = [5,5,5,5,5,5,5,4,4,4,4,4,4,3,3,3,3,3,2,2,2,2,1,1,1,0,0]
            // Result = [0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5]
            gpu_array resample(gpu_array<unsigned int> const& sample_indices) const {
                gpu_array out(sample_indices.dim);
                gpu_array::work(out, out.size(), "resample", out.data, data, sample_indices.data);
                return out;
            };

            // calculate the determinant for a square matrix. Performed on the CPU, and minimizes exchanges with the GPU. 
            template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
            float determinant() const {
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
                        gpu_array subVect(dimensions{ dimension - 1, dimension - 1, 1 }, true);
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

            // cofactor of a square matrix, essential for calculating the inverse
            template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
            gpu_array cofactor() const {
                if (this->dim.X != this->dim.Y) {
                    return make_square().cofactor();
                }
                unsigned int dimension = this->dim.X;
                gpu_array solution(dimensions{ dimension, dimension, 1 });
                if (auto W1 = solution.write()) {
                    gpu_array subVect(dimensions{ dimension - 1, dimension - 1, 1 }, true);
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
            };

            // transpose of the cofactor of a square matrix
            template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
            gpu_array adjoint() const {
                return cofactor().transpose();
            };

            // solve for the inverse of the matrix. Does not support solving for the inverse of a 3-D matrix. 
            template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
            gpu_array inverse() const {
                return adjoint() / std::abs(determinant());
            };

            // performs a cross-multiplication of two rectangular matrices. This is not accelerated by the GPU, and is CPU-bound. Uses CPU multithreading to (attempt) to speed-up this bottleneck. 
            // the number of columns in this matrix must equal the number of rows in the RHS matrix. 
            template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
            gpu_array matrix_multiply(gpu_array const& rhs) const {
                if (this->dim.Y == rhs.dim.X) {
                    // only useful for dim-2 matrices. 
                    unsigned int final_num_rows = this->dim.X;
                    unsigned int final_num_cols = rhs.dim.Y;

                    gpu_array out(dimensions{ final_num_rows, final_num_cols, 1 });
                    auto R_lhs = this->read();
                    auto R_rhs = rhs.read();
                    if (auto W = out.write()) {
                        if (R_lhs && R_rhs && W) {
                            auto N = out.dim.count();
                            for (unsigned int n = 0; n < N; ++n) {
                                if (n >= N) continue;

                                // parallel::Std_For<unsigned int>(0, out.size(), [&](unsigned int n) {
                                T v = (T)0;
                                const unsigned int destination_Y = (unsigned int)std::floor((long double)n / (long double)final_num_rows);
                                const unsigned int destination_X = n - (final_num_rows * destination_Y);
                                for (unsigned int index = 0; index < this->dim.Y; ++index) {
                                    v += R_lhs(destination_X, index) * R_rhs(index, destination_Y);
                                }
                                W[n] = v;
                            } // );
                        }
                    }
                    return out;
                }
                else if (this->dim.Y > rhs.dim.X) {
                    return matrix_multiply(rhs.copy().join(0, gpu_array(dimensions{ this->dim.Y - rhs.dim.X, rhs.dim.Y, rhs.dim.Z }) = 1));
                }
                else /*if (this->LenY < rhs.LenX)*/ {
                    // To-Do: need to set final column in joining array to 1?
                    return this->copy().join(1, gpu_array(dimensions{ this->dim.X, rhs.dim.X - this->dim.Y, this->dim.Z }) = 0).matrix_multiply(rhs);
                }
            };

            // test to see if there is any colinearity in the feature set. If so, it is impossible to solve for the linear regression. One or multiple features must be removed until it is no longer invalid.
            template<typename = std::enable_if_t<std::is_floating_point_v<T>>>
            bool is_colinear() const {
                return std::abs(this->transpose().matrix_multiply(*this).determinant()) == 0;
            };

            template<bool use_cpu = false>
            T sum() const {
                if constexpr (use_cpu) {
                    auto N = this->size();
                    T out = (T)0;
                    if (auto R = this->read()) {
                        for (unsigned int n = 0; n < N; ++n) {
                            out += R(n);
                        }
                    }
                    return out;
                }
                else {
                    if (this->size() > 1000) {
                        T out = (T)0;
                        if (1) {
                            gpu_array Temp(dimensions{ (unsigned int)std::ceil((long double)(this->size()) / (long double)64), 1, 1 });
                            gpu_array::work(Temp, this->size(), "reduce_sum", data, Temp.data, (unsigned int)this->size());
                            auto N = Temp.size();
                            if (auto R = Temp.read()) {
                                for (unsigned int n = 0; n < N; ++n) {
                                    out += R(n);
                                }
                            }
                        }
                        return out;
                    }
                    else {
                        auto N = this->size();
                        T out = (T)0;
                        if (auto R = this->read()) {
                            for (unsigned int n = 0; n < N; ++n) {
                                out += R(n);
                            }
                        }
                        return out;
                    }
                }
            };
            T avg() const {
                return (T)((long double)sum() / (long double)this->size());
            };
            T max() const {
                if (this->size() > 1000) {
                    gpu_array out(dimensions{ (unsigned int)std::ceil((long double)(this->size()) / (long double)64), 1, 1 });
                    gpu_array::work(out, this->size(), "reduce_max", data, out.data, (unsigned int)this->size(), std::numeric_limits<T>::lowest());
                    return out.max();
                }
                else {
                    auto N = this->size();
                    T out = std::numeric_limits<T>::lowest();
                    if (auto R = this->read()) {
                        for (unsigned int n = 0; n < N; ++n) {
                            out = std::max(out, R(n));
                        }
                    }
                    return out;
                }
            };
            T min() const {
                if (this->size() > 1000) {
                    gpu_array out(dimensions{ (unsigned int)std::ceil((long double)(this->size()) / (long double)64), 1, 1 });
                    gpu_array::work(out, this->size(), "reduce_min", data, out.data, (unsigned int)this->size(), std::numeric_limits<T>::max());
                    return out.min();
                }
                else {
                    auto N = this->size();
                    T out = std::numeric_limits<T>::max();
                    if (auto R = this->read()) {
                        for (unsigned int n = 0; n < N; ++n) {
                            out = std::min(out, R(n));
                        }
                    }
                    return out;
                }
            };

            gpu_array convolve(gpu_array const& kernel) const {
                if (this->dim.num_dimensions() == 2) {
                    gpu_array out(this->dim);
                    float kernel_tot = kernel.sum();
                    gpu_array::work(out, this->size(), "convolve", out.data, data, kernel.data, this->size(0), this->size(1), kernel.size(0), kernel.size(1), kernel_tot);
                    return out;
                }
                else {
                    throw std::runtime_error("Convolution not supported for this number of dimensions");
                }
            };
            static gpu_array<float> guassian_kernel(unsigned int X, unsigned int Y) {
                gpu_array<float> out(dimensions{ X, Y, 1 });
                gpu_array<float>::work(out, out.size(), "guassian", out.data, out.size(0), out.size(1));
                return out * (1.0f / out.sum());
            };

        private:
            static std::string resize(std::string&& rhs, unsigned int len, const char def = 0) {
                rhs.resize(len, def);
                return std::move(rhs);
            };
            std::string to_string_impl(reader const& R, unsigned int x) const {
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
            std::string to_string_impl(reader const& R, unsigned int x, unsigned int y) const {
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
            std::string to_string_impl(reader const& R, unsigned int x, unsigned int y, unsigned int z) const {
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
            std::vector<unsigned int> evaluate_column_sizes(reader const& R, std::vector<std::string> column_titles = {}) const {
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

        public:
            // y-axis are columns, x-axis are rows. Z-axis is ignored (for now). 
            std::string to_string(std::vector<std::string> column_titles = {}, bool doNotSkip = false) const {
                reader R = this->read();
                std::string column_spacer = " ";
                std::string out;
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
            friend std::ostream& operator<<(std::ostream& os, gpu_array const& obj) {
                os << obj.to_string();
                return os;
            };

        };
        namespace linear_regressions {
            using matrix = gpu_array<float>;
            // solve for the weights to be used when performing linearized predictions, as determined by a basic linear regression.
            __forceinline static matrix solve_for_weights(matrix const& measurements, matrix const& features) {
                return (features.transpose().matrix_multiply(features)).inverse().matrix_multiply(features.transpose()).matrix_multiply(measurements);
            };
            // solve for the linearized prediction.
            __forceinline static matrix predict(matrix const& features, matrix const& weights) {
                return features.matrix_multiply(weights);
            };
            // returns the standard error of the linear regression.
            __forceinline static matrix standard_error(matrix const& measurements, matrix const& features, matrix const& weights) {
                auto prediction = predict(features, weights);
                return ((((measurements - prediction).pow(2.0f).sum() / std::max<float>(1.0f, static_cast<float>(features.size(0)) - 2.0)) * (features.transpose().matrix_multiply(features)).inverse()).pow(0.5)).diagonal();
            };
            // returns the population standard deviation.
            __forceinline static matrix standard_deviation(
                matrix const& measurements,
                matrix const& features,
                matrix const& weights
            ) {
                return standard_error(measurements, features, weights) * std::sqrt(measurements.size(0));
            };
            // evaluate for the students-t test
            __forceinline static matrix t_statistic(matrix const& weights, matrix const& std_err) {
                return weights / std_err;
            };
            // evaluate for the p-value
            __forceinline static matrix p_value(matrix const& features, matrix const& t_stat) {
                boost::math::students_t dist(features.size(0) - features.size(1)); // n - k - 1, but should include the intercept in the features list already
                matrix out(dimensions{ t_stat.size(0), 1, 1 });
                unsigned int N = out.size();
                if (auto R = t_stat.read()) {
                    if (auto W = out.write()) {
                        for (unsigned int i = 0; i < N; ++i) {
                            W[i] = (1.0f - (float)boost::math::cdf(dist, R[i])) + boost::math::cdf(dist, -R[i]);
                            if ((W[i] > 1.0f) || (W[i] < 0.0f))
                                W[i] = (1.0f - (float)boost::math::cdf(dist, -R[i])) + boost::math::cdf(dist, R[i]);
                        }
                    }
                }
                return out.copy();
            };

            // build a collection of features for a linear regression while avoiding colinearity. 
            __declspec(noinline) static matrix build_features(matrix const& current_best) {
                return current_best.copy();
            };
            // build a collection of features for a linear regression while avoiding colinearity. 
            __declspec(noinline) static matrix build_features(matrix&& current_best) {
                return std::move(current_best);
            };
            // build a collection of features for a linear regression while avoiding colinearity. 
            template <typename T, typename... Ts> __declspec(noinline) static matrix build_features(matrix const& current_best, T const& candidate, const Ts&... further_candidates) {
                auto joined = current_best.join(1, candidate);
                if (joined.is_colinear()) {
                    return build_features(current_best, further_candidates...);
                }
                else {
                    return build_features(joined, further_candidates...);
                }
            };

            // build a collection of features for a linear regression while avoiding colinearity. 
            __declspec(noinline) static matrix build_features_fast(matrix const& current_best) {
                return current_best.copy();
            };
            // build a collection of features for a linear regression while avoiding colinearity. 
            __declspec(noinline) static matrix build_features_fast(matrix&& current_best) {
                return std::move(current_best);
            };
            // build a collection of features for a linear regression while avoiding colinearity. 
            template <typename T, typename... Ts> __declspec(noinline) static matrix build_features_fast(matrix const& current_best, T const& candidate, const Ts&... further_candidates) {
                return build_features_fast(current_best.join(1, candidate), further_candidates...);                
            };

        };
    };
}


#pragma region PUBLIC GPU-ACCELERATED, TYPE-ERASUED ARRAY
namespace GL {
    namespace GPU {
        class Array;
        static std::shared_ptr<void> array_initialize(ArrayTypes T, unsigned int X, unsigned int Y, unsigned int Z) {
            switch (T) {
            default: return nullptr;
            case ArrayTypes::CHAR: return std::static_pointer_cast<void>(std::make_shared<gpu_array<char>>(X, Y, Z));
            case ArrayTypes::UCHAR: return std::static_pointer_cast<void>(std::make_shared<gpu_array<unsigned char>>(X, Y, Z));
            case ArrayTypes::INT: return std::static_pointer_cast<void>(std::make_shared<gpu_array<int>>(X, Y, Z));
            case ArrayTypes::UINT: return std::static_pointer_cast<void>(std::make_shared<gpu_array<unsigned int>>(X, Y, Z));
            case ArrayTypes::LONG: return std::static_pointer_cast<void>(std::make_shared<gpu_array<long>>(X, Y, Z));
            case ArrayTypes::ULONG: return std::static_pointer_cast<void>(std::make_shared<gpu_array<unsigned long>>(X, Y, Z));
            case ArrayTypes::FLOAT: return std::static_pointer_cast<void>(std::make_shared<gpu_array<float>>(X, Y, Z));
            case ArrayTypes::DOUBLE: return std::static_pointer_cast<void>(std::make_shared<gpu_array<double>>(X, Y, Z));
            }
        };
        static std::shared_ptr<void> array_initialize(ArrayTypes T) {
            switch (T) {
            default: return nullptr;
            case ArrayTypes::CHAR: return std::static_pointer_cast<void>(std::make_shared<gpu_array<char>>());
            case ArrayTypes::UCHAR: return std::static_pointer_cast<void>(std::make_shared<gpu_array<unsigned char>>());
            case ArrayTypes::INT: return std::static_pointer_cast<void>(std::make_shared<gpu_array<int>>());
            case ArrayTypes::UINT: return std::static_pointer_cast<void>(std::make_shared<gpu_array<unsigned int>>());
            case ArrayTypes::LONG: return std::static_pointer_cast<void>(std::make_shared<gpu_array<long>>());
            case ArrayTypes::ULONG: return std::static_pointer_cast<void>(std::make_shared<gpu_array<unsigned long>>());
            case ArrayTypes::FLOAT: return std::static_pointer_cast<void>(std::make_shared<gpu_array<float>>());
            case ArrayTypes::DOUBLE: return std::static_pointer_cast<void>(std::make_shared<gpu_array<double>>());
            }
        };
        template<typename T> static ArrayTypes TypeOf() {
            if constexpr (std::is_same_v<T, char>) return ArrayTypes::CHAR;
            else if constexpr (std::is_same_v<T, unsigned char>) return ArrayTypes::UCHAR;
            else if constexpr (std::is_same_v<T, int>) return ArrayTypes::INT;
            else if constexpr (std::is_same_v<T, unsigned int>) return ArrayTypes::UINT;
            else if constexpr (std::is_same_v<T, long>) return ArrayTypes::LONG;
            else if constexpr (std::is_same_v<T, unsigned long>) return ArrayTypes::ULONG;
            else if constexpr (std::is_same_v<T, float>) return ArrayTypes::FLOAT;
            else if constexpr (std::is_same_v<T, double>) return ArrayTypes::DOUBLE;
            else return ArrayTypes::EMPTY;
        };
        template<typename T> static Array BuildArray(gpu_array<T>&& V) {
            return Array(TypeOf<T>(), std::static_pointer_cast<void>(std::make_shared<gpu_array<T>>(std::move(V))));
        };
        template <typename T> static auto& get_array(std::shared_ptr<void> const& rhs) {
            return *std::static_pointer_cast<gpu_array<T>>(rhs);
        };
        template <typename F> static auto visit_array(ArrayTypes _type, std::shared_ptr<void> const& _data, F const& func) {
            switch (_type) {
            default: throw std::runtime_error("Array was empty");
            case ArrayTypes::CHAR: return func(get_array<char>(_data));
            case ArrayTypes::UCHAR: return func(get_array<unsigned char>(_data));
            case ArrayTypes::INT: return func(get_array<int>(_data));
            case ArrayTypes::UINT: return func(get_array<unsigned int>(_data));
            case ArrayTypes::LONG: return func(get_array<long>(_data));
            case ArrayTypes::ULONG: return func(get_array<unsigned long>(_data));
            case ArrayTypes::FLOAT: return func(get_array<float>(_data));
            case ArrayTypes::DOUBLE: return func(get_array<double>(_data));
            }
        };
        template <typename T> static auto& get_reader(std::shared_ptr<void> const& _reader_impl) {
            return *std::static_pointer_cast<gpu_array<T>::reader>(_reader_impl);
        };
        template <typename F> static auto visit_reader(ArrayTypes _type, std::shared_ptr<void> const& _reader_impl, F const& func) {
            switch (_type) {
            default: throw std::runtime_error("Array was empty");
            case ArrayTypes::CHAR: return func(get_reader<char>(_reader_impl));
            case ArrayTypes::UCHAR: return func(get_reader<unsigned char>(_reader_impl));
            case ArrayTypes::INT: return func(get_reader<int>(_reader_impl));
            case ArrayTypes::UINT: return func(get_reader<unsigned int>(_reader_impl));
            case ArrayTypes::LONG: return func(get_reader<long>(_reader_impl));
            case ArrayTypes::ULONG: return func(get_reader<unsigned long>(_reader_impl));
            case ArrayTypes::FLOAT: return func(get_reader<float>(_reader_impl));
            case ArrayTypes::DOUBLE: return func(get_reader<double>(_reader_impl));
            }
        };
        template <typename T> static auto& get_writer(std::shared_ptr<void> const& _reader_impl) {
            return *std::static_pointer_cast<gpu_array<T>::writer>(_reader_impl);
        };
        template <typename F> static auto visit_writer(ArrayTypes _type, std::shared_ptr<void> const& _reader_impl, F const& func) {
            switch (_type) {
            default: throw std::runtime_error("Array was empty");
            case ArrayTypes::CHAR: return func(get_writer<char>(_reader_impl));
            case ArrayTypes::UCHAR: return func(get_writer<unsigned char>(_reader_impl));
            case ArrayTypes::INT: return func(get_writer<int>(_reader_impl));
            case ArrayTypes::UINT: return func(get_writer<unsigned int>(_reader_impl));
            case ArrayTypes::LONG: return func(get_writer<long>(_reader_impl));
            case ArrayTypes::ULONG: return func(get_writer<unsigned long>(_reader_impl));
            case ArrayTypes::FLOAT: return func(get_writer<float>(_reader_impl));
            case ArrayTypes::DOUBLE: return func(get_writer<double>(_reader_impl));
            }
        };

        Array::Array(ArrayTypes type) : _type(type), _data{ array_initialize(type) } {};
        Array::Array(ArrayTypes type, unsigned int X, unsigned int Y, unsigned int Z) : _type(type), _data{ array_initialize(type, X, Y, Z) } {};
        Array::reader::reader(ArrayTypes T, std::shared_ptr<void>&& _reader) : _type(T), _reader_impl(std::move(_reader)) {};
        Array::reader::operator bool() const {
            return visit_reader(_type, _reader_impl, [](auto& read) {
                return read.operator bool();
                });
        };
        Number Array::reader::operator[](unsigned int X) const {
            return visit_reader(_type, _reader_impl, [&](auto& read) {
                return Number(read[X]);
                });
        };
        Number Array::reader::operator()(unsigned int X, unsigned int Y, unsigned int Z) const {
            return visit_reader(_type, _reader_impl, [&](auto& read) {
                return Number(read(X, Y, Z));
                });
        };
        Array::writer::writer(ArrayTypes T, std::shared_ptr<void>&& _reader) : _type(T), _reader_impl(std::move(_reader)) {};
        Array::writer::operator bool() const {
            return visit_writer(_type, _reader_impl, [](auto& read) {
                return read.operator bool();
                });
        };
        Number Array::writer::load(unsigned int X, unsigned int Y, unsigned int Z) const {
            return visit_writer(_type, _reader_impl, [&](auto& read) {
                return Number(read(X, Y, Z));
                });
        };
        void Array::writer::store(Number V, unsigned int X, unsigned int Y, unsigned int Z) const {
            visit_writer(_type, _reader_impl, [&](auto& read) {
                read(X, Y, Z) = V;
                });
        };
        Number Array::writer::exchange(Number V, unsigned int X, unsigned int Y, unsigned int Z) const {
            return visit_writer(_type, _reader_impl, [&](auto& read) {
                Number out = read(X, Y, Z);
                read(X, Y, Z) = V;
                return out;
                });
        };
        // wrapper that allows reads the current values from the GPU buffer. Reading is done once on construction. 
        Array::reader Array::read() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return Array::reader(this->_type, std::static_pointer_cast<void>(std::make_shared<typename std::decay_t<decltype(arr)>::reader>(arr.read())));
                });
        };
        // wrapper that allows overwritting the current values on the GPU buffer. Updates are queued until the wrapper is destroyed then submitted all-at-once. 
        Array::writer Array::write() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return Array::writer(this->_type, std::static_pointer_cast<void>(std::make_shared<typename std::decay_t<decltype(arr)>::writer>(arr.write())));
                });
        };
        unsigned int Array::size() const {
            return visit_array(_type, _data, [](auto& arr) {
                return arr.size();
                });
        }
        unsigned int Array::size(unsigned int D) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return arr.size(D);
                });
        }
        Array Array::copy() const {
            return visit_array(_type, _data, [](auto& arr) {
                return BuildArray(arr.copy());
                });
        };
        Array& Array::operator=(Number rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr = rhs;
                });
            return *this;
        };
        Array& Array::operator+=(Number rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr += rhs;
                });
            return *this;
        };
        Array& Array::operator-=(Number rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr -= rhs;
                });
            return *this;
        };
        Array& Array::operator*=(Number rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr *= rhs;
                });
            return *this;
        };
        Array& Array::operator/=(Number rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr /= rhs;
                });
            return *this;
        };
        Array& Array::operator+=(Array const& rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr += get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data);
                });
            return *this;
        };
        Array& Array::operator-=(Array const& rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr -= get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data);
                });
            return *this;
        };
        Array& Array::operator*=(Array const& rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr *= get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data);
                });
            return *this;
        };
        Array& Array::operator/=(Array const& rhs) {
            visit_array(_type, _data, [&](auto& arr) {
                arr /= get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data);
                });
            return *this;
        };
        Array operator+(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr + get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator-(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr - get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator*(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr * get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator/(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr / get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator+(Array const& lhs, Number rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr + rhs);
                });
        };
        Array operator-(Array const& lhs, Number rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr - rhs);
                });
        };
        Array operator*(Array const& lhs, Number rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr * rhs);
                });
        };
        Array operator/(Array const& lhs, Number rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr / rhs);
                });
        };
        Array operator+(Number rhs, Array const& lhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(rhs + arr);
                });
        };
        Array operator-(Number rhs, Array const& lhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(rhs - arr);
                });
        };
        Array operator*(Number rhs, Array const& lhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(rhs * arr);
                });
        };
        Array operator/(Number rhs, Array const& lhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(rhs / arr);
                });
        };
        Array Array::operator!() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(!arr);
                });
        };
        Array Array::operator==(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr == rhs);
                });
        };
        Array Array::operator!=(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr != rhs);
                });
        };
        Array Array::operator<(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr < rhs);
                });
        };
        Array Array::operator<=(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr <= rhs);
                });
        };
        Array Array::operator>(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr > rhs);
                });
        };
        Array Array::operator>=(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr >= rhs);
                });
        };
        Array operator==(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr == get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator!=(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr != get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator<(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr < get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator<=(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr <= get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator>(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr > get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator>=(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr >= get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator%(Array const& lhs, Array const& rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr % get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array operator%(Array const& lhs, Number rhs) {
            return visit_array(lhs._type, lhs._data, [&](auto& arr) {
                return BuildArray(arr % rhs);
                });
        };
        Array Array::operator&&(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr && rhs);
                });
        };
        Array Array::operator&&(Array const& rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr && get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };
        Array Array::operator||(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr || rhs);
                });
        };
        Array Array::operator||(Array const& rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr || get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data));
                });
        };




        // power of 
        Array Array::pow(Array const& rhs) const {
            if (rhs._type == ArrayTypes::INT) {
                return visit_array(_type, _data, [&](auto& arr) {
                    return BuildArray(arr.pown(get_array<int>(rhs._data)));
                    });
            }
            else {
                return visit_array(_type, _data, [&](auto& arr) {
                    return BuildArray(arr.pow(get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data)));
                    });
            }
        };
        // specialization of POW for integer powers
        Array Array::pown(int rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.pown(rhs));
                });
        };
        // power of 
        Array Array::pow(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.pow(rhs));
                });
        };
        // sqrt
        Array Array::sqrt() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.sqrt());
                });
        };
        // round to nearest whole number
        Array Array::round() const {
            if (_type == ArrayTypes::FLOAT || _type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    return BuildArray(arr.round());
                    });
            }
            else {
                return *this;
            }
        };
        // round to higher integer
        Array Array::ceil() const {
            if (_type == ArrayTypes::FLOAT || _type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    return BuildArray(arr.ceil());
                    });
            }
            else {
                return *this;
            }
        };
        // round to lower integer
        Array Array::floor() const {
            if (_type == ArrayTypes::FLOAT || _type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    return BuildArray(arr.floor());
                    });
            }
            else {
                return *this;
            }
        };
        // absolute value
        Array Array::abs() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.abs());
                });
        };
        Array Array::cos() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.cos());
                });
        };
        Array Array::sin() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.sin());
                });
        };
        Array Array::tan() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.tan());
                });
        };
        Array Array::acos() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.acos());
                });
        };
        Array Array::asin() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.asin());
                });
        };
        Array Array::atan() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.atan());
                });
        };
        Array Array::cosh() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.cosh());
                });
        };
        Array Array::sinh() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.sinh());
                });
        };
        Array Array::tanh() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.tanh());
                });
        };
        Array Array::acosh() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.acosh());
                });
        };
        Array Array::asinh() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.asinh());
                });
        };
        Array Array::atanh() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.atanh());
                });
        };
        // e^x
        Array Array::exp() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.exp());
                });
        };
        // 2^x
        Array Array::exp2() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.exp2());
                });
        };
        // 10^x
        Array Array::exp10() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.exp10());
                });
        };
        // e^x-1
        Array Array::expm1() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.expm1());
                });
        };
        // log gamma function
        Array Array::lgamma() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.lgamma());
                });
        };
        // ln(x)
        Array Array::log() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.log());
                });
        };
        // log_2(x)
        Array Array::log2() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.log2());
                });
        };
        // log_10(x)
        Array Array::log10() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.log10());
                });
        };
        // ln(1+x)
        Array Array::log1p() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.log1p());
                });
        };
        // return this % rhs
        Array Array::mod(Number rhs) const {
            return *this % rhs;
        };
        // return this % rhs
        Array Array::mod(Array const& rhs) const {
            return *this % rhs;
        };
        // return (this * multiply) + add;
        Array Array::fma(Array const& multiply, Array const& add) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.fma(get_array<typename typename std::decay_t<decltype(arr)>::type>(multiply._data), get_array<typename typename std::decay_t<decltype(arr)>::type>(add._data)));
                });
        };
        // returns the max of the two arrays (item-by-item, as an array)
        Array Array::max(Array const& rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.max(get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data)));
                });
        };
        // returns the max of the two arrays (item-by-item, as an array)
        Array Array::max(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.max(rhs));
                });
        };
        // returns the min of the two arrays (item-by-item, as an array)
        Array Array::min(Array const& rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.min(get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data)));
                });
        };
        // returns the min of the two arrays (item-by-item, as an array)
        Array Array::min(Number rhs) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.min(rhs));
                });
        };
        std::string Array::to_string(std::vector<std::string> column_titles, bool doNotSkip) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return arr.to_string(column_titles, doNotSkip);
                });
        };
        std::ostream& operator<<(std::ostream& os, Array const& obj) {
            os << obj.to_string();
            return os;
        };
        // cast from the current type to the requested type. E.g. from int to float, or char to unsigned long, etc.
        Array Array::cast(ArrayTypes T) const {
            if (T == this->_type) return *this;
            return visit_array(_type, _data, [&](auto& arr) {
                switch (T) {
                default: throw std::runtime_error("Cannot cast to empty type");
                case ArrayTypes::CHAR: return BuildArray(arr.cast<char>());
                case ArrayTypes::UCHAR: return BuildArray(arr.cast<unsigned char>());
                case ArrayTypes::INT: return BuildArray(arr.cast<int>());
                case ArrayTypes::UINT: return BuildArray(arr.cast<unsigned int>());
                case ArrayTypes::LONG: return BuildArray(arr.cast<long>());
                case ArrayTypes::ULONG: return BuildArray(arr.cast<unsigned long>());
                case ArrayTypes::FLOAT: return BuildArray(arr.cast<float>());
                case ArrayTypes::DOUBLE: return BuildArray(arr.cast<double>());
                }
                });
        };
        // For floating-point values, returns 0-1. For all others, returns the range from 0 to the max value. 
        Array Array::random(ArrayTypes T, unsigned int X, unsigned int Y, unsigned int Z) {
            switch (T) {
            default: throw std::runtime_error("Cannot cast to empty type");
            case ArrayTypes::CHAR: return BuildArray(gpu_array<char>::random(X, Y, Z));
            case ArrayTypes::UCHAR: return BuildArray(gpu_array<unsigned char>::random(X, Y, Z));
            case ArrayTypes::INT: return BuildArray(gpu_array<int>::random(X, Y, Z));
            case ArrayTypes::UINT: return BuildArray(gpu_array<unsigned int>::random(X, Y, Z));
            case ArrayTypes::LONG: return BuildArray(gpu_array<long>::random(X, Y, Z));
            case ArrayTypes::ULONG: return BuildArray(gpu_array<unsigned long>::random(X, Y, Z));
            case ArrayTypes::FLOAT: return BuildArray(gpu_array<float>::random(X, Y, Z));
            case ArrayTypes::DOUBLE: return BuildArray(gpu_array<double>::random(X, Y, Z));
            }
        };
        // returns a random number in the range of (lower, upper]
        Array Array::random_between(ArrayTypes T, Number lower, Number upper, unsigned int X, unsigned int Y, unsigned int Z) {
            switch (T) {
            default: throw std::runtime_error("Cannot cast to empty type");
            case ArrayTypes::CHAR: return BuildArray(gpu_array<char>::random_between(lower, upper, X, Y, Z));
            case ArrayTypes::UCHAR: return BuildArray(gpu_array<unsigned char>::random_between(lower, upper, X, Y, Z));
            case ArrayTypes::INT: return BuildArray(gpu_array<int>::random_between(lower, upper, X, Y, Z));
            case ArrayTypes::UINT: return BuildArray(gpu_array<unsigned int>::random_between(lower, upper, X, Y, Z));
            case ArrayTypes::LONG: return BuildArray(gpu_array<long>::random_between(lower, upper, X, Y, Z));
            case ArrayTypes::ULONG: return BuildArray(gpu_array<unsigned long>::random_between(lower, upper, X, Y, Z));
            case ArrayTypes::FLOAT: return BuildArray(gpu_array<float>::random_between(lower, upper, X, Y, Z));
            case ArrayTypes::DOUBLE: return BuildArray(gpu_array<double>::random_between(lower, upper, X, Y, Z));
            }
        };
        // Returns a square 2-d matrix whose values are 1.0 along the diagonal, and 0.0 elsewhere.
        Array Array::identity(ArrayTypes T, unsigned int width) {
            switch (T) {
            default: throw std::runtime_error("Cannot cast to empty type");
            case ArrayTypes::CHAR: return BuildArray(gpu_array<char>::identity(width));
            case ArrayTypes::UCHAR: return BuildArray(gpu_array<unsigned char>::identity(width));
            case ArrayTypes::INT: return BuildArray(gpu_array<int>::identity(width));
            case ArrayTypes::UINT: return BuildArray(gpu_array<unsigned int>::identity(width));
            case ArrayTypes::LONG: return BuildArray(gpu_array<long>::identity(width));
            case ArrayTypes::ULONG: return BuildArray(gpu_array<unsigned long>::identity(width));
            case ArrayTypes::FLOAT: return BuildArray(gpu_array<float>::identity(width));
            case ArrayTypes::DOUBLE: return BuildArray(gpu_array<double>::identity(width));
            }
        };
        // Returns a matrix with all values linearly increasing from the low value to the high value based on their index. 
        Array Array::linear(ArrayTypes T, Number low, Number high, unsigned int X, unsigned int Y, unsigned int Z) {
            switch (T) {
            default: throw std::runtime_error("Cannot cast to empty type");
            case ArrayTypes::CHAR: return BuildArray(gpu_array<char>::linear(low, high, X, Y, Z));
            case ArrayTypes::UCHAR: return BuildArray(gpu_array<unsigned char>::linear(low, high, X, Y, Z));
            case ArrayTypes::INT: return BuildArray(gpu_array<int>::linear(low, high, X, Y, Z));
            case ArrayTypes::UINT: return BuildArray(gpu_array<unsigned int>::linear(low, high, X, Y, Z));
            case ArrayTypes::LONG: return BuildArray(gpu_array<long>::linear(low, high, X, Y, Z));
            case ArrayTypes::ULONG: return BuildArray(gpu_array<unsigned long>::linear(low, high, X, Y, Z));
            case ArrayTypes::FLOAT: return BuildArray(gpu_array<float>::linear(low, high, X, Y, Z));
            case ArrayTypes::DOUBLE: return BuildArray(gpu_array<double>::linear(low, high, X, Y, Z));
            }
        };
        // Returns a matrix with all values equal to the provided value
        Array Array::constant(ArrayTypes T, Number value, unsigned int X, unsigned int Y, unsigned int Z) {
            switch (T) {
            default: throw std::runtime_error("Cannot cast to empty type");
            case ArrayTypes::CHAR: return BuildArray(gpu_array<char>::constant(value, X, Y, Z));
            case ArrayTypes::UCHAR: return BuildArray(gpu_array<unsigned char>::constant(value, X, Y, Z));
            case ArrayTypes::INT: return BuildArray(gpu_array<int>::constant(value, X, Y, Z));
            case ArrayTypes::UINT: return BuildArray(gpu_array<unsigned int>::constant(value, X, Y, Z));
            case ArrayTypes::LONG: return BuildArray(gpu_array<long>::constant(value, X, Y, Z));
            case ArrayTypes::ULONG: return BuildArray(gpu_array<unsigned long>::constant(value, X, Y, Z));
            case ArrayTypes::FLOAT: return BuildArray(gpu_array<float>::constant(value, X, Y, Z));
            case ArrayTypes::DOUBLE: return BuildArray(gpu_array<double>::constant(value, X, Y, Z));
            }
        };
        // Returns a matrix with all values equal to the provided value
        Array Array::guassian_kernel(unsigned int X, unsigned int Y) {
            return BuildArray(gpu_array<float>::guassian_kernel(X, Y));
        };

        Array Array::from_vector(ArrayTypes T, const std::vector<Number>& parameters, unsigned int Y, unsigned int Z) {
            unsigned int count = 0;
            for (auto& x : parameters) {
                ++count;
            }
            Array out(T, (count / Y) / Z, Y, Z);
            count = 0;
            if (auto W = out.write()) {
                for (auto& x : parameters) {
                    W.store(x, count++);
                }
            }
            return out;
        };
        // joins two matrices along one of the dimensions.
        Array Array::join(unsigned int jdim, Array const& first) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.join(jdim, get_array<typename typename std::decay_t<decltype(arr)>::type>(first._data)));
                });
        };
        // transpose a 2-D matrix along its diagonal. Does not support transposition of 3-D matrices. 
        Array Array::transpose() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.transpose());
                });
        };
        // pad a matrix with zeros to make its X and Y components square. Used for calculating the inverse. 
        Array Array::make_square() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.make_square());
                });
        }
        // extracts the diagonal of a 2-D matrix as a 1-D array
        Array Array::diagonal() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.diagonal());
                });
        };
        // extract a row from this 2-D matrix as a 1-D array
        Array Array::row(unsigned int rowN) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.row(rowN));
                });
        };
        // grow a matrix by wrapping the new values around to the start. Only works for a 1-D vector. 
        Array Array::grow_by_wrapping(unsigned int new_length) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.grow_by_wrapping(new_length));
                });
        };
        // create a new array by sampling this array at the provided indices. E.g. This = [5,4,3,2,1,0]
        // Indices = [5,5,5,5,5,5,5,4,4,4,4,4,4,3,3,3,3,3,2,2,2,2,1,1,1,0,0]
        // Result = [0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,3,3,3,3,4,4,4,5,5]
        Array Array::resample(Array const& sample_indices) const {
            return visit_array(_type, _data, [&](auto& arr) {
                auto ARR = sample_indices.cast(ArrayTypes::UINT);
                return BuildArray(arr.resample(get_array<unsigned int>(ARR._data)));
                });
        };
        // calculate the determinant for a square matrix. Performed on the CPU, and minimizes exchanges with the GPU. 
        float Array::determinant() const {
            if (this->_type == ArrayTypes::FLOAT || this->_type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    if constexpr (std::is_same_v<typename typename std::decay_t<decltype(arr)>::type, float>) return arr.determinant();
                    else return 0.0f;
                    });
            }
            else {
                return this->cast(ArrayTypes::FLOAT).determinant();
            }
        }
        // cofactor of a square matrix, essential for calculating the inverse
        Array Array::cofactor() const {
            if (this->_type == ArrayTypes::FLOAT || this->_type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    if constexpr (std::is_same_v<typename typename std::decay_t<decltype(arr)>::type, float>) return BuildArray(arr.cofactor());
                    else return Array();
                    });
            }
            else {
                return this->cast(ArrayTypes::FLOAT).cofactor();
            }
        };
        // transpose of the cofactor of a square matrix
        Array Array::adjoint() const {
            if (this->_type == ArrayTypes::FLOAT || this->_type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    if constexpr (std::is_same_v<typename typename std::decay_t<decltype(arr)>::type, float>) return BuildArray(arr.adjoint());
                    else return Array();
                    });
            }
            else {
                return this->cast(ArrayTypes::FLOAT).adjoint();
            }
        };
        // solve for the inverse of the matrix. Does not support solving for the inverse of a 3-D matrix. 
        Array Array::inverse() const {
            if (this->_type == ArrayTypes::FLOAT || this->_type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    if constexpr (std::is_same_v<typename typename std::decay_t<decltype(arr)>::type, float>) return BuildArray(arr.inverse());
                    else return Array();
                    });
            }
            else {
                return this->cast(ArrayTypes::FLOAT).inverse();
            }
        };
        // performs a cross-multiplication of two rectangular matrices. This is not accelerated by the GPU, and is CPU-bound. Uses CPU multithreading to (attempt) to speed-up this bottleneck. 
        // the number of columns in this matrix must equal the number of rows in the RHS matrix. 
        Array Array::matrix_multiply(Array const& rhs) const {
            if (this->_type == ArrayTypes::FLOAT || this->_type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    if constexpr (std::is_same_v<typename typename std::decay_t<decltype(arr)>::type, float>) return BuildArray(arr.matrix_multiply(get_array<typename typename std::decay_t<decltype(arr)>::type>(rhs._data)));
                    else return Array();
                    });
            }
            else {
                return this->cast(ArrayTypes::FLOAT).matrix_multiply(rhs);
            }
        };
        // test to see if there is any colinearity in the feature set. If so, it is impossible to solve for the linear regression. One or multiple features must be removed until it is no longer invalid.
        bool Array::is_colinear() const {
            if (this->_type == ArrayTypes::FLOAT || this->_type == ArrayTypes::DOUBLE) {
                return visit_array(_type, _data, [&](auto& arr) {
                    if constexpr (std::is_same_v<typename typename std::decay_t<decltype(arr)>::type, float>) return arr.is_colinear();
                    else return true;
                    });
            }
            else {
                return this->cast(ArrayTypes::FLOAT).is_colinear();
            }
        };
        Number Array::sum() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return Number(arr.sum());
                });
        };
        Number Array::avg() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return Number(arr.avg());
                });
        };
        Number Array::max() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return Number(arr.max());
                });
        };
        Number Array::min() const {
            return visit_array(_type, _data, [&](auto& arr) {
                return Number(arr.min());
                });
        };
        Array Array::convolve(Array const& kernel) const {
            return visit_array(_type, _data, [&](auto& arr) {
                return BuildArray(arr.convolve(get_array<typename typename std::decay_t<decltype(arr)>::type>(kernel._data)));
                });
        };
        // solve for the weights to be used when performing linearized predictions, as determined by a basic linear regression.
        Array linear_regression::solve_for_weights(Array const& measurements, Array const& features) {
            return (features.transpose().matrix_multiply(features)).inverse().matrix_multiply(features.transpose()).matrix_multiply(measurements);
        };
        // solve for the linearized prediction.
        Array linear_regression::predict(Array const& features, Array const& weights) {
            return features.matrix_multiply(weights);
        };
        // returns the standard error of the linear regression.
        Array linear_regression::standard_error(Array const& measurements, Array const& features, Array const& weights) {
            auto prediction = predict(features, weights);
            return ((((double)(measurements - prediction).pow(2.0).sum() / std::max<double>(1.0, static_cast<double>(features.size(0)) - 2.0)) * (features.transpose().matrix_multiply(features)).inverse()).pow(0.5)).diagonal();
        };
        // returns the population standard deviation.
        Array linear_regression::standard_deviation(
            Array const& measurements,
            Array const& features,
            Array const& weights
        ) {
            return standard_error(measurements, features, weights) * std::sqrt((double)measurements.size(0));
        };
        // evaluate for the students-t test
        Array linear_regression::t_statistic(Array const& weights, Array const& std_err) {
            return weights / std_err;
        };
        // evaluate for the p-value
        Array linear_regression::p_value(Array const& features, Array const& t_stat) {
            boost::math::students_t dist(features.size(0) - features.size(1)); // n - k - 1, but should include the intercept in the features list already
            Array out(ArrayTypes::FLOAT, t_stat.size(0), 1, 1);
            unsigned int N = out.size();
            if (auto R = t_stat.read()) {
                if (auto W = out.write()) {
                    for (unsigned int i = 0; i < N; ++i) {
                        W.store((1.0f - (float)boost::math::cdf(dist, R[i])) + boost::math::cdf(dist, -(float)R[i]), i);
                        if (((float)W.load(i) > 1.0f) || ((float)W.load(i) < 0.0f))
                            W.store((1.0f - (float)boost::math::cdf(dist, -(float)R[i])) + boost::math::cdf(dist, (float)R[i]), i);
                    }
                }
            }
            return out.copy();
        };

        // build a collection of features for a linear regression while avoiding colinearity. 
        Array linear_regression::build_features(Array const& current_best) {
            return current_best.copy();
        };
        // build a collection of features for a linear regression while avoiding colinearity. 
        Array linear_regression::build_features(Array&& current_best) {
            return std::move(current_best);
        };
    };
};

void clear() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}
__forceinline void console_clear() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}

void fnGpuProgramming() {
    while (0) {
       GL::dynamic_allocator<int, 256> alloc;
       for (int i = 2; i <= 256; i *= 2) {
           int* p = alloc.Alloc(i);
           for (int j = 0; j < i; ++j) {
               p[j] = i;
           }
           alloc.Free(p);
       }
       for (int i = 0; i <= 1000000; i += 10000) {
           auto p = alloc.make_unique(i);
           for (int j = 0; j < i; ++j) p[j] = i;
       }
       for (int i = 1000000; i >= 0; i -= 10000) {
           auto p = alloc.make_unique(i);
           for (int j = 0; j < i; ++j) p[j] = i;
       }
       for (int i = 0; i < 10000; ++i) {
           int N = GL::util::rand_fast() * 1000000;
           auto p = alloc.make_unique(N);
           for (int j = 0; j < N; ++j) p[j] = N;
       }
    }

    while (1) {
        //Conway's Game of Life.
        if (1) {
            using namespace GL;
            using namespace GL::GPU;
            const int game_w = 50, game_h = 50;

            // Initialize the kernel array just once
            auto kernel = Array::from_vector(ArrayTypes::UINT, std::vector<Number>{
                1, 1, 1, 1, 0, 1, 1, 1, 1
            }, 3);

            auto state = (Array::random(ArrayTypes::FLOAT, game_h, game_w, 1) > 0.4f).cast(ArrayTypes::UINT);

            float prev_avg2 = 0;
            float prev_avg = 0;
            for (;;) {
                GL::stopwatch sw;

                // Convolve aligns the kernel ontop of each pixel, multiplies the neighboring pixels by the kernel, and sums the results. The edges are correctly handled using weighted-balancing on the kernel itself.
                auto nHood = state.convolve(kernel);

                // Generate conditions for life
                // state == 1 && nHood < 2 ->> state = 0
                // state == 1 && nHood > 3 ->> state = 0
                // else if state == 1 ->> state = 1
                // state == 0 && nHood == 3 ->> state = 1
                auto C0 = (nHood == 2);
                auto C1 = (nHood == 3);

                auto a0 = (state == 1) && (nHood < 2);  // Die of under population
                auto a1 = (state > 0) && (C0 || C1);   // Continue to live
                auto a2 = (state <= 0) && C1;           // Reproduction
                auto a3 = (state == 1) && (nHood > 3);  // Over-population

                // display = (a0 + a1).join(2, a1 + a2).join(2, a3).cast(ArrayTypes::FLOAT);
                auto R = a0 * a1;
                auto G = a1 * a2;
                auto B = a3;

                // Update state
                state *= C0.cast(ArrayTypes::UINT);
                state += C1.cast(ArrayTypes::UINT);

                while (sw.stop() < 1.0 / 30.0) {
                    std::this_thread::yield();
                }

                console_clear();

                print((
                    a0.cast(ArrayTypes::CHAR) * '-'
                    +
                    state.cast(ArrayTypes::CHAR) * 'O'
                    +
                    a2.cast(ArrayTypes::CHAR) * '+'
                    +
                    a3.cast(ArrayTypes::CHAR) * '-'
                ).to_string({}, true));
                print("");
                print(std::to_string(1.0 / sw.stop()) + " fps");

                // add random chance for life to spawn nearby the existing life. 
                state += ((nHood > 0).cast(ArrayTypes::FLOAT) * (Array::random(ArrayTypes::FLOAT, game_h, game_w, 1) >= 0.995f).cast(ArrayTypes::FLOAT)).cast(ArrayTypes::UINT);
                state = state.min(1);
            }

        }

        // Advertisement regression. Generally correct analysis.
        if (0) {
            /*          Coefficients    Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%
            Intercept	4.625124079	    0.307501165	    15.04099695	    1.68268E-34	    4.018688356	    5.231559801
            TV	        0.05444578	    0.001375188	    39.59152448	    1.89294E-95	    0.051733716	    0.057157845
            Radio	    0.107001228	    0.008489563	    12.60385655	    4.6021E-27	    0.090258612	    0.123743844
            Newspaper	0.000335658	    0.005788056	    0.057991479	    0.953814495	    -0.011079206	0.011750522
            */
            auto TV_Ads = GL::GPU::gpu_array<float>::from_vector(std::vector<float>{
                230.1, 44.5, 17.2, 151.5, 180.8, 8.7, 57.5, 120.2, 8.6, 199.8, 66.1, 214.7, 23.8, 97.5, 204.1, 195.4, 67.8, 281.4, 69.2, 147.3, 218.4, 237.4, 13.2, 228.3, 62.3, 262.9, 142.9, 240.1, 248.8, 70.6, 292.9, 112.9, 97.2, 265.6, 95.7, 290.7, 266.9, 74.7, 43.1, 228.0, 202.5, 177.0, 293.6, 206.9, 25.1, 175.1, 89.7, 239.9, 227.2, 66.9, 199.8, 100.4, 216.4, 182.6, 262.7, 198.9, 7.3, 136.2, 210.8, 210.7, 53.5, 261.3, 239.3, 102.7, 131.1, 69.0, 31.5, 139.3, 237.4, 216.8, 199.1, 109.8, 26.8, 129.4, 213.4, 16.9, 27.5, 120.5, 5.4, 116.0, 76.4, 239.8, 75.3, 68.4, 213.5, 193.2, 76.3, 110.7, 88.3, 109.8, 134.3, 28.6, 217.7, 250.9, 107.4, 163.3, 197.6, 184.9, 289.7, 135.2, 222.4, 296.4, 280.2, 187.9, 238.2, 137.9, 25.0, 90.4, 13.1, 255.4, 225.8, 241.7, 175.7, 209.6, 78.2, 75.1, 139.2, 76.4, 125.7, 19.4, 141.3, 18.8, 224.0, 123.1, 229.5, 87.2, 7.8, 80.2, 220.3, 59.6, .7, 265.2, 8.4, 219.8, 36.9, 48.3, 25.6, 273.7, 43.0, 184.9, 73.4, 193.7, 220.5, 104.6, 96.2, 140.3, 240.1, 243.2, 38.0, 44.7, 280.7, 121.0, 197.6, 171.3, 187.8, 4.1, 93.9, 149.8, 11.7, 131.7, 172.5, 85.7, 188.4, 163.5, 117.2, 234.5, 17.9, 206.8, 215.4, 284.3, 50.0, 164.5, 19.6, 168.4, 222.4, 276.9, 248.4, 170.2, 276.7, 165.6, 156.6, 218.5, 56.2, 287.6, 253.8, 205.0, 139.5, 191.1, 286.0, 18.7, 39.5, 75.5, 17.2, 166.8, 149.7, 38.2, 94.2, 177.0, 283.6, 232.1
            });
            auto Radio_Ads = GL::GPU::gpu_array<float>::from_vector(std::vector<float>{
                37.8, 39.3, 45.9, 41.3, 10.8, 48.9, 32.8, 19.6, 2.1, 2.6, 5.8, 24.0, 35.1, 7.6, 32.9, 47.7, 36.6, 39.6, 20.5, 23.9, 27.7, 5.1, 15.9, 16.9, 12.6, 3.5, 29.3, 16.7, 27.1, 16.0, 28.3, 17.4, 1.5, 20.0, 1.4, 4.1, 43.8, 49.4, 26.7, 37.7, 22.3, 33.4, 27.7, 8.4, 25.7, 22.5, 9.9, 41.5, 15.8, 11.7, 3.1, 9.6, 41.7, 46.2, 28.8, 49.4, 28.1, 19.2, 49.6, 29.5, 2.0, 42.7, 15.5, 29.6, 42.8, 9.3, 24.6, 14.5, 27.5, 43.9, 30.6, 14.3, 33.0, 5.7, 24.6, 43.7, 1.6, 28.5, 29.9, 7.7, 26.7, 4.1, 20.3, 44.5, 43.0, 18.4, 27.5, 40.6, 25.5, 47.8, 4.9, 1.5, 33.5, 36.5, 14.0, 31.6, 3.5, 21.0, 42.3, 41.7, 4.3, 36.3, 10.1, 17.2, 34.3, 46.4, 11.0, .3, .4, 26.9, 8.2, 38.0, 15.4, 20.6, 46.8, 35.0, 14.3, .8, 36.9, 16.0, 26.8, 21.7, 2.4, 34.6, 32.3, 11.8, 38.9, .0, 49.0, 12.0, 39.6, 2.9, 27.2, 33.5, 38.6, 47.0, 39.0, 28.9, 25.9, 43.9, 17.0, 35.4, 33.2, 5.7, 14.8, 1.9, 7.3, 49.0, 40.3, 25.8, 13.9, 8.4, 23.3, 39.7, 21.1, 11.6, 43.5, 1.3, 36.9, 18.4, 18.1, 35.8, 18.1, 36.8, 14.7, 3.4, 37.6, 5.2, 23.6, 10.6, 11.6, 20.9, 20.1, 7.1, 3.4, 48.9, 30.2, 7.8, 2.3, 10.0, 2.6, 5.4, 5.7, 43.0, 21.3, 45.1, 2.1, 28.7, 13.9, 12.1, 41.1, 10.8, 4.1, 42.0, 35.6, 3.7, 4.9, 9.3, 42.0, 8.6
            });
            auto Newspaper_Ads = GL::GPU::gpu_array<float>::from_vector(std::vector<float>{
                69.2, 45.1, 69.3, 58.5, 58.4, 75.0, 23.5, 11.6, 1.0, 21.2, 24.2, 4.0, 65.9, 7.2, 46.0, 52.9, 114.0, 55.8, 18.3, 19.1, 53.4, 23.5, 49.6, 26.2, 18.3, 19.5, 12.6, 22.9, 22.9, 40.8, 43.2, 38.6, 30.0, .3, 7.4, 8.5, 5.0, 45.7, 35.1, 32.0, 31.6, 38.7, 1.8, 26.4, 43.3, 31.5, 35.7, 18.5, 49.9, 36.8, 34.6, 3.6, 39.6, 58.7, 15.9, 60.0, 41.4, 16.6, 37.7, 9.3, 21.4, 54.7, 27.3, 8.4, 28.9, .9, 2.2, 10.2, 11.0, 27.2, 38.7, 31.7, 19.3, 31.3, 13.1, 89.4, 20.7, 14.2, 9.4, 23.1, 22.3, 36.9, 32.5, 35.6, 33.8, 65.7, 16.0, 63.2, 73.4, 51.4, 9.3, 33.0, 59.0, 72.3, 10.9, 52.9, 5.9, 22.0, 51.2, 45.9, 49.8, 100.9, 21.4, 17.9, 5.3, 59.0, 29.7, 23.2, 25.6, 5.5, 56.5, 23.2, 2.4, 10.7, 34.5, 52.7, 25.6, 14.8, 79.2, 22.3, 46.2, 50.4, 15.6, 12.4, 74.2, 25.9, 50.6, 9.2, 3.2, 43.1, 8.7, 43.0, 2.1, 45.1, 65.6, 8.5, 9.3, 59.7, 20.5, 1.7, 12.9, 75.6, 37.9, 34.4, 38.9, 9.0, 8.7, 44.3, 11.9, 20.6, 37.0, 48.7, 14.2, 37.7, 9.5, 5.7, 50.5, 24.3, 45.2, 34.6, 30.7, 49.3, 25.6, 7.4, 5.4, 84.8, 21.6, 19.4, 57.6, 6.4, 18.4, 47.4, 17.0, 12.8, 13.1, 41.8, 20.3, 35.2, 23.7, 17.6, 8.3, 27.4, 29.7, 71.8, 30.0, 19.6, 26.6, 18.2, 3.7, 23.4, 5.8, 6.0, 31.6, 3.6, 6.0, 13.8, 8.1, 6.4, 66.2, 8.7
            });
            auto Sales_Revenue = GL::GPU::gpu_array<float>::from_vector(std::vector<float>{
                22.1, 10.4, 12.0, 16.5, 17.9, 7.2, 11.8, 13.2, 4.8, 15.6, 12.6, 17.4, 9.2, 13.7, 19.0, 22.4, 12.5, 24.4, 11.3, 14.6, 18.0, 17.5, 5.6, 20.5, 9.7, 17.0, 15.0, 20.9, 18.9, 10.5, 21.4, 11.9, 13.2, 17.4, 11.9, 17.8, 25.4, 14.7, 10.1, 21.5, 16.6, 17.1, 20.7, 17.9, 8.5, 16.1, 10.6, 23.2, 19.8, 9.7, 16.4, 10.7, 22.6, 21.2, 20.2, 23.7, 5.5, 13.2, 23.8, 18.4, 8.1, 24.2, 20.7, 14.0, 16.0, 11.3, 11.0, 13.4, 18.9, 22.3, 18.3, 12.4, 8.8, 11.0, 17.0, 8.7, 6.9, 14.2, 5.3, 11.0, 11.8, 17.3, 11.3, 13.6, 21.7, 20.2, 12.0, 16.0, 12.9, 16.7, 14.0, 7.3, 19.4, 22.2, 11.5, 16.9, 16.7, 20.5, 25.4, 17.2, 16.7, 23.8, 19.8, 19.7, 20.7, 15.0, 7.2, 12.0, 5.3, 19.8, 18.4, 21.8, 17.1, 20.9, 14.6, 12.6, 12.2, 9.4, 15.9, 6.6, 15.5, 7.0, 16.6, 15.2, 19.7, 10.6, 6.6, 11.9, 24.7, 9.7, 1.6, 17.7, 5.7, 19.6, 10.8, 11.6, 9.5, 20.8, 9.6, 20.7, 10.9, 19.2, 20.1, 10.4, 12.3, 10.3, 18.2, 25.4, 10.9, 10.1, 16.1, 11.6, 16.6, 16.0, 20.6, 3.2, 15.3, 10.1, 7.3, 12.9, 16.4, 13.3, 19.9, 18.0, 11.9, 16.9, 8.0, 17.2, 17.1, 20.0, 8.4, 17.5, 7.6, 16.7, 16.5, 27.0, 20.2, 16.7, 16.8, 17.6, 15.5, 17.2, 8.7, 26.2, 17.6, 22.6, 10.3, 17.3, 20.9, 6.7, 10.8, 11.9, 5.9, 19.6, 17.3, 7.6, 14.0, 14.8, 25.5, 18.4
            });

            TV_Ads = TV_Ads.grow_by_wrapping(1000000);
            Radio_Ads = Radio_Ads.grow_by_wrapping(1000000);
            Newspaper_Ads = Newspaper_Ads.grow_by_wrapping(1000000);
            Sales_Revenue = Sales_Revenue.grow_by_wrapping(1000000);

            auto Basic{
                GL::GPU::gpu_array<float>::constant(1, Sales_Revenue.size(0))
            };

            auto features = GL::GPU::linear_regressions::build_features( // does not double-check or remove colinearity
                Basic, TV_Ads, Radio_Ads, Newspaper_Ads
            ); 

            auto weights = GL::GPU::linear_regressions::solve_for_weights(Sales_Revenue, features);
            auto std_err = GL::GPU::linear_regressions::standard_error(Sales_Revenue, features, weights);
            auto std_dev = GL::GPU::linear_regressions::standard_deviation(Sales_Revenue, features, weights);
            auto t_stat = GL::GPU::linear_regressions::t_statistic(weights, std_err);
            auto p_value = GL::GPU::linear_regressions::p_value(features, t_stat);
            auto lower_95 = weights - (1.96 * std_err);
            auto upper_95 = weights + (1.96 * std_err);
            auto prediction = GL::GPU::linear_regressions::predict(features, weights);

            print("");
            print(
                weights.join(1,
                    std_err).join(1,
                        t_stat).join(1,
                            p_value).join(1,
                                lower_95).join(1,
                                    upper_95).to_string(
                                        { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
            );
            print("");

            print(Sales_Revenue.join(1, prediction).to_string({ "Measured", "Predicted" }));
            print("");

        }

    }

















    // using namespace GL;
    using namespace GL::GPU;

    if (1) {
        Array arr = Array::random_between(ArrayTypes::FLOAT, 0.0, 1.0, 10, 10, 1);
        // arr = 10;

        print("");
        print(arr);

        Array kernel = Array::guassian_kernel(3, 3);
        // kernel = 1.0f / (float)kernel.size(); 

        print("");
        print(kernel);

        print("");
        print(arr.convolve(kernel));
    }

    if (0) {
        static const int game_w = 70, game_h = 40;

        // Initialize the kernel array just once
        auto kernel = Array::from_vector(ArrayTypes::UINT, std::vector<Number>{
            1, 1, 1, 1, 0, 1, 1, 1, 1
        }, 3);
        auto state = (Array::random_between(ArrayTypes::FLOAT, 0.0f, 1.0f, game_h, game_w, 1) > 0.5f).cast(ArrayTypes::UINT);

        while (1) {
            GL::stopwatch sw;

            // add random chance for life to spawn
            state = state.max((Array::random_between(ArrayTypes::FLOAT, 0.0f, 1.0f, game_h, game_w, 1) > 0.99f).cast(ArrayTypes::UINT));

            // Convolve gets neighbors
            auto nHood = state.convolve(kernel);

            

            // Generate conditions for life
            // state == 1 && nHood < 2 ->> state = 0
            // state == 1 && nHood > 3 ->> state = 0
            // else if state == 1 ->> state = 1
            // state == 0 && nHood == 3 ->> state = 1
            auto C0 = (nHood == 2);
            auto C1 = (nHood == 3);

            auto a0 = (state == 1) && (nHood < 2);  // Die of under population
            auto a1 = (state > 0) && (C0 || C1);   // Continue to live
            auto a2 = (state <= 0) && C1;           // Reproduction
            auto a3 = (state == 1) && (nHood > 3);  // Over-population

            // display = (a0 + a1).join(2, a1 + a2).join(2, a3).cast(ArrayTypes::FLOAT);

            // Update state
            state = state * C0.cast(ArrayTypes::UINT) + C1.cast(ArrayTypes::UINT);
            
            while (sw.stop() < 1.0 / 30.0) {
                std::this_thread::yield(); 
            }

            clear();
            print(state.to_string({}, true));
            print("");
            print(std::to_string(1.0 / sw.stop()) + " fps");
        }

    }

    if (1) {
        Array arr(ArrayTypes::FLOAT, 100, 2, 1);
        print(arr.size());

        arr = 5;
        arr.write().store(100, 0);

        print(arr);

        arr += 5.0f;
        arr += arr;

        arr = arr * arr;
        arr = 5.0f;
        arr = arr * 5.0f;

        print(arr);
        print(arr.cast(ArrayTypes::INT));

        print(Array::random_between(ArrayTypes::FLOAT, 0.0f, 100.0f, 1000, 1, 1));

        // Advertisement regression. Generally correct analysis.
        if (1) {
            /*          Coefficients    Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%
            Intercept	4.625124079	    0.307501165	    15.04099695	    1.68268E-34	    4.018688356	    5.231559801
            TV	        0.05444578	    0.001375188	    39.59152448	    1.89294E-95	    0.051733716	    0.057157845
            Radio	    0.107001228	    0.008489563	    12.60385655	    4.6021E-27	    0.090258612	    0.123743844
            Newspaper	0.000335658	    0.005788056	    0.057991479	    0.953814495	    -0.011079206	0.011750522
            */
            auto TV_Ads = Array::from_vector(ArrayTypes::FLOAT, std::vector<Number>{
                230.1, 44.5, 17.2, 151.5, 180.8, 8.7, 57.5, 120.2, 8.6, 199.8, 66.1, 214.7, 23.8, 97.5, 204.1, 195.4, 67.8, 281.4, 69.2, 147.3, 218.4, 237.4, 13.2, 228.3, 62.3, 262.9, 142.9, 240.1, 248.8, 70.6, 292.9, 112.9, 97.2, 265.6, 95.7, 290.7, 266.9, 74.7, 43.1, 228.0, 202.5, 177.0, 293.6, 206.9, 25.1, 175.1, 89.7, 239.9, 227.2, 66.9, 199.8, 100.4, 216.4, 182.6, 262.7, 198.9, 7.3, 136.2, 210.8, 210.7, 53.5, 261.3, 239.3, 102.7, 131.1, 69.0, 31.5, 139.3, 237.4, 216.8, 199.1, 109.8, 26.8, 129.4, 213.4, 16.9, 27.5, 120.5, 5.4, 116.0, 76.4, 239.8, 75.3, 68.4, 213.5, 193.2, 76.3, 110.7, 88.3, 109.8, 134.3, 28.6, 217.7, 250.9, 107.4, 163.3, 197.6, 184.9, 289.7, 135.2, 222.4, 296.4, 280.2, 187.9, 238.2, 137.9, 25.0, 90.4, 13.1, 255.4, 225.8, 241.7, 175.7, 209.6, 78.2, 75.1, 139.2, 76.4, 125.7, 19.4, 141.3, 18.8, 224.0, 123.1, 229.5, 87.2, 7.8, 80.2, 220.3, 59.6, .7, 265.2, 8.4, 219.8, 36.9, 48.3, 25.6, 273.7, 43.0, 184.9, 73.4, 193.7, 220.5, 104.6, 96.2, 140.3, 240.1, 243.2, 38.0, 44.7, 280.7, 121.0, 197.6, 171.3, 187.8, 4.1, 93.9, 149.8, 11.7, 131.7, 172.5, 85.7, 188.4, 163.5, 117.2, 234.5, 17.9, 206.8, 215.4, 284.3, 50.0, 164.5, 19.6, 168.4, 222.4, 276.9, 248.4, 170.2, 276.7, 165.6, 156.6, 218.5, 56.2, 287.6, 253.8, 205.0, 139.5, 191.1, 286.0, 18.7, 39.5, 75.5, 17.2, 166.8, 149.7, 38.2, 94.2, 177.0, 283.6, 232.1
            });
            auto Radio_Ads = Array::from_vector(ArrayTypes::FLOAT, std::vector<Number>{
                37.8, 39.3, 45.9, 41.3, 10.8, 48.9, 32.8, 19.6, 2.1, 2.6, 5.8, 24.0, 35.1, 7.6, 32.9, 47.7, 36.6, 39.6, 20.5, 23.9, 27.7, 5.1, 15.9, 16.9, 12.6, 3.5, 29.3, 16.7, 27.1, 16.0, 28.3, 17.4, 1.5, 20.0, 1.4, 4.1, 43.8, 49.4, 26.7, 37.7, 22.3, 33.4, 27.7, 8.4, 25.7, 22.5, 9.9, 41.5, 15.8, 11.7, 3.1, 9.6, 41.7, 46.2, 28.8, 49.4, 28.1, 19.2, 49.6, 29.5, 2.0, 42.7, 15.5, 29.6, 42.8, 9.3, 24.6, 14.5, 27.5, 43.9, 30.6, 14.3, 33.0, 5.7, 24.6, 43.7, 1.6, 28.5, 29.9, 7.7, 26.7, 4.1, 20.3, 44.5, 43.0, 18.4, 27.5, 40.6, 25.5, 47.8, 4.9, 1.5, 33.5, 36.5, 14.0, 31.6, 3.5, 21.0, 42.3, 41.7, 4.3, 36.3, 10.1, 17.2, 34.3, 46.4, 11.0, .3, .4, 26.9, 8.2, 38.0, 15.4, 20.6, 46.8, 35.0, 14.3, .8, 36.9, 16.0, 26.8, 21.7, 2.4, 34.6, 32.3, 11.8, 38.9, .0, 49.0, 12.0, 39.6, 2.9, 27.2, 33.5, 38.6, 47.0, 39.0, 28.9, 25.9, 43.9, 17.0, 35.4, 33.2, 5.7, 14.8, 1.9, 7.3, 49.0, 40.3, 25.8, 13.9, 8.4, 23.3, 39.7, 21.1, 11.6, 43.5, 1.3, 36.9, 18.4, 18.1, 35.8, 18.1, 36.8, 14.7, 3.4, 37.6, 5.2, 23.6, 10.6, 11.6, 20.9, 20.1, 7.1, 3.4, 48.9, 30.2, 7.8, 2.3, 10.0, 2.6, 5.4, 5.7, 43.0, 21.3, 45.1, 2.1, 28.7, 13.9, 12.1, 41.1, 10.8, 4.1, 42.0, 35.6, 3.7, 4.9, 9.3, 42.0, 8.6
            });
            auto Newspaper_Ads = Array::from_vector(ArrayTypes::FLOAT, std::vector<Number>{
                69.2, 45.1, 69.3, 58.5, 58.4, 75.0, 23.5, 11.6, 1.0, 21.2, 24.2, 4.0, 65.9, 7.2, 46.0, 52.9, 114.0, 55.8, 18.3, 19.1, 53.4, 23.5, 49.6, 26.2, 18.3, 19.5, 12.6, 22.9, 22.9, 40.8, 43.2, 38.6, 30.0, .3, 7.4, 8.5, 5.0, 45.7, 35.1, 32.0, 31.6, 38.7, 1.8, 26.4, 43.3, 31.5, 35.7, 18.5, 49.9, 36.8, 34.6, 3.6, 39.6, 58.7, 15.9, 60.0, 41.4, 16.6, 37.7, 9.3, 21.4, 54.7, 27.3, 8.4, 28.9, .9, 2.2, 10.2, 11.0, 27.2, 38.7, 31.7, 19.3, 31.3, 13.1, 89.4, 20.7, 14.2, 9.4, 23.1, 22.3, 36.9, 32.5, 35.6, 33.8, 65.7, 16.0, 63.2, 73.4, 51.4, 9.3, 33.0, 59.0, 72.3, 10.9, 52.9, 5.9, 22.0, 51.2, 45.9, 49.8, 100.9, 21.4, 17.9, 5.3, 59.0, 29.7, 23.2, 25.6, 5.5, 56.5, 23.2, 2.4, 10.7, 34.5, 52.7, 25.6, 14.8, 79.2, 22.3, 46.2, 50.4, 15.6, 12.4, 74.2, 25.9, 50.6, 9.2, 3.2, 43.1, 8.7, 43.0, 2.1, 45.1, 65.6, 8.5, 9.3, 59.7, 20.5, 1.7, 12.9, 75.6, 37.9, 34.4, 38.9, 9.0, 8.7, 44.3, 11.9, 20.6, 37.0, 48.7, 14.2, 37.7, 9.5, 5.7, 50.5, 24.3, 45.2, 34.6, 30.7, 49.3, 25.6, 7.4, 5.4, 84.8, 21.6, 19.4, 57.6, 6.4, 18.4, 47.4, 17.0, 12.8, 13.1, 41.8, 20.3, 35.2, 23.7, 17.6, 8.3, 27.4, 29.7, 71.8, 30.0, 19.6, 26.6, 18.2, 3.7, 23.4, 5.8, 6.0, 31.6, 3.6, 6.0, 13.8, 8.1, 6.4, 66.2, 8.7
            });
            auto Sales_Revenue = Array::from_vector(ArrayTypes::FLOAT, std::vector<Number>{
                22.1, 10.4, 12.0, 16.5, 17.9, 7.2, 11.8, 13.2, 4.8, 15.6, 12.6, 17.4, 9.2, 13.7, 19.0, 22.4, 12.5, 24.4, 11.3, 14.6, 18.0, 17.5, 5.6, 20.5, 9.7, 17.0, 15.0, 20.9, 18.9, 10.5, 21.4, 11.9, 13.2, 17.4, 11.9, 17.8, 25.4, 14.7, 10.1, 21.5, 16.6, 17.1, 20.7, 17.9, 8.5, 16.1, 10.6, 23.2, 19.8, 9.7, 16.4, 10.7, 22.6, 21.2, 20.2, 23.7, 5.5, 13.2, 23.8, 18.4, 8.1, 24.2, 20.7, 14.0, 16.0, 11.3, 11.0, 13.4, 18.9, 22.3, 18.3, 12.4, 8.8, 11.0, 17.0, 8.7, 6.9, 14.2, 5.3, 11.0, 11.8, 17.3, 11.3, 13.6, 21.7, 20.2, 12.0, 16.0, 12.9, 16.7, 14.0, 7.3, 19.4, 22.2, 11.5, 16.9, 16.7, 20.5, 25.4, 17.2, 16.7, 23.8, 19.8, 19.7, 20.7, 15.0, 7.2, 12.0, 5.3, 19.8, 18.4, 21.8, 17.1, 20.9, 14.6, 12.6, 12.2, 9.4, 15.9, 6.6, 15.5, 7.0, 16.6, 15.2, 19.7, 10.6, 6.6, 11.9, 24.7, 9.7, 1.6, 17.7, 5.7, 19.6, 10.8, 11.6, 9.5, 20.8, 9.6, 20.7, 10.9, 19.2, 20.1, 10.4, 12.3, 10.3, 18.2, 25.4, 10.9, 10.1, 16.1, 11.6, 16.6, 16.0, 20.6, 3.2, 15.3, 10.1, 7.3, 12.9, 16.4, 13.3, 19.9, 18.0, 11.9, 16.9, 8.0, 17.2, 17.1, 20.0, 8.4, 17.5, 7.6, 16.7, 16.5, 27.0, 20.2, 16.7, 16.8, 17.6, 15.5, 17.2, 8.7, 26.2, 17.6, 22.6, 10.3, 17.3, 20.9, 6.7, 10.8, 11.9, 5.9, 19.6, 17.3, 7.6, 14.0, 14.8, 25.5, 18.4
            });

            //TV_Ads = TV_Ads.grow_by_wrapping(1000000);
            //Radio_Ads = Radio_Ads.grow_by_wrapping(1000000);
            //Newspaper_Ads = Newspaper_Ads.grow_by_wrapping(1000000);
            //Sales_Revenue = Sales_Revenue.grow_by_wrapping(1000000);

            auto Basic{
                Array::constant(ArrayTypes::FLOAT, 1, Sales_Revenue.size(0))
            };
            auto features = linear_regression::build_features( // double-checks and removes colinearity
                Basic, TV_Ads, Radio_Ads, Newspaper_Ads
            );

            auto weights = linear_regression::solve_for_weights(Sales_Revenue, features);
            auto std_err = linear_regression::standard_error(Sales_Revenue, features, weights);
            auto std_dev = linear_regression::standard_deviation(Sales_Revenue, features, weights);
            auto t_stat = linear_regression::t_statistic(weights, std_err);
            auto p_value = linear_regression::p_value(features, t_stat);
            auto lower_95 = weights - (1.96 * std_err);
            auto upper_95 = weights + (1.96 * std_err);
            auto prediction = linear_regression::predict(features, weights);

            print("");
            print(
                weights.join(1,
                    std_err).join(1,
                        t_stat).join(1,
                            p_value).join(1,
                                lower_95).join(1,
                                    upper_95).to_string(
                                        { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
            );
            print("");

            print(Sales_Revenue.join(1, prediction).to_string({ "Measured", "Predicted" }));
            print("");
        }

        // Demonstrate the creation, use, and destruction of a floating-point matrix with 100M items as part of a CPU-bound matrix multiplication
        Array::constant(ArrayTypes::FLOAT, 1, 10000).matrix_multiply(Array::constant(ArrayTypes::FLOAT, 1, 10000).transpose()).read();
    }








    if (1) {
        auto arr = gpu_array<int>(100);
        arr = 5;
        arr += 5;
        arr *= 2;      
        arr = arr + arr;
        if (auto arr_reader = arr.read()) {
            EXPECT_EQ(arr_reader[0], 40)
        }

        if (1) {
            auto arr_f = arr.cast<float>();
            if (auto arr_reader = arr_f.read()) {
                EXPECT_EQ(arr_reader[0], 40)
            }

            auto arr_c = arr.cast<char>();
            if (auto arr_reader = arr_c.read()) {
                EXPECT_EQ(arr_reader[0], 40)
            }

            auto arr_u = arr.cast<unsigned int>();
            if (auto arr_reader = arr_u.read()) {
                EXPECT_EQ(arr_reader[0], 40)
            }
        }
    }
    if (1) {
        auto matrix = gpu_array<float>(100, 100, 100);
        auto pow_matrix = gpu_array<int>(100, 100, 100);
        matrix = 2;
        pow_matrix = 2;

        auto result = matrix.pown(pow_matrix);
        if (auto r = result.read()) {
            EXPECT_EQ(r[0], 4.0f);
        }

        auto result2 = pow_matrix.pown(pow_matrix);
        if (auto r = result2.read()) {
            EXPECT_EQ(r[0], 4);
        }

        if (auto r = result2.write()) {
            r[0] = 555;
            r[2] = 111;
        }
        if (auto r = result2.read()) {
            EXPECT_EQ(r[0], 555);
            EXPECT_EQ(r[1], 4);
            EXPECT_EQ(r[2], 111);
        }
        result2 /= 111;
        if (auto r = result2.read()) {
            EXPECT_EQ(r[0], 5);
            EXPECT_EQ(r[2], 1);
        }
    }
    if (1) {
        auto arr = gpu_array<float>::random(10000).max(gpu_array<float>::random(10000)).max(gpu_array<float>::random(10000)).max(gpu_array<float>::random(10000));
        print(arr);

        auto arr2 = gpu_array<float>::random(10);
        auto arr22 = gpu_array<float>::random(10);
        print(arr2);
        print(arr22);

        print(arr2.join(1, arr22));
        print(arr2.join(1, arr22).transpose());

        auto check = (arr > 0.5f);
        print(check);



        print("");

        auto square = gpu_array<float>::random(10, 2).make_square();
        print(square);
        print("");
        print(square.diagonal());
        print("");

        print(square.diagonal().avg());
        print("");
        print(square.determinant());
        print("");

        print(gpu_array<float>::random(10, 10).matrix_multiply(gpu_array<float>::random(10)));
        print("");
        print(gpu_array<float>::random(10, 10).matrix_multiply(gpu_array<float>::random(10)).avg());
        print("");

        print(gpu_array<float>::random(1000000).avg());
        print(gpu_array<float>::random(1000000).max());
        print(gpu_array<float>::random(1000000).min());
        print("");

        print(gpu_array<float>::linear(0, 1000, 1000));
        print("");


        auto features = linear_regressions::build_features(
            gpu_array<float>::random(1000000), gpu_array<float>::random(1000000), gpu_array<float>::random(1000000)
        );
        print(features);
    }

    // Demonstrate the creation, use, and destruction of a floating-point matrix with 100M items as part of a CPU-bound matrix multiplication
    gpu_array<float>::constant(1, 10000).matrix_multiply(gpu_array<float>::constant(1, 10000).transpose()).read();

    // Water Demand Modeling Example
    if (1) {
        gpu_array<float> DemandPatterns = gpu_array<float>::constant(1, 24); // series of demand patterns;
        for (int i = 0; i < 24; ++i) {
            DemandPatterns = DemandPatterns.join(1, ((gpu_array<float>::linear(0.0f, 3.14f, 24).sin() * 0.5f) + 0.5f) * gpu_array<float>::random_between(0.75, 1.25, 24));
        }

        const int num_timesteps = 24 * (60 / 5);
        gpu_array<float> junctions_base_multipliers = gpu_array<float>::random_between(0, 5, 40000);
        gpu_array<unsigned int> junction_pattern_indices = gpu_array<unsigned int>::random_between(1, 23, junctions_base_multipliers.size(0));
        gpu_array<float> junctions_X_flow = gpu_array<float>::constant(0, junctions_base_multipliers.size(0));

        print("");
        print(junction_pattern_indices);
        print("");

        gpu_array<unsigned int> pipe_open = gpu_array<unsigned int>::constant(1, 24000);
        gpu_array<float> pipe_flow_resistance = gpu_array<float>::random_between(80, 140, pipe_open.size(0));
        gpu_array<float> pipe_headloss_gradient = gpu_array<float>::constant(0, pipe_open.size(0));
        gpu_array<float> pipe_headloss = gpu_array<float>::constant(0, pipe_open.size(0));
        gpu_array<float> pipe_flow = gpu_array<float>::constant(0, pipe_open.size(0));
        gpu_array<float> P_coeff;
        gpu_array<float> Y_coeff;
        gpu_array<unsigned int> pipe_upstream_node_index = gpu_array<unsigned int>::random_between(0, junctions_base_multipliers.size(0) - 1, pipe_open.size(0));
        gpu_array<unsigned int> pipe_downstream_node_index = gpu_array<unsigned int>::random_between(0, junctions_base_multipliers.size(0) - 1, pipe_open.size(0));

        for (int TimeStep = 0; TimeStep < num_timesteps; ++TimeStep) {
            auto DemandPatterns_AtThisTime = DemandPatterns.row(TimeStep % DemandPatterns.size(0)); // sample a row from the demand patterns at this timestep
            auto junction_pattern_multipliers = DemandPatterns_AtThisTime.resample(junction_pattern_indices); // re-sample the row of demand pattern for each junction based on that junction's indices. 
            gpu_array<float> junction_demands_this_iteration = junction_pattern_multipliers * junctions_base_multipliers; // junction.demand * pattern[now] = current flowrate at each junction in the model

            // pipecoeff
            {
                pipe_headloss_gradient = gpu_array<float>::constant(1.852, pipe_headloss_gradient.size(0));
                pipe_headloss_gradient *= pipe_flow_resistance;
                pipe_headloss_gradient *= pipe_flow.pow(1.852 - 1.0);

                auto switch_condition =
                    (pipe_headloss_gradient < 1E-7).cast<float>();
                pipe_headloss_gradient =
                    (switch_condition * 1E-7) + ((1.0f - switch_condition) * pipe_headloss_gradient);
                pipe_headloss =
                    (switch_condition * pipe_flow * 1E-7) // if (pipe_headloss_gradient < 1E-7)
                    + ((1.0f - switch_condition) * pipe_headloss_gradient * pipe_flow / 1.852); // ... otherwise use original formula            
                //pipe_headloss *= pipe_flow.sign(); // Adjust head loss sign for flow direction

                //// P and Y coeffs.
                //P_coeff = 1.0 / pipe_headloss_gradient;
                //Y_coeff = pipe_headloss / pipe_headloss_gradient;
            }

            // linkcoeff
            {
                //auto do_nothing_check_1 = (P_coeff != 0).cast<float>();

                //// Update nodal flow excess (Xflow). (Flow out of node is (-), flow into node is (+))
                //junctions_X_flow -= pipe_flow.resample(pipe_upstream_node_index) * do_nothing_check_1;
                //junctions_X_flow += pipe_flow.resample(pipe_downstream_node_index) * do_nothing_check_1;

            }
        }






        // junctions_base_multipliers = junctions_base_multipliers.grow_by_wrapping(junctions_base_multipliers.size(0) * num_timesteps);
        // junctions_base_multipliers





        // print(DemandPatterns["0"].grow_by_wrapping(128));
    }
    // Advertisement regression. Generally correct analysis.
    if (1) {
        /*          Coefficients    Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%
        Intercept	4.625124079	    0.307501165	    15.04099695	    1.68268E-34	    4.018688356	    5.231559801
        TV	        0.05444578	    0.001375188	    39.59152448	    1.89294E-95	    0.051733716	    0.057157845
        Radio	    0.107001228	    0.008489563	    12.60385655	    4.6021E-27	    0.090258612	    0.123743844
        Newspaper	0.000335658	    0.005788056	    0.057991479	    0.953814495	    -0.011079206	0.011750522
        */
        auto TV_Ads = gpu_array<float>::from_vector(std::vector<double>{
            230.1, 44.5, 17.2, 151.5, 180.8, 8.7, 57.5, 120.2, 8.6, 199.8, 66.1, 214.7, 23.8, 97.5, 204.1, 195.4, 67.8, 281.4, 69.2, 147.3, 218.4, 237.4, 13.2, 228.3, 62.3, 262.9, 142.9, 240.1, 248.8, 70.6, 292.9, 112.9, 97.2, 265.6, 95.7, 290.7, 266.9, 74.7, 43.1, 228.0, 202.5, 177.0, 293.6, 206.9, 25.1, 175.1, 89.7, 239.9, 227.2, 66.9, 199.8, 100.4, 216.4, 182.6, 262.7, 198.9, 7.3, 136.2, 210.8, 210.7, 53.5, 261.3, 239.3, 102.7, 131.1, 69.0, 31.5, 139.3, 237.4, 216.8, 199.1, 109.8, 26.8, 129.4, 213.4, 16.9, 27.5, 120.5, 5.4, 116.0, 76.4, 239.8, 75.3, 68.4, 213.5, 193.2, 76.3, 110.7, 88.3, 109.8, 134.3, 28.6, 217.7, 250.9, 107.4, 163.3, 197.6, 184.9, 289.7, 135.2, 222.4, 296.4, 280.2, 187.9, 238.2, 137.9, 25.0, 90.4, 13.1, 255.4, 225.8, 241.7, 175.7, 209.6, 78.2, 75.1, 139.2, 76.4, 125.7, 19.4, 141.3, 18.8, 224.0, 123.1, 229.5, 87.2, 7.8, 80.2, 220.3, 59.6, .7, 265.2, 8.4, 219.8, 36.9, 48.3, 25.6, 273.7, 43.0, 184.9, 73.4, 193.7, 220.5, 104.6, 96.2, 140.3, 240.1, 243.2, 38.0, 44.7, 280.7, 121.0, 197.6, 171.3, 187.8, 4.1, 93.9, 149.8, 11.7, 131.7, 172.5, 85.7, 188.4, 163.5, 117.2, 234.5, 17.9, 206.8, 215.4, 284.3, 50.0, 164.5, 19.6, 168.4, 222.4, 276.9, 248.4, 170.2, 276.7, 165.6, 156.6, 218.5, 56.2, 287.6, 253.8, 205.0, 139.5, 191.1, 286.0, 18.7, 39.5, 75.5, 17.2, 166.8, 149.7, 38.2, 94.2, 177.0, 283.6, 232.1
        });
        auto Radio_Ads = gpu_array<float>::from_vector(std::vector<double>{
            37.8, 39.3, 45.9, 41.3, 10.8, 48.9, 32.8, 19.6, 2.1, 2.6, 5.8, 24.0, 35.1, 7.6, 32.9, 47.7, 36.6, 39.6, 20.5, 23.9, 27.7, 5.1, 15.9, 16.9, 12.6, 3.5, 29.3, 16.7, 27.1, 16.0, 28.3, 17.4, 1.5, 20.0, 1.4, 4.1, 43.8, 49.4, 26.7, 37.7, 22.3, 33.4, 27.7, 8.4, 25.7, 22.5, 9.9, 41.5, 15.8, 11.7, 3.1, 9.6, 41.7, 46.2, 28.8, 49.4, 28.1, 19.2, 49.6, 29.5, 2.0, 42.7, 15.5, 29.6, 42.8, 9.3, 24.6, 14.5, 27.5, 43.9, 30.6, 14.3, 33.0, 5.7, 24.6, 43.7, 1.6, 28.5, 29.9, 7.7, 26.7, 4.1, 20.3, 44.5, 43.0, 18.4, 27.5, 40.6, 25.5, 47.8, 4.9, 1.5, 33.5, 36.5, 14.0, 31.6, 3.5, 21.0, 42.3, 41.7, 4.3, 36.3, 10.1, 17.2, 34.3, 46.4, 11.0, .3, .4, 26.9, 8.2, 38.0, 15.4, 20.6, 46.8, 35.0, 14.3, .8, 36.9, 16.0, 26.8, 21.7, 2.4, 34.6, 32.3, 11.8, 38.9, .0, 49.0, 12.0, 39.6, 2.9, 27.2, 33.5, 38.6, 47.0, 39.0, 28.9, 25.9, 43.9, 17.0, 35.4, 33.2, 5.7, 14.8, 1.9, 7.3, 49.0, 40.3, 25.8, 13.9, 8.4, 23.3, 39.7, 21.1, 11.6, 43.5, 1.3, 36.9, 18.4, 18.1, 35.8, 18.1, 36.8, 14.7, 3.4, 37.6, 5.2, 23.6, 10.6, 11.6, 20.9, 20.1, 7.1, 3.4, 48.9, 30.2, 7.8, 2.3, 10.0, 2.6, 5.4, 5.7, 43.0, 21.3, 45.1, 2.1, 28.7, 13.9, 12.1, 41.1, 10.8, 4.1, 42.0, 35.6, 3.7, 4.9, 9.3, 42.0, 8.6
        });
        auto Newspaper_Ads = gpu_array<float>::from_vector(std::vector<double>{
            69.2, 45.1, 69.3, 58.5, 58.4, 75.0, 23.5, 11.6, 1.0, 21.2, 24.2, 4.0, 65.9, 7.2, 46.0, 52.9, 114.0, 55.8, 18.3, 19.1, 53.4, 23.5, 49.6, 26.2, 18.3, 19.5, 12.6, 22.9, 22.9, 40.8, 43.2, 38.6, 30.0, .3, 7.4, 8.5, 5.0, 45.7, 35.1, 32.0, 31.6, 38.7, 1.8, 26.4, 43.3, 31.5, 35.7, 18.5, 49.9, 36.8, 34.6, 3.6, 39.6, 58.7, 15.9, 60.0, 41.4, 16.6, 37.7, 9.3, 21.4, 54.7, 27.3, 8.4, 28.9, .9, 2.2, 10.2, 11.0, 27.2, 38.7, 31.7, 19.3, 31.3, 13.1, 89.4, 20.7, 14.2, 9.4, 23.1, 22.3, 36.9, 32.5, 35.6, 33.8, 65.7, 16.0, 63.2, 73.4, 51.4, 9.3, 33.0, 59.0, 72.3, 10.9, 52.9, 5.9, 22.0, 51.2, 45.9, 49.8, 100.9, 21.4, 17.9, 5.3, 59.0, 29.7, 23.2, 25.6, 5.5, 56.5, 23.2, 2.4, 10.7, 34.5, 52.7, 25.6, 14.8, 79.2, 22.3, 46.2, 50.4, 15.6, 12.4, 74.2, 25.9, 50.6, 9.2, 3.2, 43.1, 8.7, 43.0, 2.1, 45.1, 65.6, 8.5, 9.3, 59.7, 20.5, 1.7, 12.9, 75.6, 37.9, 34.4, 38.9, 9.0, 8.7, 44.3, 11.9, 20.6, 37.0, 48.7, 14.2, 37.7, 9.5, 5.7, 50.5, 24.3, 45.2, 34.6, 30.7, 49.3, 25.6, 7.4, 5.4, 84.8, 21.6, 19.4, 57.6, 6.4, 18.4, 47.4, 17.0, 12.8, 13.1, 41.8, 20.3, 35.2, 23.7, 17.6, 8.3, 27.4, 29.7, 71.8, 30.0, 19.6, 26.6, 18.2, 3.7, 23.4, 5.8, 6.0, 31.6, 3.6, 6.0, 13.8, 8.1, 6.4, 66.2, 8.7
        });
        auto Sales_Revenue = gpu_array<float>::from_vector(std::vector<double>{
            22.1, 10.4, 12.0, 16.5, 17.9, 7.2, 11.8, 13.2, 4.8, 15.6, 12.6, 17.4, 9.2, 13.7, 19.0, 22.4, 12.5, 24.4, 11.3, 14.6, 18.0, 17.5, 5.6, 20.5, 9.7, 17.0, 15.0, 20.9, 18.9, 10.5, 21.4, 11.9, 13.2, 17.4, 11.9, 17.8, 25.4, 14.7, 10.1, 21.5, 16.6, 17.1, 20.7, 17.9, 8.5, 16.1, 10.6, 23.2, 19.8, 9.7, 16.4, 10.7, 22.6, 21.2, 20.2, 23.7, 5.5, 13.2, 23.8, 18.4, 8.1, 24.2, 20.7, 14.0, 16.0, 11.3, 11.0, 13.4, 18.9, 22.3, 18.3, 12.4, 8.8, 11.0, 17.0, 8.7, 6.9, 14.2, 5.3, 11.0, 11.8, 17.3, 11.3, 13.6, 21.7, 20.2, 12.0, 16.0, 12.9, 16.7, 14.0, 7.3, 19.4, 22.2, 11.5, 16.9, 16.7, 20.5, 25.4, 17.2, 16.7, 23.8, 19.8, 19.7, 20.7, 15.0, 7.2, 12.0, 5.3, 19.8, 18.4, 21.8, 17.1, 20.9, 14.6, 12.6, 12.2, 9.4, 15.9, 6.6, 15.5, 7.0, 16.6, 15.2, 19.7, 10.6, 6.6, 11.9, 24.7, 9.7, 1.6, 17.7, 5.7, 19.6, 10.8, 11.6, 9.5, 20.8, 9.6, 20.7, 10.9, 19.2, 20.1, 10.4, 12.3, 10.3, 18.2, 25.4, 10.9, 10.1, 16.1, 11.6, 16.6, 16.0, 20.6, 3.2, 15.3, 10.1, 7.3, 12.9, 16.4, 13.3, 19.9, 18.0, 11.9, 16.9, 8.0, 17.2, 17.1, 20.0, 8.4, 17.5, 7.6, 16.7, 16.5, 27.0, 20.2, 16.7, 16.8, 17.6, 15.5, 17.2, 8.7, 26.2, 17.6, 22.6, 10.3, 17.3, 20.9, 6.7, 10.8, 11.9, 5.9, 19.6, 17.3, 7.6, 14.0, 14.8, 25.5, 18.4
        });

        //TV_Ads = TV_Ads.grow_by_wrapping(1000000);
        //Radio_Ads = Radio_Ads.grow_by_wrapping(1000000);
        //Newspaper_Ads = Newspaper_Ads.grow_by_wrapping(1000000);
        //Sales_Revenue = Sales_Revenue.grow_by_wrapping(1000000);

        auto Basic{
            gpu_array<float>::constant(1, Sales_Revenue.size(0))
        };
        auto features = linear_regressions::build_features( // double-checks and removes colinearity
            Basic, TV_Ads, Radio_Ads, Newspaper_Ads
        );

        auto weights = linear_regressions::solve_for_weights(Sales_Revenue, features);
        auto std_err = linear_regressions::standard_error(Sales_Revenue, features, weights);
        auto std_dev = linear_regressions::standard_deviation(Sales_Revenue, features, weights);
        auto t_stat = linear_regressions::t_statistic(weights, std_err);
        auto p_value = linear_regressions::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regressions::predict(features, weights);

        print("");
        print(
            weights.join(1,
                std_err).join(1,
                    t_stat).join(1,
                        p_value).join(1,
                            lower_95).join(1,
                                upper_95).to_string(
                                    { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");

        print(Sales_Revenue.join(1, prediction).to_string({ "Measured", "Predicted" }));
        print("");
    }
    // Custom weather regression. Generally correct analysis.
    if (1) {
        /*          Coefficients	Standard Error	t Stat	        P-value	    Lower 95%	    Upper 95%
        Intercept	93.67835922	    0.121802957	    769.0975786	    0	        93.43959502	    93.91712343
        dawn	    -14.4227875	    0.153113204	    -94.19688942	0	        -14.72292761	-14.12264739
        dusk	    -10.26652003	0.162912373	    -63.01866368	0	        -10.58586896	-9.947171106
        winter	    -10.2061204	    0.131022469	    -77.89595536	0	        -10.46295715	-9.949283644
        */

        auto measured = gpu_array<float>::random_between(2, 4, 24 * 365);
        auto random_noise = gpu_array<float>::random_between(2, 4, 24 * 365);
        auto hours = gpu_array<float>(24 * 365);
        auto months = gpu_array<float>(24 * 365);

        std::array<float, 24> hourly{
            59, 58, 59, 60, 61, 62,
            64, 69, 72, 76, 79, 81,
            82, 80, 78, 76, 75, 74,
            70, 68, 66, 64, 62, 60
        };
        std::array<float, 12> monthly{
            0, -2, 6, 10, 14, 20,
            19, 18, 16, 12, 8, 4
        };
        if (auto W = measured.write()) for (unsigned int day = 0; day < 365; ++day) for (unsigned int hr = 0; hr < 24; ++hr) W[(day * 24) + hr] += (monthly[day / 31] + hourly[hr]);
        if (auto W = hours.write()) for (unsigned int day = 0; day < 365; ++day) for (unsigned int hr = 0; hr < 24; ++hr) W[(day * 24) + hr] = hr;
        if (auto W = months.write()) for (unsigned int day = 0; day < 365; ++day) for (unsigned int hr = 0; hr < 24; ++hr) W[(day * 24) + hr] = day / 31;
        
        auto dawn = (hours < 6).cast<float>();
        auto dusk = (hours > 18).cast<float>();
        auto midday = (!((hours > 18) + (hours < 6))).cast<float>();
        auto winter = ((months >= 10) + (months <= 4)).cast<float>();
        auto summer = (!winter).cast<float>();
        auto Basic{ gpu_array<float>::constant(1, winter.size(0)) };
        auto features = linear_regressions::build_features( // double-checks and removes colinearity
            Basic, dawn, dusk, midday, winter, summer
        );

        auto weights = linear_regressions::solve_for_weights(measured, features);
        auto std_err = linear_regressions::standard_error(measured, features, weights);
        auto std_dev = linear_regressions::standard_deviation(measured, features, weights);
        auto t_stat = linear_regressions::t_statistic(weights, std_err);
        auto p_value = linear_regressions::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regressions::predict(features, weights);

        print("");
        print(
            weights.join(1,
                std_err).join(1,
                    t_stat).join(1,
                        p_value).join(1,
                            lower_95).join(1,
                                upper_95).to_string(
                                    { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");

        print(measured.join(1, prediction).to_string({ "Measured", "Predicted" }));
        print("");







    }
    // MPG regression. Not being performed correctly in any way, for reasons not yet understood.
    if (1) {
        /*              Coefficients	Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%
        Intercept	    -17.21843462	4.644294149	    -3.707438433	0.000240184	    -26.34986447	-8.087004775
        cylinders	    -0.493376319	0.323282315	    -1.526146951	0.127796468	    -1.129001385	0.142248747
        displacement	0.019895644	    0.007515079	    2.647429695	    0.008444649	    0.005119788	    0.034671499
        horsepower	    -0.016951144	0.013786891	    -1.229511695	0.219632823	    -0.044058392	0.010156103
        weight	        -0.006474043	0.000652048	    -9.928787106	7.87495E-21	    -0.007756074	-0.005192013
        acceleration	0.080575838	    0.098844957	    0.815173996	    0.415478018	    -0.113769257	0.274920933
        model year	    0.750772678	    0.050973122	    14.72879519	    3.05598E-39	    0.650551315	    0.850994041
        origin	        1.426140495	    0.278136092	    5.127491665	    4.66568E-07	    0.879280169	    1.973000822
        */

        auto mpg = gpu_array<float>::from_vector(std::vector<double>{
            18, 15, 18, 16, 17, 15, 14, 14, 14, 15, 15, 14, 15, 14, 24, 22, 18, 21, 27, 26, 25, 24, 25, 26, 21, 10, 10, 11, 9, 27, 28, 25, 19, 16, 17, 19, 18, 14, 14, 14, 14, 12, 13, 13, 18, 22, 19, 18, 23, 28, 30, 30, 31, 35, 27, 26, 24, 25, 23, 20, 21, 13, 14, 15, 14, 17, 11, 13, 12, 13, 19, 15, 13, 13, 14, 18, 22, 21, 26, 22, 28, 23, 28, 27, 13, 14, 13, 14, 15, 12, 13, 13, 14, 13, 12, 13, 18, 16, 18, 18, 23, 26, 11, 12, 13, 12, 18, 20, 21, 22, 18, 19, 21, 26, 15, 16, 29, 24, 20, 19, 15, 24, 20, 11, 20, 19, 15, 31, 26, 32, 25, 16, 16, 18, 16, 13, 14, 14, 14, 29, 26, 26, 31, 32, 28, 24, 26, 24, 26, 31, 19, 18, 15, 15, 16, 15, 16, 14, 17, 16, 15, 18, 21, 20, 13, 29, 23, 20, 23, 24, 25, 24, 18, 29, 19, 23, 23, 22, 25, 33, 28, 25, 25, 26, 27, 17.5, 16, 15.5, 14.5, 22, 22, 24, 22.5, 29, 24.5, 29, 33, 20, 18, 18.5, 17.5, 29.5, 32, 28, 26.5, 20, 13, 19, 19, 16.5, 16.5, 13, 13, 13, 31.5, 30, 36, 25.5, 33.5, 17.5, 17, 15.5, 15, 17.5, 20.5, 19, 18.5, 16, 15.5, 15.5, 16, 29, 24.5, 26, 25.5, 30.5, 33.5, 30, 30.5, 22, 21.5, 21.5, 43.1, 36.1, 32.8, 39.4, 36.1, 19.9, 19.4, 20.2, 19.2, 20.5, 20.2, 25.1, 20.5, 19.4, 20.6, 20.8, 18.6, 18.1, 19.2, 17.7, 18.1, 17.5, 30, 27.5, 27.2, 30.9, 21.1, 23.2, 23.8, 23.9, 20.3, 17, 21.6, 16.2, 31.5, 29.5, 21.5, 19.8, 22.3, 20.2, 20.6, 17, 17.6, 16.5, 18.2, 16.9, 15.5, 19.2, 18.5, 31.9, 34.1, 35.7, 27.4, 25.4, 23, 27.2, 23.9, 34.2, 34.5, 31.8, 37.3, 28.4, 28.8, 26.8, 33.5, 41.5, 38.1, 32.1, 37.2, 28, 26.4, 24.3, 19.1, 34.3, 29.8, 31.3, 37, 32.2, 46.6, 27.9, 40.8, 44.3, 43.4, 36.4, 30, 44.6, 33.8, 29.8, 32.7, 23.7, 35, 32.4, 27.2, 26.6, 25.8, 23.5, 30, 39.1, 39, 35.1, 32.3, 37, 37.7, 34.1, 34.7, 34.4, 29.9, 33, 33.7, 32.4, 32.9, 31.6, 28.1, 30.7, 25.4, 24.2, 22.4, 26.6, 20.2, 17.6, 28, 27, 34, 31, 29, 27, 24, 36, 37, 31, 38, 36, 36, 36, 34, 38, 32, 38, 25, 38, 26, 22, 32, 36, 27, 27, 44, 32, 28, 31
        });
        auto cylinders = gpu_array<float>::from_vector(std::vector<double>{
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 6, 6, 6, 4, 4, 4, 4, 4, 4, 6, 8, 8, 8, 8, 4, 4, 4, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 6, 4, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 8, 8, 8, 8, 6, 4, 4, 4, 3, 4, 6, 4, 8, 8, 4, 4, 4, 4, 8, 4, 6, 8, 6, 6, 6, 4, 4, 4, 4, 6, 6, 6, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 8, 8, 8, 8, 6, 6, 6, 6, 6, 8, 8, 4, 4, 6, 4, 4, 4, 4, 6, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 8, 8, 6, 6, 6, 6, 4, 4, 4, 4, 6, 6, 6, 6, 4, 4, 4, 4, 4, 8, 4, 6, 6, 8, 8, 8, 8, 4, 4, 4, 4, 4, 8, 8, 8, 8, 6, 6, 6, 6, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 6, 4, 3, 4, 4, 4, 4, 4, 8, 8, 8, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 8, 6, 8, 8, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 4, 6, 4, 4, 6, 6, 4, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4, 5, 8, 4, 8, 4, 4, 4, 4, 4, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 6, 3, 4, 4, 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6, 6, 8, 6, 6, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4
        });
        auto displacement = gpu_array<float>::from_vector(std::vector<double>{
            307, 350, 318, 304, 302, 429, 454, 440, 455, 390, 383, 340, 400, 455, 113, 198, 199, 200, 97, 97, 110, 107, 104, 121, 199, 360, 307, 318, 304, 97, 140, 113, 232, 225, 250, 250, 232, 350, 400, 351, 318, 383, 400, 400, 258, 140, 250, 250, 122, 116, 79, 88, 71, 72, 97, 91, 113, 97.5, 97, 140, 122, 350, 400, 318, 351, 304, 429, 350, 350, 400, 70, 304, 307, 302, 318, 121, 121, 120, 96, 122, 97, 120, 98, 97, 350, 304, 350, 302, 318, 429, 400, 351, 318, 440, 455, 360, 225, 250, 232, 250, 198, 97, 400, 400, 360, 350, 232, 97, 140, 108, 70, 122, 155, 98, 350, 400, 68, 116, 114, 121, 318, 121, 156, 350, 198, 232, 250, 79, 122, 71, 140, 250, 258, 225, 302, 350, 318, 302, 304, 98, 79, 97, 76, 83, 90, 90, 116, 120, 108, 79, 225, 250, 250, 250, 400, 350, 318, 351, 231, 250, 258, 225, 231, 262, 302, 97, 140, 232, 140, 134, 90, 119, 171, 90, 232, 115, 120, 121, 121, 91, 107, 116, 140, 98, 101, 305, 318, 304, 351, 225, 250, 200, 232, 85, 98, 90, 91, 225, 250, 250, 258, 97, 85, 97, 140, 130, 318, 120, 156, 168, 350, 350, 302, 318, 98, 111, 79, 122, 85, 305, 260, 318, 302, 250, 231, 225, 250, 400, 350, 400, 351, 97, 151, 97, 140, 98, 98, 97, 97, 146, 121, 80, 90, 98, 78, 85, 91, 260, 318, 302, 231, 200, 200, 140, 225, 232, 231, 200, 225, 258, 305, 231, 302, 318, 98, 134, 119, 105, 134, 156, 151, 119, 131, 163, 121, 163, 89, 98, 231, 200, 140, 232, 225, 305, 302, 351, 318, 350, 351, 267, 360, 89, 86, 98, 121, 183, 350, 141, 260, 105, 105, 85, 91, 151, 173, 173, 151, 98, 89, 98, 86, 151, 140, 151, 225, 97, 134, 120, 119, 108, 86, 156, 85, 90, 90, 121, 146, 91, 97, 89, 168, 70, 122, 107, 135, 151, 156, 173, 135, 79, 86, 81, 97, 85, 89, 91, 105, 98, 98, 105, 107, 108, 119, 120, 141, 145, 168, 146, 231, 350, 200, 225, 112, 112, 112, 112, 135, 151, 140, 105, 91, 91, 105, 98, 120, 107, 108, 91, 91, 91, 181, 262, 156, 232, 144, 135, 151, 140, 97, 135, 120, 119
        });
        auto horsepower = gpu_array<float>::from_vector(std::vector<double>{
            130, 165, 150, 150, 140, 198, 220, 215, 225, 190, 170, 160, 150, 225, 95, 95, 97, 85, 88, 46, 87, 90, 95, 113, 90, 215, 200, 210, 193, 88, 90, 95, 100, 105, 100, 88, 100, 165, 175, 153, 150, 180, 170, 175, 110, 72, 100, 88, 86, 90, 70, 76, 65, 69, 60, 70, 95, 80, 54, 90, 86, 165, 175, 150, 153, 150, 208, 155, 160, 190, 97, 150, 130, 140, 150, 112, 76, 87, 69, 86, 92, 97, 80, 88, 175, 150, 145, 137, 150, 198, 150, 158, 150, 215, 225, 175, 105, 100, 100, 88, 95, 46, 150, 167, 170, 180, 100, 88, 72, 94, 90, 85, 107, 90, 145, 230, 49, 75, 91, 112, 150, 110, 122, 180, 95, 100, 100, 67, 80, 65, 75, 100, 110, 105, 140, 150, 150, 140, 150, 83, 67, 78, 52, 61, 75, 75, 75, 97, 93, 67, 95, 105, 72, 72, 170, 145, 150, 148, 110, 105, 110, 95, 110, 110, 129, 75, 83, 100, 78, 96, 71, 97, 97, 70, 90, 95, 88, 98, 115, 53, 86, 81, 92, 79, 83, 140, 150, 120, 152, 100, 105, 81, 90, 52, 60, 70, 53, 100, 78, 110, 95, 71, 70, 75, 72, 102, 150, 88, 108, 120, 180, 145, 130, 150, 68, 80, 58, 96, 70, 145, 110, 145, 130, 110, 105, 100, 98, 180, 170, 190, 149, 78, 88, 75, 89, 63, 83, 67, 78, 97, 110, 110, 48, 66, 52, 70, 60, 110, 140, 139, 105, 95, 85, 88, 100, 90, 105, 85, 110, 120, 145, 165, 139, 140, 68, 95, 97, 75, 95, 105, 85, 97, 103, 125, 115, 133, 71, 68, 115, 85, 88, 90, 110, 130, 129, 138, 135, 155, 142, 125, 150, 71, 65, 80, 80, 77, 125, 71, 90, 70, 70, 65, 69, 90, 115, 115, 90, 76, 60, 70, 65, 90, 88, 90, 90, 78, 90, 75, 92, 75, 65, 105, 65, 48, 48, 67, 67, 67, 67, 62, 132, 100, 88, 72, 84, 84, 92, 110, 84, 58, 64, 60, 67, 65, 62, 68, 63, 65, 65, 74, 75, 75, 100, 74, 80, 76, 116, 120, 110, 105, 88, 85, 88, 88, 88, 85, 84, 90, 92, 74, 68, 68, 63, 70, 88, 75, 70, 67, 67, 67, 110, 85, 92, 112, 96, 84, 90, 86, 52, 84, 79, 82
        });
        auto weight = gpu_array<float>::from_vector(std::vector<double>{
            3504, 3693, 3436, 3433, 3449, 4341, 4354, 4312, 4425, 3850, 3563, 3609, 3761, 3086, 2372, 2833, 2774, 2587, 2130, 1835, 2672, 2430, 2375, 2234, 2648, 4615, 4376, 4382, 4732, 2130, 2264, 2228, 2634, 3439, 3329, 3302, 3288, 4209, 4464, 4154, 4096, 4955, 4746, 5140, 2962, 2408, 3282, 3139, 2220, 2123, 2074, 2065, 1773, 1613, 1834, 1955, 2278, 2126, 2254, 2408, 2226, 4274, 4385, 4135, 4129, 3672, 4633, 4502, 4456, 4422, 2330, 3892, 4098, 4294, 4077, 2933, 2511, 2979, 2189, 2395, 2288, 2506, 2164, 2100, 4100, 3672, 3988, 4042, 3777, 4952, 4464, 4363, 4237, 4735, 4951, 3821, 3121, 3278, 2945, 3021, 2904, 1950, 4997, 4906, 4654, 4499, 2789, 2279, 2401, 2379, 2124, 2310, 2472, 2265, 4082, 4278, 1867, 2158, 2582, 2868, 3399, 2660, 2807, 3664, 3102, 2901, 3336, 1950, 2451, 1836, 2542, 3781, 3632, 3613, 4141, 4699, 4457, 4638, 4257, 2219, 1963, 2300, 1649, 2003, 2125, 2108, 2246, 2489, 2391, 2000, 3264, 3459, 3432, 3158, 4668, 4440, 4498, 4657, 3907, 3897, 3730, 3785, 3039, 3221, 3169, 2171, 2639, 2914, 2592, 2702, 2223, 2545, 2984, 1937, 3211, 2694, 2957, 2945, 2671, 1795, 2464, 2220, 2572, 2255, 2202, 4215, 4190, 3962, 4215, 3233, 3353, 3012, 3085, 2035, 2164, 1937, 1795, 3651, 3574, 3645, 3193, 1825, 1990, 2155, 2565, 3150, 3940, 3270, 2930, 3820, 4380, 4055, 3870, 3755, 2045, 2155, 1825, 2300, 1945, 3880, 4060, 4140, 4295, 3520, 3425, 3630, 3525, 4220, 4165, 4325, 4335, 1940, 2740, 2265, 2755, 2051, 2075, 1985, 2190, 2815, 2600, 2720, 1985, 1800, 1985, 2070, 1800, 3365, 3735, 3570, 3535, 3155, 2965, 2720, 3430, 3210, 3380, 3070, 3620, 3410, 3425, 3445, 3205, 4080, 2155, 2560, 2300, 2230, 2515, 2745, 2855, 2405, 2830, 3140, 2795, 3410, 1990, 2135, 3245, 2990, 2890, 3265, 3360, 3840, 3725, 3955, 3830, 4360, 4054, 3605, 3940, 1925, 1975, 1915, 2670, 3530, 3900, 3190, 3420, 2200, 2150, 2020, 2130, 2670, 2595, 2700, 2556, 2144, 1968, 2120, 2019, 2678, 2870, 3003, 3381, 2188, 2711, 2542, 2434, 2265, 2110, 2800, 2110, 2085, 2335, 2950, 3250, 1850, 2145, 1845, 2910, 2420, 2500, 2290, 2490, 2635, 2620, 2725, 2385, 1755, 1875, 1760, 2065, 1975, 2050, 1985, 2215, 2045, 2380, 2190, 2210, 2350, 2615, 2635, 3230, 3160, 2900, 2930, 3415, 3725, 3060, 3465, 2605, 2640, 2395, 2575, 2525, 2735, 2865, 1980, 2025, 1970, 2125, 2125, 2160, 2205, 2245, 1965, 1965, 1995, 2945, 3015, 2585, 2835, 2665, 2370, 2950, 2790, 2130, 2295, 2625, 2720
        });
        auto acceleration = gpu_array<float>::from_vector(std::vector<double>{
            12, 11.5, 11, 12, 10.5, 10, 9, 8.5, 10, 8.5, 10, 8, 9.5, 10, 15, 15.5, 15.5, 16, 14.5, 20.5, 17.5, 14.5, 17.5, 12.5, 15, 14, 15, 13.5, 18.5, 14.5, 15.5, 14, 13, 15.5, 15.5, 15.5, 15.5, 12, 11.5, 13.5, 13, 11.5, 12, 12, 13.5, 19, 15, 14.5, 14, 14, 19.5, 14.5, 19, 18, 19, 20.5, 15.5, 17, 23.5, 19.5, 16.5, 12, 12, 13.5, 13, 11.5, 11, 13.5, 13.5, 12.5, 13.5, 12.5, 14, 16, 14, 14.5, 18, 19.5, 18, 16, 17, 14.5, 15, 16.5, 13, 11.5, 13, 14.5, 12.5, 11.5, 12, 13, 14.5, 11, 11, 11, 16.5, 18, 16, 16.5, 16, 21, 14, 12.5, 13, 12.5, 15, 19, 19.5, 16.5, 13.5, 18.5, 14, 15.5, 13, 9.5, 19.5, 15.5, 14, 15.5, 11, 14, 13.5, 11, 16.5, 16, 17, 19, 16.5, 21, 17, 17, 18, 16.5, 14, 14.5, 13.5, 16, 15.5, 16.5, 15.5, 14.5, 16.5, 19, 14.5, 15.5, 14, 15, 15.5, 16, 16, 16, 21, 19.5, 11.5, 14, 14.5, 13.5, 21, 18.5, 19, 19, 15, 13.5, 12, 16, 17, 16, 18.5, 13.5, 16.5, 17, 14.5, 14, 17, 15, 17, 14.5, 13.5, 17.5, 15.5, 16.9, 14.9, 17.7, 15.3, 13, 13, 13.9, 12.8, 15.4, 14.5, 17.6, 17.6, 22.2, 22.1, 14.2, 17.4, 17.7, 21, 16.2, 17.8, 12.2, 17, 16.4, 13.6, 15.7, 13.2, 21.9, 15.5, 16.7, 12.1, 12, 15, 14, 18.5, 14.8, 18.6, 15.5, 16.8, 12.5, 19, 13.7, 14.9, 16.4, 16.9, 17.7, 19, 11.1, 11.4, 12.2, 14.5, 14.5, 16, 18.2, 15.8, 17, 15.9, 16.4, 14.1, 14.5, 12.8, 13.5, 21.5, 14.4, 19.4, 18.6, 16.4, 15.5, 13.2, 12.8, 19.2, 18.2, 15.8, 15.4, 17.2, 17.2, 15.8, 16.7, 18.7, 15.1, 13.2, 13.4, 11.2, 13.7, 16.5, 14.2, 14.7, 14.5, 14.8, 16.7, 17.6, 14.9, 15.9, 13.6, 15.7, 15.8, 14.9, 16.6, 15.4, 18.2, 17.3, 18.2, 16.6, 15.4, 13.4, 13.2, 15.2, 14.9, 14.3, 15, 13, 14, 15.2, 14.4, 15, 20.1, 17.4, 24.8, 22.2, 13.2, 14.9, 19.2, 14.7, 16, 11.3, 12.9, 13.2, 14.7, 18.8, 15.5, 16.4, 16.5, 18.1, 20.1, 18.7, 15.8, 15.5, 17.5, 15, 15.2, 17.9, 14.4, 19.2, 21.7, 23.7, 19.9, 21.8, 13.8, 18, 15.3, 11.4, 12.5, 15.1, 17, 15.7, 16.4, 14.4, 12.6, 12.9, 16.9, 16.4, 16.1, 17.8, 19.4, 17.3, 16, 14.9, 16.2, 20.7, 14.2, 14.4, 16.8, 14.8, 18.3, 20.4, 19.6, 12.6, 13.8, 15.8, 19, 17.1, 16.6, 19.6, 18.6, 18, 16.2, 16, 18, 16.4, 15.3, 18.2, 17.6, 14.7, 17.3, 14.5, 14.5, 16.9, 15, 15.7, 16.2, 16.4, 17, 14.5, 14.7, 13.9, 13, 17.3, 15.6, 24.6, 11.6, 18.6, 19.4
        });
        auto model_year = gpu_array<float>::from_vector(std::vector<double>{
            70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82
        });
        auto origin = gpu_array<float>::from_vector(std::vector<double>{
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 2, 1, 3, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 2, 2, 2, 2, 1, 3, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 3, 1, 3, 3, 1, 1, 2, 1, 1, 2, 2, 2, 2, 1, 2, 3, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 1, 2, 2, 3, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 3, 2, 3, 1, 2, 1, 2, 2, 2, 2, 3, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1, 1, 1, 2, 3, 3, 1, 2, 1, 2, 3, 2, 1, 1, 1, 1, 3, 1, 2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 3, 2, 3, 2, 3, 2, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 3, 1, 1, 3, 2, 2, 2, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1, 2, 1, 2, 1, 1, 1, 3, 2, 1, 1, 1, 1, 2, 3, 1, 3, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 1, 3, 2, 2, 2, 2, 3, 3, 2, 3, 3, 2, 3, 1, 1, 1, 1, 1, 3, 1, 3, 3, 3, 3, 3, 1, 1, 1, 2, 3, 3, 3, 3, 2, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 3, 1, 1, 1, 2, 1, 1, 1
        });
        auto constant{
            gpu_array<float>::constant(1, origin.size(0))
        };
        auto features = linear_regressions::build_features( // double-checks and removes colinearity
            constant, cylinders, displacement, horsepower, weight, acceleration, model_year, origin
        );

        auto weights = linear_regressions::solve_for_weights(mpg, features);
        auto std_err = linear_regressions::standard_error(mpg, features, weights);
        auto std_dev = linear_regressions::standard_deviation(mpg, features, weights);
        auto t_stat = linear_regressions::t_statistic(weights, std_err);
        auto p_value = linear_regressions::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regressions::predict(features, weights);

        print("");
        print(
            weights.join(1,
                std_err).join(1,
                    t_stat).join(1,
                        p_value).join(1,
                            lower_95).join(1,
                                upper_95).to_string(
                                    { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");
        print(mpg.join(1, prediction).to_string({ "Measured", "Predicted" }));
        print("");
    }
    // Fish market regression. Not being performed correctly in any way, for reasons not yet understood.
    if (1) {
        /*          Coefficients	Standard Error	t Stat	        P-value	        Lower 95%	    Upper 95%
        Intercept	-499.5869554	29.57223974	    -16.89378146	8.44136E-37	    -558.0095858	-441.1643249
        Length1	    62.35521443	    40.20873868	    1.550787627	    0.123018636	    -17.08078028	141.7912091
        Length2	    -6.526752492	41.75876083	    -0.156296604	0.876005273	    -89.02495596	75.97145098
        Length3	    -29.02621861	17.35295765	    -1.672695756	0.096430795	    -63.30855369	5.256116464
        Height	    28.29735132	    8.729226223	    3.241679228	    0.001458477	    11.05197654	    45.54272611
        Width	    22.47330665	    20.37173285	    1.103161268	    0.271689248	    -17.77289147	62.71950478
        */

        auto weight = gpu_array<float>::from_vector(std::vector<double>{
            242, 290, 340, 363, 430, 450, 500, 390, 450, 500, 475, 500, 500, 340, 600, 600, 700, 700, 610, 650, 575, 685, 620, 680, 700, 725, 720, 714, 850, 1000, 920, 955, 925, 975, 950, 40, 69, 78, 87, 120, 0, 110, 120, 150, 145, 160, 140, 160, 169, 161, 200, 180, 290, 272, 390, 270, 270, 306, 540, 800, 1000, 55, 60, 90, 120, 150, 140, 170, 145, 200, 273, 300, 5.9, 32, 40, 51.5, 70, 100, 78, 80, 85, 85, 110, 115, 125, 130, 120, 120, 130, 135, 110, 130, 150, 145, 150, 170, 225, 145, 188, 180, 197, 218, 300, 260, 265, 250, 250, 300, 320, 514, 556, 840, 685, 700, 700, 690, 900, 650, 820, 850, 900, 1015, 820, 1100, 1000, 1100, 1000, 1000, 200, 300, 300, 300, 430, 345, 456, 510, 540, 500, 567, 770, 950, 1250, 1600, 1550, 1650, 6.7, 7.5, 7, 9.7, 9.8, 8.7, 10, 9.9, 9.8, 12.2, 13.4, 12.2, 19.7, 19.9
        });
        auto length1 = gpu_array<float>::from_vector(std::vector<double>{
            23.2, 24, 23.9, 26.3, 26.5, 26.8, 26.8, 27.6, 27.6, 28.5, 28.4, 28.7, 29.1, 29.5, 29.4, 29.4, 30.4, 30.4, 30.9, 31, 31.3, 31.4, 31.5, 31.8, 31.9, 31.8, 32, 32.7, 32.8, 33.5, 35, 35, 36.2, 37.4, 38, 12.9, 16.5, 17.5, 18.2, 18.6, 19, 19.1, 19.4, 20.4, 20.5, 20.5, 21, 21.1, 22, 22, 22.1, 23.6, 24, 25, 29.5, 23.6, 24.1, 25.6, 28.5, 33.7, 37.3, 13.5, 14.3, 16.3, 17.5, 18.4, 19, 19, 19.8, 21.2, 23, 24, 7.5, 12.5, 13.8, 15, 15.7, 16.2, 16.8, 17.2, 17.8, 18.2, 19, 19, 19, 19.3, 20, 20, 20, 20, 20, 20.5, 20.5, 20.7, 21, 21.5, 22, 22, 22.6, 23, 23.5, 25, 25.2, 25.4, 25.4, 25.4, 25.9, 26.9, 27.8, 30.5, 32, 32.5, 34, 34, 34.5, 34.6, 36.5, 36.5, 36.6, 36.9, 37, 37, 37.1, 39, 39.8, 40.1, 40.2, 41.1, 30, 31.7, 32.7, 34.8, 35.5, 36, 40, 40, 40.1, 42, 43.2, 44.8, 48.3, 52, 56, 56, 59, 9.3, 10, 10.1, 10.4, 10.7, 10.8, 11.3, 11.3, 11.4, 11.5, 11.7, 12.1, 13.2, 13.8
        });
        auto length2 = gpu_array<float>::from_vector(std::vector<double>{
            25.4, 26.3, 26.5, 29, 29, 29.7, 29.7, 30, 30, 30.7, 31, 31, 31.5, 32, 32, 32, 33, 33, 33.5, 33.5, 34, 34, 34.5, 35, 35, 35, 35, 36, 36, 37, 38.5, 38.5, 39.5, 41, 41, 14.1, 18.2, 18.8, 19.8, 20, 20.5, 20.8, 21, 22, 22, 22.5, 22.5, 22.5, 24, 23.4, 23.5, 25.2, 26, 27, 31.7, 26, 26.5, 28, 31, 36.4, 40, 14.7, 15.5, 17.7, 19, 20, 20.7, 20.7, 21.5, 23, 25, 26, 8.4, 13.7, 15, 16.2, 17.4, 18, 18.7, 19, 19.6, 20, 21, 21, 21, 21.3, 22, 22, 22, 22, 22, 22.5, 22.5, 22.7, 23, 23.5, 24, 24, 24.6, 25, 25.6, 26.5, 27.3, 27.5, 27.5, 27.5, 28, 28.7, 30, 32.8, 34.5, 35, 36.5, 36, 37, 37, 39, 39, 39, 40, 40, 40, 40, 42, 43, 43, 43.5, 44, 32.3, 34, 35, 37.3, 38, 38.5, 42.5, 42.5, 43, 45, 46, 48, 51.7, 56, 60, 60, 63.4, 9.8, 10.5, 10.6, 11, 11.2, 11.3, 11.8, 11.8, 12, 12.2, 12.4, 13, 14.3, 15
        });
        auto length3 = gpu_array<float>::from_vector(std::vector<double>{
            30, 31.2, 31.1, 33.5, 34, 34.7, 34.5, 35, 35.1, 36.2, 36.2, 36.2, 36.4, 37.3, 37.2, 37.2, 38.3, 38.5, 38.6, 38.7, 39.5, 39.2, 39.7, 40.6, 40.5, 40.9, 40.6, 41.5, 41.6, 42.6, 44.1, 44, 45.3, 45.9, 46.5, 16.2, 20.3, 21.2, 22.2, 22.2, 22.8, 23.1, 23.7, 24.7, 24.3, 25.3, 25, 25, 27.2, 26.7, 26.8, 27.9, 29.2, 30.6, 35, 28.7, 29.3, 30.8, 34, 39.6, 43.5, 16.5, 17.4, 19.8, 21.3, 22.4, 23.2, 23.2, 24.1, 25.8, 28, 29, 8.8, 14.7, 16, 17.2, 18.5, 19.2, 19.4, 20.2, 20.8, 21, 22.5, 22.5, 22.5, 22.8, 23.5, 23.5, 23.5, 23.5, 23.5, 24, 24, 24.2, 24.5, 25, 25.5, 25.5, 26.2, 26.5, 27, 28, 28.7, 28.9, 28.9, 28.9, 29.4, 30.1, 31.6, 34, 36.5, 37.3, 39, 38.3, 39.4, 39.3, 41.4, 41.4, 41.3, 42.3, 42.5, 42.4, 42.5, 44.6, 45.2, 45.5, 46, 46.6, 34.8, 37.8, 38.8, 39.8, 40.5, 41, 45.5, 45.5, 45.8, 48, 48.7, 51.2, 55.1, 59.7, 64, 64, 68, 10.8, 11.6, 11.6, 12, 12.4, 12.6, 13.1, 13.1, 13.2, 13.4, 13.5, 13.8, 15.2, 16.2
        });
        auto height = gpu_array<float>::from_vector(std::vector<double>{
            11.52, 12.48, 12.3778, 12.73, 12.444, 13.6024, 14.1795, 12.67, 14.0049, 14.2266, 14.2628, 14.3714, 13.7592, 13.9129, 14.9544, 15.438, 14.8604, 14.938, 15.633, 14.4738, 15.1285, 15.9936, 15.5227, 15.4686, 16.2405, 16.36, 16.3618, 16.517, 16.8896, 18.957, 18.0369, 18.084, 18.7542, 18.6354, 17.6235, 4.1472, 5.2983, 5.5756, 5.6166, 6.216, 6.4752, 6.1677, 6.1146, 5.8045, 6.6339, 7.0334, 6.55, 6.4, 7.5344, 6.9153, 7.3968, 7.0866, 8.8768, 8.568, 9.485, 8.3804, 8.1454, 8.778, 10.744, 11.7612, 12.354, 6.8475, 6.5772, 7.4052, 8.3922, 8.8928, 8.5376, 9.396, 9.7364, 10.3458, 11.088, 11.368, 2.112, 3.528, 3.824, 4.5924, 4.588, 5.2224, 5.1992, 5.6358, 5.1376, 5.082, 5.6925, 5.9175, 5.6925, 6.384, 6.11, 5.64, 6.11, 5.875, 5.5225, 5.856, 6.792, 5.9532, 5.2185, 6.275, 7.293, 6.375, 6.7334, 6.4395, 6.561, 7.168, 8.323, 7.1672, 7.0516, 7.2828, 7.8204, 7.5852, 7.6156, 10.03, 10.2565, 11.4884, 10.881, 10.6091, 10.835, 10.5717, 11.1366, 11.1366, 12.4313, 11.9286, 11.73, 12.3808, 11.135, 12.8002, 11.9328, 12.5125, 12.604, 12.4888, 5.568, 5.7078, 5.9364, 6.2884, 7.29, 6.396, 7.28, 6.825, 7.786, 6.96, 7.792, 7.68, 8.9262, 10.6863, 9.6, 9.6, 10.812, 1.7388, 1.972, 1.7284, 2.196, 2.0832, 1.9782, 2.2139, 2.2139, 2.2044, 2.0904, 2.43, 2.277, 2.8728, 2.9322
        });
        auto width = gpu_array<float>::from_vector(std::vector<double>{
            4.02, 4.3056, 4.6961, 4.4555, 5.134, 4.9274, 5.2785, 4.69, 4.8438, 4.9594, 5.1042, 4.8146, 4.368, 5.0728, 5.1708, 5.58, 5.2854, 5.1975, 5.1338, 5.7276, 5.5695, 5.3704, 5.2801, 6.1306, 5.589, 6.0532, 6.09, 5.8515, 6.1984, 6.603, 6.3063, 6.292, 6.7497, 6.7473, 6.3705, 2.268, 2.8217, 2.9044, 3.1746, 3.5742, 3.3516, 3.3957, 3.2943, 3.7544, 3.5478, 3.8203, 3.325, 3.8, 3.8352, 3.6312, 4.1272, 3.906, 4.4968, 4.7736, 5.355, 4.2476, 4.2485, 4.6816, 6.562, 6.5736, 6.525, 2.3265, 2.3142, 2.673, 2.9181, 3.2928, 3.2944, 3.4104, 3.1571, 3.6636, 4.144, 4.234, 1.408, 1.9992, 2.432, 2.6316, 2.9415, 3.3216, 3.1234, 3.0502, 3.0368, 2.772, 3.555, 3.3075, 3.6675, 3.534, 3.4075, 3.525, 3.525, 3.525, 3.995, 3.624, 3.624, 3.63, 3.626, 3.725, 3.723, 3.825, 4.1658, 3.6835, 4.239, 4.144, 5.1373, 4.335, 4.335, 4.5662, 4.2042, 4.6354, 4.7716, 6.018, 6.3875, 7.7957, 6.864, 6.7408, 6.2646, 6.3666, 7.4934, 6.003, 7.3514, 7.1064, 7.225, 7.4624, 6.63, 6.8684, 7.2772, 7.4165, 8.142, 7.5958, 3.3756, 4.158, 4.3844, 4.0198, 4.5765, 3.977, 4.3225, 4.459, 5.1296, 4.896, 4.87, 5.376, 6.1712, 6.9849, 6.144, 6.144, 7.48, 1.0476, 1.16, 1.1484, 1.38, 1.2772, 1.2852, 1.2838, 1.1659, 1.1484, 1.3936, 1.269, 1.2558, 2.0672, 1.8792
        });
        auto constant{
            gpu_array<float>::constant(1, width.size(0))
        };
        auto features = linear_regressions::build_features( // double-checks and removes colinearity
            constant, length1, length2, length3, height, width
        );

        auto weights = linear_regressions::solve_for_weights(weight, features);
        auto std_err = linear_regressions::standard_error(weight, features, weights);
        auto std_dev = linear_regressions::standard_deviation(weight, features, weights);
        auto t_stat = linear_regressions::t_statistic(weights, std_err);
        auto p_value = linear_regressions::p_value(features, t_stat);
        auto lower_95 = weights - (1.96 * std_err);
        auto upper_95 = weights + (1.96 * std_err);
        auto prediction = linear_regressions::predict(features, weights);

        print("");
        print(
            weights.join(1,
                std_err).join(1,
                    t_stat).join(1,
                        p_value).join(1,
                            lower_95).join(1,
                                upper_95).to_string(
                                    { "Coefficients", "Standard Error", "t Stat", "P-value", "Lower 95%", "Upper 95%" })
        );
        print("");

        print(weight.join(1, prediction).to_string({ "Measured", "Predicted" }));
        print("");

    }

}






