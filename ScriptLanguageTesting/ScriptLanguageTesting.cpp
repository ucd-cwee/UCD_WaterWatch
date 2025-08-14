#pragma region "Includes"
#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <thread>
#include "Utilities.h"
#include "Parallel.h"
#include "Stopwatch.h"
#include "../FiberTasks/Concurrent_Queue.h"
// #include "../GoodLang/Parallel.h"
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
#define EXPECT_EQ(a, b) if (a != b){ print(GL::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#define EXPECT_NE(a, b) if (a == b){ print(GL::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#pragma endregion

int main() {
    GL::stopwatch sw;
    while (true) {
#if 1
        // queue
        if (auto timer = sw.debug_timer(__LINE__)) {
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

        // atomic_stack
        if (auto timer = sw.debug_timer(__LINE__)) {
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

#if 0
        // LocalWriteEM
        if (auto timer = sw.debug_timer(__LINE__)) {
            // This is the type of the EM we declare
            using EM = LocalWriteEM<std::string>;
            EM em{ GetOptimalCoreNumber() };
            // em.SetGCInterval(1);
            em.StartGCThread();

            GL::parallel::For(0, 1000000, [&](int i) {
                em.Protect();
                auto* p = em.Alloc(std::to_string(i));                
                em.Free(p);

                // despite the Free call, the pointer should still be valid for several milliseconds afterwards
                EXPECT_EQ(true, p->length() > 0);
                p->operator=("");
                EXPECT_EQ(0, p->length());
            });
        }
#endif

        // thread_object
        if (auto timer = sw.debug_timer(__LINE__)) {
            if (1) {
                GL::thread_object<int> thread_local_object(100);
                GL::parallel::For(0, 1000000, [&](int i) {
                    EXPECT_EQ(100, *thread_local_object);
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    *thread_local_object = static_cast<int>(GL::get_thread_id()); // thread-local and therefore thread-safe to change its value
                });
            }
            if (1) {
                GL::thread_object<std::string> thread_local_object("TEST");
                GL::parallel::For(0, 1000000, [&thread_local_object](int) {
                    EXPECT_EQ("TEST", *thread_local_object);
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    *thread_local_object = "\"" + std::to_string(GL::get_thread_id()) + "\""; // thread-local and therefore thread-safe to change its value
                });
            }
            if (1) {
                GL::thread_object<GL::string> thread_local_object("TEST");
                GL::parallel::For(0, 1000000, [&thread_local_object](int) {
                    EXPECT_EQ("TEST", *thread_local_object);
                });
                GL::parallel::For(0, 1000000, [&](int i) {
                    *thread_local_object = "\"" + std::to_string(GL::get_thread_id()) + "\""; // thread-local and therefore thread-safe to change its value
                });
            }
        };

        // atomic_allocator
        if (auto timer = sw.debug_timer(__LINE__)) {
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

        // atomic_parallel_allocator
        if (auto timer = sw.debug_timer(__LINE__)) {
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

        // atomic_stack
        if (auto timer = sw.debug_timer(__LINE__)) {
            GL::atomic_stack<size_t> queue;

            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }

        // atomic_parallel_stack
        if (auto timer = sw.debug_timer(__LINE__)) {
            GL::atomic_parallel_stack<size_t> queue;

            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }

        // atomic_queue
        if (auto timer = sw.debug_timer(__LINE__)) {
            GL::atomic_queue<size_t> queue;

            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }

        // atomic_parallel_queue<size_t>
        if (auto timer = sw.debug_timer(__LINE__)) {
            GL::atomic_parallel_queue<size_t> queue;

            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }

        // atomic_parallel_queue<short>
        if (auto timer = sw.debug_timer(__LINE__)) {
            GL::atomic_parallel_queue<short> queue;

            GL::parallel::For(0, SHRT_MAX, [&](short i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For(0, SHRT_MAX, [&](short i) {
                queue.push(i);
            });
            GL::parallel::For(0, SHRT_MAX, [&](short i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }

        // atomic_parallel_queue<GL::string>
        if (auto timer = sw.debug_timer(__LINE__)) {
            GL::atomic_parallel_queue<GL::string> queue;

            GL::parallel::For(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For(0, 1000000, [&](size_t i) {
                GL::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }


#endif








    }
    return 0;
};
