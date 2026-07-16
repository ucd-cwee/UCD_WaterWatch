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

        thread_object_no_default<container>
            head;
    public:
        ~atomic_parallel_void_stack() {
            bool Continue = true;
            while (Continue) {
                Continue = false;
                head.for_each([&Continue](container& this_head) {
                    // if (this_head.which = !this_head.which)
                    if (InterlockedExchange8(reinterpret_cast<volatile char*>(&this_head.which), !this_head.which) == 0)
                        while (element_t* ptr = aba_problem::Pop(this_head.head_1)) {
                            ::_aligned_free(ptr);
                            Continue = true;
                        }
                    else
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
            if (thisHead.which == 0) {
                aba_problem::Stack_Push(thisHead.head_1, new_ptr);
            }
            else {
                aba_problem::Stack_Push(thisHead.head_2, new_ptr);
            }
        };
        void free_all() {
            head.for_each([](container& this_head) {
                // if (this_head.which = !this_head.which)
                if (InterlockedExchange8(reinterpret_cast<volatile char*>(&this_head.which), !this_head.which) == 0)
                    while (element_t* ptr = aba_problem::Pop(this_head.head_1)) ::_aligned_free(ptr); 
                else 
                    while (element_t* ptr = aba_problem::Pop(this_head.head_2)) ::_aligned_free(ptr);
            });            
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