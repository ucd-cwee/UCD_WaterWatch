#pragma once

#include <memory>
#include <functional>
#include <condition_variable>
#include <mutex>
#include "aba_problem.h"
#include "atomic_maps.h"

namespace GL {    
    std::mutex* __atomic_mutexes();
    std::condition_variable* __atomic_conds();

    template<class T> static std::size_t index_for_address(T* ptr) noexcept {
        return reinterpret_cast<std::size_t>(ptr) / sizeof(T) % 64;
    };
    template<class T> static void wait_for_address(T* ptr, T const& value) {
        if (*ptr != value) return;
        std::size_t index = index_for_address(ptr);
        std::unique_lock<std::mutex> lock(__atomic_mutexes()[index]);
        while (ptr->load(std::memory_order_relaxed) == value)
            __atomic_conds()[index].wait(lock);
    };
    template<class T> static void notify_address(T* ptr) {
        const std::size_t index = index_for_atomic(ptr);
        /*
         * normally we don't need to hold the mutex to notify
         * but in this case we updated the value without holding
         * the lock. Therefore without the mutex there would be
         * a race condition in wait() between the while-loop condition
         * and the loop body
         */
        std::lock_guard<std::mutex> lock(__atomic_mutexes()[index]);
        /*
         * needs to notify_all because we could have multiple waiters
         * in multiple atomics due to aliasing
         */
        __atomic_conds()[index].notify_all();
    };

    template<class T> static std::size_t index_for_atomic(std::atomic<T>* ptr) noexcept {
        return reinterpret_cast<std::size_t>(ptr) / sizeof(T) % 64;
    };
    template<class T> static void atomic_wait(std::atomic<T>* ptr, T value, std::memory_order order = std::memory_order::memory_order_relaxed) {
        if (ptr->load(order) != value)
            return;
        std::size_t index = index_for_atomic(ptr);
        std::unique_lock<std::mutex> lock(__atomic_mutexes()[index]);
        while (ptr->load(std::memory_order_relaxed) == value)
            __atomic_conds()[index].wait(lock);
    };
    template<class T> static void atomic_notify_one(std::atomic<T>* ptr) {
        const std::size_t index = index_for_atomic(ptr);
        /*
         * normally we don't need to hold the mutex to notify
         * but in this case we updated the value without holding
         * the lock. Therefore without the mutex there would be
         * a race condition in wait() between the while-loop condition
         * and the loop body
         */
        std::lock_guard<std::mutex> lock(__atomic_mutexes()[index]);
        /*
         * needs to notify_all because we could have multiple waiters
         * in multiple atomics due to aliasing
         */
        __atomic_conds()[index].notify_all();
    };

    template <class _Ty>
    class _Locked_pointer {
    private:
        static_assert(alignof(_Ty) >= (1 << 2), "2 low order bits are needed by _Locked_pointer");
        static constexpr uintptr_t _Lock_mask = 3;
        static constexpr uintptr_t _Not_locked = 0;
        static constexpr uintptr_t _Locked_notify_not_needed = 1;
        static constexpr uintptr_t _Locked_notify_needed = 2;
        static constexpr uintptr_t _Ptr_value_mask = ~_Lock_mask;

    public:
        constexpr _Locked_pointer() noexcept : _Storage{} {}
        explicit _Locked_pointer(_Ty* const _Ptr) noexcept : _Storage{ reinterpret_cast<uintptr_t>(_Ptr) } {}

        _Locked_pointer(const _Locked_pointer&) = delete;
        _Locked_pointer& operator=(const _Locked_pointer&) = delete;

        _NODISCARD _Ty* _Lock_and_load() noexcept {
            uintptr_t _Rep = _Storage.load(std::memory_order::memory_order_relaxed);
            for (;;) {
                switch (_Rep & _Lock_mask) {
                case _Not_locked: // Can try to lock now
                    if (_Storage.compare_exchange_weak(_Rep, _Rep | _Locked_notify_not_needed)) {
                        return reinterpret_cast<_Ty*>(_Rep);
                    }
                    _YIELD_PROCESSOR();
                    break;

                case _Locked_notify_not_needed: // Try to set "notify needed" and wait
                    if (!_Storage.compare_exchange_weak(_Rep, (_Rep & _Ptr_value_mask) | _Locked_notify_needed)) {
                        // Failed to set notify needed flag, try again
                        _YIELD_PROCESSOR();
                        break;
                    }
                    _Rep = (_Rep & _Ptr_value_mask) | _Locked_notify_needed;
                    [[fallthrough]];

                case _Locked_notify_needed: // "Notify needed" is already set, just wait
                    GL::atomic_wait(&_Storage, _Rep, std::memory_order::memory_order_relaxed);
                    _Rep = _Storage.load(std::memory_order::memory_order_relaxed);
                    break;

                default: // Unrecognized bit pattern
                    _CSTD abort();
                }
            }
        }

        void _Store_and_unlock(_Ty* const _Value) noexcept {
            const auto _Rep = _Storage.exchange(reinterpret_cast<uintptr_t>(_Value));
            if ((_Rep & _Lock_mask) == _Locked_notify_needed) {
                // As we don't count waiters, every waiter is notified, and then some may re-request notification
                GL::atomic_notify_one(&_Storage);
            }
        }

        _NODISCARD _Ty* _Unsafe_load_relaxed() const noexcept {
            return reinterpret_cast<_Ty*>(_Storage.load(std::memory_order::memory_order_relaxed));
        }

    private:
        std::atomic<uintptr_t> _Storage;
    };

    //template <typename T>
    //class atomic_shared_ptr {
    //private:
    //    struct control_block {
    //        std::atomic<size_t> global_refcount;
    //    };
    //    struct local_block {
    //        control_block* cblock;
    //        size_t local_refcount;
    //    };
    //    std::atomic<local_block> atomic_cptr;
    //    static_assert(decltype(atomic_cptr)::is_always_lock_free);
    //public:
    //    control_block* load() {
    //        // 1. increment local refcount
    //        local_block value = atomic_cptr.load();
    //        for (;;) {
    //            local_block new_value = value;
    //            ++new_value.local_refcount;
    //            if (atomic_cptr.compare_exchange_weak(value, new_value))
    //                break;
    //        }
    //        ++value.local_refcount;
    //        // 2. get copy
    //        control_block* cblock = value.cblock;
    //        // 3. increment global counter
    //        cblock->global_refcount.fetch_add(1);
    //        // 4. decrement local refcount
    //        local_block value_before = value;
    //        for (;;) {
    //            local_block new_value = value;
    //            --new_value.local_refcount;
    //            if (atomic_cptr.compare_exchange_weak(value, new_value))
    //                break;
    //            // if the value changed, we were not supposed to modify the global refcount
    //            if (value_before->cblock != value->cblock) {
    //                cblock->global_refcount.fetch_sub(1);
    //                break;
    //            }
    //        }
    //        return cblock;
    //    };
    //};

};
