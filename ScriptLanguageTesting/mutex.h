#pragma once

#include "util.h"
#include <ShlDisp.h>
#include <winnt.h>
#include <mutex>
#include <shared_mutex>
#include <atomic>

// Atomic Mutexes
namespace GL {
    /* *THREAD SAFE* Windows-specific high-performance lock that only locks the OS (slow) when contention actually happens. When there is no contention, this is very fast.
    Generally speaking, out-performs std::mutex under most conditions. */
    class mutex {
    private:
        using mutexHandle_t = RTL_CRITICAL_SECTION;;
        static void				Sys_MutexCreate(mutexHandle_t& handle) noexcept { InitializeCriticalSection(&handle); };
        static void				Sys_MutexDestroy(mutexHandle_t& handle) noexcept { DeleteCriticalSection(&handle); };
        static void				Sys_MutexLock(mutexHandle_t& handle) noexcept { EnterCriticalSection(&handle); };
        static bool				Sys_MutexTryLock(mutexHandle_t& handle) noexcept { return TryEnterCriticalSection(&handle) != 0; };
        static void				Sys_MutexUnlock(mutexHandle_t& handle) noexcept { LeaveCriticalSection(&handle); };

    public:
        mutex() noexcept { Sys_MutexCreate(Handle); };
        mutex(mutex const&) noexcept { Sys_MutexCreate(Handle); };
        mutex(mutex&&) noexcept { Sys_MutexCreate(Handle); };
        mutex& operator=(mutex const&) noexcept { return *this; };
        mutex& operator=(mutex&&) noexcept { return *this; };
        ~mutex() noexcept { Sys_MutexDestroy(Handle); };

        void lock() {
            Sys_MutexLock(Handle);
        };
        bool try_lock() {
            return Sys_MutexTryLock(Handle);
        };
        void unlock() {
            Sys_MutexUnlock(Handle);
        };

    protected:
        mutexHandle_t Handle;

    };

    // a fast alternative to the GoodLang::fast_shared_mutex when prioritizing readers over writers. 
    class fast_shared_mutex {
    private:
        mutable std::atomic<long long> mut{ 0 }; // Read, Write

    public:
        fast_shared_mutex() : mut{ 0 } {};
        fast_shared_mutex(fast_shared_mutex const&) noexcept : mut{ 0 } {};
        fast_shared_mutex(fast_shared_mutex&&) noexcept : mut{ 0 } {};
        fast_shared_mutex& operator=(fast_shared_mutex const&) noexcept { return *this; };
        fast_shared_mutex& operator=(fast_shared_mutex&&) noexcept { return *this; };
        ~fast_shared_mutex() = default;

        bool try_lock() const {
            thread_local long long read, planned;
            read = planned = mut.load(std::memory_order::memory_order_relaxed);
            if (reinterpret_cast<short*>(&planned)[0] == 0) { // no readers...
                if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...
                    if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
                        return true; // success!
                    }
                }
            }
            return false;
        };
        void unlock() const {
            thread_local long long read, planned;
            int i = 0;
            while (true) {
                if (++i > 40) std::this_thread::yield();
                read = planned = mut.load(std::memory_order::memory_order_relaxed);
                --reinterpret_cast<short*>(&planned)[1];
                if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
                    break; // success!
                }
            }
        };
        void lock() const {
            int i = 0;
            while (!try_lock()) {
                if (++i > 40) std::this_thread::yield();
            }
        };

        bool try_lock_shared() const {
            thread_local long long read;
            read = mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1; // immediately increments the Read count, leaves the writer count alone
            if (
                (reinterpret_cast<short*>(&read)[0] >= 1) // we are allowed to read with other readers...
                && (reinterpret_cast<long*>(&read)[1] == 0) // so long as there are no writers...
                ) {
                return true;
            }
            else {
                mut.fetch_add(-1, std::memory_order::memory_order_acq_rel); // failure -- undo our mistake.
                return false;
            }
        };
        void unlock_shared() const {
            mut.fetch_add(-1, std::memory_order::memory_order_acq_rel);
        };
        void lock_shared() const {
            thread_local long long read;
            read = mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1; // immediately increments the Read count, leaves the writer count alone
            while (reinterpret_cast<long*>(&read)[1] != 0) {
                read = mut.load();
            }


            //int i = 0;
            //while (!try_lock_shared()) {
            //	if (++i > 40) std::this_thread::yield();
            //}
        };

        // if you already hold a shared_lock and want to upgrade to a hard lock without releasing.
        // Returns true if this ideal scenario was successful. Returns false otherwise.
        bool upgrade_lock() const {
            thread_local long long read, planned;
            // increment the write count and decrement our read count...
            //for (int i = 0; i < 40; ++i) {
            planned = read = mut.load(std::memory_order::memory_order_relaxed);
            if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...					
                if (--reinterpret_cast<short*>(&planned)[0] == 0) { // we're the only reader...		
                    if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
                        return true;
                    }
                }
            }
            //else {
            //	break;
            //}
        //}

            unlock_shared();
            lock();

            return false;
        };

    };

};