#pragma region "Includes"
#pragma once

#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <thread>
#include <concurrent_unordered_map.h>
#include <stdlib.h>

#include "util.h"
#include "atomic_allocator.h"
#include "atomic_vector.h"
#include "atomic_stack.h"
#include "atomic_queue.h"
#include "atomic_numbers.h"
#include "atomic_maps.h"
#include "stopwatch.h"
#include "strings.h"
#include "atomic_shared_ptr.h"
#include "types.h"
#include "Parallel.h"
#include "shared_ptr.h"
#include "units.h"
#include "datetime.h"
#include "functions.h"
#include "scripting.h"
#include "atomic_tree.h"

#pragma endregion

#pragma region "Definitions"
#define SINGLE_ARG(...) __VA_ARGS__
#define EXPECT_EQ_PRINTF(A,B) [a = (A), b = (B)]()->bool{ \
    if (a == b) { return true; } else { \
        std::string tempA{std::to_string(a)}, tempB{std::to_string(b)}; \
        std::cout << GoodLang::printf("FAILURE AT LINE %i: (%s != %s)\n", (int)__LINE__, tempA.c_str(), tempB.c_str()); \
    return false; } \
}()

#define print(a) std::cout << a << std::endl
struct catcher {
public:
    static bool& allow_print(){ 
        static bool out{ true };
        return out;
    };
    static __declspec(noinline) void CatchMe(long L) {
        if (allow_print()) {
            std::cout << GL::printf("FAILURE AT LINE %i\n", (int)L);
        }
    }
};
#define EXPECT_EQ(a, b) if (a != b){ catcher::CatchMe(__LINE__); }
#define EXPECT_NE(a, b) if (a == b){ catcher::CatchMe(__LINE__); }
#pragma endregion

#include "../GpuProgramming/matrix.h" // Working implimentation of GPU-accelrated matrix
// #include "../ExcelInterop/Wrapper.h"

__forceinline void console_clear() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}

template <typename T, typename U> struct helper : helper<T, decltype(&U::operator())> {};
template <typename T, typename C, typename R, typename... A> struct helper<T, R(C::*)(A...) const> {
    static const bool value = std::is_convertible<T, R(*)(A...)>::value;
};
// successfully tests if a lambda is stateless or not. 
template<typename T> struct is_stateless {
    static const bool value = helper<T, T>::value;
};



int main() {
#if 0
    if (auto wb = cweeExcel::OpenExcel("S:\\Engineering\\Monthly Conservation Report\\Analysis File\\DemandSupplyShortage.xlsx")) {        
        if (auto ws = wb->active_sheet()) {
            print(ws->cell("A2")->value<std::string>());
        }
    }
    if (auto wb = cweeExcel::OpenExcel("S:\\Engineering\\Monthly Conservation Report\\Analysis File\\Demand ProRating\\ProRating Calculator.xlsx")) {        
        for (int sheet_index = 0; sheet_index < wb->sheet_count(); ++sheet_index) {
            if (auto ws = wb->sheet_by_index(sheet_index)) {
                if (auto cell = ws->cell("A1")) {
                    auto str = cell->value<std::string>();
                    print(str);
                }
            }
        }
    }
#endif

#if 0
    //auto func = [](int x) -> double { return x; };
    //typedef decltype(GL::details::detail::function_signature(&std::string::length)) function_header;
    //function_header::

    GL::scope::impl::Functions funcs;
    funcs.add_function(GL::make_converter<GL::foot, GL::meter>());
    funcs.add_function(GL::make_converter<GL::meter, GL::foot>());
    funcs.add_function(GL::make_converter<GL::meter, GL::value>());
    funcs.add_function(GL::make_converter<GL::value, GL::meter>());
    funcs.add_function(GL::make_converter<int, double>());
    funcs.add_function(GL::make_converter<int, float>());
    funcs.add_function(GL::make_converter<int, long>());
    funcs.add_function(GL::make_converter<int const&, int>());
    funcs.add_function(GL::make_callable("type_name", [](GL::any const& any_type) -> GL::string { return any_type.m_casted_type.name(); }));
    funcs.add_function(GL::make_callable("type_name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }));
    funcs.add_function(GL::make_callable("type_of", [](GL::any const& any_type) -> GL::type { return any_type.m_casted_type; }));
    funcs.add_function(GL::decl_func(&std::string::length));
    funcs.add_function(GL::decl_func(&std::string::capacity));
    funcs.add_function(GL::decl_func(&std::string::clear));
    funcs.add_function(GL::decl_func(&std::string::empty));

    funcs.for_each([](GL::Proxy_Function const& f) -> bool {
        print(f->m_signature.display());
        return false;
    });
    funcs.for_each("empty", [](GL::Proxy_Function const& f) -> bool {
        print(f->m_signature.display());
        return false;
    });
    if (1) {
        std::vector < GL::type > types{ GL::type_of<std::string>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::type > types{ GL::type_of<std::string&>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::type > types{ GL::type_of<std::string const&>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_EQ(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::type > types{ GL::type_of<std::string const&>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end()));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end()));
    }
    if (1) {
        std::vector < GL::any > types{ GL::any{ std::string{} } };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::any::fast_any > types{ GL::any{ std::string{} }.fast() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
#endif

#if 0
    if (0) {
        if (GL::stopwatch sw; auto x = sw.debug_timer("std_map 1")) {
            std::map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("std_map 2")) {
            std::map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 0.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000; ++i) {
                map[i] = i;
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 0.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000; ++i) {
                map.insert_fast(i, int{ i });
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 1.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {                
                map[i] = i;
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 1.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 2.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 2.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 3.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 3.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 4.1")) {
            GL::epoch_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 4.2")) {
            GL::epoch_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map.insert_fast(i, int{ i });
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 5")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            for (int i = 0; i < 1000000; ++i) {
                map.erase(i);
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 6")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map.erase(i);
            });
        }
        if (1) { // this feature is only possible with the epoch_map
            GL::epoch_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                auto ref = map.insert(i, (int)i);
                ref.second++;
                map.erase(i);
                ref.second++;
            });
        }

        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 0")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 1")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 1a")) {
            concurrency::concurrent_unordered_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 2")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 3")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 4")) {
            concurrency::concurrent_unordered_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 5")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map.unsafe_erase(i);
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 6")) {
            concurrency::concurrent_unordered_map<int, int> map;
            std::mutex mut;
            for (int i = 0; i < 1000000; ++i) {
                mut.lock();
                map[i] = i;
                mut.unlock();
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                mut.lock();
                map.unsafe_erase(i);
                mut.unlock();
            });
        }
    }
#endif

    std::thread test_thread([&]() {
        GL::parallel::For(0, 1000000, [&](size_t i) {});
        GL::parallel::Std_For(0, 1000000, [&](size_t i) {});

        GL::stopwatch sw;
        GL::stopwatch loop_sw;
        std::ios_base::sync_with_stdio(false);
        std::cin.tie(NULL);
        CONSOLE_SCREEN_BUFFER_INFO screen; GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

        while (true) {
            loop_sw.reset();

            EXPECT_EQ(GL::type_of<size_t>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<size_t>().is_const(), false);
            EXPECT_EQ(GL::type_of<size_t>().is_ref(), false);
            EXPECT_EQ(GL::type_of<size_t>().is_const_ref(), false);

            EXPECT_EQ(GL::type_of<size_t&>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<size_t&>().is_const(), false);
            EXPECT_EQ(GL::type_of<size_t&>().is_ref(), true);
            EXPECT_EQ(GL::type_of<size_t&>().is_const_ref(), false);

            EXPECT_EQ(GL::type_of<const size_t>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<const size_t>().is_const(), true);
            EXPECT_EQ(GL::type_of<const size_t>().is_ref(), false);
            EXPECT_EQ(GL::type_of<const size_t>().is_const_ref(), false);

            EXPECT_EQ(GL::type_of<const size_t&>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<const size_t&>().is_const(), true);
            EXPECT_EQ(GL::type_of<const size_t&>().is_ref(), true);
            EXPECT_EQ(GL::type_of<const size_t&>().is_const_ref(), true);

            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Const).get_hash(), GL::type_of<const size_t>().get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Const | GL::type::Reference).get_hash(), GL::type_of<const size_t&>().get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Temporary).get_hash(), GL::type_of<size_t&&>().get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Const | GL::type::Reference | GL::type::Temporary).get_hash(), (GL::type_of<size_t>() | GL::type::Const | GL::type::Temporary).get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Reference | GL::type::Temporary).get_hash(), (GL::type_of<size_t const&>() | GL::type::Temporary).get_hash());
            EXPECT_EQ(GL::type_of<size_t&&>().is_cpp_type(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_cpp_type());
            EXPECT_EQ(GL::type_of<size_t&&>().is_temp(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_temp());
            EXPECT_EQ(GL::type_of<size_t&&>().is_const(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_const());
            EXPECT_EQ(GL::type_of<size_t&&>().is_ref(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_ref());

            while (1) {
                if (auto timer = sw.debug_timer("1 million scopes with 10 sub-scopes")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    GL::parallel::For(0, 1000000, [&](size_t) {
                        auto a = program_root.make_scope();
                        auto b = a.make_scope();
                        auto c = b.make_scope();
                        auto d = c.make_scope();
                        auto e = d.make_scope();
                        auto f = e.make_scope();
                        auto g = f.make_scope();
                        auto h = g.make_scope();
                        auto i = h.make_scope();
                        auto j = i.make_scope();
                        auto k = j.make_scope();
                    });
                }
                if (auto timer = sw.debug_timer("example calc")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    GL::parallel::For(0, 1000000, [&](size_t i) {
                        auto x0 = program_root.call("foot", { GL::any::fast_any::instance(100.0) });
                        auto v0 = program_root.call("/", {
                            program_root.call("foot", { GL::any::fast_any::instance(10) }),
                            program_root.call("second", { GL::any::fast_any::instance(1) })
                            });
                        auto a0 = program_root.call("/", {
                            v0,
                            program_root.call("second", { GL::any::fast_any::instance(1) })
                            });
                        auto t = program_root.call("second", { GL::any::fast_any::instance(5) });
                        auto d = program_root.call("+", {
                            program_root.call("*", {
                                v0,
                                t
                            }),
                            program_root.call("*", {
                                program_root.call("*", {
                                    program_root.call("pow", {
                                        t,
                                        GL::any::fast_any::instance(2)
                                    }),
                                    a0
                                }),
                                GL::any::fast_any::instance(0.5)
                            })
                            });
                        auto x = program_root.call("+", {
                            x0,
                            d
                            });
                        });
                }
                if (auto timer = sw.debug_timer("example calc 2")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    GL::parallel::For(0, 1000000, [&](size_t i) {
                        auto temp_scope = program_root.make_scope();
                        temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                        temp_scope.insert_object_here("v0", temp_scope.call("/", {
                            temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("a0", temp_scope.call("/", {
                            temp_scope.find_object("v0"),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                        temp_scope.insert_object_here("d", temp_scope.call("+", {
                            temp_scope.call("*", {
                                temp_scope.find_object("v0"),
                                temp_scope.find_object("t")
                            }),
                            temp_scope.call("*", {
                                temp_scope.call("*", {
                                    temp_scope.call("pow", {
                                        temp_scope.find_object("t"),
                                        GL::any::fast_any::instance(2)
                                    }),
                                    temp_scope.find_object("a0")
                                }),
                                GL::any::fast_any::instance(0.5)
                            })
                            }));
                        temp_scope.insert_object_here("x", temp_scope.call("+", {
                            temp_scope.find_object("x0"),
                            temp_scope.find_object("d")
                            }));
                        });
                }
                if (auto timer = sw.debug_timer("example calc (C++ only, for the theoretical 'optimal' performance)")) {
                    GL::parallel::For(0, 1000000, [&](size_t i) {
                        using namespace GL::literals;
                        auto x0 = 100_ft;
                        auto v0 = 10_ft / 1_s;
                        auto a0 = v0 / 1_s;
                        auto t = 5_s;
                        auto d = (v0 * t) + ((t.pow(2) * a0) * 0.5);
                        auto x = x0 + d;
                    });
                }
                if (auto timer = sw.debug_timer("example calc 2 (once only)")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    auto temp_scope = program_root.make_scope();
                    temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                    temp_scope.insert_object_here("v0", temp_scope.call("/", {
                        temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("a0", temp_scope.call("/", {
                        temp_scope.find_object("v0"),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                    temp_scope.insert_object_here("d", temp_scope.call("+", {
                        temp_scope.call("*", {
                            temp_scope.find_object("v0"),
                            temp_scope.find_object("t")
                        }),
                        temp_scope.call("*", {
                            temp_scope.call("*", {
                                temp_scope.call("pow", {
                                    temp_scope.find_object("t"),
                                    GL::any::fast_any::instance(2)
                                }),
                                temp_scope.find_object("a0")
                            }),
                            GL::any::fast_any::instance(0.5)
                        })
                        }));
                    temp_scope.insert_object_here("x", temp_scope.call("+", {
                        temp_scope.find_object("x0"),
                        temp_scope.find_object("d")
                        }));
                }
                if (auto timer = sw.debug_timer("example calc 2 (once only, from scratch)")) {
                    GL::scope::impl::RootScope
                        program;
                    program.perform_builtins();

                    auto temp_scope = program.make_scope();
                    temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                    temp_scope.insert_object_here("v0", temp_scope.call("/", {
                        temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("a0", temp_scope.call("/", {
                        temp_scope.find_object("v0"),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                    temp_scope.insert_object_here("d", temp_scope.call("+", {
                        temp_scope.call("*", {
                            temp_scope.find_object("v0"),
                            temp_scope.find_object("t")
                        }),
                        temp_scope.call("*", {
                            temp_scope.call("*", {
                                temp_scope.call("pow", {
                                    temp_scope.find_object("t"),
                                    GL::any::fast_any::instance(2)
                                }),
                                temp_scope.find_object("a0")
                            }),
                            GL::any::fast_any::instance(0.5)
                        })
                        }));
                    temp_scope.insert_object_here("x", temp_scope.call("+", {
                        temp_scope.find_object("x0"),
                        temp_scope.find_object("d")
                        }));
                }
                if (auto timer = sw.debug_timer("example calc 2 (sequence, not parallel)"); false) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    for (size_t i = 0; i < 1000000; ++i) {
                        auto temp_scope = program_root.make_scope();
                        temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                        temp_scope.insert_object_here("v0", temp_scope.call("/", {
                            temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("a0", temp_scope.call("/", {
                            temp_scope.find_object("v0"),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                        temp_scope.insert_object_here("d", temp_scope.call("+", {
                            temp_scope.call("*", {
                                temp_scope.find_object("v0"),
                                temp_scope.find_object("t")
                            }),
                            temp_scope.call("*", {
                                temp_scope.call("*", {
                                    temp_scope.call("pow", {
                                        temp_scope.find_object("t"),
                                        GL::any::fast_any::instance(2)
                                    }),
                                    temp_scope.find_object("a0")
                                }),
                                GL::any::fast_any::instance(0.5)
                            })
                            }));
                        temp_scope.insert_object_here("x", temp_scope.call("+", {
                            temp_scope.find_object("x0"),
                            temp_scope.find_object("d")
                            }));
                    };
                }
                if (auto timer = sw.debug_timer("Polymorphism test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    // declare a custom namespace
                    auto& Example = root.make_namespace("Example");

                    // within that namespace is an Animal interface class
                    auto& Animal = Example.make_class("Animal");
                    auto Animal_t = Animal.this_type;
                    Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));

                    // within that namespace is an Dog impl class
                    auto& Dog = Example.make_class("Dog");
                    auto Dog_t = Dog.this_type;
                    Dog_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                    Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));

                    // within that namespace is an Cat impl class
                    auto& Cat = Example.make_class("Cat");
                    auto Cat_t = Cat.this_type;
                    Cat_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                    Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));

                    if (1) {
                        auto script_scope = root.make_scope();

                        if (1) {
                            GL::any::fast_any dog_impl = GL::any::fast_any::instance(10);
                            dog_impl.m_casted_type = Dog_t;
                            script_scope.insert_object_here("dog_impl", dog_impl);
                        }
                        if (1) {
                            GL::any::fast_any cat_impl = GL::any::fast_any::instance(10);
                            cat_impl.m_casted_type = Cat_t;
                            script_scope.insert_object_here("cat_impl", cat_impl);
                        }
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.find_object("cat_impl") }));
                    }
                }
                if (auto timer = sw.debug_timer("Var tests")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    if (auto script_scope = root.make_scope()) {
                        GL::any::fast_any x = GL::any::fast_any::instance(100);
                        GL::any::fast_any y = GL::any::fast_any::instance(100);

                        script_scope.call("+=", { x, y });
                        EXPECT_EQ(x.cast<int>(), 200);
                    }

                    if (auto script_scope = root.make_scope()) {
                        GL::var x = GL::var(GL::make_shared<GL::any>(100));
                        GL::var y = GL::var(GL::make_shared<GL::any>(100));

                        EXPECT_EQ(x.get_type(), GL::type_of<int>());

                        script_scope.call("+=", { x.get_data()->fast(), y.get_data()->fast() });
                        EXPECT_EQ(x.get_data()->cast<int>(), 200);

                        script_scope.call("+=", { x.get_data()->fast(), y.get_data()->fast() });
                        EXPECT_EQ(x.get_data()->cast<int>(), 300);
                    }

                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));

                        script_scope.call("+=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 200);
                    }

                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(0)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));

                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);
                    }

                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(0)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);
                    }
                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>()));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);
                    }
                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(200)));
                        script_scope.insert_object_here("z", GL::var(GL::make_shared<GL::any>()));
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);

                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 200);

                        EXPECT_EQ(true, script_scope.find_object("x").can_cast(GL::type_of<GL::var&>()));

                        script_scope.call(":=", { script_scope.find_object("x"), script_scope.find_object("z") });
                        EXPECT_EQ(script_scope.find_object("x").m_casted_type, GL::type_of<GL::var>());
                    }
                    if (auto script_scope = root.make_scope()) {
                        auto empty_var = script_scope.call("var", {  });
                        auto initialized_var = script_scope.call("var", { GL::any::fast_any::instance(100) });
                        auto copied_var = script_scope.call("var", { initialized_var });
                        auto assigned_var = script_scope.call("=", { script_scope.call("var", {  }), initialized_var });

                        EXPECT_EQ(empty_var.m_casted_type, GL::type_of<GL::var>());
                        EXPECT_EQ(initialized_var.m_casted_type, GL::type_of<int>());
                        EXPECT_EQ(copied_var.m_casted_type, GL::type_of<int>());
                        EXPECT_EQ(assigned_var.m_casted_type, GL::type_of<int&>());
                        EXPECT_EQ(initialized_var.cast<int>(), 100);
                        EXPECT_EQ(copied_var.cast<int>(), 100);
                        EXPECT_EQ(assigned_var.cast<int>(), 100);

                        script_scope.call("+=", { initialized_var, GL::any::fast_any::instance(25) });
                        script_scope.call("+=", { copied_var, GL::any::fast_any::instance(25) });
                        script_scope.call("+=", { assigned_var, GL::any::fast_any::instance(25) });

                        EXPECT_EQ(initialized_var.cast<int>(), 175);
                        EXPECT_EQ(copied_var.cast<int>(), 175);
                        EXPECT_EQ(assigned_var.cast<int>(), 175);

                        // handling type-changes when keeping variables locally...
                        assigned_var = script_scope.call(":=", { assigned_var, GL::any::fast_any::instance(std::string("TEST"))});
                        EXPECT_EQ(assigned_var.cast<std::string>(), "TEST");
                        EXPECT_EQ(script_scope.call< GL::string>("type_name", { assigned_var }), "string");

                        // type-changes are automatically handled when handled 100% in-script. 
                        script_scope.emplace_object_here("x", GL::var(GL::make_shared<GL::any>(100.0f)));
                        EXPECT_EQ(script_scope.call< GL::string>("type_name", { script_scope.find_object("x") }), "float");
                        script_scope.call(":=", { script_scope.find_object("x"), GL::any::fast_any::instance(std::string("TEST")) });
                        EXPECT_EQ(script_scope.call< GL::string>("type_name", { script_scope.find_object("x") }), "string");
                    }
                }
                if (auto timer = sw.debug_timer("Polymorphism var test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    // declare a custom namespace
                    auto& Example = root.make_namespace("Example");

                    // within that namespace is an Animal interface class
                    auto& Animal = Example.make_class("Animal");
                    auto Animal_t = Animal.this_type;
                    Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));

                    // within that namespace is an Dog impl class
                    auto& Dog = Example.make_class("Dog");
                    auto Dog_t = Dog.this_type;
                    Dog_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                    Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));
                    Dog.add_function(GL::make_callable(Dog_t.name(), [Dog_t]() -> GL::any::fast_any {
                        GL::any::fast_any out = GL::any::fast_any::instance(10);
                        out.m_casted_type = Dog_t;
                        return out;
                        }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Dog_t));

                    // within that namespace is an Cat impl class
                    auto& Cat = Example.make_class("Cat");
                    auto Cat_t = Cat.this_type;
                    Cat_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                    Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));
                    Cat.add_function(GL::make_callable(Cat_t.name(), [Cat_t]() -> GL::any::fast_any {
                        GL::any::fast_any out = GL::any::fast_any::instance(10);
                        out.m_casted_type = Cat_t;
                        return out;
                        }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Cat_t));

                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                        script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.call("dog_impl",{}) }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.call("cat_impl",{}) }));

                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                        EXPECT_EQ(script_scope.call<bool>("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }), true);

                        // To-Do, test for polymorphism with the casted-down type, having lost its identity. 
                        //auto found_impl = script_scope.find_object("dog_impl");
                        //found_impl.m_casted_type = Animal_t;
                        //print(script_scope.call<std::string>("speak", { found_impl }));
                    }


                }
                if (auto timer = sw.debug_timer("Polymorphism dynamic_object var test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string>(), GL::type_of<GL::string>(), false));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string>(), GL::type_of<GL::string>(), true));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string>(), false));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string>(), true));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string const&>(), false));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string const&>(), true));

                    // declare a custom namespace
                    auto& Example = root.make_namespace("Example");

                    // class Animal {
                    //      bool is_pet = true;
                    //      value& counter = value(0);
                    //      std::string speak() { return "unspecified"; };
                    // };
                    auto& Animal = Example.make_class("Animal");                
                    auto Animal_t = Animal.this_type;                
                    Animal.add_member_object("is_pet", GL::type_of<bool>(), GL::any::fast_any::instance(bool{ true }));                
                    Animal.add_member_object("counter", GL::type_of<GL::value&>(), /*Example.call("reference_cast", { */GL::any::fast_any::instance(GL::value(0.0f)) /*})*/);
                    Animal.initialize_basic_member_functions();                
                    Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t | GL::type::Reference } }, GL::type_of<std::string>()));
                   
                    // class Dog : Animal { // inherits the member objects and functions from Animal
                    //      std::string name = "Ozzy";
                    //      double weight = 24.0;
                    //      std::string speak() { return "bark"; };
                    // };
                    auto& Dog = Example.make_class("Dog");
                    auto Dog_t = Dog.this_type;
                    Dog_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                    Dog.add_member_object("name", GL::type_of<std::string>(), GL::any::fast_any::instance(std::string("Ozzy")));
                    Dog.add_member_object("weight", GL::type_of<double>(), GL::any::fast_any::instance(24.0));                
                    Dog.initialize_basic_member_functions();
                    Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t | GL::type::Reference } }, GL::type_of<std::string>()));

                    // class Cat : Animal { // inherits the member objects and functions from Animal
                    //      std::string name = "Goosie";
                    //      std::string speak() { return "meow"; }; 
                    // };
                    auto& Cat = Example.make_class("Cat");
                    auto Cat_t = Cat.this_type;
                    Cat_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                    Cat.add_member_object("name", GL::type_of<std::string>(), GL::any::fast_any::instance(std::string("Goosie")));
                    Cat.initialize_basic_member_functions();
                    Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t | GL::type::Reference } }, GL::type_of<std::string>()));

                    // class Lion : Cat, Dog { // inherits the member objects and functions from Cat, Dog, and (implied) Animal. Order matters with inheritance. 
                    //      std::string speak() { return "MEOW"; }; 
                    // };
                    auto& Lion = Example.make_class("Lion");
                    auto Lion_t = Lion.this_type;
                    Lion_t.add_base(Cat_t);
                    Lion_t.add_base(Dog_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Lion_t));
                    EXPECT_EQ(true, Dog_t.is_base_of(Lion_t));
                    EXPECT_EQ(true, Cat_t.is_base_of(Lion_t));
                    Lion.initialize_basic_member_functions();
                    Lion.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "MEOW"; }, 0, {}, { { "rhs", Lion_t | GL::type::Reference } }, GL::type_of<std::string>()));

                    // normal
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                        script_scope.insert_object_here("cat_impl", script_scope.call("Cat", {}));
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ(4, script_scope.call<size_t>("length", { script_scope.call("name", {script_scope.find_object("dog_impl")}) }));
                        EXPECT_EQ("Ozzy", script_scope.call<std::string&>("name", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("Goosie", script_scope.call<std::string&>("name", { script_scope.find_object("cat_impl") }));
                        EXPECT_EQ(true, script_scope.call<bool>("is_pet", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_impl") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_impl") }));

                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                        EXPECT_EQ(script_scope.call<bool>("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }), true);

                        script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any rhs) {
                            auto temp_scope = NearestNS->make_scope();
                            return temp_scope.call("speak", { rhs });
                        }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("dog_impl") }), "bark");
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("cat_impl") }), "meow");
                    }

                    // as `var`
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                        script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ("Ozzy", script_scope.call<std::string&>("name", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("Goosie", script_scope.call<std::string&>("name", { script_scope.find_object("cat_impl") }));
                        EXPECT_EQ(true, script_scope.call<bool>("is_pet", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_impl") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_impl") }));

                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                        EXPECT_EQ(script_scope.call<bool>("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }), true);

                        script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any rhs) {
                            auto temp_scope = NearestNS->make_scope();
                            return temp_scope.call("speak", { rhs });
                        }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("dog_impl") }), "bark");
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("cat_impl") }), "meow");
                    }

                    // test assignment and copy constructors
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_1", script_scope.call("Dog", {}));
                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_1") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_1") }), GL::any::fast_any::instance(100.0) });
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_1") }));

                        script_scope.insert_object_here("dog_2", script_scope.call("Dog", {}));
                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_2") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_2") }), GL::any::fast_any::instance(200.0) });
                        EXPECT_EQ(200.0, script_scope.call<double>("weight", { script_scope.find_object("dog_2") }));

                        script_scope.insert_object_here("dog_3", script_scope.call("Dog", { script_scope.find_object("dog_1") }));
                        EXPECT_EQ(100.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_3") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_3") }), GL::any::fast_any::instance(300.0) });
                        EXPECT_EQ(300.0, script_scope.call<double>("weight", { script_scope.find_object("dog_3") }));
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_1") }));

                        script_scope.insert_object_here("dog_4", script_scope.call("Dog", {}));
                        script_scope.call("=", { script_scope.find_object("dog_4"), script_scope.find_object("dog_1") });
                        EXPECT_EQ(100.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_4") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_4") }), GL::any::fast_any::instance(400.0) });
                        EXPECT_EQ(400.0, script_scope.call<double>("weight", { script_scope.find_object("dog_4") }));
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_1") }));
                    }
                
                    // test a reference-type member object...
                    if (1) {
                        int i = 1000000;
                        GL::parallel::For(0, i, [&root](int) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                            script_scope.call("+=", { script_scope.call("counter", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(5) });
                        });
                        if (1) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                            EXPECT_EQ((float)(i * 5), (float)script_scope.call<GL::value>("counter", {script_scope.find_object("dog_impl")}));
                        }
                    };

                    // test the double-inheritor of Lion...
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("lion_impl", script_scope.call("Example::Lion", {}));
                        EXPECT_EQ("Goosie", script_scope.call<std::string&>("name", { script_scope.find_object("lion_impl") }));
                        EXPECT_EQ("MEOW", script_scope.call<std::string>("speak", { script_scope.find_object("lion_impl") }));
                        EXPECT_EQ(true, script_scope.call<bool>("is_pet", { script_scope.find_object("lion_impl") }));

                    }
                }
                if (auto timer = sw.debug_timer("Templated var test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    // declare a custom namespace
                    auto& Shapes = root.make_namespace("Shapes"); {
                        auto& Circle = Shapes.make_class("Circle"); {
                            Circle.add_member_object("radius", GL::type_of<GL::foot>());
                            Circle.add_function(GL::make_callable("area", [&Circle](GL::any::fast_any lhs) -> GL::any::fast_any {
                                auto scope = Circle.GetRoot()->make_scope();
                                return scope.call("square_foot", { scope.call("*", {scope.call("double", { scope.call("constants::pi", {}) }), scope.call("pow", {scope.call("radius", {lhs}), GL::any::fast_any::instance(2)})}) });
                            }, GL::function_signature::Constant, {}, { { "lhs", Circle.this_type + GL::type::Const + GL::type::Reference } }, GL::type_of<GL::square_foot>()));
                            Circle.initialize_basic_member_functions();
                        }
                        auto& Square = Shapes.make_class("Square"); {
                            Square.add_member_object("side", GL::type_of<GL::foot>());
                            Square.add_function(GL::make_callable("area", [&Square](GL::any::fast_any lhs) -> GL::any::fast_any {
                                auto scope = Square.GetRoot()->make_scope();
                                return scope.call("square_foot", { scope.call("pow", {scope.call("side", {lhs}), GL::any::fast_any::instance(2)}) });
                            }, GL::function_signature::Constant, {}, { { "lhs", Square.this_type + GL::type::Const + GL::type::Reference } }, GL::type_of<GL::square_foot>()));
                            Square.initialize_basic_member_functions();
                        }
                        auto& Rectangle = Shapes.make_class("Rectangle"); {
                            Rectangle.add_member_object("width", GL::type_of<GL::foot>());
                            Rectangle.add_member_object("height", GL::type_of<GL::foot>());
                            Rectangle.add_function(GL::make_callable("area", [&Rectangle](GL::any::fast_any lhs) -> GL::any::fast_any {
                                auto scope = Rectangle.GetRoot()->make_scope();
                                return scope.call("square_foot", { scope.call("*", { scope.call("width", {lhs}), scope.call("height", {lhs}) }) });
                            }, GL::function_signature::Constant, {}, { { "lhs", Rectangle.this_type + GL::type::Const + GL::type::Reference } }, GL::type_of<GL::square_foot>()));
                            Rectangle.initialize_basic_member_functions();
                        }
                    }

                    auto& Shape = root.make_class("Shape"); {
                        Shape.template_types = { { "Which", GL::type_of<GL::template_parameter<0>>()} };
                        Shape.add_function(GL::make_callable("area", [&Shape](GL::any::fast_any lhs) -> GL::any::fast_any {
                            auto scope = Shape.GetRoot()->make_scope();
                            return scope.call("area", { lhs });
                        }, GL::function_signature::Static, {}, { { "lhs", GL::type_of<GL::template_parameter<0>>() + GL::type::Const + GL::type::Reference }}, GL::type_of<GL::square_foot>()));
                        Shape.initialize_basic_member_functions();
                    }

                    auto Cir = root.call("Shapes::Circle", {});
                    root.call("=", { root.call("radius", {Cir}), GL::any::fast_any::instance(1) });
                    EXPECT_EQ(root.call<GL::square_foot>("Shape<Shapes::Circle>::area", { Cir }), GL::square_foot((float)GL::constants::pi()));
                }
                if (auto timer = sw.debug_timer("Competing Class Name(s) test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    EXPECT_EQ(root.call("string", {}).m_casted_type, GL::type_of<GL::string>());
                    if (auto& std_ns = root.make_namespace("std")) {
                        EXPECT_EQ(std_ns.call("string", {}).m_casted_type, GL::type_of<std::string>());
                        if (auto scope = std_ns.make_scope()) {
                            EXPECT_EQ(scope.call("string", {}).m_casted_type, GL::type_of<std::string>());
                        }
                    }
                    EXPECT_EQ(root.call("std::string", {}).m_casted_type, GL::type_of<std::string>());
                    if (auto scope = root.make_scope()) {
                        EXPECT_EQ(scope.call("string", {}).m_casted_type, GL::type_of<GL::string>());
                        if (auto& std_ns = root.make_namespace("std")) {
                            scope.add_using_here(std_ns);
                            EXPECT_EQ(scope.call("string", {}).m_casted_type, GL::type_of<std::string>());
                            if (auto scope2 = scope.make_scope()) {
                                EXPECT_EQ(scope2.call("string", {}).m_casted_type, GL::type_of<std::string>());
                            }
                        }
                    }

                    EXPECT_EQ(root.find_object("string::npos").cast<size_t>(), GL::string::npos);
                    EXPECT_EQ(root.find_object("::string::npos").cast<size_t>(), GL::string::npos);
                    if (auto& std_ns = root.make_namespace("std")) {
                        try {
                            // will crash, since it searches "string" for the object but fails to find it. 
                            EXPECT_EQ(std_ns.find_object("string::npos").cast<size_t>(), GL::string::npos);
                        }
                        catch (...) {}
                        EXPECT_EQ(std_ns.find_object("::string::npos").cast<size_t>(), GL::string::npos);
                    }

                    if (auto& std_ns = root.make_namespace("std")) {
                        // std::pair<T0,T1>
                        if (1) {
                            auto& BaseClass = std_ns.make_class("pair");
                            BaseClass.template_types = { { "",GL::type_of<GL::template_parameter<0>>() }, { "",GL::type_of<GL::template_parameter<1>>() } };
                            BaseClass.add_member_object("first", GL::type_of<GL::template_parameter<0>>());
                            BaseClass.add_member_object("second", GL::type_of<GL::template_parameter<1>>());
                            BaseClass.initialize_basic_member_functions();
                        }
                        EXPECT_EQ("::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(root.call("pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                        EXPECT_EQ("::std::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(std_ns.call("pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                        EXPECT_EQ("::std::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(root.call("std::pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                        EXPECT_EQ("::std::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(std_ns.call("std::pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                    }

                }
                if (auto timer = sw.debug_timer("Template classes")) {
                    GL::scope::impl::RootScope 
                        root;
                    root.perform_builtins();

                    if (1) {
                        // ensure that a bad request doesn't hang
                        if (auto this_scope = root.make_scope()) {
                            EXPECT_NE(this_scope.DetermineType("pair"), GL::type_of<GL::undefined>());
                            EXPECT_NE(this_scope.DetermineType("pair<int, int>"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair<int, >"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair<int, "), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair<int, int"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair int, int"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair::int,::int"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair +="), GL::type_of<GL::undefined>());
                        }
                    }
                    
                    // pair<T0, T1> template class
                    if (1) {
                        if (auto this_scope = root.make_scope()) {
                            auto Pair = this_scope.call("pair<int, string>", {});
                            auto first = this_scope.call("first", { Pair });
                            auto second = this_scope.call("second", { Pair });
                            EXPECT_EQ(false, Pair.m_casted_type.is_cpp_type());
                            EXPECT_EQ(true, first.m_casted_type.is_cpp_type());
                            EXPECT_EQ(true, second.m_casted_type.is_cpp_type());
                            print(first.m_casted_type.name());
                            print(second.m_casted_type.name());
                        }
                        if (auto this_scope = root.make_scope()) {
                            auto Pair = this_scope.call("pair<int, pair<int, pair<int, string>>>", {});
                            auto first = this_scope.call("first", { Pair });
                            auto second = this_scope.call("second", { Pair });
                            EXPECT_EQ(false, Pair.m_casted_type.is_cpp_type());
                            EXPECT_EQ(true, first.m_casted_type.is_cpp_type());
                            EXPECT_EQ(false, second.m_casted_type.is_cpp_type());
                            print(first.m_casted_type.name());
                            print(second.m_casted_type.name());
                        }
                        if (auto this_scope = root.make_scope()) {
                            auto Pair = this_scope.call("pair<vector<int>,pair<int,vector<int>>>", {});
                            auto first = this_scope.call("first", { Pair });
                            auto second = this_scope.call("second", { Pair });
                            EXPECT_EQ(false, Pair.m_casted_type.is_cpp_type());
                            EXPECT_EQ(false, first.m_casted_type.is_cpp_type());
                            EXPECT_EQ(false, second.m_casted_type.is_cpp_type());
                            print(first.m_casted_type.name());
                            print(second.m_casted_type.name());
                        }
                    }

                    // vector<T0> template class
                    if (1) { 
                        /*
                        auto vec = vector<int>();
                        vec[0] = 10;
                        */
                        if (auto this_scope = root.make_scope()) { 
                            auto vec = this_scope.call("vector<int>", {}); // calling this forcefully initializes the template class, even if it was never initialized before.
                            // EXPECT_EQ(vec.m_casted_type, Class.this_type);

                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(0) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(1) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(2) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(3) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(4) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(5) });
                        
                            print(this_scope.call<size_t>("size", { vec }));
                            print(this_scope.call<int>("[]", { vec, GL::any::fast_any::instance(5) }));
                        
                            print(this_scope.call<GL::string>("to_string", { vec }));
                            print(this_scope.call<size_t>("to_hash", { vec }));

                            this_scope.call("=", { this_scope.call("[]", { vec, GL::any::fast_any::instance(5) }), GL::any::fast_any::instance(50) });
                            print(this_scope.call<int>("[]", { vec, GL::any::fast_any::instance(5) }));
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            this_scope.call("grow_to_at_least", { vec, GL::any::fast_any::instance(10) });
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            for (
                                auto iterator = this_scope.call("begin", { vec }), end = this_scope.call("end", { vec }); 
                                this_scope.call<bool>("!=", { iterator, end }); 
                                this_scope.call("++", { iterator })) 
                            {
                                print(this_scope.call<GL::string>("to_string", { this_scope.call("get", { iterator }) }));
                            }



                            auto iterator = this_scope.call("begin", { vec });
                            print(iterator.m_casted_type.name());
                            print(this_scope.call<GL::string>("to_string", { iterator }));
                            this_scope.call("++", { iterator });
                            print(this_scope.call<GL::string>("to_string", { iterator }));
                            print(this_scope.call<GL::string>("to_string", { this_scope.call("get", { iterator }) }));



                        }
                        if (auto this_scope = root.make_scope()) {                        
                            auto vec = this_scope.call("vector< :: foot>", {}); // calling this forcefully initializes the template class, even if it was never initialized before.
                            // EXPECT_EQ(vec.m_casted_type, Class.this_type);

                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(0) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(1) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(2) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(3) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(4) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(5) });

                            print(this_scope.call<size_t>("size", { vec }));
                            print(this_scope.call<GL::foot>("[]", { vec, GL::any::fast_any::instance(5) }));

                            print(this_scope.call<GL::string>("to_string", { vec }));
                            print(this_scope.call<size_t>("to_hash", { vec }));

                            this_scope.call("=", { this_scope.call("[]", { vec, GL::any::fast_any::instance(5) }), GL::any::fast_any::instance(50) });
                            print(this_scope.call<GL::foot>("[]", { vec, GL::any::fast_any::instance(5) }));
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            this_scope.call("grow_to_at_least", { vec, GL::any::fast_any::instance(10) });
                            print(this_scope.call<GL::string>("to_string", { vec }));
                        }
                        if (auto this_scope = root.make_scope()) {
                            auto vec = this_scope.call("vector<var>", {}); // calling this forcefully initializes the template class, even if it was never initialized before.
                            // EXPECT_EQ(vec.m_casted_type, Class.this_type);

                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(0) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(1) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(2) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(3) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(4) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(5) });

                            print(this_scope.call<size_t>("size", { vec }));
                            print(this_scope.call<GL::string>("to_string", { vec }));
                            print(this_scope.call<size_t>("to_hash", { vec }));

                            this_scope.call("+=", { this_scope.call("[]", { vec, GL::any::fast_any::instance(5) }), GL::any::fast_any::instance(50) });
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            this_scope.call("grow_to_at_least", { vec, GL::any::fast_any::instance(10) });
                            print(this_scope.call<GL::string>("to_string", { vec }));
                        }
                    }

                    // map<T0, T1> template class
                    if (1) {                         
                        // At no point does C++ code instantiate "map<int,value>" -- this happens automatically by even attempting to use or search for it.
                        if (1) {
                            auto vec = root.call("map<int,value>", {});
                            GL::parallel::For(0, 1000000, [&root, &vec](int i) {
                                if (auto this_scope = root.make_scope()) {
                                    auto vec_obj = this_scope.call("[]", { vec, GL::any::fast_any::instance(i % 100) }); // creates a GL::value in the map and returns it
                                    EXPECT_EQ(vec_obj.m_casted_type, GL::type_of<GL::value&>());
                                    this_scope.call("=", { vec_obj, GL::any::fast_any::instance(i % 100) });

                                    auto vec_obj_2 = this_scope.call("[]", { vec, GL::any::fast_any::instance(i % 100) });
                                    EXPECT_EQ(true, this_scope.call<bool>("==", { vec_obj_2, GL::any::fast_any::instance(i % 100) }));
                                }
                            });
                            print(root.call<GL::string>("to_string", { vec }));
                            print(root.call<size_t>("to_hash", { vec }));


                            for (
                                auto iterator = root.call("begin", { vec }), end = root.call("end", { vec });
                                root.call<bool>("!=", { iterator, end });
                                root.call("++", { iterator }))
                            {
                                print(root.call<GL::string>("to_string", { root.call("get", { iterator }) }));
                            }


                        }
                        // Shockingly, map<var,var> worked flawlessly right out of the gate. 
                        // This includes even calling to_string and to_hash on the entire map! Very cool. 
                        if (1) {
                            auto vec = root.call("map<var,var>", {});
                            try{
                                GL::parallel::For(0, 1000000, [&root, &vec](int i) {
                                    if (auto this_scope = root.make_scope()) {
                                        auto vec_obj = this_scope.call("[]", { vec, GL::any::fast_any::instance(GL::foot((float)(i % 100))) }); // creates a GL::value in the map and returns it
                                        switch (i % 3) {
                                        case 0:
                                            this_scope.call("=", { vec_obj, GL::any::fast_any::instance(GL::foot((float)(i % 100))) });
                                            break;
                                        case 1:
                                            this_scope.call("=", { vec_obj, GL::any::fast_any::instance(GL::meter((float)(i % 100))) });
                                            break;
                                        case 2:
                                            this_scope.call("=", { vec_obj, GL::any::fast_any::instance(GL::inch((float)(i % 100))) });
                                            break;
                                        }
                                    }
                                });
                            }
                            catch (std::exception& e) {
                                print(e.what());
                            }
                            print(root.call<GL::string>("to_string", { vec }));
                            print(root.call<size_t>("to_hash", { vec }));

                            for (
                                auto iterator = root.call("begin", { vec }), end = root.call("end", { vec });
                                root.call<bool>("!=", { iterator, end });
                                root.call("++", { iterator }))
                            {
                                print(root.call<GL::string>("to_string", { root.call("get", { iterator }) }));
                            }
                        }

                    }

                    // test<T0,T1>::make_pair() -> pair<T0,T1> // should automatically "figure out" that it should return a pair with the updated type information
                    if (1) {
                        auto& BaseClass = root.make_class("test");
                        BaseClass.template_types = { { "T0", GL::type_of<GL::template_parameter<0>>() }, { "T1", GL::type_of<GL::template_parameter<1>>() } };
                        BaseClass.add_member_object("my_pair", BaseClass.DetermineType("pair<T0,T1>"));
                        BaseClass.add_member_object("another_pair", BaseClass.DetermineType("pair<T0,vector<int>>"));
                        BaseClass.add_member_object("yet_another_pair", BaseClass.DetermineType("pair<T0,vector<T1>>"));
                        BaseClass.add_function(GL::make_callable("make_pair", [](GL::any::fast_any rhs) -> GL::any::fast_any {
                            if (auto* Class = GL::scope::GetClass(rhs.m_casted_type)) {
                                return Class->call("pair<T0,T1>", {});
                            }
                        }));
                        BaseClass.initialize_basic_member_functions();

                        auto Pair = root.call("test<int, double>", {});
                        for (auto& x : dynamic_cast<GL::scope::impl::ClassScope*>(root.try_find_class(Pair.m_casted_type)->this_m.scope)->template_types) {
                            print(x.first);
                            print(x.second.name());
                        }

                        dynamic_cast<GL::scope::impl::ClassScope*>(root.try_find_class(Pair.m_casted_type)->this_m.scope)->DetermineType("pair<T0,T1>");
                        print(root.call<GL::string>("to_string", { root.call("make_pair", { Pair }) }));
                    }




                    print(root.DetermineType("{0}").name());
                    print(root.DetermineType("pair<{0}, {1}>").name());
                }
                if (auto timer = sw.debug_timer("GPU matrix test"); true) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    //GL::parallel::For(0, 1000000, [&root](int) {
                        /*
                        auto state = float_matrix::random(20, 20, 1) > 0.4; // will be a uint_matrix
                        auto kernel = float_matrix::constant(1.0f, 3, 3, 1); // will be a float_matrix
                        kernel.write()[4] = 0; // ownership of the writer is guarranteed to follow with the float& accessor
                        for (;;) {
                            auto nHood = float_matrix(state).convolve(kernel);
                            auto C0 = (nHood == 2);
                            auto C1 = (nHood == 3);
                            state *= C0.cast<unsigned int>();
                            state += C1.cast<unsigned int>();
                            print(state.ASCII());
                        }
                        */
                        if (auto this_scope = root.make_scope()) {
                            GL::stopwatch sw;
                            std::deque<float> framerates;
                            std::ios_base::sync_with_stdio(false);
                            std::cin.tie(NULL);
                            CONSOLE_SCREEN_BUFFER_INFO screen; GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
                            CONSOLE_CURSOR_INFO cursorInfo;
                            GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
                            cursorInfo.bVisible = false;
                            SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
                            int game_w = screen.dwSize.X / 2, game_h = screen.dwSize.Y - 3;

                            this_scope.emplace_object_here("state", this_scope.call(">", { this_scope.call("float_matrix::random", { GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) }), GL::any::fast_any::instance(0.4) }));

                            auto kernel = this_scope.call("float_matrix::constant", { GL::any::fast_any::instance(1.0f), GL::any::fast_any::instance(3), GL::any::fast_any::instance(3), GL::any::fast_any::instance(1) });
                            this_scope.call("=", { this_scope.call("[]", { this_scope.call("write", { kernel }), GL::any::fast_any::instance(4) }), GL::any::fast_any::instance(0.0f) });
                        
                            for (;;) {
                                GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
                                int game_w2 = (screen.dwSize.X / 2), game_h2 = ((screen.dwSize.Y > 3) ? screen.dwSize.Y - 3 : 1);
                                if (game_w2 != game_w || game_h != game_h2) {
                                    game_w = game_w2;
                                    game_h = game_h2;

                                    this_scope.call("=", { this_scope.find_object("state"), this_scope.call("resize", { this_scope.find_object("state"), GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) }) });
                                    framerates.clear();
                                }

                                sw.reset();
                                if (auto for_scope = this_scope.make_scope()) {
                                    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
                                    for_scope.emplace_object_here("nHood", for_scope.call("convolve", { for_scope.call("float_matrix", { for_scope.find_object("state") }), kernel }));
                                    for_scope.emplace_object_here("C0", for_scope.call("==", { for_scope.find_object("nHood"), GL::any::fast_any::instance(2) }));
                                    for_scope.emplace_object_here("C1", for_scope.call("==", { for_scope.find_object("nHood"), GL::any::fast_any::instance(3) }));
                                    for_scope.call("*=", { for_scope.find_object("state"), for_scope.find_object("C0") });
                                    for_scope.call("+=", { for_scope.find_object("state"), for_scope.find_object("C1") });
                                    print(this_scope.call<GL::string>("to_string", { for_scope.call("ASCII", { for_scope.find_object("state") }) }));

                                    // auto mat = this_scope.call("float_matrix::random", { GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) });
                                    // this_scope.emplace_object_here("state", this_scope.call(">", { mat, GL::any::fast_any::instance(0.4) }));

                                    //if (for_scope.call<bool>("<", { for_scope.call("avg", {for_scope.call("float_matrix", {for_scope.find_object("state")})}), GL::any::fast_any::instance(0.1) })) {
                                    //    for_scope.call("+=", { for_scope.find_object("state"), this_scope.call(">", { this_scope.call("float_matrix::random", { GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) }), GL::any::fast_any::instance(0.4) }) });
                                    //}
                                }

                                auto this_frame = (float)(1.0 / sw.stop());
                                framerates.push_back(this_frame);

                                if (framerates.size() > 10000) framerates.pop_front();
                                std::deque<float> copy(framerates);
                                std::sort(copy.begin(), copy.end());
                                float q0 = 0;
                                float q1 = 0;
                                float q2 = 0;
                                float q3 = 0;
                                float q4 = 0;
                                if (copy.size() >= 4) {
                                    q0 = copy.at(0);
                                    q1 = copy.at(copy.size() / 4);
                                    q2 = copy.at(2 * copy.size() / 4);
                                    q3 = copy.at(3 * copy.size() / 4);
                                    q4 = copy.at(copy.size() - 1);
                                }

                                print(GL::printf("min{ %f }  q1{ %f }  median{ %f }  q2{ %f }  max{ %f }  ", q0, q1, q2, q3, q4) + GL::arena_memory_pool::debug() + "         \t");

                                std::cout << std::flush;
                                while (sw.stop() < (1.0 / 60.0)) {
                                    std::this_thread::yield();
                                }
                            }

                        }
                    //});
                }

                // At this point, it is a full replacement of the "Source.cpp" file and its content. It not only re-impliments everything in there, but appears to be faster, principally leak-free, and easier to use.
                // Some very big wins include:
                //  - the type system being atomic and supporting scripted types as well as C++ types, 
                //  - the units system being smaller & faster to compile, being atomic, and supporting pre-compiled as well as runtime typing, 
                //  - the atomic_shared_ptr being a working version of std::atomic<shared_ptr>, 
                //  - the lock-free fundamental types, including trees, maps, and vectors,
                //  - the parallel CPU computing tools, which support rapid and easy deployment of hundreds to millions of parallel jobs,
                //  - the parallel GPU computing tools, which are still in their infancy and require additional work, but exist as an excellent proof-of-concept,
                //  - the scope system, which supports classes, namespaces, and local scopes, and allows for: 
                //      -  objects, functions, static objects, namespaces and classes, template classes, template functions, dynamic and static typing (with automatic shortest-path type-casting including for const-ness, reference-ness, and more), and polymorphism.
                if (auto timer = sw.debug_timer("Examples"); true) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();

                    // local objects, destroyed when out-of-scope
                    if (auto scope = program_root.make_scope()) {
                        scope.insert_object_here("x", 100); // literal
                        scope.insert_object_here("y", GL::make_shared<int>(100)); // GL::shared
                        scope.insert_object_here("z", std::make_shared<int>(100)); // std::shared
                        scope.insert_object_here("w", GL::any::ref(std::string::npos)); // reference to static object
                    }
                    // static objects, destroyed when the root is destroyed.
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.insert_object_here("x", 100); // literal
                        scope.insert_object_here("y", GL::make_shared<int>(100)); // GL::shared
                        scope.insert_object_here("z", std::make_shared<int>(100)); // std::shared
                        scope.insert_object_here("w", GL::any::ref(std::string::npos)); // reference to static object
                    }
                    // functions
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.add_function(GL::make_callable("foo", []() {})); // static lambda function, no return
                        scope.add_function(GL::make_callable("bar", [](int) -> int { return 0; })); // static lambda function, returns
                        scope.add_function(GL::make_callable("zip", [](int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) -> int { return 0; })); // static lambda function, returns, up to 16 input arguments.
                        scope.add_function(GL::make_callable("empty_string", &GL::string::empty_string)); // static class function (only recommended when not overloaded)
                        scope.add_function(GL::make_callable("length", &GL::string::length)); // instanced class function (only recommended when not overloaded)
                        scope.add_function(GL::decl_func(&GL::string::length)); // easy-to-use instanced class function (only recommended when not overloaded)
                        scope.add_function(GL::make_callable("foo", [](int x) -> int { return x + 1; }, { 0 })); // provide default argument values if not provided when called. Defaults are shifted to the end.
                        scope.add_function(GL::make_callable("foo", [](int x, int y) -> int { return x + y; }, { 0 })); // Defaults are shifted to the end. E.g in this example, y is given a default.
                        scope.add_function(GL::make_callable("foo", [](int x, int y, int z) -> int { return x + y + z; }, { 0, 0 })); // Defaults are shifted to the end. E.g in this example, y and z are given defaults.
                        scope.add_function(GL::make_callable("bar", [](int const& rhs) -> int { return 0; }, 0, {}, { { "rhs", GL::type_of<int>() | GL::type::Const | GL::type::Reference } }, GL::type_of<int>())); // you can override the return types and input types, and also provide the argument names...
                        scope.add_function(GL::make_callable("bar", [](GL::any::fast_any const& rhs) -> int { return rhs.cast<int>(); }, 0, {}, { { "rhs", GL::type_of<int>() | GL::type::Const | GL::type::Reference } }, GL::type_of<int>())); // ... which is necessary when creating a function that accepts a non-c++ type or where you want to recieve the "wrapper" for the type. 
                        scope.add_function(GL::make_callable("++", [](GL::any::fast_any const& rhs) -> GL::any::fast_any { ++rhs.cast<int>(); return rhs; }, 0, {}, { { "rhs", GL::type_of<int>() | GL::type::Const | GL::type::Reference } }, GL::type_of<int const&>())); // Does this is also necessary when the intent is to return a reference-type while passing along the wrapper for the object, to ensure the lifetime protection is not lost.                        

                        // note that GL::any is an atomic object, while GL::any::fast_any is non-atomic. fast_any is better for most use-cases, but GL::any is necessary in containers or objects whose underlying value (or type) can change frequently.
                    }
                    // namespaces and classes
                    if (1) {
                        auto& std_namespace = program_root.make_namespace("std");
                        auto& std_string_namespace = program_root.make_namespace("::std::string"); // note that "::" is allowed ahead of any namespace or class specifier, hinting that the search should start at the root.
                        auto& std_string_class = std_namespace.make_class("string"); // note that std_string_class and std_string_namespace will point to the exact same place -- they are the same object!
                        auto& std_string_class_2 = std_namespace.make_class(GL::type_of<std::string>()); // note that std_string_class_2 and std_string_class will point to the exact same place -- they are the same object!
                        // however, the "std" namespace is NOT known from the C++ side -- that must be engrained in the script language through the scope system. 
                        if (auto* BC = program_root.try_find_class(GL::type_of<int>()); BC) {
                            auto& int_class = *dynamic_cast<GL::scope::impl::ClassScope*>(BC->this_m.scope);                            
                        }

                        auto& foobar_class = std_namespace.make_class("foobar"); // creates a class with the following path: "::std::foobar". 
                        foobar_class.add_member_object("int_member", GL::type_of<int>(), GL::any::fast_any::instance(0)); // adds a member object to foobar
                        // note that any members should 100% be added BEFORE initialize_basic_member_functions is called. 
                        foobar_class.initialize_basic_member_functions(); // if the class has never been initialized before, then do so, to give it the basic building-block functions such as: 
                        //                                                   "foobar()", "foobar(foobar const&)", and "foobar& operator=(foobar const&)"                        
                    }
                    // template classes
                    if (1) {
                        auto& Vector_class = program_root.make_class("Vector"); // this is the base class. 
                        Vector_class.template_types = { { "", GL::type_of<GL::template_parameter<0>>()}}; // this action suddenly declares that it is available as a template base to exactly one parameter type.
                        Vector_class.add_member_object("int_member", GL::type_of<int>(), GL::any::fast_any::instance(0)); // this type is always going to be an int, regardless of the template class type.
                        Vector_class.add_member_object("dynamic_member", GL::type_of<GL::template_parameter<0>>()); // this type is dependant on the template class type.
                        Vector_class.insert_object_here("static_member", []() -> GL::any::fast_any { GL::any::fast_any out; out.m_casted_type = GL::type_of<GL::template_parameter<0>>(); return out; }()); // this is a static class object with a dynamic type
                        // note that any members should 100% be added BEFORE initialize_basic_member_functions is called. 
                        Vector_class.initialize_basic_member_functions();

                        // Instancing of template types can be done from anywhere that "searches" for things in the scopes. The following will result in the instantiation of the requested class:
                        program_root.find_namespace("Vector<string>");
                        program_root.DetermineType("Vector<string>");
                        program_root.ParsePossiblyScopedName("Vector<string>"); 
                        program_root.call("Vector<string>", {}); // will return a Vector<string>{}
                        program_root.call("dynamic_member", { program_root.call("Vector<string>", {}) }); // will return a GL::string&
                        program_root.find_object("::Vector<string>::static_member"); // will return a GL::string&
                        program_root.call("::Vector<string>::static_member"); // will return a GL::string&
                    }
                    // template functions
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.add_function(GL::make_callable("zip", [](GL::any::fast_any const& rhs) -> void { print("I was called"); })); // You can also leave the type un-specified to keep the function as a template -- it will accept any type provided to it. 
                        scope.add_function(GL::make_callable("zap", [](GL::any::fast_any const& rhs1, int rhs2, std::shared_ptr<int> rhs3) -> void { print("I was called"); })); // You can mix-and-match types or template arguments as necessary.
                    }
                    // dynamic and static typing
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.cast<int>(GL::any::fast_any::instance(100)); 
                        scope.cast<int&>(GL::any::fast_any::instance(100));
                        scope.cast<int const&>(GL::any::fast_any::instance(100));

                        scope.cast<double>(GL::any::fast_any::instance(100));
                        scope.cast<double&>(GL::any::fast_any::instance(100));
                        scope.cast<double const&>(GL::any::fast_any::instance(100));

                        scope.cast<GL::foot>(GL::any::fast_any::instance(100));
                        scope.cast<GL::foot&>(GL::any::fast_any::instance(100));
                        scope.cast<GL::foot const&>(GL::any::fast_any::instance(100));                        

                        // due to the type system, the following are all different functions and will be looked-up based on cast rules:
                        scope.add_function(GL::make_callable("foo", [](int&) -> int { return 0; })); 
                        scope.add_function(GL::make_callable("bar", [](int const&) -> int { return 0; }));
                        scope.add_function(GL::make_callable("zip", [](GL::any::fast_any rhs) -> int { return 0; }, 0, {}, { { "rhs", GL::type_of<int&&>()}}, GL::type_of<int>()));
                        scope.add_function(GL::make_callable("zap", [](int) -> int { return 0; }));
                    }
                    // polymorphism
                    if (auto scope = program_root.make_scope()) {
                        auto& Example = scope.GetNamespace()->make_namespace("Example");

                        // within that namespace is an Animal interface class
                        auto& Animal = Example.make_class("Animal");
                        auto Animal_t = Animal.this_type;
                        Animal.add_member_object("is_pet", GL::type_of<bool>(), GL::any::fast_any::instance(bool{ true }));
                        Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));                        
                        Animal.initialize_basic_member_functions();

                        // within that namespace is an Dog impl class
                        auto& Dog = Example.make_class("Dog");
                        auto Dog_t = Dog.this_type;
                        Dog_t.add_base(Animal_t); // note that we are specifying "Animal_t" as the base to Dog_t
                        Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));
                        Dog.initialize_basic_member_functions();

                        // within that namespace is an Cat impl class
                        auto& Cat = Example.make_class("Cat");
                        auto Cat_t = Cat.this_type;
                        Cat_t.add_base(Animal_t); // note that we are specifying "Animal_t" as the base to Cat_t
                        Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));
                        Cat.initialize_basic_member_functions();
                    
                        // Cat and Dog inherit from Animal automatically once you specify the base. initialize_basic_member_functions will take care of the rest.                         
                        EXPECT_EQ(program_root.call< std::string >("speak", { program_root.call("::Example::Cat", {}) }), "meow");
                        EXPECT_EQ(program_root.call< bool >("is_pet", { program_root.call("::Example::Dog", {}) }), true);

                        // Cat and Dog will override the "speak" function, even when called in such a way that "forgets" the real static type: 
                        Example.add_function(GL::make_callable("talk_to_animal", [](GL::any::fast_any rhs) -> std::string { return GL::scope::GetCurrentCaller()->GetRoot()->call<std::string>("speak", { rhs }); }, GL::function_signature::Static, {}, { {"rhs", Animal_t | GL::type::Const | GL::type::Reference} }, GL::type_of<std::string>()));
                        EXPECT_EQ(program_root.call<std::string>("::Example::talk_to_animal", { program_root.call("::Example::Cat", {}) }), "meow");
                    }
                }

            }
        }

    });

#if 0
    // Conway's Game of Life, using the GPU. Many times faster than previous approach. From 20-30 fps to 1000-1800 fps. 
    if (1) {
        // reduces the size requirement of the arena memory pool. In exchange though, the largest single allocation is reduced to this same number. Application-dependant decision. 
        // GL::GPU::matrix<float>::maximum_allocation_size() /= 16; // = 1; // /= 16; // 16
        while (0) {
            GL::stopwatch sw;
#if 0
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Allocator (int)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(10000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[10000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * 10000));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Allocator (int)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(10000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[10000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * 10000));
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Random Allocator (int)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(i + 1000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[i + 1000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * (i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Random Allocator (int)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(i + 1000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[i + 1000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * (i + 1000)));
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Allocator (std::string)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[100], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string)*100));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Allocator (std::string)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[100], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string) * 100));
                });
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Random Allocator (std::string)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(i + 1000); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[i + 1000], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string) * (i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Random Allocator (std::string)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(i + 1000); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[i + 1000], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string) * (i + 1000)));
                    });
            }
#endif  
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Allocator (int)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>(10000); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>(10000); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>(10000));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Allocator (int)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>(10000); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>(10000); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>(10000));
                });
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Random Allocator (int)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>((unsigned int)(i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Random Allocator (int)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>((unsigned int)(i + 1000)));
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Allocator (std::string)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>(100); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>(100));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Allocator (std::string)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    //auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    //auto b = GL::arena_memory_pool::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>(100); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>(100));
                });
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Random Allocator (std::string)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>((unsigned int)(i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Random Allocator (std::string)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>((unsigned int)(i + 1000)));
                });
            }
        }

        using namespace GL;
        using namespace GL::GPU;

        std::ios_base::sync_with_stdio(false);
        std::cin.tie(NULL);
        CONSOLE_SCREEN_BUFFER_INFO screen; GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

        int game_w = screen.dwSize.X / 2, game_h = screen.dwSize.Y - 3;
        std::deque<float> framerates;

        // Initialize the kernel array
        //print(GL::arena_memory_pool::debug());
        matrix_kernel<unsigned int> kernel(matrix<unsigned int>::from_vector({
            1, 1, 1,
            1, 0, 1,
            1, 1, 1
        }, 3));
        //print(GL::arena_memory_pool::debug());
        auto state = (matrix<float>::random(game_h, game_w, 1) > 0.4f).cast<unsigned int>();
        //print(GL::arena_memory_pool::debug());

        int frame = 1;
        // Run the game of life
        for (;;) {
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
            int game_w2 = (screen.dwSize.X / 2), game_h2 = ((screen.dwSize.Y > 3) ? screen.dwSize.Y - 3 : 1);
            if (game_w2 != game_w || game_h != game_h2) {
                game_w = game_w2;
                game_h = game_h2;

                auto temp = state.resize(game_h, game_w, 1);
                state = temp;

                framerates.clear();
            }

            GL::stopwatch sw;

            // Convolve aligns the kernel ontop of each pixel, multiplies the neighboring pixels by the kernel, and sums the results. The edges are correctly handled using weighted-balancing on the kernel itself.
            auto nHood = state.convolve(static_matrix_kernel<unsigned int>{ &kernel });

            // Generate conditions for life
            // state == 1 && nHood < 2 ->> state = 0
            // state == 1 && nHood > 3 ->> state = 0
            // else if state == 1 ->> state = 1
            // state == 0 && nHood == 3 ->> state = 1
            auto C0 = (nHood == 2);
            auto C1 = (nHood == 3);

            //auto a0 = (state == 1) && (nHood < 2);  // Die of under population
            //auto a1 = (state > 0) && (C0 || C1);   // Continue to live
            //auto a2 = (state <= 0) && C1;           // Reproduction
            //auto a3 = (state == 1) && (nHood > 3);  // Over-population

            // display = (a0 + a1).join(2, a1 + a2).join(2, a3).cast(ArrayTypes::FLOAT);
            //auto R = a0 * a1;
            //auto G = a1 * a2;
            //auto B = a3;

            // Update state
            state *= C0.cast<unsigned int>();
            state += C1.cast<unsigned int>();

            // console_clear();
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });

            // print(state.ASCII().to_string({}, true));
#if 1
#if 0
            auto statef = state.cast<float>();
            auto blur_1 = statef.convolve(matrix<float>::guassian_kernel(3, 3));
            auto blur_2 = blur_1.convolve(matrix<float>::guassian_kernel(7, 7));
            auto blur_3 = blur_2.convolve(matrix<float>::guassian_kernel(11, 11));
            auto blur_4 = blur_3.convolve(matrix<float>::guassian_kernel(23, 23));
            auto blur_5 = blur_4.convolve(matrix<float>::guassian_kernel(53, 53));
            print((statef + blur_1 + blur_2 + blur_3 + blur_4 + blur_5).ASCII().to_string({}, true));
#else
            //// column position (0 to game_w)
            //auto col_pos = (matrix<float>::linear(0, game_w2 * game_h2, game_h2, game_w2, 1) / (float)game_h2).floor().cast<unsigned int>();

            //// row position (0 to game_h)
            //auto row_pos = (matrix<unsigned int>::linear(0, game_w2 * game_h2, game_h2, game_w2, 1) % game_h2);            
            //// UV coordinates for the screen
            //auto screen_U = col_pos.cast<float>() / (float)game_w2;
            //auto screen_V = row_pos.cast<float>() / (float)game_h2;

            //matrix<float>::test_vector(100);
            //matrix<float>::test_vector(1000);
            //matrix<float>::test_vector(10000);
            //matrix<float>::test_vector(100000);
            //matrix<float>::test_vector(1000000);


            class MatrixImage {
            public:
                std::vector<matrix<float>> mip_maps;
                MatrixImage() : mip_maps{} {};
                MatrixImage(matrix<float>&& srce) : mip_maps{} {
                    calculate_mip_maps(std::move(srce));
                };
                MatrixImage(MatrixImage const&) = delete;
                MatrixImage(MatrixImage&&) = delete;
                MatrixImage& operator=(MatrixImage const&) = delete;
                MatrixImage& operator=(MatrixImage&&) = delete;
                ~MatrixImage() = default;

                matrix<float> debug_display() const {
                    matrix<float> out = mip_maps[0];
                    for (int i = 1; i < mip_maps.size(); ++i) {
                        out = out.join(1, mip_maps[i].resize(mip_maps[0].size(0), mip_maps[i].size(1) + 16, 1));
                    }
                    return out;
                };
                matrix<float> sum() const {
                    std::vector<matrix<float>> mips;
                    matrix<float> out = mip_maps[0];
                    for (int i = 1; i < mip_maps.size(); ++i) {
                        mips.emplace_back(mip_maps[i].resize_stretch(mip_maps[0].size(0), mip_maps[0].size(1), 1));
                        out += mips[mips.size() - 1];
                    }
                    return out;
                };

            private:
                void calculate_mip_maps(matrix<float> && srce) {
                    mip_maps.reserve(32);
                    mip_maps.push_back(std::move(srce));
                    const matrix<float>* current = &mip_maps[0];
                    //auto kernel = matrix<float>::guassian_kernel<13, 13>();
                    while ((current->size(0) > 1) && (current->size(1) > 1)) {
                        //auto blurred = current->convolve(kernel);
                        //mip_maps.push_back(blurred.resize_stretch(std::floorf(((float)blurred.size(0) / 2.0f) + 0.5), std::floorf(((float)blurred.size(1) / 2.0f) + 0.5), 1)); //  current->halfsize<false>());

                        mip_maps.push_back(current->halfsize()); // faster but less accurate
                        current = &mip_maps[mip_maps.size() - 1];
                    }
                };

            };
            MatrixImage img(state.cast<float>());
            print(img.sum().resize_stretch(game_h, game_w, 1).ASCII().to_string({}, true));

            //auto texture_y = (screen_U * (float)I5.size(1)).cast<unsigned int>().min(I5.size(1) - 1);
            //auto texture_x = (screen_V * (float)I5.size(0)).cast<unsigned int>().min(I5.size(0) - 1);
            //auto texture_N = ((texture_y * I5.size(0)) + texture_x).min((I5.size(1) * I5.size(0)) - 1);
            //auto scaled = I5.resample(texture_N);

            //auto A = state.resize(game_h2 / 3, game_w2, 1);
            //auto B = state.cast<float>().convolve(matrix<float>::guassian_kernel(5, 5)).resize(game_h2 / 3, game_w2, 1);
            //auto C = state.halfsize().doublesize().resize(game_h2 / 3, game_w2, 1);
            //auto print_me = A.ASCII().join(0, B.ASCII()).join(0, C.ASCII());
            //print(print_me.to_string({}, true));
#endif
#endif
            // state += ((state > 0).cast<float>().convolve(matrix<float>::guassian_kernel<7,7>()) * (matrix<float>::random(game_h, game_w, 1) >= 0.995f).cast<float>()).cast<unsigned int>();
            // state = state.min(1);

            print("");
            auto this_frame = (float)(1.0 / sw.stop());
            framerates.push_back(this_frame);
           
            if (framerates.size() > 10000) framerates.pop_front();
            std::deque<float> copy(framerates);
            std::sort(copy.begin(), copy.end());
            float q0 = 0;
            float q1 = 0;
            float q2 = 0;
            float q3 = 0;
            float q4 = 0;
            if (copy.size() >= 4) {
                q0 = copy.at(0);
                q1 = copy.at(copy.size() / 4);
                q2 = copy.at(2 * copy.size() / 4);
                q3 = copy.at(3 * copy.size() / 4);
                q4 = copy.at(copy.size() - 1);
            }            

            print(GL::printf("min{ %f }  q1{ %f }  median{ %f }  q2{ %f }  max{ %f }  ", q0, q1, q2, q3, q4) + GL::arena_memory_pool::debug() + "         \t");
            std::cout << std::flush;

            // while (sw.stop() < 1.0 / 60.0) {
                // std::this_thread::yield();
            // }
        }
    }
#endif

    test_thread.join();
    return 0;
};
