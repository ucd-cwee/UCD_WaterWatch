#include "util.h"
#include "atomic_vector.h"
#include "ticket_dispensor.h"
#include "Stopwatch.h"
#include <chrono>
#include <memory>
#include <thread>
#include <random>
#include <execution>

namespace GL {
    namespace util {
        long& __thread_alive(size_t thread_id) {
            static GL::atomic_vector<long> thread_ids{};
            static std::atomic<size_t> thread_count{ 0 };
            if (thread_count.load(std::memory_order_relaxed) <= thread_id) {
                thread_ids.grow_to_at_least(thread_id + 1);
                thread_count.exchange(thread_id, std::memory_order_relaxed);
            }
            return thread_ids[thread_id];
        };
        bool get_thread_alive(size_t thread_id) {
            return static_cast<bool>(__thread_alive(thread_id));
        };
        size_t get_thread_id() {
            static ticket_dispensor tickets;
            thread_local auto ticket{ tickets.get_ticket() };
            thread_local auto scoped_alive{
                // increments the "alive" during construction, and decrements during destruction.
                std::shared_ptr<size_t>(reinterpret_cast<size_t*>(ticket),[incrementOnce = InterlockedIncrement(reinterpret_cast<volatile long*>(&__thread_alive(ticket)))](size_t* p) -> void {
                    if (p && (incrementOnce > 0)) {
                        InterlockedDecrement(reinterpret_cast<volatile long*>(&__thread_alive(reinterpret_cast<size_t&>(p))));
                        tickets.return_ticket(reinterpret_cast<size_t&>(p));
                    }
                })
            };
            return ticket;
        };
        long long get_current_epoch() {
            return clock::ms();
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
            return ::__std_parallel_algorithms_hw_threads();
            // return std::thread::hardware_concurrency();
        }


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
        // 0..1
        double rand() {
            return rand_impl().Random(0.0, 1.0);
        };
        // 0..max or max..0
        double rand(double max) {
            if (max >= 0) {
                return rand_impl().Random(0.0, max);
            }
            else {
                return rand_impl().Random(max, 0.0);
            }
        };
        // min..max or max..min
        double rand(double min, double max) {
            if (max >= min) {
                return rand_impl().Random(min, max);
            }
            else {
                return rand_impl().Random(max, min);
            }
        };

        // 0..1
        double rand_fast() {
            return rand_impl().FastRandom(0.0, 1.0);
        };
        // 0..max or max..0
        double rand_fast(double max) {
            if (max >= 0) {
                return rand_impl().FastRandom(0.0, max);
            }
            else {
                return rand_impl().FastRandom(max, 0.0);
            }
        };
        // min..max or max..min
        double rand_fast(double min, double max) {
            if (max >= min) {
                return rand_impl().FastRandom(min, max);
            }
            else {
                return rand_impl().FastRandom(max, min);
            }
        };

    };
};


