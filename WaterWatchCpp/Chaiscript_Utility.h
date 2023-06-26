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

namespace chaiscript::utility {
    /// Single step command for registering a class with ChaiScript
    ///
    /// \param[in,out] t_module Model to add class to
    /// \param[in] t_class_name Name of the class being registered
    /// \param[in] t_constructors Vector of constructors to add
    /// \param[in] t_funcs Vector of methods to add
    ///
    /// \example Adding a basic class to ChaiScript in one step
    ///
    /// \code
    /// chaiscript::utility::add_class<test>(*m,
    ///      "test",
    ///      { constructor<test ()>(),
    ///        constructor<test (const test &)>() },
    ///      { {fun(&test::function), "function"},
    ///        {fun(&test::function2), "function2"},
    ///        {fun(&test::function3), "function3"},
    ///        {fun(static_cast<std::string(test::*)(double)>(&test::function_overload)), "function_overload" },
    ///        {fun(static_cast<std::string(test::*)(int)>(&test::function_overload)), "function_overload" },
    ///        {fun(static_cast<test & (test::*)(const test &)>(&test::operator=)), "=" }
    ///        }
    ///      );
    ///
    template<typename Class, typename ModuleType>
    void add_class(ModuleType& t_module,
        const std::string& t_class_name,
        const std::vector<chaiscript::Proxy_Function>& t_constructors,
        const std::vector<std::pair<chaiscript::Proxy_Function, std::string>>& t_funcs) {
        t_module.add(chaiscript::user_type<Class>(), t_class_name);

        for (const chaiscript::Proxy_Function& ctor : t_constructors) {
            t_module.add(ctor, t_class_name);
        }

        for (const auto& fun : t_funcs) {
            t_module.add(fun.first, fun.second);
        }
    }

    template<typename Enum, typename ModuleType>
    typename std::enable_if<std::is_enum<Enum>::value, void>::type
        add_class(ModuleType& t_module,
            const std::string& t_class_name,
            const std::vector<std::pair<typename std::underlying_type<Enum>::type, std::string>>& t_constants) {
        t_module.add(chaiscript::user_type<Enum>(), t_class_name);

        t_module.add(chaiscript::constructor<Enum()>(), t_class_name);
        t_module.add(chaiscript::constructor<Enum(const Enum&)>(), t_class_name);

        using namespace chaiscript::bootstrap::operators;
        equal<Enum>(t_module);
        not_equal<Enum>(t_module);
        assign<Enum>(t_module);

        t_module.add(chaiscript::fun([](const Enum& e, const int& i) { return e == i; }), "==");
        t_module.add(chaiscript::fun([](const int& i, const Enum& e) { return i == e; }), "==");

        for (const auto& constant : t_constants) {
            t_module.add_global_const(chaiscript::const_var(Enum(constant.first)), constant.second);
        }
    }

    template<typename EnumClass, typename ModuleType>
    typename std::enable_if<std::is_enum<EnumClass>::value, void>::type
        add_class(ModuleType& t_module, const std::string& t_class_name, const std::vector<std::pair<EnumClass, std::string>>& t_constants) {
        t_module.add(chaiscript::user_type<EnumClass>(), t_class_name);

        t_module.add(chaiscript::constructor<EnumClass()>(), t_class_name);
        t_module.add(chaiscript::constructor<EnumClass(const EnumClass&)>(), t_class_name);

        using namespace chaiscript::bootstrap::operators;
        equal<EnumClass>(t_module);
        not_equal<EnumClass>(t_module);
        assign<EnumClass>(t_module);

        for (const auto& constant : t_constants) {
            t_module.add_global_const(chaiscript::const_var(EnumClass(constant.first)), constant.second);
        }
    }
} // namespace chaiscript::utility

