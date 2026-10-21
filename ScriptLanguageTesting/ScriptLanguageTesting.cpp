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
#include "ParallelTasks.h"
#include "units.h"
#include "Stopwatch.h"
//#include "stopwatch.h"
//#include "strings.h"
//#include "types.h"
#include "scripting.h"
#include "datetime.h"
#include "array_allocator.h"


namespace GL {
    class stopwatch_group {
    protected:
        GL::thread_object_no_default<std::map<long long, long long>>
            time_results;
        GL::string
            name;
        long long
            start;
    public:
        stopwatch_group(GL::string Name = GL::string::empty_string())
            : name(Name)
            , start(clock::ns())
        {};
        ~stopwatch_group() {
            long long end = clock::ns();
            std::map<long long, long double> quantile;
            long double total_count = 0;
            time_results.for_each([&](auto const& times) {
                for (auto& pair : times) {
                    quantile[pair.first] += pair.second;
                    total_count += pair.second;
                }
                });
            if (total_count > 0) {
                long long min = -1;
                long long fst = -1;
                long long mid = -1;
                long long trd = -1;
                long long max = -1;
                long double totalizer = 0.0;
                long long Total = 0;

                for (auto& pair : quantile) {
                    pair.second /= total_count;
                    Total += pair.first;
                    totalizer += pair.second;
                    pair.second = totalizer;

                    if (min == -1) {
                        min = pair.first;
                    }
                    if (fst == -1
                        && pair.second >= 0.25) {
                        fst = pair.first;
                    }
                    if (mid == -1
                        && pair.second >= 0.50) {
                        mid = pair.first;
                    }
                    if (trd == -1
                        && pair.second >= 0.75) {
                        trd = pair.first;
                    }
                    max = pair.first;
                }

                auto out = std::vector{
                    GL::millisecond(GL::nanosecond((float)min)).to_string(),
                    GL::millisecond(GL::nanosecond((float)fst)).to_string(),
                    GL::millisecond(GL::nanosecond((float)mid)).to_string(),
                    GL::millisecond(GL::nanosecond((float)trd)).to_string(),
                    GL::millisecond(GL::nanosecond((float)max)).to_string(),
                    GL::millisecond(GL::nanosecond((float)end - start)).to_string()
                };
                if (name.length() > 0) {
                    std::cout << GL::printf("%.*s: [ Min: %.*s, 25%: %.*s, 50%: %.*s, 75%: %.*s, Max: %.*s ] Over %.*s\n",
                        name.length(), name.c_str().data(),
                        out[0].length(), out[0].c_str().data(),
                        out[1].length(), out[1].c_str().data(),
                        out[2].length(), out[2].c_str().data(),
                        out[3].length(), out[3].c_str().data(),
                        out[4].length(), out[4].c_str().data(),
                        out[5].length(), out[5].c_str().data()
                    );
                }
                else {
                    std::cout << GL::printf("[ Min: %.*s, 25%: %.*s, 50%: %.*s, 75%: %.*s, Max: %.*s ] Over %.*s\n",
                        out[0].length(), out[0].c_str().data(),
                        out[1].length(), out[1].c_str().data(),
                        out[2].length(), out[2].c_str().data(),
                        out[3].length(), out[3].c_str().data(),
                        out[4].length(), out[4].c_str().data(),
                        out[5].length(), out[5].c_str().data()
                    );
                }
            }
        };
        class wrapper {
        private:
            stopwatch_group* P;
            long long startTime;

        public:
            wrapper(stopwatch_group* p) : P{ p }, startTime{ clock::ns() } {};
            wrapper(wrapper const&) = delete;
            wrapper(wrapper&&) = delete;
            wrapper& operator=(wrapper const&) = delete;
            wrapper& operator=(wrapper&&) = delete;
            ~wrapper() {
                startTime -= clock::ns();
                startTime *= -1ll;
                P->time_results->operator[](startTime)++;
            };
            constexpr operator bool() const { return true; };
        };

    public:
        auto debug_timer() {
            return wrapper(this);
        };

    };

    template<typename T, bool use_epoch_protections = true>
    class versioned_wrapper {
    public:
        using allocator = std::conditional_t<use_epoch_protections,
            GL::fast_atomic_epoch_allocator< std::pair<size_t, T> >,
            GL::fast_atomic_allocator< std::pair<size_t, T> >
        >;

    public:
        typename allocator
            alloc;
        GL::atomic_shared_ptr<std::pair<size_t, T>>
            val;
        auto guard_critical_section_impl() {
            if constexpr (use_epoch_protections) {
                return alloc.guard_critical_section();
            }
            else {
                return 0ull;
            }
        }
        typedef decltype(GL::detail::function_signature(&guard_critical_section_impl)) function_header;
        typedef typename function_header::Return_Type guard_critical_section_type;

    public:
        versioned_wrapper(T&& init = {}) : alloc(), val() {
            val.store(alloc.AllocShared(0ull, std::forward<T>(init)));
        };
        versioned_wrapper(versioned_wrapper const&) = delete;
        versioned_wrapper(versioned_wrapper&&) = delete;
        versioned_wrapper& operator=(versioned_wrapper const&) = delete;
        versioned_wrapper& operator=(versioned_wrapper&&) = delete;
        ~versioned_wrapper() = default;
        // inserts the newest input value only if the cache_version is greater than the previous value. 
        bool store(size_t cache_version, T const& input) {
            if (auto protected_copy = val.load_fast()) {
                if (protected_copy->first < cache_version) {
                    auto guard{ guard_critical_section_impl() };
                    auto new_v = alloc.AllocShared(cache_version, input);
                    while (true) {
                        if (protected_copy = val.load_fast()) {
                            if (protected_copy->first < cache_version) {
                                if (val.compare_exchange(protected_copy.get(), std::move(new_v))) {
                                    return true;
                                }
                            }
                            else {
                                // do nothing
                                return false;
                            }
                        }
                    }
                }
            }
            return false;
        };
        // inserts the newest input value only if the cache_version is greater than the previous value. Will call the provided function to spawn the input value in that case.
        template <typename F> bool store_f(size_t cache_version, F const& functional_input) {
            if (auto protected_copy = val.load_fast()) {
                if (protected_copy->first < cache_version) {
                    auto guard{ guard_critical_section_impl() };
                    auto new_v = alloc.AllocShared(cache_version, functional_input());
                    while (true) {
                        if (protected_copy = val.load_fast()) {
                            if (protected_copy->first < cache_version) {
                                if (val.compare_exchange(protected_copy.get(), std::move(new_v))) {
                                    return true;
                                }
                            }
                            else {
                                // do nothing
                                return false;
                            }
                        }
                    }
                }
            }
            return false;
        };
        GL::shared_ptr<std::pair<size_t, T>> load() {
            return val.load();
        };
        GL::fast_shared_ptr<std::pair<size_t, T>> load_fast() {
            return val.load_fast();
        };
        template <typename U = T> guard_critical_section_type guard_critical_section() {
            if constexpr (use_epoch_protections) {
                return alloc.guard_critical_section();
            }
            else {
                static_assert(false, "Epoch functions are unavailable with the provided template parameters");
                return 0ull;
            }
        };
        template <typename U = T> T& ref() {
            if constexpr (use_epoch_protections) {
                return val.load_fast()->second;
            }
            else {
                static_assert(false, "Ref functions are unavailable with the provided template parameters");
                return *((T*)nullptr);
            }
        }
    };


};

#include "../GpuProgramming/matrix.h"

int main() {
    // pre-warm the thread
    if (1) {
        GL::GPU::matrix<float> LHS = GL::GPU::matrix<float>::constant(5, 100, 100);
        GL::GPU::matrix<float> RHS = GL::GPU::matrix<float>::constant(6, 100, 100);
        auto task = GL::parallel::single_threaded::async([&LHS, &RHS]() -> GL::GPU::matrix<float> {
            return LHS * RHS;
        });
        task.get_ref();
    }
    while (true) {
        if (auto sw = GL::stopwatch::debug_timer("SINGLE_THREADED gpu_job 0.25"); false) {
            size_t len_bytes = 100;
            auto job1 = GL::parallel::single_threaded::async([&len_bytes]() {
                return len_bytes;
            });
            auto job2 = GL::parallel::single_threaded::async([job1]() {
                return GL::parallel::single_threaded::async([job1]() {
                    return (float)job1.get_ref();
                }).get();
            });
            auto job3 = GL::parallel::single_threaded::async([job2]() {
                std::cout << job2.get_ref() << std::endl;
            });
        }
        if (auto sw = GL::stopwatch::debug_timer("SINGLE_THREADED gpu_job 0.5"); false) {
            size_t len_bytes = 100;
            GL::parallel::single_threaded::async([&len_bytes]() {
                return len_bytes;
            }).and_then([](size_t& rhs) {
                return GL::parallel::single_threaded::async([rhs]() {
                    return (float)rhs;
                }).get();
            }).and_then([](float& rhs) {
                std::cout << rhs << std::endl;
            });
        }
        if (auto sw = GL::stopwatch::debug_timer("SINGLE_THREADED gpu_job 3"); false) {
            auto job_a = GL::parallel::single_threaded::async([]() {
                return GL::GPU::matrix<float>::constant(5, 10, 10);
            });
            auto job_b = GL::parallel::single_threaded::async([]() {
                return GL::GPU::matrix<float>::constant(6, 10, 10);
            });
            GL::parallel::single_threaded::async([job_a, job_b]() {
                return job_a.get_ref() * job_b.get_ref();
            }).and_then([](GL::GPU::matrix<float>& job_mult) {
                if (auto read = job_mult.read()) {
                    std::cout << read[0] << std::endl;
                }
            });
        };

        //GL::value progress{ 0 };
        //auto task_1 = GL::parallel::async(0, 10'000, [&progress]() {
        //    auto curr_prog = ++progress;
        //    if (curr_prog % 1000 == 0) 
        //        std::cout << GL::printf("%i percent\n", (int)(100.0f * ((float)curr_prog / 10'000.0f)));            
        //}).and_then([&progress]() {
        //    std::cout << GL::printf("result: %f\n", (float)progress);
        //});

        //// casting is automatic for all of these
        //GL::parallel::single_threaded::impl::task([]() -> GL::millisecond {
        //    std::cout << "First...\n";
        //    return GL::millisecond(1'000);
        //})->and_then([](GL::second t0) -> GL::millisecond {
        //    using namespace GL::literals;
        //    if (t0 != 1'000_ms) std::cout << "SHOULD HAVE MATCHED 325\n";
        //    std::cout << "Second...\n";
        //    return t0;
        //})->and_then(0, 9'000, [](size_t i, GL::millisecond& t0, GL::parallel::single_threaded::job_base& parent) -> GL::any {
        //    t0 += 1;
        //    return parent.result;
        //})->and_then([](GL::any const& t0) {
        //    using namespace GL::literals;
        //    if (t0.cast<GL::millisecond>() != 10'000_ms) std::cout << "SHOULD HAVE MATCHED " << __LINE__ << std::endl;
        //    std::cout << "Third and done.\n";
        //});

        //// casting is automatic for all of these
        //GL::parallel::single_threaded::async([]() -> GL::millisecond {
        //    std::cout << "First...\n";
        //    return GL::millisecond(1'000);
        //}).and_then([](GL::second t0) -> GL::millisecond {
        //    using namespace GL::literals;
        //    if (t0 != 1'000_ms) std::cout << "SHOULD HAVE MATCHED " << __LINE__ << std::endl;
        //    std::cout << "Second...\n";
        //    return t0;
        //}).and_then(0, 9'000, [](size_t i, GL::millisecond& t0, GL::parallel::single_threaded::job_base& parent) -> GL::any {
        //    t0 += 1;
        //    return parent.result;
        //}).and_then([](GL::any const& t0) {
        //    using namespace GL::literals;
        //    if (t0.cast<GL::millisecond>() != 10'000_ms) std::cout << "SHOULD HAVE MATCHED " << __LINE__ << std::endl;
        //    std::cout << "Third and done.\n";
        //});


        //// casting is automatic for all of these
        //GL::parallel::impl::task([]() -> GL::millisecond {
        //    std::cout << "First...\n";
        //    return GL::millisecond(1'000);
        //})->and_then([](GL::second t0) -> GL::millisecond {
        //    using namespace GL::literals;
        //    if (t0 != 1'000_ms) std::cout << "SHOULD HAVE MATCHED " << __LINE__ << std::endl;
        //    std::cout << "Second...\n";
        //    return t0;
        //})->and_then(0, 9'000, [](size_t i, GL::millisecond& t0, GL::parallel::job_base& parent) -> GL::any {
        //    t0 += 1;
        //    return parent.result;
        //})->and_then([](GL::any const& t0) {
        //    using namespace GL::literals;
        //    if (t0.cast<GL::millisecond>() != 10'000_ms) std::cout << "SHOULD HAVE MATCHED " << __LINE__ << std::endl;
        //    std::cout << "Third and done.\n";
        //});













        std::cout << GL::parallel::tasks::async([]() -> GL::millisecond {
            return GL::millisecond(1'000);
        })->get() << std::endl;
        std::cout << GL::parallel::tasks::async(0ull, 10'000ull, []() -> GL::millisecond {
            return GL::millisecond(1'000);
        })->get() << std::endl;

        while (1) {
            GL::stopwatch_group group("Iterations");
            for (int j = 0; j < 100; ++j) {
                auto timer = group.debug_timer();
#if 1
                //std::array<GL::parallel::tasks::iTask, 1000> tasks;
                //for (int i = 0; i < 1'000; ++i) {
                //    // handles the type conversions automatically before type erasure
                //    /*tasks[i] = */GL::parallel::tasks::do_task([](double i) {
                //        return i;
                //    }, i);
                //}
                /*auto task =*/GL::parallel::tasks::do_tasks(0, 1'000'000, [](GL::string const& str) {
                    //std::cout << str << std::endl;
                    return 100;
                }, std::string_view("TEST"));

                //for (int i = 0; i < 1'000; ++i) {
                //    tasks[i].wait();
                //}
                //task.wait();

                // ::Sleep(1000);



                // task.wait();


                //GL::parallel::tasks::async([]() -> int {
                //    std::cout << 1 << std::endl;
                //    return 100;
                //})->and_then(0, 1'000'000, [](size_t i, GL::any r) -> GL::any {
                //    //std::cout << *r << std::endl;
                //    return r;
                //})->and_then([](GL::any r) -> GL::any {
                //    std::cout << 3 << std::endl;
                //    return r;
                //})->and_then([](GL::any r) {
                //    std::cout << 4 << std::endl;
                //    std::cout << r.cast<int>() << std::endl;
                //});
#elif 1
                // GL::shared_ptr<int> -> GL::shared_ptr<int>
                auto task1 = GL::parallel::tasks::async([]() {
                    std::cout << 1 << std::endl;
                    return GL::make_shared<int>(100);
                });
                auto task2 = task1->and_then(0, 1'000'000, [](size_t i, GL::any const& r) {
                    //std::cout << *r << std::endl;
                });
                auto task3 = task2->and_then([Task = task1]() -> GL::any {
                    std::cout << 3 << std::endl;
                    return Task->get_any();
                });
                auto task4 = task3->and_then([Task = task1]() {
                    std::cout << 4 << std::endl;
                    std::cout << Task->get() << std::endl;
                });
#elif 0
                // GL::any -> GL::any
                auto task1 = GL::parallel::tasks::async([]() -> int {
                    std::cout << 1 << std::endl;
                    return 100;
                });
                auto task2 = task1->and_then(0, 1'000'000, [](size_t i, GL::any const& r) -> GL::any {
                    std::cout << r.cast<int>() << std::endl;
                    return r;
                });
                auto task3 = task2->and_then([](GL::shared_ptr<int> r) -> GL::shared_ptr<int> {
                    return r;
                });
                auto task4 = task3->and_then([](int& r) {
                    std::cout << r << std::endl;
                });
#elif 0
                // 
                auto task1 = GL::parallel::tasks::async([]() -> int {
                    std::cout << 1 << std::endl;
                    return 100;
                });
                auto task2 = task1->and_then(0, 1'000, [](size_t i, std::shared_ptr<int> r) -> std::shared_ptr<int> {
                    std::cout << *r << std::endl;
                    return r;
                });
                //using arg_conversion_t = typename GL::parallel::impl::function_traits<decltype(std::function(&decltype(task2)::element_type::cast_return))>::result_type;
                //sizeof(arg_conversion_t);
                auto task3 = task2->and_then([task1]() -> GL::any {
                    return task1->get_any();
                });
                //using arg_conversion_t = typename GL::parallel::impl::function_traits<decltype(std::function(&decltype(task3)::element_type::cast_return))>::result_type;
                //sizeof(arg_conversion_t);
                auto task4 = task3->and_then([](int& r) {
                    std::cout << r << std::endl;
                });
                //using arg_conversion_t = typename GL::parallel::impl::function_traits<decltype(std::function(&decltype(task4)::element_type::cast_return))>::result_type;
#else
                auto task1 = GL::parallel::tasks::async([]() -> int {
                    std::cout << 1 << std::endl;
                    return 100;
                });
                task1->and_then(0, 1'000'000, [](size_t i, int& r) {
                    std::cout << r << std::endl;
                })->and_then([task1]() {
                    return task1->get_any();
                })->and_then([](int& r) {
                    std::cout << r << std::endl;
                });
#endif
            }
        }


        // casting is automatic for all of these
        auto job123 = GL::parallel::async([]() -> GL::millisecond {
            std::cout << "First...\n";
            return GL::millisecond(1'000);
        });
        auto job234 = job123.and_then([](GL::second t0) -> GL::millisecond {
            using namespace GL::literals;
            if (t0 != 1'000_ms) std::cout << "SHOULD HAVE MATCHED " << __LINE__ << std::endl;
            std::cout << "Second...\n";
            return t0;
        });
        auto job345 = job234.and_then(0, 9'000, [](size_t i, GL::millisecond& t0, GL::parallel::job_base& parent) -> GL::any {
            t0 += 1;
            return parent.result;
        });
        auto job456 = job345.and_then([](GL::any const& t0) {
            using namespace GL::literals;
            if (t0.cast<GL::millisecond>() != 10'000_ms) std::cout << "SHOULD HAVE MATCHED " << __LINE__ << std::endl;
            std::cout << "Third and done.\n";
        });

        //if (auto sw = GL::stopwatch::debug_timer("MULTI_THREADED gpu_job 0.25"); false) {
        //    size_t len_bytes = 100;
        //    auto job1 = GL::parallel::async([&len_bytes]() {
        //        return len_bytes;
        //    });
        //    auto job2 = GL::parallel::async([job1]() {
        //        return GL::parallel::async([job1]() {
        //            return (float)job1.get();
        //        }).get();
        //    });
        //    auto job3 = GL::parallel::async([job2]() {
        //        std::cout << job2.get() << std::endl;
        //    });
        //}
        //if (auto sw = GL::stopwatch::debug_timer("MULTI_THREADED gpu_job 0.5"); false) {
        //    size_t len_bytes = 100;
        //    GL::parallel::async([&len_bytes]() {
        //        return len_bytes;
        //    }).and_then([](size_t& rhs) {
        //        return GL::parallel::async([RHS = (size_t)rhs]() {
        //            return (float)RHS;
        //        }).get();
        //    }).and_then([](float& rhs) {
        //        std::cout << rhs << std::endl;
        //    });
        //}
    }






    if (1) {
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
        matrix_kernel<float> kernel(matrix<float>::from_vector({
            1, 1, 1,
            1, 0, 1,
            1, 1, 1
            }, 3));
        auto state = (matrix<float>::random(game_h, game_w, 1) > 0.4f).cast<unsigned int>();

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
            auto nHood = state.convolve(static_matrix_kernel<float>{ &kernel });

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

#if 1
#if 0
            auto statef = state.cast<float>();
            auto blur_1 = statef.convolve(matrix<float>::guassian_kernel(3, 3));
            auto blur_2 = blur_1.convolve(matrix<float>::guassian_kernel(7, 7));
            auto blur_3 = blur_2.convolve(matrix<float>::guassian_kernel(11, 11));
            auto blur_4 = blur_3.convolve(matrix<float>::guassian_kernel(23, 23));
            auto blur_5 = blur_4.convolve(matrix<float>::guassian_kernel(53, 53));
            std::cout << (statef + blur_1 + blur_2 + blur_3 + blur_4 + blur_5).ASCII().to_string({}, true) << std::endl;
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
                void calculate_mip_maps(matrix<float>&& srce) {
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
            std::cout << img.sum().resize_stretch(game_h, game_w, 1).ASCII().to_string({}, true) << std::endl;

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
#else
            std::cout << state.ASCII().to_string({}, true) << std::endl;
#endif
            // state += ((state > 0).cast<float>().convolve(matrix<float>::guassian_kernel<7,7>()) * (matrix<float>::random(game_h, game_w, 1) >= 0.995f).cast<float>()).cast<unsigned int>();
            // state = state.min(1);

            std::cout << "" << std::endl;
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

            std::cout << GL::printf("min{ %f }  q1{ %f }  median{ %f }  q2{ %f }  max{ %f }  ", q0, q1, q2, q3, q4) + GL::arena_memory_pool::debug() + "         \t" << std::endl;
            std::cout << std::flush;

            //while (sw.stop() < 1.0 / 60.0) {
            //    std::this_thread::yield();
            //}
        }



    }

#if 0
    if (0) {
        GL::atomic_shared_ptr<GL::any> ptr;
        // CategoricalCache<GL::atomic_shared_ptr<any>, 4> cache;
        GL::parallel::For(0, 1'000'000, [&](size_t i) {
            switch (i % 4) {
            case 0:
                ptr /**cache.at<0>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            case 1:
                ptr /**cache.at<1>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            case 2:
                ptr /**cache.at<2>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            case 3:
                ptr /**cache.at<3>(i / 10000)*/ = GL::make_shared<GL::any>(GL::any::fast_any::instance(GL::printf("%zu", i)));
                break;
            }
            });
        //std::cout << cache.current<0>()->load_fast()->cast<GL::string&>() << std::endl;
        //std::cout << cache.current<1>()->load_fast()->cast<GL::string&>() << std::endl;
        //std::cout << cache.current<2>()->load_fast()->cast<GL::string&>() << std::endl;
        //std::cout << cache.current<3>()->load_fast()->cast<GL::string&>() << std::endl;

    }

    while (1)
        if (auto timer = GL::stopwatch().debug_timer("1B calls: \t")) {
            CategoricalCache<any> cache;

            //for (size_t i = 0; i < 1'000'000; ++i){
                // auto g = cache.guard_critical_section();
            GL::parallel::For(0, 1'000'000/*'000*/, [&](size_t i) {
                cache.insert(i % 10000, GL::make_shared<any>(any::instance(GL::printf("%zu - 0", i))));
                });




            //// GL::atomic_shared_ptr<any> ptr;
            //CategoricalCache<any> cache;
            //GL::parallel::For(0, 1'000'000/*'000*/, [&](size_t i) {
            //    switch (i % 4) {
            //    case 0:
            //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 0", i)); }, i);
            //        break;
            //    case 1:
            //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 1", i)); }, i);
            //        break;
            //    case 2:
            //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 2", i)); }, i);
            //        break;
            //    case 3:
            //        cache.at(i % 10000, [](size_t const& i) { return any::instance(GL::printf("%zu - 3", i)); }, i);
            //        break;
            //    default:
            //        break;
            //    }
            //});
            //std::cout << cache.current()->cast<GL::string&>() << std::endl;
            ////std::cout << cache.current<1>()->cast<GL::string&>() << std::endl;
            ////std::cout << cache.current<2>()->cast<GL::string&>() << std::endl;
            ////std::cout << cache.current<3>()->cast<GL::string&>() << std::endl;
        };
#endif

    while (1) {
        GL::parallel_generalized_allocator alloc([](size_t sz) -> void* {
            return GL::malloc(sz);
            }, [](void* p) -> void {
                GL::mfree(p);
            });
        GL::parallel::For(0ull, 100'000ull, [&](size_t i) {
            int* p = alloc.alloc<int>((int)GL::util::rand_very_fast(1'000.0, (double)(i + 1)));
            alloc.free(p);
            });
    }

    // experimenting with an (unlikely) memory leak associated with epoch_btree_map? Or just a design flaw?
    while (true) {

#if 0
#include <iostream>
#include <cmath>
#include <cstdint>
#include <limits>

        // Decomposes a ratio into a 16-bit packed format using dynamic scale flags
        uint16_t compress_ratio_with_scale(double ratio) {
            constexpr int TOTAL_PAYLOAD_BITS = 10; // 10 bits available for the fraction split

            if (ratio <= 0.0) {
                return (1U << 12) | 0; // Fallback: 0/1 split
            }

            uint16_t best_num = 1;
            uint16_t best_den = 1;
            int best_num_bits = 1;
            uint16_t best_xn_flag = 0;
            uint16_t best_xd_flag = 0;
            double min_error = std::numeric_limits<double>::max();

            // Define the scale factors to evaluate
            const double num_scales[] = { 1.0, 1000.0 };
            const double den_scales[] = { 1.0, 1000.0 };

            // 1. Loop through both scale flags
            for (int xn = 0; xn <= 1; ++xn) {
                for (int xd = 0; xd <= 1; ++xd) {
                    double ns = num_scales[xn];
                    double ds = den_scales[xd];

                    // 2. Loop through all legal bit splits for the 10 payload bits
                    for (int num_bits = 1; num_bits <= (TOTAL_PAYLOAD_BITS - 1); ++num_bits) {
                        int den_bits = TOTAL_PAYLOAD_BITS - num_bits;

                        uint32_t max_n = (1U << num_bits) - 1;
                        uint32_t max_d = (1U << den_bits) - 1;

                        for (uint32_t d = 1; d <= max_d; ++d) {
                            // Base target adjusted by our multiplier flags
                            double exact_num = ratio * (d * ds) / ns;

                            uint32_t n = static_cast<uint32_t>(std::round(exact_num));
                            if (n < 1) n = 1;
                            if (n > max_n) n = max_n;

                            // Calculate actual resulting ratio with multipliers applied
                            double current_ratio = (static_cast<double>(n) * ns) / (static_cast<double>(d) * ds);
                            double error = std::abs(ratio - current_ratio);

                            if (error < min_error) {
                                min_error = error;
                                best_num = static_cast<uint16_t>(n);
                                best_den = static_cast<uint16_t>(d);
                                best_num_bits = num_bits;
                                best_xn_flag = xn;
                                best_xd_flag = xd;

                                if (min_error == 0.0) {
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // 3. Pack fields into a single uint16_t
            int den_bits = TOTAL_PAYLOAD_BITS - best_num_bits;

            uint16_t packed_value = 0;
            packed_value |= (static_cast<uint16_t>(best_num_bits) & 0x0F) << 12; // Bits 12-15: 4-bit header
            packed_value |= (best_xn_flag & 1) << 11;                             // Bit 11: Numerator scale
            packed_value |= (best_xd_flag & 1) << 10;                             // Bit 10: Denominator scale
            packed_value |= (best_num & ((1U << best_num_bits) - 1)) << den_bits; // Shifted numerator payload
            packed_value |= (best_den & ((1U << den_bits) - 1));                  // Denominator payload at bottom

            return packed_value;
        }

        // Unpacks the uint16_t and decodes it back to a standard double ratio
        double decompress_scale_to_ratio(uint16_t packed_value) {
            int num_bits = (packed_value >> 12) & 0x0F;
            int xn_flag = (packed_value >> 11) & 1;
            int xd_flag = (packed_value >> 10) & 1;

            int den_bits = 10 - num_bits;

            uint16_t num_mask = (1U << num_bits) - 1;
            uint16_t den_mask = (1U << den_bits) - 1;

            uint16_t base_num = (packed_value >> den_bits) & num_mask;
            uint16_t base_den = packed_value & den_mask;

            if (base_den == 0) return 0.0;

            double final_num = static_cast<double>(base_num) * (xn_flag ? 1000.0 : 1.0);
            double final_den = static_cast<double>(base_den) * (xd_flag ? 1000.0 : 1.0);

            return final_num / final_den;
        }

        void test_ratio(double target) {
            uint16_t packed = compress_ratio_with_scale(target);
            double restored = decompress_scale_to_ratio(packed);

            int num_bits = (packed >> 12) & 0x0F;
            int xn = (packed >> 11) & 1;
            int xd = (packed >> 10) & 1;

            std::cout << "Target: " << target << "\n"
                << "  Packed Value:   0x" << std::hex << packed << std::dec << "\n"
                << "  Bit Split:      " << num_bits << " num bits / " << (10 - num_bits) << " den bits\n"
                << "  Scale Flags:    xN=" << xn << ", xD=" << xd << "\n"
                << "  Decoded Ratio:  " << restored << " (Error: " << std::abs(target - restored) << ")\n\n";
        }

        int main() {
            test_ratio(12500.5);   // High target: Evaluates seamlessly via xN flag
            test_ratio(0.00015);   // Low target: Evaluates seamlessly via xD flag
            test_ratio(3.14159);   // Normal target: Evaluates accurately with flags at 0
            return 0;
        }
#endif 


        while (false) {
            if (true) {
                GL::atomic_uint_stack
                    stack;
                std::set<unsigned int>
                    set;
                unsigned int
                    j;
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    stack.push(i);
                    });
                while (stack.try_pop(j))
                    set.insert(j);
                if (set.size() != 1'000'000ull)
                    std::cout << "atomic_uint_stack failure" << std::endl;
            }
            if (true) {
                GL::parallel_atomic_uint_stack
                    stack2;
                std::set<unsigned int>
                    set2;
                unsigned int
                    j;
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    stack2.push(i);
                    });
                while (stack2.try_pop(j))
                    set2.insert(j);
                if (set2.size() != 1'000'000ull)
                    std::cout << "parallel_atomic_uint_stack failure" << std::endl;
            }

            if (true) {
                GL::atomic_uint_stack
                    stack;
                concurrency::concurrent_unordered_map<unsigned int, unsigned int>
                    set;
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    stack.push(i);
                    });
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    if (!stack.try_pop(i))
                        std::cout << "atomic_uint_stack failure" << std::endl;
                    set[i] = i;
                    });
                if (set.size() != 1'000'000ull)
                    std::cout << "atomic_uint_stack failure" << std::endl;
            }
            if (true) {
                GL::parallel_atomic_uint_stack
                    stack2;
                concurrency::concurrent_unordered_map<unsigned int, unsigned int>
                    set2;
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    stack2.push(i);
                    });
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    if (!stack2.try_pop(i))
                        std::cout << "parallel_atomic_uint_stack failure" << std::endl;
                    set2[i] = i;
                    });
                if (set2.size() != 1'000'000ull)
                    std::cout << "parallel_atomic_uint_stack failure" << std::endl;
            }

            if (true) {
                GL::atomic_uint_stack
                    stack;
                concurrency::concurrent_unordered_map<unsigned int, unsigned int>
                    set;
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    stack.push(i);
                    if (!stack.try_pop(i))
                        std::cout << "atomic_uint_stack failure" << std::endl;
                    set[i] = i;
                    });
                if (set.size() != 1'000'000ull)
                    std::cout << "atomic_uint_stack failure" << std::endl;
            }
            if (true) {
                GL::parallel_atomic_uint_stack
                    stack2;
                concurrency::concurrent_unordered_map<unsigned int, unsigned int>
                    set2;
                GL::parallel::For(0u, 1'000'000u, [&](unsigned int i) {
                    stack2.push(i);
                    if (!stack2.try_pop(i))
                        std::cout << "parallel_atomic_uint_stack failure" << std::endl;
                    set2[i] = i;
                    });
                if (set2.size() != 1'000'000ull)
                    std::cout << "parallel_atomic_uint_stack failure" << std::endl;
            }

            if (auto timer = GL::stopwatch::debug_timer("atomic_uint_stack"); true) {
                GL::atomic_uint_stack
                    stack;
                unsigned int j;
                for (int k = 0; k < 100; ++k) {
                    while (stack.try_pop(j)) {
                        std::cout << j << std::endl;
                    }
                    for (int i = 0; i < 1'000'000; ++i) {
                        stack.push(i);
                    }
                    for (int i = 1'000'000 - 1; i >= 0; --i) {
                        if (stack.try_pop(j)) {
                            if (j != i)
                                std::cout << j << std::endl;
                        }
                    }
                    while (stack.try_pop(j)) {
                        std::cout << j << std::endl;
                    }
                }
            }
            if (auto timer = GL::stopwatch::debug_timer("aba_problem::stack<unsigned int>"); false) {
                GL::aba_problem::stack<unsigned int>
                    stack;

                unsigned int j;
                for (int k = 0; k < 100; ++k) {
                    while (stack.try_pop(j)) {
                        std::cout << j << std::endl;
                    }
                    for (int i = 0; i < 1'000'000; ++i) {
                        stack.push(i);
                    }
                    for (int i = 1'000'000 - 1; i >= 0; --i) {
                        if (stack.try_pop(j)) {
                            if (j != i)
                                std::cout << j << std::endl;
                        }
                    }
                    while (stack.try_pop(j)) {
                        std::cout << j << std::endl;
                    }
                }
            }
            if (auto timer = GL::stopwatch::debug_timer("parallel_atomic_uint_stack"); true) {
                GL::parallel_atomic_uint_stack
                    stack;
                unsigned int j;
                for (int k = 0; k < 100; ++k) {
                    while (stack.try_pop(j)) {
                        std::cout << j << std::endl;
                    }
                    for (int i = 0; i < 1'000'000; ++i) {
                        stack.push(i);
                    }
                    for (int i = 1'000'000 - 1; i >= 0; --i) {
                        if (stack.try_pop(j)) {
                            //if (j != i)
                                //std::cout << j << std::endl;
                        }
                        else {
                            std::cout << i << std::endl;
                        }
                    }
                    while (stack.try_pop(j)) {
                        std::cout << j << std::endl;
                    }
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("\tParallel atomic_uint_stack"); true) {
                GL::atomic_uint_stack
                    stack;
                GL::parallel::For(0, 100, [&]() {
                    unsigned int j;
                    for (int i = 0; i < 10'000; ++i) {
                        stack.push(i);
                    }
                    for (int i = 10'000 - 1; i >= 0; --i) {
                        if (!stack.try_pop(j)) {
                            std::cout << i << std::endl;
                        }
                    }
                    });
            }
            if (auto timer = GL::stopwatch::debug_timer("\tParallel aba_problem::stack<unsigned int>"); false) {
                GL::aba_problem::stack<unsigned int>
                    stack;
                GL::parallel::For(0, 100, [&]() {
                    unsigned int j;
                    for (int i = 0; i < 10'000; ++i) {
                        stack.push(i);
                    }
                    for (int i = 10'000 - 1; i >= 0; --i) {
                        if (!stack.try_pop(j)) {
                            std::cout << i << std::endl;
                        }
                    }
                    });
            }
            if (auto timer = GL::stopwatch::debug_timer("\tParallel parallel_atomic_uint_stack"); true) {
                GL::parallel_atomic_uint_stack
                    stack;
                GL::parallel::For(0, 100, [&]() {
                    unsigned int j;
                    for (int i = 0; i < 10'000; ++i) {
                        stack.push(i);
                    }
                    for (int i = 10'000 - 1; i >= 0; --i) {
                        if (!stack.try_pop(j)) {
                            std::cout << i << std::endl;
                        }
                    }
                    });
            }
        }
        while (true) {
            try {
                if (1) {
                    GL::versioned_wrapper<GL::foot, false>
                        Wrap;
                    std::atomic<long>
                        version = 0;
                    auto timer =
                        GL::stopwatch::debug_timer("\tversioned_wrapper<false> load_fast");
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        if (i % 10'000 == 0)
                            version.fetch_add(1, std::memory_order::memory_order_relaxed);
                        Wrap.store_f(version.load(),
                            [&]() -> GL::foot { return (float)i; });
                        if (auto p = Wrap.load_fast(); p)
                            p->second += GL::foot(1);
                        });
                }

                if (1) {
                    GL::versioned_wrapper<GL::foot, true>
                        Wrap;
                    std::atomic<long>
                        version = 0;
                    auto timer =
                        GL::stopwatch::debug_timer("\tversioned_wrapper<true> load_fast");
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        if (i % 10'000 == 0)
                            version.fetch_add(1, std::memory_order::memory_order_relaxed);
                        Wrap.store_f(version.load(),
                            [&]() -> GL::foot { return (float)i; });
                        if (auto p = Wrap.load_fast(); p)
                            p->second += GL::foot(1);
                        });
                }

                if (1) {
                    GL::versioned_wrapper<GL::foot, true>
                        Wrap;
                    std::atomic<long>
                        version = 0;
                    auto timer =
                        GL::stopwatch::debug_timer("\tversioned_wrapper<true> guard_critical_section");
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        if (i % 10'000 == 0)
                            version.fetch_add(1, std::memory_order::memory_order_relaxed);
                        Wrap.store_f(version.load(),
                            [&]() -> GL::foot { return (float)i; });
                        auto g{ Wrap.guard_critical_section() };
                        Wrap.ref() += GL::foot(1);
                        });
                }
            }
            catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }


    }




#if 0
    while (false) {
        using namespace GL::literals;
        if (1) {
            Function unix_s = wrap_function({}, [](GL::second x, GL::minute y) -> GL::hour { return x + y; });
            Function unix_s_1Default = wrap_function({ "", { { "x", nullptr }, { "y", any::instance(1_min) } } }, [](GL::second x, GL::minute y) -> GL::hour { return x + y; });
            Function unix_s_2Default = wrap_function({ "", { { "x", any::instance(1_s) }, { "y", any::instance(1_min) } } }, [](GL::second x, GL::minute y) -> GL::hour { return x + y; });

            std::cout << unix_s.do_call<GL::hour>(1_s, 1_min) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(any::instance(1_s), any::instance(1_min)) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(atomic_any::instance(1_s), atomic_any::instance(1_min)) << std::endl;

            std::cout << unix_s.async_call<GL::hour>(1_s, 1_min).get() << std::endl;
            std::cout << unix_s.async_call<GL::hour>(any::instance(1_s), any::instance(1_min)).get() << std::endl;
            std::cout << unix_s.async_call<GL::hour>(atomic_any::instance(1_s), atomic_any::instance(1_min)).get() << std::endl;

            std::cout << GL::prints(GL::string(R"(
Argument Error(s): 
    Correct Inputs:
        Standard:   %f; 
        Any:        %f; 
        Atomic Any: %f; 
    1 Wrong Input:
        Standard:   %f; 
        Any:        %f; 
        Atomic Any: %f;
    2 Wrong Inputs:
        Standard:   %f; 
        Any:        %f; 
        Atomic Any: %f;
    Missing Input (No Default):
        Standard:   %f; 
        Any:        %f; 
        Atomic Any: %f;
    2 Missing Inputs (No Default):
        All:        %f;
    Missing Input (1 Default):
        Standard:   %f; 
        Any:        %f; 
        Atomic Any: %f;
    2 Missing Inputs (1 Default):
        All:        %f;
    Missing Input (2 Default):
        Standard:   %f; 
        Any:        %f; 
        Atomic Any: %f;
    2 Missing Inputs (2 Default):
        All:        %f;
)").remove_leading_and_trailing_whitespace(),
unix_s.argument_error(1_s, 1_min),
unix_s.argument_error(any::instance(1_s), any::instance(1_min)),
unix_s.argument_error(atomic_any::instance(1_s), atomic_any::instance(1_min)),

unix_s.argument_error(1_min, 1_min),
unix_s.argument_error(any::instance(1_min), any::instance(1_min)),
unix_s.argument_error(atomic_any::instance(1_min), atomic_any::instance(1_min)),

unix_s.argument_error(1_min, 1_s),
unix_s.argument_error(any::instance(1_min), any::instance(1_s)),
unix_s.argument_error(atomic_any::instance(1_min), atomic_any::instance(1_s)),

unix_s.argument_error(1_s),
unix_s.argument_error(any::instance(1_s)),
unix_s.argument_error(atomic_any::instance(1_s)),
unix_s.argument_error(),

unix_s_1Default.argument_error(1_s),
unix_s_1Default.argument_error(any::instance(1_s)),
unix_s_1Default.argument_error(atomic_any::instance(1_s)),
unix_s_1Default.argument_error(),

unix_s_2Default.argument_error(1_s),
unix_s_2Default.argument_error(any::instance(1_s)),
unix_s_2Default.argument_error(atomic_any::instance(1_s)),
unix_s_2Default.argument_error()
);
        }
        if (1) {
            Function unix_s = wrap_function({}, [](GL::second x, GL::minute y) -> any { return any::instance<GL::hour>(x + y); });
            std::cout << unix_s.do_call<GL::hour>(1_s, 1_min) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(any::instance(1_s), any::instance(1_min)) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(atomic_any::instance(1_s), atomic_any::instance(1_min)) << std::endl;

            std::cout << unix_s.async_call<GL::hour>(1_s, 1_min).get() << std::endl;
            std::cout << unix_s.async_call<GL::hour>(any::instance(1_s), any::instance(1_min)).get() << std::endl;
            std::cout << unix_s.async_call<GL::hour>(atomic_any::instance(1_s), atomic_any::instance(1_min)).get() << std::endl;
        }
        if (1) {
            Function unix_s = wrap_function({}, [](GL::second x, GL::minute y) -> atomic_any { return atomic_any::instance<GL::hour>(x + y); });
            std::cout << unix_s.do_call<GL::hour>(1_s, 1_min) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(any::instance(1_s), any::instance(1_min)) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(atomic_any::instance(1_s), atomic_any::instance(1_min)) << std::endl;

            std::cout << unix_s.async_call<GL::hour>(1_s, 1_min).get() << std::endl;
            std::cout << unix_s.async_call<GL::hour>(any::instance(1_s), any::instance(1_min)).get() << std::endl;
            std::cout << unix_s.async_call<GL::hour>(atomic_any::instance(1_s), atomic_any::instance(1_min)).get() << std::endl;
        }

        if (1) {
            Function unix_s = wrap_function({}, [](GL::second x, GL::datetime y) -> GL::hour { return x + (GL::minute)y; });
            auto N = GL::datetime::Now();
            std::cout << unix_s.do_call<GL::hour>(0_s, N) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(120_s, N) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(12000_s, N) << std::endl;
            std::cout << unix_s.do_call<GL::hour>(120000000000_s, N) << std::endl;
        }
        if (1) {
            Function unix_s = wrap_function({}, [](GL::datetime y, GL::second x) -> GL::datetime { return y + x; });
            auto N = GL::datetime::Now();
            std::cout << unix_s.do_call<GL::datetime>(N, 0_s) << std::endl;
            std::cout << unix_s.do_call<GL::datetime>(N, 120_s) << std::endl;
            std::cout << unix_s.do_call<GL::datetime>(N, 1_d) << std::endl;
            std::cout << unix_s.do_call<GL::datetime>(N, 12000_s) << std::endl;
            std::cout << unix_s.do_call<GL::datetime>(N, 1200000_s) << std::endl;
            std::cout << unix_s.do_call<GL::datetime>(N, 120000000_s) << std::endl;
        }

        while (true) {
            if (1) {
                Function converter = wrap_converter<GL::foot, GL::value>();
                GL::stopwatch_group timer2("wrap_converter<GL::foot, GL::value>");
                GL::parallel::For(0, 1'000'000, [&](size_t i) {
                    auto timer = timer2.debug_timer();
                    if (converter.do_call<GL::value>(GL::foot(i)) != GL::foot(i)) throw "DID NOT EQUAL";
                    });
            }
            if (1) {
                Function converter = wrap_converter<GL::foot, GL::value>();
                GL::stopwatch_group timer2("wrap_converter<GL::foot, GL::value>");
                GL::parallel::For(0, 1'000'000, [&](size_t i) {
                    auto timer = timer2.debug_timer();
                    if (converter.do_call<::any>(GL::foot(i)).cast<GL::value>() != GL::foot(i)) throw "DID NOT EQUAL";
                    });
            }
            if (1) {
                auto converter = GL::make_converter< GL::foot, GL::value>();
                GL::stopwatch_group timer2("GL::make_converter< GL::foot, GL::value>");
                GL::parallel::For(0, 1'000'000, [&](size_t i) {
                    auto timer = timer2.debug_timer();
                    if (converter->operator()(GL::any::fast_any::instance(GL::foot(i))).cast<GL::value>() != GL::foot(i)) throw "DID NOT EQUAL";
                    });
            }
        }





        if (1) {
            Function converter = wrap_converter<GL::value, GL::foot>();
            GL::stopwatch_group timer2("wrap_converter<GL::value, GL::foot>");
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();
                if (converter.do_call<GL::foot>(GL::value(GL::foot(i))) != GL::foot(i)) throw "DID NOT EQUAL";
                });
        }



        if (1) {
            Function converter = wrap_function({ "int_to_double", { { "from", nullptr } }, "Compiled function to convert between types.", true }, [](int x) -> double { return x; });
            GL::stopwatch_group timer2("int_to_double FAST");
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();
                if (converter.do_call<double>((int)i) != (double)(int)i) throw "DID NOT EQUAL";
                });
        }
        if (1) {
            Function converter = wrap_converter<int, double>();
            GL::stopwatch_group timer2("wrap_converter<int, double>");
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();
                if (converter.do_call<double>((int)i) != (double)(int)i) throw "DID NOT EQUAL";
                });
        }
        if (1) {
            Function converter = wrap_function({ "int_to_double", { { "from", nullptr } }, "Compiled function to convert between types.", false }, [](int const& x) -> double { return x; });
            GL::stopwatch_group timer2("int_to_double SLOW");
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();
                if (converter.do_call<double>((int)i) != (double)(int)i) throw "DID NOT EQUAL";
                });
        }











        std::cout << "Linear:\n";
        if (1) {
            GL::stopwatch_group timer2;
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                any temp;
                switch (i % 3) {
                case 0: temp = any::instance(100); break;
                case 1: temp = any::instance(100.0); break;
                case 2: temp = any::instance(100.0f); break;
                }
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                any temp = any::instance(std::string("TEST"));
                temp = any::instance(100);
                temp.cast<int>() += 100;
                if (temp.cast<int>() != 200) throw "SHOULD HAVE MATCHED";
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            any temp = any::instance(GL::meter(0));
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                temp.cast<GL::meter>() += GL::meter(100);
                });
        }

        std::cout << "Atomic, Linearly:\n";
        if (1) {
            GL::stopwatch_group timer2;
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                atomic_any temp;
                switch (i % 3) {
                case 0: temp = atomic_any::instance(100); break;
                case 1: temp = atomic_any::instance(100.0); break;
                case 2: temp = atomic_any::instance(100.0f); break;
                }
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                atomic_any temp = atomic_any::instance(std::string("TEST"));
                temp = atomic_any::instance(100);
                temp.cast<int>() += 100;
                if (temp.cast<int>() != 200) throw "SHOULD HAVE MATCHED";
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            atomic_any temp = atomic_any::instance(GL::meter(0));
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                temp.cast<GL::meter>() += GL::meter(100);
                });
        }

        std::cout << "Atomic, In Parallel:\n";
        if (1) {
            GL::stopwatch_group timer2;
            atomic_any temp;
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                switch (i % 3) {
                case 0: temp = atomic_any::instance(100); break;
                case 1: temp = atomic_any::instance(100.0); break;
                case 2: temp = atomic_any::instance(100.0f); break;
                }
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            atomic_any temp = atomic_any::instance(100);
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                switch (i % 3) {
                case 0: temp = temp + GL::type::Const; break;
                case 1: temp = temp + GL::type::Reference; break;
                case 2: temp = temp + GL::type::Temporary; break;
                }
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            atomic_any temp = atomic_any::instance(GL::meter(0));
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();
                temp.cast<GL::meter>() += GL::meter(100);
                });
        }

        std::cout << "Linear with Atomics:\n";
        if (1) {
            GL::stopwatch_group timer2;
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                atomic_any temp;
                switch (i % 3) {
                case 0: temp = any::instance(100); break;
                case 1: temp = any::instance(100.0); break;
                case 2: temp = any::instance(100.0f); break;
                }
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();

                any temp = any::instance(std::string("TEST"));
                temp = any::instance(100);
                temp.cast<int>() += 100;
                if (temp.cast<int>() != 200) throw "SHOULD HAVE MATCHED";
                });
        }
        if (1) {
            GL::stopwatch_group timer2;
            atomic_any temp = any::instance(GL::meter(0));
            GL::parallel::Std_For(0, 1'000'000, [&](size_t i) {
                auto timer = timer2.debug_timer();
                temp.cast<GL::meter>() += GL::meter(100);
                });
        }

        std::cout << "\n";
    }

    while (false) {
        std::cout << "\n";

        //for (int i = 0; i < 100; ++i)
            //std::cout << GL::util::rand_very_fast() << std::endl;

        // GL::parallel::fire_and_forget([]() {
        if (auto timer = GL::stopwatch::debug_timer("GL::task automatic scheduling"); true) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                auto time{ timer2.debug_timer() };

                GL::parallel_generic_array_allocator<32768> alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::task(0, 1'000, [](size_t) {

                    })->and_then(0, 1'000, [&](size_t i) {
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
                        })->and_then(0, 1'000'000, [](size_t) {

                            })->and_then(0, 1'000'000, [&](size_t i) {
                                if (GL::util::rand_very_fast(0, 100) > 50) {
                                    switch (i % 10) {
                                    case 0:
                                        ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                        break;
                                    case 1:
                                        ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                        break;
                                    case 2:
                                        ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                        break;
                                    case 3:
                                        ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(10, 100)));
                                        break;
                                    case 4:
                                        ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(10, 100)));
                                        break;
                                    case 5:
                                        ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(10, 100)));
                                        break;
                                    case 6:
                                        ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                        break;
                                    case 7:
                                        ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                        break;
                                    case 8:
                                        ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                        break;
                                    case 9:
                                        ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                        break;
                                    default:
                                        break;
                                    }
                                }
                                else if (void* p; ptrs.try_pop(p)) {
                                    alloc.free(p);
                                }
                                });
            }
        }
        // });

        if (auto timer = GL::stopwatch::debug_timer("A1. For 1'000"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::For(0, 1'000, [&](size_t const& i) {
                    if (auto t = timer2.debug_timer()) {
#if 0
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }
        if (auto timer = GL::stopwatch::debug_timer("A2. Std_For 1'000"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::Std_For(0, 1'000, [&](size_t const& i) noexcept {
                    if (auto t = timer2.debug_timer()) {
#if 0
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }
        if (auto timer = GL::stopwatch::debug_timer("B3. For 1'000 (work)"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::For(0, 1'000, [&](size_t const& i) {
                    if (auto t = timer2.debug_timer()) {
#if 1
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }
        if (auto timer = GL::stopwatch::debug_timer("B4. Std_For 1'000 (work)"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::Std_For(0, 1'000, [&](size_t const& i) {
                    if (auto t = timer2.debug_timer()) {
#if 1
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }
        if (auto timer = GL::stopwatch::debug_timer("C5. For 1'000'000"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::For(0, 1'000'000, [&](size_t const& i) {
                    if (auto t = timer2.debug_timer()) {
#if 0
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<GL::any>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }
        if (auto timer = GL::stopwatch::debug_timer("C6. Std_For 1'000'000"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::Std_For(0, 1'000'000, [&](size_t const& i) noexcept {
                    if (auto t = timer2.debug_timer()) {
#if 0
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<GL::any>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }
        if (auto timer = GL::stopwatch::debug_timer("D7. For 1'000'000 (work)"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::For(0, 1'000'000, [&](size_t const& i) {
                    if (auto t = timer2.debug_timer()) {
#if 1
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<GL::any>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }
        if (auto timer = GL::stopwatch::debug_timer("D8. Std_For 1'000'000 (work)"); false) {
            GL::stopwatch_group timer2;
            for (int loops = 0; loops < 100; ++loops) {
                GL::parallel_generic_array_allocator alloc;
                GL::atomic_queue<void*> ptrs;
                GL::parallel::Std_For(0, 1'000'000, [&](size_t const& i) {
                    if (auto t = timer2.debug_timer()) {
#if 1
                        if (GL::util::rand_very_fast(0, 100) > 50) {
                            switch (i % 10) {
                            case 0:
                                ptrs.push(alloc.alloc<int>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 1:
                                ptrs.push(alloc.alloc<float>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 2:
                                ptrs.push(alloc.alloc<long double>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 3:
                                ptrs.push(alloc.alloc<std::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 4:
                                ptrs.push(alloc.alloc<GL::string>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 5:
                                ptrs.push(alloc.alloc<GL::shared_ptr<std::string>>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 6:
                                ptrs.push(alloc.alloc<any>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            case 7:
                                ptrs.push(alloc.alloc<GL::any>(GL::util::rand_very_fast(100, 1'000)));
                                break;
                            case 8:
                                ptrs.push(alloc.alloc<GL::atomic_bag<std::string>>(GL::util::rand_very_fast(10, 100)));
                                break;
                            case 9:
                                ptrs.push(alloc.alloc<GL::meter>(GL::util::rand_very_fast(100, 10'000)));
                                break;
                            default:
                                break;
                            }
                        }
                        else if (void* p; ptrs.try_pop(p)) {
                            alloc.free(p);
                        }
#endif
                    }
                    });
            }
        }

    }

    while (true) {
        while (0) {
            //GL::fast_atomic_general_allocator alloc;            
            //if (auto timer = GL::stopwatch::debug_timer("fast_atomic_general_allocator"); true) {
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        alloc.Free(alloc.Alloc(GL::util::rand_fast(32,256)));
            //    }
            //    std::vector<void*> ps(1'000'000, nullptr);
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        ps[i] = alloc.Alloc(GL::util::rand_fast(32, 256));
            //    }
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        alloc.Free(ps[i]);
            //    }
            //}

            if (auto timer = GL::stopwatch::debug_timer("parallel_array_allocator float"); false) {
                GL::parallel_array_allocator<float>
                    allocator;

                std::vector<float*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.alloc((int)GL::util::rand_very_fast(1 << 4, 1 << 16));
                        allocator.free(ptr);
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        p = allocator.alloc((int)GL::util::rand_very_fast(1 << 4, 1 << 16));
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        allocator.free(p);
                        });
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_generic_array_allocator float"); false) {
                GL::parallel_generic_array_allocator
                    allocator;

                std::vector<float*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.alloc<float>((int)GL::util::rand_very_fast(1 << 4, 1 << 16));
                        allocator.free(ptr);
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        p = allocator.alloc<float>((int)GL::util::rand_very_fast(1 << 4, 1 << 16));
                        });
                    //GL::parallel::For_Each(ptrs, [&](float*& p) {
                    //    allocator.free(p);
                    //});
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete float"); false) {
                std::vector<float*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        int n = GL::util::rand_very_fast(1 << 4, 1 << 16);
                        auto* ptr = new float[n];
                        delete[] ptr;
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        int n = GL::util::rand_very_fast(1 << 4, 1 << 16);
                        p = new float[n];
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        delete[] p;
                        });
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_array_allocator std::string"); true) {
                GL::parallel_array_allocator<std::string>
                    allocator;

                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.alloc((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                        allocator.free(ptr);
                        });
                    GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                        p = allocator.alloc((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                        // *p = "TESTING";
                        });
                    //GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                    //    allocator.free(p);
                    //});
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_generic_array_allocator std::string"); true) {
                GL::parallel_generic_array_allocator
                    allocator;

                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.alloc<std::string>((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                        allocator.free(ptr);
                        });
                    GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                        p = allocator.alloc<std::string>((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                        });
                    //GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                    //    allocator.free(p);
                    //});
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete std::string"); true) {
                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        int n = GL::util::rand_very_fast(1 << 4, 1 << 10);
                        auto* ptr = new std::string[n];
                        delete[] ptr;
                        });
                    GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                        int n = GL::util::rand_very_fast(1 << 4, 1 << 10);
                        p = new std::string[n];
                        // *p = "TESTING";
                        });
                    GL::parallel::For_Each(ptrs, [&](std::string*& p) {
                        delete[] p;
                        });
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_array_allocator std::string (linear)"); true) {
                GL::parallel_array_allocator<std::string>
                    allocator;

                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    for (int i = 0; i < 1'000'000; ++i) {
                        auto* ptr = allocator.alloc((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                        allocator.free(ptr);
                    };
                    for (std::string*& p : ptrs) {
                        p = allocator.alloc((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                    };
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_generic_array_allocator std::string (linear)"); true) {
                GL::parallel_generic_array_allocator
                    allocator;

                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    for (int i = 0; i < 1000000; ++i) {
                        auto* ptr = allocator.alloc<std::string>((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                        allocator.free(ptr);
                    };
                    for (std::string*& p : ptrs) {
                        p = allocator.alloc<std::string>((int)GL::util::rand_very_fast(1 << 4, 1 << 10));
                    };
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete std::string (linear)"); true) {
                std::vector<std::string*> ptrs(2'000, nullptr);
                if (1) {
                    for (int i = 0; i < 1000000; ++i) {
                        int n = GL::util::rand_very_fast(1 << 4, 1 << 10);
                        auto* ptr = new std::string[n];
                        delete[] ptr;
                    };
                    for (std::string*& p : ptrs) {
                        int n = GL::util::rand_very_fast(1 << 4, 1 << 10);
                        p = new std::string[n];
                    };
                    for (std::string*& p : ptrs) {
                        delete[] p;
                    };
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_allocator GL::epoch_map<int, int> singleton (linear)"); true) {
                GL::atomic_parallel_allocator<GL::epoch_map<int, int>>
                    allocator;

                std::vector<GL::epoch_map<int, int>*> ptrs(2'000, nullptr);
                if (1) {
                    for (int i = 0; i < 1000000; ++i) {
                        auto* ptr = allocator.Alloc();
                        allocator.Free(ptr);
                    };
                    for (auto*& p : ptrs) {
                        p = allocator.Alloc();
                    };
                    for (auto*& p : ptrs) {
                        allocator.Free(p);
                    };
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_allocator GL::epoch_map<int, int> singleton"); true) {
                GL::atomic_parallel_allocator<GL::epoch_map<int, int>>
                    allocator;

                std::vector<GL::epoch_map<int, int>*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.Alloc();
                        allocator.Free(ptr);
                        });
                    GL::parallel::For_Each(ptrs, [&](GL::epoch_map<int, int>*& p) {
                        p = allocator.Alloc();
                        });
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_generic_singleton_array_allocator GL::epoch_map<int, int> singleton (linear)"); true) {
                GL::parallel_generic_singleton_array_allocator
                    allocator;

                std::vector<GL::epoch_map<int, int>*> ptrs(2'000, nullptr);
                if (1) {
                    for (int i = 0; i < 1000000; ++i) {
                        auto* ptr = allocator.alloc<GL::epoch_map<int, int>>();
                        allocator.free(ptr);
                    };
                    for (auto*& p : ptrs) {
                        p = allocator.alloc<GL::epoch_map<int, int>>();
                    };
                    for (auto*& p : ptrs) {
                        allocator.free(p);
                    };
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_generic_singleton_array_allocator GL::epoch_map<int, int> singleton"); true) {
                GL::parallel_generic_singleton_array_allocator
                    allocator;

                std::vector<GL::epoch_map<int, int>*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.alloc<GL::epoch_map<int, int>>();
                        allocator.free(ptr);
                        });
                    GL::parallel::For_Each(ptrs, [&](GL::epoch_map<int, int>*& p) {
                        p = allocator.alloc<GL::epoch_map<int, int>>();
                        });
                    //GL::parallel::For_Each(ptrs, [&](GL::epoch_map<int, int>*& p) {
                    //    allocator.free(p);
                    //});
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete GL::epoch_map<int, int> singleton (linear)"); true) {
                std::vector<GL::epoch_map<int, int>*> ptrs(2'000, nullptr);
                if (1) {
                    for (int i = 0; i < 1000000; ++i) {
                        auto* ptr = new GL::epoch_map<int, int>();
                        delete ptr;
                    };
                    for (auto*& p : ptrs) {
                        p = new GL::epoch_map<int, int>();
                    };
                    for (auto*& p : ptrs) {
                        delete p;
                    };
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete GL::epoch_map<int, int> singleton"); true) {
                std::vector<GL::epoch_map<int, int>*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = new GL::epoch_map<int, int>();
                        delete ptr;
                        });
                    GL::parallel::For_Each(ptrs, [&](GL::epoch_map<int, int>*& p) {
                        p = new GL::epoch_map<int, int>();
                        });
                    GL::parallel::For_Each(ptrs, [&](GL::epoch_map<int, int>*& p) {
                        delete p;
                        });
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("parallel_generic_singleton_array_allocator float singleton"); true) {
                GL::parallel_generic_singleton_array_allocator
                    allocator;

                std::vector<float*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = allocator.alloc<float>();
                        allocator.free(ptr);
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        p = allocator.alloc<float>();
                        });
                    //GL::parallel::For_Each(ptrs, [&](float*& p) {
                    //    allocator.free(p);
                    //});
                }
            }

            if (auto timer = GL::stopwatch::debug_timer("new/delete float singleton"); true) {
                std::vector<float*> ptrs(2'000, nullptr);
                if (1) {
                    GL::parallel::For(0, 1'000'000, [&](int i) {
                        auto* ptr = new float();
                        delete ptr;
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        p = new float();
                        });
                    GL::parallel::For_Each(ptrs, [&](float*& p) {
                        delete p;
                        });
                }
            }

            //if (auto timer = GL::stopwatch::debug_timer("b_tree"); true) {
            //    GL::fast_atomic_allocator<GL::b_tree<float, int, 10>::b_treeNode, 128> allocator;
            //    GL::thread_object_no_default< GL::b_tree<float, int, 10>> trees;
            //    trees._after_construction = [&allocator](GL::b_tree<float, int, 10>& tree) { tree.nodeAllocator = &allocator; };

            //    GL::parallel::For(0, 1'000'000, [&](int i) {
            //        //for (int i = 0; i < 1'000'000; ++i){
            //        trees->Add(nullptr, i);
            //        if (auto* node = trees->NodeFind(i)) {
            //            if (i % 10 == 0) {
            //                trees->Remove(node);
            //            }
            //        }
            //        else {
            //            throw "ERROR";
            //        }
            //        });
            //}
        }



        if (1) {
            GL::epoch_btree_map<int, int>
                tree;
            GL::stopwatch_group grp("epoch_btree_map 1");
            if (true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i] = i;
                    });
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp("epoch_btree_map 2");
            if (true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i % 10'000] = i;
                    tree.erase(i % 10'000);
                    });
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp("epoch_btree_map 3");
            if (true) {
                GL::parallel::For(0, 1'000'000, [&](int i) {
                    auto time = grp.debug_timer();
                    tree.erase(i % 100);
                    tree[i % 100] = i; // protected (temporarily!) by the epoch-based protections.                     
                    });
            }
        }

        if (1) {
            GL::epoch_btree_map<int, int>
                tree;
            GL::stopwatch_group grp("epoch_btree_map 1 linear");
            if (true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i] = i;
                };
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp("epoch_btree_map 2 linear");
            if (true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    auto g{ tree.guard_critical_section() };
                    tree[i % 10'000] = i;
                    tree.erase(i % 10'000);
                };
            }
        }
        if (1) {
            GL::epoch_btree_map<GL::value, int>
                tree;
            GL::stopwatch_group grp("epoch_btree_map 3 linear");
            if (true) {
                for (auto i = 0; i < 1'000'000; ++i) {
                    auto time = grp.debug_timer();
                    tree.erase(i % 100);
                    tree[i % 100] = i; // protected (temporarily!) by the epoch-based protections.                     
                };
            }
        }
    }
#endif

    while (true) {
        GL::epoch_btree<int, int>
            tree;
        //std::vector<typename decltype(tree)::epoch_btreeNode*> arr;
        //arr.resize(1'000'000);

        //for (int i = 0; i < 100; ++i) {
        //    tree.Add((int)i, i);
        //}
        if (auto timer = GL::stopwatch::debug_timer("epoch_btree"); true) {
            GL::parallel::For(0, 10'000'000, [&](int i) {
                auto g{ tree.guard_critical_section() };
                auto node = tree.Add((int)i, i);
                //arr[i] = node;

                if (*node->object() != i) {
                    std::cout << GL::printf("Intended 1: %i, Found 1: %i\n", i, *node->object());
                }

                //node->unlock();
                //if (!tree.root->validate_node_structure(node)) {                                        
                    //node->validate_node_structure(node);
                    //std::cout << GL::printf("Not validated! %i @ %i\n", i, node->key);
                //}

                //auto node = tree.GetNextLeaf(tree.root, false, true);
                //while (node) {
                //    node = tree.GetNextLeaf(node, true, true);
                //}

                ////if ((i % 10'000) == 0) {
                //    auto g = tree.guard_critical_section();
                //    if (auto node2 = tree.NodeFindSmallestLargerEqual(i, tree.root, false, true)) {
                //        if (*node2->object() != i) {
                //            if (node != node2) {
                //                std::cout << GL::printf("Intended 2: %i, Found 2: %i\n", i, *node2->object());
                //                node2 = tree.NodeFindSmallestLargerEqual(i, tree.root, true, true);
                //                if (*node2->object() != i) {
                //                    std::cout << GL::printf("Intended 3: %i, Found 3: %i\n", i, *node2->object());
                //                }
                //            }
                //        }
                //        node2->unlock_shared();
                //        
                //        
                //        //while (node) {
                //        //    std::cout << *node->object() << std::endl;
                //        //    node = tree.GetNextLeaf(node, true, true);
                //        //}
                //    }
                ////}

                });

            GL::parallel::For(0, 10'000'000, [&](int i) {
                auto g{ tree.guard_critical_section() };
                if (auto [node2, locker] = tree.NodeFindSmallestLargerEqual(i, false); node2) {
                    if (*node2->object() != i) {
                        std::cout << GL::printf("Intended 4: %i, Found 4: %i\n", i, *node2->object());
                        //if (node2 = arr[i]) {
                        //    if (*node2->object() != i) {
                        //        std::cout << GL::printf("Intended 4.1: %i, Found 4.1: %i\n", i, *node2->object());
                        //    }
                        //}

                    }
                }
                });
        }



        auto g = tree.guard_critical_section();
        auto [node, locker] = tree.GetRootShared();
        node = tree.GetNextLeaf(node, locker);
        while (node) {
            std::cout << *node->object() << std::endl;
            node = tree.GetNextLeaf(node, locker);
        }
    }




    // object pooling test. 3-4 times faster than normal allocation / deallocation. 
    while (true) {
        if (auto timer = GL::stopwatch().debug_timer("Normal Allocation / Deletion (int)"); false) {
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = new int();
                delete p;
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = new int();
                }
                for (int j = 0; j < 36; ++j) {
                    delete arrs[j];
                }
                });
        }
        if (auto timer = GL::stopwatch().debug_timer("Normal Allocation / Deletion (GL::string)"); false) {
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = new GL::string();
                delete p;
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = new GL::string();
                }
                for (int j = 0; j < 36; ++j) {
                    delete arrs[j];
                }
                });
        }

        if (auto timer = GL::stopwatch().debug_timer("atomic_parallel_allocator<int>"); true) {
            GL::atomic_parallel_allocator<int> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
                });
        }
        if (auto timer = GL::stopwatch().debug_timer("atomic_parallel_allocator<GL::string>"); true) {
            GL::atomic_parallel_allocator<GL::string> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
                });
        }

        //if (auto timer = GL::stopwatch().debug_timer("GL::epoch_search_tree<GL::string, long long, 10>"); false) {
        //    GL::epoch_search_tree<GL::value, size_t, 10> tree;
        //    GL::parallel::For(0, 1'000'000, [&](size_t i) {
        //        auto g = tree.ProtectCurrentEpoch();
        //        if (i % 100 == 0) {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);
        //            tree.Remove(ptr);
        //            ptr->object()->operator=(i);
        //        }
        //        else {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);
        //            ptr->object()->operator=(i);
        //        }           
        //    });
        //}
        //if (auto timer = GL::stopwatch().debug_timer("GL::epoch_search_tree<GL::string, long long, 10>"); true) {
        //    GL::epoch_search_tree<GL::value, size_t, 10> tree;
        //    GL::parallel::For(0, 1'000'000, [&](size_t i) {
        //        auto g = tree.guard_critical_section();
        //        if (i % 100 == 0) {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);
        //            tree.Remove(ptr);
        //            ptr->object()->operator=(i);
        //        }
        //        else {
        //            auto ptr = tree.get_or_make(i % 100, [&]() -> GL::value { return i; }, nullptr);                   
        //            ptr->object()->operator=(i);
        //        }
        //    });
        //}



        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<int> (linear only)"); true) {
            GL::fast_atomic_allocator<int> pool;
            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                auto* p = pool.Alloc();
                pool.Free(p);
            };
            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            };
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<int>"); true) {
            GL::fast_atomic_allocator<int> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
                });
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<GL::string>"); true) {
            GL::fast_atomic_allocator<GL::string> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                pool.Free(p);
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
                });
        }
        if (false) {
            GL::fast_atomic_allocator<GL::string> pool;
            while (true) {
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<GL::string>"); true) {
                    //bool quit = false;
                    //std::thread todo([&]() {
                    //    while (!quit) {
                    //        auto* p = pool.Alloc();
                    //        pool.Free(p);
                    //    }
                    //});
                    GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                        auto* p = pool.Alloc();
                        pool.Free(p);
                        });
                    GL::parallel::For(0, 1'000'000, [&](size_t i) {
                        std::array<GL::string*, 36> arrs;
                        for (int j = 0; j < 36; ++j) {
                            arrs[j] = pool.Alloc();
                        }
                        for (int j = 0; j < 36; ++j) {
                            pool.Free(arrs[j]);
                        }
                        });
                    //quit = true;
                    //todo.join();
                }
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_allocator<GL::string> consumer-producer system"); true) {
                    concurrency::concurrent_queue< GL::string* > ptrs;
                    GL::parallel::For(0, 10'000'000, [&](size_t i) {
                        if (i % 2 == 0)
                            ptrs.push(pool.Alloc());
                        else {
                            GL::string* p;
                            if (ptrs.try_pop(p)) {
                                pool.Free(p);
                            }
                        }
                        });
                    GL::string* p;
                    while (ptrs.try_pop(p)) {
                        pool.Free(p);
                    }
                }
            }
        }

        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<int> (linear only)"); true) {
            GL::fast_atomic_epoch_allocator<int> pool;

            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                auto* p = pool.Alloc();
                auto g = pool.guard_critical_section();
                pool.Free(p);
            };
            for (long long i = 1'000'000'000ll / (long long)(GL::util::get_hardware_thread_count() * 10); i > 0; --i) {
                std::array<int*, 36> arrs;
                auto g = pool.guard_critical_section();
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
            };
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<int>"); true) {
            GL::fast_atomic_epoch_allocator<int> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                auto g = pool.guard_critical_section();
                pool.Free(p);
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<int*, 36> arrs;
                auto g = pool.guard_critical_section();
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
                });
        }
        if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<GL::string>"); true) {
            GL::fast_atomic_epoch_allocator<GL::string> pool;
            GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                auto* p = pool.Alloc();
                auto g = pool.guard_critical_section();
                pool.Free(p);
                });
            GL::parallel::For(0, 1'000'000, [&](size_t i) {
                std::array<GL::string*, 36> arrs;
                auto g = pool.guard_critical_section();
                for (int j = 0; j < 36; ++j) {
                    arrs[j] = pool.Alloc();
                }
                for (int j = 0; j < 36; ++j) {
                    pool.Free(arrs[j]);
                }
                });
        }
        if (true) {
            GL::fast_atomic_epoch_allocator<GL::string> pool;
            while (true) {
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<GL::string>"); true) {
                    GL::parallel::For(0, 1'000'000'000, [&](size_t i) {
                        auto* p = pool.Alloc();
                        auto g = pool.guard_critical_section();
                        pool.Free(p);
                        });
                    GL::parallel::For(0, 1'000'000, [&](size_t i) {
                        std::array<GL::string*, 36> arrs;
                        for (int j = 0; j < 36; ++j) {
                            arrs[j] = pool.Alloc();
                        }
                        auto g = pool.guard_critical_section();
                        for (int j = 0; j < 36; ++j) {
                            pool.Free(arrs[j]);
                        }
                        });
                }
                if (auto timer = GL::stopwatch().debug_timer("fast_atomic_epoch_allocator<GL::string> consumer-producer system"); true) {
                    concurrency::concurrent_queue< GL::string* > ptrs;
                    GL::parallel::For(0, 10'000'000, [&](size_t i) {
                        if (i % 2 == 0) {
                            auto g = pool.guard_critical_section();
                            ptrs.push(pool.Alloc());
                        }
                        else {
                            auto g = pool.guard_critical_section();
                            GL::string* p;
                            if (ptrs.try_pop(p)) {
                                pool.Free(p);
                            }
                        }
                        });
                    GL::string* p;
                    auto g = pool.guard_critical_section();
                    while (ptrs.try_pop(p)) {
                        pool.Free(p);
                    }
                }
            }
        }
    }

#if 0
    using namespace GL::literals;
    if (1) {
        Function unix_s = wrap_function({}, [](GL::second x, GL::minute y) -> GL::hour { return x + y; });
        std::cout << unix_s.do_call<GL::hour>(1_s, 1_min) << std::endl;
    }
    if (1) {
        Function unix_s = wrap_function({}, [](GL::second x, GL::datetime y) -> GL::hour { return x + (GL::minute)y; });
        auto N = GL::datetime::Now();
        std::cout << unix_s.do_call<GL::hour>(0_s, N) << std::endl;
        std::cout << unix_s.do_call<GL::hour>(120_s, N) << std::endl;
        std::cout << unix_s.do_call<GL::hour>(12000_s, N) << std::endl;
        std::cout << unix_s.do_call<GL::hour>(120000000000_s, N) << std::endl;
    }
    if (1) {
        Function unix_s = wrap_function({}, [](GL::datetime y, GL::second x) -> GL::datetime { return y + x; });
        auto N = GL::datetime::Now();
        std::cout << unix_s.do_call<GL::datetime>(N, 0_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 120_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 1_d) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 12000_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 1200000_s) << std::endl;
        std::cout << unix_s.do_call<GL::datetime>(N, 120000000_s) << std::endl;
    }

    while (1) {
        switch ((int)std::floor(GL::util::rand(0, 15.999))) {
        case 0: {
            GL::string ref = "this";
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (unboxed value)\t")) {
                // for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)ref.begins_with("this");
                    });
            }
        } break;
        case 1: {
            GL::string ref = "this";
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (unboxed ref)\t")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    GL::string& Ref = ref;
                    (void)Ref.begins_with("this");
                    });
            }
        } break;
        case 2: {
            auto boxed = any::instance(GL::make_shared<GL::string>("this"));
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed GL::shared_ptr)\t")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)boxed.cast<GL::string&>().begins_with("this");
                    });
            }
        } break;
        case 3: {
            auto boxed = any::instance(GL::string("this"));
            if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed value)\t")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)boxed.cast<GL::string&>().begins_with("this");
                    });
            }
        } break;
        case 4: if (0) {
            //auto boxed = any::instance(GL::string("this"));
            //if (auto timer = GL::stopwatch().debug_timer("direct function call w/o converters (from boxed cast)\t")) {
            //    for (int i = 0; i < 1'000'000; ++i) {
            //        GL::string& Ref = boxed.cast();
            //        (void)Ref.begins_with("this");
            //    }
            //}
        } break;
        case 5: {
            auto callable = GL::make_callable("begins_with", &GL::string::begins_with);
            std::array<GL::any::fast_any, 2> example{
                GL::any::fast_any::instance(GL::string("this")),
                GL::any::fast_any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with callable and w/o converters, no conversion needed, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable->operator()(&example[0], &example[0] + example.size(), false);
                    });
            }
        } break;
        case 6: {
            auto callable{ Const_Member_Function_Caller(&GL::string::begins_with) };

            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with templatized callable and w/o converters, no conversion needed, w/ return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.call(&example[0]);
                    });
            }
        } break;
        case 7: {
            Function callable{ wrap_function({}, &GL::string::begins_with) };
            //std::cout << callable.to_string() << std::endl;
            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("\"this\".begins_with(\"this\") with generalized callable and w/o converters, no conversion needed, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.do_call(example[0], example[1]);
                    });
            }
        } break;
        case 9: {
            Function callable{ wrap_function({},[](GL::string const& LHS, GL::string const& RHS) -> auto { return LHS.begins_with(RHS); }) };
            //std::cout << callable.to_string() << std::endl;
            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with non-capturing lambda function and w/o converters, no conversion needed, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.do_call(example[0], example[1]);
                    });
            }
        } break;
        case 10: {
            Function callable{ wrap_function({}, [](GL::string const& LHS, GL::string const& RHS) -> bool { return LHS.begins_with(RHS); }) };
            //std::cout << callable.to_string() << std::endl;
            std::array<any, 2> example{
                any::instance(GL::string("this")),
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with do-call w/ return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    // callable.do_call<bool>(any(example[0], true), any(example[1], true));
                    callable.do_call<bool>(example[0], example[1]);
                    });

            }
        } break;
        case 11: {
            struct TEST {
                GL::string obj1;
                GL::string obj2;
            };
            std::array<any, 1> example{
                any::instance(TEST{ GL::string("this"), GL::string("that") })
            };
            Function callable{ wrap_function({}, &TEST::obj2) };
            //std::cout << callable.to_string() << std::endl;
            if (auto timer = GL::stopwatch().debug_timer("TEST::obj, w/o return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    (void)callable.do_call(any(example[0], true));
                    });
            }
        } break;
        case 12: {
            struct TEST {
                GL::string obj1;
                GL::string obj2;
            };
            std::array<any, 1> example{
                any::instance(TEST{ GL::string("this"), GL::string("that") })
            };
            Function callable{ wrap_function({}, &TEST::obj2) };
            //std::cout << callable.to_string() << std::endl;
            if (auto timer = GL::stopwatch().debug_timer("TEST::obj, w/ return")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() { any out;
                (void)callable.do_call<GL::string&>(any(example[0], true));
                // if (callable.do_call<GL::string&>(any(example[0], true)) != GL::string("that")) throw "ERROR";
                    });
            }
        } break;
        case 13: {
            Function callable{ wrap_function({}, [](GL::string const& LHS, GL::string const& RHS) -> bool { return LHS.begins_with(RHS); }) };
            callable.name = "begins_with";
            callable.defaults[1] = any::instance(GL::string("this"));
            // std::cout << callable.to_string() << std::endl;
            std::array<any, 1> example{
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with do-call w/o return w/ default value")) {
                // for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    callable.do_call(any(example[0], true));
                    });
            }
        } break;
        case 14: {
            GL::script_type Custom_Type("Custom");
            Function callable{ wrap_function({}, [](any const& LHS, any const& RHS) -> bool {
                return LHS.cast<GL::string const&>().begins_with(RHS.cast<GL::string const&>());
            }) };
            callable.name = "begins_with";
            callable.arguments[0] = Custom_Type.load();
            callable.arguments[1] = Custom_Type.load();
            callable.defaults[1] = any::instance(GL::string("this"));

            // std::cout << callable.to_string() << std::endl;
            std::array<any, 1> example{
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("Custom::begins_with(LHS, RHS) with do-call w/o return w/ default value")) {
                //for (int i = 0; i < 1'000'000; ++i) {
                GL::parallel::For(0, 1'000'000, [&]() {
                    callable.do_call(any(example[0], true));
                    });
            }
        } break;
        case 15: {
            Function callable{ []() {
                Function out { wrap_function({}, [](GL::string const& LHS, GL::string const& RHS) -> bool { return LHS.begins_with(RHS); }) };
                out.name = "begins_with";
                out.defaults[1] = any::instance(GL::string("this"));
                return out;
            }() };
            std::array<any, 1> example{
                any::instance(GL::string("this"))
            };
            if (auto timer = GL::stopwatch().debug_timer("GL::string::begins_with() with async_call w/o return w/ default value")) {
                std::vector<GL::parallel::promise> jobs(1'000'000ull, GL::parallel::promise{});
                GL::parallel::For(0, 1'000'000, [&](size_t i) {
                    jobs[i] = callable.async_call(example[0]);
                    });
            }
        } break;
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
            auto task_sequence = GL::parallel::async([]() -> GL::millisecond {
                std::cout << "First...\n";
                return GL::millisecond(0);
            }).and_then([](GL::second t0) -> GL::millisecond {
                std::cout << "Second...\n";
                return t0;
            }).and_then(0, 10'000, [](size_t i, GL::millisecond& t0, GL::parallel::job_base& parent) -> GL::any {
                t0 += 1;
                return parent.result;
            }).and_then([](GL::any const& t0) {
                if (t0.cast<GL::millisecond>() != 10'000_ms) throw "SHOULD HAVE MATCHED";
                std::cout << "Third and done.\n";
            });

            GL::value progress = 0;
            auto task_1 = GL::parallel::async(0, 10'000, [&progress]() {
                if ((++progress).mod(1000) == 0) {
                    std::cout << GL::printf("%i percent\n", (int)(100.0f * ((float)progress / 10'000.0f)));
                }
                GL::stopwatch sw; sw.reset();
                while (sw.check() < 0.001) {}
            });

            // casting is automatic for this
            GL::parallel::For(0, 1'000'000, [](size_t i) {});
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

    return 0;
};

