#pragma once

#include <thread>
#include <exception>
#include <stdexcept>
#include "atomic_vector.h"
#include "util.h"

// Atomic Thread-Local Objects
namespace GL {
    // Equivalent to thread_local, for member objects. New threads that attempt to re-use old indexes are caught, and the object is re-initialized accordingly. 
    template <typename T>
    class thread_object {
    private:
        mutable size_t _tls_size{ 0 };
        T const _default; // for initializing new thread objects
        mutable atomic_vector<std::pair<size_t, T*>> _tls;
        static size_t actual_thread_id() {
            static thread_local size_t unique_hash{ GL::util::inline_hash(GL::util::get_current_epoch(), std::this_thread::get_id()) };
            return unique_hash;

            //static thread_local std::thread::id thread_id{ std::this_thread::get_id() };
            //return thread_id;
        };

        auto& GetTLS() const {
            auto _tl_index = GL::util::get_thread_id(); // index of our thread, kept to the smallest number(s) we can. Indexes are re-used frequently, even during the lifetime of this thread_object. 
            auto _tl_unique_id = actual_thread_id(); // actual unique hash id of our thread. Will not be re-used by any thread. Even if the same thread dies and is re-born, the epoch may catch that. 

            // step 1, grow the _tls if necessary
            if (_tls_size <= _tl_index) { // lazy growth, taking advantage of grow_to_at_least being safe to call on repeat. 
                (void)_tls.grow_to_at_least(_tl_index + 1);
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_size), _tl_index + 1);
            }

            // step 2, get the address of the unique _tls slot
            auto& _tls_slot = _tls.at(_tl_index);

            // step 2, detect if the thread id changed (including if it was never initialized at all)
            if (_tls_slot.first != _tl_unique_id) {
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_slot.first), _tl_unique_id);
                T* newPtr{ new T(_default) };
                if (T* old_ptr = reinterpret_cast<T*>(InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&_tls_slot.second), newPtr))) {
                    delete old_ptr;
                }
            }

            // step 3, return the resultign pointer, which should be properly initialized.
            return *_tls_slot.second;
        };
        auto& GetTLS(size_t thread_index) const {
            auto& _tls_slot = _tls[thread_index];
            if (!_tls_slot.second) {
                throw std::runtime_error("The TLS should be previously initialized by the appropriate thread before access");
            }
            return *_tls_slot.second;
        };

    public:
        thread_object() : _default{}, _tls{}, _tls_size{ 0 } {};
        thread_object(T const& original) : _default{ original }, _tls{}, _tls_size{ 0 }  {};
        thread_object(T&& original) : _default{ std::move(original) }, _tls{}, _tls_size{ 0 }  {};
        thread_object(thread_object const& rhs) : _default{ rhs._default }, _tls{}, _tls_size{ 0 } {
            size_t index = 0;
            for (auto& x : rhs._tls) {
                _tls.grow_to_at_least(index + 1);
                _tls[index].first = x.first;
                if (x.second) {
                    _tls[index].second = new T(*x.second);
                }
                ++index;
            }
        };
        thread_object(thread_object&& rhs) : _default{ std::move(rhs._default) }, _tls{ std::move(rhs._tls) }, _tls_size{ rhs._tls_size } { rhs._tls.clear(); };
        thread_object& operator=(thread_object const&) = delete;
        thread_object& operator=(thread_object&&) = delete;
        ~thread_object() {
            for (auto& x : _tls) if (x.second) delete x.second;
        };

        T* operator->() { return &GetTLS(); };
        const T* operator->() const { return &const_cast<thread_object*>(this)->GetTLS(); };
        T& operator*() { return GetTLS(); };
        const T& operator*() const { return const_cast<thread_object*>(this)->GetTLS(); };

        T& operator[](size_t thread_index) { return GetTLS(thread_index); };

        template <typename T> bool for_each_cancellable(T const& func) {
            const auto index = GL::util::get_thread_id();
            size_t i;
            for (i = index; i < _tls_size; ++i) {
                auto& x = _tls[i];
                if (x.second) {
                    if (func(*x.second)) { return true; }
                }
            }
            for (i = 0; (i < index) && (i < _tls_size); ++i) {
                auto& x = _tls[i];
                if (x.second) {
                    if (func(*x.second)) { return true; }
                }
            }
            return false;
        };
        template <typename T> void for_each(T const& func) {
            (void)for_each_cancellable([](auto& x) -> bool {
                func(x);
                return false;
                });
        };

    };

    // Equivalent to thread_local, for member objects. New threads that attempt to re-use old indexes are caught, and the object is re-initialized accordingly. Initializes from nothing, and does not accept a default parameter. 
    template <typename T>
    class thread_object_no_default {
    private:
        mutable size_t _tls_size{ 0 };
        mutable atomic_vector<std::pair<size_t, T*>> _tls;
        static size_t actual_thread_id() {
            static thread_local size_t unique_hash{ GL::util::inline_hash(GL::util::get_current_epoch(), std::this_thread::get_id()) };
            return unique_hash;

            //static thread_local std::thread::id thread_id{ std::this_thread::get_id() };
            //return thread_id;
        };
        template <typename... Args>
        // issue: only one thread could access this call at a time per-thread. However, other threads may (and do) loop over the _tls while it's being initialized.
        auto& InitTLS(Args&&... args) const {
            auto _tl_index = GL::util::get_thread_id(); // index of our thread, kept to the smallest number(s) we can. Indexes are re-used frequently, even during the lifetime of this thread_object. 
            auto _tl_unique_id = actual_thread_id(); // actual unique hash id of our thread. Will not be re-used by any thread. Even if the same thread dies and is re-born, the epoch may catch that. 

            // step 1, grow the _tls if necessary
            if (_tls_size <= _tl_index) { // lazy growth, taking advantage of grow_to_at_least being safe to call on repeat. 
                (void)_tls.grow_to_at_least(_tl_index + 1);
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_size), _tl_index + 1);
            }

            // step 2, get the address of the unique _tls slot
            auto& _tls_slot = _tls.at(_tl_index);

            // step 2, detect if the thread id changed (including if it was never initialized at all)
            if (_tls_slot.first != _tl_unique_id) {
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_slot.first), _tl_unique_id);
                T* newPtr{ new T(std::move(args)...) };
                if (T* old_ptr = reinterpret_cast<T*>(InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&_tls_slot.second), newPtr))) {
                    delete old_ptr;
                }
            }

            // step 3, return the resultign pointer, which should be properly initialized.
            return *_tls_slot.second;
        };

        template <typename... Args>
        // issue: only one thread could access this call at a time per-thread. However, other threads may (and do) loop over the _tls while it's being initialized.
        auto& InitTLS(Args const&... args) const {
            auto _tl_index = GL::util::get_thread_id(); // index of our thread, kept to the smallest number(s) we can. Indexes are re-used frequently, even during the lifetime of this thread_object. 
            auto _tl_unique_id = actual_thread_id(); // actual unique hash id of our thread. Will not be re-used by any thread. Even if the same thread dies and is re-born, the epoch may catch that. 

            // step 1, grow the _tls if necessary
            if (_tls_size <= _tl_index) { // lazy growth, taking advantage of grow_to_at_least being safe to call on repeat. 
                (void)_tls.grow_to_at_least(_tl_index + 1);
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_size), _tl_index + 1);
            }

            // step 2, get the address of the unique _tls slot
            auto& _tls_slot = _tls.at(_tl_index);

            // step 2, detect if the thread id changed (including if it was never initialized at all)
            if (_tls_slot.first != _tl_unique_id) {
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_slot.first), _tl_unique_id);
                T* newPtr{ new T(args...) };
                if (T* old_ptr = reinterpret_cast<T*>(InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&_tls_slot.second), newPtr))) {
                    delete old_ptr;
                }
            }

            // step 3, return the resultign pointer, which should be properly initialized.
            return *_tls_slot.second;
        };

        // issue: only one thread could access this call at a time per-thread. However, other threads may (and do) loop over the _tls while it's being initialized.
        auto& GetTLS() const {
            auto _tl_index = GL::util::get_thread_id(); // index of our thread, kept to the smallest number(s) we can. Indexes are re-used frequently, even during the lifetime of this thread_object. 
            auto _tl_unique_id = actual_thread_id(); // actual unique hash id of our thread. Will not be re-used by any thread. Even if the same thread dies and is re-born, the epoch may catch that. 

            // step 1, grow the _tls if necessary
            if (_tls_size <= _tl_index) { // lazy growth, taking advantage of grow_to_at_least being safe to call on repeat. 
                (void)_tls.grow_to_at_least(_tl_index + 1);
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_size), _tl_index + 1);
            }

            // step 2, get the address of the unique _tls slot
            auto& _tls_slot = _tls.at(_tl_index);

            // step 2, detect if the thread id changed (including if it was never initialized at all)
            if (_tls_slot.first != _tl_unique_id) {
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_slot.first), _tl_unique_id);
                T* newPtr{ new T() };
                if (T* old_ptr = reinterpret_cast<T*>(InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&_tls_slot.second), newPtr))) {
                    delete old_ptr;
                }
            }

            // step 3, return the resultign pointer, which should be properly initialized.
            return *_tls_slot.second;
        };
        // valid call to get a _tls slot when it was properly initialized at some point previously. 
        auto& GetTLS(size_t thread_index) const {
            auto& _tls_slot = _tls[thread_index];
            if (!_tls_slot.second) {
                throw std::runtime_error("The TLS should be previously initialized by the appropriate thread before access");
            }
            return *_tls_slot.second;
        };

    public:
        thread_object_no_default() : _tls{}, _tls_size{ 0 } {};
        thread_object_no_default(thread_object_no_default const& rhs) : _tls{}, _tls_size{ 0 } {
            size_t index = 0;
            for (auto& x : rhs._tls) {
                _tls.grow_to_at_least(index + 1);
                _tls[index].first = x.first;
                if (x.second) {
                    _tls[index].second = new T(*x.second);
                }
                ++index;
            }
        };
        thread_object_no_default(thread_object_no_default&& rhs) : _tls{ std::move(rhs._tls) }, _tls_size{ rhs._tls_size } { rhs._tls.clear(); };
        thread_object_no_default& operator=(thread_object_no_default const&) = delete;
        thread_object_no_default& operator=(thread_object_no_default&&) = delete;
        ~thread_object_no_default() {
            for (auto& x : _tls) if (x.second) delete x.second;
        };

        T* operator->() { return &GetTLS(); };
        const T* operator->() const { return &const_cast<thread_object_no_default*>(this)->GetTLS(); };
        T& operator*() { return GetTLS(); };
        const T& operator*() const { return const_cast<thread_object_no_default*>(this)->GetTLS(); };
        template <typename... Args> T& get_or_init(Args&&... args) { return InitTLS(std::move(args)...); };
        template <typename... Args> T& get_or_init(Args const&... args) { return InitTLS(args...); };

        T& operator[](size_t thread_index) { return GetTLS(thread_index); };
        const T& operator[](size_t thread_index) const { return GetTLS(thread_index); };

        template <typename T> bool for_each_cancellable_alive(T const& func) {
            const auto index = GL::util::get_thread_id();
            size_t i;
            for (i = index; i < _tls_size; ++i) {
                if (GL::util::get_thread_alive(i)) {
                    auto& x = _tls[i];
                    if (x.second) {
                        if (func(*x.second)) { return true; }
                    }
                }
            }
            for (i = 0; (i < index) && (i < _tls_size); ++i) {
                if (GL::util::get_thread_alive(i)) {
                    auto& x = _tls[i];
                    if (x.second) {
                        if (func(*x.second)) { return true; }
                    }
                }
            }
            return false;
        };
        template <typename T> bool for_each_cancellable(T const& func) {
            const auto index = GL::util::get_thread_id();
            size_t i;
            for (i = index; i < _tls_size; ++i) {
                auto& x = _tls[i];
                if (x.second) {
                    if (func(*x.second)) { return true; }
                }
            }
            for (i = 0; (i < index) && (i < _tls_size); ++i) {
                auto& x = _tls[i];
                if (x.second) {
                    if (func(*x.second)) { return true; }
                }
            }
            return false;
        };
        template <typename T> bool for_each_cancellable(const size_t index, T const& func) {
            size_t i;
            for (i = index; i < _tls_size; ++i) {
                auto& x = _tls[i];
                if (x.second) {
                    if (func(*x.second)) { return true; }
                }
            }
            for (i = 0; (i < index) && (i < _tls_size); ++i) {
                auto& x = _tls[i];
                if (x.second) {
                    if (func(*x.second)) { return true; }
                }
            }
            return false;
        };
        template <typename T> void for_each(T const& func) {
            (void)for_each_cancellable([&func](auto& x) -> bool {
                func(x);
                return false;
            });
        };
        template <typename T> void for_each_alive(T const& func) {
            (void)for_each_cancellable_alive([&func](auto& x) -> bool {
                func(x);
                return false;
                });
        };
    };
};