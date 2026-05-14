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
#include "units.h"
#include "Stopwatch.h"
//#include "stopwatch.h"
//#include "strings.h"
//#include "types.h"
#include "scripting.h"

int main() {
#if 1
    while (1) {
        GL::stopwatch sw;
        if (1) {            
            if (auto timer = sw.debug_timer("direct function call w/o converters (unboxed value)\t")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)GL::string("this").size();
                }
            }
        }
        if (1) {
            auto callable = GL::make_callable("size", &GL::string::size);
            std::array<GL::any::fast_any, 1> example{
                 GL::any::fast_any::instance(GL::string("this"))
            };
            if (auto timer = sw.debug_timer("direct function call w/o converters (from boxed value)\t")) {
                for (int i = 0; i < 1000000; ++i) {
                    (void)example[0].cast<GL::string>().size();
                }
            }
        }
        if (0) {
            auto callable = GL::make_callable("size", &GL::string::size);
            std::array<GL::any::fast_any, 1> example{
                 GL::any::fast_any::instance(GL::string("this"))
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
            auto callable = GL::make_callable("size", &GL::string::size);
            if (1) {
                std::array<GL::any::fast_any, 1> example{
                     GL::any::fast_any::instance(GL::string("this"))
                };
                if (auto timer = sw.debug_timer("operator() with callable and w/converters, no conversion needed")) {
                    for (int i = 0; i < 1000000; ++i) {
                        (void)callable->operator()(&example[0], &example[0] + example.size());
                    }
                }
            }
            if (1) {
                std::array<GL::any::fast_any, 2> example{
                     GL::any::fast_any::instance(std::string("this"))
                };
                if (auto timer = sw.debug_timer("operator() with callable and w/converters, conversion needed")) {
                    for (int i = 0; i < 1000000; ++i) {
                        (void)callable->operator()(&example[0], &example[0] + example.size());
                    }
                }
            }
        }
    }   
#endif
    using namespace GL::literals;
    struct F {
        static void ToDo(GL::foot const& i) {
            if (i > 0ull) throw std::runtime_error("e");
        };
        static GL::meter AsyncTest(GL::meter const& i) {
            return i;
        };
    };

    // in parallel, each thread attempts this parallel-tasking test suite
    GL::parallel::While(
        []() -> bool { return true; }, 
        []() {
            // casting is automatic for all of these
            auto task_sequence = GL::parallel::task([]() -> GL::millisecond {
                std::cout << "First...\n";
                return GL::millisecond(0);
            })->and_then([](GL::second t0) -> GL::millisecond {
                std::cout << "Second...\n";
                return t0;
            })->and_then(0, 10'000, [](size_t i, GL::millisecond& t0, GL::job_base& parent) -> GL::any::fast_any {
                t0 += 1;
                return parent.result;
            })->and_then([](GL::any::fast_any const& t0) {
                if (t0.cast<GL::millisecond>() != 10'000_ms) throw "SHOULD HAVE MATCHED";
                std::cout << "Third and done.\n";
            });
            
            GL::value progress = 0;
            auto task_1 = GL::parallel::task(0, 10'000, [&progress]() {
                if ((++progress).mod(1000) == 0) {
                    std::cout << GL::printf("%i percent\n", (int)(100.0f * ((float)progress / 10'000.0f)));
                }
                GL::stopwatch sw; sw.reset();
                while (sw.check() < 0.001) {}
            });

            // casting is automatic for this
            GL::parallel::For(0, 1'000'000, [](size_t i) {});
            // casting is automatic for this
            (void)GL::parallel::async([](double i) { return i; }, 100);
            // no casting required
            GL::parallel::Until(
                []() {},
                []() -> bool { return true; }
            );
            // no casting required
            GL::parallel::While(
                []() -> bool { return false; },
                []() {}
            );
            std::cout << "All jobs submitted.\n";
        }
    );

    for (int j = 0; j < 1'000'000; ++j) {
        auto future1 = GL::parallel::async(&F::AsyncTest, 10_ft);
        if (GL::type_of<GL::meter>() != future1.as_promise().Type()) throw "SHOULD HAVE MATCHED";
        if (future1.get_ref() != 10_ft) throw "SHOULD HAVE MATCHED";

        auto future2 = GL::parallel::async([](GL::millisecond const& t) {
            ::Sleep((long long)t.operator float());
        }, 0.01_s);

        GL::parallel::For(0, -1'000'000, &F::ToDo);
    }
    return 0;
};

