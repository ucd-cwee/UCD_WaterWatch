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

namespace GL {
    namespace util {
        class ticket_return {
        private:
            GL::ticket_dispensor<false>& parent;
            size_t ticket;
        public:
            ticket_return(GL::ticket_dispensor<false>& p, size_t t) : parent{ p }, ticket{ t }  {}
            ~ticket_return() {
                parent.return_ticket(ticket);
            };
        };

        size_t get_thread_id() {
            static ticket_dispensor<false> tickets;
            thread_local auto ticket{ tickets.get_ticket() };
            thread_local ticket_return r(tickets, ticket);
            return ticket;
        };
#if 1
        template <void (*Func)(void)> class Taskable {
            std::atomic<bool>
                alive;
            std::condition_variable
                wakeCondition;
            std::mutex
                wakeMutex;
            std::thread
                thread;

        public:
            Taskable()
                : alive{ 1 }, wakeMutex{}, wakeCondition{}
            {
                thread = std::thread{ [this] {
                    // pre-warm this thread's heap
                    for (int i = 0; i < 100000; i++) delete (new int(i));

                    while (this->alive.load()) {
                        // Work until no more jobs are found
                        Func();

                        // go to sleep, to be awoken when new jobs are added
                        auto lock{ std::unique_lock(this->wakeMutex) };
                        // this->wakeCondition.wait(lock);
                        this->wakeCondition.wait_for(lock, std::chrono::microseconds(500));
                    }
                } };
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
#endif
        long long get_current_epoch() {
#if 1
            static std::atomic<long long> _epoch{ clock::ms() };
            struct Wrap {
                __declspec(noinline) static void UpdateEpoch(void) {
                    _epoch.store(clock::ms(), std::memory_order_relaxed);
                };
            };
            static Taskable<Wrap::UpdateEpoch> _update_thread{};
            return _epoch.load(std::memory_order_relaxed);
#else
            return clock::ms();
#endif
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

                    result_type operator()() noexcept {
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
                    static uint32_t xorshift32(xorshift32_state* state) {
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
                double FastRandom(double t1 = 0.0, double t2 = 1.0) const noexcept { return FastRandom_HighRes(t1, t2); };
            private:
                double Random_Impl() const noexcept {
                    return u(rand);
                };
                double FastRandom_Impl() const noexcept {
                    return u_fast.operator()(*randFast);
                };
                double Random_HighRes(double t1, double t2) const noexcept {
                    t2 -= t1;
                    t2 *= Random_Impl();
                    t1 += t2;
                    return t1;
                };
                double FastRandom_HighRes(double t1, double t2) const noexcept {
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

        size_t type_hash_impl::get_next_ticket(size_t original_hash) {
            static GL::ticket_dispensor builtin_ticket_dispensor{};
            static std::map<size_t, size_t> builtin_tickets{};
            size_t& out = builtin_tickets[original_hash];
            if (out == 0) {
                InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&out), builtin_ticket_dispensor.get_ticket() | (1 << 20), 0);
            }
            return out;
        };
    };
};