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
#include "chaiscript_wrapper.h"
#include "EPAnetWrapper.h" 
#include "enum.h"

namespace chaiscript {
    namespace WaterWatch_Lib {
        void DefVectors_3(ModulePtr& lib) {
            using namespace ::epanet;
            using namespace cwee_units;


            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Prule);

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(foot_t);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(cubic_foot_per_second_t);

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(cubic_foot_t);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(SCALER);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(cfs_p_ft_t);

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(StatusType);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(int);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(float);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(double);
            DEF_DECLARE_VECTOR_OF_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE(double, double);

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(FlowDirection);

            {
                using namespace cweeUnitValues;
                DEF_DECLARE_VECTOR_OF_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE(unit_value, unit_value);                
            }
        };
    };
};



namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_3p2() {
            auto lib = chaiscript::make_shared<Module>();

            // Specific Implimentations of cweeLists to assist with data member navigation
            if (1) {
                {
                    DefVectors_3(lib);
                }
            }

            return lib;
        };
    };
}; // namespace chaiscript