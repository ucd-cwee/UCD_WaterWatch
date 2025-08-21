#pragma region "Includes"
#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <thread>

#include "util.h"
#include "atomic_allocator.h"
#include "atomic_vector.h"
#include "atomic_stack.h"
#include "atomic_queue.h"
#include "atomic_numbers.h"
#include "atomic_maps.h"
#include "stopwatch.h"
#include "strings.h"





#include "Parallel.h"

#include "../FiberTasks/Concurrent_Queue.h"


#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>

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
#if 0
        if (auto timer = sw.debug_timer(GL::string("queue"))) {
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
        if (auto timer = sw.debug_timer("atomic_stack")) {
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
        if (auto timer = sw.debug_timer("thread_object")) {
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
        if (auto timer = sw.debug_timer("atomic_allocator")) {
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
        if (auto timer = sw.debug_timer("atomic_parallel_allocator")) {
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
        if (auto timer = sw.debug_timer("atomic_epoch_allocator")) {
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
        if (auto timer = sw.debug_timer("GL::atomic_map 1")) {
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
        if (auto timer = sw.debug_timer("GL::atomic_map 2")) {
            GL::atomic_map<size_t, GL::atomic_double> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                (void)map.erase(i % 10);
                map[i % 10] = i;
            });
        }
        if (auto timer = sw.debug_timer("atomic_stack<size_t>")) {
            GL::atomic_stack<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_stack<size_t>")) {
            GL::atomic_parallel_stack<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_queue<size_t>")) {
            GL::atomic_queue<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_queue<size_t>")) {
            GL::atomic_parallel_queue<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push(i);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_queue<short>")) {
            GL::atomic_parallel_queue<short> queue;

            GL::parallel::For(0, 1000000, [&](short i) {
                queue.push(i);
                EXPECT_EQ(true, queue.try_pop(i));
            });
            GL::parallel::For(0, 1000000, [&](short i) {
                queue.push(i);
            });
            GL::parallel::For(0, 1000000, [&](short i) {
                EXPECT_EQ(true, queue.try_pop(i));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_stack<GL::string>")) {
            GL::atomic_parallel_stack<GL::string> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_queue<GL::string>")) {
            GL::atomic_parallel_queue<GL::string> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                GL::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("atomic_priority_queue<std::string>")) {         
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
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("atomic_parallel_priority_queue<std::string>")) {
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
                EXPECT_EQ(true, queue.try_pop(str));
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str = std::to_string(i);
                queue.push(str);
            });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                std::string str;
                EXPECT_EQ(true, queue.try_pop(str));
            });
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_vector<size_t>")) {
            concurrency::concurrent_vector<size_t> queue;
            queue.grow_to_at_least(1000000);
        }
        if (auto timer = sw.debug_timer("GL::atomic_vector<size_t>")) {
            GL::atomic_vector<size_t> queue;
            queue.grow_to_at_least(1000000);
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_vector<size_t>")) {
            concurrency::concurrent_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_vector<size_t>")) {
            GL::atomic_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
            });
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_vector<size_t>")) {
            concurrency::concurrent_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
                });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++queue[i];
                });
        }
        if (auto timer = sw.debug_timer("GL::atomic_vector<size_t>")) {
            GL::atomic_vector<size_t> queue;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                queue.push_back(i);
                });
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++queue[i];
                });
        }
        if (auto timer = sw.debug_timer("GL::atomic_map<size_t, size_t>")) {
            GL::atomic_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_hash_map<size_t, size_t>")) {
            GL::atomic_hash_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
        }
        if (auto timer = sw.debug_timer("concurrency::concurrent_unordered_map<size_t, size_t>")) {
            concurrency::concurrent_unordered_map<size_t, size_t> map;

            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                map[i] += i;
            });
        }
        if (auto timer = sw.debug_timer("GL::atomic_map<size_t, size_t> w/ erasure")) {
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
        if (auto timer = sw.debug_timer("GL::atomic_hash_map<size_t, size_t> w/ erasure")) {
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
#if 1
        if (auto timer = sw.debug_timer("GL::atomic_double")) {
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
        if (auto timer = sw.debug_timer("GL::atomic_float")) {
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
    }
    return 0;
};
