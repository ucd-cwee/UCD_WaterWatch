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
#include "WaterWatch_Module_P3.h"
#include "cweeScheduler.h"
#include "EPAnetWrapper.h" 
#include "AppLayerRequests.h" // queue for job-requests for processing by another system
#include "enum.h"

namespace chaiscript {
    namespace WaterWatch_Lib {
        void DefVectors_1(ModulePtr& lib) {
            using namespace ::epanet;
            using namespace cwee_units;

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Sdemand);

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Passet);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Pnode);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Plink);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Ptank);

            DEF_DECLARE_VECTOR_OF_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE(Plink, direction_t);
        };
    };
};



namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_3() {
            auto lib = chaiscript::make_shared<Module>();

            // impliment cweePair
            if (1) {
#define STR_FUNC( thing ) cweeStr(#thing)
#define DEF_DECLARE_PAIR( ValueTypeAClass, ValueTypeBClass ) \
                { \
                    cweeStr pairName = (cweeStr("pair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
                    chaiscript::bootstrap::standard_library::pair_type<std::pair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
                    lib->add(chaiscript::type_conversion<std::pair<ValueTypeAClass, ValueTypeBClass>, std::string>([](const std::pair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(t_bt.first).c_str(), cweeStr::ToString<ValueTypeBClass>(t_bt.second).c_str()).c_str()); }, nullptr)); \
                    lib->add(chaiscript::fun([](std::string& a, const std::pair<ValueTypeAClass, ValueTypeBClass>& b) {	a = std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); return a; }), "="); \
                    lib->add(chaiscript::fun([](const std::pair<ValueTypeAClass, ValueTypeBClass>& b) {	return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); }), "to_string"); \
                } \
                { \
                    cweeStr pairName = (cweeStr("cweepair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
                    chaiscript::bootstrap::standard_library::pair_type<cweePair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
                    lib->add(chaiscript::type_conversion<cweePair<ValueTypeAClass, ValueTypeBClass>, std::string>([](const cweePair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(t_bt.first).c_str(), cweeStr::ToString<ValueTypeBClass>(t_bt.second).c_str()).c_str()); }, nullptr)); \
                    lib->add(chaiscript::fun([](std::string& a, const cweePair<ValueTypeAClass, ValueTypeBClass>& b) {	a = std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); return a; }), "="); \
                    lib->add(chaiscript::fun([](const cweePair<ValueTypeAClass, ValueTypeBClass>& b) {	return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); }), "to_string"); \
                }

                //using namespace cweeUnitValues;

                //DEF_DECLARE_PAIR(unit_value, unit_value);
            }

            // Generic implimentation of the cweeVector
            if (1) {
                cweeStr vecName = "Vector";
                cweeStr cweeVecName = "cweeVector";
                lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, cweeThreadedList<chaiscript::Boxed_Value>>(nullptr));
                lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, std::vector<chaiscript::Boxed_Value>>(nullptr));
                lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, cweeThreadedList<chaiscript::Boxed_Value>>(nullptr));
                chaiscript::bootstrap::standard_library::vector_type<cweeThreadedList<chaiscript::Boxed_Value>>(cweeVecName.c_str(), *lib);
                lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const std::vector<chaiscript::Boxed_Value>&)>(), cweeVecName.c_str());
                lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const cweeThreadedList<chaiscript::Boxed_Value>&)>(), cweeVecName.c_str());      
                lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, const cweeThreadedList<chaiscript::Boxed_Value>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)b; return t_ti; }), "=");                
                lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<chaiscript::Boxed_Value>& b) { t_ti = b; return t_ti; }), "=");
                lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, const std::vector<chaiscript::Boxed_Value>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");
                lib->add(chaiscript::type_conversion<cweeThreadedList<chaiscript::Boxed_Value>&, std::vector<chaiscript::Boxed_Value>&>([](const cweeThreadedList<chaiscript::Boxed_Value>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));
                lib->add(chaiscript::type_conversion<std::vector<chaiscript::Boxed_Value>&, cweeThreadedList<chaiscript::Boxed_Value>&>([](const std::vector<chaiscript::Boxed_Value>& t_bt) { cweeThreadedList<chaiscript::Boxed_Value> out; out = (cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));
            }

            // Specific Implimentations of cweeLists to assist with data member navigation
            if (1) {
                // bootstrap::standard_library::vector_type<cweeList<Boxed_Value>>("cweeList", *lib);
                // bootstrap::standard_library::vector_type<cweeList<Boxed_Value>>("cweeVector", *lib.get());
                {
                    DefVectors_1(lib);

                    DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(cweeStr);
                    DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(std::string);
                }
            }

            return lib;
        };
    };
}; // namespace chaiscript