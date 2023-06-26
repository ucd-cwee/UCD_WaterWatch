/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "chaiscript_wrapper.h"
#include "cweeAny.h"
#include "SharedPtr.h"
#include "cweeJob.h"
#include "cwee_math.h"
#include "Strings.h"
#include "List.h"
#include "WaterWatch_Module_Header.h"
#include "WaterWatch_Module_P1.h"
#include "WaterWatch_Module_P2.h"
#include "WaterWatch_Module_P3.h"
#include "WaterWatch_Module_EPAnet.h"
#include "WaterWatch_Module_P4.h"

namespace chaiscript {
    class WaterWatch_ChaiScript : public ChaiScript_Basic {
    public:
        WaterWatch_ChaiScript(std::vector<std::string> t_modulepaths = {}, std::vector<std::string> t_usepaths = {}, std::vector<Options> t_opts = chaiscript::default_options());
        ~WaterWatch_ChaiScript();
    };
}; // namespace chaiscript