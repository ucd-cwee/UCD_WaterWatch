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
#include "atomic_shared_ptr.h"

#include "types.h"



#include "Parallel.h"
#include "shared_ptr.h"


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
    GL::parallel::For<size_t>(0, 1000, [](size_t) {});
    GL::stopwatch sw;
    while (true) {
        for (size_t repeats = 10; repeats <= 1000000; repeats *= 10) {
            print(repeats);
            if (auto timer = sw.debug_timer("parallel::std alloc")) {
                GL::atomic_shared_ptr<size_t> ptr; 
                GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
                    ptr.store(GL::shared_ptr<size_t>(new size_t(i)));
                    ptr = nullptr;
                });
            }
            if (auto timer = sw.debug_timer("parallel::manual alloc")) {
                GL::atomic_shared_ptr<size_t> ptr;
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    ptr.store(GL::shared_ptr<size_t>(new size_t(i)));
                    ptr = nullptr;
                });
            }
            if (auto timer = sw.debug_timer("parallel::std increment")) {
                std::atomic<size_t> D{ 0 };
                GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
                    ++D;
                });
            }
            if (auto timer = sw.debug_timer("parallel::manual increment")) {
                std::atomic<size_t> D{ 0 };
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    ++D;
                });
            }
            if (auto timer = sw.debug_timer("parallel::std map")) {
                concurrency::concurrent_unordered_map<size_t, size_t> map;
                GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
                    map[i] = i;
                });
            }
            if (auto timer = sw.debug_timer("parallel::manual map")) {
                concurrency::concurrent_unordered_map<size_t, size_t> map;
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    map[i] = i;
                });
            }
        }











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
        if (auto timer = sw.debug_timer("atomic_parallel_allocator ST")) {
            GL::atomic_parallel_allocator<std::string, 1024> alloc;
            if (1) {
                for (int i = 0; i < 1000000; ++i){
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
        if (auto timer = sw.debug_timer("atomic_parallel_allocator MT")) {
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
#if 0
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
#if 1
#if 0
        if (auto timer = sw.debug_timer("GL::impl::atomic_ptr")) {
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
        if (auto timer = sw.debug_timer("GL::impl::atomic_ptr")) {

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

        if (auto timer = sw.debug_timer("increment as individuals")) {
            GL::thread_object<size_t> counter{ 0 };
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++*counter;
            });
        }
        if (auto timer = sw.debug_timer("increment as atomic")) {
            std::atomic<size_t> counter{ 0 };
            GL::parallel::For<size_t>(0, 1000000, [&](size_t i) {
                ++counter;
            });
        }

        // under low contention, the GL::atomic_shared_ptr using fast_shared_ptr is ~40% faster than a locked shared_ptr, even keeping pace with accessing a shared pointer without copying it. 
        // under moderate contention, this is still true, up to about 50 reads per value change
        // under extremely heavy contention (around 10 reads for every value change), the GL::atomic_shared_ptr is significantly bloated and results in significant slow-downs.
        for (double ratio = 1000000.0; ratio >= 1.0; ratio /= 10) {
            print(ratio);

            if (auto timer = sw.debug_timer("std::shared_ptr<std::string> with std shared lock")) {
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
            if (auto timer = sw.debug_timer("std::shared_ptr<std::string> access with std shared lock")) {
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
            if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<std::string> slow test")) {
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
            if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<std::string> fast test")) {
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

        if (auto timer = sw.debug_timer("GL::atomic_shared_ptr<void>")) {
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
        if (auto timer = sw.debug_timer("std::shared_ptr<void>")) {
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

        auto& void_type = GL::type_of<void>();
        EXPECT_EQ(void_type.base_hash, 0);
        auto& int_type = GL::type_of<int>();
        EXPECT_EQ(int_type.get_name(), "int");
        auto& double_type = GL::type_of<double>();
        EXPECT_EQ(double_type.get_name(), "double");
        auto& float_type = GL::type_of<float>();
        EXPECT_EQ(float_type.get_name(), "float");
        auto& str_type = GL::type_of<GL::string>();
        EXPECT_EQ(str_type.get_name(), "class GL::string");

        (void)int_type.add_parent(GL::impl::base_type_ptr<float>());
        EXPECT_EQ(false, float_type.add_parent(GL::impl::base_type_ptr<int>()));
        EXPECT_EQ(true, float_type.is_parent_of(int_type));
        EXPECT_EQ(true, int_type.can_cast(float_type));
        EXPECT_EQ(false, float_type.can_cast(int_type));
        EXPECT_EQ(true, float_type.is_base());

        auto& const_float_type = GL::type_of<const float>();
        EXPECT_EQ(float_type.base_hash, const_float_type.base_hash);
        EXPECT_EQ(true, const_float_type.is_parent_of(int_type));
        EXPECT_EQ(true, int_type.can_cast(const_float_type));
        EXPECT_EQ(false, const_float_type.can_cast(int_type));
        EXPECT_EQ(true, const_float_type.is_const());

        auto& const_ref_float_type = GL::type_of<const float&>();
        EXPECT_EQ(float_type.base_hash, const_ref_float_type.base_hash);
        EXPECT_EQ(true, float_type.is_parent_of(const_ref_float_type));
        EXPECT_EQ(true, const_ref_float_type.is_parent_of(int_type));
        EXPECT_EQ(true, int_type.can_cast(const_ref_float_type));
        EXPECT_EQ(false, const_ref_float_type.can_cast(int_type));
        EXPECT_EQ(true, const_ref_float_type.is_const_ref());

        auto const_float_type_2 = float_type | GL::type::Const;
        EXPECT_EQ(float_type.base_hash, const_float_type_2.base_hash);
        EXPECT_EQ(true, const_float_type_2.is_parent_of(int_type));
        EXPECT_EQ(true, int_type.can_cast(const_float_type_2));
        EXPECT_EQ(false, const_float_type_2.can_cast(int_type));
        EXPECT_EQ(true, const_float_type_2.is_const());        

#endif
#if 0
        if (auto timer = sw.debug_timer("atomic_wait")) {
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






        if (1) {
            GL::atomic_shared_ptr<int> ptr;
            auto* p = ptr.load();



        }

#endif
    }
    return 0;
};
