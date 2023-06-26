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
    class ContainerValueType_Definition {
    public:
        ContainerValueType_Definition() : containerType_m(), value_type_m() {};
        ContainerValueType_Definition(Type_Info const& container, Type_Info const& value_type) : containerType_m(container), value_type_m(value_type) {};
        ContainerValueType_Definition(ContainerValueType_Definition const& o) : containerType_m(o.containerType_m), value_type_m(o.value_type_m) {};
        ContainerValueType_Definition& operator=(ContainerValueType_Definition const& o) {
            containerType_m = o.containerType_m;
            value_type_m = o.value_type_m;
            return *this;
        };
        ~ContainerValueType_Definition() {};
        Type_Info containerType_m;
        Type_Info value_type_m;
    };
};
namespace chaiscript {
    INLINE ContainerValueType_Definition containerValueType(Type_Info const& container, Type_Info const& value_type) noexcept {
        return ContainerValueType_Definition(container, value_type);
    };
};