#pragma once

#include "aba_problem.h"
#include <type_traits>
#include "thread_object.h"
#include <shared_mutex>

// Atomic Allocators
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
            return new block_t();
        };
        void PopBlock(block_t* p) {
            delete p;
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
        __declspec(noinline) void ReleaseBlocks() {
            while (block_t* ptr = aba_problem::Pop(blocks)) {
                if constexpr (!std::is_pod<T>::value) {
                    for (auto& element : ptr->elements) {
                        if (element.initialized) {
                            reinterpret_cast<T*>(&element.data[0])->~T();
                            element.initialized = false;
                        }
                    }
                }
                PopBlock(ptr);
            }
        };

    public:
        static constexpr bool callable_size = support_count;

        atomic_allocator() : blocks{}, free{} { free.m_n64 = 0; };
        ~atomic_allocator() { ReleaseBlocks(); };

        // calling this unloads all the data and prevents use of the allocator. Should be used when the allocator is about to be deleted but (for whatever reason) needs to be unloaded at a specific schedule.
        void unsafe_unload() {
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
                        if constexpr (!skipInitialization) {
                            if (sizeof...(a) > 0) {
                                new (data) T(std::forward<TArgs>(a)...);
                            }
                            else {
                                std::memset(data, 0, sizeof(T));
                            }
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

    // Thread-safe, lock-free, high-performance page-based allocator with LIFO functionality for memory re-use. Optimized for heavy multithreading. 
    template <typename _type_, size_t num_items = 128, bool skipInitialization = false, bool support_count = false>
    class atomic_parallel_allocator {
    private:
        struct innerType {
            _type_ // actual object, must be the first item...
                T;
            size_t // ... and attached data comes after the actual object.
                threadID;
        };
        thread_object_no_default<atomic_allocator<innerType, num_items, skipInitialization>>
            TLS;
        long
            count;

    public:
        static constexpr bool callable_size = support_count;

        atomic_parallel_allocator() = default;
        ~atomic_parallel_allocator() = default;

        void unsafe_unload() {
            TLS.for_each([](auto& x) {
                x.unsafe_unload();
            });
        };

        template <typename... TArgs> __declspec(noinline) _type_* Alloc(TArgs&&... a) {
            if constexpr (support_count) InterlockedIncrement(reinterpret_cast<volatile long*>(&count));

            innerType* out;
            const auto threadID = GL::util::get_thread_id();
            if constexpr (sizeof...(a) > 0) {
                out = TLS->Alloc(innerType{ _type_{std::forward<TArgs>(a)...}, threadID });
            }
            else {
                out = TLS->Alloc();
                out->threadID = threadID;
            }
            return (_type_*)(out);
        };
        __declspec(noinline) void Free(void* t) {
            innerType* impl = static_cast<innerType*>(t);
            TLS[impl->threadID].Free(impl);
            if constexpr (support_count) InterlockedDecrement(reinterpret_cast<volatile long*>(&count));
        };
        template <typename... TArgs> std::shared_ptr< _type_ > AllocShared(TArgs&&... a) {
            return std::shared_ptr<_type_>(Alloc(std::forward<TArgs>(a)...), [this](_type_* p) { Free(p); });
        };
        size_t size() const {
            if constexpr (callable_size) {
                return count;
            }
            else {
                static_assert("Not compiled to be able to call size() with this allocator.");
            }
        };

    };
};