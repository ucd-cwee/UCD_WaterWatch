#pragma once
#pragma hdrstop

#ifndef NOMINMAX 
#define NOMINMAX
#endif

#include <cstdarg>
#include <type_traits>
#include <tuple>
#include <ShlDisp.h>
#include <winnt.h>
#include <mutex>
#include <shared_mutex>
#include "basic_atomic_allocator.h"

// Good Language namespace
namespace GL {
    // utilities
    namespace util {
        __forceinline static void hash(size_t& seed) { };
        template <typename T, typename... Rest> __forceinline static void hash(size_t& seed, T const& v, Rest const&... rest) {
            if constexpr (std::is_same_v<size_t, typename std::remove_reference_t<typename std::decay<T>>>) {
                seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            if constexpr (std::is_same_v<double, typename std::remove_reference_t<typename std::decay<T>>>) {
                seed ^= *(size_t*)(void*)(&v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            else {
                std::hash<T> hasher{};
                seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            hash(seed, rest...);
        };
        template <typename T, typename... Rest> __forceinline static size_t inline_hash(T const& v, Rest const&... rest) {
            size_t seed{ 0 };
            hash(seed, v, rest...);
            return seed;
        };

        // returns the thread ID of the current, requesting thread from [1, inf). Thread IDs will be re-used once a thread terminates, resulting in low-digit IDs e.g. in practice the id's are between [1,20)
        size_t get_thread_id();
        // get the approximate count of milliseconds since the application launched. 
        long long get_current_epoch();
        // return the number of threads on the current hardware. 
        size_t get_hardware_thread_count();

        // 0..1
        double rand();
        // 0..max or max..0
        double rand(double max);
        // min..max or max..min
        double rand(double min, double max);
        // 0..1 (faster, but not truly random)
        double rand_fast();
        // 0..max or max..0 (faster, but not truly random)
        double rand_fast(double max);
        // min..max or max..min (faster, but not truly random)
        double rand_fast(double min, double max);
    };
};

#pragma region iterator_definition
#define SETUP_ITERATOR(parentClassType, it_state)   \
		class Iterator {   \
		public:   \
			using thisType = typename it_state::thisType;   \
            friend class parentClassType; \
			using value_type = typename it_state::value_type;   \
			using difference_type = ptrdiff_t;   \
		protected:   \
			thisType* parent;   \
			it_state state;   \
		private:   \
			void Initialize() { state.Initialize(parent); };   \
			void ToBeginning() { state.ToBeginning(parent); };   \
			void ToEnd() { state.ToEnd(parent); };   \
			void Next() { state.Next(parent); };   \
			void Prev() { state.Prev(parent); };   \
			decltype(auto) Get() const { return state.Get(parent); };   \
			difference_type Distance(Iterator const& other) const { return state.Distance(other.state); };   \
		public:   \
			Iterator() = default;   \
            Iterator(thisType* _parent, bool toBeginning = true) : parent{ _parent }, state{} { Initialize(); if (toBeginning) ToBeginning(); else ToEnd(); };   \
			Iterator(thisType* _parent, it_state&& _state) : parent{ _parent }, state{ std::forward<it_state>(_state) } {  };   \
			Iterator(const Iterator& rhs) = default;   \
			Iterator(Iterator&& rhs) = default;   \
			Iterator& operator=(const Iterator& rhs) = default;   \
			Iterator& operator=(Iterator&& rhs) = default;   \
			~Iterator() = default;   \
            /*explicit operator bool() const { return state.Valid(parent); }; */  \
			bool operator==(const Iterator& rhs) const { return state == rhs.state; };   \
			bool operator!=(const Iterator& rhs) const { return !operator==(rhs); };   \
			Iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) Next(); return *this; };   \
			Iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) Prev(); return *this; };   \
			difference_type operator-(Iterator const& other) const { return Distance(other); };   \
			Iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) Prev(); return *this; };   \
			Iterator& operator++() { Next(); return *this; };   \
			Iterator& operator--() { Prev(); return *this; };   \
			Iterator operator++(int) { Iterator out(*this); Next(); return out; };   \
			Iterator operator--(int) { Iterator out(*this); Prev(); return out; };   \
			Iterator begin() const { Iterator out(*this); out.ToBeginning(); return out; };   \
			Iterator end() const { Iterator out(*this); out.ToEnd(); return out; };   \
			decltype(auto) operator*() { return Get(); };   \
			decltype(auto) operator*() const { return Get(); };   \
			decltype(auto) operator->() { return &Get(); };   \
			decltype(auto) operator->() const { return &Get(); };   \
		};   \
		using iterator = Iterator;   \
		using const_iterator = Iterator;   \
		Iterator begin() const {   \
			typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;   \
			return Iterator(const_cast<thisType*>(this), true);   \
		};   \
		Iterator end() const {   \
			typedef typename std::remove_const_t< typename std::remove_pointer_t< decltype(&*this) > > thisType;   \
			return Iterator(const_cast<thisType*>(this), false);   \
		};   \
		Iterator cbegin() const { return begin(); };   \
		Iterator cend() const { return end(); };
#pragma endregion 

// Good Language namespace
namespace GL {
    /// <summary>
    /// Iterator that steps through a list, without needing to instance the whole list. 
    /// </summary>
    /// <typeparam name="Type"></typeparam>
    template<typename Type = size_t>
    class sequence {
    private:
        Type min;
        Type max;
        Type step;

        static std::tuple<Type, Type, Type> DetermineSteps(Type N0, Type N1, Type Step) {
            if (Step >= 0) {
                // want to go from small to large
                if (N1 >= N0) {
                    return { N0, N1, Step };
                }
                else {
                    return { N1, N0, Step };
                }
            }
            else {
                // want to go from large to small
                if (N1 >= N0) {
                    return { N1, N0, Step };
                }
                else {
                    return { N0, N1, Step };
                }
            }
        };

    public:
        sequence(Type N0, Type N1, Type Step) {
            std::tie(min, max, step) = DetermineSteps(std::move(N0), std::move(N1), std::move(Step));
        };
        sequence() : sequence(0, 0, 1) {};
        sequence(Type N) : sequence(0, N, 1) {};
        sequence(Type N0, Type N1) : sequence(N0, N1, 1) {};

        class Iterator { // : public std::iterator<std::random_access_iterator_tag, Type>
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = Type;
            using difference_type = ptrdiff_t;
            using pointer = Type*;
            using reference = Type&;

            Iterator() : _ptr(0), _min(0), _step(1) {}
            Iterator(Type rhs, Type min, Type step) : _ptr(rhs), _min(min), _step(step) {}
            Iterator(const Iterator& rhs) : _ptr(rhs._ptr), _min(rhs._min), _step(rhs._step) {}

            inline Iterator& operator+=(difference_type rhs) { _ptr += static_cast<Type>(rhs) * _step; return *this; }
            inline Iterator& operator-=(difference_type rhs) { _ptr -= static_cast<Type>(rhs) * _step; return *this; }
            inline Type& operator*() { return _ptr; }
            inline Type* operator->() { return &_ptr; }
            inline Type operator[](difference_type rhs) { return static_cast<Type>(_min + static_cast<Type>(rhs) * _step); }
            inline const Type& operator*() const { return _ptr; }
            inline const Type* operator->() const { return &_ptr; }
            inline const Type operator[](difference_type rhs) const { return static_cast<Type>(_min + static_cast<Type>(rhs) * _step); }

            inline Iterator& operator++() { _ptr += _step; return *this; }
            inline Iterator& operator--() { _ptr -= _step; return *this; }
            inline Iterator operator++(int) { Iterator tmp(*this); _ptr += _step; return tmp; }
            inline Iterator operator--(int) { Iterator tmp(*this); _ptr -= _step; return tmp; }
            inline difference_type operator-(const Iterator& rhs) const { return (_ptr - rhs._ptr) / _step; }
            inline Iterator operator+(difference_type rhs) const { return Iterator(_ptr + static_cast<Type>(rhs) * _step, _min, _step); }
            inline Iterator operator-(difference_type rhs) const { return Iterator(_ptr - static_cast<Type>(rhs) * _step, _min, _step); }
            friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { return Iterator((static_cast<Type>(lhs) * rhs._step) + rhs._ptr, rhs._min, rhs._step); }
            friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { return Iterator((static_cast<Type>(lhs) * rhs._step) - rhs._ptr, rhs._min, rhs._step); }

            inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
            inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
            inline bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
            inline bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
            inline bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
            inline bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

        protected:
            Type _min;
            Type _ptr;
            Type _step;
        };

        using iterator = Iterator;
        using const_iterator = iterator;

        auto begin() { return Iterator(min, min, step); };
        auto end() { return Iterator(max, min, step); };
        auto cbegin() const { return iterator(min, min, step); };
        auto cend() const { return iterator(max, min, step); };
        auto begin() const { return iterator(min, min, step); };
        auto end() const { return iterator(max, min, step); };
    };
};

// Good Language namespace
namespace GL {
    // Deferred Objects (instantiate when needed or used)
    // Thread-safe wrapper that only initializes an object when actually used. May simply never initialize an object if never used. 
    template <typename T>
    class deferred {
    private:
        //static auto& shared_allocator() {
        //    static GL::atomic_allocator<T> alloc;
        //    return alloc;
        //};
        T* ptr;
        std::unique_ptr<T> p;
    public:
        deferred() : ptr{ nullptr } {};
        //deferred(T* data) : ptr{ data } {};
        deferred(deferred const& rhs) = delete; 
        deferred(deferred&& rhs) = delete; 
        deferred& operator=(deferred const&) = delete;
        deferred& operator=(deferred&& rhs) = delete;
        __declspec(noinline) ~deferred() noexcept {
            ptr = nullptr;
            p.reset(nullptr);

            //if (ptr != nullptr) {
            //    //shared_allocator().Free(ptr);

            //    ptr = nullptr;
            //}
        };

        bool valid() const {
            return ptr;
        };
        T* operator->() const {            
            if (!ptr) {
                if constexpr (std::is_constructible_v<T>) {
                    //auto* newPtr = shared_allocator().Alloc();
                    auto pTemp = std::make_unique<T>();
                    auto* newPtr = pTemp.get();
                    if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(reinterpret_cast<PVOID*>(const_cast<T**>(&ptr))), newPtr, nullptr) != nullptr) {

                        //shared_allocator().Free(newPtr);
                    }
                    else {
                        const_cast<deferred*>(this)->p = std::move(pTemp);
                    }
                }
                else {
                    std::string error = std::string("Cannot construct type \"") + typeid(T).name() + "\"";
                    throw std::runtime_error(error);
                }
            }
            return ptr;
        };
        T& operator*() const {
            return *operator->();
        };
        operator bool() const { return valid(); };
    };
};

// Good Language namespace
namespace GL {
    // a fast alternative to the GoodLang::fast_shared_mutex when prioritizing readers over writers. 
    class fast_shared_mutex {
    private:
        mutable std::atomic<long long> mut; // Read, Write

    public:
        fast_shared_mutex() : mut{ 0 } {};
        fast_shared_mutex(fast_shared_mutex const&) : mut{ 0 } {};
        fast_shared_mutex(fast_shared_mutex&&) noexcept : mut{ 0 } {};
        fast_shared_mutex& operator=(fast_shared_mutex const&) { return *this; };
        fast_shared_mutex& operator=(fast_shared_mutex&&) noexcept { return *this; };
        ~fast_shared_mutex() noexcept = default;

        bool try_lock() const {
            long long read, planned;
            read = planned = mut.load(std::memory_order::memory_order_relaxed);
            if (reinterpret_cast<short*>(&planned)[0] == 0) { // no readers...
                if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...
                    if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
                        return true; 
                    }
                }
            }
            return false;
        };
        void unlock() const {
            long long read, planned;
            while (true) {
                read = planned = mut.load(std::memory_order::memory_order_relaxed);
                --reinterpret_cast<short*>(&planned)[1];
                if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_acq_rel)) {
                    break; 
                }
            }
        };
        void lock() const {
            while (!try_lock()) {}
        };
        bool try_lock_shared() const {
            long long read{ mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1 }; // immediately increments the Read count, leaves the writer count alone
            if (
                (reinterpret_cast<short*>(&read)[0] >= 1) // we are allowed to read with other readers...
                && (reinterpret_cast<long*>(&read)[1] == 0) // so long as there are no writers...
                ) {
                return true;
            }
            else {
                mut.fetch_add(-1, std::memory_order::memory_order_relaxed); // failure -- undo our mistake.
                return false;
            }
        };
        void unlock_shared() const {
            mut.fetch_add(-1, std::memory_order::memory_order_relaxed);
        };
        void lock_shared() const {
            long long read{ mut.fetch_add(1, std::memory_order::memory_order_relaxed) + 1 }; // immediately increments the Read count, leaves the writer count alone
            while (reinterpret_cast<long*>(&read)[1] != 0) {
                read = mut.load(std::memory_order::memory_order_relaxed);
            }
        };

        // if you already hold a shared_lock and want to upgrade to a hard lock without releasing.
        // Returns true if this ideal scenario was successful. Returns false otherwise.
        bool upgrade_lock() const {
            long long read, planned;
            // increment the write count and decrement our read count...
            //for (int i = 0; i < 40; ++i) {
            planned = read = mut.load(std::memory_order::memory_order_relaxed);
            if (++reinterpret_cast<short*>(&planned)[1] == 1) { // we're the only writer...					
                if (--reinterpret_cast<short*>(&planned)[0] == 0) { // we're the only reader...		
                    if (mut.compare_exchange_weak(read, planned, std::memory_order::memory_order_relaxed)) {
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

// Good Language namespace
namespace GL {
    // Thread-safe wrapper that only initializes an object when actually used. Allows locking the object (shared or exclusive)
    template <class T> 
    class shared_lockable {
    private:
        deferred<T> obj;
        fast_shared_mutex mut;

    public:
        class locked {
        private:
            T& obj;
            fast_shared_mutex* mut;
        public: 
            locked(T& _obj, fast_shared_mutex& _mut) : obj(_obj), mut(&_mut) {
                mut->lock();
            };
            locked(locked const&) = delete;
            locked(locked&&) = delete;
            locked& operator=(locked const&) = delete;
            locked& operator=(locked&&) = delete;
            ~locked() {
                if (mut) mut->unlock();
            };

            void unlock() {
                if (mut) {
                    mut->unlock();
                    mut = nullptr;
                }
            };
            T* operator->() {
                return &obj;
            };
            T& operator*() {
                return obj;
            };
            const T* operator->() const {
                return &obj;
            };
            const T& operator*() const {
                return obj;
            };

        };
        class shared_locked {
        private:
            T& obj;
            fast_shared_mutex* mut;
            bool upgraded;
        public:
            shared_locked(T& _obj, fast_shared_mutex& _mut) : obj(_obj), mut(&_mut), upgraded(false) {
                mut->lock_shared();
            };
            shared_locked(shared_locked const&) = delete;
            shared_locked(shared_locked&&) = delete;
            shared_locked& operator=(shared_locked const&) = delete;
            shared_locked& operator=(shared_locked&&) = delete;
            ~shared_locked() {
                unlock_shared();
            };
            void unlock_shared() {
                if (mut) {
                    if (upgraded) mut->unlock(); 
                    else mut->unlock_shared();
                    mut = nullptr;
                }
            };
            void upgrade_lock() {
                if (mut && !upgraded) {
                    upgraded = true;
                    mut->upgrade_lock();
                }
            };
            T* operator->() {
                return &obj;
            };
            T& operator*() {
                return obj;
            };
            const T* operator->() const {
                return &obj;
            };
            const T& operator*() const {
                return obj;
            };
        };

        shared_lockable() = default;
        shared_lockable(shared_lockable const& rhs) {
            auto locked1 = std::shared_lock(rhs.mut);
            *obj = *rhs.obj;
        };
        shared_lockable(shared_lockable && rhs) {
            auto locked1 = std::scoped_lock(rhs.mut);
            *obj = std::move(*rhs.obj);
        };
        shared_lockable& operator=(shared_lockable const& rhs) {
            auto locked1 = std::shared_lock(rhs.mut);
            auto locked2 = std::scoped_lock(mut);
            *obj = *rhs.obj;
            return *this;
        };
        shared_lockable& operator=(shared_lockable&& rhs) {
            auto locked1 = std::scoped_lock(rhs.mut);
            auto locked2 = std::scoped_lock(mut);
            *obj = std::move(*rhs.obj);
            return *this;
        };
        __declspec(noinline) ~shared_lockable() noexcept = default;

    public:
        locked lock() const {
            return locked(*const_cast<shared_lockable*>(this)->obj, const_cast<shared_lockable*>(this)->mut);
        };
        shared_locked lock_shared() const {
            return shared_locked(*const_cast<shared_lockable*>(this)->obj, const_cast<shared_lockable*>(this)->mut);
        };
        operator bool() const { return (bool)obj; };
    };
};




