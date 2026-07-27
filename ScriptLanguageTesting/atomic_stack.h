#pragma once

#include <atomic>
#include "aba_problem.h"
#include "thread_object.h"
#include "atomic_allocator.h"

// Atomic Stacks
namespace GL {
    // Thread-safe, lock-free, high-performance queue with LIFO functionality.
    template <typename T>
    class atomic_parallel_stack {
        struct element_t {
            T
                data;
            element_t*
                m_pNext;
        };
        atomic_parallel_allocator<element_t, 128, true>
            allocator;
        thread_object_no_default<aba_problem::THead<element_t>>
            head; // 
        std::atomic<size_t>
            count;
    public:
        size_t push(T const& obj) {
            // get a new element
            element_t* new_ptr;
            new_ptr = allocator.Alloc();
            new_ptr->data = obj;
            new_ptr->m_pNext = nullptr;

            aba_problem::Stack_Push(*head, new_ptr);
            return ++count;
        };
        size_t push(T&& obj) {
            // get a new element
            element_t* new_ptr;
            new_ptr = allocator.Alloc();
            new_ptr->data = std::move(obj);
            new_ptr->m_pNext = nullptr;
            aba_problem::Stack_Push(*head, new_ptr);
            return ++count;
        };
        size_t push(size_t thread_index, T const& obj) {
            // get a new element
            element_t* new_ptr;
            new_ptr = allocator.Alloc();
            new_ptr->data = obj;
            new_ptr->m_pNext = nullptr;
            aba_problem::Stack_Push(head[thread_index], new_ptr);
            return ++count;
        };
        size_t push(size_t thread_index, T&& obj) {
            // get a new element
            element_t* new_ptr;
            new_ptr = allocator.Alloc();
            new_ptr->data = std::move(obj);
            new_ptr->m_pNext = nullptr;
            aba_problem::Stack_Push(head[thread_index], new_ptr);
            return ++count;
        };
        bool try_pop(T& out) {
            if (count.load(std::memory_order_relaxed) == 0) return false;
            return head.for_each_cancellable([&](auto& this_head) -> bool {
                if (element_t* ptr = aba_problem::Pop(this_head)) {
                    if constexpr (std::is_move_assignable<T>::value) {
                        out = std::move(ptr->data);
                    }
                    else {
                        out = ptr->data;
                    }
                    allocator.Free(ptr);
                    --count;
                    return true;
                }
                else {
                    return false;
                }
                });
        };
        template <typename F>
        void for_each_pop(F const& func) {
            if (count == 0) return;
            head.for_each([&](auto& this_head) {
                while (element_t* ptr = aba_problem::Pop(this_head)) {
                    if constexpr (std::is_move_assignable<T>::value) {
                        func(ptr->data);
                    }
                    else {
                        func(ptr->data);
                    }
                    allocator.Free(ptr);
                    --count;
                }
                });
        };
        size_t size() const {
            return count.load();
        };
    };
    
#if 0
    // Thread-safe, lock-free, good-performance queue with LIFO functionality. Lower memory footprint than atomic_parallel_stack.
    template <typename T>
    class atomic_stack {
        struct element_t {
            T
                data;
            element_t*
                m_pNext;
        };
        atomic_allocator<element_t, 128, true>
            allocator;
        aba_problem::THead<element_t>
            head; // 
        std::atomic<size_t>
            count;
    public:
        void push(T const& obj) {
            // get a new element
            element_t* new_ptr;
            //if constexpr (!std::is_pod<T>::value) {
            //    // if pod, try to insert it!
            //    new_ptr = allocator.Alloc(element_t{ obj, nullptr });
            //}
            //else {
            new_ptr = allocator.Alloc();
            new_ptr->data = obj;
            new_ptr->m_pNext = nullptr;
            //}

            aba_problem::Stack_Push(head, new_ptr);
            ++count;
        };
        void push(T&& obj) {
            // get a new element
            element_t* new_ptr;
            //if constexpr (!std::is_pod<T>::value) {
            //    // if pod, try to insert it!
            //    new_ptr = allocator.Alloc(element_t{ std::move(obj), nullptr });
            //}
            //else {
            new_ptr = allocator.Alloc();
            new_ptr->data = std::move(obj);
            new_ptr->m_pNext = nullptr;
            //}

            aba_problem::Stack_Push(head, new_ptr);
            ++count;
        };
        bool try_pop(T& out) {
            if (element_t* ptr = aba_problem::Pop(head)) {
                if constexpr (std::is_move_assignable<T>::value) {
                    out = std::move(ptr->data);
                }
                else {
                    out = ptr->data;
                }
                allocator.Free(ptr);
                --count;
                return true;
            }
            return false;
        };
        size_t size() const {
            return count.load();
        };
    };
#else
    template<typename T>
    using atomic_stack = atomic_parallel_stack<T>;
#endif
};