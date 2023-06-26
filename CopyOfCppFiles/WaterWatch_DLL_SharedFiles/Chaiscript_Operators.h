#pragma once
#include "Precompiled.h"

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
            auto type = lhs + rhs;

            std::vector<chaiscript::Boxed_Value> out;
            for (decltype(type) x = lhs; x < rhs; ++x) {
                out.push_back(chaiscript::Boxed_Value(x));
            }
            for (decltype(type) x = lhs; x > rhs; --x) {
                out.push_back(chaiscript::Boxed_Value(x));
            }
            out.push_back(chaiscript::Boxed_Value((decltype(type))rhs));

            return out;
            }), "..");
    }
} // namespace chaiscript::bootstrap::operators
