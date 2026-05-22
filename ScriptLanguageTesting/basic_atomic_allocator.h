#pragma once
#include "aba_problem.h"
#include "util.h"
#include <type_traits>
#include <shared_mutex>
#include <iostream>

namespace GL {
    // Thread-safe, lock-free, good-performance page-based allocator with LIFO functionality for memory re-use. Lower memory footprint than atomic_parallel_allocator.
    template <typename T, size_t BlockSize = 128, bool skipInitialization = false, bool support_count = false>
    class atomic_allocator {
    private:
        struct element_t {
            unsigned char
                data[(((sizeof(T) + sizeof(element_t*) + sizeof(bool)) + 15) & ~15) - sizeof(element_t*) - sizeof(bool)]; // wrapped to 16-byte blocks for the entire element_t
            element_t*
                m_pNext;
            bool
                initialized;
        };
        struct block_t {
            element_t
                elements[BlockSize];
            block_t*
                m_pNext;
        };
        
        block_t* PushBlock() {
            block_t* p = reinterpret_cast<block_t*>(GL::malloc(sizeof(block_t)));
            if constexpr (!skipInitialization) if (p) std::memset(p, 0, sizeof(block_t));
            return p;
        };
        void PopBlock(block_t* p) {
            GL::mfree(p);
        };

        // Allocate one new block of contiguous elements
        __declspec(noinline) void AllocBlock() {
            block_t* new_block_ptr = PushBlock();
            aba_problem::Stack_Push(blocks, new_block_ptr);
            block_t& block = *new_block_ptr;

            // add the new elements to the list
            // std::memset(&block->elements[0], 0, sizeof(block_t));
            for (int i = 0; i < BlockSize - 1; ++i) block.elements[i].m_pNext = &block.elements[i + 1];
            block.elements[BlockSize - 1].m_pNext = nullptr;

            // push pNode onto head of list.
            uint64_t old;
            aba_problem::THead<element_t> New;
            while (true) { // race loop
                // Get an atomic copy of head and call it old.
                // Copy old and call it new.                    
                old = New.m_n64 = free.m_n64;

                // Wire the tail of this block to connect to the old head ptr
                block.elements[BlockSize - 1].m_pNext = New.Node();

                // change New's head ptr, which bumps internal aba
                New.Node(&block.elements[0]); // head shall be the start of this block

                // compare and swap New with Head if it still matches Old.
                if (aba_problem::CAS(&free.m_n64, old, New.m_n64))
                    break; // success
                // race, try again
            }
        };

        // Release all memory held by all blocks
        __declspec(noinline) void ReleaseBlocks() noexcept {
            while (true) {                
                if (block_t* ptr = aba_problem::Pop(blocks)) {
                    if constexpr (!std::is_pod_v<T>) {
                        for (int element_i = 0; element_i < BlockSize; ++element_i) {
                            auto& element = ptr->elements[element_i];
                            if (element.initialized) {
                                reinterpret_cast<T*>(&element.data[0])->~T();
                                element.initialized = false;
                            }
                        }
                    }
                    PopBlock(ptr);
                }
                else {
                    break;
                }
            }
        };

    public:
        static constexpr bool callable_size = support_count;

        atomic_allocator() : blocks{}, free{}, count{ 0 } { 
            free.m_n64 = 0; 
        };
        atomic_allocator(atomic_allocator const&) = delete;
        atomic_allocator(atomic_allocator &&) = delete;
        atomic_allocator& operator=(atomic_allocator const&) = delete;
        atomic_allocator& operator=(atomic_allocator&&) = delete;
        __declspec(noinline) ~atomic_allocator() noexcept {
            ReleaseBlocks(); 
        };

        // calling this unloads all the data and prevents use of the allocator. Should be used when the allocator is about to be deleted but (for whatever reason) needs to be unloaded at a specific schedule.
        __declspec(noinline) void unsafe_unload() {
            ReleaseBlocks();
        };

        // Acquire a new element from the free list and construct it.
        template <typename... TArgs> __declspec(noinline) T* Alloc(TArgs &&... a) {
            if constexpr (support_count) InterlockedIncrement(reinterpret_cast<volatile long*>(&count));

            element_t* element{ nullptr };
            while (1) {
                if (element = aba_problem::Pop(free)) {
                    element->initialized = true;
                    T* data{ (T*)&element->data[0] };
                    if constexpr (std::is_pod<T>::value) {
                        if constexpr (sizeof...(a) > 0) {
                            new (data) T(std::forward<TArgs>(a)...);
                        }
                        else if constexpr (!skipInitialization) {
                            std::memset(data, 0, sizeof(T));
                        }
                    }
                    else {
                        new (data) T(std::forward<TArgs>(a)...);
                    }
                    return data;
                }
                else {
                    AllocBlock();
                }
            }
        };

        // Destroys the element and return its memory to the free list
        __declspec(noinline) void Free(T* element) {
            element_t* t = (element_t*)(element);
            if constexpr (!std::is_pod<T>::value) {
                if (t->initialized) {
                    element->~T();
                }
            }
            t->initialized = false;
            aba_problem::Stack_Push(free, t);
            if constexpr (support_count) InterlockedDecrement(reinterpret_cast<volatile long*>(&count));
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

        size_t size() const {
            if constexpr (callable_size) {
                return count;
            }
            else {
                static_assert("Not compiled to be able to call size() with this allocator.");
            }
        };

    private:
        aba_problem::THead<block_t>
            blocks;
        aba_problem::THead<element_t>
            free;
        long
            count;
    };
};