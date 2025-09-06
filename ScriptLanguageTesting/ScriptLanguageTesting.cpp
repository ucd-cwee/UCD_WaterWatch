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

#include "units.h"

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
        // prove that GL::shared_ptr supports custom deleter functions. Note that these are always called on a different thread than the pointer was made on... 
        GL::shared_ptr<int> temp_ptr(new int(100), [](int* p) { 
            EXPECT_EQ(*p, 100);
            delete p; 
        });

        if (1) {
            GL::value val;
            auto package = val.load();
            package.m_bits.val += 10;
            val.store(package);
            auto package2 = val.load();
            EXPECT_EQ(10, (int)package.m_bits.val);

            GL::value meter(GL::value::get_si_unit(1, 0, 0, 0, 0).get_impl_unit(1.0, "meter", "m"));
            EXPECT_EQ(false, meter.is_scaler());
            EXPECT_EQ(0, (int)meter.load().m_bits.val);
            EXPECT_EQ(meter.name(), "meter");
            EXPECT_EQ(meter.abbreviation(), "m");

            GL::value foot(GL::value::get_si_unit(1, 0, 0, 0, 0).get_impl_unit(381.0 / 1250.0, "foot", "ft"));
            EXPECT_EQ(false, foot.is_scaler());

            GL::value inch(GL::value::get_si_unit(1, 0, 0, 0, 0).get_impl_unit((1.0 / 12.0) * (381.0 / 1250.0), "inch", "in"));
            EXPECT_EQ(false, inch.is_scaler());

            GL::value square_meter(GL::value::get_si_unit(2, 0, 0, 0, 0).get_impl_unit(1.0, "square_meter", "sq_m"));
            EXPECT_EQ(false, square_meter.is_scaler());

            GL::value square_foot(GL::value::get_si_unit(2, 0, 0, 0, 0).get_impl_unit((381.0 / 1250.0) * (381.0 / 1250.0), "square_foot", "sq_ft"));
            EXPECT_EQ(false, square_foot.is_scaler());

            GL::value cubic_meter(GL::value::get_si_unit(3, 0, 0, 0, 0).get_impl_unit(1.0, "cubic_meter", "cu_m"));
            EXPECT_EQ(false, cubic_meter.is_scaler());

            GL::value scaler;
            EXPECT_EQ(0, (int)scaler.load().m_bits.val);
            EXPECT_EQ(0, scaler.load().m_bits.si_unit);
            EXPECT_EQ(0, scaler.load().m_bits.impl_unit);
            EXPECT_EQ(0, scaler.load().m_bits2.unit_hash);
            EXPECT_EQ(1, (int)scaler.ratio());
            EXPECT_EQ(true, scaler.is_scaler());
            EXPECT_EQ(scaler.name(), "scaler");

            GL::value scaler2(GL::value::get_si_unit(0, 0, 0, 0, 0).get_impl_unit(1, "scaler", ""));
            EXPECT_EQ(0, (int)scaler2.load().m_bits.val);
            EXPECT_EQ(0, scaler2.load().m_bits.si_unit);
            EXPECT_EQ(0, scaler2.load().m_bits.impl_unit);
            EXPECT_EQ(0, scaler2.load().m_bits2.unit_hash);
            EXPECT_EQ(1, (int)scaler2.ratio());
            EXPECT_EQ(true, scaler2.is_scaler());
            EXPECT_EQ(scaler2.name(), "scaler");

            meter += 10.0f;
            EXPECT_EQ(10, (int)meter.load().m_bits.val);
            foot += 1.0f;
            EXPECT_EQ(1, (int)foot.load().m_bits.val);
            inch += 12.0f;
            EXPECT_EQ(12, (int)inch.load().m_bits.val);
            foot += inch;
            EXPECT_EQ(2, (int)foot.load().m_bits.val);
            scaler += 100.0f;
            EXPECT_EQ(100, (int)scaler.load().m_bits.val);

            cubic_meter += 1;
            EXPECT_EQ(1, (int)cubic_meter.load().m_bits.val);

            cubic_meter += scaler;
            EXPECT_EQ(101, (int)cubic_meter.load().m_bits.val);

            try {
                cubic_meter += inch;
                EXPECT_EQ(true, false);
            } catch (...) {}


            auto manual_sq_m = meter * meter;
            print(manual_sq_m.name());
            print(manual_sq_m.abbreviation());

            auto manual_cu_m = manual_sq_m * meter;
            print(manual_cu_m.name());
            print(manual_cu_m.abbreviation());

            auto manual_sq_ft = foot * foot;
            print(manual_sq_ft.name());
            print(manual_sq_ft.abbreviation());

            auto manual_cu_ft = manual_sq_ft * foot;
            print(manual_cu_ft.name());
            print(manual_cu_ft.abbreviation());

            auto manual_sq_in = inch * inch;
            print(manual_sq_in.name());
            print(manual_sq_in.abbreviation());

            auto manual_scaler = manual_cu_ft / manual_cu_m;
            print(manual_scaler.name());
            print(manual_scaler.abbreviation());

            GL::value scaler3;
            scaler3 += 3.0f;
            auto manual_cu_m2 = meter.pow(scaler3);
            print(manual_cu_m2.name());
            print(manual_cu_m2.abbreviation());
            print(manual_cu_m2.load().m_bits.val);

            // SOMETHING IS WRONG! CHECK YOU POWER MATH
            auto manual_cu_ft2 = foot.pow(scaler3);
            print(manual_cu_ft2.name());
            print(manual_cu_ft2.abbreviation());
            print(manual_cu_ft2.load().m_bits.val);


            if (auto timer = sw.debug_timer(__LINE__)) {
                GL::value v(GL::value::get_si_unit(1, 0, 0, 0, 0).get_impl_unit(381.0 / 1250.0, "foot", "ft"));
                GL::parallel::For(0, 1000000, [&](size_t const& index) {
                    ++v;
                });
            }
            if (auto timer = sw.debug_timer(__LINE__)) {
                GL::atomic_double v{ 0 };
                GL::parallel::For(0, 1000000, [&](size_t const& index) {
                    ++v;
                });
            }
        }

        









        //GL::builtin_type::declare_cpp_derived<long, int>();
        //EXPECT_EQ(true, GL::builtin_type::get_builtin_type<int>().is_derived_from(GL::builtin_type::get_builtin_type<long>().base_hash));
        //EXPECT_EQ(true, GL::builtin_type::get_builtin_type<long>().is_base_of(GL::builtin_type::get_builtin_type<int>().base_hash));
        if (1) {
            GL::type ti = GL::type_of<std::string>();
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(false, ti.is_const());
            EXPECT_EQ(false, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            ti |= GL::type::Qualifiers::Const;
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_const());
            EXPECT_EQ(false, ti.is_base());
            EXPECT_EQ(false, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            print(ti.name());
        }
        if (1) {
            GL::type ti = GL::type_of<const std::string&>();
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(false, ti.is_base());
            EXPECT_EQ(true, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_const());
            EXPECT_EQ(true, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            print(ti.name());
        }
        if (1) {
            GL::type ti;
            EXPECT_EQ(true, ti.is_void());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(true, ti.is_cpp_type());
            print(ti.name());
        }
        if (1) {
            GL::type ti = GL::type_of<void>();
            EXPECT_EQ(true, ti.is_void());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(true, ti.is_cpp_type());
            print(ti.name());
        }

        if (1) {
            auto string_hash = GL::impl::checkout_scripted_type("string");
            GL::type ti(string_hash);
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(true, ti.is_base());
            EXPECT_EQ(false, ti.is_cpp_type());
            EXPECT_EQ(false, ti.is_const());
            EXPECT_EQ(false, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            print(ti.name());
            ti |= GL::type::Qualifiers::Const;
            ti |= GL::type::Qualifiers::Reference;
            EXPECT_EQ(false, ti.is_void());
            EXPECT_EQ(false, ti.is_base());
            EXPECT_EQ(false, ti.is_cpp_type());
            EXPECT_EQ(true, ti.is_const());
            EXPECT_EQ(true, ti.is_ref());
            EXPECT_EQ(false, ti.is_temp());
            print(ti.name());
            GL::impl::return_scripted_type(string_hash);
        }

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
            if (auto instanced = GL::shared_ptr<shared_data<std::string>>(new shared_data<std::string>(GL::make_shared<std::string>("TEST")))) {
                // EXPECT_EQ(instanced.get()->m_ptr, "TEST");
                std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                EXPECT_EQ(*p, "TEST");
            }
            if (auto instanced = GL::static_pointer_cast<any_data>(GL::shared_ptr<shared_data<std::string>>(new shared_data<std::string>(GL::make_shared<std::string>("TEST"))))) {
                std::string* p = static_cast<std::string*>(instanced.get()->m_data);
                EXPECT_EQ(*p, "TEST");
            }
            if (1) {
                GL::atomic_shared_ptr< any_data > atomic{ GL::static_pointer_cast<any_data>(GL::shared_ptr<shared_data<std::string>>(new shared_data<std::string>(GL::make_shared<std::string>("TEST")))) };
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
                if (auto timer = sw.debug_timer(__LINE__)) {
                    any wrap;                
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        wrap = std::to_string(index);
                    });
                }
                if (auto timer = sw.debug_timer(__LINE__)) {
                    any wrap = GL::string("TEST");
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        auto& ptr = wrap.cast<GL::string>();
                        EXPECT_EQ(ptr, "TEST");
                    });
                }
                if (auto timer = sw.debug_timer(__LINE__)) {
                    any wrap = GL::string("TEST");
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        auto ptr = wrap.cast<GL::shared_ptr<GL::string>>();
                        EXPECT_EQ(*ptr, "TEST");
                    });
                }
                if (auto timer = sw.debug_timer(__LINE__)) {
                    any wrap{ GL::string("TEST") };
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {
                        wrap = GL::string("TEST");
                        if (auto ptr = wrap.cast<GL::shared_ptr<GL::string>>()) {
                            EXPECT_EQ(*ptr, "TEST");
                        }
                    });
                }

                if (auto timer = sw.debug_timer(__LINE__)) {
                    var wrap(GL::make_shared<any>(GL::string("TEST")));
                    GL::parallel::For(0, 1000000, [&](size_t const& index) {                        
                        wrap = var(GL::make_shared<any>(GL::string("TEST")));
                        if (auto ptr = wrap.p_data.load_fast()) {
                            if (auto ptr2 = ptr->cast<GL::shared_ptr<GL::string>>()) {
                                EXPECT_EQ(*ptr2, "TEST");
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










#if 1
        for (size_t repeats = 10; repeats <= 1000000; repeats *= 10) {
            print(repeats);

            GL::atomic_double result = GL::parallel::Dispatch(repeats, GL::atomic_double{ 0 }, [](size_t pos, GL::atomic_double& D) {
                ++D;
            });
            EXPECT_EQ((size_t)result.load(), repeats);

            if (1) {
                std::vector<std::string> calcs(repeats, "");
                if (auto timer = sw.debug_timer("parallel::std single-threaded calculations")) {                    
                    GL::parallel::Std_For(0ull, repeats, [&](size_t const& index) {
                        calcs[index] = std::to_string(index);
                    });
                };
                if (auto timer = sw.debug_timer("parallel::manual single-threaded calculations")) {
                    GL::parallel::For(0ull, repeats, [&](size_t const& index) {
                        calcs[index] = std::to_string(index);
                    });
                };
                if (auto timer = sw.debug_timer("single-threaded single-threaded calculations")) {
                    size_t index = 0ull;
                    for (; index < repeats; ) {
                        calcs[index] = std::to_string(index);
                        ++index;
                    };
                };
            }

            //if (auto timer = sw.debug_timer("parallel::std alloc")) {
            //    GL::atomic_shared_ptr<size_t> ptr; 
            //    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
            //        ptr.store(GL::shared_ptr<size_t>(new size_t(i)));
            //        ptr = nullptr;
            //    });
            //}
            if (auto timer = sw.debug_timer("parallel::manual alloc")) {
                GL::atomic_shared_ptr<size_t> ptr;
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    ptr.store(GL::shared_ptr<size_t>(new size_t(i)));
                    ptr = nullptr;
                });
            }
            //if (auto timer = sw.debug_timer("parallel::std increment")) {
            //    std::atomic<size_t> D{ 0 };
            //    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
            //        ++D;
            //    });
            //}
            if (auto timer = sw.debug_timer("parallel::manual increment")) {
                std::atomic<size_t> D{ 0 };
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    ++D;
                });
            }
            //if (auto timer = sw.debug_timer("parallel::std map")) {
            //    concurrency::concurrent_unordered_map<size_t, size_t> map;
            //    GL::parallel::Std_For<size_t>(0, repeats, [&](size_t i) {
            //        map[i] = i;
            //    });
            //}
            if (auto timer = sw.debug_timer("parallel::manual map")) {
                concurrency::concurrent_unordered_map<size_t, size_t> map;
                GL::parallel::For<size_t>(0, repeats, [&](size_t i) {
                    map[i] = i;
                });
            }

            //if (auto timer = sw.debug_timer("parallel::std ForEach")) {
            //    std::vector<size_t*> vec(1000000, nullptr);
            //    GL::parallel::Std_ForEach(vec, [](size_t*& p) {
            //        p = reinterpret_cast<size_t*>(100);
            //    });
            //}
            if (1) {
                std::vector<size_t*> vec(repeats, nullptr);
                if (auto timer = sw.debug_timer("parallel::manual ForEach")) {
                    GL::parallel::For_Each(vec, [](size_t*& p) {
                        p = reinterpret_cast<size_t*>(100);
                    });
                }
            }


        }
#endif










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
