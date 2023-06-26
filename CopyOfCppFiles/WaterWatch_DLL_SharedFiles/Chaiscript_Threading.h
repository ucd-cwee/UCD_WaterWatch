#pragma once
#include "Precompiled.h"

namespace chaiscript::detail::threading {
#ifndef CHAISCRIPT_NO_THREADS
    template<typename T>
    using unique_lock = std::unique_lock<T>;

    template<typename T>
    using shared_lock = std::shared_lock<T>;

    template<typename T>
    using lock_guard = std::lock_guard<T>;

    using std::shared_mutex;

    using std::mutex;

    using std::recursive_mutex;

    /// Typesafe thread specific storage. If threading is enabled, this class uses a mutex protected map. If
    /// threading is not enabled, the class always returns the same data, regardless of which thread it is called from.
    template<typename T>
    class Thread_Storage {
    public:
        Thread_Storage() = default;
        Thread_Storage(const Thread_Storage&) = delete;
        Thread_Storage(Thread_Storage&&) = delete;
        Thread_Storage& operator=(const Thread_Storage&) = delete;
        Thread_Storage& operator=(Thread_Storage&&) = delete;

        ~Thread_Storage() { t().erase(this); }

        inline const T* operator->() const noexcept {
            cweeThreadedMap<const void*, T>& m = t();
            AUTO g = m.Guard();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            return &*ptr;
        }
        inline const T& operator*() const noexcept {
            cweeThreadedMap<const void*, T>& m = t();
            AUTO g = m.Guard();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            return *ptr;
        }
        inline T* operator->() noexcept {
            cweeThreadedMap<const void*, T>& m = t();
            AUTO g = m.Guard();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            return &*ptr;
        }
        inline T& operator*() noexcept {
            cweeThreadedMap<const void*, T>& m = t();
            AUTO g = m.Guard();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            return *ptr;
        }
        void* m_key;

    private:
        /// todo: is it valid to make this noexcept? The allocation could fail, but if it
        /// does there is no possible way to recover
        static cweeThreadedMap<const void*, T>& t() noexcept {
            static thread_local cweeThreadedMap<const void*, T> my_t;
            return my_t;
        }
    };

#else // threading disabled
    template<typename T>
    class unique_lock {
    public:
        constexpr explicit unique_lock(T&) noexcept {}
        constexpr void lock() noexcept {}
        constexpr void unlock() noexcept {}
    };

    template<typename T>
    class shared_lock {
    public:
        constexpr explicit shared_lock(T&) noexcept {}
        constexpr void lock() noexcept {}
        constexpr void unlock() noexcept {}
    };

    template<typename T>
    class lock_guard {
    public:
        constexpr explicit lock_guard(T&) noexcept {}
    };

    class shared_mutex {
    };

    class recursive_mutex {
    };

    template<typename T>
    class Thread_Storage {
    public:
        constexpr explicit Thread_Storage() noexcept {}

        constexpr inline T* operator->() const noexcept { return &obj; }

        constexpr inline T& operator*() const noexcept { return obj; }

    private:
        mutable T obj;
    };

#endif
}; // namespace chaiscript::detail::threading
