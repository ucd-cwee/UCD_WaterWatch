#pragma once

#define NOMINMAX
#include <cstdarg>
#include <type_traits>
#include <tuple>
#include <ShlDisp.h>
#include <winnt.h>

// Good Language namespace
namespace GL {
    // utilities
    namespace util {
        inline static void hash(size_t& seed) { };
        template <typename T, typename... Rest> inline static void hash(size_t& seed, T const& v, Rest const&... rest) {
            if constexpr (std::is_same_v<double, typename std::remove_reference_t<typename std::decay<T>>>) {
                seed ^= *(size_t*)(void*)(&v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            else {
                std::hash<T> hasher{};
                seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            hash(seed, rest...);
        };
        template <typename T, typename... Rest> inline static size_t inline_hash(T const& v, Rest const&... rest) {
            size_t seed{ 0 };
            hash(seed, v, rest...);
            return seed;
        };

        // returns true if a thread at this ID is alive and running.    
        bool get_thread_alive(size_t thread_id);
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

// Sequences
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

// Deferred Objects (instantiate when needed or used)
namespace GL {
    // Thread-safe wrapper that only initializes an object when actually used. May simply never initialize an object if never used. 
    template <typename T>
    class deferred {
    private:
        T* ptr{ nullptr };

    public:
        deferred() = default;
        deferred(T const& data) : ptr(new T(data)) {};
        deferred(T&& data) : ptr(new T(std::move(data))) {};
        deferred(deferred const&) = delete;
        deferred(deferred&& rhs) : ptr(std::move(rhs.ptr)) { rhs.ptr = nullptr; };
        deferred& operator=(deferred const&) = delete;
        deferred& operator=(deferred&& rhs) {
            if (ptr) delete ptr;
            ptr = std::move(rhs.ptr);
            rhs.ptr = nullptr;
            return *this;
        };
        ~deferred() {
            if (ptr) delete ptr;
        };

        bool valid() const {
            return ptr;
        };
        T* operator->() const {
            if (!ptr) {
                if (auto* newPtr = new T()) {
                    if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(reinterpret_cast<PVOID*>(const_cast<T**>(&ptr))), newPtr, nullptr) != nullptr) {
                        delete newPtr;
                    }
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





