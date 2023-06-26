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
#include "Precompiled.h"

namespace chaiscript::utility {
    struct Static_String {
        template<size_t N>
        constexpr Static_String(const char(&str)[N]) noexcept
            : m_size(N - 1)
            , data(&str[0]) {
        }

        constexpr size_t size() const noexcept { return m_size; }

        constexpr const char* c_str() const noexcept { return data; }

        constexpr auto begin() const noexcept { return data; }

        constexpr auto end() const noexcept { return data + m_size; }

        constexpr bool operator==(std::string_view other) const noexcept {
            // return std::string_view(data, m_size) == other;
            auto b1 = begin();
            const auto e1 = end();
            auto b2 = other.begin();
            const auto e2 = other.end();

            if (e1 - b1 != e2 - b2) {
                return false;
            }

            while (b1 != e1) {
                if (*b1 != *b2) {
                    return false;
                }
                ++b1;
                ++b2;
            }
            return true;
        }

        bool operator==(const std::string& t_str) const noexcept { return std::equal(begin(), end(), std::cbegin(t_str), std::cend(t_str)); }

        operator const char* () const {
            return c_str();
        };

        const size_t m_size;
        const char* data = nullptr;
    };
} // namespace chaiscript::utility