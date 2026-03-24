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
#if 0

            // GL::atomic_map is leak-free. No doubt. Issue is that it scales poorly (memory-wise) after 1 thread, and also struggles to perform in multi-threaded cases. 
#if 0
            // atomic_epoch_allocator<std::string> == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_epoch_allocator<std::string>>(), [](size_t i, auto& shared_obj) {
                shared_obj->ProtectCurrentEpoch_Fast(); 
                auto* p = shared_obj->Alloc("TEST");
                shared_obj->Free(p);
            })) {
                allocator = nullptr;
            }

            // GL::deferred<atomic_epoch_allocator<std::string>> == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::deferred<GL::atomic_epoch_allocator<std::string>>>(), [](size_t i, auto& shared_obj) {
                (*shared_obj)->ProtectCurrentEpoch_Fast();
                auto* p = (*shared_obj)->Alloc("TEST");
                (*shared_obj)->Free(p);
            })) {
                allocator = nullptr;
            }

            // GL::atomic_btree<std::string, size_t> (adding only) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_btree<std::string, size_t>>(), [](size_t i, auto& shared_obj) {
                shared_obj->Add("TEST", (size_t)GL::util::rand_fast(1000000));
            })) {
                allocator = nullptr;
            }

            // GL::deferred<GL::atomic_btree<std::string, size_t>> (adding only) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::deferred<GL::atomic_btree<std::string, size_t>>>(), [](size_t i, auto& shared_obj) {
                (*shared_obj)->Add("TEST", (size_t)GL::util::rand_fast(1000000));
            })) {
                allocator = nullptr;
            }

            // GL::atomic_btree<std::string, size_t> (adding THEN removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_btree<std::string, size_t>>(), [](size_t i, auto& shared_obj) {
                shared_obj->Add("TEST", i);
            })) {
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, GL::shared_ptr<GL::atomic_btree<std::string, size_t>>& shared_obj) {
                    if (auto* p = shared_obj->NodeFind(i)) {
                        shared_obj->Remove(p);
                    }                    
                }) = nullptr;
            }
            
            // GL::deferred<GL::atomic_btree<std::string, size_t>> (adding THEN removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::deferred<GL::atomic_btree<std::string, size_t>>>(), [](size_t i, auto& shared_obj) {
                (*shared_obj)->Add("TEST", (size_t)GL::util::rand_fast(1000000));
            })) {
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, GL::shared_ptr<GL::deferred<GL::atomic_btree<std::string, size_t>>>& shared_obj) {
                    if (auto* p = (*shared_obj)->NodeFind(i)) {
                        (*shared_obj)->Remove(p);
                    }
                }) = nullptr;
            }

            // GL::atomic_btree<std::string, size_t> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_btree<std::string, size_t>>(), [](size_t i, GL::shared_ptr<GL::atomic_btree<std::string, size_t>>& shared_obj) {
                shared_obj->Add("TEST", i);
                if (auto* p = shared_obj->NodeFind(i)) {
                    shared_obj->Remove(p);
                }
            })) {
                allocator = nullptr;
            }
            
            // GL::deferred<GL::atomic_btree<std::string, size_t>> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::deferred<GL::atomic_btree<std::string, size_t>>>(), [](size_t i, GL::shared_ptr<GL::deferred<GL::atomic_btree<std::string, size_t>>>& shared_obj) {
                (*shared_obj)->Add("TEST", (size_t)GL::util::rand_fast(1000000));
                if (auto* p = (*shared_obj)->NodeFind(i)) {
                    (*shared_obj)->Remove(p);
                }
            })) {
                allocator = nullptr;
            }

            // GL::atomic_map<std::string, size_t> (adding THEN removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_map<size_t, std::string>>(), [](size_t i, auto& shared_obj) {
                shared_obj->operator[](i);
                
            })) {
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, auto& shared_obj) {
                    shared_obj->erase(i);
                }) = nullptr;
            }

            // GL::deferred<GL::atomic_map<std::string, size_t>> (adding THEN removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::atomic_map<size_t, std::string>>>(), [](size_t i, auto& shared_obj) {
                (*shared_obj)->operator[](i);
            })) {
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, auto& shared_obj) {
                    (*shared_obj)->erase(i);
                }) = nullptr;
            }

            // GL::atomic_map<std::string, size_t> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_map<size_t, std::string>>(), [](size_t i, auto& shared_obj) {
                shared_obj->operator[](i);
                shared_obj->erase(i);
            })) {
                allocator = nullptr;
            }

            // GL::deferred<GL::atomic_map<std::string, size_t>> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::atomic_map<size_t, std::string>>>(), [](size_t i, auto& shared_obj) {
                (*shared_obj)->operator[](i);
                (*shared_obj)->erase(i);
            })) {
                allocator = nullptr;
            }

            // GL::atomic_map<size_t, GL::atomic_map<std::string, size_t>> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_map<size_t, GL::atomic_map<std::string, size_t>>>(), [](size_t i, auto& shared_obj) {
                shared_obj->operator[](i);
                shared_obj->erase(i);
            })) {
                allocator = nullptr;
            }

            // GL::deferred<GL::atomic_map<size_t, GL::atomic_map<std::string, size_t>>> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::atomic_map<size_t, GL::atomic_map<std::string, size_t>>>>(), [](size_t i, auto& shared_obj) {
                (*shared_obj)->operator[](i);
                (*shared_obj)->erase(i);
                })) {
                allocator = nullptr;
            }

            // GL::atomic_map<size_t, GL::deferred<GL::atomic_map<std::string, size_t>>> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<std::string, size_t>>>>(), [](size_t i, auto& shared_obj) {
                shared_obj->operator[](i);
                shared_obj->erase(i);
            })) {
                allocator = nullptr;
            }

            // GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<std::string, size_t>>>> (adding AND removal) == no leak
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<std::string, size_t>>>>>(), [](size_t i, auto& shared_obj) {
                (*shared_obj)->operator[](i);
                (*shared_obj)->erase(i);
            })) {
                allocator = nullptr;
            }
#endif

            // this code snippet demonstrates the poor scaling of GL::atomic_map. :(
#if 0
            // GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<std::string, size_t>>>> (adding AND removal) == 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<size_t, GL::value>>>>>(), [](size_t i, GL::shared_ptr<GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<size_t, GL::value>>>>>& shared_obj) {
                auto index = i % 10;

                auto& map1 = *(*shared_obj);
                map1.ProtectCurrentEpoch_Fast(); // guarrantee the protections during this scope...
                auto& map2 = *map1[i];

                map2[index] = GL::foot((float)index);

                map1.erase(i); // erases while others may be accessing it... 
            })) {
                allocator = nullptr;
            }

            // GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<std::string, size_t>>>> (adding AND removal) == 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<size_t, GL::value>>>>>(), [](size_t i, GL::shared_ptr<GL::deferred<GL::atomic_map<size_t, GL::deferred<GL::atomic_map<size_t, GL::value>>>>>& shared_obj) {
                auto index = i % 10;

                auto& map1 = *(*shared_obj);
                // map1.ProtectCurrentEpoch(); // guarrantee the protections during this scope...
                auto& map2 = *map1[i];

                map2[index] = GL::foot((float)index);

                map1.erase(i); // erases while others may be accessing it... 
                })) {
                allocator = nullptr;
            }
#endif

            // epoch_search_tree scales MUCH better in heavy multi-threaded cases. However, it seems to have a memory leak, which may be releated. :/
            // Also, epoch_search_tree seemed to perform random crashes that were difficult to predict or prevent. 
            //print(__LINE__);
            //// GL::epoch_search_tree<std::string, size_t> (adding ONLY) 
            //if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::epoch_search_tree<std::string, size_t>>(), [](size_t i, GL::shared_ptr<GL::epoch_search_tree<std::string, size_t>>& shared_obj) {
            //    shared_obj->Add("TEST", i);
            //})) {
            //    allocator = nullptr;
            //}



#if 1
            print(__LINE__);
            // GL::deferred<GL::epoch_search_tree<std::string, size_t>> (heavy contention on a parallel creation) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>(), [](size_t i, GL::shared_ptr< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>& shared_obj) {
                (*shared_obj)->operator[](0);
            })) {
                allocator = nullptr;
            }
            print(__LINE__);
            // GL::deferred<GL::epoch_search_tree<std::string, size_t>> (adding ONLY) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>(), [](size_t i, GL::shared_ptr< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>& shared_obj) {
                (*shared_obj)->Add("TEST", i);
            })) {
                allocator = nullptr;
            }
            print(__LINE__);
            // GL::epoch_search_tree<std::string, size_t> (adding THEN removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::epoch_search_tree<std::string, size_t>>(), [](size_t i, GL::shared_ptr<GL::epoch_search_tree<std::string, size_t>>& shared_obj) {
                // auto g = shared_obj->ProtectCurrentEpoch();
                shared_obj->Add("TEST", i);
            })) {                
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, auto& shared_obj) {
                    auto g = shared_obj->ProtectCurrentEpoch();
                    if (auto [node, locking] = shared_obj->NodeFind_ForRemoval(i); node) {
                        shared_obj->Remove(node, locking);
                    }
                }) = nullptr;
            } // crashes here... because allocator died before the guard did??
            print(__LINE__);
            // GL::epoch_search_tree<std::string, size_t> (adding THEN removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::epoch_search_tree<std::string, size_t>>(), [](size_t i, GL::shared_ptr<GL::epoch_search_tree<std::string, size_t>>& shared_obj) {
                shared_obj->Add("TEST", i);
            })) {
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, auto& shared_obj) {
                    auto g = shared_obj->ProtectCurrentEpoch();
                    if (auto [node, locking] = shared_obj->NodeFind_ForRemoval(i); node) {
                        shared_obj->Remove(node, locking);
                    }
                }) = nullptr;
            }
            print(__LINE__);
            // GL::epoch_search_tree<std::string, size_t> (adding THEN removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::epoch_search_tree<std::string, size_t>>(), [](size_t i, GL::shared_ptr<GL::epoch_search_tree<std::string, size_t>>& shared_obj) {
                shared_obj->Add("TEST", i);
            })) {
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, auto& shared_obj) {
                    auto g = shared_obj->ProtectCurrentEpoch();
                    if (auto [node, locking] = shared_obj->NodeFind_ForRemoval(i); node) {
                        shared_obj->Remove(node, locking);
                    }
                }) = nullptr;
            }
            print(__LINE__);
            // GL::deferred<GL::epoch_search_tree<std::string, size_t>> (adding THEN removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>(), [](size_t i, GL::shared_ptr< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>& shared_obj) {
                (*shared_obj)->Add("TEST", i);
            })) {
                GL::parallel::Dispatch(1000000, allocator, [](size_t i, auto& shared_obj) {
                    auto g = (*shared_obj)->ProtectCurrentEpoch();
                    if (auto [node, locking] = (*shared_obj)->NodeFind_ForRemoval(i); node) {
                        (*shared_obj)->Remove(node, locking);
                    }
                }) = nullptr;
            }
            print(__LINE__);
            // GL::epoch_search_tree<std::string, size_t> (adding AND removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::epoch_search_tree<std::string, size_t>>(), [](size_t i, GL::shared_ptr<GL::epoch_search_tree<std::string, size_t>>& shared_obj) {
                auto g = shared_obj->ProtectCurrentEpoch();
                shared_obj->Add("TEST", i);                
                if (auto [node, locking] = shared_obj->NodeFind_ForRemoval(i); node) {
                    shared_obj->Remove(node, locking);
                }
            })) {
                allocator = nullptr;
            }
            print(__LINE__);
            // GL::deferred<GL::epoch_search_tree<std::string, size_t>> (adding AND removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>(), [](size_t i, GL::shared_ptr< GL::deferred<GL::epoch_search_tree<std::string, size_t>>>& shared_obj) {
                auto g = (*shared_obj)->ProtectCurrentEpoch();
                (*shared_obj)->Add("TEST", i);
                if (auto [node, locking] = (*shared_obj)->NodeFind_ForRemoval(i); node) {
                    (*shared_obj)->Remove(node, locking);
                }
            })) {
                allocator = nullptr;
            }

            print(__LINE__);
            // GL::deferred<GL::epoch_search_tree<std::string, size_t>> (adding AND removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::deferred<GL::epoch_search_tree<GL::deferred<GL::epoch_search_tree<std::string, size_t>>, size_t>>>(), [](size_t i, GL::shared_ptr< GL::deferred<GL::epoch_search_tree<GL::deferred<GL::epoch_search_tree<std::string, size_t>>, size_t>>>& shared_obj) {
                auto g = (*shared_obj)->ProtectCurrentEpoch();
                (*shared_obj)->operator[](i % 100)->operator[](i % 100) = "TEST";
                if (auto [node, locking] = (*shared_obj)->NodeFind_ForRemoval(i % 100); node) {
                    (*shared_obj)->Remove(node, locking);
                }
            })) {
                allocator = nullptr;
            }

            print(__LINE__);
            // GL::deferred<GL::epoch_search_tree<std::string, size_t>> (adding AND removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared< GL::epoch_search_tree<GL::epoch_search_tree<std::string, size_t>, size_t>>(), [](size_t i, GL::shared_ptr< GL::epoch_search_tree<GL::epoch_search_tree<std::string, size_t>, size_t>>& shared_obj) {
                auto g = shared_obj->ProtectCurrentEpoch();
                shared_obj->operator[](i % 100)[i % 100] = "TEST";
                if (auto [node, locking] = shared_obj->NodeFind_ForRemoval(i % 100); node) {
                    shared_obj->Remove(node, locking);
                }
            })) {
                allocator = nullptr;
            }

            print(__LINE__);
            // GL::epoch_search_tree<std::string, size_t> (adding AND removal) 
            if (auto allocator = GL::parallel::Dispatch(1000000, GL::make_shared<GL::epoch_search_tree<GL::deferred<GL::epoch_search_tree<std::string, size_t>>, size_t>>(), [](size_t i, GL::shared_ptr<GL::epoch_search_tree<GL::deferred<GL::epoch_search_tree<std::string, size_t>>, size_t>>& shared_obj) {
                auto g = shared_obj->ProtectCurrentEpoch();
                shared_obj->operator[](i % 100)->operator[](i % 100) = "TEST";                
                if (auto [node, locking] = shared_obj->NodeFind_ForRemoval(i % 100); node) {
                    shared_obj->Remove(node, locking);
                }
            })) {
                allocator = nullptr;
            }

#endif

#else
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
#if 0
                if (1) {
                    GL::scope::impl::RootScope
                        program_root;
                    program_root.perform_builtins();                    

                    if (1) {
                        GL::any::fast_any instance;
                        std::vector<GL::any::fast_any> empty_params;
                        instance = program_root.call("char", {});
                        GL::any::fast_any instance_ref = program_root.call("=", { instance, GL::any::fast_any::instance('A') });
                        EXPECT_EQ(instance_ref.cast<char>(), instance.cast<char>());
                        EXPECT_EQ(&instance_ref.cast<char&>(), &instance.cast<char&>());
                    }

                    if (0) {
                        auto From = GL::type_of<double>();
                        GL::atomic_allocator<
                            std::variant<GL::scope::impl::Functions::UniformCostSearchNode, GL::scope::impl::Functions::UniformCostSearchNodeBestPath>
                            , 1024
                        > temp_alloc;
                        auto converters
                            = program_root.constructors.CreateConversionPaths(temp_alloc, From);
                        for (auto& To : converters) {
                            if (To.second) {
                                print(From.name() + " to " + To.first.name() + ": ");

                                std::vector<GL::type> best_path;
                                To.second->bestPath->get(best_path);
                                GL::string t;
                                t = t.add_to_delim(From.name(), "->");
                                for (auto& path : best_path) {
                                    t = t.add_to_delim(path.name(), "->");
                                }
                                print("\t" + t + ": ");

                                auto converter = To.second->bestPath->make_converter(From, program_root.constructors);
                                if (converter) {
                                    print("\t" + converter->m_signature.display());
                                }
                            }
                        }
                    }
                    // print("");


                    if (1) {                        
                        GL::any::fast_any len_ft = program_root.call("foot", { GL::any::fast_any::instance(100.0) });
                        GL::any::fast_any len_m = program_root.call("meter", { GL::any::fast_any::instance(GL::foot(100)) });
                        EXPECT_EQ(true, program_root.call("==", { len_ft, len_m }).cast<bool>());
                    }

                    if (1) {
                        GL::any::fast_any cu_ft = program_root.call("*", { GL::any::fast_any::instance(GL::foot(100)), program_root.call("*", { GL::any::fast_any::instance(GL::foot(100)), GL::any::fast_any::instance(GL::foot(100)) }) });
                        EXPECT_EQ("1000000 cu_ft", program_root.call("to_string", { cu_ft }).cast<GL::string>());
                        EXPECT_EQ(GL::gallon(GL::cubic_foot(1000000)), program_root.call("gallon", { cu_ft }).cast<GL::gallon>());
                    }

                    if (1) {
                        GL::any::fast_any cu_ft = program_root.call("*", { GL::any::fast_any::instance(GL::foot(100)), program_root.call("*", { GL::any::fast_any::instance(GL::foot(100)), GL::any::fast_any::instance(GL::foot(100)) }) });
                        //print(program_root.call("pow", { cu_ft, GL::any(1.0/3.0).fast() }).cast<GL::value>()); // note that it says m^1.0000 instead of m or ft. The cubic-root was imperfect though very, very close.
                    }

                    if (1) {
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
                        //print(program_root.call("to_string", { x }).cast<GL::string>());
                    }
                }

                // demonstrate calling a function from a class-scope implicitely. 
                if (1) {
                    std::vector<GL::any::fast_any> params{ GL::any::fast_any::instance(GL::string{ "TEST" }) };

                    GL::scope::impl::RootScope
                        program_root;
                    program_root.perform_builtins();

                    auto& utility_namespace
                        = program_root.make_namespace("utility_functions");
                    utility_namespace.add_function(GL::make_callable("length", [](GL::string const& r) { return r.length(); }));
                    // EXPECT_EQ(nullptr, program_root.try_find_callable("length", params.begin(), params.end()));

                    auto& string_class
                        = program_root.make_class(GL::type_of<GL::string>());
                    EXPECT_EQ(string_class.this_type, GL::type_of<GL::string>());
                    string_class.add_function(GL::make_callable("length", [](GL::string const& r) { return r.length(); }));
                    EXPECT_NE(nullptr, string_class.try_find_callable("length", params.begin(), params.end()));

                    // I

                    EXPECT_NE(nullptr, program_root.try_find_callable("length", params.begin(), params.end()));
                    EXPECT_EQ(4, program_root.call("length", params.begin(), params.end()).cast<GL::string::size_type>());                    
                }

                // demonstrate the implicit-cast system works with calling a function from nowhere... 
                if (1) {
                    GL::scope::impl::RootScope
                        program_root;
                    program_root.perform_builtins();
                    EXPECT_EQ(2.0f, program_root.call("log10", { GL::any::fast_any::instance(100.0) }).cast<float&>()); // double can cast to GL::value, which has access to "log10", therefore this call works OK. 
                    EXPECT_EQ(100, program_root.call("abs", { GL::any::fast_any::instance(-100) }).cast<GL::value&>()); // int can cast to GL::value, which has access to "abs", therefore this call works OK. 

                    // custom base class...
                    GL::script_type custom_unit_type_base("custom_unit_type_base");
                    // custom class that impliments base...
                    GL::script_type custom_unit_type("custom_unit_type");
                    // set-up the relationship
                    EXPECT_EQ(false, custom_unit_type.load().add_base(GL::type_of<GL::value>())); // won't work since a cpp-type cannot be tied to a script type. 
                    EXPECT_EQ(true, custom_unit_type.load().add_base(custom_unit_type_base)); // this sets-up the relationship.
                    EXPECT_EQ(true, custom_unit_type_base.load().is_base_of(custom_unit_type)); // simply confirms the relationship was set-up correctly.
                    
                    GL::any::fast_any stand_in = GL::any::fast_any::instance(100);
                    stand_in.m_casted_type = custom_unit_type.load();

                    auto& base_class_scope = program_root.make_class(custom_unit_type_base);
                    auto& impl_class_scope = program_root.make_class(custom_unit_type);

                    base_class_scope.add_function(GL::make_callable("hidden_function", [](GL::any::fast_any const& Any) -> std::string { return "success"; }, 0, {}, { { "a", custom_unit_type.load() + GL::type::Const + GL::type::Reference } }, GL::type_of<std::string>()));
                    EXPECT_EQ("success", base_class_scope.call("hidden_function", { stand_in }).cast<std::string>());
                    EXPECT_EQ("success", impl_class_scope.call("hidden_function", { stand_in }).cast<std::string>());
                    EXPECT_EQ("success", program_root.call("hidden_function", { stand_in }).cast<std::string>());

                    auto func = GL::make_callable("hidden_function_2", [](GL::any::fast_any const& Any) -> std::string { return "success"; });
                    EXPECT_EQ(true, func->m_signature.can_call_with_cast(&stand_in, &stand_in + 1));                    
                    EXPECT_EQ("success", func->operator()(stand_in).cast<std::string>());
                    base_class_scope.add_function(func);
                    EXPECT_NE(nullptr, base_class_scope.try_find_callable("hidden_function_2", &stand_in, &stand_in + 1));
                    EXPECT_EQ("success", base_class_scope.call("hidden_function_2", { stand_in }).cast<std::string>());
                    EXPECT_EQ("success", impl_class_scope.call("hidden_function_2", { stand_in }).cast<std::string>());
                    EXPECT_EQ("success", program_root.call("hidden_function_2", { stand_in }).cast<std::string>());

                    stand_in = GL::any::fast_any::instance(100);
                    EXPECT_EQ("success", base_class_scope.call("hidden_function_2", { stand_in }).cast<std::string>()); // base class has the function directly
                    EXPECT_EQ("success", impl_class_scope.call("hidden_function_2", { stand_in }).cast<std::string>()); // searches the impl, then the base, and find it. 
                    EXPECT_EQ(nullptr, program_root.try_find_callable("hidden_function_2", &stand_in, &stand_in + 1)); // searches the root (not its children) and searches the "int" class, but fails to find the function.
                    stand_in.m_casted_type = custom_unit_type.load(); 
                    EXPECT_EQ("success", program_root.call("hidden_function_2", { stand_in }).cast<std::string>()); // searches the root (not its children) but fails to find the function. Searches the impl_class_scope, then its base, and finally succeeds in its search.
                }

                // demonstrate an extension to an existing class, competing with a local function-object with the same name. 
                if (1) {
                    GL::scope::impl::RootScope
                        program_root;
                    program_root.perform_builtins();

                    // 
                    if (auto* BC = program_root.try_find_class(GL::type_of<int>()); BC != nullptr) {
                        if (BC->this_m.is_class()) {
                            if (auto* Class = dynamic_cast<GL::scope::impl::ClassScope*>(BC->this_m.scope); Class != nullptr) {
                                Class->add_function(GL::make_callable("print", [](GL::any::fast_any const& o) -> int { return o.cast<int>(); }, 0, {}, { { "srce", GL::type_of<int const&>() } }, GL::type_of<int>()));
                            }
                        }                        
                    }

                    EXPECT_EQ(100, program_root.call("print", { GL::any::fast_any::instance(100) }).cast<int>()); 
                    EXPECT_EQ(200, program_root.call("print", { GL::any::fast_any::instance(200.0) }).cast<int>());
                    EXPECT_EQ(300, program_root.call("print", { GL::any::fast_any::instance(300.0f) }).cast<int>());
                    EXPECT_EQ(400, program_root.call("print", { GL::any::fast_any::instance(GL::foot(400)) }).cast<int>());

                    if (auto temp_scope = program_root.make_scope(); !temp_scope.is_namespace()) {
                        EXPECT_EQ(100, temp_scope.call("print", { GL::any::fast_any::instance(100) }).cast<int>());
                        EXPECT_EQ(200, temp_scope.call("print", { GL::any::fast_any::instance(200.0) }).cast<int>());
                        EXPECT_EQ(300, temp_scope.call("print", { GL::any::fast_any::instance(300.0f) }).cast<int>());
                        EXPECT_EQ(400, temp_scope.call("print", { GL::any::fast_any::instance(GL::foot(400)) }).cast<int>());
                    }

                    if (auto temp_scope = program_root.make_scope(); !temp_scope.is_namespace()) {
                        temp_scope.insert_object_here("print", GL::make_callable("print", [](GL::any::fast_any const& o) -> int { return o.cast<int>() / 2; }, 0, {}, { { "srce", GL::type_of<int const&>() } }, GL::type_of<int>()));
                        EXPECT_EQ(50, temp_scope.call("print", { GL::any::fast_any::instance(100) }).cast<int>());
                        EXPECT_EQ(100, temp_scope.call("print", { GL::any::fast_any::instance(200.0) }).cast<int>());
                        EXPECT_EQ(150, temp_scope.call("print", { GL::any::fast_any::instance(300.0f) }).cast<int>());
                        EXPECT_EQ(200, temp_scope.call("print", { GL::any::fast_any::instance(GL::foot(400)) }).cast<int>());
                    }

                    EXPECT_EQ(100, program_root.call("print", { GL::any::fast_any::instance(100) }).cast<int>());
                    EXPECT_EQ(200, program_root.call("print", { GL::any::fast_any::instance(200.0) }).cast<int>());
                    EXPECT_EQ(300, program_root.call("print", { GL::any::fast_any::instance(300.0f) }).cast<int>());
                    EXPECT_EQ(400, program_root.call("print", { GL::any::fast_any::instance(GL::foot(400)) }).cast<int>());

                    if (auto temp_scope = program_root.make_scope(); !temp_scope.is_namespace()) {
                        EXPECT_EQ(100, temp_scope.call("print", { GL::any::fast_any::instance(100) }).cast<int>());
                        EXPECT_EQ(200, temp_scope.call("print", { GL::any::fast_any::instance(200.0) }).cast<int>());
                        EXPECT_EQ(300, temp_scope.call("print", { GL::any::fast_any::instance(300.0f) }).cast<int>());
                        EXPECT_EQ(400, temp_scope.call("print", { GL::any::fast_any::instance(GL::foot(400)) }).cast<int>());
                        temp_scope.insert_object_here("print", GL::make_callable("print", [](GL::any::fast_any const& o) -> int { return o.cast<int>() / 2; }, 0, {}, { { "srce", GL::type_of<int const&>() } }, GL::type_of<int>()));
                        EXPECT_EQ(50, temp_scope.call("print", { GL::any::fast_any::instance(100) }).cast<int>());
                        EXPECT_EQ(100, temp_scope.call("print", { GL::any::fast_any::instance(200.0) }).cast<int>());
                        EXPECT_EQ(150, temp_scope.call("print", { GL::any::fast_any::instance(300.0f) }).cast<int>());
                        EXPECT_EQ(200, temp_scope.call("print", { GL::any::fast_any::instance(GL::foot(400)) }).cast<int>());
                    }
                }

                if (1) {
                    GL::script_type custom_unit_type_base("custom_unit_type_base");
                    GL::script_type custom_unit_type("custom_unit_type");
                    EXPECT_EQ(false, custom_unit_type.load().add_base(GL::type_of<GL::value>()));
                    EXPECT_EQ(true, custom_unit_type.load().add_base(custom_unit_type_base));
                    EXPECT_EQ(true, custom_unit_type_base.load().is_base_of(custom_unit_type));

                    GL::scope::impl::RootScope
                        program_root;
                    program_root.perform_builtins();

                    auto& std_namespace
                        = program_root.make_namespace("std");
                    auto& std_string_namespace
                        = std_namespace.make_namespace("string");
                    auto& std_map_namespace
                        = std_namespace.make_namespace("map");
                    auto& std_numeric_limits_namespace
                        = std_namespace.make_namespace("numeric_limits");

                    /*program_root.add_function(GL::make_converter<GL::foot, GL::meter>());
                    program_root.add_function(GL::make_converter<GL::meter, GL::foot>());
                    program_root.add_function(GL::make_converter<GL::meter, GL::value>());
                    program_root.add_function(GL::make_converter<GL::value, GL::meter>());
                    program_root.add_function(GL::make_converter<GL::value, GL::foot>());
                    program_root.add_function(GL::make_converter<GL::value, float>());
                    program_root.add_function(GL::make_converter<float, GL::value>());*/

                    program_root.add_function(GL::make_callable("ref_cast", [](GL::any::fast_any const& from) -> GL::any::fast_any {
                        GL::any::fast_any out(from);
                        out.m_casted_type = GL::type_of<float&>();
                        return out;
                        }, GL::function_signature::Constructor | GL::function_signature::Async | GL::function_signature::NoCost, {}, { { "From", GL::type_of<int&>() } }, GL::type_of<float&>()));

                    program_root.add_function(GL::make_callable("value", [](GL::any::fast_any const& from) -> GL::value {
                        return GL::meter();
                        }, GL::function_signature::Constructor | GL::function_signature::Async | GL::function_signature::NoCost, {}, { { "From", custom_unit_type.load() | GL::type::Const | GL::type::Reference } }, GL::type_of< GL::value>()));
                    program_root.add_function(GL::make_callable("custom_unit_type", [x = custom_unit_type.load()](GL::meter const& from)->GL::any::fast_any {
                        GL::any::fast_any out;
                        out.m_casted_type = x;
                        return out;
                    }, GL::function_signature::Constructor | GL::function_signature::Async | GL::function_signature::NoCost, {}, { { "From", GL::type_of<GL::meter const&>() } }, custom_unit_type.load()));


                    std_string_namespace.add_function(GL::decl_func(&std::string::length));
                    std_string_namespace.insert_object_here("length", GL::make_callable(GL::string::empty_string(), []() -> size_t { return std::numeric_limits<size_t>::max(); })); // insert a function as an object. Basically a lambda!
                    std_string_namespace.add_function(GL::decl_func(&std::string::capacity));
                    std_string_namespace.add_function(GL::decl_func(&std::string::clear));
                    std_string_namespace.add_function(GL::decl_func(&std::string::empty));

                    program_root.add_function(GL::make_callable(
                        "print"
                        , [](GL::any::fast_any const& any_type) -> std::string { return any_type.cast<std::string>() + "_root"; }
                        , { std::string{ "root_default" } }
                    ));
                    program_root.add_function(GL::make_callable("type_name", [](GL::any const& any_type) -> GL::string { return any_type.m_casted_type.name(); }));
                    program_root.add_function(GL::make_callable("type_name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }));
                    program_root.add_function(GL::make_callable("type_of", [](GL::any const& any_type) -> GL::type { return any_type.m_casted_type; }));

                    program_root.add_function(GL::make_callable("+2", [](GL::foot const& a, GL::foot const& b) -> GL::foot { return a + b; }));
                    program_root.add_function(GL::make_callable("+2", [](GL::meter const& a, GL::meter const& b) -> GL::meter { return a + b; }));
                    program_root.add_function(GL::make_callable("+2", [](int const& a, int const& b) -> int { return a + b; }));
                    program_root.add_function(GL::make_callable("+2", [](float const& a, float const& b) -> float { return a + b; }));
                    program_root.add_function(GL::make_callable("+2", [](double const& a, double const& b) -> double { return a + b; }));
                    program_root.add_function(GL::make_callable("+2", [](long double const& a, long double const& b) -> long double { return a + b; }));

                    std_string_namespace.add_function(GL::make_callable(
                        "print"
                        , [](GL::any::fast_any const& any_type) -> std::string {
                            if (any_type.can_cast(GL::type_of<std::string>())) {
                                return any_type.cast<std::string>() + "_std";
                            }
                            else {
                                return "any_std";
                            }
                        }
                        , { std::string{ "std_string_default" } }
                        ));
                    std_string_namespace.add_function(GL::make_callable(
                        "print"
                        , [](std::string const& any_type) -> std::string { return any_type + "_std_string"; }
                    ));

                    if (1) {
                        std::vector < GL::any > empty_types;
                        EXPECT_NE(nullptr, std_string_namespace.try_find_callable("length", empty_types.begin(), empty_types.end(), GL::scope::impl::Functions::free_cast_only));
                    }

                    // function calling with a custom script type with user-defined name(s)
                    if (1) {
                        GL::script_type custom_type("custom_type");
                        GL::any custom_impl = 100;
                        custom_impl.m_casted_type = custom_type.load() | GL::type::Const | GL::type::Reference;
                        EXPECT_EQ(false, custom_impl.m_casted_type.is_cpp_type());
                        EXPECT_EQ(true, custom_impl.m_casted_type.is_const_ref());
                        EXPECT_EQ("const custom_type&", std_string_namespace.call("type_name", { custom_impl.fast() }).cast<GL::string>());
                    }

                    if (0) {
                        auto from_type = GL::type_of<GL::meter&&>();
                        auto& func = program_root.constructors.try_find_callable(GL::type_of<GL::value&>(), &from_type, &from_type + 1, 0);
                        if (func) print(func->m_signature.display());
                    }
                    if (0) {
                        auto from_type = GL::type_of<GL::meter&&>();
                        auto& func = program_root.constructors.try_find_callable(GL::type_of<GL::meter&>(), &from_type, &from_type + 1, 0);
                        if (func) print(func->m_signature.display());
                    }

                    EXPECT_EQ(false, GL::type_of<GL::meter&&>().can_free_cast(GL::type_of<GL::value&>()));
                    EXPECT_EQ(false, GL::type_of<GL::meter&&>().can_free_cast(GL::type_of<GL::value>()));
                    EXPECT_EQ(false, GL::type_of<GL::meter>().can_free_cast(GL::type_of<GL::value>()));
                    EXPECT_EQ(true, GL::type_of<GL::meter&>().can_free_cast(GL::type_of<GL::value&>()));
                    EXPECT_EQ(true, GL::type_of<GL::meter>().can_free_cast(GL::type_of<GL::value&>()));
                    EXPECT_EQ(true, GL::type_of<GL::meter&>().can_free_cast(GL::type_of<GL::value&>()));
                    EXPECT_EQ(true, GL::type_of<GL::meter&&>().can_free_cast(GL::type_of<GL::value&&>()));

                    EXPECT_EQ(false, GL::type_of<GL::value&&>().can_free_cast(GL::type_of<GL::meter&>()));
                    EXPECT_EQ(false, GL::type_of<GL::value&&>().can_free_cast(GL::type_of<GL::meter>()));
                    EXPECT_EQ(false, GL::type_of<GL::value&>().can_free_cast(GL::type_of<GL::meter&>()));
                    EXPECT_EQ(false, GL::type_of<GL::value&&>().can_free_cast(GL::type_of<GL::meter&&>()));

                    if (0) {
                        auto From = GL::type_of<int&>();
                        //auto conversions = program_root.constructors.CreateConversions(From);
                        //for (auto& To : conversions) {
                        //    print(To.second->m_signature.display());
                        //    auto converted = To.second->operator()({ From.instance() });
                        //    EXPECT_EQ(true, converted.m_casted_type.can_free_cast(To.first));

                        //}
                        auto conversions = program_root.constructors.CreateConversions(From);
                        for (auto& To : conversions) {
                            auto converted = To.second->operator()({ From.instance() });
                            EXPECT_EQ(true, converted.m_casted_type.can_free_cast(To.first));
                            //print(To.second->m_signature.display());
                        }
                        GL::atomic_allocator<
                            std::variant<GL::scope::impl::Functions::UniformCostSearchNode, GL::scope::impl::Functions::UniformCostSearchNodeBestPath>
                            , 1024
                        > temp_alloc;
                        auto converters
                            = program_root.constructors.CreateConversionPaths(temp_alloc, From);

                        for (auto& To : converters) {
                            if (To.second) {
                                print(From.name() + " to " + To.first.name() + ": " + std::to_string(To.first.get_qualifiers()));

                                std::vector<GL::type> best_path;
                                To.second->bestPath->get(best_path);
                                GL::string t;
                                t = t.add_to_delim(From.name(), "->");
                                for (auto& path : best_path) {
                                    t = t.add_to_delim(path.name(), "->");
                                }
                                print("\t" + t + ": ");
                                auto converter = To.second->bestPath->make_converter(From, program_root.constructors);
                                auto converted = converter->operator()({ 1 });
                                EXPECT_EQ(true, converted.m_casted_type.can_free_cast(To.first));
                                print("\t" + converter->m_signature.display());
                            }
                        }
                    }
                    //print("");

                    if (0) {
                        auto From = GL::type_of<int const&>();
                        /*                    auto conversions = program_root.constructors.CreateConversions(From);
                                            for (auto& To : conversions) {
                                                print(To.second->m_signature.display());
                                                auto converted = To.second->operator()({ From.instance() });
                                                EXPECT_EQ(true, converted.m_casted_type.can_free_cast(To.first));

                                            }   */
                        auto conversions = program_root.constructors.CreateConversions(From);
                        for (auto& To : conversions) {
                            auto converted = To.second->operator()({ From.instance() });
                            EXPECT_EQ(true, converted.m_casted_type.can_free_cast(To.first));
                            //print(To.second->m_signature.display());
                        }
                        GL::atomic_allocator<
                            std::variant<GL::scope::impl::Functions::UniformCostSearchNode, GL::scope::impl::Functions::UniformCostSearchNodeBestPath>
                            , 1024
                        > temp_alloc;
                        auto converters
                            = program_root.constructors.CreateConversionPaths(temp_alloc, From);
                        for (auto& To : converters) {
                            if (To.second) {
                                print(From.name() + " to " + To.first.name() + ": ");

                                std::vector<GL::type> best_path;
                                To.second->bestPath->get(best_path);
                                GL::string t;
                                t = t.add_to_delim(From.name(), "->");
                                for (auto& path : best_path) {
                                    t = t.add_to_delim(path.name(), "->");
                                }
                                print("\t" + t + ": ");

                                auto converter = To.second->bestPath->make_converter(From, program_root.constructors);
                                auto converted = converter->operator()({ 1 });
                                EXPECT_EQ(true, converted.m_casted_type.can_free_cast(To.first));
                                print("\t" + converter->m_signature.display());
                            }
                        }
                    }
                    //print("");

                    if (0) {
                        auto From = custom_unit_type.load() + GL::type::Const + GL::type::Reference;
                        GL::atomic_allocator<
                            std::variant<GL::scope::impl::Functions::UniformCostSearchNode, GL::scope::impl::Functions::UniformCostSearchNodeBestPath>
                            , 1024
                        > temp_alloc;
                        auto converters
                            = program_root.constructors.CreateConversionPaths(temp_alloc, From);
                        for (auto& To : converters) {
                            if (To.second) {
                                print(From.name() + " to " + To.first.name() + ": ");

                                std::vector<GL::type> best_path;
                                To.second->bestPath->get(best_path);
                                GL::string t;
                                t = t.add_to_delim(From.name(), "->");
                                for (auto& path : best_path) {
                                    t = t.add_to_delim(path.name(), "->");
                                }
                                print("\t" + t + ": ");

                                auto converter = To.second->bestPath->make_converter(From, program_root.constructors);
                                if (converter) {
                                    print("\t" + converter->m_signature.display());
                                }
                            }
                        }
                    }
                    // print("");

                    if (0) {
                        std::vector < GL::type > types = { GL::type_of<int>(), GL::type_of<float>(), GL::type_of<double>(), GL::type_of<std::string>(), GL::type_of<GL::meter>(), GL::type_of<GL::meter&&>(), GL::type_of<GL::meter const&>() };
                        for (auto& from : types) {
                            auto converters = program_root.constructors.CreateConversions(from);
                            for (auto& to : converters) {
                                std::vector<GL::any::fast_any> params{ from.instance().fast() };
                                GL::string out = from.name() + " -> " + to.first.name();
                                print(out);
                                try {
                                    EXPECT_EQ(to.second->operator()(params.begin(), params.end()).m_casted_type.can_free_cast(to.first), true);
                                }
                                catch (std::exception const& e) {
                                    out = std::string("\tcould not perform conversion due to exception: ") + std::string(e.what());
                                    print(out);
                                }
                            }
                        }
                    }

                    if (auto timer = sw.debug_timer("1 million scopes with 10 sub-scopes")) {
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

                    if (auto timer = sw.debug_timer("example calc"); false) {
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

                    if (auto timer = sw.debug_timer("example calc 2"); false) {
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

                    if (auto timer = sw.debug_timer("example calc 2 (once only)")) {
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

                    if (auto timer = sw.debug_timer("try_get_converter")) {
                        // for (int i = 0; i < 1000000; ++i) {
                        GL::parallel::For(0, 1000000, [&](size_t i) {
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int>(), GL::type_of<double>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int>(), GL::type_of<double&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int>(), GL::type_of<double const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&&>(), GL::type_of<double>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&&>(), GL::type_of<double&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&&>(), GL::type_of<double const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&>(), GL::type_of<double>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&>(), GL::type_of<double&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&>(), GL::type_of<double const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<double>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<double&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<double const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int>(), GL::type_of<int>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int>(), GL::type_of<int&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int>(), GL::type_of<int const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int>(), GL::type_of<int&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&&>(), GL::type_of<int>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&&>(), GL::type_of<int&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int&&>(), GL::type_of<int const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<int>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<int&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<int const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<double>(), GL::type_of<GL::meter>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<double>(), GL::type_of<GL::meter const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter>(), GL::type_of<double>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter>(), GL::type_of<double const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter>(), GL::type_of<GL::value>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter&>(), GL::type_of<GL::value&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter>(), GL::type_of<GL::value&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter&&>(), GL::type_of<GL::value&&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter const&>(), GL::type_of<GL::value const&>()));

                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter>(), custom_unit_type));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter&&>(), custom_unit_type));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter&&>(), custom_unit_type.load() | GL::type::Temporary));
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter const&>(), custom_unit_type)); // requires a copy operation, but since it's going to a base type, that's OK
                            EXPECT_EQ(true, program_root.try_get_converter(GL::type_of<GL::meter const&>(), custom_unit_type.load() | GL::type::Temporary)); // requires a copy operation, but since it's going to a temp type, that's OK
                            EXPECT_EQ(true, program_root.try_get_converter(custom_unit_type, GL::type_of<GL::value>()));
                            EXPECT_EQ(true, program_root.try_get_converter(custom_unit_type, GL::type_of<GL::meter const&>()));
                            EXPECT_EQ(true, program_root.try_get_converter(custom_unit_type.load() | GL::type::Reference, custom_unit_type_base.load() | GL::type::Reference));
                            EXPECT_EQ(true, program_root.try_get_converter(custom_unit_type.load() | GL::type::Reference | GL::type::Const, custom_unit_type_base.load() | GL::type::Reference | GL::type::Const));
                            EXPECT_EQ(true, program_root.try_get_converter(custom_unit_type.load() | GL::type::Reference, custom_unit_type_base.load() | GL::type::Reference | GL::type::Const));

                            EXPECT_EQ(false, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<double&>()));
                            EXPECT_EQ(false, program_root.try_get_converter(GL::type_of<int&&>(), GL::type_of<int&>()));
                            EXPECT_EQ(false, program_root.try_get_converter(GL::type_of<int const&>(), GL::type_of<int&>()));
                            EXPECT_EQ(false, program_root.try_get_converter(GL::type_of<double>(), GL::type_of<GL::meter&>()));
                            EXPECT_EQ(false, program_root.try_get_converter(GL::type_of<GL::meter&&>(), GL::type_of<GL::value&>()));
                            EXPECT_EQ(false, program_root.try_get_converter(custom_unit_type_base.load() | GL::type::Reference, custom_unit_type.load() | GL::type::Reference));
                        });
                    }

                    if (auto timer = sw.debug_timer("try_find_callable")) {
                        // for (int i = 0; i < 1000000; ++i) {
                        GL::parallel::For(0, 1000000, [&](size_t i) {
                            using namespace GL::literals;
                            using namespace std::literals::string_literals;

                            EXPECT_EQ(2.0f, program_root.call("log10", { GL::any::fast_any::instance(100.0) }).cast<float&>());
                            EXPECT_EQ(100, program_root.call("abs", { GL::any::fast_any::instance(-100) }).cast<GL::value&>());

                            std::vector < GL::any > types{ GL::any{ "test"s } };
                            std::vector < GL::any > types2{ GL::any{ 100_ft } };
                            std::vector < GL::any > types3{ GL::any{ 100_ft }, GL::any{ 100_ft } };
                            std::vector < GL::any > types4{ GL::any{ 100 }, GL::any{ 100 } };
                            std::vector < GL::any > types5{ GL::any{ 'a' }, GL::any{ 'b' } };
                            std::vector < GL::any > empty_types;

                            EXPECT_NE(nullptr, program_root.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
                            EXPECT_EQ(nullptr, program_root.try_find_callable("length", empty_types.begin(), empty_types.end(), GL::scope::impl::Functions::free_cast_only)); // finds an object
                            EXPECT_NE(nullptr, program_root.try_find_callable("type_name", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
                            EXPECT_NE(nullptr, program_root.try_find_callable("type_of", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));

                            EXPECT_NE(nullptr, std_string_namespace.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
                            EXPECT_NE(nullptr, std_string_namespace.try_find_callable("length", empty_types.begin(), empty_types.end(), GL::scope::impl::Functions::free_cast_only));
                            EXPECT_NE(nullptr, std_string_namespace.try_find_callable("type_name", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
                            EXPECT_NE(nullptr, std_string_namespace.try_find_callable("type_of", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));

                            if (auto const& f = std_string_namespace.try_find_callable("type_of", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("string", f->operator()(types).cast<GL::type>().name()); // std_string
                            }
                            if (auto const& f = std_string_namespace.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ(4, f->operator()(types).cast<size_t>()); // 4
                            }

                            // finds the ProxyFunction object. objects are preferred over functions if that object is a compatable function. Note that template-function-objects are preferred over free-cast namespaced-functions! 
                            if (auto f = std_string_namespace.try_find_callable("length", empty_types.begin(), empty_types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ(std::numeric_limits<size_t>::max(), f->operator()(types).cast<size_t>());
                            }
                            if (auto f = program_root.try_find_callable("::print", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("test_root", f->operator()(types).cast<std::string>()); // default
                            }
                            if (auto f = program_root.try_find_callable("print", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("test_root", f->operator()(types).cast<std::string>()); // default
                            }
                            if (auto f = program_root.try_find_callable("print", empty_types.begin(), empty_types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("root_default_root", f->operator()(empty_types).cast<std::string>()); // default
                                EXPECT_EQ("root_default_root", f->operator()().cast<std::string>()); // default
                            }
                            if (auto f = std_string_namespace.try_find_callable("::print", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("test_root", f->operator()(types).cast<std::string>()); // default
                            }
                            if (auto f = std_string_namespace.try_find_callable("print", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("test_std_string", f->operator()(types).cast<std::string>()); // test
                                //print(f->operator()(types).cast<std::string>() + " @ 898");
                            }
                            if (auto f = program_root.try_find_callable("std::string::print", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("test_std_string", f->operator()(types).cast<std::string>()); // test
                                //print(f->operator()(types).cast<std::string>() + " @ 902");
                            }
                            if (auto f = std_string_namespace.try_find_callable("print", types2.begin(), types2.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ(false, program_root.get_converters().can_convert(types2[0].m_casted_type, GL::type_of<std::string>()));
                                EXPECT_EQ("any_std", f->operator()(types2).cast<std::string>()); // test
                            }
                            if (auto f = std_string_namespace.try_find_callable("print", types2.begin(), types2.end()); f) {
                                auto result = program_root.get_converters().call_with_conversions(&*f, types2);
                                EXPECT_EQ("any_std", result.cast<std::string>()); // test
                            }
                            if (auto f = std_string_namespace.try_find_callable("print", empty_types.begin(), empty_types.end(), GL::scope::impl::Functions::free_cast_only); f) {
                                EXPECT_EQ("std_string_default_std", f->operator()(empty_types).cast<std::string>()); // default
                                EXPECT_EQ("std_string_default_std", f->operator()().cast<std::string>()); // default
                            }
                            if (auto f = program_root.try_find_callable("+2", types3.begin(), types3.end()); f) {
                                auto result = program_root.get_converters().call_with_conversions(&*f, types3); // will not perform any cast ('foot' was ok already) then call the function, returning a 'foot'
                                EXPECT_EQ("foot", result.m_casted_type.name());
                            }
                            if (auto f = program_root.try_find_callable("+2", types4.begin(), types4.end()); f) {
                                auto result = program_root.get_converters().call_with_conversions(&*f, types4); // will not perform any cast ('int' was ok already) then call the function, returning a 'int'
                                EXPECT_EQ("int", result.m_casted_type.name());
                            }
                            if (auto f = program_root.try_find_callable("+2", types5.begin(), types5.end()); f) {
                                auto result = program_root.get_converters().call_with_conversions(&*f, types5); // 'char' is convertable to either 'foot' or 'int' or 'float', etc., therefore no guarrantee what is selected. 
                                EXPECT_EQ("int", result.m_casted_type.name()); // should return one of the options with fewest conversion -- e.g. int, float, double would all meet that definition. In this case, int is closest to 'char' and is selected. 
                            }
                        });
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
                        Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t }}, GL::type_of<std::string>()));

                        // within that namespace is an Dog impl class
                        auto& Dog = Example.make_class("Dog");
                        auto Dog_t = Dog.this_type;
                        Dog_t.add_base(Animal_t);
                        EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                        Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));

                        // within that namespace is an Cat impl class
                        auto& Cat = Example.make_class("Cat");
                        auto Cat_t = Cat.this_type;
                        Cat_t.add_base(Animal_t);
                        EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                        Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));

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
                            EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                            EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());
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
                        Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));

                        // within that namespace is an Dog impl class
                        auto& Dog = Example.make_class("Dog");
                        auto Dog_t = Dog.this_type;
                        Dog_t.add_base(Animal_t);
                        EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                        Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));
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
                        Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));
                        Cat.add_function(GL::make_callable(Cat_t.name(), [Cat_t]() -> GL::any::fast_any {
                            GL::any::fast_any out = GL::any::fast_any::instance(10);
                            out.m_casted_type = Cat_t;
                            return out;
                        }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Cat_t));

                        if (1) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                            script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                            EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                            EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());

                            EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                            EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                            EXPECT_EQ(script_scope.call("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }).cast<bool>(), true);

                            // To-Do, test for polymorphism with the casted-down type, having lost its identity. 
                            //auto found_impl = script_scope.find_object("dog_impl");
                            //found_impl.m_casted_type = Animal_t;
                            //print(script_scope.call("speak", { found_impl }).cast<std::string>());
                        }


                    }

                    if (auto timer = sw.debug_timer("Polymorphism dynamic_object var test")) {
                        GL::scope::impl::RootScope
                            root;
                        root.perform_builtins();

                        // declare a custom namespace
                        auto& Example = root.make_namespace("Example");

                        // within that namespace is an Animal interface class
                        auto& Animal = Example.make_class("Animal");
                        auto Animal_t = Animal.this_type;                        
                        Animal.add_member_object("is_pet", GL::type_of<bool>(), GL::any::fast_any::instance(bool{ true }));
                        Animal.initialize_basic_member_functions();
                        Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t | GL::type::Reference } }, GL::type_of<std::string>()));

                        // within that namespace is an Dog impl class
                        auto& Dog = Example.make_class("Dog");
                        auto Dog_t = Dog.this_type;
                        Dog_t.add_base(Animal_t);
                        EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                        Dog.add_member_object("name", GL::type_of<std::string>(), GL::any::fast_any::instance(std::string("Ozzy")));
                        Dog.add_member_object("weight", GL::type_of<double>(), GL::any::fast_any::instance(24.0));
                        Dog.initialize_basic_member_functions();
                        Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t | GL::type::Reference } }, GL::type_of<std::string>()));

                        // within that namespace is an Cat impl class
                        auto& Cat = Example.make_class("Cat");
                        auto Cat_t = Cat.this_type;
                        Cat_t.add_base(Animal_t);
                        EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                        Cat.add_member_object("name", GL::type_of<std::string>(), GL::any::fast_any::instance(std::string("Goosie")));
                        Cat.initialize_basic_member_functions();
                        Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t | GL::type::Reference } }, GL::type_of<std::string>()));

                        // normal
                        if (1) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                            script_scope.insert_object_here("cat_impl", script_scope.call("Cat", {}));
                            EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                            EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());

                            EXPECT_EQ(4, script_scope.call("length", { script_scope.call("name", {script_scope.find_object("dog_impl")}) }).cast<size_t>());
                            EXPECT_EQ("Ozzy", script_scope.call("name", { script_scope.find_object("dog_impl") }).cast<std::string&>());
                            EXPECT_EQ("Goosie", script_scope.call("name", { script_scope.find_object("cat_impl") }).cast<std::string&>());
                            EXPECT_EQ(true, script_scope.call("is_pet", { script_scope.find_object("cat_impl") }).cast<bool>());

                            EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double&>());
                            script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                            EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double>());

                            EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                            EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                            EXPECT_EQ(script_scope.call("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }).cast<bool>(), true);

                            script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any const& rhs) {
                                auto temp_scope = NearestNS->make_scope();
                                return temp_scope.call("speak", { rhs });
                            }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                            EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("dog_impl") }).cast<std::string>(), "bark");
                            EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("cat_impl") }).cast<std::string>(), "meow");
                        }

                        // as `var`
                        if (1) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                            script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                            EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                            EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());

                            EXPECT_EQ("Ozzy", script_scope.call("name", { script_scope.find_object("dog_impl") }).cast<std::string&>());
                            EXPECT_EQ("Goosie", script_scope.call("name", { script_scope.find_object("cat_impl") }).cast<std::string&>());
                            EXPECT_EQ(true, script_scope.call("is_pet", { script_scope.find_object("cat_impl") }).cast<bool>());

                            EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double&>());
                            script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                            EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double>());

                            EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                            EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                            EXPECT_EQ(script_scope.call("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }).cast<bool>(), true);

                            script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any const& rhs) {
                                auto temp_scope = NearestNS->make_scope();
                                return temp_scope.call("speak", { rhs });
                            }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                            EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("dog_impl") }).cast<std::string>(), "bark");
                            EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("cat_impl") }).cast<std::string>(), "meow");
                        }

                        // test assignemt and copy constructors
                        if (1) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_1", script_scope.call("Dog", {}));
                            EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double&>());
                            script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_1") }), GL::any::fast_any::instance(100.0) });
                            EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double>());

                            script_scope.insert_object_here("dog_2", script_scope.call("Dog", {}));
                            EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_2") }).cast<double&>());
                            script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_2") }), GL::any::fast_any::instance(200.0) });
                            EXPECT_EQ(200.0, script_scope.call("weight", { script_scope.find_object("dog_2") }).cast<double>());

                            script_scope.insert_object_here("dog_3", script_scope.call("Dog", { script_scope.find_object("dog_1") }));
                            EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_3") }).cast<double&>());
                            script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_3") }), GL::any::fast_any::instance(300.0) });
                            EXPECT_EQ(300.0, script_scope.call("weight", { script_scope.find_object("dog_3") }).cast<double>());
                            EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double>());

                            script_scope.insert_object_here("dog_4", script_scope.call("Dog", {}));
                            script_scope.call("=", { script_scope.find_object("dog_4"), script_scope.find_object("dog_1") });
                            EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_4") }).cast<double&>());
                            script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_4") }), GL::any::fast_any::instance(400.0) });
                            EXPECT_EQ(400.0, script_scope.call("weight", { script_scope.find_object("dog_4") }).cast<double>());
                            EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double>());
                        }
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
                            } catch (...) {}
                            EXPECT_EQ(std_ns.find_object("::string::npos").cast<size_t>(), GL::string::npos);
                        }
                    }

                    







                    std_string_namespace.insert_object_here("npos", GL::any::ref(std::string::npos));
                    std_numeric_limits_namespace.insert_object_here("min", std::numeric_limits<double>::lowest());
                    std_numeric_limits_namespace.insert_object_here("max", std::numeric_limits<double>::max());

                    if (auto f = std_string_namespace.find_object("npos")) {
                        EXPECT_EQ(f.m_casted_type.is_const_ref(), true);
                        EXPECT_EQ(f.m_casted_type.is_const(), true);
                        EXPECT_EQ(f.m_casted_type.is_ref(), true);
                        EXPECT_EQ(f.m_casted_type.is_cpp_type(), true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t const&>()), true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t&>()), false);
                        EXPECT_EQ(f.m_casted_type.can_cast(GL::type_of<size_t>()), true);
                    }                   

                    if (auto f = std_namespace.find_object("string::npos")) {
                        EXPECT_EQ((f.m_casted_type.get_qualifiers() & GL::type::Const) > 0, true);
                        EXPECT_EQ((f.m_casted_type.get_qualifiers() & GL::type::Reference) > 0, true);
                        EXPECT_EQ((f.m_casted_type.get_qualifiers() & GL::type::CppType) > 0, true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t const&>()), true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t&>()), false);
                        EXPECT_EQ(f.m_casted_type.can_cast(GL::type_of<size_t>()), true);
                    }                    

                    if (auto f = program_root.find_object("std::string::npos")) {
                        EXPECT_EQ(f.m_casted_type.is_const(), true);
                        EXPECT_EQ(f.m_casted_type.is_ref(), true);
                        EXPECT_EQ(f.m_casted_type.is_cpp_type(), true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t const&>()), true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t&>()), false);
                        EXPECT_EQ(f.m_casted_type.can_cast(GL::type_of<size_t>()), true);
                    }                   

                    try {
                        EXPECT_EQ(program_root.find_object("npos"), nullptr);
                    }
                    catch (...) {}

                    auto constructor1 = GL::make_callable("string", []() -> std::string { return std::string(); }, GL::function_signature::Static | GL::function_signature::Async | GL::function_signature::Constant);
                    auto constructor2 = GL::make_callable("string", [](std::string const& rhs) -> std::string { return std::string(rhs); }, GL::function_signature::Static | GL::function_signature::Async | GL::function_signature::Constant);
                    auto set_operator = GL::make_callable("=", // function name
                        [](GL::any::fast_any const& lhs, std::string const& rhs) -> GL::any::fast_any { // can use GL::any::fast_any or GL::any for these and it will work either way. fast_any is more efficient and 'honest' with the underlying system, and is therefore recommended.
                            lhs.cast<std::string&>() = rhs;
                            return lhs;
                        }, // function impl
                        {}, // defaults
                        { { "lhs", GL::type_of<std::string&>() }, { "rhs", GL::type_of<std::string const&>() } }, // arguments
                            GL::type_of<std::string&>() // return type
                            );
                    auto length_func = GL::decl_func(&std::string::length);
                    EXPECT_EQ(((set_operator->m_signature.state_m & GL::function_signature::Template) > 0), false);
                    EXPECT_EQ(((length_func->m_signature.state_m & GL::function_signature::Template) > 0), false);
                    EXPECT_EQ(((length_func->m_signature.state_m & GL::function_signature::Constant) > 0), true);
                    EXPECT_EQ(((length_func->m_signature.state_m & GL::function_signature::Async) > 0), false);

                    auto constructed1_str = constructor1->operator()({});
                    constructed1_str.cast<std::string&>() = "TEST";
                    auto constructed2_str = constructor2->operator()({ constructed1_str });
                    EXPECT_EQ(constructed2_str.cast<std::string&>(), "TEST");
                    constructed2_str.cast<std::string&>() = "TEST2";
                    auto ref_str = set_operator->operator()({ constructed1_str, constructed2_str });
                    EXPECT_EQ(constructed2_str.cast<std::string&>(), "TEST2");
                    EXPECT_EQ(ref_str.cast<std::string&>(), "TEST2");
                    EXPECT_EQ(constructed1_str.cast<std::string&>(), "TEST2");
                    EXPECT_EQ(length_func->operator()({ ref_str }).cast<size_t>(), 5);
                    ref_str.cast<std::string&>() = "TEST3";
                    EXPECT_EQ(ref_str.cast<std::string&>(), "TEST3");
                    EXPECT_EQ(constructed1_str.cast<std::string&>(), "TEST3");

                    //EXPECT_EQ(std_map_namespace.find_object("npos"), nullptr);
                    //EXPECT_EQ(std_map_namespace.find_object("npos"), nullptr);
                    std_map_namespace.add_using_here(std_string_namespace);
                    if (auto f = std_map_namespace.find_object("npos")) {
                        EXPECT_EQ(f.m_casted_type.is_const(), true);
                        EXPECT_EQ(f.m_casted_type.is_ref(), true);
                        EXPECT_EQ(f.m_casted_type.is_cpp_type(), true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t const&>()), true);
                        EXPECT_EQ(f.m_casted_type.can_free_cast(GL::type_of<size_t&>()), false);
                        EXPECT_EQ(f.m_casted_type.can_cast(GL::type_of<size_t>()), true);
                    }
                    else {
                        EXPECT_EQ(true, false);
                    }

                    //EXPECT_EQ(program_root.find_object("x"), nullptr);
                    //EXPECT_EQ(std_map_namespace.find_object("x"), nullptr);
                    program_root.insert_object_here("x", 100.0f);
                    EXPECT_NE(program_root.find_object("x"), nullptr);
                    EXPECT_NE(std_map_namespace.find_object("x"), nullptr);

                    //EXPECT_EQ(program_root.find_object("y"), nullptr);
                    //EXPECT_EQ(std_namespace.find_object("y"), nullptr);
                    //EXPECT_EQ(std_map_namespace.find_object("y"), nullptr);
                    std_namespace.insert_object_here("y", 500.0);
                    //EXPECT_EQ(program_root.find_object("y"), nullptr);
                    EXPECT_NE(std_namespace.find_object("y"), nullptr);
                    EXPECT_NE(std_map_namespace.find_object("y"), nullptr);

                    //EXPECT_EQ(program_root.find_object("std::map::z"), nullptr);
                    //EXPECT_EQ(std_namespace.find_object("std::map::z"), nullptr);
                    //EXPECT_EQ(std_map_namespace.find_object("std::map::z"), nullptr);
                    std_map_namespace.insert_object_here("z", 500);
                    EXPECT_NE(program_root.find_object("std::map::z"), nullptr);
                    EXPECT_NE(std_namespace.find_object("std::map::z"), nullptr);
                    EXPECT_NE(std_map_namespace.find_object("std::map::z"), nullptr);

                    if (auto scope = std_map_namespace.make_scope(); !scope.is_namespace()) {
                        scope.insert_object_here("w", std::string("TEST")); // inserting an object into a basic scope does NOT invalidate any search cache's
                        EXPECT_NE(nullptr, scope.find_object("x")); // found at ::
                        EXPECT_NE(nullptr, scope.find_object("y")); // found in ::std
                        EXPECT_NE(nullptr, scope.find_object("z")); // found in ::std::map
                        EXPECT_NE(nullptr, scope.find_object("npos")); // found in ::std::string, which is 'used' by std::map
                        EXPECT_NE(nullptr, scope.find_object("w")); // found in ::std::map::{}

                        //EXPECT_EQ(nullptr, std_map_namespace.find_object("max"));
                        //EXPECT_EQ(nullptr, scope.find_object("max"));
                        scope.add_using_here(std_numeric_limits_namespace);
                        EXPECT_NE(nullptr, scope.find_object("max"));
                        //EXPECT_EQ(nullptr, std_map_namespace.find_object("max"));
                    }
                    //EXPECT_EQ(nullptr, std_map_namespace.find_object("max"));
                }
#else
#if 0
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
                Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));

                // within that namespace is an Dog impl class
                auto& Dog = Example.make_class("Dog");
                auto Dog_t = Dog.this_type;
                Dog_t.add_base(Animal_t);
                EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));

                // within that namespace is an Cat impl class
                auto& Cat = Example.make_class("Cat");
                auto Cat_t = Cat.this_type;
                Cat_t.add_base(Animal_t);
                EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));

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
                    EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                    EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());
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
                    EXPECT_EQ(script_scope.call("type_name", { assigned_var }).cast< GL::string>(), "string");

                    // type-changes are automatically handled when handled 100% in-script. 
                    script_scope.emplace_object_here("x", GL::var(GL::make_shared<GL::any>(100.0f)));
                    EXPECT_EQ(script_scope.call("type_name", { script_scope.find_object("x") }).cast< GL::string>(), "float");
                    script_scope.call(":=", { script_scope.find_object("x"), GL::any::fast_any::instance(std::string("TEST")) });
                    EXPECT_EQ(script_scope.call("type_name", { script_scope.find_object("x") }).cast< GL::string>(), "string");
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
                Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));

                // within that namespace is an Dog impl class
                auto& Dog = Example.make_class("Dog");
                auto Dog_t = Dog.this_type;
                Dog_t.add_base(Animal_t);
                EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));
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
                Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));
                Cat.add_function(GL::make_callable(Cat_t.name(), [Cat_t]() -> GL::any::fast_any {
                    GL::any::fast_any out = GL::any::fast_any::instance(10);
                    out.m_casted_type = Cat_t;
                    return out;
                    }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Cat_t));

                if (1) {
                    auto script_scope = root.make_scope();
                    script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                    script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                    EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                    EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());

                    EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                    EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                    EXPECT_EQ(script_scope.call("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }).cast<bool>(), true);

                    // To-Do, test for polymorphism with the casted-down type, having lost its identity. 
                    //auto found_impl = script_scope.find_object("dog_impl");
                    //found_impl.m_casted_type = Animal_t;
                    //print(script_scope.call("speak", { found_impl }).cast<std::string>());
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
                Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t | GL::type::Reference } }, GL::type_of<std::string>()));
                   
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
                Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t | GL::type::Reference } }, GL::type_of<std::string>()));

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
                Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t | GL::type::Reference } }, GL::type_of<std::string>()));

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
                Lion.add_function(GL::make_callable("speak", [](GL::any::fast_any const& rhs) -> std::string { return "MEOW"; }, 0, {}, { { "rhs", Lion_t | GL::type::Reference } }, GL::type_of<std::string>()));

                // normal
                if (1) {
                    auto script_scope = root.make_scope();
                    script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                    script_scope.insert_object_here("cat_impl", script_scope.call("Cat", {}));
                    EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                    EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());

                    EXPECT_EQ(4, script_scope.call("length", { script_scope.call("name", {script_scope.find_object("dog_impl")}) }).cast<size_t>());
                    EXPECT_EQ("Ozzy", script_scope.call("name", { script_scope.find_object("dog_impl") }).cast<std::string&>());
                    EXPECT_EQ("Goosie", script_scope.call("name", { script_scope.find_object("cat_impl") }).cast<std::string&>());
                    EXPECT_EQ(true, script_scope.call("is_pet", { script_scope.find_object("cat_impl") }).cast<bool>());

                    EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double&>());
                    script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                    EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double>());

                    EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                    EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                    EXPECT_EQ(script_scope.call("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }).cast<bool>(), true);

                    script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any const& rhs) {
                        auto temp_scope = NearestNS->make_scope();
                        return temp_scope.call("speak", { rhs });
                    }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                    EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("dog_impl") }).cast<std::string>(), "bark");
                    EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("cat_impl") }).cast<std::string>(), "meow");
                }

                // as `var`
                if (1) {
                    auto script_scope = root.make_scope();
                    script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                    script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                    EXPECT_EQ("bark", script_scope.call("speak", { script_scope.find_object("dog_impl") }).cast<std::string>());
                    EXPECT_EQ("meow", script_scope.call("speak", { script_scope.find_object("cat_impl") }).cast<std::string>());

                    EXPECT_EQ("Ozzy", script_scope.call("name", { script_scope.find_object("dog_impl") }).cast<std::string&>());
                    EXPECT_EQ("Goosie", script_scope.call("name", { script_scope.find_object("cat_impl") }).cast<std::string&>());
                    EXPECT_EQ(true, script_scope.call("is_pet", { script_scope.find_object("cat_impl") }).cast<bool>());

                    EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double&>());
                    script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                    EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_impl") }).cast<double>());

                    EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                    EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                    EXPECT_EQ(script_scope.call("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }).cast<bool>(), true);

                    script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any const& rhs) {
                        auto temp_scope = NearestNS->make_scope();
                        return temp_scope.call("speak", { rhs });
                    }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                    EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("dog_impl") }).cast<std::string>(), "bark");
                    EXPECT_EQ(script_scope.call("talk_to", { script_scope.find_object("cat_impl") }).cast<std::string>(), "meow");
                }

                // test assignment and copy constructors
                if (1) {
                    auto script_scope = root.make_scope();
                    script_scope.insert_object_here("dog_1", script_scope.call("Dog", {}));
                    EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double&>());
                    script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_1") }), GL::any::fast_any::instance(100.0) });
                    EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double>());

                    script_scope.insert_object_here("dog_2", script_scope.call("Dog", {}));
                    EXPECT_EQ(24.0, script_scope.call("weight", { script_scope.find_object("dog_2") }).cast<double&>());
                    script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_2") }), GL::any::fast_any::instance(200.0) });
                    EXPECT_EQ(200.0, script_scope.call("weight", { script_scope.find_object("dog_2") }).cast<double>());

                    script_scope.insert_object_here("dog_3", script_scope.call("Dog", { script_scope.find_object("dog_1") }));
                    EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_3") }).cast<double&>());
                    script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_3") }), GL::any::fast_any::instance(300.0) });
                    EXPECT_EQ(300.0, script_scope.call("weight", { script_scope.find_object("dog_3") }).cast<double>());
                    EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double>());

                    script_scope.insert_object_here("dog_4", script_scope.call("Dog", {}));
                    script_scope.call("=", { script_scope.find_object("dog_4"), script_scope.find_object("dog_1") });
                    EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_4") }).cast<double&>());
                    script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_4") }), GL::any::fast_any::instance(400.0) });
                    EXPECT_EQ(400.0, script_scope.call("weight", { script_scope.find_object("dog_4") }).cast<double>());
                    EXPECT_EQ(100.0, script_scope.call("weight", { script_scope.find_object("dog_1") }).cast<double>());
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
                        EXPECT_EQ((float)(i * 5), (float)script_scope.call("counter", {script_scope.find_object("dog_impl")}).cast<GL::value>());
                    }
                };

                // test the double-inheritor of Lion...
                if (1) {
                    auto script_scope = root.make_scope();
                    script_scope.insert_object_here("lion_impl", script_scope.call("Example::Lion", {}));
                    EXPECT_EQ("Goosie", script_scope.call("name", { script_scope.find_object("lion_impl") }).cast<std::string&>());
                    EXPECT_EQ("MEOW", script_scope.call("speak", { script_scope.find_object("lion_impl") }).cast<std::string>());
                    EXPECT_EQ(true, script_scope.call("is_pet", { script_scope.find_object("lion_impl") }).cast<bool>());

                }
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
            }
#endif
            if (auto timer = sw.debug_timer("Template classes")) {
                GL::scope::impl::RootScope 
                    root;
                root.perform_builtins();

                if (1) {
                    /*
                    class vector<T> {
                        T obj;
                        T& operator[](uint x){ 
                            return obj;
                        }
                    }
                    */
                    auto& BaseClass = root.make_class("vector");
                    BaseClass.add_member_object("obj", GL::type_of<GL::template_parameter<0>>());
                    BaseClass.initialize_basic_member_functions();
                    BaseClass.add_function(GL::make_callable("[]", [&BaseClass](GL::any::fast_any const& lhs, unsigned int rhs) -> GL::any::fast_any {
                        return BaseClass.GetRoot()->call("obj", { lhs });
                    }, 0, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::type_of<unsigned int>() } }, GL::type_of<GL::template_parameter<0>>() | GL::type::Reference));

                    /* vector<int> */
                    auto& Class = BaseClass.make_inherited_template_class({ GL::type_of<int>() });

                    /*
                    auto vec = vector<int>();
                    vec[0] = 10;
                    */
                    if (auto this_scope = root.make_scope()) {
                        auto vec = this_scope.call("vector<int>", {});
                        EXPECT_EQ(vec.m_casted_type, Class.this_type);

                        auto vec_obj = this_scope.call("[]", { vec, GL::any::fast_any::instance(0) });
                        EXPECT_EQ(vec_obj.m_casted_type, GL::type_of<int&>());
                        this_scope.call("=", { vec_obj, GL::any::fast_any::instance(10) });
                        EXPECT_EQ(vec.cast<GL::dynamic_object>()["obj"]->cast<int>(), 10);
                    }
                }

                if (1) {
                    // make template class
                    auto& BaseClass = root.make_class("map");
                    if (1) {
                        // teach it how to create the generic map
                        auto& AnyMap = BaseClass.make_class(GL::type_of<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>>());
                        const_cast<GL::type&>(AnyMap.this_type).try_update_name("map_impl");
                        AnyMap.add_function(GL::make_callable(AnyMap.this_type.name(), []() -> GL::shared_ptr<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>> { return GL::make_shared<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>>(); }, GL::function_signature::Constructor));
                        AnyMap.add_function(GL::make_callable(AnyMap.this_type.name(), [&AnyMap](GL::epoch_map<std::pair<GL::any, GL::any>, size_t> const& rhs) -> GL::shared_ptr<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>> {
                            auto out = GL::make_shared<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>>();
                            for (auto& x : rhs) {
                                out->insert_fast(*x.first, { 
                                    [&]() -> GL::any::fast_any {
                                        if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second->first.m_casted_type)) {
                                            return BC->this_m.scope->call(x.second->first.m_casted_type.name(), { x.second->first.fast() });
                                        }
                                        return GL::any::fast_any();
                                    }(), [&]() {
                                        if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second->second.m_casted_type)) {
                                            return BC->this_m.scope->call(x.second->first.m_casted_type.name(), { x.second->second.fast() });
                                        }
                                        return GL::any::fast_any();
                                    }() 
                                });
                            }
                            return out;
                        }, GL::function_signature::Constructor));       

                        AnyMap.GetRoot()->add_function(GL::make_callable("=", [&AnyMap](GL::any::fast_any const& Lhs, GL::epoch_map<std::pair<GL::any, GL::any>, size_t> const& rhs) -> GL::any::fast_any {
                            auto& out = Lhs.cast<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>&>();
                            for (auto& x : rhs) {
                                auto& destination = out.get_or_make(*x.first, [&]() -> std::pair<GL::any, GL::any> { return {
                                        [&]() -> GL::any::fast_any {
                                            if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second->first.m_casted_type)) {
                                                return BC->this_m.scope->call(x.second->first.m_casted_type.name(), { x.second->first.fast() });
                                            }
                                            return GL::any::fast_any();
                                        }(), 
                                        [&]() {
                                            if (auto* BC = AnyMap.GetRoot()->try_find_class(x.second->second.m_casted_type)) {
                                                return BC->this_m.scope->call(x.second->first.m_casted_type.name(), { x.second->second.fast() });
                                            }
                                            return GL::any::fast_any();
                                        }()
                                    }; 
                                });
                                AnyMap.GetRoot()->call("=", { destination.first.fast(), x.second->first.fast() });
                                AnyMap.GetRoot()->call("=", { destination.second.fast(), x.second->second.fast() });
                            }
                            return Lhs;
                         }, 0, {}, { { "lhs", GL::type_of<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>&>() }, { "rhs", GL::type_of<GL::epoch_map<std::pair<GL::any, GL::any>, size_t> const&>() } }, GL::type_of<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>&>()));

                    }
                    BaseClass.add_member_object("impl", GL::type_of<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>>());
                    BaseClass.add_function(GL::make_callable("[]", [&BaseClass](GL::any::fast_any const& lhs, GL::any::fast_any const& rhs) -> GL::any::fast_any {
                        if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("impl"); implp && *implp) {
                            auto& impl = (*implp)->cast<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>>();
                            auto hash = BaseClass.GetRoot()->call("to_hash", { rhs }).cast<size_t>();
                            if (auto* p = impl.try_at(hash); p) {
                                return p->second.fast() | GL::type::Reference | GL::type::Const;
                            }
                            else {
                                throw std::range_error("Key was not found in map");
                            }                            
                        }
                        throw std::runtime_error("Could not instantiate the map internals");
                    }, GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::type_of<GL::template_parameter<0>>() | GL::type::Const | GL::type::Reference } }, GL::type_of<GL::template_parameter<1>>() | GL::type::Const | GL::type::Reference));
                    BaseClass.add_function(GL::make_callable("[]", [&BaseClass](GL::any::fast_any const& lhs, GL::any::fast_any const& rhs) -> GL::any::fast_any {
                        if (auto* implp = lhs.cast<GL::dynamic_object>().try_at("impl"); implp && *implp) {
                            auto& impl = (*implp)->cast<GL::epoch_map<std::pair<GL::any, GL::any>, size_t>>();
                            auto hash = BaseClass.GetRoot()->call("to_hash", { rhs }).cast<size_t>();
                            return impl.get_or_make(hash, [&]() -> std::pair<GL::any, GL::any> {
                                GL::any::fast_any obj_t;
                                if (auto* impl_class = BaseClass.GetRoot()->try_find_class(lhs.m_casted_type); impl_class && impl_class->this_m.is_class()) {
                                    auto* impl_class_p = dynamic_cast<GL::scope::impl::ClassScope*>(impl_class->this_m.scope);
                                    if (impl_class_p->template_types.size() >= 2) {
                                        if (auto* obj_type_class = BaseClass.GetRoot()->try_find_class(impl_class_p->template_types[1]); obj_type_class && obj_type_class->this_m.is_class()) {
                                            auto* obj_type_class_p = dynamic_cast<GL::scope::impl::ClassScope*>(obj_type_class->this_m.scope);
                                            obj_t = obj_type_class_p->call(obj_type_class_p->this_type.name(), {});
                                        }
                                    }
                                }
                                return { rhs, obj_t };
                            }).second.fast() + GL::type::Reference;
                        }
                        throw std::runtime_error("Could not instantiate the map internals");
                    }, GL::function_signature::Constant, {}, { { "lhs", BaseClass.this_type | GL::type::Reference } , { "rhs", GL::type_of<GL::template_parameter<0>>() | GL::type::Const | GL::type::Reference } }, GL::type_of<GL::template_parameter<1>>() | GL::type::Reference));
                    BaseClass.initialize_basic_member_functions();

                    print(root.DetermineType("map<::string,::value>").name());
                    print(root.DetermineType("map<std::string,::value>").name());

                    print(root.DetermineType("int").name());
                    print(root.DetermineType("int const&").name());
                    print(root.DetermineType("int&&").name());
                    print(root.DetermineType("int const").name());
                    print(root.DetermineType("const int").name());
                    print(root.DetermineType("vector").name());
                    print(root.DetermineType("vector<int>").name());
                    print(root.DetermineType("vector<double>").name());
                    print(root.DetermineType("vector<int> const&").name());
                    print(root.DetermineType("vector<double> const&").name());
                    print(root.DetermineType("vector<int&>").name());
                    print(root.DetermineType("vector<int const&>").name());
                    print(root.DetermineType("map").name());
                    print(root.DetermineType("map<string,value>").name());
                    print(root.DetermineType(" map < string , value > ").name());
                    print(root.DetermineType("std::string").name());
                    print(root.DetermineType("vector").name());
                    print(root.DetermineType("vector<int>").name());
                    print(root.DetermineType("vector<double>").name());
                    print(root.DetermineType("map<std::string,value>").name());
                    print(root.DetermineType("map<string,::value>").name());
                    print(root.DetermineType("::map<std::string,::value> const&").name());
                    print(root.DetermineType("::map< std::string, ::value> const&").name());
                    print(root.DetermineType("::map< std::string, ::map< ::value, ::vector> > const&").name());

                    //Class.for_each_function([&](GL::Proxy_Function const& f)->bool { print(f->m_signature.display()); return false; });

                    GL::parallel::For(0, 1000000, [&root](int) {
                        if (auto this_scope = root.make_scope()) {
                            auto vec = this_scope.call("map<string,value>", {});
                            EXPECT_EQ(vec.m_casted_type, Class.this_type);

                            auto vec_obj = this_scope.call("[]", { vec, GL::any::fast_any::instance(GL::string("TEST")) }); // creates a GL::value in the map and returns it
                            EXPECT_EQ(vec_obj.m_casted_type, GL::type_of<GL::value&>());
                            this_scope.call("=", { vec_obj, GL::any::fast_any::instance(10) });

                            auto vec_obj_2 = this_scope.call("[]", { vec, GL::any::fast_any::instance(GL::string("TEST")) });
                            EXPECT_EQ(true, this_scope.call("==", { vec_obj_2, GL::any::fast_any::instance(10) }).cast<bool>());
                        }
                    });
                }














                //auto& BaseClass = root.make_class("vector");
                //BaseClass.add_member_object("obj", GL::type_of<GL::template_parameter<0>>());
                //BaseClass.initialize_basic_member_functions();
                //BaseClass.add_function(GL::make_callable("[]", [&BaseClass](GL::any::fast_any const& lhs, unsigned int rhs) -> GL::any::fast_any {
                //    return BaseClass.GetRoot()->call("obj", { lhs });
                //}, 0, {}, { { "lhs", BaseClass.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::type_of<unsigned int>() }}, GL::type_of<GL::template_parameter<0>>() | GL::type::Reference));

                //auto& Class = BaseClass.make_inherited_template_class({ GL::type_of<int>() });
                //Class.initialize_basic_member_functions();
                ////print(Class.this_type.name());
                //// Class.for_each_function([&](GL::Proxy_Function const& f)->bool { print(f->m_signature.display()); return false; });

                ////auto& Class = root.make_class("vector_int");
                ////const_cast<GL::type&>(Class.this_type).add_base(BaseClass.this_type);
                ////Class.add_member_object("obj", GL::type_of<int>(), GL::any::fast_any::instance(0));
                ////Class.initialize_basic_member_functions();
                ////Class.add_function(GL::make_callable("[]", [&Class](GL::any::fast_any const& lhs, unsigned int rhs) -> GL::any::fast_any {
                ////    return Class.call("obj", { lhs });
                ////}, 0, {}, { { "lhs", Class.this_type | GL::type::Const | GL::type::Reference } , { "rhs", GL::type_of<unsigned int>() } }, GL::type_of<int&>()));
                //
                //if (auto this_scope = root.make_scope()) {
                //    auto vec = this_scope.call("vector<int>", {});
                //    EXPECT_EQ(vec.m_casted_type, Class.this_type);

                //    auto vec_obj = this_scope.call("[]", { vec, GL::any::fast_any::instance(0) });
                //    EXPECT_EQ(vec_obj.m_casted_type, GL::type_of<int&>());
                //    this_scope.call("=", { vec_obj, GL::any::fast_any::instance(10) });
                //    EXPECT_EQ(vec.cast<GL::dynamic_object>()["obj"]->cast<int>(), 10);
                //}
            }

            if (auto timer = sw.debug_timer("GPU matrix test")) {
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
                                print(this_scope.call("to_string", { for_scope.call("ASCII", { for_scope.find_object("state") }) }).cast<GL::string>());

                                // auto mat = this_scope.call("float_matrix::random", { GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) });
                                // this_scope.emplace_object_here("state", this_scope.call(">", { mat, GL::any::fast_any::instance(0.4) }));

                                //if (for_scope.call("<", { for_scope.call("avg", {for_scope.call("float_matrix", {for_scope.find_object("state")})}), GL::any::fast_any::instance(0.1) }).cast<bool>()) {
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




#endif
            }


            // Testing Scopes::Scopes
#if 0
            if (1) {
                GL::scope::impl::RootScope root; // successfully starts a new script root

                // >> TEST SCOPES
                GL::parallel::For(0, 1000000, [&](int i) {
                    ++i;
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    auto scope{ root.make_scope() };
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    auto& scope{ root.make_namespace("std") };
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    auto& scope{ root.make_namespace("std") };
                    scope.invalidate_cache();
                });

                // Test recursive update calls. Should only recurse one time until the "call num" saturates. 
                if (1) {
                    auto& scope1{ root.make_namespace("std") };
                    auto& scope2{ root.make_namespace("UI") };

                    scope2.add_using_here(scope1);
                    scope1.add_using_here(scope2);

                    scope1.invalidate_cache();
                    scope2.invalidate_cache();
                    root.invalidate_cache();
                }
                GL::parallel::For(0, 1000000, [&](int i) {
                    switch (i % 3) {
                    case 0: {
                        auto& scope1{ root.make_namespace("std") };
                        auto& scope2{ scope1.make_namespace("impl") };
                        auto scope3{ scope2.make_scope() };

                        scope3.add_using_here(scope2);
                        scope3.add_using_here(scope1);
                        EXPECT_EQ(false, scope3.add_using_here(root));

                        auto scope5{ scope3.make_scope() };
                        scope5.get_unique_index();

                        break;
                    }
                    case 1: {
                        auto& scope1{ root.make_namespace("std") };
                        auto& scope2{ scope1.make_namespace("string") };
                        auto& scope3{ scope2.make_namespace("impl") };
                        auto scope4{ scope3.make_scope() };

                        scope2.emplace_object_here("npos", 100); // slow due to conflict with GoodLang::shared_ptr... 

                        scope4.add_using_here(scope3);
                        scope4.add_using_here(scope2);
                        scope4.add_using_here(scope1);
                        EXPECT_EQ(false, scope4.add_using_here(root));

                        auto scope5{ scope3.make_scope() };
                        auto scope6{ scope4.make_scope() };
                        scope5.get_unique_index();
                        scope6.get_unique_index();
                        break;
                    }
                    case 2: {
                        auto& scope1{ root.make_namespace("string") };
                        auto& scope2{ scope1.make_namespace("impl") };
                        auto scope3{ scope2.make_scope() };

                        scope1.emplace_object_here("npos", 200);

                        scope3.add_using_here(scope2);
                        scope3.add_using_here(scope1);
                        EXPECT_EQ(false, scope3.add_using_here(root));

                        auto scope5{ scope3.make_scope() };
                        scope5.get_unique_index();
                        break;
                    }
                    }
                });
#if 1


#if 1

#endif // << NO LEAK

#if 1
                auto s = GL::string("::std::string::");
                //print(s.left_of("d::").c_str());
                //print(s.right_of("d::").c_str());
                //print(s.left_of("::std").c_str());
                //print(s.right_of("::std").c_str());
                //print(s.left_of("string::").c_str());
                //print(s.right_of("string::").c_str());

                //print(s.left_of_last("d::").c_str());
                //print(s.right_of_last("d::").c_str());
                //print(s.left_of_last("::std").c_str());
                //print(s.right_of_last("::std").c_str());
                //print(s.left_of_last("string::").c_str());
                //print(s.right_of_last("string::").c_str());

                EXPECT_EQ(true, s.ends_with("::"));
                EXPECT_EQ(true, s.begins_with("::"));

                EXPECT_NE(nullptr, root.find_namespace(GL::string("")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("::")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::string::")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::string::impl::")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("::string::")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("::string::impl::")));

                EXPECT_EQ(nullptr, root.find_namespace(GL::string("impl"))); // could not find "impl" from the root, which is (arguably) correct!             
                EXPECT_NE(nullptr, root.find_namespace(GL::string("std")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("std::string")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("std::string::impl")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("string"))->this_m.scope->find_namespace(GL::string("impl")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("string")));
                EXPECT_NE(nullptr, root.find_namespace(GL::string("string::impl")));

                EXPECT_NE(nullptr, root.find_namespace(GL::string("::std::string::"))->this_m.scope->find_object_here("npos"));
                EXPECT_NE(nullptr, root.find_object("::std::string::npos"));
                EXPECT_EQ(nullptr, root.find_object("npos")); // should not be successfully found.
                EXPECT_NE(nullptr, root.find_object("std::string::npos"));
                //EXPECT_EQ("100", GoodLang::ToString(**root.find_object("std::string::npos")));
                EXPECT_NE(nullptr, root.find_object("::string::npos"));
                //EXPECT_EQ("200", GoodLang::ToString(**root.find_object("::string::npos"))); 
                EXPECT_EQ(nullptr, root.find_object("::npos")); // should not be successfully found.

                EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("npos"));
                EXPECT_NE(nullptr, root.find_namespace("std")->this_m.scope->find_object("string::npos"));
                EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("string"));
                EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("string2::npos")); // this namespace does not exist and will not be found. 
                EXPECT_EQ(nullptr, root.find_object("std::npos")); // should not be successfully found.
                EXPECT_EQ(nullptr, root.find_object("std::string")); // should not be successfully found.
                EXPECT_EQ(nullptr, root.find_object("std::string2::npos")); // should not be successfully found.

                EXPECT_NE(nullptr, root.find_namespace("::string::impl::")->this_m.scope->find_object("npos"));
                EXPECT_NE(nullptr, root.find_namespace("std::string::impl::")->this_m.scope->find_object("npos"));

                //EXPECT_EQ("100", GoodLang::ToString(**root.find_namespace("std::string::impl::")->this_m.scope->find_object("npos")));
                //EXPECT_EQ("200", GoodLang::ToString(**root.find_namespace("::string::impl::")->this_m.scope->find_object("npos")));

                GL::parallel::For(0, 1000000, [&](int i) {
                    EXPECT_NE(nullptr, root.find_namespace("std")->this_m.scope->find_object("string::npos"));
                    EXPECT_NE(nullptr, root.find_object("std::string::npos"));
                });

                GL::parallel::For(0, 1000000, [&](int i) {
                    EXPECT_EQ(nullptr, root.find_object("std::string2::npos")); // should not be successfully found.
                });

                if (1) {
                    auto scope1{ root.make_scope() };
                    EXPECT_EQ(true, scope1.add_using_here(*scope1.find_namespace("::std::string::")->this_m.scope->GetNamespace()));
                    EXPECT_NE(nullptr, scope1.find_object("npos")); // should be successfully found now, due to the using statement.
                }

                if (1) {
                    auto scope1{ root.make_scope() };
                    EXPECT_EQ(true, scope1.add_using_here(*scope1.find_namespace("::std::string::")->this_m.scope->GetNamespace()));
                    EXPECT_NE(nullptr, scope1.find_object("npos")); // should be successfully found now, due to the using statement.
                }

                if (1) {
                    auto scope1{ root.make_scope() };
                    EXPECT_EQ(true, scope1.add_using_here(*root.find_namespace("::std::string::")->this_m.scope->GetNamespace()));
                    EXPECT_NE(nullptr, scope1.find_object("npos")); // should be successfully found now, due to the using statement.
                }

                if (1) {
                    EXPECT_EQ(false, root.add_using_here(*root.find_namespace("::std::string::")->this_m.scope->GetNamespace()));
                    EXPECT_EQ(nullptr, root.find_object("npos")); // should be successfully found now, due to the using statement.
                }
                EXPECT_NE(nullptr, root.find_object("::std::string::npos"));
                EXPECT_EQ(nullptr, root.find_object("npos"));
                EXPECT_EQ(nullptr, root.find_object("::npos"));
                // EXPECT_NE(nullptr, root.find_namespace("UI")->this_m.scope->find_object("npos"));

#endif // << NO LEAK
#endif // << NO LEAK AND NO CATCH FAILURE

            // >> TEST FUNCTION CALLS
#if 00
                Functions funcs;
                funcs.emplace("a", utilities::FunctionWrapper(GoodLang::make_callable([](void) -> int { return 0; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
                funcs.emplace("a", utilities::FunctionWrapper(GoodLang::make_callable([](int i) -> int { return i; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
                funcs.emplace("b", utilities::FunctionWrapper(GoodLang::make_callable([](int i) -> int { return i; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
                funcs.emplace("c", utilities::FunctionWrapper(GoodLang::make_callable([](int i, int j) -> int { return i + j; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
                funcs.emplace("d", utilities::FunctionWrapper(GoodLang::make_callable([](int i, int j, int k) -> int { return i + j + k; }), utilities::FunctionWrapper::FunctionState::Normal, { 10, 10, 10 })); // has defaults!

                funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                    [](int const& i, int const& j, int const& k) -> std::string { return "3 params"; }
                ), utilities::FunctionWrapper::FunctionState::Normal,
                    { 10, 10, 10 }));

                funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                    [](int const& i, int const& j) -> std::string { return "2 params"; }
                ), utilities::FunctionWrapper::FunctionState::Normal,
                    { 10, 10 }));

                funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                    [](int const& i) -> std::string { return "1 param"; }
                ), utilities::FunctionWrapper::FunctionState::Normal,
                    { 10 }));

                funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                    []() -> std::string { return "no params"; }
                ), utilities::FunctionWrapper::FunctionState::Normal,
                    {}));

                funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                    [](int const& i, int const& j, int const& k) -> std::string { return "3 params"; }
                ), utilities::FunctionWrapper::FunctionState::Normal,
                    { 10.0, 10, 10.0 }));

                funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                    [](int const& i, int const& j) -> std::string { return "2 params"; }
                ), utilities::FunctionWrapper::FunctionState::Normal,
                    { 10.0, 10 }));

                funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                    [](double const& i, double const& j) -> std::string { return "2 param doubles!"; }
                ), utilities::FunctionWrapper::FunctionState::Normal));

                GoodLang::TypeConverter converter;
                converter.AddConverter<bool, int>();
                converter.AddConverter<int, bool>();
                converter.AddConverter<double, int>();
                converter.AddConverter<int, double>();
                converter.AddConverter<float, int>();
                converter.AddConverter<int, float>();
                converter.AddConverter<bool, float>();
                converter.AddConverter<float, bool>();
                converter.AddConverter<double, float>();
                converter.AddConverter<float, double>();
                converter.AddConverter<bool, double>();
                converter.AddConverter<double, bool>();

                // including these conversion checks "fixes" it. IDK why. 
                print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<int>(), GoodLang::user_type_shared_ptr<double>()));
                print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<int>(), GoodLang::user_type_shared_ptr<double>()->MakeConstRef().lock()));
                print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<double>(), GoodLang::user_type_shared_ptr<int>()));
                print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<double>(), GoodLang::user_type_shared_ptr<int>()->MakeConstRef().lock()));





                EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes(), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function); // test providing more params than needed
                EXPECT_NE(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);

                print(GoodLang::ToString(funcs.Call("a", {}, converter)));
                print(GoodLang::ToString(funcs.Call("b", { 100.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("c", { 200.0, 200.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("d", { 500.0, 50, true }, converter)));
                print(GoodLang::ToString(funcs.Call("a", { 100, 200.0 }, converter)));
                EXPECT_EQ(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes(), converter).function);
                EXPECT_EQ(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes(), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
                EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes(), converter).function);
                print(GoodLang::ToString(funcs.Call("d", {}, converter)));

                print(GoodLang::ToString(funcs.Call("example", {}, converter)));
                print(GoodLang::ToString(funcs.Call("example", { 10 }, converter)));
                print(GoodLang::ToString(funcs.Call("example", { 10, 10 }, converter)));
                print(GoodLang::ToString(funcs.Call("example", { 10, 10, 10 }, converter)));
                print(GoodLang::ToString(funcs.Call("example", { 10, 10, 10, 10 }, converter)));

                print(GoodLang::ToString(funcs.Call("example", { 10.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0, 10.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0, 10.0, 10.0 }, converter)));

                print(GoodLang::ToString(funcs.Call("example2", { 10, 10 }, converter)));
                print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("example2", { 10, 10, 10 }, converter)));
                print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0, 10.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0, 10.0, 10.0 }, converter)));
                print(GoodLang::ToString(funcs.Call("example2", {}, converter)));
                print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10 }, converter))); // prefers the double-type since it keeps the first type
                print(GoodLang::ToString(funcs.Call("example2", { 10, 10.0 }, converter))); // prefers the int-type since it keeps the first type

#endif // << NO LEAK

       // TEST SEARCHING FOR SCOPES
#if 1
                GL::scope::impl::Breadcrumb
                    *nearest;

                nearest = nullptr;
                EXPECT_EQ(nullptr, root.find_namespace(GL::string("impl"), nearest)); // does not find it, but returns the root as the nearest location
                EXPECT_NE(nullptr, nearest);

                nearest = nullptr;
                EXPECT_NE(nullptr, root.find_namespace(GL::string("std::string::impl"), nearest)); // successfully finds it
                EXPECT_NE(nullptr, nearest);

                nearest = nullptr;
                EXPECT_NE(nullptr, root.find_namespace(GL::string("std::impl"), nearest)); // successfully finds it
                EXPECT_NE(nullptr, nearest);

                nearest = nullptr;
                EXPECT_NE(nullptr, root.find_namespace(GL::string("string::impl"), nearest)); // successfully finds it
                EXPECT_NE(nullptr, nearest);

                nearest = nullptr;
                EXPECT_EQ(nullptr, root.find_namespace(GL::string("string::impl::impl"), nearest)); // does not find it, but does locate the nearest location
                EXPECT_NE(nullptr, nearest);

                GL::parallel::For(0, 1000000, [&](int i) {
                    auto scope{ root.make_scope() };
                });

                GL::parallel::For(0, 1000000, [&](int i) {
                    auto scope{ root.make_scope() };
                    scope.emplace_object_here(GL::printf("%i", i), GL::any(i)); // x = 100.0;
                });

                GL::parallel::For(0, 1000000, [&](int i) {
                    auto scope{ root.make_scope() };
                    scope.emplace_object_here(GL::printf("%i", i), GL::any(i)); // x = 100.0;
                    if (auto* p = scope.find_object_here(GL::printf("%i", i))) {}
                    else EXPECT_EQ(true, false);                    
                });

#endif // << NO LEAK

            }
#endif // << NO LEAK

#if 0
            if (1) {
                GL::scope::impl::RootScope this_root;
                GL::any
                    * x,
                    * npos;
                auto& std_namespace = this_root.make_namespace("std");
                auto& std_string_namespace = std_namespace.make_namespace("string"); {
                    std_string_namespace.emplace_object_here("npos", std::string::npos);
                    // EXPECT_NE(std_string_namespace.find_object("std::string::npos"), nullptr);
                    // EXPECT_NE(std_namespace.find_object("std::string::npos"), nullptr);
                    // EXPECT_NE(root.find_object("std::string::npos"), nullptr);
                }
                auto& std_vector_namespace = std_namespace.make_namespace("vector");
                auto& std_map_namespace = std_namespace.make_namespace("map");
                auto& std_set_namespace = std_namespace.make_namespace("set");
                auto& std_unordered_map_namespace = std_namespace.make_namespace("unordered_map");
                auto& std_unordered_set_namespace = std_namespace.make_namespace("unordered_set");

                auto function_scope = this_root.make_scope();
                function_scope.emplace_object_here("x", GL::any(0.0));
                x = function_scope.find_object("x");
                npos = function_scope.find_object("std::string::npos");
                if ((x != nullptr) && (npos != nullptr)) {
                    auto callable = GL::make_callable("+", [](double a, size_t b) -> size_t { return b; });
                    EXPECT_EQ(std::string::npos, callable->operator()({ *x, *npos }).cast<size_t>());
                }
                else {
                    EXPECT_EQ(true, false);
                    x = function_scope.find_object("x");
                    EXPECT_NE(nullptr, x);
                    npos = function_scope.find_object("std::string::npos");
                    EXPECT_NE(nullptr, npos);
                    npos = function_scope.find_object("std::string::npos");
                }


            }
#endif

#if 0
            if (1) {
                GL::function_signature sig(
                    "sum",
                    GL::type_of<int>(),
                    { { "a", GL::type_of<int const&>() }, { "b", GL::type_of<int const&>() } },
                    {}
                );
                EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int>(), GL::type_of<int>() }));
                EXPECT_EQ(true, sig.can_call_with_free_cast({ GL::type_of<int>(), GL::type_of<int>() }));

                EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));
                EXPECT_EQ(true, sig.can_call_with_free_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));

                EXPECT_EQ(false, sig.can_call_with_cast({ GL::type_of<int const&>() }));
                EXPECT_EQ(false, sig.can_call_with_free_cast({ GL::type_of<int const&>() }));
            }
            if (1) {
                GL::function_signature sig(
                    "sum",
                    GL::type_of<int>(),
                    { { "a", GL::type_of<int>() }, { "b", GL::type_of<int>() } },
                    { GL::any{ 0 }, GL::any{ 0 } }
                );
                EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int>(), GL::type_of<int>() }));
                EXPECT_EQ(true, sig.can_call_with_free_cast({ GL::type_of<int>(), GL::type_of<int>() }));

                EXPECT_EQ(true, sig.can_call_with_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));
                EXPECT_EQ(false, sig.can_call_with_free_cast({ GL::type_of<int const&>(), GL::type_of<int const&>() }));
                EXPECT_EQ(true, sig.can_call_with_cast({  }));
                EXPECT_EQ(true, sig.can_call_with_free_cast({  }));
            }
            if (1) {
                GL::details::Explicit_Function_Impl function(std::function([](int i) -> int {
                    EXPECT_EQ(i, 100);
                    return i + 1;
                    }));
                EXPECT_EQ(101, function.operator()({ 100 }).cast<int>());
            }
            if (1) {
                GL::details::Explicit_Function_Impl function([](int i) -> int {
                    return i + 1;
                    }, { 0 });
                EXPECT_EQ(101, function.operator()({ 100 }).cast<int>());
                EXPECT_EQ(1, function.operator()({}).cast<int>());
            }
            if (1) {
                GL::details::Explicit_Function_Impl function([]() -> double {
                    return 1.0;
                    });
                EXPECT_EQ(1.0, function.operator()({}).cast<double>());
            }
            if (1) {
                class temp {
                public:
                    int x;
                    double y;

                    temp() : x{ 0 }, y{ 0 } {};
                    temp(int X, double Y) : x{ X }, y{ Y } {};
                    temp(temp const&) = default;
                    temp(temp&&) = default;
                    temp& operator=(temp const&) = default;
                    temp& operator=(temp&&) = default;
                    ~temp() = default;

                    static double FUNC() {
                        return 100.0;
                    };
                    double SUM() {
                        return x + y;
                    };
                    double CONST_SUM() const noexcept {
                        return x + y;
                    };
                    int& Increment() {
                        return x;
                    };
                    const int& Get() const {
                        return x;
                    };
                };

                if (1) {
                    GL::details::Attribute_Access_Impl function(&temp::x);
                    EXPECT_EQ(100, function.operator()({ temp(100, 200.0) }).cast<int>());

                    auto T_ptr = GL::make_shared<temp>();
                    if (1) {
                        T_ptr->x = 100;
                        T_ptr->y = 200.0;
                        EXPECT_EQ(100, function.operator()({ GL::any(GL::shared_ptr<temp>(T_ptr)) }).cast<int>());
                    }
                    if (1) {
                        T_ptr->x = 500;
                        EXPECT_EQ(500, function.operator()({ GL::any(GL::shared_ptr<temp>(T_ptr)) }).cast<int>());
                    }
                }
                if (1) {
                    GL::details::Static_Function_Impl function(&temp::FUNC);
                    EXPECT_EQ(100.0, function().cast<double>());
                }
                if (1) {
                    GL::details::Default_Member_Function_Impl function(&temp::SUM);
                    EXPECT_EQ(300.0, function({ temp(100, 200.0) }).cast<double>());
                }
                if (1) {
                    GL::details::Const_Member_Function_Impl function(&temp::CONST_SUM);
                    EXPECT_EQ(300.0, function({ temp(100, 200.0) }).cast<double>());
                }

                EXPECT_EQ(300.0, GL::make_callable("CONST_SUM", &temp::CONST_SUM)->operator()({ temp(100, 200.0) }).cast<double>());
                EXPECT_EQ(300.0, GL::make_callable("SUM", &temp::SUM)->operator()({ temp(100, 200.0) }).cast<double>());
                EXPECT_EQ(100, GL::make_callable("x", &temp::x)->operator()({ temp(100, 200.0) }).cast<int>());
                EXPECT_EQ(100.0, GL::make_callable("FUNC", &temp::FUNC)->operator()().cast<double>());

                EXPECT_EQ(decl_func(&temp::CONST_SUM)->m_signature.name_m, "CONST_SUM");

                decl_func(&temp::CONST_SUM, GL::function_signature::Constant, {}, { "parent" });
                EXPECT_EQ("int&", decl_func(&temp::x, GL::function_signature::Async, {}, { {"parent", GL::type_of<temp&>()} })->operator()({ temp(100, 200.0) }).m_casted_type.name());
                EXPECT_EQ("const int&", decl_func(&temp::x, GL::function_signature::Async | GL::function_signature::Constant, {}, { { "parent", GL::type_of<temp const&>() } })->operator()({ GL::any(temp(100, 200.0)) + GL::type::Const }).m_casted_type.name());

                GL::decl_func(&temp::x);
                GL::decl_func(&temp::y, {}, {});


                EXPECT_EQ("int&", GL::decl_func(&temp::Increment)->operator()({ temp(100, 200.0) }).m_casted_type.name());
                EXPECT_EQ("const int&", GL::decl_func(&temp::Get)->operator()({ GL::any(temp(100, 200.0)) + GL::type::Const }).m_casted_type.name());

                if (1) {
                    GL::any Temp;
                    if (1) {
                        Temp = GL::decl_func(&temp::Get)->operator()({ GL::any(temp(100, 200.0)) + GL::type::Const });
                    }
                    EXPECT_EQ(100, Temp.cast<int>());
                }

                if (1) {
                    auto converter_func = GL::make_callable("int", [](double x) -> int {
                        return static_cast<int>(x);
                        }, GL::function_signature::Async | GL::function_signature::Constant | GL::function_signature::Static);
                    auto to_convert = GL::any(100.0).fast();

                    EXPECT_EQ(100, (*converter_func)(&to_convert, &to_convert + 1).cast<int>());
                }
                if (1) {
                    auto to_convert = GL::any(100.0).fast();
                    EXPECT_EQ(100, (*GL::make_converter<double, int>())(&to_convert, &to_convert + 1).cast<int>());
                }
                if (1) {
                    auto to_convert = GL::any(GL::foot(100.0f)).fast();
                    EXPECT_EQ(100, (*GL::make_converter<GL::foot, int>())(&to_convert, &to_convert + 1).cast<int>());
                }
                if (1) {
                    auto to_convert = GL::any(100).fast();
                    EXPECT_EQ(GL::foot(100.0f), (*GL::make_converter<int, GL::foot>())(&to_convert, &to_convert + 1).cast<GL::foot>());
                }
                if (1) {
                    auto to_convert = GL::any(GL::meter(100.0f)).fast();
                    EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::foot>())(&to_convert, &to_convert + 1).cast<GL::foot>());
                }
                // because value and can constructed from a meter, and the user is requesting as-value, it will construct a new object.
                if (1) {
                    auto to_convert = GL::any(GL::meter(100.0f)).fast();
                    EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::value>())(&to_convert, &to_convert + 1).cast<GL::value>());
                }
                // because temp and can constructed from a temp2, and the user is requesting as-value, it will construct a new object.
                if (1) {
                    class temp2 final : public temp {};
                    auto to_convert = GL::any(temp2()).fast();
                    EXPECT_EQ(0, (*GL::make_converter<temp2, temp>())(&to_convert, &to_convert + 1).cast<temp>().x);
                }
                // because foot is not a base of meter, it will construct a new object.
                if (1) {
                    auto to_convert = GL::any(GL::meter(100.0f)).fast();
                    EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::foot&>())(&to_convert, &to_convert + 1).cast<GL::foot>());
                }
                // because a value is base of meter AND we are requesting a reference or pointer, this will perform a polymorphic cast.
                if (1) {
                    auto to_convert = GL::any(GL::meter(100.0f)).fast();
                    EXPECT_EQ(GL::meter(100.0f), (*GL::make_converter<GL::meter, GL::value&>())(&to_convert, &to_convert + 1).cast<GL::value>());
                }
                // because a temp is base of temp2 AND we are requesting a reference or pointer, this will perform a polymorphic cast.
                if (1) {
                    class temp2 final : public temp {};
                    auto to_convert = GL::any(temp2()).fast();
                    EXPECT_EQ(0, (*GL::make_converter<temp2, temp&>())(&to_convert, &to_convert + 1).cast<temp>().x);
                }

                if (1) {
                    GL::script_type Type("CustomString");
                    auto callable = GL::make_callable("custom_function", [&Type](GL::dynamic_object const& x) {
                        EXPECT_EQ(x.m_type, Type.load());
                        return x.m_type;
                        }, 0, {}, { { "Parent", Type.load() } });
                    auto temp_obj = GL::make_shared< GL::dynamic_object >(Type.load());
                    EXPECT_EQ((*callable)({ temp_obj }).cast<GL::type>(), Type.load());
                }




            }

            // prove that GL::shared_ptr supports custom deleter functions. Note that these are always called on a different thread than the pointer was made on... 
            GL::shared_ptr<int> temp_ptr(new int(100), [](int* p) {
                EXPECT_EQ(*p, 100);
                delete p;
            });

            // check GL::value and GL::datetime
            if (1) {
                GL::value val{ 10.0f };
                val = 10.0f;
                val = 10;
                val = 10.0;
                val = 10ull;
                val = val;

                EXPECT_EQ(10, (int)(float)val);
                EXPECT_EQ(true, val.is_scalar());

                GL::value meter(GL::value::get_si_unit(1, 0, 0, 0, 0, 0).get_impl_unit(1.0, "meter", "m"));
                EXPECT_EQ(false, meter.is_scalar());
                EXPECT_EQ(0, (int)(float)meter);
                EXPECT_EQ(meter.name(), "meter");
                EXPECT_EQ(meter.abbreviation(), "m");

                meter += GL::value(0);
                EXPECT_EQ(false, meter.is_scalar());
                EXPECT_EQ(0, (int)(float)meter);
                meter -= GL::value(0);
                EXPECT_EQ(false, meter.is_scalar());
                EXPECT_EQ(0, (int)(float)meter);
                meter *= GL::value(0);
                EXPECT_EQ(false, meter.is_scalar());
                EXPECT_EQ(0, (int)(float)meter);
                meter /= GL::value(1);
                EXPECT_EQ(false, meter.is_scalar());
                EXPECT_EQ(0, (int)(float)meter);

                GL::value foot(GL::value::get_si_unit(1, 0, 0, 0, 0, 0).get_impl_unit(381.0 / 1250.0, "foot", "ft"));
                EXPECT_EQ(false, foot.is_scalar());

                GL::value inch(GL::value::get_si_unit(1, 0, 0, 0, 0, 0).get_impl_unit((1.0 / 12.0) * (381.0 / 1250.0), "inch", "in"));
                EXPECT_EQ(false, inch.is_scalar());

                GL::value square_meter(GL::value::get_si_unit(2, 0, 0, 0, 0, 0).get_impl_unit(1.0, "square_meter", "sq_m"));
                EXPECT_EQ(false, square_meter.is_scalar());

                GL::value square_foot(GL::value::get_si_unit(2, 0, 0, 0, 0, 0).get_impl_unit(((381.0 / 1250.0) * (381.0 / 1250.0)), "square_foot", "sq_ft"));
                EXPECT_EQ(false, square_foot.is_scalar());

                GL::value cubic_meter(GL::value::get_si_unit(3, 0, 0, 0, 0, 0).get_impl_unit(1.0, "cubic_meter", "cu_m"));
                EXPECT_EQ(false, cubic_meter.is_scalar());

                GL::value scalar;
                EXPECT_EQ(0, (int)(float)scalar);
                EXPECT_EQ(true, scalar.is_scalar());
                EXPECT_EQ(scalar.name(), "scalar");

                GL::value scalar2(GL::value::get_si_unit(0, 0, 0, 0, 0, 0).get_impl_unit(1, "scalar", ""));
                EXPECT_EQ(0, (int)(float)scalar2);
                EXPECT_EQ(true, scalar2.is_scalar());
                EXPECT_EQ(scalar2.name(), "scalar");

                meter = 0.0f;
                meter += 10.0f;
                EXPECT_EQ(10, (int)(float)meter);
                foot += 1.0f;
                EXPECT_EQ(1, (int)(float)foot);
                inch += 12.0f;
                EXPECT_EQ(12, (int)(float)inch);
                foot += inch;
                EXPECT_EQ(2, (int)(float)foot);
                scalar += 100.0f;
                EXPECT_EQ(100, (int)(float)scalar);

                cubic_meter += 1;
                EXPECT_EQ(1, (int)(float)cubic_meter);

                cubic_meter += scalar;
                EXPECT_EQ(101, (int)(float)cubic_meter);

                try { // expected to throw an error, because adding an inch to a cubic meter is nonsense. 
                    cubic_meter += inch;
                    EXPECT_EQ(true, false);
                }
                catch (...) {}

                auto manual_sq_m = meter * meter;
                EXPECT_EQ(manual_sq_m.abbreviation(), "sq_m");
                EXPECT_EQ(manual_sq_m.name(), "square_meter");
                auto manual_cu_m = manual_sq_m * meter;
                EXPECT_EQ(manual_cu_m.abbreviation(), "cu_m");
                EXPECT_EQ(manual_cu_m.name(), "cubic_meter");
                auto manual_sq_ft = foot * foot;
                EXPECT_EQ(manual_sq_ft.abbreviation(), "sq_ft");
                EXPECT_EQ(manual_sq_ft.name(), "square_foot");
                auto manual_cu_ft = manual_sq_ft * foot;
                EXPECT_EQ(manual_cu_ft.abbreviation(), "cu_ft");
                EXPECT_EQ(manual_cu_ft.name(), "cubic_foot");
                auto manual_sq_in = inch * inch;
                EXPECT_EQ(manual_sq_in.abbreviation(), "sq_in");
                EXPECT_EQ(manual_sq_in.name(), "square_inch");
                auto manual_scalar = manual_cu_ft / manual_cu_m;
                EXPECT_EQ(manual_scalar.abbreviation(), "");
                EXPECT_EQ(manual_scalar.name(), "scalar");
                EXPECT_EQ(GL::foot(100), GL::foot(100));
                EXPECT_EQ(GL::meter(GL::foot(100)), GL::foot(100));
                EXPECT_EQ(GL::millimeter(1000), GL::meter(1));
                EXPECT_EQ(GL::megameter(1), GL::meter(1000000));
                EXPECT_EQ(GL::second(60), GL::minute(1));
                EXPECT_EQ(GL::miles_per_hour(1), (GL::mile(1) / GL::hour(1)));

                if (1) {
                    using namespace GL::literals;
                    EXPECT_EQ(100_ft, 100_ft);
                    EXPECT_EQ(GL::meter(100_ft), 100_ft);
                    EXPECT_EQ(1000_mm, 1_m);
                    EXPECT_EQ(1_Mm, 1000000_m);
                    EXPECT_EQ(60_s, 1_min);
                    EXPECT_EQ(1_mph, 1_mi / 1_hr);
                    GL::datetime DT1 = GL::datetime(2025, 1, 1, 0, 0, 0);
                    GL::datetime DT2 = GL::datetime(2025, 1, 1, 0, 0, 1.05f);
                    EXPECT_EQ(DT2 - DT1, 1.05_s);
                    EXPECT_EQ(365, (int)(float)(GL::day((DT1 + 365_d) - DT1)));
                    EXPECT_EQ(DT1.ToNextDay() - DT1.ToStartOfDay(), GL::day(1));
                    EXPECT_EQ(DT1.ToNextHour() - DT1.ToStartOfHour(), GL::hour(1));
                    EXPECT_EQ(DT1.ToNextMinute() - DT1.ToStartOfMinute(), GL::minute(1));
                }

                if (1) {
                    GL::foot v{ 100 };
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        ++v;
                    });
                    EXPECT_EQ((int)(float)v, 1000100);
                }
                if (1) {
                    GL::foot v{ 0 };
                    GL::scalar s{ 0 };
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        v *= s;
                    });
                    EXPECT_EQ((int)(float)v, 0);
                }
                if (1) {
                    GL::foot v = 0;
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        for (;;) {
                            GL::foot expected = v;
                            if (v.compare_exchange(expected, expected + 1)) {
                                break;
                            }
                        }
                    });
                    EXPECT_EQ((int)(float)v, 1000000);
                }
                if (1) {
                    GL::datetime v = GL::datetime(2025, 1, 1, 0, 0, 0);
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        v += GL::minute(1);
                    });
                    EXPECT_EQ((int)(float)GL::minute(v - GL::datetime(2025, 1, 1, 0, 0, 0)), 1000000);
                }
                if (1) {
                    GL::datetime v = GL::datetime(2025, 1, 1, 0, 0, 0);
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        for (;;) {
                            GL::datetime expected = v;
                            if (v.compare_exchange(expected, expected.ToNextMinute())) {
                                break;
                            }
                        }
                    });
                    EXPECT_EQ((int)(float)GL::minute(v - GL::datetime(2025, 1, 1, 0, 0, 0)), 1000000);
                }


            }

            // check GL::type
            if (1) {
                GL::type ti = GL::type_of<std::string>();
                EXPECT_EQ(false, ti.is_void());
                EXPECT_EQ(true, ti.is_cpp_type());
                EXPECT_EQ(true, ti.is_base());
                EXPECT_EQ(false, ti.is_const());
                EXPECT_EQ(false, ti.is_ref());
                EXPECT_EQ(false, ti.is_temp());
                ti |= GL::type::Const;
                EXPECT_EQ(false, ti.is_void());
                EXPECT_EQ(true, ti.is_cpp_type());
                EXPECT_EQ(true, ti.is_const());
                EXPECT_EQ(false, ti.is_base());
                EXPECT_EQ(false, ti.is_ref());
                EXPECT_EQ(false, ti.is_temp());
            }
            if (1) {
                GL::type ti = GL::type_of<const std::string&>();
                EXPECT_EQ(false, ti.is_void());
                EXPECT_EQ(false, ti.is_base());
                EXPECT_EQ(true, ti.is_cpp_type());
                EXPECT_EQ(true, ti.is_const());
                EXPECT_EQ(true, ti.is_ref());
                EXPECT_EQ(false, ti.is_temp());
            }
            if (1) {
                GL::type ti;
                EXPECT_EQ(true, ti.is_void());
                EXPECT_EQ(true, ti.is_base());
                EXPECT_EQ(true, ti.is_cpp_type());
                EXPECT_EQ(ti.name(), "void");
            }
            if (1) {
                GL::type ti = GL::type_of<void>();
                EXPECT_EQ(true, ti.is_void());
                EXPECT_EQ(true, ti.is_base());
                EXPECT_EQ(true, ti.is_cpp_type());
                EXPECT_EQ(ti.name(), "void");
            }
            if (1) {
                GL::script_type custom_type("string");
                GL::type ti = custom_type;
                EXPECT_EQ(false, ti.is_void());
                EXPECT_EQ(true, ti.is_base());
                EXPECT_EQ(false, ti.is_cpp_type());
                EXPECT_EQ(false, ti.is_const());
                EXPECT_EQ(false, ti.is_ref());
                EXPECT_EQ(false, ti.is_temp());
                EXPECT_EQ(ti.name(), "string");
                ti |= GL::type::Const;
                ti |= GL::type::Reference;
                EXPECT_EQ(false, ti.is_void());
                EXPECT_EQ(false, ti.is_base());
                EXPECT_EQ(false, ti.is_cpp_type());
                EXPECT_EQ(true, ti.is_const());
                EXPECT_EQ(true, ti.is_ref());
                EXPECT_EQ(false, ti.is_temp());
                EXPECT_EQ(ti.name(), "const string&");

                // a second type (with the same name!) being named by a different class should have a different hash, and be recognized as a different type. 
                GL::script_type custom_type2("string");
                GL::type ti2 = custom_type2;
                EXPECT_EQ(false, ti2.is_void());
                EXPECT_EQ(true, ti2.is_base());
                EXPECT_EQ(false, ti2.is_cpp_type());
                EXPECT_EQ(false, ti2.is_const());
                EXPECT_EQ(false, ti2.is_ref());
                EXPECT_EQ(false, ti2.is_temp());
                EXPECT_EQ(ti2.name(), "string");
                ti2 |= GL::type::Const;
                ti2 |= GL::type::Reference;
                EXPECT_EQ(false, ti2.is_void());
                EXPECT_EQ(false, ti2.is_base());
                EXPECT_EQ(false, ti2.is_cpp_type());
                EXPECT_EQ(true, ti2.is_const());
                EXPECT_EQ(true, ti2.is_ref());
                EXPECT_EQ(false, ti2.is_temp());
                EXPECT_EQ(ti2.name(), "const string&");

                EXPECT_EQ(false, (bool)(ti == ti2));
                EXPECT_EQ(true, (bool)(ti != ti2));
            }

            // check GL::any, including casting and multi-threaded overwrites and access. 
            if (1) {
                using namespace GL;
                using namespace GL::type_erasure;
                if (1) {
                    shared_data<std::string> instanced(GL::make_shared<std::string>("TEST"));
                    // EXPECT_EQ(instanced.m_ptr, "TEST");
                    std::string* p = static_cast<std::string*>(instanced.m_data);
                    EXPECT_EQ(*p, "TEST");
                }
                if (auto instanced = new shared_data<std::string>(GL::make_shared<std::string>("TEST"))) {
                    // EXPECT_EQ(instanced->m_ptr, "TEST");
                    std::string* p = static_cast<std::string*>(instanced->m_data);
                    EXPECT_EQ(*p, "TEST");
                    delete instanced;
                }
                if (auto instanced = GL::make_shared<shared_data<std::string>>(GL::make_shared<std::string>("TEST"))) {
                    // EXPECT_EQ(instanced.get()->m_ptr, "TEST");
                    std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                    EXPECT_EQ(*p, "TEST");
                }
                if (auto instanced = GL::static_pointer_cast<any_data>(GL::make_shared<shared_data<std::string>>(GL::make_shared<std::string>("TEST")))) {
                    std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                    EXPECT_EQ(*p, "TEST");
                }
                if (1) {
                    GL::atomic_shared_ptr< any_data > atomic{ GL::static_pointer_cast<any_data>(GL::make_shared<shared_data<std::string>>(GL::make_shared<std::string>("TEST"))) };
                    if (auto instanced = atomic.load()) {
                        std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                        EXPECT_EQ(*p, "TEST");
                    }
                    if (auto instanced = atomic.load_fast()) {
                        std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                        EXPECT_EQ(*p, "TEST");
                    }
                }

                if (1) {
                    GL::any wrap; {
                        wrap = std::string("TEST");

                        EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string>()));
                        EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string const&>()));
                        EXPECT_EQ(true, wrap.can_free_cast(GL::type_of<std::string>()));
                        EXPECT_EQ(true, wrap.can_free_cast(GL::type_of<std::string const&>()));

                        wrap += GL::type::Const | GL::type::Reference;

                        EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string const&>()));
                        EXPECT_EQ(true, wrap.can_cast(GL::type_of<std::string>()));
                        EXPECT_EQ(false, wrap.can_free_cast(GL::type_of<std::string>())); // cannot free-cast from const& to && because it requires a constructor. 
                        EXPECT_EQ(true, wrap.can_free_cast(GL::type_of<std::string const&>()));

                        EXPECT_EQ(wrap.cast<std::string>(), "TEST");
                        if (auto p = wrap.cast<GL::shared_ptr<std::string>>()) {
                            EXPECT_EQ(*p, "TEST");
                        }
                        EXPECT_EQ(wrap.cast<std::string const&>(), "TEST");
                    }
                }

                if (1) {
                    if (1) {
                        any wrap;
                        GL::parallel::For(0, 1000000, [&](size_t const& index) {
                            wrap = std::to_string(index);
                            });
                    }
                    if (1) {
                        any wrap = GL::string("TEST");
                        GL::parallel::For(0, 1000000, [&](size_t const& index) {
                            auto& ptr = wrap.cast<GL::string>();
                            EXPECT_EQ(ptr, GL::string("TEST"));
                            });
                    }
                    if (1) {
                        any wrap = GL::string("TEST");
                        GL::parallel::For(0, 1000000, [&](size_t const& index) {
                            auto ptr = wrap.cast<GL::shared_ptr<GL::string>>();
                            EXPECT_EQ(*ptr, GL::string("TEST"));
                        });
                    }
                    if (1) {
                        any wrap{ GL::string("TEST") };
                        GL::parallel::For(0, 1000000, [&](size_t const& index) {
                            wrap = GL::string("TEST");
                            auto cmp = GL::string("TEST");
                            if (auto ptr = wrap.cast<GL::shared_ptr<GL::string>>()) {
                                EXPECT_EQ(*ptr, GL::string("TEST"));
                            }
                            else {
                                EXPECT_EQ(false, true);
                            }
                        });
                    }
                    if (1) {
                        GL::var wrap(GL::make_shared<any>(GL::string("TEST")));
                        GL::parallel::For(0, 1000000, [&](size_t const& index) {
                            wrap = GL::var(GL::make_shared<any>(GL::string("TEST")));
                            if (auto ptr = wrap.p_data.load_fast()) {
                                if (auto ptr2 = ptr->cast<GL::shared_ptr<GL::string>>()) {
                                    EXPECT_EQ(*ptr2, GL::string("TEST"));
                                }
                            }
                            });
                    }
                    if (1) {
                        any temp = 100;
                        any temp2 = temp.m_casted_type.instance_by_copy(temp);
                        temp2.cast<int>() += 100;
                        EXPECT_EQ(100, temp.cast<int&>());
                        EXPECT_EQ(200, temp2.cast<int&>());
                        EXPECT_EQ(100, GL::type_of<int>().instance_by_value(100.0f).cast<int>());
                    }
                }
            }

#if 0
            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task(0, 1000000, [&](size_t i) {
                    ++L;
                    })->and_then([&]() {
                        EXPECT_EQ(1000000, L.load());
                        });
                    EXPECT_EQ(1000000, L.load());
            }
            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 1;
                    return L.load();
                    })->and_then([&](GL::job_base& parent) {
                        auto V = parent.result.cast<long long>();
                        EXPECT_EQ(1, L.load());
                        EXPECT_EQ(1, V);
                        });
                    EXPECT_EQ(1, L.load());
            }

            if (1) {
                auto ptr = GL::parallel::task([]() {
                    print("I was Async 2");
                    });
            }
            if (1) {
                GL::parallel::task([]() {
                    print("I was Async 3");
                    });
            }
            if (1) {
                GL::parallel::task([]() {
                    ::Sleep(1000);
                    print("I was Async 1");
                    })->and_then([]() {
                        print("I was Async 2");
                        });
            }
            if (1) {
                auto job1 = GL::parallel::task([]() {
                    ::Sleep(1000);
                    print("I was Async 1");
                    });
                auto job2 = job1->and_then([]() {
                    print("I was Async 2");
                    });
            }

            if (1) {
                auto job1 = GL::parallel::task([]() {
                    ::Sleep(1000);
                    print("I was Async 1");
                    });
                auto job2 = job1->and_then([]() {
                    print("I was Async 2");
                    });
                job2 = nullptr;
                job1 = nullptr;
            }

            if (1) {
                GL::parallel::task([]() {
                    ::Sleep(1000);
                    print("I was Async 1");
                    })->and_then([]() {
                        print("I was Async 2");
                        });
            }
            if (1) {
                auto job = GL::parallel::task([]() {
                    print("1");
                    ::Sleep(1000);
                    print("2");
                    return 10;
                    });
                job->and_then([]() {
                    print("3");
                    })->and_then([]() {
                        ::Sleep(1000);
                        print("4");
                        })->and_then([]() {
                            print("5");
                            })->and_then([]() {
                                print("6");
                                });
                            job->wait(); // waits for only this job, and does not wait for its children. 
                            print(job->result.cast<int>());
            }
            if (1) {
                auto job = GL::parallel::task([]() {
                    print("1");
                    ::Sleep(1000);
                    print("2");
                    return 10;
                    });
                job->and_then([]() {
                    print("3");
                    })->and_then([]() {
                        ::Sleep(1000);
                        print("4");
                        })->and_then([]() {
                            print("5");
                            })->and_then([]() {
                                print("6");
                                });
                            job->wait(); // waits for only this job, and does not wait for its children. 
                            print(job->result.cast<int>());
            }
            if (1) { // if (auto timer = sw.debug_timer("Inline Test")) {
                size_t out = 0;
                std::vector<size_t> jobs;
                {
                    jobs.resize(1000000, 0);
                    for (size_t i = 0; i < 1000000; ++i) {
                        EXPECT_EQ(1000000, jobs.size());
                        auto start = GL::clock::ns();
                        while ((GL::clock::ns() - start) < 1000) {}
                        ++jobs[i];
                    }
                    out = std::accumulate(jobs.begin(), jobs.end(), 0);
                }
                EXPECT_EQ(1000000, out);
            }
            if (1) { // if (auto timer = sw.debug_timer("Parallel Jobs Test 1")) {
                size_t out = 0; {
                    auto job1 = GL::parallel::task([&]() {
                        std::vector<size_t> jobs;
                        jobs.resize(1000000, 0);
                        return jobs;
                        });
                    job1->and_then(0, 1000000, [&](size_t i, GL::job_base& parent) {
                        auto& jobs = parent.result.cast< std::vector<size_t> >();
                        EXPECT_EQ(1000000, jobs.size());
                        auto start = GL::clock::ns();
                        while ((GL::clock::ns() - start) < 1000) {}
                        ++jobs[i];
                        })->and_then([job1, &out]() {
                            std::vector<size_t>& jobs = job1->result.cast();
                            EXPECT_EQ(1000000, jobs.size());
                            out = std::accumulate(jobs.begin(), jobs.end(), 0);
                            EXPECT_EQ(1000000, out);
                            print("success?");
                            });
                        job1->wait();
                }
                EXPECT_EQ(1000000, out);
            }
            if (1) { // if (auto timer = sw.debug_timer("Parallel Jobs Test 2")) {
                size_t out = 0;
                {
                    std::vector<size_t> jobs;
                    jobs.resize(1000000, 0);
                    GL::parallel::task(0, 1000000, [&](size_t i) {
                        EXPECT_EQ(1000000, jobs.size());
                        auto start = GL::clock::ns();
                        while ((GL::clock::ns() - start) < 1000) {}
                        ++jobs[i];
                        });
                    out = std::accumulate(jobs.begin(), jobs.end(), 0);

                }
                EXPECT_EQ(1000000, out);
            }
            if (1) { // if (auto timer = sw.debug_timer("Parallel Jobs Test 3")) {
                size_t out = 0;
                {
                    std::vector<size_t> jobs;
                    jobs.resize(1000000, 0);
                    GL::parallel::For(0, 1000000, [&](size_t i) {
                        EXPECT_EQ(1000000, jobs.size());
                        auto start = GL::clock::ns();
                        while ((GL::clock::ns() - start) < 1000) {}
                        ++jobs[i];
                        });
                    out = std::accumulate(jobs.begin(), jobs.end(), 0);
                }
                EXPECT_EQ(1000000, out);
            }


#else
// to-do, re-enable
#if 0
            (void)GL::parallel::async([]() {
                return std::string("TEST 0");
            }).wait();

            (void)GL::parallel::async([](int i) {
                EXPECT_EQ(i, 10);
                return std::string("TEST 1");
            }, 10).wait();

            (void)GL::parallel::async([](int& i, int j) {
                EXPECT_EQ(i, 10);
                EXPECT_EQ(j, 10);
                return std::string("TEST 2");
            }, 10, 10).wait();

            (void)GL::parallel::async([](int& i, int* j, double k) {
                EXPECT_EQ(i, 10);
                EXPECT_EQ(*j, 10);
                EXPECT_EQ((int)k, 10);
                return std::string("TEST 3");
            }, 10, 10, 10.0).wait();

            // will complete immediately since it has to wait on destruction
            GL::parallel::async([](GL::string& i, float& j, double& k, int& L) {
                (void)(i + " World -> " + GL::printf("%f %f %i", j, k, L));
            }, GL::string("Hello"), 1.0f, 2.0, 3);

            EXPECT_EQ(true, GL::type_of<GL::foot>().is_derived_from(GL::type_of<GL::value>()));
            EXPECT_EQ(true, GL::type_of<GL::millinewton>().is_derived_from(GL::type_of<GL::value>()));
            EXPECT_EQ(true, GL::type_of<GL::value>().is_base_of(GL::type_of<GL::decigallon>()));

            if (1) {
                using namespace GL::literals;

                GL::parallel::For(0, 1000000, [](size_t i) {});
                if (1){//auto timer = sw.debug_timer("No Sin()")) {
                    GL::parallel::For(0, 1000000, [](size_t i) {
                        (void)GL::degree(static_cast<float>(i));
                    });
                }
                if (1) {//auto timer = sw.debug_timer("Sin()")) {
                    GL::parallel::For(0, 1000000, [](size_t i) {
                        (void)GL::degree(static_cast<float>(i)).sin();
                    });
                }
                if (1) {//auto timer = sw.debug_timer("SinFast()")) {
                    GL::parallel::For(0, 1000000, [](size_t i) {
                        (void)GL::degree(static_cast<float>(i)).sin_fast();
                    });
                }






                EXPECT_EQ(3_kg * 10_mps / 5_s, 6_N);

                auto t_rest = (
                    (-2.40_mps_sq + GL::value((2.40_mps_sq).pow(2) - 4.0 * 0.5 * (0.3_mps_sq / 1_s) * -12.0_mps).sqrt()) / (2.0 * 0.5 * (0.3_mps_sq / 1_s))
                    ).max(
                        (-2.40_mps_sq - GL::value((2.40_mps_sq).pow(2) - 4.0 * 0.5 * (0.3_mps_sq / 1_s) * -12.0_mps).sqrt()) / (2.0 * 0.5 * (0.3_mps_sq / 1_s))
                    );
                EXPECT_EQ(t_rest, 4_s);
                EXPECT_EQ(GL::constants::pi(), 180_deg);
                EXPECT_EQ((0_deg).sin(), 0.0f);
                //print((0_deg).sin_fast());
                EXPECT_EQ((0_deg).cos(), 1.0f);
                EXPECT_EQ((90_deg).sin(), 1.0f);
                //print((90_deg).sin_fast());
                EXPECT_EQ((90_deg).cos(), 0.0f);
                EXPECT_EQ((180_deg).cos(), -1.0f);
                EXPECT_EQ((180_deg).sin(), 0.0f);
                //print((180_deg).sin_fast());
                EXPECT_EQ((270_deg).sin(), -1.0f);
                //print((270_deg).sin_fast());
                EXPECT_EQ((270_deg).cos(), 0.0f);
                EXPECT_EQ((GL::constants::pi() - 37_deg).sin(), (37_deg).sin().abs()); // trig identity
                //print((GL::constants::pi() - 37_deg).sin());
                //print((GL::constants::pi() - 37_deg).sin_fast());
                //print((37_deg).sin_fast());

                //print((137_deg).sin());
                //print((137_deg).sin_fast());

                //print((237_deg).sin());
                //print((237_deg).sin_fast());

                //print((337_deg).sin());
                //print((337_deg).sin_fast());

                //print((-337_deg).sin());
                //print((-337_deg).sin_fast());

                EXPECT_EQ(GL::celsius(0.0f), 0_degC);
                EXPECT_EQ(0_degC, 32_degF);
                EXPECT_EQ(32_degF, 0_degC);
                EXPECT_EQ(41_degF, 5_degC);
                EXPECT_EQ(5_degC, 41_degF);
                EXPECT_EQ(GL::foot(56_ft).wrap(0_mm, GL::meter(5_ft)), 1_ft);

                auto T_0 = GL::fahrenheit(10);
                auto T_melting = GL::celsius(0);
                auto mass_ice = 4_kg;
                auto specific_heat_ice = (2100_J / 1_kg) / 1_degC;
                auto specific_heat_water = (4186_J / 1_kg) / 1_degC;
                auto latent_heat_of_fusion_of_water = 333000.0_J / 1_kg;

                GL::joule heat_to_raise_temp_of_ice = mass_ice * specific_heat_ice * (T_melting - T_0);
                //print(heat_to_raise_temp_of_ice);

                GL::joule heat_to_melt_ice = mass_ice * latent_heat_of_fusion_of_water;
                //print(heat_to_melt_ice);

                GL::joule total_heat = heat_to_raise_temp_of_ice + heat_to_melt_ice;
                //print(total_heat);

                auto time = total_heat / 500_W;
                //print(time);

                auto heat_added = 500_W * 1200_s;
                GL::celsius final_water_temp = 0_degC + (heat_added / (mass_ice * specific_heat_water));
                //print(final_water_temp);
                //print(GL::fahrenheit(final_water_temp));



                //print(GL::value(-1.0f).asin());
                //print(GL::value(1.0f).asin());
                //print(GL::value(-2.0f).asin());
                //print(GL::value(2.0f).asin());
                //print(GL::value(-0.999f).asin());
                //print(GL::value(0.999f).asin());

                //print(GL::value(-1.0f).acos());
                //print(GL::value(1.0f).acos());
                //print(GL::value(-2.0f).acos());
                //print(GL::value(2.0f).acos());
                //print(GL::value(-0.999f).acos());
                //print(GL::value(0.999f).acos());





            }

            // can automatically cast from a foot to float...
            EXPECT_EQ(100, (int)GL::type_of<float>().instance_by_value(GL::foot(100.0f)).cast<float>());
            // can automatically cast from a foot to double...
            EXPECT_EQ(100, (int)GL::type_of<double>().instance_by_value(GL::foot(100.0f)).cast<double>());
            // can automatically cast from a foot to a meter...
            EXPECT_EQ(100, (int)(float)GL::type_of<GL::meter>().instance_by_value(GL::foot(GL::meter(100.0f))).cast<GL::meter>());


            try {
                GL::parallel::task([&]() {
                    return GL::foot(100) + GL::gallon(1); // will throw
                    })->wait(); // calling wait gives the opportunity to catch the exception.
                    EXPECT_EQ(true, false);
            }
            catch (std::exception&) {} // exception from the async job will ultimately be caught here

            GL::parallel::task([&]() {
                return GL::foot(100) + GL::gallon(1); // will throw
                }); // the task may dispatch on construction, but will wait till complete on destruction. Rethrowing exceptions during destruction is a recipe for death of a program, and so the exception is free'd and nothing is done with it. Basically silent failure.

            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 1;
                    return L.load();
                    })->and_then([&](GL::job_base& parent) {
                        auto V = parent.result.cast<long long>();
                        EXPECT_EQ(1, L.load());
                        EXPECT_EQ(1, V);
                        });
                    EXPECT_EQ(1, L.load());
            }
            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 1;
                    return L.load();
                    })->and_then([&](long long V) {
                        EXPECT_EQ(1, L.load());
                        EXPECT_EQ(1, V);
                        });
                    EXPECT_EQ(1, L.load());
            }
            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 1;
                    return L.load();
                    })->and_then([&](GL::job_base& parent, long long V) {
                        EXPECT_EQ(1, parent.result.cast<long long>());
                        EXPECT_EQ(1, L.load());
                        EXPECT_EQ(1, V);
                        });
                    EXPECT_EQ(1, L.load());
            }
            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 100;
                    })->and_then(0, 1000000, [&](size_t i) {
                        L += 1;
                        })->and_then([&]() {
                            L -= 100;
                            });
                        EXPECT_EQ(L.load(), 1000000);
            }

            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 100;
                })->and_then(0, 1000000, [&](size_t i, GL::job_base& parent) {
                    L += 1;
                })->and_then([&]() {
                    L -= 100;
                });
                EXPECT_EQ(L.load(), 1000000);
            }

            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 100;
                    return 1ull;
                })->and_then(0, 1000000, [&](size_t i, GL::job_base& parent, unsigned long long V) {
                    L += V;
                })->and_then([&]() {
                    L -= 100;
                });
                EXPECT_EQ(L.load(), 1000000);
            }

            if (1) {
                std::atomic<long long> L{ 0 };
                std::shared_ptr<GL::job_base> job; {
                    auto job1 = GL::parallel::task([&]() {
                        L += 100;
                        return 1ull;
                    }); // this job is dispatched
                    auto job2 = job1->and_then(0, 1000000, [&](size_t i, unsigned long long V, GL::job_base& parent) {
                        L += V;
                    });
                    job = std::dynamic_pointer_cast<GL::job_base>(job2->and_then([&]() {
                        L -= 100;
                    }));
                }
                job->wait();
                EXPECT_EQ(L.load(), 1000000);
            }

            if (1) {
                std::atomic<long long> L{ 0 };
                GL::parallel::task([&]() {
                    L += 100;
                    return 1ull;
                })->and_then(0, 1000000, [&](size_t i, unsigned long long V, GL::job_base& parent) {
                    L += V;
                })->and_then([&]() {
                    L -= 100;
                });
                EXPECT_EQ(L.load(), 1000000);
            }

            if (1) {
                auto job = GL::parallel::task([]() {
                    ::Sleep(10);
                    return 10;
                });
                job->and_then([]() {
                })->and_then([]() {
                    ::Sleep(10);
                })->and_then([]() {
                })->and_then([]() {
                });
                job->wait();
                EXPECT_EQ(10, job->result.cast<int>());
            }
            if (1) {
                auto job1 = GL::parallel::task([]() {
                    ::Sleep(10);
                    return 10;
                });
                auto job2 = job1->and_then([]() {
                })->and_then([]() {
                    ::Sleep(10);
                })->and_then([]() {
                })->and_then([]() {
                });
                job1->wait();
                EXPECT_EQ(10, job1->result.cast<int>());
            }
            if (1) { // if (auto timer = sw.debug_timer("Inline Test")) {
                size_t out = 0;
                std::vector<size_t> jobs;
                {
                    jobs.resize(10000000, 0);
                    for (size_t i = 0; i < 10000000; ++i) {
                        EXPECT_EQ(10000000, jobs.size());
                        ++jobs[i];
                    }
                    out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
                }
                EXPECT_EQ(10000000, out);
            }
            if (1) { // if (auto timer = sw.debug_timer("Parallel Jobs Test 1")) {
                size_t out = 0; {
                    GL::parallel::task([&]() {
                        std::vector<size_t> jobs;
                        jobs.resize(10000000, 0);
                        return jobs;
                    })->and_then(0, 10000000, [](size_t i, GL::job_base& parent) {
                        auto& jobs = parent.result.cast<std::vector<size_t>>();
                        EXPECT_EQ(10000000, jobs.size());
                        ++jobs[i];
                    })->and_then([&out](GL::job_base& parent) {
                        if (auto* p = parent.parent_ptr()) {
                            auto& jobs = p->result.cast<std::vector<size_t>>();
                            EXPECT_EQ(10000000, jobs.size());
                            out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
                            EXPECT_EQ(10000000, out);
                        }
                    });
                }
                EXPECT_EQ(10000000, out);
            }
            if (1) { // if (auto timer = sw.debug_timer("Parallel Jobs Test 2")) {
                size_t out = 0;
                {
                    std::vector<size_t> jobs;
                    jobs.resize(10000000, 0);
                    GL::parallel::task(0, 10000000, [&](size_t i) {
                        EXPECT_EQ(10000000, jobs.size());
                        ++jobs[i];
                    });
                    out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
                }
                EXPECT_EQ(10000000, out);
            }
            if (1) { // if (auto timer = sw.debug_timer("Parallel Jobs Test 3")) {
                size_t out = 0;
                {
                    std::vector<size_t> jobs;
                    jobs.resize(10000000, 0);
                    GL::parallel::For(0, 10000000, [&](size_t i) {
                        EXPECT_EQ(10000000, jobs.size());
                        ++jobs[i];
                    });
                    out = std::accumulate(jobs.begin(), jobs.end(), 0ull);
                }
                EXPECT_EQ(10000000, out);
            }

#endif
#endif

#if 1
            for (size_t repeats = 10; repeats <= 10000; repeats *= 10) {
                //print(repeats);

                GL::atomic_double result = GL::parallel::Dispatch(repeats, GL::atomic_double{ 0 }, [](size_t pos, GL::atomic_double& D) {
                    ++D;
                });
                EXPECT_EQ((size_t)result.load(), repeats);

                if (1) {
                    std::vector<std::string> calcs(repeats, "");
                    if (1) { // if (auto timer = sw.debug_timer("parallel::std single-threaded calculations")) {
                        GL::parallel::Std_For(0ull, repeats, [&](size_t const& index) {
                            calcs[index] = std::to_string(index);
                        });
                    };
                    if (1) { // if (auto timer = sw.debug_timer("parallel::manual single-threaded calculations")) {
                        GL::parallel::For(0ull, repeats, [&](size_t const& index) {
                            calcs[index] = std::to_string(index);
                        });
                    };
                    if (1) { // if (auto timer = sw.debug_timer("single-threaded single-threaded calculations")) {
                        size_t index = 0ull;
                        for (; index < repeats; ) {
                            calcs[index] = std::to_string(index);
                            ++index;
                        };
                    };
                }

                if (1) { // if (auto timer = sw.debug_timer("parallel::manual alloc")) {
                    GL::atomic_shared_ptr<size_t> ptr;
                    GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                        ptr.store(GL::make_shared<size_t>(i));
                        ptr = nullptr;
                    });
                }
                if (1) { // if (auto timer = sw.debug_timer("parallel::std increment")) {
                    std::atomic<size_t> D{ 0 };
                    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
                        ++D;
                    });
                }
                if (1) { // if (auto timer = sw.debug_timer("parallel::manual increment")) {
                    std::atomic<size_t> D{ 0 };
                    GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                        ++D;
                    });
                }
                if (1) { // if (auto timer = sw.debug_timer("parallel::std map")) {
                    concurrency::concurrent_unordered_map<size_t, size_t> map;
                    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
                        map[i] = i;
                    });
                }
                if (1) { // if (auto timer = sw.debug_timer("parallel::manual map")) {
                    concurrency::concurrent_unordered_map<size_t, size_t> map;
                    GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                        map[i] = i;
                    });
                }

                if (1) { // if (auto timer = sw.debug_timer("parallel::std ForEach")) {
                    std::vector<size_t*> vec(1000000, nullptr);
                    GL::parallel::Std_ForEach(vec, [](size_t*& p) {
                        p = reinterpret_cast<size_t*>(100);
                    });
                }
                if (1) {
                    std::vector<size_t*> vec(repeats, nullptr);
                    if (1) { // if (auto timer = sw.debug_timer("parallel::manual ForEach")) {
                        GL::parallel::For_Each(vec, [](size_t*& p) {
                            p = reinterpret_cast<size_t*>(100);
                        });
                    }
                }

            }
#endif

#if 1
                            if (1) { // if (auto timer = sw.debug_timer(GL::string("queue"))) {
                                GL::atomic_queue<size_t> queue;
                                size_t L;
                                queue.push(0);
                                queue.push(1);
                                queue.push(2);
                                EXPECT_EQ(true, queue.try_pop(L));
                                EXPECT_EQ(L, 0);
                                EXPECT_EQ(true, queue.try_pop(L));
                                EXPECT_EQ(L, 1);
                                EXPECT_EQ(true, queue.try_pop(L));
                                EXPECT_EQ(L, 2);
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_stack")) {
                                GL::atomic_stack<size_t> queue;
                                size_t L;
                                queue.push(0);
                                queue.push(1);
                                queue.push(2);
                                EXPECT_EQ(true, queue.try_pop(L));
                                EXPECT_EQ(L, 2);
                                EXPECT_EQ(true, queue.try_pop(L));
                                EXPECT_EQ(L, 1);
                                EXPECT_EQ(true, queue.try_pop(L));
                                EXPECT_EQ(L, 0);
                            }
                            if (1) { // if (auto timer = sw.debug_timer("thread_object")) {
                                if (1) {
                                    GL::thread_object<int> thread_local_object(100);
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        EXPECT_EQ(100, *thread_local_object);
                                        });
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        *thread_local_object = static_cast<int>(GL::util::get_thread_id()); // thread-local and therefore thread-safe to change its value
                                        });
                                }
                                if (1) {
                                    GL::thread_object<std::string> thread_local_object("TEST");
                                    GL::parallel::For(0, 1000000, [&thread_local_object](int) {
                                        EXPECT_EQ("TEST", *thread_local_object);
                                        });
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        *thread_local_object = "\"" + std::to_string(GL::util::get_thread_id()) + "\""; // thread-local and therefore thread-safe to change its value
                                        });
                                }
                                if (1) {
                                    GL::thread_object<GL::string> thread_local_object("TEST");
                                    GL::parallel::For(0, 1000000, [&thread_local_object](int) {
                                        EXPECT_EQ("TEST", *thread_local_object);
                                        });
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        *thread_local_object = "\"" + std::to_string(GL::util::get_thread_id()) + "\""; // thread-local and therefore thread-safe to change its value
                                        });
                                }
                            };
                            if (1) { // if (auto timer = sw.debug_timer("atomic_allocator")) {
                                GL::atomic_allocator<std::string, 1024> alloc;
                                if (1) {
                                    GL::parallel::For(0, 1000000, [&](int) {
                                        alloc.Free(alloc.Alloc());
                                        });
                                }
                                if (1) {
                                    std::vector< std::string* > ptrs(1000000, nullptr);
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        ptrs[i] = alloc.Alloc();
                                        });
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        alloc.Free(ptrs[i]);
                                        });
                                }
                                if (1) {
                                    GL::atomic_parallel_stack<std::string*> to_delete;
                                    GL::parallel::For(0, 1000000, [&](int) {
                                        to_delete.push(alloc.Alloc());
                                        std::string* p{ nullptr };
                                        if (to_delete.try_pop(p)) {
                                            alloc.Free(p);
                                        }
                                        });
                                }
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_allocator ST")) {
                                GL::atomic_parallel_allocator<std::string, 1024> alloc;
                                if (1) {
                                    for (int i = 0; i < 1000000; ++i) {
                                        alloc.Free(alloc.Alloc());
                                    };
                                }
                                if (1) {
                                    std::vector< std::string* > ptrs(1000000, nullptr);
                                    for (int i = 0; i < 1000000; ++i) {
                                        ptrs[i] = alloc.Alloc();
                                    };
                                    for (int i = 0; i < 1000000; ++i) {
                                        alloc.Free(ptrs[i]);
                                    };
                                }
                                if (1) {
                                    GL::atomic_parallel_stack<std::string*> to_delete;
                                    for (int i = 0; i < 1000000; ++i) {
                                        to_delete.push(alloc.Alloc());
                                        std::string* p{ nullptr };
                                        if (to_delete.try_pop(p)) {
                                            alloc.Free(p);
                                        }
                                    };
                                }
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_allocator MT")) {
                                GL::atomic_parallel_allocator<std::string, 1024> alloc;
                                if (1) {
                                    GL::parallel::For(0, 1000000, [&](int) {
                                        alloc.Free(alloc.Alloc());
                                        });
                                }
                                if (1) {
                                    std::vector< std::string* > ptrs(1000000, nullptr);
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        ptrs[i] = alloc.Alloc();
                                        });
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        alloc.Free(ptrs[i]);
                                        });
                                }
                                if (1) {
                                    GL::atomic_parallel_stack<std::string*> to_delete;
                                    GL::parallel::For(0, 1000000, [&](int) {
                                        to_delete.push(alloc.Alloc());
                                        std::string* p{ nullptr };
                                        if (to_delete.try_pop(p)) {
                                            alloc.Free(p);
                                        }
                                        });
                                }
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_epoch_allocator")) {
                                GL::atomic_epoch_allocator<std::string> alloc;
                                if (1) {
                                    GL::parallel::For(0, 1000000, [&](int) {
                                        auto Protected = alloc.ProtectCurrentEpoch();
                                        alloc.Free(alloc.Alloc());
                                        });
                                }
                                if (1) {
                                    std::vector< std::string* > ptrs(1000000, nullptr);
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        ptrs[i] = alloc.Alloc();
                                        });
                                    GL::parallel::For(0, 1000000, [&](int i) {
                                        auto Protected = alloc.ProtectCurrentEpoch();
                                        alloc.Free(ptrs[i]);
                                        });
                                }
                                if (1) {
                                    GL::atomic_parallel_stack<std::string*> to_delete;
                                    GL::parallel::For(0, 1000000, [&](int) {
                                        auto Protected = alloc.ProtectCurrentEpoch();
                                        to_delete.push(alloc.Alloc());
                                        std::string* p{ nullptr };
                                        if (to_delete.try_pop(p)) {
                                            alloc.Free(p);

                                            p->push_back('t');
                                            p->push_back('e');
                                            p->push_back('s');
                                            p->push_back('t');
                                            (void)p->c_str();
                                        }
                                        });
                                }
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_map 1")) {
                                GL::atomic_map<size_t, GL::atomic_double> map;

                                GL::parallel::For(0, 1000000, [&](int i) {
                                    if (i % 2 == 0) {
                                        map[(size_t)GL::util::rand(0, 10)] = i;
                                    }
                                    else {
                                        (void)map.erase((size_t)GL::util::rand(0, 10));
                                    }
                                    });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_map 2")) {
                                GL::atomic_map<size_t, GL::atomic_double> map;
                                GL::parallel::For(0, 1000000, [&](int i) {
                                    (void)map.erase(i % 10);
                                    map[i % 10] = i;
                                    });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_stack<size_t>")) {
                                GL::atomic_stack<size_t> queue;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                    queue.try_pop(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.try_pop(i);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_stack<size_t>")) {
                                GL::atomic_parallel_stack<size_t> queue;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                    queue.try_pop(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.try_pop(i);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_queue<size_t>")) {
                                GL::atomic_queue<size_t> queue;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                    queue.try_pop(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.try_pop(i);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_queue<size_t>")) {
                                GL::atomic_parallel_queue<size_t> queue;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                    queue.try_pop(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.try_pop(i);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_queue<short>")) {
                                GL::atomic_parallel_queue<short> queue;

                                GL::parallel::For(0, 1000000, [&](short i) {
                                    queue.push(i);
                                    queue.try_pop(i);
                                });
                                GL::parallel::For(0, 1000000, [&](short i) {
                                    queue.push(i);
                                });
                                GL::parallel::For(0, 1000000, [&](short i) {
                                    queue.try_pop(i);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_stack<GL::string>")) {
                                GL::atomic_parallel_stack<GL::string> queue;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    GL::string str = std::to_string(i);
                                    queue.push(str);
                                    queue.try_pop(str);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    GL::string str = std::to_string(i);
                                    queue.push(str);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    GL::string str;
                                    queue.try_pop(str);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_queue<GL::string>")) {
                                GL::atomic_parallel_queue<GL::string> queue;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    GL::string str = std::to_string(i);
                                    queue.push(str);
                                    queue.try_pop(str); // not guarranteed -- may miss
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    GL::string str = std::to_string(i);
                                    queue.push(str); // not guarranteed -- may miss
                                    });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    GL::string str;
                                    queue.try_pop(str); // not guarranteed -- may miss
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_priority_queue<std::string>")) {
                                GL::atomic_priority_queue < std::string > queue;
                                std::string str;

                                queue.push("1");
                                queue.push("2");
                                queue.push("3");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "1");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "2");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "3");

                                queue.push("banana");
                                queue.push("cucumber");
                                queue.push("apple");
                                queue.push("Banana");
                                queue.push("Cucumber");
                                queue.push("Apple");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "Apple");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "Banana");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "Cucumber");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "apple");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "banana");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "cucumber");

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    std::string str = std::to_string(i);
                                    queue.push(str);
                                    queue.try_pop(str);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    std::string str = std::to_string(i);
                                    queue.push(str);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    std::string str;
                                    queue.try_pop(str);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("atomic_parallel_priority_queue<std::string>")) {
                                GL::atomic_parallel_priority_queue < std::string > queue;
                                std::string str;

                                queue.push("1");
                                queue.push("2");
                                queue.push("3");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "1");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "2");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "3");

                                queue.push("banana");
                                queue.push("cucumber");
                                queue.push("apple");
                                queue.push("Banana");
                                queue.push("Cucumber");
                                queue.push("Apple");

                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "Apple");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "Banana");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "Cucumber");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "apple");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "banana");
                                EXPECT_EQ(true, queue.try_pop(str));
                                EXPECT_EQ(str, "cucumber");

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    std::string str = std::to_string(i);
                                    queue.push(str);
                                    queue.try_pop(str);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    std::string str = std::to_string(i);
                                    queue.push(str);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    std::string str;
                                    queue.try_pop(str);
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_vector<size_t>")) {
                                GL::atomic_vector<size_t> queue;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    queue.push_back(i);
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    ++queue[i];
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_map<size_t, size_t>")) {
                                GL::atomic_map<size_t, size_t> map;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    map[i] += i;
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_hash_map<size_t, size_t>")) {
                                GL::atomic_hash_map<size_t, size_t> map;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    map[i] += i;
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("concurrency::concurrent_unordered_map<size_t, size_t>")) {
                                concurrency::concurrent_unordered_map<size_t, size_t> map;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    map[i] += i;
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_map<size_t, size_t> w/ erasure")) {
                                GL::atomic_map<size_t, size_t> map;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    map[i] += i;
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    auto& loc = map[i];
                                    map.erase(i);
                                    ++loc;
                                });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_hash_map<size_t, size_t> w/ erasure")) {
                                GL::atomic_hash_map<size_t, size_t> map;

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    map[i] += i;
                                });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    auto& loc = map[i];
                                    map.erase(i);
                                    ++loc;
                                });
                            }
#endif
#if 0
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_double")) {
                                GL::atomic_double d;
                                EXPECT_EQ(0, d);
                                EXPECT_EQ(sizeof(GL::atomic_double), sizeof(double));

                                d = 0;
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    ++d;
                                });
                                EXPECT_EQ(1000000, d);

                                d = 0;
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    d += 2;
                                });
                                EXPECT_EQ(1000000, d / 2);
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_float")) {
                                GL::atomic_float d;
                                EXPECT_EQ(0, d);
                                EXPECT_EQ(sizeof(GL::atomic_float), sizeof(float));

                                d = 0;
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    ++d;
                                    });
                                EXPECT_EQ(1000000, d);

                                d = 0;
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    d += 2;
                                    });
                                EXPECT_EQ(1000000, d / 2);
                            }
#endif
#if 1
#if 0
                            if (1) { // if (auto timer = sw.debug_timer("GL::impl::atomic_ptr")) {
                                GL::impl::atomic_ptr<long long> ptr{ reinterpret_cast<long long*>(100), 0 };

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    EXPECT_EQ(reinterpret_cast<long long*>(100), ptr.load().first);
                                    EXPECT_EQ(0, ptr.load().second);
                                    });

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    auto prev_val = ptr.exchange(reinterpret_cast<long long*>(i), 0);
                                    });

                                std::atomic<char> flag{ 0 };
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    // will only succeed in the exchange if the 'flag' matches our flag value. 
                                    auto prev_val = ptr.exchange_if(reinterpret_cast<long long*>(i), flag++);
                                    if (std::get<0>(prev_val)) {
                                        // print(GL::printf("%i: %i\n", reinterpret_cast<int>(prev_val.first), static_cast<int>(prev_val.second)));
                                    }
                                    });
                            }
                            if (1) { // if (auto timer = sw.debug_timer("GL::impl::atomic_ptr")) {

                                GL::impl::atomic_ptr<long long> ptr{ reinterpret_cast<long long*>(100), 1 };

                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    if (i == 10101) {
                                        // kills the pointer, sets flag to zero, returns previous value
                                        ptr.exchange(reinterpret_cast<long long*>(i), 0); // sets the pointer to 0
                                    }
                                    else {
                                        // keeps the pointer at 1 (if it was still at 1), otherwise returns empty. 
                                        auto prev_val = ptr.exchange_if(reinterpret_cast<long long*>(i), 1);
                                        if (std::get<2>(prev_val)) {
                                            print(GL::printf("%i: %i\n", reinterpret_cast<int>(std::get<0>(prev_val)), static_cast<int>(std::get<1>(prev_val))));
                                        }
                                    }
                                    });

                            }
#endif

            if (1) { // if (auto timer = sw.debug_timer("increment as individuals")) {
                GL::thread_object<size_t> counter{ 0 };
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    ++* counter;
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("increment as atomic")) {
                std::atomic<size_t> counter{ 0 };
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    ++counter;
                    });
            }

            // under low contention, the GL::atomic_shared_ptr using fast_shared_ptr is ~40% faster than a locked shared_ptr, even keeping pace with accessing a shared pointer without copying it. 
            // under moderate contention, this is still true, up to about 50 reads per value change
            // under extremely heavy contention (around 10 reads for every value change), the GL::atomic_shared_ptr is significantly bloated and results in significant slow-downs.
            for (double ratio = 1000000.0; ratio >= 1.0; ratio /= 10) {
                //print(ratio);

                if (1) { // if (auto timer = sw.debug_timer("std::shared_ptr<std::string> with std shared lock")) {
                    std::shared_mutex mut;
                    std::shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };
                    GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                        std::shared_ptr<std::string> ptr2;
                        if (i % (int)ratio == 0) {
                            mut.lock();
                            ptr = std::shared_ptr<std::string>(new std::string(std::to_string(i + 1)));
                            mut.unlock();
                        }

                        mut.lock_shared();
                        ptr2 = ptr;
                        mut.unlock_shared();

                        if (ptr2) {
                            EXPECT_EQ((ptr2->length() > 0), true);
                        }
                        });
                }
                if (1) { // if (auto timer = sw.debug_timer("std::shared_ptr<std::string> access with std shared lock")) {
                    std::shared_mutex mut;
                    std::shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };
                    GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                        if (i % (int)ratio == 0) {
                            mut.lock();
                            ptr = std::shared_ptr<std::string>(new std::string(std::to_string(i + 1)));
                            mut.unlock();
                        }

                        mut.lock_shared();
                        EXPECT_EQ((ptr->length() > 0), true);
                        mut.unlock_shared();
                        });
                }
                if (1) { // if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<std::string> slow test")) {
                    GL::atomic_shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };
                    GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                        if (i % (int)ratio == 0) {
                            ptr.store(GL::shared_ptr<std::string>(new std::string(std::to_string(i + 1))));
                        }
                        if (auto ptr2 = ptr.load()) {
                            EXPECT_EQ((ptr2->length() > 0), true);
                        }
                    });
                }
                if (1) { // if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<std::string> fast test")) {
                    GL::atomic_shared_ptr<std::string> ptr{ new std::string(std::to_string(1)) };
                    GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                        if (i % (int)ratio == 0) {
                            ptr.store(GL::shared_ptr<std::string>(new std::string(std::to_string(i + 1))));
                        }
                        if (auto ptr2 = ptr.load_fast()) {
                            EXPECT_EQ((ptr2->length() > 0), true);
                        }
                    });
                }
            }

            if (1) { // if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<void>")) {
                auto ptr = GL::static_pointer_cast<void>(GL::atomic_shared_ptr<std::string>(new std::string("test")));
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    if (i % 50 == 0) {
                        ptr.store(GL::shared_ptr<std::string>(new std::string(std::to_string(i + 1))));
                    }
                    if (auto ptr2 = GL::static_pointer_cast<std::string>(ptr.load())) {
                        EXPECT_EQ((ptr2->length() > 0), true);
                    }
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("std::shared_ptr<void>")) {
                std::shared_mutex mut;
                std::shared_ptr<void> ptr{ std::shared_ptr<std::string>(new std::string("test")) };
                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                    if (i % 50 == 0) {
                        mut.lock();
                        ptr = std::shared_ptr<std::string>(new std::string(std::to_string(i + 1)));
                        mut.unlock();
                    }
                    mut.lock_shared();
                    auto ptr2 = std::static_pointer_cast<std::string>(ptr);
                    mut.unlock_shared();
                    if (ptr2) {
                        EXPECT_EQ((ptr2->length() > 0), true);
                    }
                    });
            }

            auto void_type = GL::type_of<void>();
            auto int_type = GL::type_of<int>();
            EXPECT_EQ(int_type.name(), "int");
            auto double_type = GL::type_of<double>();
            EXPECT_EQ(double_type.name(), "double");
            auto float_type = GL::type_of<float>();
            EXPECT_EQ(float_type.name(), "float");
            auto str_type = GL::type_of<GL::string>();
            EXPECT_EQ(str_type.name(), "class GL::string");

#endif
#if 1
                            if (1) { // if (auto timer = sw.debug_timer("atomic_wait")) {
                                std::atomic<long> lock{ 0 };
                                std::thread temp_thread([&]() {
                                    ::Sleep(1100);
                                    lock.store(1);
                                    GL::atomic_notify_one(&lock);
                                    });
                                GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                    GL::stopwatch sw2;
                                    sw2.reset();
                                    GL::atomic_wait(&lock, 0l);
                                    EXPECT_EQ(true, (sw.stop() > 1));
                                    });
                                temp_thread.join();


                                //std::atomic<size_t> prog{ 0 };
                                //GL::_Locked_pointer<long> ptr{ reinterpret_cast<long*>(1ull) };
                                //GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                                //    size_t p = reinterpret_cast<size_t>(ptr._Lock_and_load());
                                //    EXPECT_EQ(++prog, p++);
                                //    ptr._Store_and_unlock(reinterpret_cast<long*>(p));
                                //});
                                //EXPECT_EQ(1000000, reinterpret_cast<size_t>(ptr._Unsafe_load_relaxed()));
                            }








#endif

#endif

            //auto this_frame = (float)(1.0 / sw.stop());
            //SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
            //print(std::to_string(this_frame) + " this fps. \t" + GL::arena_memory_pool::debug() + "        \t");
#endif
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
