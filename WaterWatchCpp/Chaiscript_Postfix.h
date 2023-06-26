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
    class Postfix_Definition {
    public:
        Postfix_Definition() : postFix_m(), functionName_m() {};
        Postfix_Definition(std::string const& postFix_p, std::string const& functionName_p) : postFix_m(postFix_p), functionName_m(functionName_p) {};
        ~Postfix_Definition() {};
        std::string postFix_m;
        std::string functionName_m;
    };
};
namespace chaiscript {
    INLINE Postfix_Definition postfix(std::string const& postFix_p, std::string const& functionName_p) noexcept {
        return Postfix_Definition(postFix_p, functionName_p);
    };
};