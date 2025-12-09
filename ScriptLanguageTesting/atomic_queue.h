#pragma once

#include <atomic>
#include "aba_problem.h"
#include "thread_object.h"
#include "atomic_allocator.h"
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>

// Atomic Queues
namespace GL {
    // Thread-safe, lock-free, okay-performance queue with FIFO functionality. 
    // FIFO is a slower guarrantee than LIFO, so if FIFO is not needed, prefer use of a stack. 
    template<typename T>
    class atomic_parallel_queue {
        constexpr static bool is_pod = std::is_pod_v<T> && (sizeof(T) <= 8);
        using copy_type = std::conditional_t<is_pod, T, T*>;

        thread_object_no_default<concurrency::concurrent_queue<copy_type>>
            _que{};
        atomic_parallel_allocator<T>
            _alloc{};
        std::atomic<size_t>
            count{ 0 };

    public:
        size_t push(T const& obj) {
            if constexpr (is_pod) {
                _que->push(obj);
            }
            else {
                _que->push(_alloc.Alloc(obj));
            }
            return ++count;
        };
        size_t push(T&& obj) {
            if constexpr (is_pod) {
                _que->push(std::move(obj));
            }
            else {
                _que->push(_alloc.Alloc(std::move(obj)));
            }
            return ++count;
        };
        size_t push(size_t thread_index, T const& obj) {
            if constexpr (is_pod) {
                _que[thread_index].push(obj);
            }
            else {
                _que[thread_index].push(_alloc.Alloc(obj));
            }
            return ++count;
        };
        size_t push(size_t thread_index, T&& obj) {
            if constexpr (is_pod) {
                _que[thread_index].push(std::move(obj));
            }
            else {
                _que[thread_index].push(_alloc.Alloc(std::move(obj)));
            }
            return ++count;
        };
        bool try_pop(T& out) {
            if (count.load(std::memory_order_relaxed) == 0) return false;
            if constexpr (is_pod) {
                if (_que.for_each_cancellable([&out](auto& Q) -> bool {
                    return Q.try_pop(out);
                    })) {
                    --count;
                    return true;
                }
                else {
                    return false;
                };
            }
            else {
                T* ptr{ nullptr };
                if (_que.for_each_cancellable([&ptr](auto& Q) -> bool {
                    return Q.try_pop(ptr);
                    })) {
                    if (ptr) {
                        --count;
                        if constexpr (std::is_move_assignable<T>::value) {
                            out = std::move(*ptr);
                        }
                        else {
                            out = *ptr;
                        }
                        _alloc.Free(ptr);
                        return true;
                    }
                };
                return false;
            }
        };
        size_t size() const {
            return count.load();
        };

    };

#if 0
    // Thread-safe, lock-free, poor-performance queue with FIFO functionality. Lower memory footprint than atomic_parallel_queue. Just use the atomic_parallel_queue. 
    template<typename T>
    class atomic_queue {
        constexpr static bool is_pod = std::is_pod_v<T> && (sizeof(T) <= 8);
        using copy_type = std::conditional_t<is_pod, T, T*>;

        concurrency::concurrent_queue<copy_type>
            _que{};
        atomic_allocator<T>
            _alloc{};
        std::atomic<size_t>
            count{ 0 };
    public:
        void push(T const& obj) {
            if constexpr (is_pod) {
                _que.push(obj);
            }
            else {
                _que.push(_alloc.Alloc(obj));
            }
            ++count;
        };
        void push(T&& obj) {
            if constexpr (is_pod) {
                _que.push(std::move(obj));
            }
            else {
                _que.push(_alloc.Alloc(std::move(obj)));
            }
            ++count;
        };
        bool try_pop(T& out) {
            if constexpr (is_pod) {
                if (_que.try_pop(out)) {
                    --count;
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                T* ptr{ nullptr };
                if (!_que.try_pop(ptr)) {
                    return false;
                }

                if (ptr) {
                    --count;
                    if constexpr (std::is_move_assignable<T>::value) {
                        out = std::move(*ptr);
                    }
                    else {
                        out = *ptr;
                    }
                    _alloc.Free(ptr);
                    return true;
                }
                else {
                    return false;
                }
            }
        };
        size_t size() const {
            return count.load();
        };
    };
#else
    template<typename T>
    using atomic_queue = atomic_parallel_queue<T>;
#endif

    // Thread-safe, lock-free, poor-performance queue with sorting.     
    // std::greater results in smaller objects getting pop'd first. 
    // std::less results in larger objects getting pop'd first.
    // Order is guarranteed in this variety. 
    template<typename T, typename Compare = std::greater<T>>
    using atomic_priority_queue = concurrency::concurrent_priority_queue<T, Compare>;

    // Thread-safe, lock-free, okay-performance queue with sorting.     
    // std::greater results in smaller objects getting pop'd first. 
    // std::less results in larger objects getting pop'd first.
    // Order is not 100% guarranteed in this variety, due to the parallelization, but is "likely" to be ordered. 
    template<typename T, typename Compare = std::greater<T>>
    class atomic_parallel_priority_queue {
        struct cmp_ptrs {
            _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef T* _FIRST_ARGUMENT_TYPE_NAME;
            _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef T* _SECOND_ARGUMENT_TYPE_NAME;
            _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef bool _RESULT_TYPE_NAME;

            _NODISCARD constexpr bool operator()(T* _Left, T* _Right) const {
                return Compare{}(*_Left, *_Right);
            }
        };

        constexpr static bool is_pod = std::is_pod_v<T> && (sizeof(T) <= 8);
        thread_object_no_default<atomic_priority_queue<T*, cmp_ptrs>>
            _que{};
        atomic_parallel_allocator<T>
            _alloc{};
        std::atomic<size_t>
            count{ 0 };

    public:
        void push(T const& obj) {
            _que->push(_alloc.Alloc(obj));
            ++count;
        };
        void push(T&& obj) {
            _que->push(_alloc.Alloc(std::move(obj)));
            ++count;
        };
        bool try_pop(T& out) {
            T* ptr{ nullptr };
            if (_que.for_each_cancellable([&ptr](auto& Q) -> bool {
                return Q.try_pop(ptr);
                })) {
                if (ptr) {
                    --count;
                    if constexpr (std::is_move_assignable<T>::value) {
                        out = std::move(*ptr);
                    }
                    else {
                        out = *ptr;
                    }
                    _alloc.Free(ptr);
                    return true;
                }
            };
            return false;
        };
        size_t size() const {
            return count.load();
        };

    };

};