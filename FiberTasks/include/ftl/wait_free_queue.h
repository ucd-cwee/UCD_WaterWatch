/**
 * FiberTaskingLib - A tasking library that uses fibers for efficient task switching
 *
 * This library was created as a proof of concept of the ideas presented by
 * Christian Gyrling in his 2015 GDC Talk 'Parallelizing the Naughty Dog Engine Using Fibers'
 *
 * http://gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine
 *
 * FiberTaskingLib is the legal property of Adrian Astley
 * Copyright Adrian Astley 2015 - 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * This is an implementation of 'Correct and Efficient Work-Stealing for Weak Memory Models' by Le et. al [2013]
 *
 * https://hal.inria.fr/hal-00802885
 */

#pragma once

#include <functional>

#include "ftl/assert.h"

#include <atomic>
#include <memory>
#include <stddef.h>
#include <stdint.h>
#include <vector>

#pragma region "atomic queue"
 /* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_DEFS_H_INCLUDED
#define ATOMIC_QUEUE_DEFS_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include <atomic>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <emmintrin.h>
namespace atomic_queue {
    constexpr int CACHE_LINE_SIZE = 64;
    static inline void spin_loop_pause() noexcept {
        _mm_pause();
    }
} // namespace atomic_queue
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM64)
namespace atomic_queue {
    constexpr int CACHE_LINE_SIZE = 64;
    static inline void spin_loop_pause() noexcept {
#if (defined(__ARM_ARCH_6K__) || \
     defined(__ARM_ARCH_6Z__) || \
     defined(__ARM_ARCH_6ZK__) || \
     defined(__ARM_ARCH_6T2__) || \
     defined(__ARM_ARCH_7__) || \
     defined(__ARM_ARCH_7A__) || \
     defined(__ARM_ARCH_7R__) || \
     defined(__ARM_ARCH_7M__) || \
     defined(__ARM_ARCH_7S__) || \
     defined(__ARM_ARCH_8A__) || \
     defined(__aarch64__))
        asm volatile ("yield" ::: "memory");
#elif defined(_M_ARM64)
        __yield();
#else
        asm volatile ("nop" ::: "memory");
#endif
    }
} // namespace atomic_queue
#elif defined(__ppc64__) || defined(__powerpc64__)
namespace atomic_queue {
    constexpr int CACHE_LINE_SIZE = 128; // TODO: Review that this is the correct value.
    static inline void spin_loop_pause() noexcept {
        asm volatile("or 31,31,31 # very low priority"); // TODO: Review and benchmark that this is the right instruction.
    }
} // namespace atomic_queue
#elif defined(__s390x__)
namespace atomic_queue {
    constexpr int CACHE_LINE_SIZE = 256; // TODO: Review that this is the correct value.
    static inline void spin_loop_pause() noexcept {} // TODO: Find the right instruction to use here, if any.
} // namespace atomic_queue
#elif defined(__riscv)
namespace atomic_queue {
    constexpr int CACHE_LINE_SIZE = 64;
    static inline void spin_loop_pause() noexcept {
        asm volatile (".insn i 0x0F, 0, x0, x0, 0x010");
    }
} // namespace atomic_queue
#else
#ifdef _MSC_VER
#pragma message("Unknown CPU architecture. Using L1 cache line size of 64 bytes and no spinloop pause instruction.")
#else
#warning "Unknown CPU architecture. Using L1 cache line size of 64 bytes and no spinloop pause instruction."
#endif
namespace atomic_queue {
    constexpr int CACHE_LINE_SIZE = 64; // TODO: Review that this is the correct value.
    static inline void spin_loop_pause() noexcept {}
} // namespace atomic_queue
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

#if defined(__GNUC__) || defined(__clang__)
#define ATOMIC_QUEUE_LIKELY(expr) __builtin_expect(static_cast<bool>(expr), 1)
#define ATOMIC_QUEUE_UNLIKELY(expr) __builtin_expect(static_cast<bool>(expr), 0)
#define ATOMIC_QUEUE_NOINLINE __attribute__((noinline))
#else
#define ATOMIC_QUEUE_LIKELY(expr) (expr)
#define ATOMIC_QUEUE_UNLIKELY(expr) (expr)
#define ATOMIC_QUEUE_NOINLINE
#endif

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto constexpr A = std::memory_order_acquire;
    auto constexpr R = std::memory_order_release;
    auto constexpr X = std::memory_order_relaxed;
    auto constexpr C = std::memory_order_seq_cst;
    auto constexpr AR = std::memory_order_acq_rel;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_DEFS_H_INCLUDED

/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED
#define ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include <atomic>
#include <cstdlib>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Spinlock {
        std::atomic<long> _s;

    public:
        using scoped_lock = std::lock_guard<Spinlock>;

        Spinlock() noexcept : _s(0) {}

        Spinlock(Spinlock const&) = delete;
        Spinlock& operator=(Spinlock const&) = delete;

        ~Spinlock() noexcept {}

        void lock() noexcept {
            while (_s.fetch_add(1) != 1) {
                _s.fetch_sub(1);
            }
        }

        void unlock() noexcept {
            _s.fetch_sub(1);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class TicketSpinlock {
        alignas(CACHE_LINE_SIZE) std::atomic<unsigned> ticket_{ 0 };
        alignas(CACHE_LINE_SIZE) std::atomic<unsigned> next_{ 0 };

    public:
        class LockGuard {
            TicketSpinlock* const m_;
            unsigned const ticket_;
        public:
            LockGuard(TicketSpinlock& m) noexcept
                : m_(&m)
                , ticket_(m.lock())
            {}

            LockGuard(LockGuard const&) = delete;
            LockGuard& operator=(LockGuard const&) = delete;

            ~LockGuard() noexcept {
                m_->unlock(ticket_);
            }
        };

        using scoped_lock = LockGuard;

        TicketSpinlock() noexcept = default;
        TicketSpinlock(TicketSpinlock const&) = delete;
        TicketSpinlock& operator=(TicketSpinlock const&) = delete;

        ATOMIC_QUEUE_NOINLINE unsigned lock() noexcept {
            auto ticket = ticket_.fetch_add(1, std::memory_order_relaxed);
            for (;;) {
                auto position = ticket - next_.load(std::memory_order_acquire);
                if (ATOMIC_QUEUE_LIKELY(!position))
                    break;
                do
                    spin_loop_pause();
                while (--position);
            }
            return ticket;
        }

        void unlock() noexcept {
            unlock(next_.load(std::memory_order_relaxed) + 1);
        }

        void unlock(unsigned ticket) noexcept {
            next_.store(ticket + 1, std::memory_order_release);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class UnfairSpinlock {
        std::atomic<unsigned> lock_{ 0 };

    public:
        using scoped_lock = std::lock_guard<UnfairSpinlock>;

        UnfairSpinlock(UnfairSpinlock const&) = delete;
        UnfairSpinlock& operator=(UnfairSpinlock const&) = delete;

        void lock() noexcept {
            for (;;) {
                if (!lock_.load(std::memory_order_relaxed) && !lock_.exchange(1, std::memory_order_acquire))
                    return;
                spin_loop_pause();
            }
        }

        void unlock() noexcept {
            lock_.store(0, std::memory_order_release);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // class SpinlockHle {
    //     int lock_ = 0;

    // #ifdef __gcc__
    //     static constexpr int HLE_ACQUIRE = __ATOMIC_HLE_ACQUIRE;
    //     static constexpr int HLE_RELEASE = __ATOMIC_HLE_RELEASE;
    // #else
    //     static constexpr int HLE_ACQUIRE = 0;
    //     static constexpr int HLE_RELEASE = 0;
    // #endif

    // public:
    //     using scoped_lock = std::lock_guard<Spinlock>;

    //     SpinlockHle(SpinlockHle const&) = delete;
    //     SpinlockHle& operator=(SpinlockHle const&) = delete;

    //     void lock() noexcept {
    //         for(int expected = 0;
    //             !__atomic_compare_exchange_n(&lock_, &expected, 1, false, __ATOMIC_ACQUIRE | HLE_ACQUIRE, __ATOMIC_RELAXED);
    //             expected = 0)
    //             spin_loop_pause();
    //     }

    //     void unlock() noexcept {
    //         __atomic_store_n(&lock_, 0, __ATOMIC_RELEASE | HLE_RELEASE);
    //     }
    // };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // class AdaptiveMutex {
    //     pthread_mutex_t m_;

    // public:
    //     using scoped_lock = std::lock_guard<AdaptiveMutex>;

    //     AdaptiveMutex() noexcept {
    //         pthread_mutexattr_t a;
    //         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutexattr_init(&a)))
    //             std::abort();
    //         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutexattr_settype(&a, PTHREAD_MUTEX_ADAPTIVE_NP)))
    //             std::abort();
    //         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_init(&m_, &a)))
    //             std::abort();
    //         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutexattr_destroy(&a)))
    //             std::abort();
    //         m_.__data.__spins = 32767;
    //     }

    //     AdaptiveMutex(AdaptiveMutex const&) = delete;
    //     AdaptiveMutex& operator=(AdaptiveMutex const&) = delete;

    //     ~AdaptiveMutex() noexcept {
    //         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_destroy(&m_)))
    //             std::abort();
    //     }

    //     void lock() noexcept {
    //         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_lock(&m_)))
    //             std::abort();
    //     }

    //     void unlock() noexcept {
    //         if(ATOMIC_QUEUE_UNLIKELY(::pthread_mutex_unlock(&m_)))
    //             std::abort();
    //     }
    // };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED

/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef BARRIER_H_INCLUDED
#define BARRIER_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class Barrier {
        std::atomic<unsigned> counter_ = {};

    public:
        void wait() noexcept {
            counter_.fetch_add(1, std::memory_order_acquire);
            while (counter_.load(std::memory_order_relaxed))
                spin_loop_pause();
        }

        void release(unsigned expected_counter) noexcept {
            while (expected_counter != counter_.load(std::memory_order_relaxed))
                spin_loop_pause();
            counter_.store(0, std::memory_order_release);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BARRIER_H_INCLUDED

/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_ATOMIC_QUEUE_H_INCLUDED
#define ATOMIC_QUEUE_ATOMIC_QUEUE_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

    using std::uint32_t;
    using std::uint64_t;
    using std::uint8_t;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    namespace details {

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        template<size_t elements_per_cache_line> struct GetCacheLineIndexBits { static int constexpr value = 0; };
        template<> struct GetCacheLineIndexBits<256> { static int constexpr value = 8; };
        template<> struct GetCacheLineIndexBits<128> { static int constexpr value = 7; };
        template<> struct GetCacheLineIndexBits< 64> { static int constexpr value = 6; };
        template<> struct GetCacheLineIndexBits< 32> { static int constexpr value = 5; };
        template<> struct GetCacheLineIndexBits< 16> { static int constexpr value = 4; };
        template<> struct GetCacheLineIndexBits<  8> { static int constexpr value = 3; };
        template<> struct GetCacheLineIndexBits<  4> { static int constexpr value = 2; };
        template<> struct GetCacheLineIndexBits<  2> { static int constexpr value = 1; };

        template<bool minimize_contention, unsigned array_size, size_t elements_per_cache_line>
        struct GetIndexShuffleBits {
            static int constexpr bits = GetCacheLineIndexBits<elements_per_cache_line>::value;
            static unsigned constexpr min_size = 1u << (bits * 2);
            static int constexpr value = array_size < min_size ? 0 : bits;
        };

        template<unsigned array_size, size_t elements_per_cache_line>
        struct GetIndexShuffleBits<false, array_size, elements_per_cache_line> {
            static int constexpr value = 0;
        };

        // Multiple writers/readers contend on the same cache line when storing/loading elements at
        // subsequent indexes, aka false sharing. For power of 2 ring buffer size it is possible to re-map
        // the index in such a way that each subsequent element resides on another cache line, which
        // minimizes contention. This is done by swapping the lowest order N bits (which are the index of
        // the element within the cache line) with the next N bits (which are the index of the cache line)
        // of the element index.
        template<int BITS>
        constexpr unsigned remap_index(unsigned index) noexcept {
            unsigned constexpr mix_mask{ (1u << BITS) - 1 };
            unsigned const mix{ (index ^ (index >> BITS)) & mix_mask };
            return index ^ mix ^ (mix << BITS);
        }

        template<>
        constexpr unsigned remap_index<0>(unsigned index) noexcept {
            return index;
        }

        template<int BITS, typename T>
        constexpr T& map(T* elements, unsigned index) noexcept {
            return elements[remap_index<BITS>(index)];
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Implement a "bit-twiddling hack" for finding the next power of 2 in either 32 bits or 64 bits
        // in C++11 compatible constexpr functions. The library no longer maintains C++11 compatibility.

        // "Runtime" version for 32 bits
        // --a;
        // a |= a >> 1;
        // a |= a >> 2;
        // a |= a >> 4;
        // a |= a >> 8;
        // a |= a >> 16;
        // ++a;

        template<typename T>
        constexpr T decrement(T x) noexcept {
            return x - 1;
        }

        template<typename T>
        constexpr T increment(T x) noexcept {
            return x + 1;
        }

        template<typename T>
        constexpr T or_equal(T x, unsigned u) noexcept {
            return x | x >> u;
        }

        template<typename T, class... Args>
        constexpr T or_equal(T x, unsigned u, Args... rest) noexcept {
            return or_equal(or_equal(x, u), rest...);
        }

        constexpr uint32_t round_up_to_power_of_2(uint32_t a) noexcept {
            return increment(or_equal(decrement(a), 1, 2, 4, 8, 16));
        }

        constexpr uint64_t round_up_to_power_of_2(uint64_t a) noexcept {
            return increment(or_equal(decrement(a), 1, 2, 4, 8, 16, 32));
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        template<typename T>
        constexpr T nil() noexcept {
#if __cpp_lib_atomic_is_always_lock_free // Better compile-time error message requires C++17.
            static_assert(std::atomic<T>::is_always_lock_free, "Queue element type T is not atomic. Use AtomicQueue2/AtomicQueueB2 for such element types.");
#endif
            return {};
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    } // namespace details

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Derived>
    class AtomicQueueCommon {
    protected:
        // Put these on different cache lines to avoid false sharing between readers and writers.
        alignas(CACHE_LINE_SIZE) std::atomic<unsigned> head_ = {};
        alignas(CACHE_LINE_SIZE) std::atomic<unsigned> tail_ = {};

        // The special member functions are not thread-safe.

        AtomicQueueCommon() noexcept = default;

        AtomicQueueCommon(AtomicQueueCommon const& b) noexcept
            : head_(b.head_.load(X))
            , tail_(b.tail_.load(X)) {}

        AtomicQueueCommon& operator=(AtomicQueueCommon const& b) noexcept {
            head_.store(b.head_.load(X), X);
            tail_.store(b.tail_.load(X), X);
            return *this;
        }

        void swap(AtomicQueueCommon& b) noexcept {
            unsigned h = head_.load(X);
            unsigned t = tail_.load(X);
            head_.store(b.head_.load(X), X);
            tail_.store(b.tail_.load(X), X);
            b.head_.store(h, X);
            b.tail_.store(t, X);
        }

        template<typename T, T NIL>
        static T do_pop_atomic(std::atomic<T>& q_element) noexcept {
            if (Derived::spsc_) {
                for (;;) {
                    T element = q_element.load(A);
                    if (ATOMIC_QUEUE_LIKELY(element != NIL)) {
                        q_element.store(NIL, X);
                        return element;
                    }
                    if (Derived::maximize_throughput_)
                        spin_loop_pause();
                }
            }
            else {
                for (;;) {
                    T element = q_element.exchange(NIL, A); // (2) The store to wait for.
                    if (ATOMIC_QUEUE_LIKELY(element != NIL))
                        return element;
                    // Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                    do
                        spin_loop_pause();
                    while (Derived::maximize_throughput_ && q_element.load(X) == NIL);
                }
            }
        }

        template<typename T, T NIL>
        static void do_push_atomic(T element, std::atomic<T>& q_element) noexcept {
            assert(element != NIL);
            if (Derived::spsc_) {
                while (ATOMIC_QUEUE_UNLIKELY(q_element.load(X) != NIL))
                    if (Derived::maximize_throughput_)
                        spin_loop_pause();
                q_element.store(element, R);
            }
            else {
                for (T expected = NIL; ATOMIC_QUEUE_UNLIKELY(!q_element.compare_exchange_weak(expected, element, R, X)); expected = NIL) {
                    do
                        spin_loop_pause(); // (1) Wait for store (2) to complete.
                    while (Derived::maximize_throughput_ && q_element.load(X) != NIL);
                }
            }
        }

        enum State : unsigned char { EMPTY, STORING, STORED, LOADING };

        template<typename T>
        static T do_pop_any(std::atomic<unsigned char>& state, T& q_element) noexcept {
            if (Derived::spsc_) {
                while (ATOMIC_QUEUE_UNLIKELY(state.load(A) != STORED))
                    if (Derived::maximize_throughput_)
                        spin_loop_pause();
                T element{ std::move(q_element) };
                state.store(EMPTY, R);
                return element;
            }
            else {
                for (;;) {
                    unsigned char expected = STORED;
                    if (ATOMIC_QUEUE_LIKELY(state.compare_exchange_weak(expected, LOADING, A, X))) {
                        T element{ std::move(q_element) };
                        state.store(EMPTY, R);
                        return element;
                    }
                    // Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                    do
                        spin_loop_pause();
                    while (Derived::maximize_throughput_ && state.load(X) != STORED);
                }
            }
        }

        template<typename U, typename T>
        static void do_push_any(U&& element, std::atomic<unsigned char>& state, T& q_element) noexcept {
            if (Derived::spsc_) {
                while (ATOMIC_QUEUE_UNLIKELY(state.load(A) != EMPTY))
                    if (Derived::maximize_throughput_)
                        spin_loop_pause();
                q_element = std::forward<U>(element);
                state.store(STORED, R);
            }
            else {
                for (;;) {
                    unsigned char expected = EMPTY;
                    if (ATOMIC_QUEUE_LIKELY(state.compare_exchange_weak(expected, STORING, A, X))) {
                        q_element = std::forward<U>(element);
                        state.store(STORED, R);
                        return;
                    }
                    // Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                    do
                        spin_loop_pause();
                    while (Derived::maximize_throughput_ && state.load(X) != EMPTY);
                }
            }
        }

    public:
        template<typename T>
        bool try_push(T&& element) noexcept {
            auto head = head_.load(X);
            if (Derived::spsc_) {
                if (static_cast<int>(head - tail_.load(X)) >= static_cast<int>(static_cast<Derived&>(*this).size_))
                    return false;
                head_.store(head + 1, X);
            }
            else {
                do {
                    if (static_cast<int>(head - tail_.load(X)) >= static_cast<int>(static_cast<Derived&>(*this).size_))
                        return false;
                } while (ATOMIC_QUEUE_UNLIKELY(!head_.compare_exchange_weak(head, head + 1, X, X))); // This loop is not FIFO.
            }

            static_cast<Derived&>(*this).do_push(std::forward<T>(element), head);
            return true;
        }

        template<typename T>
        bool try_pop(T& element) noexcept {
            auto tail = tail_.load(X);
            if (Derived::spsc_) {
                if (static_cast<int>(head_.load(X) - tail) <= 0)
                    return false;
                tail_.store(tail + 1, X);
            }
            else {
                do {
                    if (static_cast<int>(head_.load(X) - tail) <= 0)
                        return false;
                } while (ATOMIC_QUEUE_UNLIKELY(!tail_.compare_exchange_weak(tail, tail + 1, X, X))); // This loop is not FIFO.
            }

            element = static_cast<Derived&>(*this).do_pop(tail);
            return true;
        }

        template<typename T>
        void push(T&& element) noexcept {
            unsigned head;
            if (Derived::spsc_) {
                head = head_.load(X);
                head_.store(head + 1, X);
            }
            else {
                constexpr auto memory_order = Derived::total_order_ ? std::memory_order_seq_cst : std::memory_order_relaxed;
                head = head_.fetch_add(1, memory_order); // FIFO and total order on Intel regardless, as of 2019.
            }
            static_cast<Derived&>(*this).do_push(std::forward<T>(element), head);
        }

        auto pop() noexcept {
            unsigned tail;
            if (Derived::spsc_) {
                tail = tail_.load(X);
                tail_.store(tail + 1, X);
            }
            else {
                constexpr auto memory_order = Derived::total_order_ ? std::memory_order_seq_cst : std::memory_order_relaxed;
                tail = tail_.fetch_add(1, memory_order); // FIFO and total order on Intel regardless, as of 2019.
            }
            return static_cast<Derived&>(*this).do_pop(tail);
        }

        bool was_empty() const noexcept {
            return !was_size();
        }

        bool was_full() const noexcept {
            return was_size() >= static_cast<int>(static_cast<Derived const&>(*this).size_);
        }

        unsigned was_size() const noexcept {
            // tail_ can be greater than head_ because of consumers doing pop, rather that try_pop, when the queue is empty.
            return std::max(static_cast<int>(head_.load(X) - tail_.load(X)), 0);
        }

        unsigned capacity() const noexcept {
            return static_cast<Derived const&>(*this).size_;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T, unsigned SIZE, T NIL = details::nil<T>(), bool MINIMIZE_CONTENTION = true, bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
    class AtomicQueue : public AtomicQueueCommon<AtomicQueue<T, SIZE, NIL, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
        using Base = AtomicQueueCommon<AtomicQueue<T, SIZE, NIL, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
        friend Base;

        static constexpr unsigned size_ = MINIMIZE_CONTENTION ? details::round_up_to_power_of_2(SIZE) : SIZE;
        static constexpr int SHUFFLE_BITS = details::GetIndexShuffleBits<MINIMIZE_CONTENTION, size_, CACHE_LINE_SIZE / sizeof(std::atomic<T>)>::value;
        static constexpr bool total_order_ = TOTAL_ORDER;
        static constexpr bool spsc_ = SPSC;
        static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;

        alignas(CACHE_LINE_SIZE) std::atomic<T> elements_[size_] = {}; // Empty elements are NIL.

        T do_pop(unsigned tail) noexcept {
            std::atomic<T>& q_element = details::map<SHUFFLE_BITS>(elements_, tail % size_);
            return Base::template do_pop_atomic<T, NIL>(q_element);
        }

        void do_push(T element, unsigned head) noexcept {
            std::atomic<T>& q_element = details::map<SHUFFLE_BITS>(elements_, head % size_);
            Base::template do_push_atomic<T, NIL>(element, q_element);
        }

    public:
        using value_type = T;

        AtomicQueue() noexcept {
            assert(std::atomic<T>{NIL}.is_lock_free()); // Queue element type T is not atomic. Use AtomicQueue2/AtomicQueueB2 for such element types.
            if (details::nil<T>() != NIL)
                for (auto& element : elements_)
                    element.store(NIL, X);
        }

        AtomicQueue(AtomicQueue const&) = delete;
        AtomicQueue& operator=(AtomicQueue const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T, unsigned SIZE, bool MINIMIZE_CONTENTION = true, bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
    class AtomicQueue2 : public AtomicQueueCommon<AtomicQueue2<T, SIZE, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
        using Base = AtomicQueueCommon<AtomicQueue2<T, SIZE, MINIMIZE_CONTENTION, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
        using State = typename Base::State;
        friend Base;

        static constexpr unsigned size_ = MINIMIZE_CONTENTION ? details::round_up_to_power_of_2(SIZE) : SIZE;
        static constexpr int SHUFFLE_BITS = details::GetIndexShuffleBits<MINIMIZE_CONTENTION, size_, CACHE_LINE_SIZE / sizeof(State)>::value;
        static constexpr bool total_order_ = TOTAL_ORDER;
        static constexpr bool spsc_ = SPSC;
        static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;

        alignas(CACHE_LINE_SIZE) std::atomic<unsigned char> states_[size_] = {};
        alignas(CACHE_LINE_SIZE) T elements_[size_] = {};

        T do_pop(unsigned tail) noexcept {
            unsigned index = details::remap_index<SHUFFLE_BITS>(tail % size_);
            return Base::template do_pop_any(states_[index], elements_[index]);
        }

        template<typename U>
        void do_push(U&& element, unsigned head) noexcept {
            unsigned index = details::remap_index<SHUFFLE_BITS>(head % size_);
            Base::template do_push_any(std::forward<U>(element), states_[index], elements_[index]);
        }

    public:
        using value_type = T;

        AtomicQueue2() noexcept = default;
        AtomicQueue2(AtomicQueue2 const&) = delete;
        AtomicQueue2& operator=(AtomicQueue2 const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T, typename A = std::allocator<T>, T NIL = details::nil<T>(), bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
    class AtomicQueueB : private std::allocator_traits<A>::template rebind_alloc<std::atomic<T>>,
        public AtomicQueueCommon<AtomicQueueB<T, A, NIL, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
        using AllocatorElements = typename std::allocator_traits<A>::template rebind_alloc<std::atomic<T>>;
        using Base = AtomicQueueCommon<AtomicQueueB<T, A, NIL, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
        friend Base;

        static constexpr bool total_order_ = TOTAL_ORDER;
        static constexpr bool spsc_ = SPSC;
        static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;

        static constexpr auto ELEMENTS_PER_CACHE_LINE = CACHE_LINE_SIZE / sizeof(std::atomic<T>);
        static_assert(ELEMENTS_PER_CACHE_LINE, "Unexpected ELEMENTS_PER_CACHE_LINE.");

        static constexpr auto SHUFFLE_BITS = details::GetCacheLineIndexBits<ELEMENTS_PER_CACHE_LINE>::value;
        static_assert(SHUFFLE_BITS, "Unexpected SHUFFLE_BITS.");

        // AtomicQueueCommon members are stored into by readers and writers.
        // Allocate these immutable members on another cache line which never gets invalidated by stores.
        alignas(CACHE_LINE_SIZE) unsigned size_;
        std::atomic<T>* elements_;

        T do_pop(unsigned tail) noexcept {
            std::atomic<T>& q_element = details::map<SHUFFLE_BITS>(elements_, tail & (size_ - 1));
            return Base::template do_pop_atomic<T, NIL>(q_element);
        }

        void do_push(T element, unsigned head) noexcept {
            std::atomic<T>& q_element = details::map<SHUFFLE_BITS>(elements_, head & (size_ - 1));
            Base::template do_push_atomic<T, NIL>(element, q_element);
        }

    public:
        using value_type = T;
        using allocator_type = A;

        // The special member functions are not thread-safe.

        AtomicQueueB(unsigned size, A const& allocator = A{})
            : AllocatorElements(allocator)
            , size_(std::max(details::round_up_to_power_of_2(size), 1u << (SHUFFLE_BITS * 2)))
            , elements_(AllocatorElements::allocate(size_)) {
            assert(std::atomic<T>{NIL}.is_lock_free()); // Queue element type T is not atomic. Use AtomicQueue2/AtomicQueueB2 for such element types.
            for (auto p = elements_, q = elements_ + size_; p < q; ++p)
                p->store(NIL, X);
        }

        AtomicQueueB(AtomicQueueB&& b) noexcept
            : AllocatorElements(static_cast<AllocatorElements&&>(b)) // TODO: This must be noexcept, static_assert that.
            , Base(static_cast<Base&&>(b))
            , size_(std::exchange(b.size_, 0))
            , elements_(std::exchange(b.elements_, nullptr))
        {}

        AtomicQueueB& operator=(AtomicQueueB&& b) noexcept {
            b.swap(*this);
            return *this;
        }

        ~AtomicQueueB() noexcept {
            if (elements_)
                AllocatorElements::deallocate(elements_, size_); // TODO: This must be noexcept, static_assert that.
        }

        A get_allocator() const noexcept {
            return *this;
        }

        void swap(AtomicQueueB& b) noexcept {
            using std::swap;
            swap(static_cast<AllocatorElements&>(*this), static_cast<AllocatorElements&>(b));
            Base::swap(b);
            swap(size_, b.size_);
            swap(elements_, b.elements_);
        }

        friend void swap(AtomicQueueB& a, AtomicQueueB& b) noexcept {
            a.swap(b);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T, typename A = std::allocator<T>, bool MAXIMIZE_THROUGHPUT = true, bool TOTAL_ORDER = false, bool SPSC = false>
    class AtomicQueueB2 : private std::allocator_traits<A>::template rebind_alloc<unsigned char>,
        public AtomicQueueCommon<AtomicQueueB2<T, A, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>> {
        using StorageAllocator = typename std::allocator_traits<A>::template rebind_alloc<unsigned char>;
        using Base = AtomicQueueCommon<AtomicQueueB2<T, A, MAXIMIZE_THROUGHPUT, TOTAL_ORDER, SPSC>>;
        using State = typename Base::State;
        using AtomicState = std::atomic<unsigned char>;
        friend Base;

        static constexpr bool total_order_ = TOTAL_ORDER;
        static constexpr bool spsc_ = SPSC;
        static constexpr bool maximize_throughput_ = MAXIMIZE_THROUGHPUT;

        // AtomicQueueCommon members are stored into by readers and writers.
        // Allocate these immutable members on another cache line which never gets invalidated by stores.
        alignas(CACHE_LINE_SIZE) unsigned size_;
        AtomicState* states_;
        T* elements_;

        static constexpr auto STATES_PER_CACHE_LINE = CACHE_LINE_SIZE / sizeof(AtomicState);
        static_assert(STATES_PER_CACHE_LINE, "Unexpected STATES_PER_CACHE_LINE.");

        static constexpr auto SHUFFLE_BITS = details::GetCacheLineIndexBits<STATES_PER_CACHE_LINE>::value;
        static_assert(SHUFFLE_BITS, "Unexpected SHUFFLE_BITS.");

        T do_pop(unsigned tail) noexcept {
            unsigned index = details::remap_index<SHUFFLE_BITS>(tail & (size_ - 1));
            return Base::template do_pop_any(states_[index], elements_[index]);
        }

        template<typename U>
        void do_push(U&& element, unsigned head) noexcept {
            unsigned index = details::remap_index<SHUFFLE_BITS>(head & (size_ - 1));
            Base::template do_push_any(std::forward<U>(element), states_[index], elements_[index]);
        }

        template<typename U>
        U* allocate_() {
            U* p = reinterpret_cast<U*>(StorageAllocator::allocate(size_ * sizeof(U)));
            assert(reinterpret_cast<uintptr_t>(p) % alignof(U) == 0); // Allocated storage must be suitably aligned for U.
            return p;
        }

        template<typename U>
        void deallocate_(U* p) noexcept {
            StorageAllocator::deallocate(reinterpret_cast<unsigned char*>(p), size_ * sizeof(U)); // TODO: This must be noexcept, static_assert that.
        }

    public:
        using value_type = T;
        using allocator_type = A;

        // The special member functions are not thread-safe.

        AtomicQueueB2(unsigned size, A const& allocator = A{})
            : StorageAllocator(allocator)
            , size_(std::max(details::round_up_to_power_of_2(size), 1u << (SHUFFLE_BITS * 2)))
            , states_(allocate_<AtomicState>())
            , elements_(allocate_<T>()) {
            for (auto p = states_, q = states_ + size_; p < q; ++p)
                p->store(Base::EMPTY, X);
            A a = get_allocator();
            for (auto p = elements_, q = elements_ + size_; p < q; ++p)
                std::allocator_traits<A>::construct(a, p);
        }

        AtomicQueueB2(AtomicQueueB2&& b) noexcept
            : StorageAllocator(static_cast<StorageAllocator&&>(b)) // TODO: This must be noexcept, static_assert that.
            , Base(static_cast<Base&&>(b))
            , size_(std::exchange(b.size_, 0))
            , states_(std::exchange(b.states_, nullptr))
            , elements_(std::exchange(b.elements_, nullptr))
        {}

        AtomicQueueB2& operator=(AtomicQueueB2&& b) noexcept {
            b.swap(*this);
            return *this;
        }

        ~AtomicQueueB2() noexcept {
            if (elements_) {
                A a = get_allocator();
                for (auto p = elements_, q = elements_ + size_; p < q; ++p)
                    std::allocator_traits<A>::destroy(a, p);
                deallocate_(elements_);
                deallocate_(states_);
            }
        }

        A get_allocator() const noexcept {
            return *this;
        }

        void swap(AtomicQueueB2& b) noexcept {
            using std::swap;
            swap(static_cast<StorageAllocator&>(*this), static_cast<StorageAllocator&>(b));
            Base::swap(b);
            swap(size_, b.size_);
            swap(states_, b.states_);
            swap(elements_, b.elements_);
        }

        friend void swap(AtomicQueueB2& a, AtomicQueueB2& b) noexcept {
            a.swap(b);
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename Queue>
    struct RetryDecorator : Queue {
        using T = typename Queue::value_type;

        using Queue::Queue;

        void push(T element) noexcept {
            while (!this->try_push(element))
                spin_loop_pause();
        }

        T pop() noexcept {
            T element;
            while (!this->try_pop(element))
                spin_loop_pause();
            return element;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_ATOMIC_QUEUE_H_INCLUDED

/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef ATOMIC_QUEUE_ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED
#define ATOMIC_QUEUE_ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include <mutex>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename M>
    struct ScopedLockType {
        using type = typename M::scoped_lock;
    };

    template<>
    struct ScopedLockType<std::mutex> {
        using type = std::unique_lock<std::mutex>;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<typename T, typename Mutex, unsigned SIZE, bool MINIMIZE_CONTENTION>
    class AtomicQueueMutexT {
        static constexpr unsigned size_ = MINIMIZE_CONTENTION ? details::round_up_to_power_of_2(SIZE) : SIZE;

        Mutex mutex_;
        alignas(CACHE_LINE_SIZE) unsigned head_ = 0;
        alignas(CACHE_LINE_SIZE) unsigned tail_ = 0;
        alignas(CACHE_LINE_SIZE) T q_[size_] = {};

        static constexpr int SHUFFLE_BITS = details::GetIndexShuffleBits<MINIMIZE_CONTENTION, size_, CACHE_LINE_SIZE / sizeof(T)>::value;

        using ScopedLock = typename ScopedLockType<Mutex>::type;

    public:
        using value_type = T;

        template<class U>
        bool try_push(U&& element) noexcept {
            ScopedLock lock(mutex_);
            if (ATOMIC_QUEUE_LIKELY(head_ - tail_ < size_)) {
                q_[details::remap_index<SHUFFLE_BITS>(head_ % size_)] = std::forward<U>(element);
                ++head_;
                return true;
            }
            return false;
        }

        bool try_pop(T& element) noexcept {
            ScopedLock lock(mutex_);
            if (ATOMIC_QUEUE_LIKELY(head_ != tail_)) {
                element = std::move(q_[details::remap_index<SHUFFLE_BITS>(tail_ % size_)]);
                ++tail_;
                return true;
            }
            return false;
        }

        bool was_empty() const noexcept {
            return static_cast<int>(head_ - tail_) <= 0;
        }

        bool was_full() const noexcept {
            return static_cast<int>(head_ - tail_) >= static_cast<int>(size_);
        }
    };

    template<typename T, unsigned SIZE, typename Mutex, bool MINIMIZE_CONTENTION = true>
    using AtomicQueueMutex = AtomicQueueMutexT<T, Mutex, SIZE, MINIMIZE_CONTENTION>;

    template<typename T, unsigned SIZE, bool MINIMIZE_CONTENTION = true>
    using AtomicQueueSpinlock = AtomicQueueMutexT<T, Spinlock, SIZE, MINIMIZE_CONTENTION>;

    // template<typename T, unsigned SIZE, bool MINIMIZE_CONTENTION = true>
    // using AtomicQueueSpinlockHle = AtomicQueueMutexT<T, SpinlockHle, SIZE, MINIMIZE_CONTENTION>;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}; // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ATOMIC_QUEUE_ATOMIC_QUEUE_SPIN_LOCK_H_INCLUDED
#pragma endregion

#include <ppl.h>
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>

namespace ftl {
#if 0
    template <typename T> class queue_container {
    public:
        queue_container(long long n = 1000) : queue(static_cast<int>(n)), prev(nullptr) {};

    public:
        ::atomic_queue::AtomicQueueB2<T> queue;
        std::atomic< queue_container<T>* > prev;
    };
    template <typename T> class WaitFreeQueue {
    protected:
        std::atomic< queue_container<T>* > last;
        std::atomic<int> lock;

        void AddNewQueue(long long n = 1000) {
            auto* prevL1 = last.load();
            while ((lock.fetch_add(1)+1) != 1) lock.fetch_sub(1);
            {
                auto* prevL2 = last.load();
                if (prevL1 == prevL2) {
                    queue_container<T>* newPtr = new queue_container<T>(std::max(1000ll, n));
                    queue_container<T>* prevLast = last.exchange(newPtr);
                    newPtr->prev.store(prevLast);
                }
            }
            lock.fetch_sub(1);
        };

    public:
        WaitFreeQueue() : last(new queue_container<T>()), lock(0) {};
        WaitFreeQueue(WaitFreeQueue const&) = delete;
        WaitFreeQueue(WaitFreeQueue&&) noexcept = delete;
        WaitFreeQueue& operator=(WaitFreeQueue const&) = delete;
        WaitFreeQueue& operator=(WaitFreeQueue&&) noexcept = delete;
        ~WaitFreeQueue() {
            for (auto* queue = last.load(); queue; ) {
                auto* queue_copy = queue;
                queue = queue->prev.load();
                if (queue_copy) {
                    delete queue_copy;
                }
            }
        };

    public:
        template<typename T_o>
        void Push(T_o* values, long long n, std::function<T(T_o const&)> converter) {
            long long pos = 0;
            while (pos < n) {
                bool satisfied = false;
                while (!satisfied) {
                    for (auto* queue = last.load(); queue; queue = queue->prev.load()) {
                        if (queue->queue.try_push(converter(values[pos]))) {
                            satisfied = true;
                            break;
                        }
                    }
                    if (!satisfied) {
                        AddNewQueue(16ll + (n - pos));
                    }
                }
                ++pos;
            }
        };
        void Push(T const& value) {
            while (true) {
                for (auto* queue = last.load(); queue; queue = queue->prev.load()) {
                    if (queue->queue.try_push(value)) {
                        return;
                    }
                }
                AddNewQueue();
            }
        };
        bool Pop(T* value) {
            for (auto* queue = last.load(); queue; queue = queue->prev.load()) {
                if (queue->queue.try_pop(*value)) {
                    return true;
                }
            }     
            return false;
        };
        bool Steal(T* const value) {
            for (auto* queue = last.load(); queue; queue = queue->prev.load()) {
                if (queue->queue.try_pop(*value)) {
                    return true;
                }
            }
            return false;
        };

    };
#else
    template <typename T> class WaitFreeQueue {
    protected:
        concurrency::concurrent_queue<T> queue;

    public:
        WaitFreeQueue() = default;
        WaitFreeQueue(WaitFreeQueue const&) = delete;
        WaitFreeQueue(WaitFreeQueue&&) noexcept = delete;
        WaitFreeQueue& operator=(WaitFreeQueue const&) = delete;
        WaitFreeQueue& operator=(WaitFreeQueue&&) noexcept = delete;
        ~WaitFreeQueue() = default;

    public:
        template<typename T_o>
        void Push(T_o* values, long long n, std::function<T(T_o const&)> converter) {
            for (int i = 0; i < n; i++) {
                queue.push(converter(values[i]));
            }
        };
        void Push(T const& value) {
            queue.push(value);
        };
        bool Pop(T* value) {
            return queue.try_pop(*value);
        };
        bool Steal(T* const value) {
            return queue.try_pop(*value);
        };

    };
#endif

} // End of namespace ftl






















































