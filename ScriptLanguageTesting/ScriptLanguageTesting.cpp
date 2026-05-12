#pragma once
//#include <math.h>
//#include <stdio.h>
//#include <algorithm>
//#include <iterator>
//#include <fstream>
//#include <iostream>
//#include <sstream>
//#include <string>
//#include <vector>
//#include <map>
//#include <iostream>
//#include <string>
//#include <string_view>
//#include <regex>
//#include <list>
//#include <thread>
//#include <concurrent_unordered_map.h>
//#include <stdlib.h>
#include "aba_problem.h"
#include "util.h"
#include "Parallel.h"

//#include "stopwatch.h"
//#include "strings.h"
//#include "types.h"
//#include "scripting.h"

int main() {
#if 0
    if (0) {
        GL::stopwatch sw;
        if (0) {
            auto callable = GL::make_callable("at", &GL::string::at);
            std::array<GL::any::fast_any, 2> example{
                 GL::any::fast_any::instance(GL::string("this")),
                 GL::any::fast_any::instance(0ull)
            };
            if (auto timer = sw.debug_timer("operator() with callable and w/o converters, no conversion needed")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable->operator()(&example[0], &example[0] + example.size());
                }
            }
        }
        if (0) {
            GL::scope::impl::RootScope root;
            root.perform_builtins();
            root.try_get_converter(GL::type_of<GL::string>(), GL::type_of<GL::string>());
            auto converters = root.get_converters();
            auto callable = GL::make_callable("at", &GL::string::at);
            std::array<GL::any::fast_any, 2> example{
                 GL::any::fast_any::instance(GL::string("this")),
                 GL::any::fast_any::instance(0ull)
            };
            if (auto timer = sw.debug_timer("operator() with callable and w/converters, no conversion needed")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)callable->operator()(&example[0], &example[0] + example.size());
                }
            }
        }
    }   
#endif
    for (int j = 0; j < 1000000; ++j) {
        auto f = [](int i) {};
        GL::parallel::impl::function_traits<decltype(f)>::arguments;

        GL::parallel::For(0, 1000000, [](int i) {
            if (i < 0) throw std::runtime_error("e");
        });
    }
    return 0;
};

