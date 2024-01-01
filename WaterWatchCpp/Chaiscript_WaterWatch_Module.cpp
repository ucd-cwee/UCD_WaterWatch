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
#include "Chaiscript_WaterWatch_Module.h"
#include "WaterWatch_Module_P1.h"
#include "WaterWatch_Module_P2.h"
#include "WaterWatch_Module_P3.h"
#include "WaterWatch_Module_EPAnet.h"
#include "WaterWatch_Module_P4.h"
#include "../GDAL/GDAL.h"
#include "../ExcelInterop/Wrapper.h"
#include "MachineLearning.h"
#include "odbc.h"

namespace chaiscript {
    WaterWatch_ChaiScript::WaterWatch_ChaiScript(std::vector<std::string> t_modulepaths, std::vector<std::string> t_usepaths, std::vector<Options> t_opts)
        : ChaiScript_Basic(
            {
                chaiscript::Std_Lib::library(),
                chaiscript::WaterWatch_Lib::library_1(),
                chaiscript::WaterWatch_Lib::ODBC_library(),
                chaiscript::WaterWatch_Lib::library_2(),
                chaiscript::WaterWatch_Lib::library_3(),
                chaiscript::WaterWatch_Lib::library_3p1(),
                chaiscript::WaterWatch_Lib::library_3p2(),
                chaiscript::WaterWatch_Lib::library_3p3(),
                chaiscript::WaterWatch_Lib::library_EPAnet(),
                chaiscript::WaterWatch_Lib::library_4(),
                chaiscript::WaterWatch_Lib::Excel_library(),
                chaiscript::WaterWatch_Lib::GDAL_library(),
                chaiscript::WaterWatch_Lib::MachineLearning_Library()                
            }
            , chaiscript::make_parser<eval::Noop_Tracer, optimizer::Optimizer_Default>()
            , std::move(t_modulepaths)
            , std::move(t_usepaths)
            , std::move(t_opts)
        ) {};
    WaterWatch_ChaiScript::~WaterWatch_ChaiScript(){ 
        
    };
}; 







