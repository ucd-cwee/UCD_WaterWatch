#pragma region "Includes"
#pragma once
#define NOMINMAX
#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <cstdarg>
#include <ShlDisp.h>
#include <winnt.h>
#include <functional>
#include <atomic>
#include <thread>
#include <array>
#include <limits>
#include <map>
#include <set>
#include "Strings.h"
#include "TicketDispensor.h"
#include <concurrent_priority_queue.h>
#include <shared_mutex>
#pragma endregion

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
    // utilities
    namespace util {
        inline static void hash(std::size_t& seed) { };
        template <typename T, typename... Rest> inline static void hash(std::size_t& seed, T const& v, Rest const&... rest) {
            if constexpr (std::is_same_v<double, typename std::remove_reference_t<typename std::decay<T>>>) {
                seed ^= *(uint64_t*)(void*)(&v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            else {
                std::hash<T> hasher{};
                seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            hash(seed, rest...);
        };
        template <typename T, typename... Rest> inline static std::size_t inline_hash(T const& v, Rest const&... rest) {
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
    };
    
    // solutions to the A-B-A atomic switch problem
    namespace aba_problem {
        template <typename T>
        class Node {
        public:
            T data;
            Node* m_pNext;

            Node() = default;
            Node(T&& _data, Node*&& _m_pNext) : data(std::move(_data)), m_pNext(std::move(_m_pNext)) {};
            Node(Node const&) = default;
            Node(Node&&) = default;
            Node& operator=(Node const&) = default;
            Node& operator=(Node&&) = default;
            ~Node() = default;
        };

        template<class T>
        union THead {
        public:
            struct bitset {
            public:
                uint64_t // must sum to 64
                    m_nABA : 12, // 8, 12, and 18 work. Larger = less likelihood of crashing due to ABA bug.
                    m_pNode : 52; // Windows only supports 44 bits addressing anyway.
            };
            uint64_t
                m_n64; // for CAS
            bitset
                m_bits;

            static T* Finalize(T* p) {
                THead<T> out;
                out.m_bits.m_pNode = (uint64_t)p;
                out.m_bits.m_nABA = 0;
                return (T*)out.m_bits.m_pNode;
            };
            bool is_null() const {
                return m_bits.m_pNode == 0;
            };
            // this constructor will make an atomic copy on intel 
            // THead() : m_n64{ 0 } {}
            // THead(THead& r) { m_n64 = r.m_n64; }
            T* Node() { return reinterpret_cast<T*>(m_bits.m_pNode); }
            // changeing Node bumps aba
            THead* Node(T* p) { m_bits.m_nABA++; m_bits.m_pNode = (uint64_t)p; return this; }
        };

        static bool CAS(uint64_t* Destination, uint64_t& Comperand, uint64_t& Exchange) {
            return InterlockedCompareExchange(reinterpret_cast<volatile uint64_t*>(Destination), Exchange, Comperand) == Comperand;
        };

        // pop pNode from head of list.
        template<class T> __declspec(noinline) T* Pop(THead<T>& Head) {
            THead<T> Old, New; // Get an atomic copy of head and call it old.
            while (1) { // race loop                
                New.m_n64 = (Old.m_n64 = Head.m_n64);
                if (Old.is_null()) { break; }
                New.Node(Old.Node()->m_pNext); // change New's Node, which bumps internal aba                
                if (CAS(&Head.m_n64, Old.m_n64, New.m_n64)) // compare and swap New with Head if it still matches Old.       
                    return THead<T>::Finalize(Old.Node()); // success                        
            } // race, try again
            return nullptr; // Head.m_n64.m_pNode was nullptr ... e.g. nothing to pop
        };

        // push pNode onto head of list. 
        template<class T> __declspec(noinline) void Stack_Push(THead<T>& Head, T* pNode) {
            THead<T> Old, New;
            while (1) { // race loop                
                New.m_n64 = Old.m_n64 = Head.m_n64; // Get an atomic copy of head and call it old. Copy old and call it new.                
                pNode->m_pNext = New.Node(); // Wire node t Head                
                New.Node(pNode); // change New's head ptr, which bumps internal aba                
                if (CAS(&Head.m_n64, Old.m_n64, New.m_n64)) // compare and swap New with Head if it still matches Old.
                    break; // success                
            } // race, try again
        }

    };

};

// Atomic Vector
namespace GL {
    // equivalent to concurrency::concurrent_vector. Equal performance or slightly faster (~15%) for small number of items, and slightly worse (~20%) performance for many items.
    // buckets increase the number of allocations by *2 each time. Maximum number of buckets must be known at compile-time. 
    // Therefore, there is a maximum size this vector may have. Note that increasing the maximum number of buckets 
    // should only result in a minor increase to the memory use, and little to no impact on performance. Suggest 24 for ~ 33M items, while 64 would handle nearly all use cases possible. 
    template <typename T, size_t max_num_buckets = 32>
    class atomic_vector {
        inline static const short tab64[64] = {
            63,  0, 58,  1, 59, 47, 53,  2,
            60, 39, 48, 27, 54, 33, 42,  3,
            61, 51, 37, 40, 49, 18, 28, 20,
            55, 30, 34, 11, 43, 14, 22,  4,
            62, 57, 46, 52, 38, 26, 32, 41,
            50, 36, 17, 19, 29, 10, 13, 21,
            56, 45, 25, 31, 35, 16,  9, 12,
            44, 24, 15,  8, 23,  7,  6,  5
        };
        static short log2_64(uint64_t value) noexcept {
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            value |= value >> 32;
            return tab64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
        }
        // 0 -> 0, 4 -> 1, 8 -> 2, 16 -> 3, etc.
        static short global_index_to_block(size_t index) noexcept {
            if (index <= 3ull) return 0;
            else return log2_64(index) - 1;
        };
        // 0 -> 0, 3 -> 3, 4 -> 0, 7 -> 3, 8 -> 0, 15 -> 7, 16 -> 0, 31 -> 15, etc.
        static size_t global_index_to_local_index(size_t index) noexcept {
            if (index <= 3ull) return index;
            else return index - (2ull << (log2_64(index) - 1));
        };
        static size_t global_index_to_local_index(size_t index, short blockN) noexcept {
            if (index <= 3ull) return index;
            else return index - (2ull << blockN);
        };
        // 0 -> 4, 1 -> 4, 2 -> 8, 3 -> 16, 4 -> 32, etc.
        static size_t block_to_allocsize(short block_n) noexcept {
            if (block_n <= 1) return 4ull;
            else return 2ull << block_n;
        };

        using element_t = T;
        std::array< std::vector< element_t >*, max_num_buckets >
            blocks;
        std::atomic<size_t>
            current_pos;
        size_t
            valid_pos;
        short
            current_blockN;

        void EnsureBlockExists(short block_n) noexcept {
            for (short blockN = 0; blockN <= block_n; ++blockN) {
                if (!blocks[blockN]) {
                    auto* new_ptr = new std::vector< element_t >();
                    new_ptr->resize(block_to_allocsize(blockN));
                    if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&blocks[blockN]), new_ptr, nullptr) == nullptr) {}
                    else {
                        delete new_ptr;
                    }
                }
            }
        };
        void grow_to_at_least_blocksN(short blockN) noexcept { EnsureBlockExists(blockN); };
    public:
        atomic_vector() noexcept : blocks{}, current_pos{ 0 }, valid_pos{ 0 }, current_blockN{ -1 } {};
        ~atomic_vector() noexcept {
            for (auto& block : blocks) {
                if (block) {
                    delete block;
                }
                else {
                    break;
                }
            }
        };

        element_t& operator[](size_t index) noexcept {
            return blocks[global_index_to_block(index)]->operator[](global_index_to_local_index(index));
        };
        element_t& at(size_t index) noexcept {
            return blocks[global_index_to_block(index)]->operator[](global_index_to_local_index(index));
        };
        const element_t& operator[](size_t index) const noexcept {
            return blocks[global_index_to_block(index)]->operator[](global_index_to_local_index(index));
        };
        const element_t& at(size_t index) const noexcept {
            return blocks[global_index_to_block(index)]->operator[](global_index_to_local_index(index));
        };
        void grow_to_at_least(size_t index) noexcept {
            EnsureBlockExists(global_index_to_block(index));
            while (true) {
                size_t prevValid = valid_pos;
                if (prevValid < index) {
                    if (InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&valid_pos), index, prevValid) == prevValid) {
                        break;
                    }
                }
                else {
                    break;
                }
            }
        };
        void push_back(element_t const& srce) noexcept {
            size_t position;
            short blockN;

            position = current_pos++;
            blockN = global_index_to_block(position);
            if (current_blockN < blockN) {
                grow_to_at_least_blocksN(blockN);
                InterlockedExchange16(reinterpret_cast<volatile short*>(&current_blockN), blockN);
            }
            blocks[blockN]->operator[](global_index_to_local_index(position, blockN)) = srce;
            InterlockedIncrement(reinterpret_cast<volatile size_t*>(&valid_pos));
        };
        void push_back(element_t&& srce) noexcept {
            size_t position;
            short blockN;

            position = current_pos++;
            blockN = global_index_to_block(position);
            if (current_blockN < blockN) {
                grow_to_at_least_blocksN(blockN);
                InterlockedExchange16(reinterpret_cast<volatile short*>(&current_blockN), blockN);
            }
            blocks[blockN]->operator[](global_index_to_local_index(position, blockN)) = std::move(srce);
            InterlockedIncrement(reinterpret_cast<volatile size_t*>(&valid_pos));
        };

        class Iterator {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = element_t;
            using difference_type = ptrdiff_t;
            using pointer = element_t*;
            using reference = element_t&;

            Iterator(const atomic_vector* p = nullptr, difference_type pos = 0) : _ptr(pos), parent(const_cast<atomic_vector*>(p)) {}
            Iterator(const Iterator& rhs) : _ptr(rhs._ptr), parent(rhs.parent) {}

            inline Iterator& operator+=(difference_type rhs) { _ptr += rhs; return *this; }
            inline Iterator& operator-=(difference_type rhs) { _ptr -= rhs; return *this; }
            inline reference operator*() { return parent->at(_ptr); }
            inline pointer operator->() { return &parent->at(_ptr); }
            inline reference operator[](difference_type rhs) { return parent->at(rhs); }
            inline const reference operator*() const { return parent->at(_ptr); }
            inline const pointer operator->() const { return &parent->at(_ptr); }
            inline const reference operator[](difference_type rhs) const { return parent->at(rhs); }

            inline Iterator& operator++() { _ptr++; return *this; }
            inline Iterator& operator--() { _ptr--; return *this; }
            inline Iterator operator++(int) { Iterator tmp(*this); _ptr++; return tmp; }
            inline Iterator operator--(int) { Iterator tmp(*this); _ptr--; return tmp; }
            inline difference_type operator-(const Iterator& rhs) const { return (_ptr - rhs._ptr); }
            inline Iterator operator+(difference_type rhs) const { Iterator tmp(*this); tmp._ptr += rhs; return tmp; }
            inline Iterator operator-(difference_type rhs) const { Iterator tmp(*this); tmp._ptr -= rhs; return tmp; }
            friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { Iterator tmp(rhs); tmp._ptr = lhs; tmp._ptr += rhs._ptr; return tmp; }
            friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { Iterator tmp(rhs); tmp._ptr = lhs; tmp._ptr -= rhs._ptr; return tmp; }

            inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
            inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
            inline bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
            inline bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
            inline bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
            inline bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

        protected:
            difference_type _ptr;
            atomic_vector* parent;

        };

        using iterator = Iterator;
        using const_iterator = iterator;

        auto begin() { return Iterator(this, 0); };
        auto end() { return Iterator(this, valid_pos); };
        auto cbegin() const { return iterator(this, 0); };
        auto cend() const { return iterator(this, valid_pos); };
        auto begin() const { return iterator(this, 0); };
        auto end() const { return iterator(this, valid_pos); };

    };
};

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
                GL::string str = GL::printf("The TLS should be previously initialized by the appropriate thread before access by [%i].", (int)thread_index);
                std::cout << str << std::endl;
                throw std::runtime_error(str.to_string());
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
                GL::string str = GL::printf("The TLS should be previously initialized by the appropriate thread before access by [%i].", (int)thread_index);
                std::cout << str << std::endl;
                throw std::runtime_error(str.to_string());
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

// Atomic Allocators
namespace GL {
    // Thread-safe, lock-free, good-performance page-based allocator with LIFO functionality for memory re-use. Lower memory footprint than atomic_parallel_allocator.
    template <typename T, size_t BlockSize = 128, bool skipInitialization = false>
    class atomic_allocator {
    private:
        struct element_t {
            unsigned char
                data[sizeof(T)];
            bool
                initialized;
            element_t*
                m_pNext;
        };
        struct block_t {
            element_t
                elements[BlockSize];
            block_t*
                m_pNext;
        };

        // Allocate one new block of contiguous elements
        __declspec(noinline) void AllocBlock() {
            auto* new_block_ptr = new block_t();
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
                delete ptr;
            }
        };

    public:
        atomic_allocator() : blocks{}, free{} { free.m_n64 = 0; };
        ~atomic_allocator() { ReleaseBlocks(); };

        // calling this unloads all the data and prevents use of the allocator. Should be used when the allocator is about to be deleted but (for whatever reason) needs to be unloaded at a specific schedule.
        void unsafe_unload() {
            ReleaseBlocks();
        };

        // Acquire a new element from the free list and construct it.
        template <typename... TArgs> __declspec(noinline) T* Alloc(TArgs &&... a) {
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
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

        aba_problem::THead<block_t>
            blocks;
        aba_problem::THead<element_t>
            free;
    };

    // Thread-safe, lock-free, high-performance page-based allocator with LIFO functionality for memory re-use.
    template <typename _type_, size_t num_items = 128, bool skipInitialization = false>
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

    public:
        atomic_parallel_allocator() = default;
        ~atomic_parallel_allocator() = default;

        void unsafe_unload() {
            TLS.for_each([](auto& x) {
                x.unsafe_unload();
                });
        };

        template <typename... TArgs> __declspec(noinline) _type_* Alloc(TArgs&&... a) {
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
        __declspec(noinline) void Free(const _type_* t) {
            innerType* impl = static_cast<innerType*>(static_cast<void*>(const_cast<_type_*>(t)));
            TLS[impl->threadID].Free(impl);
        };
        template <typename... TArgs> std::shared_ptr< _type_ > AllocShared(TArgs&&... a) {
            return std::shared_ptr<_type_>(Alloc(std::forward<TArgs>(a)...), [this](_type_* p) { Free(p); });
        };
    };

};

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
        void push(T const& obj) {
            // get a new element
            element_t* new_ptr;
            new_ptr = allocator.Alloc();
            new_ptr->data = obj;
            new_ptr->m_pNext = nullptr;

            aba_problem::Stack_Push(*head, new_ptr);
            ++count;
        };
        void push(T&& obj) {
            // get a new element
            element_t* new_ptr;
            new_ptr = allocator.Alloc();
            new_ptr->data = std::move(obj);
            new_ptr->m_pNext = nullptr;
            aba_problem::Stack_Push(*head, new_ptr);
            ++count;
        };
        bool try_pop(T& out) {
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
        size_t size() const {
            return count.load();
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

// Atomic Queues
namespace GL {
    // Thread-safe, lock-free, okay-performance queue with FIFO functionality. 
    // FIFO is a slower guarrantee than LIFO, so if FIFO is not needed, prefer use of a stack. 
    template<typename T>
    class atomic_parallel_queue {
        constexpr static bool is_pod = std::is_pod_v<T> && (sizeof(T) <= 8);
        using copy_type = std::conditional_t<is_pod, T, T*>;

        thread_object_no_default<concurrency::concurrent_queue<copy_type>>
            _que{};
        atomic_parallel_allocator<T>
            _alloc{};
        std::atomic<size_t>
            count{ 0 };

    public:
        void push(T const& obj) {
            if constexpr (is_pod) {
                _que->push(obj);
            }
            else {
                _que->push(_alloc.Alloc(obj));
            }
            ++count;
        };
        void push(T&& obj) {
            if constexpr (is_pod) {
                _que->push(std::move(obj));
            }
            else {
                _que->push(_alloc.Alloc(std::move(obj)));
            }
            ++count;
        };
        bool try_pop(T& out) {
            if constexpr (is_pod) {
                if (_que.for_each_cancellable([&out](auto& Q) -> bool {
                    return Q.try_pop(out);
                })) {
                    --count;
                    return true;
                }
                else {
                    return false;
                };
            }
            else {
                T* ptr{ nullptr };
                if (_que.for_each_cancellable([&ptr](auto& Q) -> bool {
                    return Q.try_pop(ptr);
                })) {
                    if (ptr) {
                        --count;
                        if constexpr (std::is_move_assignable<T>::value) {
                            out = std::move(*ptr);
                        }
                        else {
                            out = *ptr;
                        }
                        _alloc.Free(ptr);
                        return true;
                    }
                };
                return false;
            }
        };
        size_t size() const {
            return count.load();
        };

    };

#if 0
    // Thread-safe, lock-free, poor-performance queue with FIFO functionality. Lower memory footprint than atomic_parallel_queue. Just use the atomic_parallel_queue. 
    template<typename T> 
    class atomic_queue {
        constexpr static bool is_pod = std::is_pod_v<T> && (sizeof(T) <= 8);
        using copy_type = std::conditional_t<is_pod, T, T*>;

        concurrency::concurrent_queue<copy_type>
            _que{};
        atomic_allocator<T>
            _alloc{};
        std::atomic<size_t>
            count{ 0 };
    public:
        void push(T const& obj) {
            if constexpr (is_pod) {
                _que.push(obj);
            }
            else {
                _que.push(_alloc.Alloc(obj));
            }
            ++count;
        };
        void push(T&& obj) {
            if constexpr (is_pod) {
                _que.push(std::move(obj));
            }
            else {
                _que.push(_alloc.Alloc(std::move(obj)));
            }
            ++count;
        };
        bool try_pop(T& out) {
            if constexpr (is_pod) {
                if (_que.try_pop(out)) {
                    --count;
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                T* ptr{ nullptr };
                if (!_que.try_pop(ptr)) {
                    return false;
                }

                if (ptr) {
                    --count;
                    if constexpr (std::is_move_assignable<T>::value) {
                        out = std::move(*ptr);
                    }
                    else {
                        out = *ptr;
                    }
                    _alloc.Free(ptr);
                    return true;
                }
                else {
                    return false;
                }
            }
        };
        size_t size() const {
            return count.load();
        };
    };
#else
    template<typename T>
    using atomic_queue = atomic_parallel_queue<T>;
#endif

    // Thread-safe, lock-free, poor-performance queue with sorting.     
    // std::greater results in smaller objects getting pop'd first. 
    // std::less results in larger objects getting pop'd first.
    // Order is guarranteed in this variety. 
    template<typename T, typename Compare = std::greater<T>>
    using atomic_priority_queue = concurrency::concurrent_priority_queue<T, Compare>;

    // Thread-safe, lock-free, okay-performance queue with sorting.     
    // std::greater results in smaller objects getting pop'd first. 
    // std::less results in larger objects getting pop'd first.
    // Order is not 100% guarranteed in this variety, due to the parallelization, but is "likely" to be ordered. 
    template<typename T, typename Compare = std::greater<T>>
    class atomic_parallel_priority_queue {
        struct cmp_ptrs {
            _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef T* _FIRST_ARGUMENT_TYPE_NAME;
            _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef T* _SECOND_ARGUMENT_TYPE_NAME;
            _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef bool _RESULT_TYPE_NAME;

            _NODISCARD constexpr bool operator()(T* _Left, T* _Right) const {
                return Compare{}(*_Left, *_Right);
            }
        };

        constexpr static bool is_pod = std::is_pod_v<T> && (sizeof(T) <= 8);
        thread_object_no_default<atomic_priority_queue<T*, cmp_ptrs>>
            _que{};
        atomic_parallel_allocator<T>
            _alloc{};
        std::atomic<size_t>
            count{ 0 };

    public:
        void push(T const& obj) {
            _que->push(_alloc.Alloc(obj));            
            ++count;
        };
        void push(T&& obj) {
            _que->push(_alloc.Alloc(std::move(obj)));            
            ++count;
        };
        bool try_pop(T& out) {            
            T* ptr{ nullptr };
            if (_que.for_each_cancellable([&ptr](auto& Q) -> bool {
                return Q.try_pop(ptr);
            })) {
                if (ptr) {
                    --count;
                    if constexpr (std::is_move_assignable<T>::value) {
                        out = std::move(*ptr);
                    }
                    else {
                        out = *ptr;
                    }
                    _alloc.Free(ptr);
                    return true;
                }
            };
            return false;            
        };
        size_t size() const {
            return count.load();
        };

    };

};

// Epoch Allocator
namespace GL {
    // Thread-safe, lock-free, okay-performance allocator that delays destruction until likely safe to do so. 
    // uses the real clock of the OS to time when enough periods have passed that a free'd pointer is likely forgotten. 
    template <typename _type_, typename AllocatorType = atomic_parallel_allocator<_type_>>
    class atomic_epoch_allocator {
    private:
        struct DeleteType {
            long long epoch;
            _type_* ptr;

            friend bool operator<(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch < rhs.epoch;
            };
            friend bool operator>(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch > rhs.epoch;
            };
            friend bool operator<=(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch <= rhs.epoch;
            };
            friend bool operator>=(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch >= rhs.epoch;
            };
            friend bool operator==(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch == rhs.epoch;
            };
            friend bool operator!=(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch != rhs.epoch;
            };
        };

        class TLS {
        public:
            long long
                _scope_count;
            long long
                EpochLimit{ -1 };
            long long
                Epoch_3{ -1 }; // oldest Epoch
            long long
                Epoch_2{ -1 }; // middle Epoch
            long long
                Epoch_1{ -1 }; // youngest Epoch

            long long ForwardEpoch(long long CurrentEpoch) {
                EpochLimit = Epoch_3;
                Epoch_3 = Epoch_2;
                Epoch_2 = Epoch_1;
                Epoch_1 = CurrentEpoch;
                return EpochLimit;
            };
            bool EpochCheck(long long CurrentEpoch) {
                if (_scope_count == 0) {
                    return ForwardEpoch(CurrentEpoch) >= 0;
                }
                else {
                    return false;
                }
            };
            class EpochGuard {
            private:
                atomic_epoch_allocator* _parent_parent;
                TLS* _parent;
                long long _CurrentEpoch;

                void RunGC() {
                    if (_parent_parent && _parent) {
                        if (--_parent->_scope_count == 0) {
                            if (_parent->ForwardEpoch(_CurrentEpoch) >= 0) {
                                _parent_parent->RunGC();
                            }
                        }
                    }
                }

            public:
                EpochGuard() : _parent_parent{ nullptr }, _parent{ nullptr }, _CurrentEpoch{} {};
                EpochGuard(atomic_epoch_allocator* parent_parent, TLS* parent, long long CurrentEpoch) : _parent_parent{ parent_parent }, _parent{ parent }, _CurrentEpoch{ CurrentEpoch } {
                    ++parent->_scope_count;
                };
                EpochGuard(EpochGuard const&) = delete;
                EpochGuard(EpochGuard&& rhs) : _parent_parent{ std::move(rhs._parent_parent) }, _parent{ std::move(rhs._parent) }, _CurrentEpoch{ std::move(rhs._CurrentEpoch) } {
                    rhs._parent = nullptr;
                };
                EpochGuard& operator=(EpochGuard const&) = delete;
                EpochGuard& operator=(EpochGuard&& rhs) {
                    RunGC();
                    _parent_parent = std::move(rhs.__parent_parentparent);
                    _parent = std::move(rhs._parent);
                    _CurrentEpoch = std::move(rhs._CurrentEpoch);
                    rhs._parent_parent = nullptr;
                    rhs._parent = nullptr;
                }
                ~EpochGuard() {
                    RunGC();
                };
            };
        };
        // Allocator means larger memory footprint, but faster when multiple threads are in use. 
        AllocatorType // Allocator<_type_, 32> // , 32 // ABA_Problem::BlockAlloc<_type_, 32> // 
            _alloc;
        atomic_parallel_priority_queue<DeleteType>
            _delete_list; // note that these are NOT available for re-use yet -- these may still be being used by certain threads. 
        GL::thread_object_no_default<TLS>
            _TLS;
        long long
            _lastGC;

    public:
        // Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
        void RunGC() {
            static constexpr long long duration_ms{ 2 };

            long long curr_epoch{ GL::util::get_current_epoch() };
            long long previous_epoch{ _lastGC };
            long long _EpochLimit{ std::numeric_limits<long long>::max() };
            DeleteType out;

            if (((curr_epoch - previous_epoch) > duration_ms) && (InterlockedCompareExchange64(reinterpret_cast<volatile long long*>(&_lastGC), curr_epoch, previous_epoch) == previous_epoch)) {
                _TLS.for_each_alive([&_EpochLimit](TLS& _tls) {
                    if (long long L = _tls.EpochLimit; L >= 0 && L < _tls.Epoch_1) {
                        _EpochLimit = std::min<long long>(_EpochLimit, L);
                    }
                    });

                if ((_EpochLimit > 0) && (_EpochLimit < std::numeric_limits<long long>::max())) {
                    while (_delete_list.try_pop(out)) {
                        if (out.epoch < _EpochLimit) { // deemed safe to delete
                            _alloc.Free(out.ptr);
                        }
                        else { // deemed unsafe to delete just yet
                            _delete_list.push(out);
                            break;
                        }
                    }
                }
            }
        };

    public:
        using GuardType = typename TLS::EpochGuard;

        atomic_epoch_allocator()
            : _alloc{}
            , _delete_list{}
            , _TLS{}
            , _lastGC{ GL::util::get_current_epoch() }
        {};
        atomic_epoch_allocator(atomic_epoch_allocator const&) = delete;
        atomic_epoch_allocator(atomic_epoch_allocator&&) = delete;
        atomic_epoch_allocator& operator=(atomic_epoch_allocator const&) = delete;
        atomic_epoch_allocator& operator=(atomic_epoch_allocator&&) = delete;
        ~atomic_epoch_allocator() = default;

        void unsafe_unload() {
            _alloc.unsafe_unload();
        };

    public:
        GuardType ProtectCurrentEpoch() const {
            return TLS::EpochGuard(
                const_cast<atomic_epoch_allocator*>(this),
                const_cast<TLS*>(&*_TLS),
                GL::util::get_current_epoch()
            );
        };
        void ProtectCurrentEpoch_Fast() const {
            if (const_cast<TLS*>(&*_TLS)->_scope_count == 0) {
                const_cast<TLS*>(&*_TLS)->ForwardEpoch(GL::util::get_current_epoch());
            }
        };

        // Request a new memory pointer
        template <typename... TArgs> _type_* Alloc(TArgs &&... a) {
            return _alloc.Alloc(std::forward<TArgs>(a)...);
        };

        // Frees the memory pointer
        void Free(const _type_* element) {
            _delete_list.push({ GL::util::get_current_epoch(), const_cast<_type_*>(element) });
            if (_TLS->EpochCheck(GL::util::get_current_epoch())) {
                // will only succeed if we are in scope-level 0, which only happens if this thread has not made any protecting guards.
                RunGC();
            }
        };

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

// Atomic Maps
namespace GL {
    // Thread-safe ordered B-Tree, which guarrantees valid and safe access to
    // pointers during erasure or modification of the tree when using the Epoch-guard
    // protection, which will delay actual deletion until the guard is satisfactorily old.
    template< class objType, class keyType, int maxChildrenPerNode = 10> class atomic_btree {
    public:
        struct TreeNode {
            keyType // key used for sorting
                key;
            objType* // if != NULL pointer to object stored in leaf node 
                object;
            TreeNode* // parent node 
                parent;
            TreeNode* // next sibling
                next;
            TreeNode* // prev sibling
                prev;
            long long // number of children	  
                numChildren;
            TreeNode* // first child 
                firstChild;
            TreeNode* // last child
                lastChild;
        };
        typedef TreeNode _iterType;

    private:
        static _iterType*
            InitNode(_iterType* p) {
            p->key = {};
            p->object = nullptr;
            p->parent = nullptr;
            p->next = nullptr;
            p->prev = nullptr;
            p->numChildren = 0;
            p->firstChild = nullptr;
            p->lastChild = nullptr;
            return p;
        };

    private:
        std::atomic<long long>
            Num;
        _iterType
            * root, // must be locked when handled
            * first, // will be exchanged using atomics
            * last; // will be exchanged using atomics
        deferred< atomic_epoch_allocator<objType> >
            objAllocator;
        atomic_epoch_allocator<_iterType>
            nodeAllocator;
        GL::fast_shared_mutex
            mutex;

        class EpochGuard {
        private:
            typename atomic_epoch_allocator<objType>::GuardType guard_1;
            typename atomic_epoch_allocator<_iterType>::GuardType guard_2;

        public:
            EpochGuard(atomic_btree const* parent) : guard_1{ parent->objAllocator->ProtectCurrentEpoch() }, guard_2{ parent->nodeAllocator.ProtectCurrentEpoch() } {};
            EpochGuard(EpochGuard const&) = delete;
            EpochGuard(EpochGuard&& rhs) = delete;
            EpochGuard& operator=(EpochGuard const&) = delete;
            EpochGuard& operator=(EpochGuard&&) = delete;
            ~EpochGuard() = default;
        };

    public:
        static _iterType*
            GetNextLeaf(_iterType* node) {
            if (node) {
                if (node->firstChild) {
                    while (node->firstChild) {
                        node = node->firstChild;
                    }
                }
                else {
                    while (node && !node->next) {
                        node = node->parent;
                    }
                    if (node) {
                        node = node->next;
                        while (node->firstChild) {
                            node = node->firstChild;
                        }
                    }
                    else {
                        node = nullptr;
                    }
                }
            }
            return node;
        };	// goes through all leaf nodes of the tree;
        static _iterType*
            GetPrevLeaf(_iterType* node) {
            if (!node) return nullptr;
            if (node->lastChild) {
                while (node->lastChild) {
                    node = node->lastChild;
                }
                return node;
            }
            else {
                while (node && node->prev == nullptr) {
                    node = node->parent;
                }
                if (node) {
                    node = node->prev;
                    while (node->lastChild) {
                        node = node->lastChild;
                    }
                    return node;
                }
                else {
                    return nullptr;
                }
            }
        };	// goes through all leaf nodes of the tree;
        static _iterType*
            GetNext(_iterType* node) {
            if (node) {
                if (node->firstChild) {
                    node = node->firstChild;
                }
                else {
                    while (node && node->next == nullptr) {
                        node = node->parent;
                    }
                }
            }
            return node;
        };		// goes through all nodes of the tree;
        static _iterType*
            NodeFind(keyType  const& key, _iterType* root) {
            _iterType* node = NodeFindLargestSmallerEqual(key, root);
            if (node && node->object && node->key == key) return node; // EQUALS
            return nullptr;
        };								// find an object using the given key;
        static _iterType*
            NodeFindByIndex(int index, _iterType* Root) {
            int startIndex{ 0 };

            if (Root == nullptr) {
                return nullptr;
            }

            while (Root) {
                if (index == startIndex && Root->object) { return Root; }

                if (startIndex <= index && (startIndex + Root->numChildren) > index) {
                    // one of my children has this index				
                    Root = Root->firstChild;
                }
                else {
                    // one of my neighbors has this index				
                    if (Root->object) ++startIndex;
                    else startIndex += Root->numChildren;

                    Root = Root->next;
                }
            }

            return Root;
        };			// find an object with the largest key smaller equal the given key;
        static _iterType*
            NodeFindSmallestLargerEqual(keyType const& key, _iterType* Root) {
            _iterType* node, * smaller;

            if (Root == nullptr) {
                return nullptr;
            }

            smaller = nullptr;
            for (node = Root->lastChild; node != nullptr; node = node->lastChild) {
                while (node->prev) {
                    if (node->key <= key) {
                        if (!smaller) {
                            smaller = GetPrevLeaf(Root);
                        }
                        break;
                    }
                    smaller = node;
                    node = node->prev;
                }
                if (node->object) {
                    if (node->key >= key) {
                        break;
                    }
                    else if (smaller == nullptr) {
                        return nullptr;
                    }
                    else {
                        node = smaller;
                        if (node->object) {
                            break;
                        }
                    }
                }
            }

            return node;
        };			// find an object with the smallest key larger equal the given key;
        static _iterType*
            NodeFindLargestSmallerEqual(keyType const& key, _iterType* Root) {
            _iterType* node, * smaller;

            if (Root == nullptr) {
                return nullptr;
            }

            smaller = nullptr;
            for (node = Root->firstChild; node != nullptr; node = node->firstChild) {
                while (node->next) {
                    if (node->key >= key) {
                        if (!smaller) {
                            smaller = GetNextLeaf(Root);
                        }
                        break;
                    }
                    smaller = node;
                    node = node->next;
                }
                if (node->object) {
                    if (node->key <= key) {
                        break;
                    }
                    else if (smaller == nullptr) {
                        return nullptr;
                    }
                    else {
                        node = smaller;
                        if (node->object) {
                            break;
                        }
                    }
                }
            }
            return node;
        };			// find an object with the largest key smaller equal the given key;

    public:
        using GuardType = typename EpochGuard;
        EpochGuard ProtectCurrentEpoch() const { return EpochGuard(this); };

        atomic_btree()
            : Num(0)
            , root(nullptr)
            , first(nullptr)
            , last(nullptr)
            , objAllocator()
            , nodeAllocator()
            , mutex()
        {
            static_assert(maxChildrenPerNode >= 4);
            root = AllocNode();
        };
        atomic_btree(atomic_btree const&) = delete;
        atomic_btree(atomic_btree&& rhs) = delete;
        atomic_btree& operator=(atomic_btree const&) = delete;
        atomic_btree& operator=(atomic_btree&&) = delete;
        ~atomic_btree() = default;

        void unsafe_unload() {
            if (objAllocator) objAllocator->unsafe_unload();
            nodeAllocator.unsafe_unload();
            root = first = last = nullptr;
            Num = 0;
        };

        template <bool EmplaceIfExists = true> _iterType*
            Add(objType object, keyType const& key) {
            _iterType
                * node,
                * child,
                * newNode;

            // check that the key does not already exist		
            if constexpr (EmplaceIfExists) {
                auto locked{ std::shared_lock(mutex) };
                node = NodeFind(key, root);
                if (node && node->object) {
                    *node->object = std::move(object);
                    return node;
                }
            }
            else {
                auto locked{ std::shared_lock(mutex) };
                node = NodeFind(key, root);
                if (node && node->object) {
                    return node;
                }
            }

            auto locked{ std::scoped_lock(mutex) };

            // check that the key does not already exist		
            if constexpr (EmplaceIfExists) {
                node = NodeFind(key, root);
                if (node && node->object) {
                    *node->object = std::move(object);
                    return node;
                }
            }
            else {
                node = NodeFind(key, root);
                if (node && node->object) {
                    return node;
                }
            }

            newNode = AllocNode();
            newNode->key = key;
            newNode->object = objAllocator->Alloc(std::move(object));
            Num++;

            if (root->numChildren >= maxChildrenPerNode) {
                // DOING MODIFICATIONS
                if (1) {
                    node = AllocNode();
                    node->key = root->key;
                    node->firstChild = root;
                    node->lastChild = root;
                    node->numChildren = 1;
                    root->parent = node;
                    SplitNode(root);
                    root = node;
                }
            };

            for (node = root; node->firstChild; node = child) {
                if (key > node->key) node->key = key;

                // find the first child with a key larger equal to the key of the new node
                for (child = node->firstChild; child->next; child = child->next)
                    if (key <= child->key)
                        break;

                if (child->object) {
                    // DOING MODIFICATIONS
                    if (1) {
                        if (key <= child->key) {
                            // insert new node before child
                            if (child->prev) child->prev->next = newNode;
                            else node->firstChild = newNode;
                            newNode->prev = child->prev;
                            newNode->next = child;
                            child->prev = newNode;
                        }
                        else {
                            // insert new node after child
                            if (child->next) child->next->prev = newNode;
                            else node->lastChild = newNode;
                            newNode->prev = child;
                            newNode->next = child->next;
                            child->next = newNode;
                        }
                        newNode->parent = node;
                        ++node->numChildren;
                        return CheckLastNode(CheckFirstNode(newNode));
                    }
                }

                // make sure the child has room to store another node
                if (child->numChildren >= maxChildrenPerNode) {
                    // DOING MODIFICATIONS
                    if (1) {
                        SplitNode(child);
                        if (key <= child->prev->key)
                            child = child->prev;
                    }
                }
            }

            // DOING MODIFICATIONS
            if (1) {
                // we only end up here if the root node is empty
                newNode->parent = root;
                root->key = key;
                root->firstChild = newNode;
                root->lastChild = newNode;
                ++root->numChildren;
                return CheckLastNode(CheckFirstNode(newNode));
            }
        };
        __declspec(noinline) _iterType*
            GetOrInstance(keyType const& key) {
            _iterType
                * node,
                * child,
                * newNode;

            // check that the key does not already exist		
            if (1) {
                auto locked{ std::shared_lock(mutex) };
                node = NodeFind(key, root);
                if (node && node->object) {
                    return node;
                }
            }

            auto locked{ std::scoped_lock(mutex) };

            // check that the key does not already exist		
            if (1) {
                node = NodeFind(key, root);
                if (node && node->object) {
                    return node;
                }
            }

            newNode = AllocNode();
            newNode->key = key;
            newNode->object = objAllocator->Alloc();
            Num++;

            if (root->numChildren >= maxChildrenPerNode) {
                // DOING MODIFICATIONS
                if (1) {
                    node = AllocNode();
                    node->key = root->key;
                    node->firstChild = root;
                    node->lastChild = root;
                    node->numChildren = 1;
                    root->parent = node;
                    SplitNode(root);
                    root = node;
                }
            };

            for (node = root; node->firstChild; node = child) {
                if (key > node->key) node->key = key;

                // find the first child with a key larger equal to the key of the new node
                for (child = node->firstChild; child->next; child = child->next)
                    if (key <= child->key)
                        break;

                if (child->object) {
                    // DOING MODIFICATIONS
                    if (1) {
                        if (key <= child->key) {
                            // insert new node before child
                            if (child->prev) child->prev->next = newNode;
                            else node->firstChild = newNode;
                            newNode->prev = child->prev;
                            newNode->next = child;
                            child->prev = newNode;
                        }
                        else {
                            // insert new node after child
                            if (child->next) child->next->prev = newNode;
                            else node->lastChild = newNode;
                            newNode->prev = child;
                            newNode->next = child->next;
                            child->next = newNode;
                        }
                        newNode->parent = node;
                        ++node->numChildren;
                        return CheckLastNode(CheckFirstNode(newNode));
                    }
                }

                // make sure the child has room to store another node
                if (child->numChildren >= maxChildrenPerNode) {
                    // DOING MODIFICATIONS
                    if (1) {
                        SplitNode(child);
                        if (key <= child->prev->key)
                            child = child->prev;
                    }
                }
            }

            // DOING MODIFICATIONS
            if (1) {
                // we only end up here if the root node is empty
                newNode->parent = root;
                root->key = key;
                root->firstChild = newNode;
                root->lastChild = newNode;
                ++root->numChildren;
                return CheckLastNode(CheckFirstNode(newNode));
            }
        };
        template <typename iter_type, bool EmplaceIfExists = true> void
            Add_Bulk(iter_type begin, iter_type const& end) {
            _iterType
                * node,
                * child,
                * newNode;

            auto locked{ std::scoped_lock(mutex) };
            for (; begin != end; begin++) {
                // check that the key does not already exist		
                if constexpr (EmplaceIfExists) {
                    node = NodeFind(begin->first, root);
                    if (node && node->object) {
                        *node->object = begin->second;
                        continue;
                    }
                }

                newNode = AllocNode();
                newNode->key = begin->first;
                newNode->object = objAllocator->Alloc(begin->second);
                Num++;

                if (root->numChildren >= maxChildrenPerNode) {
                    // DOING MODIFICATIONS
                    if (1) {
                        node = AllocNode();
                        node->key = root->key;
                        node->firstChild = root;
                        node->lastChild = root;
                        node->numChildren = 1;
                        root->parent = node;
                        SplitNode(root);
                        root = node;
                    }
                };

                bool should_continue = false;
                for (node = root; node->firstChild; node = child) {
                    if (begin->first > node->key) node->key = begin->first;

                    // find the first child with a key larger equal to the key of the new node
                    for (child = node->firstChild; child->next; child = child->next)
                        if (begin->first <= child->key)
                            break;

                    if (child->object) {
                        // DOING MODIFICATIONS
                        if (1) {
                            if (begin->first <= child->key) {
                                // insert new node before child
                                if (child->prev) child->prev->next = newNode;
                                else node->firstChild = newNode;
                                newNode->prev = child->prev;
                                newNode->next = child;
                                child->prev = newNode;
                            }
                            else {
                                // insert new node after child
                                if (child->next) child->next->prev = newNode;
                                else node->lastChild = newNode;
                                newNode->prev = child;
                                newNode->next = child->next;
                                child->next = newNode;
                            }
                            newNode->parent = node;
                            ++node->numChildren;
                            CheckLastNode(CheckFirstNode(newNode));

                            should_continue = true;
                            break;
                        }
                    }

                    // make sure the child has room to store another node
                    if (child->numChildren >= maxChildrenPerNode) {
                        // DOING MODIFICATIONS
                        if (1) {
                            SplitNode(child);
                            if (begin->first <= child->prev->key)
                                child = child->prev;
                        }
                    }
                }
                if (should_continue) continue;

                // DOING MODIFICATIONS
                if (1) {
                    // we only end up here if the root node is empty
                    newNode->parent = root;
                    root->key = begin->first;
                    root->firstChild = newNode;
                    root->lastChild = newNode;
                    ++root->numChildren;
                    CheckLastNode(CheckFirstNode(newNode));

                    continue;
                }
            }
        };
        auto // guard-lock the tree							
            Lock() {
            return std::unique_lock(this->mutex);
        };
        bool // remove an object node from the tree								
            Remove_Unsafe(_iterType* node, objType* object_copy) {
            _iterType
                * parent,
                * oldRoot{ nullptr };

            if (!node) return false;
            else {
                auto g{ this->nodeAllocator.ProtectCurrentEpoch() };

                if (first == node)
                    first = this->GetNextLeaf(node);
                if (last == node)
                    last = this->GetPrevLeaf(node);

                // unlink the node from it's parent
                if (node->prev)
                    node->prev->next = node->next;
                else
                    node->parent->firstChild = node->next;
                if (node->next)
                    node->next->prev = node->prev;
                else
                    node->parent->lastChild = node->prev;
                node->parent->numChildren--;

                // make sure there are no parent nodes with a single child
                for (parent = node->parent; parent != root && parent->numChildren <= 1; parent = parent->parent) {
                    if (parent->next)
                        parent = MergeNodes(parent, parent->next);
                    else if (parent->prev)
                        parent = MergeNodes(parent->prev, parent);

                    // a parent may not use a key higher than the key of it's last child
                    if (parent->key > parent->lastChild->key)
                        parent->key = parent->lastChild->key;

                    if (parent->numChildren > maxChildrenPerNode) {
                        SplitNode(parent);
                        break;
                    }
                }
                for (; parent && parent->lastChild; parent = parent->parent)
                    // a parent may not use a key higher than the key of it's last child
                    if (parent->key > parent->lastChild->key)
                        parent->key = parent->lastChild->key;

                // remove the root node if it has a single internal node as child
                if (root->numChildren == 1 && root->firstChild->object == nullptr) {
                    oldRoot = root;
                    root->firstChild->parent = nullptr;
                    root = root->firstChild;
                }
            }

            // free the nodes
            if constexpr (std::is_copy_assignable< objType >::value) {
                if (object_copy) *object_copy = *node->object;
            }
            FreeNode(node);
            if (oldRoot) FreeNode(oldRoot);

            return true;
        };
        bool // remove an object node from the tree								
            Remove(_iterType* node) {
            auto locked{ std::scoped_lock(this->mutex) };
            return Remove_Unsafe(node, nullptr);
        };
        bool // remove an object node from the tree								
            RemoveAt(keyType const& key, objType* object_copy = nullptr) {
            auto locked{ std::scoped_lock(this->mutex) };
            if (auto* p = this->NodeFind(key, root)) {
                return Remove_Unsafe(p, object_copy);
            }
            return false;
        };
        _iterType*
            NodeFindByIndex(int index) const {
            if (index <= 0) return GetFirst();
            else if (index >= (Num - 1)) return GetLast();
            else {
                auto locked{ std::shared_lock(mutex) };
                return NodeFindByIndex(index, root);
            }
        };
        _iterType*
            NodeFind(keyType  const& key) const {
            auto locked{ std::shared_lock(mutex) };
            return NodeFind(key, root);
        };								// find an object using the given key;
        _iterType* // find an object with the smallest key larger equal the given key;
            NodeFindSmallestLargerEqual(keyType const& key) const {
            auto locked{ std::shared_lock(mutex) };
            return NodeFindSmallestLargerEqual(key, root);
        };
        _iterType* // find an object with the largest key smaller equal the given key;
            NodeFindLargestSmallerEqual(keyType const& key) const {
            auto locked{ std::shared_lock(mutex) };
            return NodeFindLargestSmallerEqual(key, root);
        };
        objType* // find an object using the given key;
            Find(keyType  const& key) const {
            auto locked{ std::shared_lock(mutex) };
            _iterType* node = NodeFind(key, root);
            if (node) return node->object;
            else return nullptr;
        };
        objType* // find an object with the smallest key larger equal the given key;
            FindSmallestLargerEqual(keyType const& key) const {
            auto locked{ std::shared_lock(mutex) };
            _iterType* node = NodeFindSmallestLargerEqual(key, root);
            if (node == nullptr) {
                return nullptr;
            }
            else {
                return node->object;
            }
        };
        objType* // find an object with the largest key smaller equal the given key;
            FindLargestSmallerEqual(keyType const& key) const {
            auto locked{ std::shared_lock(mutex) };
            _iterType* node = NodeFindLargestSmallerEqual(key, root);
            if (node == nullptr) {
                return nullptr;
            }
            else {
                return node->object;
            }
        };
        _iterType*
            GetFirst_Unsafe() const {
            return first;
        };
        _iterType*
            GetLast_Unsafe() const {
            return last;
        };
        _iterType*
            GetFirst() const {
            auto locked{ std::shared_lock(mutex) };
            return first;
        };
        _iterType*
            GetLast() const {
            auto locked{ std::shared_lock(mutex) };
            return last;
        };
        _iterType*
            GetRoot() const {
            auto locked{ std::shared_lock(mutex) };
            return root;
        };
        long long // returns the total number of nodes in the tree;							
            GetNodeCount() const {
            return Num.load();
        };

    private:
        _iterType*
            CheckFirstNode(_iterType* newNode) {
            if (newNode) {
                if (!first || (first->key > newNode->key)) {
                    first = newNode;
                }
            }
            return newNode;
        };
        _iterType*
            CheckLastNode(_iterType* newNode) {
            if (newNode) {
                if (!last || (last->key < newNode->key)) {
                    last = newNode;
                }
            }
            return newNode;
        };
        _iterType*
            AllocNode() {
            _iterType* node;
            node = nodeAllocator.Alloc();
            return InitNode(node);
        };
        void
            FreeNode(_iterType* node) {
            if (node) {
                if (node->object) {
                    objAllocator->Free(node->object);
                    Num--;
                }
                nodeAllocator.Free(node);
            }
        };
        void
            SplitNode(_iterType* node) {
            long long
                i;
            _iterType
                * child,
                * newNode;

            // allocate a new node
            newNode = AllocNode();
            newNode->parent = node->parent;

            // divide the children over the two nodes
            child = node->firstChild;
            child->parent = newNode;
            for (i = 3; i < node->numChildren; i += 2) {
                child = child->next;
                child->parent = newNode;
            }

            newNode->key = child->key;
            newNode->numChildren = node->numChildren / 2;
            newNode->firstChild = node->firstChild;
            newNode->lastChild = child;

            node->numChildren -= newNode->numChildren;
            node->firstChild = child->next;

            child->next->prev = nullptr;
            child->next = nullptr;

            if (node->prev) node->prev->next = newNode;
            else node->parent->firstChild = newNode;

            newNode->prev = node->prev;
            newNode->next = node;
            node->prev = newNode;

            node->parent->numChildren++;
        };
        _iterType*
            MergeNodes(_iterType* node1, _iterType* node2) {
            _iterType* child;

            for (child = node1->firstChild; child->next; child = child->next) child->parent = node2;
            child->parent = node2;
            child->next = node2->firstChild;
            node2->firstChild->prev = child;
            node2->firstChild = node1->firstChild;
            node2->numChildren += node1->numChildren;

            // unlink the first node from the parent
            if (node1->prev) node1->prev->next = node2;
            else node1->parent->firstChild = node2;

            node2->prev = node1->prev;
            node2->parent->numChildren--;

            FreeNode(node1);

            return node2;
        };

    };

    // fast, thread-safe sorted map. Allows simultaneous reading / writing / erasure. Slower than concurrent_unordered_map when erasure is not necessary. 
    template<class KeyType, class ValueType> class atomic_map {
        friend class it_state;
    protected:
        deferred<atomic_btree<ValueType, KeyType>>
            tree;

    public:
        class WrappedReference {
        private:
            typename atomic_btree<ValueType, KeyType>::GuardType
                guard;

        public:
            const KeyType&
                first;
            ValueType&
                second;

            // WrappedReference() = delete;
            WrappedReference(const KeyType& _first, ValueType& _second, atomic_btree<ValueType, KeyType>* _parent)
                : first{ _first }
                , second{ _second }
                , guard{ _parent->ProtectCurrentEpoch() }
            {};
            WrappedReference(WrappedReference const&) = delete;
            WrappedReference(WrappedReference&&) = delete;
            WrappedReference& operator=(WrappedReference const&) = delete;
            WrappedReference& operator=(WrappedReference&&) = delete;
            ~WrappedReference() = default;
        };

    public:
        atomic_map()
            : tree{}
        {};
        atomic_map(atomic_map const& rhs) = delete;
        atomic_map(atomic_map&& rhs) = delete;
        atomic_map& operator=(atomic_map const& rhs) = delete;
        atomic_map& operator=(atomic_map&& rhs) = delete;
        ~atomic_map() = default;

        void unsafe_unload() {
            tree->unsafe_unload();
        };
        auto // Protect future member function calls from deleting node or object pointers until some time after this object expires.
            ProtectCurrentEpoch() const {
            return tree->ProtectCurrentEpoch();
        };
        size_t // returns the current number of objects in the container. Thread-safe, but out-of-date immediately after the call is made. 
            size() const {
            return tree->GetNodeCount();
        };
        WrappedReference // if already exists, returns the existing value pair. 
            insert(const KeyType& time, ValueType&& value) {
            auto g{ tree->ProtectCurrentEpoch() };
            auto* iter = tree->Add<false>(std::move(value), time);
            return WrappedReference(iter->key, *iter->object, &*tree);
        };
        WrappedReference // if already exists, overwrites the value and returns the value pair. 
            emplace(const KeyType& time, ValueType&& value) {
            auto g{ tree->ProtectCurrentEpoch() };
            auto* iter = tree->Add<true>(std::move(value), time);
            return WrappedReference(iter->key, *iter->object, &*tree);
        };
        void // if already exists, does nothing
            insert_fast(const KeyType& time, ValueType&& value) {
            (void)tree->Add<false>(std::move(value), time);
        };
        void // if already exists, overwrites the value. 
            emplace_fast(const KeyType& time, ValueType&& value) {
            (void)tree->Add<true>(std::move(value), time);
        };
        template <typename iter_type> void // bulk insertion. if already exists, does nothing.
            insert_bulk(iter_type begin, iter_type const& end) {
            auto g{ tree->ProtectCurrentEpoch() };
            tree->Add_Bulk<iter_type, false>(std::move(begin), end);
        };
        template <typename iter_type> void // bulk insertion. if already exists, overwrites the value. 
            emplace_bulk(iter_type begin, iter_type const& end) {
            auto g{ tree->ProtectCurrentEpoch() };
            tree->Add_Bulk<iter_type, true>(std::move(begin), end);
        };
        size_t // returns 1 if the key is found, otherwise 0.
            count(const KeyType& time) const {
            return (bool)tree->NodeFind(time) ? 1 : 0;
        };
        ValueType& // throws if the key is not found. 
            at(const KeyType& time) const {
            auto g{ tree->ProtectCurrentEpoch() };
            if (auto* iter = tree->NodeFind(time)) {
                return *iter->object;
            }
            else {
                throw std::range_error("Could not find key");
            }
        };
        ValueType* // returns nullptr if the key is not found. 
            try_at(const KeyType& time) const {
            auto g{ tree->ProtectCurrentEpoch() };
            if (auto* iter = tree->NodeFind(time)) {
                return iter->object;
            }
            else {
                return nullptr;
            }
        };
        template <typename Func> __declspec(noinline) bool // calls func(key, object) on the first (smallest key) node in the map
            do_at_beginning(Func const& func) const {
            auto g{ tree->ProtectCurrentEpoch() };
            if (auto* p = tree->GetFirst()) {
                func(p->key, *p->object);
                return true;
            }
            return false;
        };
        template <typename Func> __declspec(noinline) void // calls func(key, object) on all nodes in the map
            for_all(Func const& func) const {
            auto g{ tree->ProtectCurrentEpoch() };
            auto p = tree->GetFirst();
            while (p) {
                func(p->key, *p->object);
                p = tree->GetNextLeaf(p);
            }
        };
        template <typename Func> __declspec(noinline) bool // calls func(key, object) on the last (largest key) node in the map
            do_at_end(Func const& func) const {
            auto g{ tree->ProtectCurrentEpoch() };
            if (auto* p = tree->GetLast()) {
                func(p->key, *p->object);
                return true;
            }
            return false;
        };
        bool // removes the first (smallest key) node in the map
            pop_front() {
            auto g{ tree->ProtectCurrentEpoch() };
            if (auto* p = tree->GetFirst()) {
                tree->Remove(p);
                return true;
            }
            return false;
        };
        bool // removes the last (largest key) node in the map
            pop_back() {
            auto g{ tree->ProtectCurrentEpoch() };
            if (auto* p = tree->GetLast()) {
                tree->Remove(p);
                return true;
            }
            return false;
        };
        template <typename Func> __declspec(noinline) bool // removes the first (smallest key) node in the map if func(key, object) returns true
            pop_front_if(Func const& func) {
            bool out = false;
            auto g{ tree->ProtectCurrentEpoch() };
            auto g2{ tree->Lock() };
            if (auto* p = tree->GetFirst_Unsafe()) {
                if (func(p->key, *p->object)) {
                    tree->Remove_Unsafe(p, nullptr);
                    out = true;
                }
            }
            return out;
        };
        template <typename Func> __declspec(noinline) bool // removes the last (largest key) node in the map if func(key, object) returns true
            pop_back_if(Func const& func) {
            bool out = false;
            auto g{ tree->ProtectCurrentEpoch() };
            auto g2{ tree->Lock() };
            if (auto* p = tree->GetLast_Unsafe()) {
                if (func(p->key, *p->object)) {
                    tree->Remove_Unsafe(p, nullptr);
                    out = true;
                }
            }
            return out;
        };
        __declspec(noinline) ValueType& // if already exists, returns the value. Otherwise, creates the value (default init) and returns the value. May throw under heavy conflict. 
            operator[](const KeyType& time) {
            auto g{ tree->ProtectCurrentEpoch() };
            if (ValueType* p = tree->Find(time)) {
                return *p;
            }
            else {
                if (auto* p = tree->GetOrInstance(time)) {
                    return *p->object;
                }
                else {
                    throw std::range_error("Could not find key");
                }
            }
        };
        template <typename Func> ValueType& // same as operator[], except it will call the provided function to initialize the value if no value was found. 
            get_or_make(const KeyType& time, Func const& func, bool* ExistedAlready = nullptr) {
            auto g{ tree->ProtectCurrentEpoch() };
            if (ValueType* p = tree->Find(time)) {
                if (ExistedAlready) *ExistedAlready = true;
                return *p;
            }
            else {
                if (ExistedAlready) *ExistedAlready = false;
                if (auto* p = tree->Add(func(), time)) {
                    return *p->object;
                }
                else {
                    throw std::range_error("Could not find key");
                }
            }
        };
        bool // erase the value pair at the specified key. Optionally, can copy the value at the key before erasure.
            erase(const KeyType& time, ValueType* out = nullptr) {
            auto g{ tree->ProtectCurrentEpoch() };
            return tree->RemoveAt(time, out);
        };
        void // clear the map
            clear() {
            while (pop_front()) {}
        };
    private:
        class it_state {
        public:
            using thisType = atomic_map;
            using value_type = WrappedReference;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = ptrdiff_t;

            // data
            mutable typename atomic_btree<ValueType, KeyType>::_iterType*
                _ptr{};
            mutable std::unique_ptr<value_type>
                _out;

            // functions
            void Initialize(thisType* ref) {};
            void ToBeginning(thisType* ref) {
                _ptr = ref->tree->GetFirst();
            };
            void ToEnd(thisType* ref) {
                _ptr = nullptr;
            };
            void Next(thisType* ref) {
                this->_ptr = ref->tree->GetNextLeaf(this->_ptr);
            };
            void Prev(thisType* ref) {
                this->_ptr = ref->tree->GetPrevLeaf(this->_ptr);
            };
            value_type& Get(thisType* ref) const {
                _out = std::make_unique<value_type>(_ptr->key, *_ptr->object, &*ref->tree);
                return *_out;
            };
            bool operator==(it_state const& rhs) const {
                return _ptr == rhs._ptr;
            };
            difference_type Distance(it_state const& other) const {
                return _ptr - other._ptr;
            };
        };

    public:
        SETUP_ITERATOR(atomic_map, it_state);
        iterator // returns an iterator 
            find(const KeyType& _Keyval) const {
            auto g{ tree->ProtectCurrentEpoch() };
            auto iter = this->end();
            if (auto* p = this->tree->NodeFind(_Keyval)) {
                iter.state._ptr = p;
            }
            return iter;
        };

    };

    // fast, thread-safe sorted map, which sorts on key hash values rather than keys themselves. Allows simultaneous reading / writing / erasure. Slower than concurrent_unordered_map when erasure is not necessary. 
    template<class KeyType, class ValueType, typename HashType = std::hash<KeyType>> class atomic_hash_map {
        friend class it_state;
    protected:
        std::unique_ptr< deferred<atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>> >
            tree;
        HashType
            hasher;
        size_t
            hash(KeyType const& k) const {
            return hasher(k);
        };

    public:
        class WrappedReference {
        private:
            typename  atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>::GuardType
                guard;

        public:
            const KeyType&
                first;
            ValueType&
                second;

            WrappedReference(const KeyType& _first, ValueType& _second, atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>* _parent)
                : first{ _first }
                , second{ _second }
                , guard{ _parent->ProtectCurrentEpoch() }
            {};
            WrappedReference(WrappedReference const&) = delete;
            WrappedReference(WrappedReference&&) = delete;
            WrappedReference& operator=(WrappedReference const&) = delete;
            WrappedReference& operator=(WrappedReference&&) = delete;
            ~WrappedReference() = default;
        };

    public:
        atomic_hash_map()
            : tree{ std::make_unique< deferred<atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>>>() }, hasher{ HashType{} }
        {};
        atomic_hash_map(atomic_hash_map const& rhs) = delete;
        atomic_hash_map(atomic_hash_map&& rhs) : tree{ std::move(rhs.tree) }, hasher{ HashType{} }
        {};
        atomic_hash_map& operator=(atomic_hash_map const& rhs) = delete;
        atomic_hash_map& operator=(atomic_hash_map&& rhs) = delete;
        ~atomic_hash_map() = default;

        void unsafe_unload() {
            if (*tree) tree->operator*().unsafe_unload();
        };
        auto // Protect future member function calls from deleting node or object pointers until some time after this object expires.
            ProtectCurrentEpoch() const {
            return tree->operator*().ProtectCurrentEpoch();
        };
        size_t // returns the current number of objects in the container. Thread-safe, but out-of-date immediately after the call is made. 
            size() const {
            return tree->operator*().GetNodeCount();
        };
        WrappedReference // if already exists, returns the existing value pair. 
            insert(const KeyType& time, ValueType&& value) {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            auto* iter = tree->operator*().Add<false>({ time, std::make_shared<ValueType>(std::move(value)) }, hash(time));
            return WrappedReference(iter->object->first, *iter->object->second, &**tree);
        };
        WrappedReference // if already exists, overwrites the value and returns the value pair. 
            emplace(const KeyType& time, ValueType&& value) {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            auto* iter = tree->operator*().Add<true>({ time, std::make_shared<ValueType>(std::move(value)) }, hash(time));
            return WrappedReference(iter->object->first, *iter->object->second, &**tree);
        };
        void // if already exists, returns the existing value pair. 
            insert_fast(const KeyType& time, ValueType&& value) {
            (void)tree->operator*().Add<false>({ time, std::make_shared<ValueType>(std::move(value)) }, hash(time));
        };
        void // if already exists, overwrites the value and returns the value pair. 
            emplace_fast(const KeyType& time, ValueType&& value) {
            (void)tree->operator*().Add<true>({ time, std::make_shared<ValueType>(std::move(value)) }, hash(time));
        };
        size_t // returns 1 if the key is found, otherwise 0.
            count(const KeyType& time) const {
            return (bool)tree->operator*().NodeFind(hash(time)) ? 1 : 0;
        };
        ValueType& // throws if the key is not found. 
            at(const KeyType& time) const {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            if (auto* iter = tree->operator*().NodeFind(hash(time))) {
                return *iter->object->second;
            }
            else {
                throw std::range_error("Could not find key");
            }
        };
        ValueType* // returns nullptr if the key is not found. 
            try_at(const KeyType& time) const {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            if (auto* iter = tree->operator*().NodeFind(hash(time))) {
                return &*iter->object->second;
            }
            else {
                return nullptr;
            }
        };
        template <typename Func> __declspec(noinline) void // calls func(key, object) on all nodes in the map
            for_all(Func const& func) const {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            auto p = tree->operator*().GetFirst();
            while (p) {
                func(p->object->first, *p->object->second);
                p = tree->operator*().GetNextLeaf(p);
            }
        };
        template <typename Func> ValueType& // same as operator[], except it will call the provided function to initialize the value if no value was found. 
            get_or_make(const KeyType& time, Func const& func, bool* ExistedAlready = nullptr) {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            if (std::pair<KeyType, std::shared_ptr<ValueType>>* p = tree->operator*().Find(hash(time))) {
                if (ExistedAlready) *ExistedAlready = true;
                return *p->second;
            }
            else {
                if (ExistedAlready) *ExistedAlready = false;
                if (auto* p = tree->operator*().Add<false>(std::pair<KeyType, std::shared_ptr<ValueType>>(time, std::make_shared<ValueType>(func())), hash(time))) {
                    return *p->object->second;
                }
                else {
                    throw std::range_error("Could not find key");
                }
            }
        };
        __declspec(noinline) ValueType& // if already exists, returns the value. Otherwise, creates the value (default init) and returns the value. May throw under heavy conflict. 
            operator[](const KeyType& time) {
            return get_or_make(time, []() -> ValueType { return ValueType(); }, nullptr);
        };

        bool // erase the value pair at the specified key. Optionally, can copy the value at the key before erasure.
            erase(const KeyType& time, ValueType* out = nullptr) {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            std::pair<KeyType, std::shared_ptr<ValueType>> temp;
            bool result = tree->operator*().RemoveAt(hash(time), &temp);
            if (out) *out = *temp.second;
            return result;
        };
        void // clear the map
            clear() {
            while (true) {
                auto g{ tree->operator*().ProtectCurrentEpoch() };
                if (auto* p = tree->operator*().GetFirst()) {
                    tree->operator*().Remove(p);
                }
                else {
                    break;
                }
            }
        };

    private:
        class it_state {
        public:
            using thisType = atomic_hash_map;
            using value_type = WrappedReference;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

            // data
            mutable typename  atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>::_iterType*
                _ptr{};
            mutable std::unique_ptr<value_type>
                _out;

            // functions
            void Initialize(thisType* ref) {};
            void ToBeginning(thisType* ref) {
                _ptr = ref->tree->operator*().GetFirst();
            };
            void ToEnd(thisType* ref) {
                _ptr = nullptr;
            };
            void Next(thisType* ref) {
                this->_ptr = ref->tree->operator*().GetNextLeaf(this->_ptr);
            };
            void Prev(thisType* ref) {
                this->_ptr = ref->tree->operator*().GetPrevLeaf(this->_ptr);
            };
            value_type& Get(thisType* ref) const {
                _out = std::make_unique<value_type>(_ptr->object->first, *_ptr->object->second, &**ref->tree);
                return *_out;
            };
            bool operator==(it_state const& rhs) const {
                return _ptr == rhs._ptr;
            };
            difference_type Distance(it_state const& other) const {
                return _ptr - other._ptr;
            };
        };

    public:
        SETUP_ITERATOR(atomic_hash_map, it_state);
        iterator // returns an iterator 
            find(const KeyType& _Keyval) const {
            auto g{ tree->operator*().ProtectCurrentEpoch() };
            auto iter = this->end();
            if (auto* p = this->tree->operator*().NodeFind(hash(_Keyval))) {
                iter.state._ptr = p;
            }
            return iter;
        };

    };

};

// Atomic Numbers
namespace GL {
    // atomic variant of double. Keeps the final bit equal to 0 to support tags. 100x slower than using a normal double, so do not use this unless necessary. 
    class atomic_double {
    protected:
        static  double _abs(double val) {
            return val >= (double)0 ? val : -val;
        }
        static  double _floor(double val) {
            // casting to int truncates the value, which is floor(val) for positive values,
            // but we have to substract 1 for negative values (unless val is already floored == recasted int val)
            const auto val_int = (int64_t)val;
            const double fval_int = (double)val_int;
            return (val >= (double)0 ? fval_int : (val == fval_int ? val : fval_int - (double)1));
        };
        
        // floating_point to integral conversion: approximation full of magic numbers. 
        // assumes the structure of the double is MANTISSA, EXPONENT, SIGN. 
        // and assumes that the exponent can be reduced by one bit, the sign can be moved over, and the final bit can be cleared, reserved for CAS swaps.
        static  uint64_t pack_fast(double value) {
            if (value == 0) return 0;
            struct tempContainer { short value : 10; };
            uint64_t toReturn = (*(uint64_t*)(void*)&value << (64 - (DBL_MANT_DIG - 1))) >> (64 - (DBL_MANT_DIG - 1));
            uint64_t exponent_literal{ *(uint64_t*)(void*)&value >> (DBL_MANT_DIG - 1) };
            auto exponent_signed{ tempContainer{ static_cast<short>(static_cast<long long>(exponent_literal) - 1023ll) } };
            exponent_signed.value += 50;
            return (toReturn | ((*(uint64_t*)(void*)&exponent_signed) << (DBL_MANT_DIG - 1))) | (((*(uint64_t*)(void*)&value) >> 63) << 62);
        };
        static  double unpack_fast(uint64_t value) {
            if (value == 0) return 0;
            uint64_t toReturn{ (value << (64 - (DBL_MANT_DIG - 1))) >> (64 - (DBL_MANT_DIG - 1)) };
            uint64_t exponent_signed{ ((*(uint64_t*)(void*)&value) << 2) >> (DBL_MANT_DIG + 1) };
            uint64_t exponent_literal{ static_cast<uint64_t>(static_cast<long long>(exponent_signed) - 50ll + 1023ll) };
            toReturn |= ((exponent_literal << (DBL_MANT_DIG - 1)) | ((((*(uint64_t*)(void*)&value) >> 62) << 63)));
            return *(double*)(void*)&toReturn;
        };

    protected:
        uint64_t representation;

    protected:
        void pack(double a) {
            InterlockedExchange(static_cast<volatile unsigned long long*>(&representation), pack_fast(a));
        };
         double unpack() const {
            return unpack_fast(representation);
        };

    public:
        atomic_double() noexcept : representation{ pack_fast(0) } {};
        template <typename T, typename = std::enable_if_t<(!std::is_same<std::decay_t<T>, atomic_double>::value) && std::is_pod_v<std::decay_t<T>>>>  atomic_double(T const& a) : representation{ pack_fast(static_cast<double>(a)) } {};
        atomic_double(const atomic_double& a) = default;
        atomic_double& operator=(const atomic_double& a) noexcept {
            InterlockedExchange(static_cast<volatile unsigned long long*>(&representation), a.representation);        
            return *this;
        };
         atomic_double(atomic_double&& a) = default;
        atomic_double& operator=(atomic_double&& a) noexcept {
            InterlockedExchange(static_cast<volatile unsigned long long*>(&representation), a.representation);
            return *this;
        };
        ~atomic_double() = default;

    public:
        operator double() const { return load(); };
        atomic_double operator+(atomic_double& b) {
            return atomic_double{ load() + b.load() };
        };
        atomic_double operator-(atomic_double& b) {
            return atomic_double{ load() - b.load() };
        };
        atomic_double operator/(atomic_double& b) {
            return atomic_double{ load() / b.load() };
        };
        atomic_double operator*(atomic_double& b) {
            return atomic_double{ load() * b.load() };
        };

        atomic_double& operator++() {
            (void)update([](double const& x) -> double { return x + 1; });
            return *this;
        };
        atomic_double& operator--() {
            (void)update([](double const& x) -> double { return x - 1; });
            return *this;
        };
        atomic_double operator++(int) { return operator++() - 1; };
        atomic_double operator--(int) { return operator--() + 1; };
        

        atomic_double& operator+=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x + i.load(); });
            return *this;
        };
        atomic_double& operator-=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x - i.load(); });
            return *this;
        };
        atomic_double& operator/=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x / i.load(); });
            return *this;
        };
        atomic_double& operator*=(const atomic_double& i) {
            (void)update([i](double const& x) -> double { return x * i.load(); });
            return *this;
        };

         bool operator==(atomic_double const& b) const {
            return representation == atomic_double(b).representation;
            // return std::abs(load() - (double)b) <= 0.00005l;
        };
         bool operator!=(atomic_double const& b) const { return !operator==(b); };
         bool operator<=(atomic_double const& b) { return load() <= b.load(); };
         bool operator>=(atomic_double const& b) { return load() >= b.load(); };
         bool operator<(atomic_double const& b) { return !operator>=(b); };
         bool operator>(atomic_double const& b) { return !operator<=(b); };

        atomic_double pow(atomic_double const& V) const {
            return atomic_double{ std::pow(load(), V.load()) };
        };
        atomic_double sqrt() const {
            return atomic_double{ std::sqrt(load()) };
        };
         atomic_double abs() const {
            return atomic_double{ _abs(load()) };
        };
         atomic_double floor() const {
            return atomic_double{ _floor(load()) };
        };
         atomic_double ceil() const {
            return atomic_double{ _floor(load() + static_cast<double>(1)) };
        };

    private:
        template<typename Func>
        uint64_t update(Func const& updateFunction) {
            uint64_t prev;
            while (true) {
                prev = representation;
                if (InterlockedCompareExchange(static_cast<volatile unsigned long long*>(&representation), pack_fast(updateFunction(unpack_fast(prev))), prev) == prev) {
                    break;
                }
            }
            return prev;
        }; // returns the previous value while incrementing the actual counter

        template<bool returns = true>
        auto swap(double const& input) {
            if constexpr (!returns) {
                pack(input);
            }
            else {
                auto out{ load() };
                pack(input);
                return double(out);
            }            
        }; // returns the previous value while changing the underlying value
        
    public: // std::atomic compatability
        double fetch_add(double const& v) {
            return unpack_fast(update([&v](double const& from) -> double {
                return from + v;
            }));
        }; // returns the previous value while incrementing the actual counter
        double fetch_sub(double const& v) {
            return unpack_fast(update([&v](double const& from) -> double {
                return from - v;
            }));
        }; // returns the previous value while decrementing the actual counter
        double exchange(double const& v) {
            return swap<true>(v);
        }; // returns the previous value while setting the value to the input
        double load() const {
            return unpack();
        }; // gets the value
        void store(double const& v) {
            swap<false>(v);
            return;
        }; // sets the value to the input

    };

    // atomic variant of float. Keeps the final bit equal to 0 to support tags. 100x slower than using a normal float, so do not use this unless necessary. 
    class atomic_float {
    protected:
        static  float _abs(float val) {
            return val >= (float)0 ? val : -val;
        }
        static  float _floor(float val) {
            // casting to int truncates the value, which is floor(val) for positive values,
            // but we have to substract 1 for negative values (unless val is already floored == recasted int val)
            const auto val_int = (int32_t)val;
            const float fval_int = (float)val_int;
            return (val >= (float)0 ? fval_int : (val == fval_int ? val : fval_int - (float)1));
        };

        // floating_point to integral conversion: approximation full of magic numbers. 
        // assumes the structure of the float is MANTISSA, EXPONENT, SIGN. 
        // and assumes that the exponent can be reduced by one bit, the sign can be moved over, and the final bit can be cleared, reserved for CAS swaps.        
        static uint32_t pack_fast(float value) {
            if (value == 0) return 0;
            struct tempContainer { short value : 7; };
            uint32_t toReturn = (*(uint32_t*)(void*)&value << (32 - (FLT_MANT_DIG - 1))) >> (32 - (FLT_MANT_DIG - 1));
            uint32_t exponent_literal{ *(uint32_t*)(void*)&value >> (FLT_MANT_DIG - 1) };
            tempContainer exponent_signed{ static_cast<short>(static_cast<long long>(exponent_literal) - 128ll) };
            exponent_signed.value += 50;
            return toReturn | (*(uint32_t*)(void*)&exponent_signed << (FLT_MANT_DIG - 1)) | (((*(uint32_t*)(void*)&value >> (32 - 1)) << (32 - 2)));
        };
        static float unpack_fast(uint32_t value) {
            if (value == 0) return 0;
            uint32_t toReturn{ (value << (32 - (FLT_MANT_DIG - 1))) >> (32 - (FLT_MANT_DIG - 1)) };
            uint32_t exponent_signed{ (*(uint32_t*)(void*)&value << 2) >> (FLT_MANT_DIG + 1) };
            uint32_t exponent_literal{ static_cast<uint32_t>(static_cast<long long>(exponent_signed) - 50ll + 128ll) };
            toReturn |= (exponent_literal << (FLT_MANT_DIG - 1)) | ((*(uint32_t*)(void*)&value >> (32 - 2)) << (32 - 1));
            return *(float*)(void*)&toReturn;
        };

    protected:
        uint32_t representation;

    protected:
        void pack(float a) {
            InterlockedExchange(static_cast<volatile uint32_t*>(&representation), pack_fast(a));
        };
        float unpack() const {
            return unpack_fast(representation);
        };

    public:
        atomic_float() noexcept : representation{ pack_fast(0) } {};
        template <typename T, typename = std::enable_if_t<(!std::is_same<std::decay_t<T>, atomic_float>::value) && std::is_pod_v<std::decay_t<T>>>>  atomic_float(T const& a) : representation{ pack_fast(static_cast<float>(a)) } {};
        atomic_float(const atomic_float& a) = default;
        atomic_float& operator=(const atomic_float& a) noexcept {
            InterlockedExchange(static_cast<volatile uint32_t*>(&representation), a.representation);
            return *this;
        };
        atomic_float(atomic_float&& a) = default;
        atomic_float& operator=(atomic_float&& a) noexcept {
            InterlockedExchange(static_cast<volatile uint32_t*>(&representation), a.representation);
            return *this;
        };
        ~atomic_float() = default;

    public:
        operator float() const { return load(); };
        atomic_float operator+(atomic_float& b) {
            return atomic_float{ load() + b.load() };
        };
        atomic_float operator-(atomic_float& b) {
            return atomic_float{ load() - b.load() };
        };
        atomic_float operator/(atomic_float& b) {
            return atomic_float{ load() / b.load() };
        };
        atomic_float operator*(atomic_float& b) {
            return atomic_float{ load() * b.load() };
        };

        atomic_float& operator++() {
            (void)update([](float const& x) -> float { return x + 1; });
            return *this;
        };
        atomic_float& operator--() {
            (void)update([](float const& x) -> float { return x - 1; });
            return *this;
        };
        atomic_float operator++(int) { return operator++() - 1; };
        atomic_float operator--(int) { return operator--() + 1; };


        atomic_float& operator+=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x + i.load(); });
            return *this;
        };
        atomic_float& operator-=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x - i.load(); });
            return *this;
        };
        atomic_float& operator/=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x / i.load(); });
            return *this;
        };
        atomic_float& operator*=(const atomic_float& i) {
            (void)update([i](float const& x) -> float { return x * i.load(); });
            return *this;
        };


        bool operator==(atomic_float const& b) const {
            return representation == atomic_float(b).representation;
            // return std::abs(load() - (float)b) <= 0.00005l;
        };
        bool operator!=(atomic_float const& b) const { return !operator==(b); };
        bool operator<=(atomic_float const& b) { return load() <= b.load(); };
        bool operator>=(atomic_float const& b) { return load() >= b.load(); };
        bool operator<(atomic_float const& b) { return !operator>=(b); };
        bool operator>(atomic_float const& b) { return !operator<=(b); };

        atomic_float pow(atomic_float const& V) const {
            return atomic_float{ std::pow(load(), V.load()) };
        };
        atomic_float sqrt() const {
            return atomic_float{ std::sqrt(load()) };
        };
        atomic_float abs() const {
            return atomic_float{ _abs(load()) };
        };
        atomic_float floor() const {
            return atomic_float{ _floor(load()) };
        };
        atomic_float ceil() const {
            return atomic_float{ _floor(load() + static_cast<float>(1)) };
        };

    private:
        template<typename Func>
        uint32_t update(Func const& updateFunction) {
            uint32_t prev;
            while (true) {
                prev = representation;
                if (InterlockedCompareExchange(static_cast<volatile uint32_t*>(&representation), pack_fast(updateFunction(unpack_fast(prev))), prev) == prev) {
                    break;
                }
            }
            return prev;
        }; // returns the previous value while incrementing the actual counter

        template<bool returns = true>
        auto swap(float const& input) {
            if constexpr (!returns) {
                pack(input);
            }
            else {
                auto out{ load() };
                pack(input);
                return float(out);
            }
        }; // returns the previous value while changing the underlying value

    public: // std::atomic compatability
        float fetch_add(float const& v) {
            return unpack_fast(update([&v](float const& from) -> float {
                return from + v;
            }));
        }; // returns the previous value while incrementing the actual counter
        float fetch_sub(float const& v) {
            return unpack_fast(update([&v](float const& from) -> float {
                return from - v;
            }));
        }; // returns the previous value while decrementing the actual counter
        float exchange(float const& v) {
            return swap<true>(v);
        }; // returns the previous value while setting the value to the input
        float load() const {
            return unpack();
        }; // gets the value
        void store(float const& v) {
            swap<false>(v);
            return;
        }; // sets the value to the input

    };

};


#undef SETUP_ITERATOR