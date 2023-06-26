/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

This file is distributed under the BSD License.
Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
http://www.chaiscript.com

 History: RTG	/	2023		1. Modified original source code to use WaterWatch tools, and for better real-time support, including object-typing from parsed code, pre-parsing code without running, real multithreaded code analysis, and more.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "chaiscript_wrapper.h"

namespace chaiscript::bootstrap::operators {
    template<typename T>
    void assign(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs = rhs; }), "=");
    }

    template<typename T>
    void assign_bitwise_and(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs &= rhs; }), "&=");
    }

    template<typename T>
    void assign_xor(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs ^= rhs; }), "^=");
    }

    template<typename T>
    void assign_bitwise_or(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs |= rhs; }), "|=");
    }

    template<typename T>
    void assign_difference(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs -= rhs; }), "-=");
    }

    template<typename T>
    void assign_left_shift(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs <<= rhs; }), "<<=");
    }

    template<typename T>
    void assign_product(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs <<= rhs; }), "*=");
    }

    template<typename T>
    void assign_quotient(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs /= rhs; }), "/=");
    }

    template<typename T>
    void assign_remainder(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs %= rhs; }), "%=");
    }

    template<typename T>
    void assign_right_shift(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs >>= rhs; }), ">>=");
    }

    template<typename T>
    void assign_sum(Module& m) {
        m.add(chaiscript::fun([](T& lhs, const T& rhs) -> T& { return lhs += rhs; }), "+=");
    }

    template<typename T>
    void prefix_decrement(Module& m) {
        m.add(chaiscript::fun([](T& lhs) -> T& { return --lhs; }), "--");
    }

    template<typename T>
    void prefix_increment(Module& m) {
        m.add(chaiscript::fun([](T& lhs) -> T& { return ++lhs; }), "++");
    }

    template<typename T>
    void equal(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs == rhs; }), "==");
    }

    template<typename T>
    void greater_than(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs > rhs; }), ">");
    }

    template<typename T>
    void greater_than_equal(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs >= rhs; }), ">=");
    }

    template<typename T>
    void less_than(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs < rhs; }), "<");
    }

    template<typename T>
    void less_than_equal(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs <= rhs; }), "<=");
    }

    template<typename T>
    void logical_compliment(Module& m) {
        m.add(chaiscript::fun([](const T& lhs) { return !lhs; }), "!");
    }

    template<typename T>
    void not_equal(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs != rhs; }), "!=");
    }

    template<typename T>
    void addition(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs + rhs; }), "+");
    }

    template<typename T>
    void unary_plus(Module& m) {
        m.add(chaiscript::fun([](const T& lhs) { return +lhs; }), "+");
    }

    template<typename T>
    void subtraction(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs - rhs; }), "-");
    }

    template<typename T>
    void unary_minus(Module& m) {
        m.add(chaiscript::fun([](const T& lhs) { return -lhs; }), "-");
    }

    template<typename T>
    void bitwise_and(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs & rhs; }), "&");
    }

    template<typename T>
    void bitwise_compliment(Module& m) {
        m.add(chaiscript::fun([](const T& lhs) { return ~lhs; }), "~");
    }

    template<typename T>
    void bitwise_xor(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs ^ rhs; }), "^");
    }

    template<typename T>
    void bitwise_or(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs | rhs; }), "|");
    }

    template<typename T>
    void division(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs / rhs; }), "/");
    }

    template<typename T>
    void left_shift(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs << rhs; }), "<<");
    }

    template<typename T>
    void multiplication(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs * rhs; }), "*");
    }

    template<typename T>
    void remainder(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs % rhs; }), "%");
    }

    template<typename T>
    void right_shift(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) { return lhs >> rhs; }), ">>");
    }

    template<typename T>
    void logical_list(Module& m) {
        m.add(chaiscript::fun([](const T& lhs, const T& rhs) {
            T rhs_as_lhs = lhs; 
            rhs_as_lhs = rhs; // in the case of cweeUnitedValues, this ensures we correctly converted the rhs to using the units (and presenation) of the lhs

            std::vector<chaiscript::Boxed_Value> out;
            if (lhs <= rhs) {
                for (T x = lhs; x < rhs_as_lhs; ++x) {
                    out.push_back(chaiscript::Boxed_Value(x));
                }
            }
            else {
                for (T x = lhs; x > rhs_as_lhs; --x) {
                    out.push_back(chaiscript::Boxed_Value(x));
                }
            }
            out.push_back(chaiscript::Boxed_Value(rhs_as_lhs));

            return out;
        }), "..");
    }
} // namespace chaiscript::bootstrap::operators
