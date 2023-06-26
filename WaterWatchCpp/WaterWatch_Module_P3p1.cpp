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
        void DefVectors_2(ModulePtr& lib) {
            using namespace ::epanet;
            using namespace cwee_units;

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Ppump);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Pvalve);

            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Pzone);


            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Ppattern);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Pcurve);
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(Pcontrol);

            DEF_DECLARE_VECTOR_OF_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE(SCALER, SCALER);            
        };
    };
};



namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr library_3p1() {
            auto lib = chaiscript::make_shared<Module>();

            // Specific Implimentations of cweeLists to assist with data member navigation
            if (1) {
                {
                    DefVectors_2(lib);
                }
            }

            return lib;
        };
    };
}; // namespace chaiscript