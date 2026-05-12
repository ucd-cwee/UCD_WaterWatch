#pragma once
#include <memory>
#include <functional>
#include <atomic>
#include <memory>
#include <ShlDisp.h>
#include <winnt.h>
// #include "../GpuProgramming/matrix.h"
// #include "atomic_allocator.h"
#include "basic_atomic_allocator.h"

namespace /* atomic_shared_ptr */ GL {
    constexpr size_t MAGIC_LEN = 16;
    constexpr size_t MAGIC_MASK = 0x0000'0000'0000'FFFF;
    constexpr int CACHE_LINE_SIZE = 128;

    // base class for the shared_ptr control block
    struct control_block_base {
        explicit control_block_base() = delete;
        explicit control_block_base(void* data)
            : data(data)
            , refCount(1)
        {}
        ~control_block_base() = default;

        void* data;
        std::atomic<size_t> refCount;

        virtual void Delete() = 0;
        virtual void DeleteSelf(control_block_base*) = 0;

        // slight optimization by defering deletion of shared pointers to a specialized thread. 
        static void DeferredDeletion(control_block_base* to_delete);

    };
    
    // specialized, derived class for control blocks with specialized types.
    template<typename T> struct control_block final : public control_block_base {
        explicit control_block() = delete;
        explicit control_block(T* _data) : control_block_base(reinterpret_cast<void*>(_data)) {}
        ~control_block() = default;

        __declspec(noinline) void Delete() override {
            delete static_cast<T*>(this->data);
        };
        __declspec(noinline) void DeleteSelf(control_block_base* p) override {
            delete reinterpret_cast<control_block<T>*>(p);
        };

    public:
        template<class... _Types> static control_block<T>* AllocateSelf(T* _data) {
            return new control_block<T>(_data);
        };

    };

    // specialized, derived class for control blocks with specialized types.
    template<typename T> struct deleter_control_block final : public control_block_base{
        explicit deleter_control_block() = delete;
        explicit deleter_control_block(T* data, std::function<void(T*)>&& deleter) : control_block_base(static_cast<void*>(data)), delete_func(std::move(deleter)) {}
        ~deleter_control_block() = default;

        std::function<void(T*)> 
            delete_func;

        __declspec(noinline) void Delete() override {
            delete_func(static_cast<T*>(this->data));
        };
        __declspec(noinline) void DeleteSelf(control_block_base* p) override {
            delete reinterpret_cast<deleter_control_block<T>*>(p);
        };

    public:
        template<class... _Types> static deleter_control_block<T>* AllocateSelf(T* data, std::function<void(T*)>&& deleter) {            
            return new deleter_control_block<T>(data, std::move(deleter));
        };
    };

    // specialized, derived class for control blocks with specialized types.

    template<typename T> struct embedded_control_block final : public control_block_base {
        template<class... _Types> explicit embedded_control_block(_Types&&... _Args) : control_block_base(static_cast<void*>(reinterpret_cast<T*>(&obj))), obj(_STD forward<_Types>(_Args)...) {}
        ~embedded_control_block() = default;
        
        T obj;

        __declspec(noinline) void Delete() override {
            obj.~T();
        };
        __declspec(noinline) void DeleteSelf(control_block_base* p) override {
            delete reinterpret_cast<embedded_control_block<T>*>(p);
        };

    public:
        template<class... _Types> static embedded_control_block<T>* AllocateSelf(_Types&&... _Args) {
            return new embedded_control_block<T>(_STD forward<_Types>(_Args)...);
        };

    };
    // shared pointer that manages lifetime of the provided class. NOT THREAD-SAFE. May be modified. 
    template<typename T> class shared_ptr {
        template<typename A> friend class shared_ptr;
        template<typename A> friend class atomic_shared_ptr;

    public:
        shared_ptr() : controlBlock(nullptr), data{ nullptr } {}
        shared_ptr(std::nullptr_t) : controlBlock(nullptr), data{ nullptr } {}

        template<class U> explicit shared_ptr(U* Data) : controlBlock(dynamic_cast<control_block_base*>(control_block<U>::AllocateSelf(Data))), data{ Data } {}
        template<class U> explicit shared_ptr(U* Data, std::function<void(T*)>&& deleter) : controlBlock(dynamic_cast<control_block_base*>(deleter_control_block<U>::AllocateSelf(Data, std::move(deleter)))), data{ Data } {}

        explicit shared_ptr(control_block_base* ControlBlock, bool) : controlBlock(ControlBlock), data{ reinterpret_cast<T*>(ControlBlock ? ControlBlock->data : nullptr) } {}
        static shared_ptr clone_from_control_block(control_block_base* ControlBlock) {
            shared_ptr out;
            out.controlBlock = ControlBlock;            
            if (out.controlBlock != nullptr) {
                out.controlBlock->refCount.fetch_add(1, std::memory_order_relaxed);
            }
            out.data = reinterpret_cast<T*>(ControlBlock ? ControlBlock->data : nullptr);
            return out;
        };

        bool compare_exchange(control_block_base* const expected, control_block_base* released_control_block) {
            control_block_base* oldP = reinterpret_cast<control_block_base*>(InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&controlBlock), released_control_block, expected));
            if (oldP == expected) {
                // exchange was successful
                if (oldP) control_block_base::DeferredDeletion(oldP);   
                data = reinterpret_cast<T*>(released_control_block ? released_control_block->data : nullptr);
                return true;
            }
            else {
                // exchange failed
                if (released_control_block) control_block_base::DeferredDeletion(released_control_block);
                return false;
            }
        };

        shared_ptr(const shared_ptr& other) {
            controlBlock = other.controlBlock;
            data = other.data;
            if (controlBlock != nullptr) {
                controlBlock->refCount.fetch_add(1, std::memory_order_relaxed);
            }
        };
        shared_ptr(shared_ptr&& other) noexcept {
            controlBlock = other.controlBlock;
            data = other.data;
            other.controlBlock = nullptr;
            other.data = nullptr;
        };
        template<class U> shared_ptr(const shared_ptr<U>& other) {
            controlBlock = const_cast<shared_ptr<T>&>(reinterpret_cast<const shared_ptr<T>&>(other)).controlBlock;
            data = const_cast<shared_ptr<T>&>(reinterpret_cast<const shared_ptr<T>&>(other)).data;
            if (controlBlock != nullptr) {
                controlBlock->refCount.fetch_add(1, std::memory_order_relaxed);
            }
        };
        template<class U> shared_ptr(shared_ptr<U>&& other) {
            controlBlock = const_cast<shared_ptr<T>&>(reinterpret_cast<const shared_ptr<T>&>(other)).controlBlock;
            data = const_cast<shared_ptr<T>&>(reinterpret_cast<const shared_ptr<T>&>(other)).data;
            other.controlBlock = nullptr;
            other.data = nullptr;
        };

        shared_ptr& operator=(const shared_ptr& other) {
            auto old = controlBlock;
            controlBlock = other.controlBlock;
            data = other.data;
            if (controlBlock != nullptr) {
                controlBlock->refCount.fetch_add(1, std::memory_order_relaxed);
            }
            control_block_base::DeferredDeletion(old);
            return *this;
        };
        shared_ptr& operator=(shared_ptr&& other) {
            if (controlBlock != other.controlBlock) {
                auto old = controlBlock;
                controlBlock = other.controlBlock;
                data = other.data;
                other.controlBlock = nullptr;          
                other.data = nullptr;
                control_block_base::DeferredDeletion(old);
            }
            return *this;
        }        
        shared_ptr& operator=(std::nullptr_t) {
            control_block_base::DeferredDeletion(controlBlock);
            controlBlock = nullptr;
            data = nullptr;
            return *this;
        }
        __declspec(noinline) ~shared_ptr() {
            if (controlBlock) {
                control_block_base::DeferredDeletion(controlBlock);
            }
        };

        T* get() const { 
            return data;
        }
        T* operator->() const { 
            return get();
        }
        template <class _Ty2 = T, std::enable_if_t<!std::disjunction_v<std::is_array<_Ty2>, std::is_void<_Ty2>>, int> = 0>
        decltype(auto) operator*() const {
            return *get();
        };
        operator bool() const {
            return data != nullptr;
        };

        friend bool operator==(const shared_ptr& a, const shared_ptr& b) noexcept { return a.data == b.data; };
        friend bool operator!=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.data != b.data; };
        friend bool operator<(const shared_ptr& a, const shared_ptr& b) noexcept { return a.data < b.data; };
        friend bool operator<=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.data <= b.data; };
        friend bool operator>(const shared_ptr& a, const shared_ptr& b) noexcept { return a.data > b.data; };
        friend bool operator>=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.data >= b.data; };

        control_block_base* copy_control_block() const {
            return controlBlock;
        };
        control_block_base* release_control_block() {
            control_block_base* out = controlBlock;
            controlBlock = nullptr;
            data = nullptr;
            return out;
        };
        void set_pointer_without_modifying_control_block(T* ptr) {
            data = ptr;
        };
    protected:        
        control_block_base* controlBlock;
        T* data;
    };

    // shared pointer that manages lifetime of the provided class. NOT THREAD-SAFE. Read-only, and cannot be shared without the move operator. 
    template<typename T> class fast_shared_ptr {
        template<typename A> friend class atomic_shared_ptr;
    public:
        explicit fast_shared_ptr(shared_ptr<T>&& Data);
        fast_shared_ptr() : knownValue(0), foreignPackedPtr(nullptr), data(nullptr) {};
        fast_shared_ptr(std::nullptr_t) : knownValue(0), foreignPackedPtr(nullptr), data(nullptr) {};
        fast_shared_ptr(fast_shared_ptr<T>&& other)
            : knownValue(other.knownValue)
            , foreignPackedPtr(other.foreignPackedPtr)
            , data(other.data)
        {
            other.foreignPackedPtr = nullptr;
        };
        fast_shared_ptr& operator=(fast_shared_ptr<T>&& other) {
            destroy();
            knownValue = other.knownValue;
            foreignPackedPtr = other.foreignPackedPtr;
            data = other.data;
            other.foreignPackedPtr = nullptr;
            return *this;
        }
        fast_shared_ptr(fast_shared_ptr<T> const&) = delete;
        fast_shared_ptr& operator=(fast_shared_ptr<T> const&) = delete;
        fast_shared_ptr& operator=(std::nullptr_t) {
            destroy();
            knownValue = 0;
            foreignPackedPtr = nullptr;
            data = nullptr;
            return *this;
        };
        ~fast_shared_ptr() {
            destroy();
        };

        T* get() const { return data; }
        T* operator->() const { return data; }
        template <class _Ty2 = T, std::enable_if_t<!std::disjunction_v<std::is_array<_Ty2>, std::is_void<_Ty2>>, int> = 0>
        decltype(auto) operator*() {
            return *get();
        };
        operator bool() const {
            return (bool)(data);
        };
        friend bool operator==(const fast_shared_ptr& a, const fast_shared_ptr& b) noexcept { return a.data == b.data; };
        friend bool operator!=(const fast_shared_ptr& a, const fast_shared_ptr& b) noexcept { return a.data != b.data; };
        friend bool operator<(const fast_shared_ptr& a, const fast_shared_ptr& b) noexcept { return a.data < b.data; };
        friend bool operator<=(const fast_shared_ptr& a, const fast_shared_ptr& b) noexcept { return a.data <= b.data; };
        friend bool operator>(const fast_shared_ptr& a, const fast_shared_ptr& b) noexcept { return a.data > b.data; };
        friend bool operator>=(const fast_shared_ptr& a, const fast_shared_ptr& b) noexcept { return a.data >= b.data; };

    private:
        control_block_base* get_control_block() { return reinterpret_cast<control_block_base*>(knownValue >> MAGIC_LEN); }
        void destroy() {
            if (foreignPackedPtr != nullptr) {
                size_t expected = knownValue;
                while (!foreignPackedPtr->compare_exchange_weak(expected, expected - 1)) {
                    if (((expected >> MAGIC_LEN) != (knownValue >> MAGIC_LEN)) || !(expected & MAGIC_MASK)) {
                        control_block_base* block = reinterpret_cast<control_block_base*>(knownValue >> MAGIC_LEN);
                        control_block_base::DeferredDeletion(block);
                        break;
                    }
                }
            }
        };
        fast_shared_ptr(std::atomic<size_t>* packedPtr)
            : knownValue(packedPtr->fetch_add(1) + 1)
            , foreignPackedPtr(packedPtr)
            , data(reinterpret_cast<T*>(get_control_block()->data))
        {
            auto block = get_control_block();
            int diff = knownValue & MAGIC_MASK;
            while (diff > 1000 && block == get_control_block()) {
                block->refCount.fetch_add(diff, std::memory_order_relaxed);
                if (packedPtr->compare_exchange_strong(knownValue, knownValue - diff)) {
                    foreignPackedPtr = nullptr;
                    break;
                }
                block->refCount.fetch_sub(diff, std::memory_order_relaxed);
                diff = knownValue & MAGIC_MASK;
            }
        };

        size_t knownValue;
        std::atomic<size_t>* foreignPackedPtr;
        T* data;
    };

    // lock-free, thread-safe version of std::atomic<shared_ptr>.
    template<typename T> class atomic_shared_ptr {
    public:
        atomic_shared_ptr(shared_ptr<T> && data) {
            control_block_base* block = dynamic_cast<control_block_base*>(control_block<T>::AllocateSelf(nullptr));
            packedPtr.store(reinterpret_cast<size_t>(block) << MAGIC_LEN);     
            //while (true) {
                auto holder = this->load_fast();
                /*if (*/compare_exchange(holder.get(), holder.get_control_block(), std::move(data))/*) {*/
                    //break
                    ;
                /*}*/
            //}
        };
        atomic_shared_ptr(T* data = nullptr) {
            control_block_base* block = dynamic_cast<control_block_base*>(control_block<T>::AllocateSelf(data));
            packedPtr.store(reinterpret_cast<size_t>(block) << MAGIC_LEN);
        };
        explicit atomic_shared_ptr(control_block_base* controlBlock, bool) {
            control_block_base* block = dynamic_cast<control_block_base*>(controlBlock);
            packedPtr.store(reinterpret_cast<size_t>(block) << MAGIC_LEN);
        };
        ~atomic_shared_ptr() {
            size_t packedPtrCopy = packedPtr.load();
            if (packedPtrCopy > 0) {
                auto block = reinterpret_cast<control_block<T>*>(packedPtrCopy >> MAGIC_LEN);
                size_t diff = packedPtrCopy & MAGIC_MASK;
                if (diff != 0) {
                    block->refCount.fetch_add(diff, std::memory_order_relaxed);
                }
                control_block_base::DeferredDeletion(block);
            }
        };

        atomic_shared_ptr(const atomic_shared_ptr& other) : atomic_shared_ptr(const_cast<atomic_shared_ptr&>(other).load()) {};
        atomic_shared_ptr(atomic_shared_ptr&& other) noexcept : packedPtr(other.packedPtr.exchange(0)) {};
        atomic_shared_ptr& operator=(const atomic_shared_ptr& other) {
            auto ptr = const_cast<atomic_shared_ptr&>(other).load();
            this->store(std::move(ptr));
            return *this;
        };
        atomic_shared_ptr& operator=(atomic_shared_ptr&& other) noexcept {
            auto ptr = other.load();
            this->store(std::move(ptr));
            return *this;
        };
        atomic_shared_ptr& operator=(std::nullptr_t) {
            return operator=(atomic_shared_ptr());
        };

        // return a shared_ptr meant for accessing the value of the current ptr.
        shared_ptr<T> load() {
            // taking copy and notifying about read in progress
            size_t packedPtrCopy = packedPtr.fetch_add(1);
            auto block = reinterpret_cast<control_block<T>*>(packedPtrCopy >> MAGIC_LEN);
            block->refCount.fetch_add(1, std::memory_order_relaxed);
            // copy is completed

            // notifying about completed copy
            size_t expected = packedPtrCopy + 1;
            while (true) {
                size_t expCopy = expected;
                if (packedPtr.compare_exchange_weak(expected, expected - 1)) {
                    break;
                }

                // if control block pointer just changed, then
                // handling object's ref count is not our responsibility
                if (((expected >> MAGIC_LEN) != (packedPtrCopy >> MAGIC_LEN)) ||
                    ((expected & MAGIC_MASK) == 0)) // >20 hours wasted here
                {
                    block->refCount.fetch_sub(1, std::memory_order_relaxed);
                    break;
                }

                if ((expected & MAGIC_MASK) == 0) {
                    abort();
                    break;
                }
            }
            // notification finished

            return shared_ptr<T>(block, true);
        };
        // return a read-only, optimized shared_ptr meant for quickly accessing the value of the current ptr.
        fast_shared_ptr<T> load_fast() {
            return fast_shared_ptr<T>(&packedPtr);
        };

    private:
        // set the atomic_shared_ptr if the expected value is found in-place. returns true if successful. 
        bool compare_exchange(T* expected, control_block_base* control, shared_ptr<T>&& newOne) {
            if (expected == newOne.get()) {
                return true;
            }
            else {
                size_t holdedPtr = reinterpret_cast<size_t>(control);
                size_t desiredPackedPtr = reinterpret_cast<size_t>(newOne.controlBlock) << MAGIC_LEN;
                size_t expectedPackedPtr = holdedPtr << MAGIC_LEN;
                while (holdedPtr == (expectedPackedPtr >> MAGIC_LEN)) {
                    if (expectedPackedPtr & MAGIC_MASK) {
                        int diff = expectedPackedPtr & MAGIC_MASK;
                        control->refCount.fetch_add(diff, std::memory_order_relaxed);
                        if (!packedPtr.compare_exchange_weak(expectedPackedPtr, expectedPackedPtr & ~MAGIC_MASK)) {
                            control->refCount.fetch_sub(diff, std::memory_order_relaxed);
                        }
                        continue;
                    }
                    if (packedPtr.compare_exchange_weak(expectedPackedPtr, desiredPackedPtr)) {
                        newOne.controlBlock = nullptr;
                        control_block_base::DeferredDeletion(reinterpret_cast<control_block_base*>(expectedPackedPtr >> MAGIC_LEN));
                        return true;
                    }
                }
            }
            return false;
        }; // this actually is strong version

    public:
        // set the atomic_shared_ptr if the expected value is found in-place. returns true if successful. 
        bool compare_exchange(T* expected, shared_ptr<T>&& newOne) {
            if (expected == newOne.get()) {
                return true;
            }
            auto holder = this->load_fast();            
            if (holder.get() == expected) {
                size_t holdedPtr = reinterpret_cast<size_t>(holder.get_control_block());
                size_t desiredPackedPtr = reinterpret_cast<size_t>(newOne.controlBlock) << MAGIC_LEN;
                size_t expectedPackedPtr = holdedPtr << MAGIC_LEN;
                while (holdedPtr == (expectedPackedPtr >> MAGIC_LEN)) {
                    if (expectedPackedPtr & MAGIC_MASK) {
                        int diff = expectedPackedPtr & MAGIC_MASK;
                        holder.get_control_block()->refCount.fetch_add(diff, std::memory_order_relaxed);
                        if (!packedPtr.compare_exchange_weak(expectedPackedPtr, expectedPackedPtr & ~MAGIC_MASK)) {
                            holder.get_control_block()->refCount.fetch_sub(diff, std::memory_order_relaxed);
                        }
                        continue;
                    }
                    if (packedPtr.compare_exchange_weak(expectedPackedPtr, desiredPackedPtr)) {
                        newOne.controlBlock = nullptr;
                        control_block_base::DeferredDeletion(reinterpret_cast<control_block_base*>(expectedPackedPtr >> MAGIC_LEN));
                        return true;
                    }
                }
            }
            return false;
        }; // this actually is strong version

        // set the value of the atomic_shared_pointer to this pointer. 
        void store(shared_ptr<T>&& data) {
            while (true) {
                fast_shared_ptr<T> holder = this->load_fast();
                if (compare_exchange(holder.get(), holder.get_control_block(), std::move(data))) {
                    break;
                }
            }
        };

        friend bool operator==(const atomic_shared_ptr& a, const atomic_shared_ptr& b) noexcept { return reinterpret_cast<control_block<T>*>(a.packedPtr.load() >> MAGIC_LEN) == reinterpret_cast<control_block<T>*>(b.packedPtr.load() >> MAGIC_LEN); };
        friend bool operator!=(const atomic_shared_ptr& a, const atomic_shared_ptr& b) noexcept { return reinterpret_cast<control_block<T>*>(a.packedPtr.load() >> MAGIC_LEN) != reinterpret_cast<control_block<T>*>(b.packedPtr.load() >> MAGIC_LEN); };
        friend bool operator<(const atomic_shared_ptr& a, const atomic_shared_ptr& b) noexcept { return reinterpret_cast<control_block<T>*>(a.packedPtr.load() >> MAGIC_LEN) < reinterpret_cast<control_block<T>*>(b.packedPtr.load() >> MAGIC_LEN); };
        friend bool operator<=(const atomic_shared_ptr& a, const atomic_shared_ptr& b) noexcept { return reinterpret_cast<control_block<T>*>(a.packedPtr.load() >> MAGIC_LEN) <= reinterpret_cast<control_block<T>*>(b.packedPtr.load() >> MAGIC_LEN); };
        friend bool operator>(const atomic_shared_ptr& a, const atomic_shared_ptr& b) noexcept { return reinterpret_cast<control_block<T>*>(a.packedPtr.load() >> MAGIC_LEN) > reinterpret_cast<control_block<T>*>(b.packedPtr.load() >> MAGIC_LEN); };
        friend bool operator>=(const atomic_shared_ptr& a, const atomic_shared_ptr& b) noexcept { return reinterpret_cast<control_block<T>*>(a.packedPtr.load() >> MAGIC_LEN) >= reinterpret_cast<control_block<T>*>(b.packedPtr.load() >> MAGIC_LEN); };

    private:
        /* first 48 bit - pointer to control block
         * last 16 bit - local ref count if anyone is accessing control block
         * through current atomic_shared_ptr instance right now */
        std::atomic<size_t> packedPtr;
        static_assert(sizeof(T*) == sizeof(size_t));
    };

    template <class _Ty, class... _Types> _NODISCARD shared_ptr<_Ty> make_shared(_Types&&... _Args) {
        return shared_ptr<_Ty>(dynamic_cast<control_block_base*>(embedded_control_block<_Ty>::AllocateSelf(_STD forward<_Types>(_Args)...)), true);
    };
    template <class _Ty> _NODISCARD shared_ptr<_Ty> make_shared_forwarded(_Ty&& _Args) {
        return shared_ptr<_Ty>(dynamic_cast<control_block_base*>(embedded_control_block<_Ty>::AllocateSelf(_STD forward<_Ty>(_Args))), true);
    };
    template<typename To, typename From> static _NODISCARD atomic_shared_ptr<To> static_pointer_cast(atomic_shared_ptr<From> && from) {
        return atomic_shared_ptr<To>(shared_ptr<To>(from.load()));
    };
    template<typename To, typename From> static _NODISCARD shared_ptr<To> static_pointer_cast(shared_ptr<From> && from) {
        return shared_ptr<To>(std::move(from));
    };
    template<typename T> __forceinline fast_shared_ptr<T>::fast_shared_ptr(shared_ptr<T>&& Data) : knownValue(reinterpret_cast<size_t>(Data.release_control_block()) << MAGIC_LEN), foreignPackedPtr(nullptr), data(nullptr) {
        data = reinterpret_cast<T*>(get_control_block()->data);
    };

};
namespace /* hash */ std {
    template <typename T> struct hash<GL::shared_ptr<T>> {
        std::size_t operator()(const GL::shared_ptr<T>& k) const {
            static std::hash<T> hasher{};
            if (auto* p = k.get()) {
                return hasher(*p);
            }
            return 0;
        };
    };
    template <typename T> struct hash<GL::fast_shared_ptr<T>> {
        std::size_t operator()(const GL::fast_shared_ptr<T>& k) const {
            static std::hash<T> hasher{};
            if (auto* p = k.get()) {
                return hasher(*p);
            }
            return 0;
        };
    };
    template <typename T> struct hash<GL::atomic_shared_ptr<T>> {
        std::size_t operator()(const GL::atomic_shared_ptr<T>& k) const {
            static std::hash<GL::fast_shared_ptr<T>> hasher{};
            if (auto p = k.get_fast()) {
                return hasher(p);
            }
            return 0;
        };
    };
};


