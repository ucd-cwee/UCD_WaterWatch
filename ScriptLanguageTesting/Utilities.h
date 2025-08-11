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
#pragma endregion

namespace GL {
    bool get_thread_alive(size_t thread_id);
    size_t get_thread_id();
    long long get_current_epoch();

    namespace impl {
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

    /// <summary>
    /// Block allocator atomicly allocates *BlockSize* number of T-types at a time. 
    /// Optionally, may skip initialization, allowing the user to initialize data on their own. 
    /// Note that for non-POD types, data must be initalized before being freed.
    /// </summary>
    /// <typeparam name="T">Type to be allocated. POD-types are more efficiently managed than non-POD.</typeparam>
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
            impl::Stack_Push(blocks, new_block_ptr);
            block_t& block = *new_block_ptr;

            // add the new elements to the list
            // std::memset(&block->elements[0], 0, sizeof(block_t));
            for (int i = 0; i < BlockSize - 1; ++i) block.elements[i].m_pNext = &block.elements[i + 1];
            block.elements[BlockSize - 1].m_pNext = nullptr;

            // push pNode onto head of list.
            uint64_t old;
            impl::THead<element_t> New;
            while (true) { // race loop
                // Get an atomic copy of head and call it old.
                // Copy old and call it new.                    
                old = New.m_n64 = free.m_n64;

                // Wire the tail of this block to connect to the old head ptr
                block.elements[BlockSize - 1].m_pNext = New.Node();

                // change New's head ptr, which bumps internal aba
                New.Node(&block.elements[0]); // head shall be the start of this block

                // compare and swap New with Head if it still matches Old.
                if (impl::CAS(&free.m_n64, old, New.m_n64))
                    break; // success
                // race, try again
            }
        };

        // Release all memory held by all blocks
        __declspec(noinline) void ReleaseBlocks() {
            while (block_t* ptr = impl::Pop(blocks)) {
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
                if (element = impl::Pop(free)) {
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
            impl::Stack_Push(free, t);
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

        impl::THead<block_t>
            blocks;
        impl::THead<element_t>
            free;
    };

    /// <summary>
    /// Thread-safe lock-free queue. More performant than concurrency's queue, but less performant than moodycamel's queue. 
    /// However, it requires more memory than concurrency's queue.
    /// But, it is more predictable and consistant than moodycamel, with guarranteed memory recovery and resists "missing" inserts/withdrawls. 
    /// Meant to be extremely balanced for most use-cases. 
    /// </summary>
    /// <typeparam name="T"></typeparam>
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
        impl::THead<element_t>
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

            impl::Stack_Push(head, new_ptr);
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

            impl::Stack_Push(head, new_ptr);
            ++count;
        };
        bool try_pop(T& out) {
            if (element_t* ptr = impl::Pop(head)) {
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

    // Equivalent to thread_local, for member objects. New threads that attempt to re-use old indexes are caught, and the object is reinitialized accordingly. 
    template <typename T> 
    class thread_object final {
    private:
        mutable size_t _tls_size{ 0 };        
        mutable concurrency::concurrent_vector<std::pair<std::thread::id, T*>> _tls;
        T const _default; // for initializing new thread objects

        auto& GetTLS() const {
            static thread_local std::thread::id thread_id{ std::this_thread::get_id() };
            auto index = get_thread_id();
            if (_tls_size <= index) {
                (void)_tls.grow_to_at_least(index + 1);
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_size), index + 1);
            }
            std::pair<std::thread::id, T*>& to_return = _tls[index];
            if (!to_return.second) {
                T* newPtr{ new T(_default) };
                if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&to_return.second), newPtr, nullptr) == nullptr) {
                    to_return.first = thread_id;
                }
                else {
                    delete newPtr;
                }
            }
            if (to_return.first != thread_id) {
                T* newPtr{ new T(_default) };
                if (auto* p = InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&to_return.second), newPtr)) {
                    delete p;
                }
                to_return.first = thread_id;
            };
            return *to_return.second;
        };
        auto& GetTLS(size_t thread_index) const {
            auto index = get_thread_id();
            if (_tls_size <= thread_index) {
                (void)_tls.grow_to_at_least(thread_index + 1);
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&_tls_size), thread_index + 1);
            }
            std::pair<std::thread::id, T*>& to_return = _tls[thread_index];
            if (!to_return.second) {
                throw std::runtime_error("The TLS should be previously initialized by the appropriate thread before access by [] operator.");
            }
            return *to_return.second;
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
            for (auto& x : _tls) {
                if (x.second) delete x.second;
            }
        };

        T* operator->() { return &GetTLS(); };
        const T* operator->() const { return &const_cast<thread_object*>(this)->GetTLS(); };
        T& operator*() { return GetTLS(); };
        const T& operator*() const { return const_cast<thread_object*>(this)->GetTLS(); };

        T& operator[](size_t thread_index) { return GetTLS(thread_index); };


        template <typename T> void for_each(T const& func) {
            for (auto& x : _tls) {
                if (x.second) {
                    func(*x.second);
                }
            }
        };
        template <typename T> bool for_each_cancellable(T const& func) {
            for (auto& x : _tls) {
                if (x.second) {
                    if (func(*x.second)) { return true; }
                }
            }
            return false;
        };
        //template <typename T> void for_each_alive(T const& func) {
        //    for (auto& x : _tls) {
        //        if (get_thread_alive(x.first)) {
        //            if (x.second) {
        //                func(*x.second);
        //            }
        //        }
        //    }
        //};
        //template <typename T> bool for_each_alive_cancellable(T const& func) {
        //    for (auto& x : _tls) {
        //        if (get_thread_alive(x.first)) {
        //            if (x.second) {
        //                if (func(*x.second)) { return true; }
        //            }
        //        }
        //    }
        //    return false;
        //};
    };

    /// <summary>
    /// Fastest allocator to-date, leveraging a block-allocator per-thread, significantly reducing contention, to the degree that this is now the fastest way to allocate memory!
    /// Plus, it is thread-safe and garbage-collected on end-of-scope. These features are effectively free now. 
    /// </summary>
    /// <typeparam name="_type_"></typeparam>
    template <typename _type_, size_t num_items = 128, bool skipInitialization = false>
    class atomic_parallel_allocator {
    private:
        struct innerType {
            _type_ // actual object, must be the first item...
                T;
            size_t // ... and attached data comes after the actual object.
                threadID;
        };
        thread_object<atomic_allocator<innerType, num_items, skipInitialization>>
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
            if constexpr (sizeof...(a) > 0) {
                out = TLS->Alloc(innerType{ _type_{std::forward<TArgs>(a)...}, get_thread_id() });
            }
            else {
                out = TLS->Alloc();
                out->threadID = get_thread_id();
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

    /// <summary>
    /// Thread-safe lock-free queue. More performant than concurrency's queue, and equal performance to moodycamel's queue. 
    /// However, it requires more memory than concurrency's queue.
    /// But, it is more predictable and consistant than moodycamel, with guarranteed memory recovery and resists "missing" inserts/withdrawls. 
    /// Meant to be extremely balanced for most use-cases. 
    /// </summary>
    /// <typeparam name="T"></typeparam>
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
        thread_object<impl::THead<element_t>>
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

            impl::Stack_Push(*head, new_ptr);
            ++count;
        };
        void push(T&& obj) {
            // get a new element
            element_t* new_ptr;
            new_ptr = allocator.Alloc();
            new_ptr->data = std::move(obj);
            new_ptr->m_pNext = nullptr;
            impl::Stack_Push(*head, new_ptr);
            ++count;
        };
        bool try_pop(T& out) {
            if (element_t* ptr = impl::Pop(*head)) {
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
            return head.for_each_cancellable([&](auto& this_head) -> bool {
                if (element_t* ptr = impl::Pop(this_head)) {
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

	/// <summary>
	/// Iterator that steps through a list, without needing to instance the whole list. 
	/// </summary>
	/// <typeparam name="Type"></typeparam>
	template<typename Type = size_t> 
    class Sequence {
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
		Sequence(Type N0, Type N1, Type Step) {
			std::tie(min, max, step) = DetermineSteps(std::move(N0), std::move(N1), std::move(Step));
		};
		Sequence() : Sequence(0, 0, 1) {};
		Sequence(Type N) : Sequence(0, N, 1) {};
		Sequence(Type N0, Type N1) : Sequence(N0, N1, 1) {};

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



#ifndef _LOCAL_WRITE_EM_H
#define _LOCAL_WRITE_EM_H

#ifndef _COMMON_H
#define _COMMON_H

#include <cassert>
#include <thread>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <cstdint> 

#define DEBUG_PRINT

#ifdef DEBUG_PRINT

#define dbg_printf(fmt, ...)                              \
  do {                                                    \
    fprintf(stderr, "%-24s: " fmt, __FUNCTION__, ##__VA_ARGS__); \
    fflush(stdout);                                       \
  } while (0);

#else

static void dummy(const char*, ...) {}

#define dbg_printf(fmt, ...)   \
  do {                         \
    dummy(fmt, ##__VA_ARGS__); \
  } while (0);

#endif

// I copied this from Linux kernel code to favor branch prediction unit on CPU
// if there is one
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif

static const size_t CACHE_LINE_SIZE = 64;

template<typename GarbageType>
class LocalWriteEMFactory;

/*
 * class PaddedData() - Pad a data type to a certain fixed length by appending
 *                      extra bytes after useful data field
 *
 * The basic constraint is that the length of the padded structure must be
 * greater than or equal to the streucture being padded
 */
template <typename T, uint64_t length>
class PaddedData {
public:
    // Define few compile time constants
    static constexpr uint64_t data_size = sizeof(T);
    static constexpr uint64_t padding_size = length - sizeof(T);
    static constexpr uint64_t total_size = length;

    // Make sure no data could be crossing cache lines
    static_assert(data_size <= CACHE_LINE_SIZE,
        "Data must be within the size of a cache line!");

    T data;

    /*
     * operator T() - Type conversion overloading
     */
    operator T() { return data; }

    /*
     * operator-> - We use this to access elements inside the data member of
     *              the wrapped class
     *
     * So the class being wrapped is accessed like we are using a pointer
     */
    T* operator->() { return &data; }

    /*
     * Get() - Explicitly call to return a reference of the data being wrapped
     */
    T& Get() const { return data; }

private:
    // This is the padding part
    char padding[padding_size];
};

/*
 * class LocalWriteEM - Epoch manager for garbage collection that only uses
 *                      local writes
 *
 * This function is used to reduce scalability problem brought about by a
 * traditional global-counter based epoch manager, where each thread has to
 * call EnterEpoch() and LeaveEpoch() on each operation to maintain global
 * counters that counts the number of active threads entering the system
 * at a certain epoch time period. The innovativity of local write epoch
 * manager is that for each operation, the worker thread only needs to
 * conduct a local write to a variable which is only maintained for each CPU
 * core (i.e. explicitly local for L1 cache dedicated to each core), and there
 * is no global synchronization except for the epoch thread. Epoch thread
 * checks each local counter and finds the minimum one. After that it uses
 * the minimum live worker thread's epoch to reclaim garbage nodes whose
 * epoch of deletion < the epoch of oldest living worker thread
 *
 * The template argument is the type of garbage node. We keep a
 * pointer type to GarbageType in the garbage node.
 */
template<typename GarbageType>
class LocalWriteEM {
public:
    // It is the type of the cuonter we use to represent an epoch
    using CounterType = uint64_t;

    // This is a padded version of epoch counter
    using ElementType = PaddedData<std::atomic<CounterType>, CACHE_LINE_SIZE>;

private:

    /*
     * class GarbageNode - The node we use to hold garbage
     *
     * All garbage nodes in the systems forms a garbage chain in which all
     * delayed allocation together with a counter recording the time it was
     * removed are stored.
     *
     * Upon garbage collection, the GC thread scans the garbage chain linked
     * list, and compares the deleted epoch with the current minimum epoch
     * announced by all threads using the per-core counter. Garbage nodes
     * with its deleted epoch being smaller than the global epoch will be
     * removed
     */
    class GarbageNode {
    public:
        CounterType deleted_epoch;
        GarbageType* garbage_p;

        // This will be updated in an unsuccessful CAS, so make it public
        GarbageNode* next_p;

        /*
         * Constructor
         *
         * Note that we do not initialize next_p here since it will be part of
         * the CAS process
         */
        GarbageNode(GarbageType* p_garbage_p, CounterType p_deleted_epoch) :
            deleted_epoch{ p_deleted_epoch },
            garbage_p{ p_garbage_p }
        {}

        /*
         * LinkTo() - Given the linked list head, try to link itself onto that
         *            linked list
         *
         * Note that although this function uses CAS instead of lock, it is
         * not wait-free - the CAS loop is effectively like a spin lock
         */
        inline void LinkTo(std::atomic<GarbageNode*>* head_p) {
            next_p = head_p->load();

            // Empty loop
            // Note that the next_p will be loaded with the most up-to-date
            // value of head_p, so we do not need to load it explicitly
            while (head_p->compare_exchange_strong(next_p, this) == false) {}

            return;
        }
    };

    // Number of cores this structure mainatains
    uint64_t core_num;

    // This is the address we should call free() on
    void* alloc_p;

    // Cache line aligned array for atomic operation
    // TODO: Using a pointer inncreases the overhead since everytime we
    // need to dereference this pointer
    // If we use a static array then the overhead could be eliminated
    // at the cost of having to statically encode the number of cores
    // which is also undesirable
    ElementType* per_core_counter_list_p;

    // This is the epoch counter that each thread needs to read when entering
    // an epoch
    // Note that according to the design, the epoch timer is set to a relatively
    // large value (50 ms, etc.) so most of the read operation for every thread
    // should be a local read unless the counter happens to be increamented by
    // the epoch thread, which does not consitute a major overhead
    ElementType epoch_counter;

    // The following does not have to be cache aligned since they 
    // are not usually operated frequently OR could not benefit from
    // cache alignment

    // This is the head of the linked list where garbage nodes are linked
    // into
    // In the future we might want to use a per core garbage list to reduce
    // contention and further accelerate the Insert() procedure
    std::atomic<GarbageNode*> garbage_head_p;

    // This is set if the destructor is called and we need to terminate the
    // GC thread, if there is one
    // Or if there is an external it should also check this flag
    // Note that on Intel platform this need not be an atomic variable since
    // Intel CPU read/write are of acquire/release semantics
    // But to accomondate other platforms that have weaker memory ordering
    // we should make it an atiomic to avoid potential bugs
    std::atomic<bool> exited_flag;

    // This is a pointer to the control structure of the GC thread if there
    // is one.
    // The GC thread is invoked explicitly by calling the member function
    // and it does not start automatically during construction since other
    // necessary structure might have not been prepared properly
    // If thread is not created by this object then the pointer is set to nullptr
    std::thread* gc_thread_p;

    // This defaults to 50ms
    uint64_t gc_interval;

    GL::atomic_parallel_allocator< GarbageNode >
        gn_alloc;

    GL::atomic_parallel_allocator< GarbageType >
        gt_alloc;

#ifndef NDEBUG
    // Under debug mode we keep a counter to record how many times 
    // FreeGarbageNode() is called by the GC thread
    uint64_t node_freed_count;

    // Number of nodes left unfreed in the EM when it is destroyed
    uint64_t node_left_count;
#endif

private:

    /*
     * AlignToCacheLine() - Aligns a given memory address to the nearest cache
     *                      line boundary by advancing it
     */
    ElementType* AlignToCacheLine(void* p) {
        // 0xFFFF FFFF FFFF FFC0 (64-bit)
        static constexpr uint64_t cache_line_mask = ~(CACHE_LINE_SIZE - 1);

        // This is the pointer after alignment
        ElementType* q = reinterpret_cast<ElementType*>(
            (reinterpret_cast<uint64_t>(p) +
                (CACHE_LINE_SIZE - 1)) & cache_line_mask);

        assert((reinterpret_cast<uint64_t>(q) % CACHE_LINE_SIZE) == 0);
        dbg_printf("Memory alignment: %p -> %p\n", p, q);

        return q;
    }

public:

    /*
     * Constructor
     *
     * The constructor has been deliberately declared as private member to
     * prevent construction on unaligned address. Please use WriteLocalEMFactory
     * class to allocate it in a cache aligned manner
     */
    LocalWriteEM(uint64_t p_core_num) :
        core_num{ p_core_num } {
        dbg_printf("C'tor for %lu cores called\n", core_num);

        // Store this for memory free
        // Allocate one more slot for alignment
        alloc_p = malloc((core_num + 1) * CACHE_LINE_SIZE);
        assert(alloc_p != nullptr);

        // Must align it to cache line boundary (64 byte typically)
        per_core_counter_list_p = AlignToCacheLine(alloc_p);

        // Initialization - all counter should be set to 0 since the global
        // epoch counter also starts at 0
        for (size_t i = 0; i < core_num; i++) {
            per_core_counter_list_p[i]->store(0);
        }

        // Also set the current epoch to be 0
        epoch_counter->store(0);

        // The end of the linked list
        garbage_head_p.store(nullptr);

        // This will be set true in destructor
        exited_flag.store(false);

        // If this is nullptr then we do not wait for it in destructor
        gc_thread_p = nullptr;

        gc_interval = 50;

#ifndef NDEBUG
        node_freed_count = 0;
        node_left_count = 0;
#endif

        return;
    }

    /*
     * Destructor - This could only be called by the factory class
     */
    ~LocalWriteEM() {
        dbg_printf("D'tor for %lu cores called\n", core_num);

        // If gc thread is inkoved inside this object then we wait for it
        if (gc_thread_p != nullptr) {
            // Signal all threads reading this variable that the epoch manager object 
            // will soon be destroyed, so just stop
            SignalExit();

            gc_thread_p->join();
        }
        else {
            // Otherwise the flag must be set true
            assert(HasExited() == true);
        }

        // The thread has already exited
        delete gc_thread_p;

        // Free all nodes currently in the GC that has not been freed
        FreeAllGarbage();

#ifndef NDEBUG
        dbg_printf("    # of nodes freed in d'tor = %lu\n", GetNodeLeftCount());
        dbg_printf("    # of nodes freed in total = %lu\n", GetNodeFreedCount());
#endif

        // Must free the array explicitly using the raw pointer rathter than 
        // aligned pointer
        free(alloc_p);

        return;
    }

private:
    /*
     * FreeAllGarbage() - This function frees all garbage nodes remaining in
     *                    the epoch manager no matter what is the value of its
     *                    epoch counter
     *
     * This function should be called in only single threaded environment. It
     * traverses the linked list and frees garbage node one by one. This is
     * usually called inside the destructor where we know all nodes deleted should
     * be freed immediately o.w. there would be a memory leak
     */
    void FreeAllGarbage() {
        GarbageNode* node_p = garbage_head_p.load();

        while (node_p != nullptr) {
            GarbageNode* next_p = node_p->next_p;

            // Free garbage itself
            // Note that this also contributes to number of nodes freed
            // by the EM
            FreeGarbageNode(node_p->garbage_p);

            gn_alloc.Free(node_p);

            node_p = next_p;

#ifndef NDEBUG
            node_left_count++;
#endif
        }

        // Restore it to nullptr to avoid it being used by accident
        // in future development
        garbage_head_p.store(nullptr);

        return;
    }

public:
    // Disallow any form of copying and construction without explicitly
    // aligning it to 64 byte boundary by the public
    LocalWriteEM(const LocalWriteEM&) = delete;
    LocalWriteEM(LocalWriteEM&&) = delete;
    LocalWriteEM& operator=(const LocalWriteEM&) = delete;
    LocalWriteEM& operator=(LocalWriteEM&&) = delete;

private:
    /*
     * HasExited() - Whether the exit signal has been issued
     *
     * This function is a wrapped to allow external access of the exited_flag
     * variable. If an external thread is used as the GC thread then the user
     * of this EM should signal exiting first, wait for external threads on this
     * condition, and then call destructor of the EM
     */
    bool HasExited() {
        return exited_flag.load();
    }

    /*
     * SignalExit() - Signals that the epoch manager will exit by setting an
     *                atomic flag to true
     *
     * If the epoch manager uses its own thread as the GC thread then after this
     * function we should wait for that thread to stop and continue. However,
     * if an external thread is used for GC, then after signaling this, the
     * external thread should react to the signal by calling query function for
     * the status of exited_flag, and then exit. The user of the epoch manager
     * should then wait for the external thread to exit before destroying the
     * EM object. Otherwise the thread might still be running after the EM has
     * been destroyed, corrupting random memory location.
     */
    void SignalExit() {
        exited_flag.store(true);

        return;
    }

public:
    /*
     * SetGCInterval() - Sets the GC interval for GC thread
     *
     * Afther each GC operation, the GC thread will sleep for a certain amount
     * of time to let worker threads cache the counter in their own L1 cache
     * and the counter shall stay there unchanged for a relatively long time
     */
    inline void SetGCInterval(uint64_t interval) {
        gc_interval = interval;

        return;
    }

private:
    /*
     * GetGCIntervale() - As name suggests
     */
    inline uint64_t GetGCInterval() const {
        return gc_interval;
    }

#ifndef NDEBUG

    /*
     * GetNodeFreedCount() - Returns a debug mode counter representing how many
     *                       times FreeGarbageNode() has been called
     */
    inline uint64_t GetNodeFreedCount() const {
        return node_freed_count;
    }

    /*
     * GetNodeLeftCount() - Return the number of nodes left uncollected when
     *                      the EM is destroyed
     *
     * This should always be called after the destructor returned, o.w. 0
     * is returned
     */
    inline uint64_t GetNodeLeftCount() const {
        return node_left_count;
    }

#endif

private:
    /*
     * Protect() - Announces that a thread enters the system
     *
     * This effectively let a thread running on the core it claimed to be
     * (through function argument) read the global epoch counter (which
     * should be a local cache read in most of the time) and then write
     * into its local latest enter epoch
     */
    inline void Protect(uint64_t core_id) {
        // Under debug mopde let's assert core id is correct to avoid
        // serious bugs
        assert(core_id < core_num);

        // This is a strict read/write ordering - load must always happen
        // before store
        per_core_counter_list_p[core_id]->store(epoch_counter->load());

        return;
    }

public:
    /*
     * Protect() - Announces that a thread enters the system
     *
     * This effectively let a thread running on the core it claimed to be
     * (through function argument) read the global epoch counter (which
     * should be a local cache read in most of the time) and then write
     * into its local latest enter epoch
     */
    inline void Protect() {
        Protect(GL::get_thread_id() % core_num);
    }

    /*
     * Free() - Adds a node whose deallocation will be delayed
     *
     * This function creates a garbage node linked list node and puts both
     * garbage node and the epoch counter into the garbage chain
     *
     * Note: When this function is called the caller should guarantee that the
     * node is already not visible by other threads, otherwise the assumption
     * made about the garbage collection mechanism will break
     */
    void Free(GarbageType* garbage_p) {
        // We load epoch counter here such that the time the garbage node disappears
        // from the system <= current epoch time
        // As long as we only do GC for nodes whose epoch counter < earlest
        // accessing epoch counter which further <= time of the thread touching
        // any shared resource, then we know it is saft to reclaim the memory
        GarbageNode* gn_p = gn_alloc.Alloc(garbage_p, epoch_counter->load());

        // Use CAS to link the node onto the linked list
        gn_p->LinkTo(&garbage_head_p);

        return;
    }

    // Acquire a new element from the free list and construct it.
    template <typename... TArgs> __declspec(noinline) GarbageType* Alloc(TArgs &&... a) {
        return gt_alloc.Alloc(std::forward<TArgs>(a)...);
    };

private:
    /*
     * FreeGarbageNode() - Frees a garbage type
     *
     * If users want to write their own epoch manager to destroy objects in a
     * customized way, then they should modify this function. Here we just
     * call operator delete to free
     *
     * Note that this function must be called in single threaded environment
     */
    inline void FreeGarbageNode(GarbageType* garbage_p) {
        gt_alloc.Free(garbage_p);

#ifndef NDEBUG
        node_freed_count++;
#endif

        return;
    }

    /*
     * GotoNextEpoch() - Increases the epoch counter value by 1
     */
    inline void GotoNextEpoch() {
        // Atomically increase the epoch counter
        epoch_counter->fetch_add(1);

        return;
    }

    /*
     * GetEpochCounter() - Get the epoch counter for debugging
     */
    inline CounterType GetCurrentEpochCounter() {
        return epoch_counter->load();
    }

    /*
     * DoGC() - This is the main function for doing garbage collection
     *
     * Note that worker threads could only access the head of linked list, which
     * should not be modified by the GC thread, otherwise ABA problem might
     * emerge since if we free a node during GC, and in the meantime a worker
     * thread comes in and allocates a node that is exactly the node we
     * just freed (malloc() tends to do that, actually) then we will have a
     * ABA problem. ABA problem might or might not be harmful depending on the
     * context, but it is best practice for us to aovid it since if the design
     * is changed in the future we will have less potential undocumented problems
     *
     * NOTE: This function does not increase epoch counter, since the pace that
     * epoch counter increases could optionally differ from the GC pace
     */
    void DoGC() {
        dbg_printf("Performing GC\n");

        // We use this to remember the minimum number of cores
        uint64_t min_epoch = per_core_counter_list_p[0]->load();

        // If there are more than 1 core then we just loop through
        // counters for each core and pick the smaller one everytime
        for (uint64_t i = 1; i < core_num; i++) {
            uint64_t counter = per_core_counter_list_p[i]->load();

            if (counter < min_epoch) {
                min_epoch = counter;
            }
        }

        // Now we have the miminum epoch which is the time <= the earlist thread
        // entering the system
        // We could collect all garbage nodes before this time

        // Load the head of the linked list
        GarbageNode* current_node_p = garbage_head_p.load();
        if (current_node_p == nullptr) {
            return;
        }

        GarbageNode* next_node_p = current_node_p->next_p;

        while (next_node_p != nullptr) {
            CounterType next_counter = next_node_p->deleted_epoch;

            if (next_counter < min_epoch) {
                // If next node is qualified then remove both the garbage node
                // and the wrapper, and since current_node_p->next_p has already
                // been set to the next node, we know these two pointers are still
                // pointing to neighbor nodes
                current_node_p->next_p = next_node_p->next_p;

                FreeGarbageNode(next_node_p->garbage_p);
                gn_alloc.Free(next_node_p);

                next_node_p = current_node_p->next_p;
            }
            else {
                // Otherwise, we know next_node_p will not be freed, and it is
                // a valid pointer, so change current_node_p to it
                // and check its next node
                current_node_p = next_node_p;
                next_node_p = next_node_p->next_p;
            }
        }

        return;
    }

    /*
     * ThreadFunc() - This is the function body for GC thread
     *
     * This function mainly wraps DoGC(), with a delay, the purpose of which is
     * to control the frequency we do GC (and invalidate local caches of the
     * global counter kept by each worker thread in their own CPU cores).
     */
    static void ThreadFunc(LocalWriteEM<GarbageType>* em) {
        // Loop on the atomic flag that will be set when destructor is called
        // (it is the first operation inside the destructor)
        while (em->HasExited() == false) {
            em->GotoNextEpoch();
            em->DoGC();

            // Sleep for gc_interval
            std::this_thread::sleep_for(std::chrono::milliseconds{ em->GetGCInterval() });
        }

        dbg_printf("Built-in GC thread has exited\n");

        return;
    }

public:
    /*
     * StartGCThread() - Starts the GC thread inside the EM object
     *
     * The GC thread will be created as a std::thread object running ThreadFunc()
     * as its thread body. It periodically wakes up and does garbage collection,
     * and will stop & exit after SignalExit() has been called
     *
     * A reasonable external GC procedure should roughly follow the same way,
     * especially before the destruction of the EM object, an external thread
     * must be signaled to stop, and thus it has to check HasExited()
     */
    void StartGCThread() {
        // Could not start new thread if the EM has been destroyed
        assert(HasExited() == false);
        assert(gc_thread_p == nullptr);

        gc_thread_p = new std::thread{ LocalWriteEM<GarbageType>::ThreadFunc, this };

        return;
    }

};

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

unsigned int GetOptimalCoreNumber();

#endif