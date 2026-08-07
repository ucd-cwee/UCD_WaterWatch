#pragma once
#pragma hdrstop
#include <chrono>
#include <memory>
#include <thread>
#include <random>
#include <execution>
#include "util.h"
#include "atomic_vector.h"
#include "ticket_dispensor.h"
#include "stopwatch.h"
#include <map>
#include "atomic_stack.h"

namespace GL {
    template <void (*Func)(void), int MicrosecondWait = 50> class Taskable {
    protected:
        std::atomic<bool>
            alive;
        std::condition_variable
            wakeCondition;
        std::mutex
            wakeMutex;
        std::thread
            thread;
        std::wstring
            name;
    public:
        Taskable(std::wstring Name = L"unspecified")
            : alive{ 0 }, wakeMutex{}, wakeCondition{}, name{Name}
        {
            thread = std::thread{ [this] {
                while (!this->alive.load()) {};

#ifdef _WIN32
                // Do Windows-specific thread setup:
                HANDLE handle = (HANDLE)this->thread.native_handle();

                // Name the thread:
                std::wstring wthreadname = L"Taskable_" + name;
                HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
#endif

                // pre-warm this thread's heap
                for (int i = 0; i < 100000; i++) delete (new int(i));

                while (this->alive.load()) {
                    // Work until no more jobs are found
                    Func();

                    // go to sleep, to be awoken when new jobs are added
                    auto lock{ std::unique_lock(this->wakeMutex) };
                    // this->wakeCondition.wait(lock);
                    this->wakeCondition.wait_for(lock, std::chrono::microseconds(MicrosecondWait)); // std::chrono::microseconds(500)
                }
            } };

            alive.exchange(true);
        }
        Taskable(Taskable const&) = delete;
        Taskable(Taskable&&) = delete;
        Taskable& operator=(Taskable const&) = delete;
        Taskable& operator=(Taskable&&) = delete;
        ~Taskable() {
            if (alive) {
                alive = false; // indicate that new jobs cannot be started from this point                
                wakeCondition.notify_all();
                thread.join();
            }
        }

        void wake() { // try to wake-up the thread if it is sleeping. 
            wakeCondition.notify_one();
        };
    };
    // does not allocate any new memory. Utilizes pointers that are provided to hold pointers to next position in stack. 
    class atomic_parallel_void_stack {
        struct element_t {
            element_t*
                m_pNext;
        };
        struct container {
            aba_problem::THead<element_t>
                head_1;
            aba_problem::THead<element_t>
                head_2;
            char
                which;
        };

        thread_object_no_malloc<container>
            head;
    public:
        atomic_parallel_void_stack() {
            const_cast<container&>(head._default) = { aba_problem::THead<element_t>{0}, aba_problem::THead<element_t>{0}, 0 };
            head._before_destruction = [](container& head) {
                bool Continue = true;
                while (Continue) {
                    Continue = false;
                    while (element_t* ptr = aba_problem::Pop(head.head_1)) {
                        ::_aligned_free(ptr);
                        Continue = true;
                    }
                    while (element_t* ptr = aba_problem::Pop(head.head_2)) {
                        ::_aligned_free(ptr);
                        Continue = true;
                    }
                }
            };
        };
        ~atomic_parallel_void_stack() {
            bool Continue = true;
            while (Continue) {
                Continue = false;
                head.for_each([&Continue](container& this_head) {
                    while (element_t* ptr = aba_problem::Pop(this_head.head_1)) {
                        ::_aligned_free(ptr);
                        Continue = true;
                    }
                    while (element_t* ptr = aba_problem::Pop(this_head.head_2)) {
                        ::_aligned_free(ptr);
                        Continue = true;
                    }
                });
            }
        };
        void push(void* obj) {
            element_t* new_ptr = reinterpret_cast<element_t*>(obj);
            new_ptr->m_pNext = nullptr;

            auto& thisHead = *head;
            if (thisHead.which == 0)
                aba_problem::Stack_Push(thisHead.head_1, new_ptr);
            else
                aba_problem::Stack_Push(thisHead.head_2, new_ptr);
        };
        void* try_pop() {
            element_t* ptr{ nullptr };
            container& this_head = *head;
            if (this_head.which == 0) {
                if (ptr = GL::aba_problem::Pop(this_head.head_1)) return ptr;
                if (ptr = GL::aba_problem::Pop(this_head.head_2)) return ptr;
            }
            else {
                if (ptr = GL::aba_problem::Pop(this_head.head_2)) return ptr;
                if (ptr = GL::aba_problem::Pop(this_head.head_1)) return ptr;
            }
            return ptr;
        };
        void free_all() {
            head.for_each([](container& this_head) {
                element_t* ptr;
                if (InterlockedExchangeNoFence8(reinterpret_cast<volatile char*>(&this_head.which), !this_head.which) == 0)
                    while (ptr = aba_problem::Pop(this_head.head_1)) ::_aligned_free(ptr);
                else
                    while (ptr = aba_problem::Pop(this_head.head_2)) ::_aligned_free(ptr);
            });
        };
        void free_fast() {
            container& this_head = *head;
            bool Continue = true;
            element_t* ptr;
            if (InterlockedExchangeNoFence8(reinterpret_cast<volatile char*>(&this_head.which), !this_head.which) == 0)
                while (Continue) {
                    Continue = false;
                    if (ptr = aba_problem::Pop(this_head.head_1)) {
                        ::_aligned_free(ptr);
                        Continue = true;
                    }
                }
            else
                while (Continue) {
                    Continue = false;
                    if (ptr = aba_problem::Pop(this_head.head_2)) {
                        ::_aligned_free(ptr);
                        Continue = true;
                    }
                }
        };
        void free_some() {
            head.for_each([](container& this_head) {
                element_t* ptr;
                // if (this_head.which = !this_head.which)
                if (InterlockedExchangeNoFence8(reinterpret_cast<volatile char*>(&this_head.which), !this_head.which) == 0)
                    if (ptr = aba_problem::Pop(this_head.head_1)) ::_aligned_free(ptr);
                    else if (ptr = aba_problem::Pop(this_head.head_2)) ::_aligned_free(ptr);
                });
        };
    };

    // GL::atomic_parallel_void_stack
        // freed_pointers; // only works because allocations are guarranteed to be aligned to 16-bytes. 

    void* malloc(size_t bytes) {
        if (bytes > (std::hardware_destructive_interference_size / 2))
            return ::_aligned_malloc(bytes, std::hardware_destructive_interference_size);
        else 
            return ::_aligned_malloc(bytes, sizeof(void*));
    };
    void mfree(void* ptr) {
        // if (ptr) freed_pointers.push(ptr);
        if (ptr) ::_aligned_free(ptr);
    };

    namespace util {
        static GL::ticket_dispensor<false> parent_ticket_dispensor;
        class ticket_return {
        private:
        public:
            const size_t ticket;
        public:
            ticket_return() : ticket{ parent_ticket_dispensor.get_ticket() }  {}
            ~ticket_return() {
                parent_ticket_dispensor.return_ticket(ticket);
            };
        };

        //size_t get_thread_id() {
        //    thread_local ticket_return ticket;
        //    thread_local size_t out{ ticket.ticket };
        //    return out;
        //};
        thread_local ticket_return ticket;
        size_t get_thread_id() {
            thread_local size_t out = 0ull;
            // out += (out == 0ull) * ticket.ticket;
            if (!out) out = ticket.ticket;
            return out;
        };

//        long long get_current_epoch() {
//#if 1
//            static std::atomic<long long> _epoch{ clock::ms() };
//            struct Wrap {
//                __declspec(noinline) static void UpdateEpoch(void) {
//                    _epoch.store(clock::ms(), std::memory_order_relaxed);
//                };
//            };
//            static Taskable<Wrap::UpdateEpoch> _update_thread{};
//            return _epoch.load(std::memory_order_relaxed);
//#else
//            return clock::ms();
//#endif
//        };

        size_t get_actual_unique_thread_id() {
            thread_local size_t out{ GL::util::inline_hash(GL::util::get_current_epoch(), std::this_thread::get_id()) };
            return out;
        };

        /*
         * GetOptimalCoreNumber() - Return the concurrency level on hardware
         *
         * This function returns the hardware concurrency level which usually
         * means independent sets of execution contexts. On C++11 this is the
         * return value of std::thread::hardware_concurrency, and its return value
         * counts hyper-threaded core as two different execution units.
         *
         * Nevertheless, the return value of this function is considered as the
         * optimal way of constructing an EM
         *
         * Please note that the EM is declared as a template which must take compile
         * time constant as its instanciation value. This function is only used for
         * debugging, and should not be used to derive the template argument
         */
        size_t get_hardware_thread_count() {
            return std::thread::hardware_concurrency();
        }

#if 1
        static auto& rand_impl() {
            class impl_rand {
            public:
                class fast_rand {
                public:
                    fast_rand() : m_state() {};
                    using result_type = uint32_t;
                    static constexpr result_type(min)() { return 0; }
                    static constexpr result_type(max)() { return UINT32_MAX; }

                    __declspec(noinline) result_type operator()() noexcept {
                        return xorshift32(&m_state);
                        // return xorshift128(&m_state);
                    };
                    void discard(unsigned long long n) noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };

                private:
                    class xorshift32_state {
                    public:
                        xorshift32_state() {
                            impl_pcg rand;
                            rand();
                            a = rand();
                        };
                        xorshift32_state(xorshift32_state const&) = delete;
                        xorshift32_state(xorshift32_state&&) = delete;
                        xorshift32_state& operator=(xorshift32_state const&) = delete;
                        xorshift32_state& operator=(xorshift32_state&&) = delete;
                        ~xorshift32_state() = default;

                        std::atomic<uint32_t> a;
                    }; /* The state must be initialized to non-zero */
                    __declspec(noinline) static uint32_t xorshift32(xorshift32_state* state) {
                        /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
                        uint32_t x = state->a;
                        x ^= x << 13;
                        x ^= x >> 17;
                        x ^= x << 5;
                        return state->a = x;
                    };

                    class xorshift64_state {
                    public:
                        xorshift64_state() {
                            impl_pcg rand;
                            rand();
                            a = rand();
                        };
                        xorshift64_state(xorshift64_state const&) = delete;
                        xorshift64_state(xorshift64_state&&) = delete;
                        xorshift64_state& operator=(xorshift64_state const&) = delete;
                        xorshift64_state& operator=(xorshift64_state&&) = delete;
                        ~xorshift64_state() = default;

                        std::atomic < uint64_t> a;
                    };
                    static uint64_t xorshift64(xorshift64_state* state) {
                        uint64_t x = state->a;
                        x ^= x << 13;
                        x ^= x >> 7;
                        x ^= x << 17;
                        return state->a = x;
                    };

                    class xorshift128_state {
                        /* struct xorshift128_state can alternatively be defined as a pair of uint64_t or a uint128_t where supported */
                    public:
                        xorshift128_state() {
                            impl_pcg rand;
                            rand();

                            x[0] = rand();
                            x[1] = rand();
                            x[2] = rand();
                            x[3] = rand();
                        };
                        xorshift128_state(xorshift128_state const&) = delete;
                        xorshift128_state(xorshift128_state&&) = delete;
                        xorshift128_state& operator=(xorshift128_state const&) = delete;
                        xorshift128_state& operator=(xorshift128_state&&) = delete;
                        ~xorshift128_state() = default;

                        std::atomic<uint32_t> x[4];
                    }; /* The state must be initialized to non-zero */
                    static uint32_t xorshift128(xorshift128_state* state) {
                        /* Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs" */
                        uint32_t s = state->x[0];
                        uint32_t t = state->x[3].exchange(state->x[2].exchange(state->x[1].exchange(s)));
                        t ^= t << 11;
                        t ^= t >> 8;
                        return state->x[0] = t ^ s ^ (s >> 19);
                    };

                    xorshift32_state m_state;
                    // xorshift128_state m_state;
                };
                class impl_pcg {
                public:
                    using result_type = uint32_t;
                    static constexpr result_type(min)() { return 0; }
                    static constexpr result_type(max)() { return UINT32_MAX; }

                    impl_pcg() noexcept : m_state(0), m_inc(0), rd() { seed(); };
                    void seed() noexcept {
                        uint64_t s0 = uint64_t(rd()) << 31 | uint64_t(rd());
                        uint64_t s1 = uint64_t(rd()) << 31 | uint64_t(rd());
                        m_state = 0;
                        m_inc = (s1 << 1) | 1;
                        (void)operator()();
                        m_state += s0;
                        (void)operator()();
                    };
                    result_type operator()() const noexcept {
                        uint64_t oldstate = m_state.load();
                        m_state.store(oldstate * 6364136223846793005ULL + m_inc);
                        uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
                        int rot = oldstate >> 59u;
                        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
                    };
                    void discard(unsigned long long n) const noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };

                private:
                    mutable std::atomic_int64_t m_state;
                    uint64_t m_inc;
                    std::random_device rd;
                };

            private:
                mutable impl_pcg rand;
                mutable std::uniform_real_distribution<double> u;

                mutable std::unique_ptr<fast_rand> randFast;
                mutable std::uniform_real_distribution<double> u_fast;

            public:
                impl_rand() noexcept : rand(), u(0.0, 1.0), randFast(new fast_rand()), u_fast(0.0, 1.0) {
                    Random_Impl();
                };
                double Random(double t1 = 0.0, double t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
                __declspec(noinline) double FastRandom(double t1 = 0.0, double t2 = 1.0) const noexcept { return FastRandom_HighRes(t1, t2); };
                __declspec(noinline) double random_base() const noexcept {
                    return FastRandom_Impl();
                };
            private:
                double Random_Impl() const noexcept {
                    return u(rand);
                };
                __declspec(noinline) double FastRandom_Impl() const noexcept {
                    return u_fast.operator()(*randFast);
                };
                double Random_HighRes(double t1, double t2) const noexcept {
                    t2 -= t1;
                    t2 *= Random_Impl();
                    t1 += t2;
                    return t1;
                };
                __declspec(noinline) double FastRandom_HighRes(double t1, double t2) const noexcept {
                    t2 -= t1;
                    t2 *= FastRandom_Impl();
                    t1 += t2;
                    return t1;
                };
            };
            static impl_rand random_generator;
            return random_generator;
        };
#endif
        // 0..1
        double rand() {
            return rand_impl().Random(0.0, 1.0);
        };
        // 0..max or max..0
        double rand(double max) {
            if (max >= 0) return rand_impl().Random(0.0, max);
            else return rand_impl().Random(max, 0.0);
        };
        // min..max or max..min
        double rand(double min, double max) {
            if (max >= min) return rand_impl().Random(min, max);
            else return rand_impl().Random(max, min);
        };

        // 0..1
        double rand_fast() {
            return rand_impl().FastRandom(0.0, 1.0);
        };
        // 0..max or max..0
        double rand_fast(double max) {
            if (max >= 0) return rand_impl().FastRandom(0.0, max);
            else return rand_impl().FastRandom(max, 0.0);
        };
        // min..max or max..min
        double rand_fast(double min, double max) {
            if (max >= min) return rand_impl().FastRandom(min, max);
            else return rand_impl().FastRandom(max, min);
        };

        static long long _epoch = 0;
        /*thread_local*/ static double _global_random = rand_impl().random_base();
        struct Wrap {
            static void UpdateEpoch(void) {
                InterlockedExchangeNoFence64(reinterpret_cast<volatile long long*>(&_epoch), clock::ms());
                double new_rand = rand_impl().random_base();
                InterlockedExchangeNoFence64(reinterpret_cast<volatile long long*>(&_global_random), *reinterpret_cast<long long*>(&new_rand));
            };
            //static void PerformFree(void) {
            //    freed_pointers.free_all();
            //};
        };
        static Taskable<Wrap::UpdateEpoch, 50> _update_thread(L"TimeAndRandom");
        // static Taskable<Wrap::PerformFree, 1> _free_thread(L"MemoryFree");
        long long get_current_epoch() {
            return _epoch;
        };
        // 0..1
        double rand_very_fast() {
            return _global_random;// = (*reinterpret_cast<unsigned long long*>(&_global_random) ^ ((*reinterpret_cast<unsigned long long*>(&_global_random) + 10101010101010) >> 11)) * 0x1.0p-53;
        };
        // 0..max or max..0
        double rand_very_fast(double max) {
            return _global_random * max;// = (*reinterpret_cast<unsigned long long*>(&_global_random) ^ ((*reinterpret_cast<unsigned long long*>(&_global_random) + 10101010101010) >> 11)) * 0x1.0p-53 * max;
        };
        // min..max or max..min
        double rand_very_fast(double min, double max) {
            return min + ((max - min) * _global_random);
            // return _global_random;// = min + (*reinterpret_cast<unsigned long long*>(&_global_random) ^ ((*reinterpret_cast<unsigned long long*>(&_global_random) + 10101010101010) >> 11)) * 0x1.0p-53 * (max - min);
        };



        GL::type_hash_t type_hash_impl::get_next_ticket(size_t original_hash) {
            static GL::ticket_dispensor builtin_ticket_dispensor{};
            static std::map<size_t, GL::type_hash_t> builtin_tickets{};
            GL::type_hash_t& out = builtin_tickets[original_hash];
            if (out == 0) {
                InterlockedCompareExchangeNoFence(reinterpret_cast<volatile GL::type_hash_t*>(&out), (GL::type_hash_t)(builtin_ticket_dispensor.get_ticket() | (1 << 20)), 0);
            }
            return out;
        };
    };
};