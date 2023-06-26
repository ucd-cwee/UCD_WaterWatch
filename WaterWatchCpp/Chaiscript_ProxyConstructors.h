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

namespace chaiscript::dispatch::detail {
    template<typename Class, typename... Params>
    Proxy_Function build_constructor_(Class(*)(Params...)) {
        if constexpr (!std::is_copy_constructible_v<Class>) {
            auto call = [](auto &&...param) { return chaiscript::make_shared<Class>(std::forward<decltype(param)>(param)...); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base,
                dispatch::Proxy_Function_Callable_Impl<chaiscript::shared_ptr<Class>(Params...), decltype(call)>>(call));
        }
        else if constexpr (true) {
            auto call = [](auto &&...param) { return Class(std::forward<decltype(param)>(param)...); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Class(Params...), decltype(call)>>(
                    call));
        }
    }
} // namespace chaiscript::dispatch::detail

namespace chaiscript::dispatch::detail {
    template<typename Class, typename Class2>
    Proxy_Function build_base_class_with_derived() {
        if constexpr (!std::is_copy_constructible_v<Class>) {
            auto call = []() { return chaiscript::make_shared<Class>(Class2()); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base,
                dispatch::Proxy_Function_Callable_Impl<chaiscript::shared_ptr<Class>(), decltype(call)>>(call));
        }
        else if constexpr (true) {
            auto call = [](auto &&...param) { return Class(Class2()); };

            return Proxy_Function(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Class(), decltype(call)>>(
                    call));
        }
    }
} // namespace chaiscript::dispatch::detail

namespace chaiscript {
    /// \brief Generates a constructor function for use with ChaiScript
    ///
    /// \tparam T The signature of the constructor to generate. In the form of: ClassType (ParamType1, ParamType2, ...)
    ///
    /// Example:
    /// \code
    ///    chaiscript::ChaiScript chai;
    ///    // Create a new function that creates a MyClass object using the (int, float) constructor
    ///    // and call that function "MyClass" so that it appears as a normal constructor to the user.
    ///    chai.add(constructor<MyClass (int, float)>(), "MyClass");
    /// \endcode
    template<typename T>
    Proxy_Function constructor() {
        T* f = nullptr;
        return (dispatch::detail::build_constructor_(f));
    }

    template<typename T, typename T2>
    Proxy_Function constructor_from_derived() {
        return (dispatch::detail::build_base_class_with_derived<T,T2>());
    }
} // namespace chaiscript