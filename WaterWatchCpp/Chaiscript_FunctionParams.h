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

namespace chaiscript {
    class Function_Params {
    public:
        constexpr Function_Params(const Boxed_Value* const t_begin, const Boxed_Value* const t_end)
            : m_begin(t_begin)
            , m_end(t_end) {
        }

        explicit Function_Params(const Boxed_Value& bv)
            : m_begin(&bv)
            , m_end(m_begin + 1) {
        }

        explicit Function_Params(const std::vector<Boxed_Value>& vec)
            : m_begin(vec.empty() ? nullptr : &vec.front())
            , m_end(vec.empty() ? nullptr : &vec.front() + vec.size()) {
        }

        template<size_t Size>
        constexpr explicit Function_Params(const std::array<Boxed_Value, Size>& a)
            : m_begin(&a.front())
            , m_end(&a.front() + Size) {
        }

        [[nodiscard]] constexpr const Boxed_Value& operator[](const std::size_t t_i) const noexcept { return m_begin[t_i]; }

        [[nodiscard]] constexpr const Boxed_Value* begin() const noexcept { return m_begin; }

        [[nodiscard]] constexpr const Boxed_Value& front() const noexcept { return *m_begin; }

        [[nodiscard]] constexpr const Boxed_Value* end() const noexcept { return m_end; }

        [[nodiscard]] constexpr std::size_t size() const noexcept { return std::size_t(m_end - m_begin); }

        [[nodiscard]] std::vector<Boxed_Value> to_vector() const { return std::vector<Boxed_Value>{m_begin, m_end}; }

        [[nodiscard]] constexpr bool empty() const noexcept { return m_begin == m_end; }

    private:
        const Boxed_Value* m_begin = nullptr;
        const Boxed_Value* m_end = nullptr;
    };

    // Constructor specialization for array of size 0
    template<>
    constexpr Function_Params::Function_Params(const std::array < Boxed_Value, size_t{ 0 } > & /* a */)
        : m_begin(nullptr)
        , m_end(nullptr) {
    }

} // namespace chaiscript
