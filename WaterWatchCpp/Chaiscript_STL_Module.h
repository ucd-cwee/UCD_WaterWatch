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
    class Std_Lib {
    public:
        [[nodiscard]] static ModulePtr library() {
            auto lib = chaiscript::make_shared<Module>();
            bootstrap::Bootstrap::bootstrap(*lib);

            bootstrap::standard_library::vector_type<chaiscript::small_vector<Boxed_Value>>("Vector", *lib);
            bootstrap::standard_library::string_type<std::string>("string", *lib);
            bootstrap::standard_library::map_type<std::map<std::string, Boxed_Value>>("Map", *lib);
            bootstrap::standard_library::pair_type<std::pair<Boxed_Value, Boxed_Value>>("Pair", *lib);

            json_wrap::library(*lib);

            lib->eval(ChaiScript_Prelude::chaiscript_prelude() /*, "standard prelude"*/);

            return lib;
        }
    };
} // namespace chaiscript