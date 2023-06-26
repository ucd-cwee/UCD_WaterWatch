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

namespace chaiscript {
    namespace utility {
        namespace fnv1a {
            template<typename Itr>
            static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4307)
#endif
                std::uint32_t h = 0x811c9dc5;

                while (begin != end) {
                    h = (h ^ (*begin)) * 0x01000193;
                    ++begin;
                }
                return h;

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
            }

            template<size_t N>
            static constexpr std::uint32_t hash(const char(&str)[N]) noexcept {
                return hash(std::begin(str), std::end(str) - 1);
            }

            static constexpr std::uint32_t hash(std::string_view sv) noexcept {
                return hash(sv.begin(), sv.end());
            }

            static inline std::uint32_t hash(const std::string& s) noexcept {
                return hash(s.begin(), s.end());
            }
        } // namespace fnv1a

        namespace jenkins_one_at_a_time {
            template<typename Itr>
            static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
                std::uint32_t hash = 0;

                while (begin != end) {
                    hash += std::uint32_t(*begin);
                    hash += hash << 10;
                    hash ^= hash >> 6;
                    ++begin;
                }

                hash += hash << 3;
                hash ^= hash >> 11;
                hash += hash << 15;
                return hash;
            }

            template<size_t N>
            static constexpr std::uint32_t hash(const char(&str)[N]) noexcept {
                return hash(std::begin(str), std::end(str) - 1);
            }

            static constexpr std::uint32_t hash(std::string_view sv) noexcept {
                return hash(sv.begin(), sv.end());
            }

            static inline std::uint32_t hash(const std::string& s) noexcept {
                return hash(s.begin(), s.end());
            }
        } // namespace jenkins_one_at_a_time

        using fnv1a::hash;
    } // namespace utility
} // namespace chaiscript