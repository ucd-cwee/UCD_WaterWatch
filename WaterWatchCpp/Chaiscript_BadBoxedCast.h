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
    namespace exception {
        /// \brief Thrown in the event that a Boxed_Value cannot be cast to the desired type
        ///
        /// It is used internally during function dispatch and may be used by the end user.
        ///
        /// \sa chaiscript::boxed_cast
        class bad_boxed_cast : public std::bad_cast {
        public:
            bad_boxed_cast(Type_Info t_from, const std::type_info& t_to, utility::Static_String t_what) noexcept
                : from(t_from)
                , to(&t_to)
                , m_what(std::move(t_what)) {
            }

            bad_boxed_cast(Type_Info t_from, const std::type_info& t_to) noexcept
                : from(t_from)
                , to(&t_to)
                , m_what("Cannot perform boxed_cast") {
            }

            explicit bad_boxed_cast(utility::Static_String t_what) noexcept
                : m_what(std::move(t_what)) {
            }

            bad_boxed_cast(const bad_boxed_cast&) noexcept = default;
            ~bad_boxed_cast() noexcept override = default;

            /// \brief Description of what error occurred
            const char* what() const noexcept override { return m_what.c_str(); }

            Type_Info from; ///< Type_Info contained in the Boxed_Value
            const std::type_info* to = nullptr; ///< std::type_info of the desired (but failed) result type

        private:
            utility::Static_String m_what;
        };
    } // namespace exception
} // namespace chaiscript