#pragma once

#include "aba_problem.h"
#include <type_traits>
#include "thread_object.h"
#include <shared_mutex>
#include "basic_atomic_allocator.h"

// Atomic Allocators
namespace GL {
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
            count{ 0 };

    public:
        static constexpr bool callable_size = support_count;

        atomic_parallel_allocator() = default;
        atomic_parallel_allocator(atomic_parallel_allocator const&) = default;
        atomic_parallel_allocator(atomic_parallel_allocator &&) = default;
        atomic_parallel_allocator& operator=(atomic_parallel_allocator const&) = default;
        atomic_parallel_allocator& operator=(atomic_parallel_allocator&&) = default;
        ~atomic_parallel_allocator() noexcept = default;

        void unsafe_unload() {
            TLS.for_each([](auto& x) {
                x.unsafe_unload();
            });
        };

        template <typename... TArgs> __declspec(noinline) _type_* Alloc(TArgs&&... a) {
            if constexpr (support_count) InterlockedIncrementNoFence(reinterpret_cast<volatile long*>(&count));

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
            if constexpr (support_count) InterlockedDecrementNoFence(reinterpret_cast<volatile long*>(&count));
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