/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

This file is distributed under the BSD License.
Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
http://www.chaiscript.com

 History: RTG	/	2023		1. Modified original source code to use WaterWatch tools, and for better real-time support, including object-typing from parsed code, pre-parsing code without running, real multithreaded code analysis, and more.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "chaiscript_wrapper.h"

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
            m.Lock();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            m.Unlock();
            return &*ptr;
        }
        inline const T& operator*() const noexcept {
            cweeThreadedMap<const void*, T>& m = t();
            m.Lock();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            m.Unlock();
            return *ptr;
        }
        inline T* operator->() noexcept {
            cweeThreadedMap<const void*, T>& m = t();
            m.Lock();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            m.Unlock();
            return &*ptr;
        }
        inline T& operator*() noexcept {
            cweeThreadedMap<const void*, T>& m = t();
            m.Lock();
            if (!m.UnsafeCheck(this)) {
                m.UnsafeEmplace(this, T());
            }
            AUTO ptr = m.UnsafeGetPtr(this);
            m.Unlock();
            return *ptr;
        }
        void* m_key;

    private:
        /// todo: is it valid to make this noexcept? The allocation could fail, but if it
        /// does there is no possible way to recover
        static cweeThreadedMap<const void*, T>& t() noexcept {
            static thread_local cweeSharedPtr<cweeThreadedMap<const void*, T>> my_t(new cweeThreadedMap<const void*, T>());
            return *my_t;
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
