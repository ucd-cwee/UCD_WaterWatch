#pragma once
#include "Precompiled.h"

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

#ifndef CHAISCRIPT_NO_THREADS
            bootstrap::standard_library::future_type<std::future<chaiscript::Boxed_Value>>("future", *lib);
            lib->add(chaiscript::fun(
                [](const std::function<chaiscript::Boxed_Value()>& t_func) { return std::async(std::launch::async, t_func); }),
                "async");
#endif

            json_wrap::library(*lib);

            lib->eval(ChaiScript_Prelude::chaiscript_prelude() /*, "standard prelude"*/);

            return lib;
        }
    };
} // namespace chaiscript