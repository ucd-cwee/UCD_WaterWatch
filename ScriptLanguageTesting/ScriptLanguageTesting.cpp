#pragma region "Includes"
#include <iostream>
#include <string>
#include <string_view>
#include "../GoodLang/Any.h"
#include "../GoodLang/Proxy_Function.h"
#include "../GoodLang/ThreadSafeContainers.h"
#include "../GoodLang/Units.h"
#include "../GoodLang/DateTime.h"
#include "../GoodLang/Parallel.h"
#include "../WaterWatchCpp/Clock.h"
#include "../GoodLang/Scopes.h"
#include "../FiberTasks/Concurrent_Queue.h"
#include <regex>
#pragma endregion

#pragma region "Definitions"
#define SINGLE_ARG(...) __VA_ARGS__
#define EXPECT_EQ_PRINTF(A,B) [a = (A), b = (B)]()->bool{ \
    if (a == b) { return true; } else { \
        std::string tempA{std::to_string(a)}, tempB{std::to_string(b)}; \
        std::cout << GoodLang::printf("FAILURE AT LINE %i: (%s != %s)\n", (int)__LINE__, tempA.c_str(), tempB.c_str()); \
    return false; } \
}()
#define print(a) std::cout << a << std::endl
#define EXPECT_EQ(a, b) if (a != b){ print(GoodLang::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#define EXPECT_NE(a, b) if (a == b){ print(GoodLang::printf("FAILURE AT LINE %i\n", (int)__LINE__)); }
#pragma endregion

namespace utilities {       
    class string {
    public:
        using size_type = std::string_view::size_type;
        static constexpr const auto npos = std::string_view::npos;

    protected:
        std::shared_ptr<std::string> 
            _data; // maintains ownership of the data if necessary
        std::string_view 
            data;
        size_type 
            _hash{ npos };

    public:
        string() {};
        string(string const&) = default;
        string(string&&) = default;
        string& operator=(string const&) = default;
        string& operator=(string&&) = default;
        virtual ~string() = default;

        template <size_t N> __forceinline string(const char(&r)[N]) : data(r) {};
        string(std::string && _Copy) : _data(std::make_shared<std::string>(std::move(_Copy))) {
            data = *_data;
        };
        string(std::string_view&& _Copy) : data(_Copy) {};

        friend bool operator==(string const& A, string const& V) noexcept {
            if (A.data.length() != V.data.length()) return false;
            else if (A.data.length() > 1) return A.hash() == V.hash();
            else return A.data == V.data;
        };
        friend bool operator<(string const& A, string const& V) {
            if (A.data.length() < V.data.length()) return true;
            else if (A.data.length() > V.data.length()) return false;
            else /*if (A.length() > 1)*/ return A.hash() < V.hash();
            //else return A.data < V.data;
        };
        friend bool operator<=(string const& A, string const& V) {
            if (A.data.length() < V.data.length()) return true;
            else if (A.data.length() > V.data.length()) return false;
            else /*if (A.length() > 1)*/ return A.hash() <= V.hash();
            //else return A.data <= V.data;
        };
        friend bool operator>(string const& A, string const& V) { return !operator<=(A, V); };
        friend bool operator>=(string const& A, string const& V) { return !operator<(A, V); };
        friend bool operator!=(string const& A, string const& V) noexcept { return !operator==(A, V); };

    public:
        std::string to_string() const {
            return std::string(data);
        };
        const char* c_str() const {
            return data.data();
        };
        std::string_view substr(const size_type _Off = 0, size_type _Count = npos) const {
            return data.substr(_Off, _Count);
        };
        bool empty() const {
            return data.empty();
        };
        size_type size() const {
            return data.length();
        };
        size_type length() const {
            return data.length();
        };
        const char& at(const size_type _Off) const {
            return data.at(_Off);
        };
        const char& front() const {
            return data.front();
        };
        const char& back() const {
            return data.back();
        };
        const char& operator[](const size_type index) const {
            return data.operator[](index);
        };
        size_type hash(size_type out = 0) const {
            if (out == 0 && length() > 16) {
                if (_hash == npos) {
                    for (auto& x : data) out ^= x + 0x9e3779b9 + (out << 6) + (out >> 2);
                    InterlockedExchange(reinterpret_cast<volatile size_type*>(const_cast<size_type*>(&_hash)), out);
                }
                return _hash;
            }
            for (auto& x : data) out ^= x + 0x9e3779b9 + (out << 6) + (out >> 2);
            return out;
        };
        void remove_prefix(const size_type _Count) noexcept {
            data.remove_prefix(_Count);
        };
        void remove_suffix(const size_type _Count) noexcept {
            data.remove_suffix(_Count);
        };
        size_type rfind(const string& _Right) const {
            return data.rfind(_Right.data);
        };
        string remove_trailing(char _Right) const {
            string out{ *this };
            while (
                (out.length() > 0)
                && ((out.operator[](out.length() - 1) == _Right))
                ) {
                out.remove_suffix(1);
            }
            return out;
        };
        string remove_leading(char _Right) const {
            string out{ *this };
            while (
                (out.length() > 0)
                && (out.operator[](0) == _Right)
                ) {
                out.remove_prefix(1);
            }
            return out;
        };
        string remove_leading_and_trailing(char _Right) const {
            string out{ *this };
            return out.remove_trailing(_Right).remove_leading(_Right);
        };

        static const string& empty_string() {
            static string out{ "" };
            return out;
        };
        static const string& namespace_colons() {
            static string out{ "::" };
            return out;
        };
    };

    // all const-functions are thread-safe
    class compound_shared_string {
    public:
        string a;
        string b;
        string c;

    public:
        compound_shared_string(string A = "", string B = "", string C = "") : a{ A }, b{ B }, c{ C } {};
        compound_shared_string(compound_shared_string const&) = default;
        compound_shared_string(compound_shared_string&&) = default;
        compound_shared_string& operator=(compound_shared_string const&) = default;
        compound_shared_string& operator=(compound_shared_string&&) = default;
        ~compound_shared_string() = default;

        const char& operator[](size_t index) const {
            if (index < a.length()) {
                return a[index];
            }
            index -= a.length();
            if (index < b.length()) {
                return b[index];
            }
            index -= b.length();
            return c[index];
        }
        size_t length() const {
            return a.length() + b.length() + c.length();
        }
        size_t hash(size_t out = 0) const {
            return c.hash(b.hash(a.hash(out)));
        }
        bool empty() const {
            return length() == 0;
        };
        size_t Cmpn(const string& rhs, size_t n = std::string::npos) const {
            long long j = std::min<long long>(rhs.length(), length());
            if (n > j) return false;
            for (j = j - 1; (j >= 0) && (n > 0); --j, --n) {
                if (rhs[j] != operator[](j)) return false;
            }
            return true;
        };
        friend bool operator==(compound_shared_string const& lhs, const string& rhs) {
            if (rhs.length() != lhs.length()) return false;
            for (long long j = lhs.length() - 1; j >= 0; --j) {
                if (rhs[j] != lhs[j]) return false;
            }
            return true;
        };
        friend bool operator==(const string& rhs, compound_shared_string const& lhs) {
            if (rhs.length() != lhs.length()) return false;
            for (long long j = lhs.length() - 1; j >= 0; --j) {
                if (rhs[j] != lhs[j]) return false;
            }
            return true;
        };
        friend bool operator!=(compound_shared_string const& lhs, const string& rhs) {
            return !operator==(lhs, rhs);
        };
        friend bool operator!=(const string& rhs, compound_shared_string const& lhs) {
            return !operator==(lhs, rhs);
        };
        operator string() const {
            std::string out(a.c_str());
            out.append(b.c_str(), b.length());
            out.append(c.c_str(), c.length());
            return out;
        };

        void remove_prefix(long long n) {
            if ((n > 0) && a.length() > 0) {
                long long toRemove = std::min<long long>(n, a.length());
                if (toRemove > 0) {
                    n -= toRemove;
                    a.remove_prefix(toRemove);
                }
            }
            if ((n > 0) && b.length() > 0) {
                long long toRemove = std::min<long long>(n, b.length());
                if (toRemove > 0) {
                    n -= toRemove;
                    b.remove_prefix(toRemove);
                }
            }
            if ((n > 0) && c.length() > 0) {
                long long toRemove = std::min<long long>(n, c.length());
                if (toRemove > 0) {
                    n -= toRemove;
                    c.remove_prefix(toRemove);
                }
            }
        };
        void remove_suffix(long long n) {
            if ((n > 0) && c.length() > 0) {
                long long toRemove = std::min<long long>(n, c.length());
                if (toRemove > 0) {
                    n -= toRemove;
                    c.remove_suffix(toRemove);
                }
            }
            if ((n > 0) && b.length() > 0) {
                long long toRemove = std::min<long long>(n, b.length());
                if (toRemove > 0) {
                    n -= toRemove;
                    b.remove_suffix(toRemove);
                }
            }
            if ((n > 0) && a.length() > 0) {
                long long toRemove = std::min<long long>(n, a.length());
                if (toRemove > 0) {
                    n -= toRemove;
                    a.remove_suffix(toRemove);
                }
            }
        };
        compound_shared_string substr(size_t offset = 0, size_t count = std::string::npos) const {
            compound_shared_string copy = *this;
            if (offset > 0) copy.remove_prefix(offset);
            if (count < copy.length()) copy.remove_suffix(copy.length() - count);
            return copy;
        }
        compound_shared_string remove_trailing(char _Right) const {
            compound_shared_string out{ *this };
            while (
                (out.length() > 0)
                && ((out.operator[](out.length() - 1) == _Right))
                ) {
                out.remove_suffix(1);
            }
            return out;
        };
        compound_shared_string remove_leading(char _Right) const {
            compound_shared_string out{ *this };
            while (
                (out.length() > 0)
                && (out.operator[](0) == _Right)
                ) {
                out.remove_prefix(1);
            }
            return out;
        };
        compound_shared_string remove_leading_and_trailing(char _Right) const {
            compound_shared_string out{ *this };
            return out.remove_trailing(_Right).remove_leading(_Right);
        };
    };

    template <typename T>
    class DelayedInstantiation {
    private:
        std::unique_ptr<T> ptr{ nullptr };
        GoodLang::fast_shared_mutex mut{};
        long initialized{ 0 };

    public:
        DelayedInstantiation() = default;
        ~DelayedInstantiation() = default;

        bool valid() const {
            return initialized > 0;
        };
        T* operator->() {
            if (initialized == 0) {
                mut.lock();
                if (initialized == 0) {
                    ptr = std::make_unique<T>();
                    InterlockedIncrement(static_cast<volatile long*>(&initialized));
                }
                mut.unlock();
            }
            return ptr.get();
        };
        T& operator*() {
            return *operator->();
        };
        operator bool() const { return valid(); };
    };

    namespace ABA_Problem {


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
            T* Node() { return (T*)m_bits.m_pNode; }
            // changeing Node bumps aba
            THead* Node(T* p) { m_bits.m_nABA++; m_bits.m_pNode = (uint64_t)p; return this; }
        };
        
        static bool CAS(uint64_t* Destination, uint64_t& Comperand, uint64_t& Exchange) {
            return InterlockedCompareExchange(static_cast<volatile uint64_t*>(Destination), Exchange, Comperand) == Comperand;
        };
        
        // pop pNode from head of list.
        template<class T>
        __declspec(noinline) T* Pop(THead<T>& Head) {
            THead<T> Old, New; 
            while (1) { // race loop
                // Get an atomic copy of head and call it old.
                Old.m_n64 = Head.m_n64;
                if (Old.is_null()) return nullptr;
                // 
                New.m_n64 = Old.m_n64;
                // change New's Node, which bumps internal aba
                New.Node(Old.Node()->m_pNext);
                // compare and swap New with Head if it still matches Old.
                if (CAS(&Head.m_n64, Old.m_n64, New.m_n64)) return THead<T>::Finalize(Old.Node()); // success                
                // race, try again
            }
        }
       
        // push pNode onto head of list.
        template<class T>
        __declspec(noinline) void Push(THead<T>& Head, T* pNode) {
            THead<T> Old, New;
            while (1) { // race loop
                // Get an atomic copy of head and call it old.
                // Copy old and call it new.
                New.m_n64 = Old.m_n64 = Head.m_n64;
                // Wire node t Head
                pNode->m_pNext = New.Node();
                // change New's head ptr, which bumps internal aba
                New.Node(pNode);
                // compare and swap New with Head if it still matches Old.
                if (CAS(&Head.m_n64, Old.m_n64, New.m_n64)) break; // success
                // race, try again
            }
        }

        /// <summary>
        /// Block allocator atomicly allocates *BlockSize* number of T-types at a time. 
        /// Optionally, may skip initialization, allowing the user to initialize data on their own. 
        /// Note that for non-POD types, data must be initalized before being freed.
        /// </summary>
        /// <typeparam name="T">Type to be allocated. POD-types are more efficiently managed than non-POD.</typeparam>
        template <typename T, size_t BlockSize, bool skipInitialization = false> class BlockAlloc {
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
            };

            // Allocate one new block of contiguous elements
            __declspec(noinline) void AllocBlock() {
                std::unique_ptr<block_t>& block = *blocks.push_back(std::make_unique<block_t>());

                // add the new elements to the list
                // std::memset(&block->elements[0], 0, sizeof(block_t));
                for (int i = 0; i < BlockSize - 1; ++i) block->elements[i].m_pNext = &block->elements[i + 1];
                block->elements[BlockSize - 1].m_pNext = nullptr;

                // push pNode onto head of list.
                uint64_t old;
                THead<element_t> New;
                while (true) { // race loop
                    // Get an atomic copy of head and call it old.
                    // Copy old and call it new.                    
                    old = New.m_n64 = free.m_n64;

                    // Wire the tail of this block to connect to the old head ptr
                    block->elements[BlockSize - 1].m_pNext = New.Node();

                    // change New's head ptr, which bumps internal aba
                    New.Node(&block->elements[0]); // head shall be the start of this block

                    // compare and swap New with Head if it still matches Old.
                    if (CAS(&free.m_n64, old, New.m_n64))
                        break; // success
                    // race, try again
                }                
            };

            // Release all memory held by all blocks
            __declspec(noinline) void ReleaseBlocks() {
                if constexpr (!std::is_pod<T>::value) {
                    for (auto& block : blocks) {
                        for (auto& element : block->elements) {
                            if (element.initialized) {
                                reinterpret_cast<T*>(&element.data[0])->~T();
                                element.initialized = false;
                            }
                        }
                    }
                }
            };

        public:
            BlockAlloc() : blocks{}, free{} { free.m_n64 = 0; };
            ~BlockAlloc() { ReleaseBlocks(); };
            
            // calling this unloads all the data and prevents use of the allocator. Should be used when the allocator is about to be deleted but (for whatever reason) needs to be unloaded at a specific schedule.
            void unsafe_unload() {
                ReleaseBlocks();
            };

            // Acquire a new element from the free list and construct it.
            template <typename... TArgs> __declspec(noinline) T* Alloc(TArgs &&... a) {
                element_t* element{ nullptr };
                while (1) {
                    if (element = Pop(free)) {
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
                Push(free, t);
            };


            concurrency::concurrent_vector<std::unique_ptr<block_t>>
                blocks;
            THead<element_t>
                free;
        };

        /// <summary>
        /// Fastest allocator to-date, leveraging a block-allocator per-thread, significantly reducing contention, to the degree that this is now the fastest way to allocate memory!
        /// Plus, it is thread-safe and garbage-collected on end-of-scope. These features are effectively free now. 
        /// </summary>
        /// <typeparam name="_type_"></typeparam>
        template <typename _type_, size_t num_items = sizeof(_type_) << 4, size_t num_parallel_allocators = 4, bool skipInitialization = false>
        class Allocator final : public GoodLang::EpochGarbageCollectorImpl {
        private:
            struct innerType {
                _type_ // actual object, must be the first item...
                    T;
                size_t // ... and attached data comes after the actual object.
                    threadID;
            };

            std::atomic<size_t> 
                parallel_allocator_index{ 0 };
            std::array<ABA_Problem::BlockAlloc<innerType, num_items, skipInitialization>, num_parallel_allocators> 
                TLS_arr{};

        public:
            Allocator() = default;
            ~Allocator() = default;

            void unsafe_unload() {
                for (auto& x : TLS_arr)
                    x.unsafe_unload();
            };

            template <typename... TArgs> __declspec(noinline) _type_* Alloc(TArgs&&... a) {
                size_t thisThreadIndex = ++parallel_allocator_index % num_parallel_allocators;
                innerType* out;
                if constexpr (sizeof...(a) > 0) {
                    out = TLS_arr[thisThreadIndex].Alloc(innerType{ _type_{std::forward<TArgs>(a)...}, thisThreadIndex });
                }
                else {
                    out = TLS_arr[thisThreadIndex].Alloc();
                    out->threadID = thisThreadIndex;
                }
                return (_type_*)(out);
            };
            __declspec(noinline) void Free(const _type_* t) {
                innerType* impl = static_cast<innerType*>(static_cast<void*>(const_cast<_type_*>(t)));
                TLS_arr[impl->threadID].Free(impl);
            };
            template <typename... TArgs> std::shared_ptr< _type_ > AllocShared(TArgs&&... a) {
                return std::shared_ptr<_type_>(Alloc(std::forward<TArgs>(a)...), [this](_type_* p) { Free(p); });
            };
        };

        template <typename _type_>
        class EpochAllocator {
        private:
            class TLS {
            public:
                long long
                    _scope_count;
                std::atomic<long long>
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
                    return EpochLimit.load();
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
                    EpochAllocator* _parent_parent;
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
                    EpochGuard(EpochAllocator* parent_parent, TLS* parent, long long CurrentEpoch) : _parent_parent{ parent_parent }, _parent{ parent }, _CurrentEpoch{ CurrentEpoch } {
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
            Allocator<_type_> // , 32
                _alloc;
            moodycamel::ConcurrentQueue<std::pair<long long, _type_*>>
                _delete_list; // note that these are NOT available for re-use yet -- these may still be being used by certain threads. 
            GoodLang::ThreadLocalInstance<TLS>
                _TLS;
            std::atomic<long long> 
                _lastGC;

            // Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
            void RunGC()  {
                static constexpr auto duration{ std::chrono::milliseconds(1) };
                static thread_local std::pair<long long, _type_*> out{};
                auto currentGC{ std::chrono::milliseconds(GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch()) };

                if ((currentGC - std::chrono::milliseconds(_lastGC.load())) > duration) {
                    _lastGC.store(currentGC.count());

                    long long _EpochLimit{ std::numeric_limits<long long>::max() };

                    _TLS.for_each([&_EpochLimit](TLS& _tls) {
                        long long L = _tls.EpochLimit.load();
                        if (L >= 0) {
                            _EpochLimit = std::min<long long>(_EpochLimit, L);
                        }
                    });

                    if ((_EpochLimit > 0) && (_EpochLimit < std::numeric_limits<long long>::max())) {
                        while (_delete_list.try_pop(out)) {
                            if (out.first < _EpochLimit) {
                                // deemed safe to delete
                                _alloc.Free(out.second);
                            }
                            else {
                                // deemed unsafe to delete just yet
                                _delete_list.push(out); // pushing to the end of the queue is lazy deferred sorting -- literally wasting time and hoping it'll be sorted later-on.
                                break;
                            }
                        }
                    }
                }

            };

        public:
            using GuardType = typename TLS::EpochGuard;

            EpochAllocator() = default;
            ~EpochAllocator() = default;

            void unsafe_unload() {
                _alloc.unsafe_unload();
            };

        public:
            GuardType ProtectCurrentEpoch() const {
                return TLS::EpochGuard(const_cast<EpochAllocator*>(this), const_cast<TLS*>(&*_TLS), GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch());
            };

            // Request a new memory pointer
            template <typename... TArgs> _type_* Alloc(TArgs &&... a) {
                return _alloc.Alloc(std::forward<TArgs>(a)...);
            };

            // Frees the memory pointer
            void Free(const _type_* element) {
                _delete_list.push({ GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch(), const_cast<_type_*>(element) });                
                if (_TLS->EpochCheck(GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch())) {
                    // will only succeed if we are in scope-level 0, which only happens if this thread has not made any protecting guards.
                    RunGC();
                }
            };

        };

    };

    // Multi-threaded socket system for adding/removing "listeners" in parallel based on tickets, provided by the TicketDispensor.
    // Tickets should be kept as small as possible and re-used as much as possible, to reduce the size of the sockets, which significantly impacts performance.
    template <typename T> class Callback {
    public:
        class ScopedListener {
        public:
            ScopedListener()
                : _index(0), _parent(nullptr) {};
            ScopedListener(size_t index, Callback& parent) 
                : _index(index), _parent(&parent) {};
            ScopedListener(ScopedListener const& rhs) = delete;
            ScopedListener(ScopedListener&& rhs) 
                : _index(std::move(rhs._index)), _parent(std::move(rhs._parent)) 
            {
                rhs._index = 0;
            };
            ScopedListener& operator=(ScopedListener const& rhs) = delete;
            ScopedListener& operator=(ScopedListener&& rhs) 
            {
                if (_index > 0)
                    _parent->remove_listener(_index);

                _index = std::move(rhs._index);
                _parent = std::move(rhs._parent);
                rhs._index = 0;

                return *this;
            };            
            ~ScopedListener() {
                if (_index > 0)
                    _parent->remove_listener(_index);
            };

        private:
            size_t _index;
            Callback* _parent;
        };

    private:
        struct Wrap { 
            long alive;
            long count;
            T* ptr;
            size_t call_version;
        };

        static size_t&
            _call_version() {
            static size_t call_version{ 0 };
            return call_version;
        };
        size_t
            _size{ 0 };
        concurrency::concurrent_vector<Wrap>
            _listeners;
        void (T::*_callback)(long*, size_t);
        std::atomic<bool>
            alive{ false };

        // add a listener to the list
        __declspec(noinline) void add_listener(size_t index, T* p) {
            if (alive.load()) {
                if (_size <= index) {
                    if (_listeners.size() <= index) (void)_listeners.grow_to_at_least((index + 2) + ((index + 2) % 16));
                    // InterlockedIncrement(static_cast<volatile size_t*>(&_size)); // 
                    InterlockedExchange(static_cast<volatile size_t*>(&_size), index);
                }
                Wrap& wrap = _listeners[index - 1];
                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&wrap.ptr), static_cast<PVOID>(p));
                InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1 << 8);
                InterlockedIncrement(static_cast<volatile long*>(&wrap.alive));
            }
        };
        // remove a listener from the list
        __declspec(noinline) void remove_listener(size_t index) {
            if (alive.load() && _listeners.size() >= index) {
                Wrap& wrap = _listeners[index - 1];
                InterlockedDecrement(static_cast<volatile long*>(&wrap.alive));
                if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), -(1 << 8)) == 0) {}
                else while (wrap.count != 0) if (!wrap.ptr) InterlockedExchange(static_cast<volatile long*>(&wrap.count), 0);
                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&wrap.ptr), static_cast<PVOID>(nullptr));
            }
        };

    public:
        Callback(void (T::*listener)(long*, size_t))
            : _callback{ listener }, alive{ true }
        {};
        ~Callback() {
            alive = false;
            _listeners.clear();            
        };

        ScopedListener listener(size_t index, T* p) {
            add_listener(index, p);
            return ScopedListener(index, *this);
        };
        // callback performed on all listeners
        __declspec(noinline) void speak(long* parent_alive, size_t call_number = 0) {
            if (call_number == 0) 
                call_number = InterlockedIncrement(static_cast<volatile size_t*>(&_call_version()));

            for (size_t i = 0; i < _size; ++i) {
                Wrap& wrap = _listeners[i];
                if (wrap.alive) {
                    if (!parent_alive || *parent_alive) {
                        if (wrap.call_version >= call_number) { continue; }
                        else {
                            InterlockedExchange(static_cast<volatile size_t*>(&wrap.call_version), call_number);
                        }

                        if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1) >= (1 << 8))
                            (wrap.ptr->*_callback)(&wrap.alive, call_number); // _callback(wrap.ptr, &wrap.alive);
                        InterlockedAdd(static_cast<volatile long*>(&wrap.count), -1);
                    }
                    else break;
                }                
            }            
        };
    };

    class ObjectWrapper {
    public:
        enum ObjectState {
            Normal = 0,
            Static = 1,
            Constant = 2
        };

        ObjectWrapper(GoodLang::Any const& obj = {}, int s = 0)
            : object{ std::make_shared<GoodLang::Any>(obj) }
            , state{ s }
        {
            if (object->GetFlag(GoodLang::AnyData::Flag::constant)) {
                state = state | Constant;
            }
            if (is_const()) {
                object->SetFlag(GoodLang::AnyData::Flag::constant, true);
            }
        };

        std::shared_ptr<GoodLang::Any> object;
        int state = 0;

        bool is_const() const {
            return state & Constant;
        };
        bool is_static() const {
            return state & Static;
        };
    };

    // Manages tickets in the range of [1, INF) and assumes ticket 0 is already given to the owner of TicketDispensor
    // Prints new tickets as needed, but recycles old tickets as much as possible. 
    class TicketDispensor {
    public:
        class ScopedTicket {
        public:
            ScopedTicket()
                : _index(0), _parent(nullptr) {};
            ScopedTicket(size_t index, TicketDispensor& parent)
                : _index(index), _parent(&parent) {};
            ScopedTicket(ScopedTicket const& rhs) = delete;
            ScopedTicket(ScopedTicket&& rhs)
                : _index(std::move(rhs._index)), _parent(std::move(rhs._parent))
            {
                rhs._index = 0;
            };
            ScopedTicket& operator=(ScopedTicket const& rhs) = delete;
            ScopedTicket& operator=(ScopedTicket&& rhs)
            {
                _index = std::move(rhs._index);
                _parent = std::move(rhs._parent);
                rhs._index = 0;
                return *this;
            };
            ~ScopedTicket() {
                if (_index)
                    _parent->return_ticket(_index);
            };

            size_t _index;
            TicketDispensor* _parent;            
        };
            
    public:
        moodycamel::ConcurrentQueue<size_t>
            queue{};
        std::atomic<size_t>
            indexes{ 0 };

    public:
        size_t num_tickets() const {
            return indexes.load() + 1;
        };
        __declspec(noinline) ScopedTicket get_scoped_ticket() {
            return ScopedTicket(get_ticket(), *this);
        };
        __declspec(noinline) size_t get_ticket() {
            size_t out;
            if (!queue.try_pop(out)) {
                out = ++indexes;
            }
            return out;
        };
        __declspec(noinline) void return_ticket(size_t ticket) {
            queue.push(ticket);
        };
        void reserve(int n) {
            std::vector<size_t> tickets;
            tickets.reserve(n);

            for (int i = 0; i < n; i++) {
                tickets.push_back(this->get_ticket());
            }
            for (auto& x : tickets) {
                this->return_ticket(x);
            }
        };
    };

    /* Thread-safe ordered B-Tree, which guarrantees valid and safe access to 
    pointers during erasure or modification of the tree when using the Epoch-guard 
    protection, which will delay actual deletion until the guard is satisfactorily old. */
    template< class objType, class keyType, int maxChildrenPerNode = 10> class BTree {
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
        utilities::ABA_Problem::EpochAllocator<objType>
            objAllocator;
        utilities::ABA_Problem::EpochAllocator<_iterType>
            nodeAllocator;
        GoodLang::fast_shared_mutex // mutable std::shared_mutex // 
            mutex;

        class EpochGuard {
        private:
            typename decltype(BTree::objAllocator)::GuardType guard_1;
            typename decltype(BTree::nodeAllocator)::GuardType guard_2;

        public:
            EpochGuard(BTree const* parent) : guard_1{ parent->objAllocator.ProtectCurrentEpoch() }, guard_2{ parent->nodeAllocator.ProtectCurrentEpoch() } {};
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

        BTree() 
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
        ~BTree() = default;

        void unsafe_unload() {
            objAllocator.unsafe_unload();
            nodeAllocator.unsafe_unload();
            root = first = last = nullptr;
            Num = 0;
        };

        template <bool EmplaceIfExists = true> _iterType* 
            Add(objType const& object, keyType const& key) {
            _iterType
                *node, 
                *child, 
                *newNode; 

            // check that the key does not already exist		
            if constexpr (EmplaceIfExists) {
                auto locked{ std::shared_lock(mutex) };
                node = NodeFind(key, root);
                if (node && node->object) {
                    *node->object = object;
                    return node;
                }
            }

            auto locked{ std::scoped_lock(mutex) };

            // check that the key does not already exist		
            if constexpr (EmplaceIfExists) {
                node = NodeFind(key, root);
                if (node && node->object) {
                    *node->object = object;
                    return node;
                }
            }

            newNode = AllocNode();
            newNode->key = key;
            newNode->object = objAllocator.Alloc(object);
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
            newNode->object = objAllocator.Alloc();
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
                newNode->object = objAllocator.Alloc(begin->second);
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
                    objAllocator.Free(node->object);
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
                *child, 
                *newNode;

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

            // add the new child to the parent before the split node
            assert(node->parent->numChildren < maxChildrenPerNode);

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

    // fast, thread-safe sorted map. Allows simultaneous reading / writing / erasure. 
    template<class KeyType, class ValueType> class atomic_map {
        friend class it_state;
    protected:
        BTree<ValueType, KeyType>
            tree;

    public:
        class WrappedReference {
        private:
            typename BTree<ValueType, KeyType>::GuardType
                guard;

        public:
            const KeyType&
                first;
            ValueType&
                second;

            // WrappedReference() = delete;
            WrappedReference(const KeyType& _first, ValueType& _second, BTree<ValueType, KeyType>* _parent)
                : first{ _first }
                , second{ _second }
                , guard{ _parent->ProtectCurrentEpoch() }
            {};
            WrappedReference(WrappedReference const&) = delete;
            WrappedReference(WrappedReference &&) = delete;
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
            tree.unsafe_unload();
        };
        auto // Protect future member function calls from deleting node or object pointers until some time after this object expires.
            ProtectCurrentEpoch() const {
            return tree.ProtectCurrentEpoch();
        };
        size_t // returns the current number of objects in the container. Thread-safe, but out-of-date immediately after the call is made. 
            size() const {
            return tree.GetNodeCount();
        };
        WrappedReference // if already exists, returns the existing value pair. 
            insert(const KeyType& time, ValueType&& value) {
            auto g{ tree.ProtectCurrentEpoch() };
            auto* iter = tree.Add<false>(std::move(value), time);
            return WrappedReference(iter->key, *iter->object, &tree);
        };
        WrappedReference // if already exists, overwrites the value and returns the value pair. 
            emplace(const KeyType& time, ValueType&& value) {
            auto g{ tree.ProtectCurrentEpoch() };
            auto* iter = tree.Add<true>(std::move(value), time);
            return WrappedReference(iter->key, *iter->object, &tree);
        };
        template <typename iter_type> void // bulk insertion. if already exists, does nothing.
            insert_bulk(iter_type begin, iter_type const& end) {
            auto g{ tree.ProtectCurrentEpoch() };
            tree.Add_Bulk<iter_type, false>(std::move(begin), end);
        };
        template <typename iter_type> void // bulk insertion. if already exists, overwrites the value. 
            emplace_bulk(iter_type begin, iter_type const& end) {
            auto g{ tree.ProtectCurrentEpoch() };
            tree.Add_Bulk<iter_type, true>(std::move(begin), end);
        };
        size_t // returns 1 if the key is found, otherwise 0.
            count(const KeyType& time) const {
            return (bool)tree.NodeFind(time) ? 1 : 0;
        };
        ValueType& // throws if the key is not found. 
            at(const KeyType& time) const {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* iter = tree.NodeFind(time)) {
                return *iter->object;
            }
            else {
                throw std::range_error("Could not find key");
            }
        };
        ValueType* // returns nullptr if the key is not found. 
            try_at(const KeyType& time) const {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* iter = tree.NodeFind(time)) {
                return iter->object;
            }
            else {
                return nullptr;
            }
        };
        template <typename Func> __declspec(noinline) bool // calls func(key, object) on the first (smallest key) node in the map
            do_at_beginning(Func const& func) const {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* p = tree.GetFirst()) {
                func(p->key, *p->object);
                return true;
            }
            return false;
        };
        template <typename Func> __declspec(noinline) void // calls func(key, object) on all nodes in the map
            for_all(Func const& func) const {
            auto g{ tree.ProtectCurrentEpoch() };
            auto p = tree.GetFirst();
            while (p) {
                func(p->key, *p->object);
                p = tree.GetNextLeaf(p);
            }
        };
        template <typename Func> __declspec(noinline) bool // calls func(key, object) on the last (largest key) node in the map
            do_at_end(Func const& func) const {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* p = tree.GetLast()) {
                func(p->key, *p->object);
                return true;
            }
            return false;
        };
        bool // removes the first (smallest key) node in the map
            pop_front() {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* p = tree.GetFirst()) {
                tree.Remove(p);
                return true;
            }
            return false;
        };
        bool // removes the last (largest key) node in the map
            pop_back() {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* p = tree.GetLast()) {
                tree.Remove(p);
                return true;
            }
            return false;
        };
        template <typename Func> __declspec(noinline) bool // removes the first (smallest key) node in the map if func(key, object) returns true
            pop_front_if(Func const& func) {
            bool out = false;
            auto g{ tree.ProtectCurrentEpoch() };     
            auto g2{ tree.Lock() };
            if (auto* p = tree.GetFirst_Unsafe()) {                
                if (func(p->key, *p->object)) {
                    tree.Remove_Unsafe(p, nullptr);
                    out = true;
                }  
            }            
            return out;
        };
        template <typename Func> __declspec(noinline) bool // removes the last (largest key) node in the map if func(key, object) returns true
            pop_back_if(Func const& func) {
            bool out = false;
            auto g{ tree.ProtectCurrentEpoch() };
            auto g2{ tree.Lock() };
            if (auto* p = tree.GetLast_Unsafe()) {
                if (func(p->key, *p->object)) {
                    tree.Remove_Unsafe(p, nullptr);
                    out = true;
                }
            }
            return out;
        };
        __declspec(noinline) ValueType& // if already exists, returns the value. Otherwise, creates the value (default init) and returns the value. May throw under heavy conflict. 
            operator[](const KeyType& time) {
            auto g{ tree.ProtectCurrentEpoch() };
            if (ValueType* p = tree.Find(time)) {                
                return *p;
            }
            else {
                if (auto* p = tree.GetOrInstance(time)) {
                    return *p->object;
                }
                else {
                    throw std::range_error("Could not find key");
                }
            }
        };
        template <typename Func> ValueType& // same as operator[], except it will call the provided function to initialize the value if no value was found. 
            get_or_make(const KeyType& time, Func const& func, bool* ExistedAlready = nullptr) {
            auto g{ tree.ProtectCurrentEpoch() };
            if (ValueType* p = tree.Find(time)) {
                if (ExistedAlready) *ExistedAlready = true;
                return *p;
            }
            else {
                if (ExistedAlready) *ExistedAlready = false;
                if (auto* p = tree.Add(func(), time)) {
                    return *p->object;
                }
                else {
                    throw std::range_error("Could not find key");
                }
            }
        };
        bool // erase the value pair at the specified key. Optionally, can copy the value at the key before erasure.
            erase(const KeyType& time, ValueType* out = nullptr) {
            auto g{ tree.ProtectCurrentEpoch() };
            return tree.RemoveAt(time, out);
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
            using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

            // data
            mutable typename BTree<ValueType, KeyType>::_iterType*
                _ptr{};
            mutable std::unique_ptr<value_type>
                _out;

            // functions
            void Initialize(thisType* ref) {};
            void ToBeginning(thisType* ref) {
                _ptr = ref->tree.GetFirst();
            };
            void ToEnd(thisType* ref) {
                _ptr = nullptr;
            };
            void Next(thisType* ref) {
                this->_ptr = ref->tree.GetNextLeaf(this->_ptr);
            };
            void Prev(thisType* ref) {
                this->_ptr = ref->tree.GetPrevLeaf(this->_ptr);
            };
            value_type& Get(thisType* ref) const {
                _out = std::make_unique<value_type>(_ptr->key, *_ptr->object, &ref->tree);
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
            auto g{ tree.ProtectCurrentEpoch() };
            auto iter = this->end();
            if (auto* p = this->tree.NodeFind(_Keyval)) {
                iter.state._ptr = p;
            }
            return iter;
        };

    };

    // fast, thread-safe unsorted map. Allows simultaneous reading / writing / erasure. 
    template<class KeyType, class ValueType, typename HashType = std::hash<KeyType>> class atomic_unordered_map {
        friend class it_state;
    protected:
        BTree<std::pair<KeyType, ValueType>, size_t>
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
            typename  BTree<std::pair<KeyType, ValueType>, size_t>::GuardType
                guard;

        public:
            const KeyType&
                first;
            ValueType&
                second;

            // WrappedReference() = delete;
            WrappedReference(const KeyType& _first, ValueType& _second, BTree<std::pair<KeyType, ValueType>, size_t>* _parent)
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
        atomic_unordered_map()
            : tree{}, hasher{ HashType{} }
        {};
        atomic_unordered_map(atomic_unordered_map const& rhs) = delete;
        atomic_unordered_map(atomic_unordered_map&& rhs) = delete;
        atomic_unordered_map& operator=(atomic_unordered_map const& rhs) = delete;
        atomic_unordered_map& operator=(atomic_unordered_map&& rhs) = delete;
        ~atomic_unordered_map() = default;

        void unsafe_unload() {
            tree.unsafe_unload();
        };
        auto // Protect future member function calls from deleting node or object pointers until some time after this object expires.
            ProtectCurrentEpoch() const {
            return tree.ProtectCurrentEpoch();
        };
        size_t // returns the current number of objects in the container. Thread-safe, but out-of-date immediately after the call is made. 
            size() const {
            return tree.GetNodeCount();
        };
        WrappedReference // if already exists, returns the existing value pair. 
            insert(const KeyType& time, ValueType&& value) {
            auto g{ tree.ProtectCurrentEpoch() };
            auto* iter = tree.Add<false>({ time, std::move(value) }, hash(time));
            return WrappedReference(iter->object->first, iter->object->second, &tree);
        };
        WrappedReference // if already exists, overwrites the value and returns the value pair. 
            emplace(const KeyType& time, ValueType&& value) {
            auto g{ tree.ProtectCurrentEpoch() };
            auto* iter = tree.Add<true>({ time, std::move(value) }, hash(time));
            return WrappedReference(iter->object->first, iter->object->second, &tree);
        };
        template <typename iter_type> void // bulk insertion. if already exists, does nothing.
            insert_bulk(iter_type begin, iter_type const& end) {
            auto g{ tree.ProtectCurrentEpoch() };
            tree.Add_Bulk<iter_type, false>(std::move(begin), end);
        };
        template <typename iter_type> void // bulk insertion. if already exists, overwrites the value. 
            emplace_bulk(iter_type begin, iter_type const& end) {
            auto g{ tree.ProtectCurrentEpoch() };
            tree.Add_Bulk<iter_type, true>(std::move(begin), end);
        };
        size_t // returns 1 if the key is found, otherwise 0.
            count(const KeyType& time) const {
            return (bool)tree.NodeFind(hash(time)) ? 1 : 0;
        };
        ValueType& // throws if the key is not found. 
            at(const KeyType& time) const {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* iter = tree.NodeFind(hash(time))) {
                return iter->object->second;
            }
            else {
                throw std::range_error("Could not find key");
            }
        };
        ValueType* // returns nullptr if the key is not found. 
            try_at(const KeyType& time) const {
            auto g{ tree.ProtectCurrentEpoch() };
            if (auto* iter = tree.NodeFind(hash(time))) {
                return &iter->object->second;
            }
            else {
                return nullptr;
            }
        };
        template <typename Func> __declspec(noinline) void // calls func(key, object) on all nodes in the map
            for_all(Func const& func) const {
            auto g{ tree.ProtectCurrentEpoch() };
            auto p = tree.GetFirst();
            while (p) {
                func(p->object->first, p->object->second);
                p = tree.GetNextLeaf(p);
            }
        };
        template <typename Func> ValueType& // same as operator[], except it will call the provided function to initialize the value if no value was found. 
            get_or_make(const KeyType& time, Func const& func, bool* ExistedAlready = nullptr) {
            auto g{ tree.ProtectCurrentEpoch() };
            if (ValueType* p = tree.Find(hash(time))) {
                if (ExistedAlready) *ExistedAlready = true;
                return *p;
            }
            else {
                if (ExistedAlready) *ExistedAlready = false;
                if (auto* p = tree.Add(func(), time)) {
                    return *p->object;
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
            auto g{ tree.ProtectCurrentEpoch() };
            std::pair<KeyType, ValueType> temp;
            bool result = tree.RemoveAt(hash(time), &temp);
            if (out) *out = temp.second;
            return result;
        };
        void // clear the map
            clear() {
            while (true) {
                auto g{ tree.ProtectCurrentEpoch() };
                if (auto* p = tree.GetFirst()) {
                    tree.Remove(p);
                }
                else {
                    break;
                }
            }
        };

    private:
        class it_state {
        public:
            using thisType = atomic_unordered_map;
            using value_type = WrappedReference;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

            // data
            mutable typename  BTree<std::pair<KeyType, ValueType>, size_t>::_iterType*
                _ptr{};
            mutable std::unique_ptr<value_type>
                _out;

            // functions
            void Initialize(thisType* ref) {};
            void ToBeginning(thisType* ref) {
                _ptr = ref->tree.GetFirst();
            };
            void ToEnd(thisType* ref) {
                _ptr = nullptr;
            };
            void Next(thisType* ref) {
                this->_ptr = ref->tree.GetNextLeaf(this->_ptr);
            };
            void Prev(thisType* ref) {
                this->_ptr = ref->tree.GetPrevLeaf(this->_ptr);
            };
            value_type& Get(thisType* ref) const {
                _out = std::make_unique<value_type>(_ptr->object->first, _ptr->object->second, &ref->tree);
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
        SETUP_ITERATOR(atomic_unordered_map, it_state);
        iterator // returns an iterator 
            find(const KeyType& _Keyval) const {
            auto g{ tree.ProtectCurrentEpoch() };
            auto iter = this->end();
            if (auto* p = this->tree.NodeFind(hash(_Keyval))) {
                iter.state._ptr = p;
            }
            return iter;
        };

    };

};
namespace std {
    template <> struct hash<utilities::string> {
        std::size_t operator()(const utilities::string& k) const {
            return k.hash();
        };
    };
    template <> struct less<utilities::string> {
        std::size_t operator()(const utilities::string& lhs, const utilities::string& rhs) const {
            return lhs < rhs;
        };
    };
    template <> struct greater<utilities::string> {
        std::size_t operator()(const utilities::string& lhs, const utilities::string& rhs) const {
            return lhs > rhs;
        };
    };
    template <> struct equal_to<utilities::string> {
        std::size_t operator()(const utilities::string& lhs, const utilities::string& rhs) const {
            return lhs == rhs;
        };
    };
};

class Scopes {
private:
    // clean-up the name of a scope: e.g. "::" becomes "::", "Units" becomes "::Units::"
    static utilities::compound_shared_string CleanUpScopeName(utilities::string const& x) {
        utilities::compound_shared_string out("::", x, "::");
        if (x.length() >= 2) {
            if (x.substr(0, 2) == "::") {
                out.a = "";
            }
            if (x.substr(x.length() - 2, 2) == "::") {
                out.c = "";
            }
        }
        return out;
    };

public:
    enum ScopeType {
        Basic = 1,
        Namespace = 2,
        Class = 4,
        Root = 8
    };

public:
    class Breadcrumb;
    class BasicScope;
    class NamespaceScope;
    class ClassScope;
    class RootScope;

private:
    // Identity of an individual scope
    class ScopeID {
    friend class Breadcrumb;
    public:
        utilities::string
            scope_name; // e.g. "Color"
        Scopes::BasicScope*
            scope;
    private:
        std::unique_ptr<utilities::string>
            current_namespace; // e.g. "::" or "::UI::Color::"
        int
            scope_type; // may be a compound of multiple types, e.g. a root is also a namespace

    public:
        ScopeID(utilities::string && scope_name_p = {}, int scope_type_p = ScopeType::Basic)
            : scope_name{ std::move(scope_name_p) }
            , scope{ nullptr }            
            , current_namespace{ nullptr }
            , scope_type{ std::move(scope_type_p) }
        {}
        ScopeID(ScopeID&& rhs) 
            : scope_name{ std::move(rhs.scope_name) }
            , scope{ std::move(rhs.scope) }
            , scope_type{ std::move(rhs.scope_type) }
            , current_namespace{ std::move(rhs.current_namespace) }
        {};
        ScopeID(ScopeID const&) = delete;
        ScopeID& operator=(ScopeID&&) = delete;
        ScopeID& operator=(ScopeID const&) = delete;
        ~ScopeID() = default;

        bool is_namespace() const {
            return scope_type & ScopeType::Namespace;
        };
        bool is_class() const {
            return scope_type & ScopeType::Class;
        };
        bool is_root() const {
            return scope_type & ScopeType::Root;
        };
    };

public:
    // Used to track and hash the current scope position. 
    class Breadcrumb {
    public:
        ScopeID
            this_m; // will always point to the owner node's scope ID
        Breadcrumb*
            parent_m; // may be nullptr for root nodes, otherwise will point to the parent breadcrumb node
        Breadcrumb*
            root_m; // may point to this
        Breadcrumb*
            namespace_m; // may point to this
    private:
        utilities::TicketDispensor::ScopedTicket
            scope_index; // unique index of this scope for check_flags

    public:
        size_t GetScopeIndex() {
            if (scope_index._index == 0) {
                if (parent_m) {
                    if (auto* root_ptr = dynamic_cast<RootScope*>(root_m->this_m.scope)) {
                        InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&scope_index._parent), static_cast<PVOID>(&root_ptr->scope_indexs));
                        auto new_index = root_ptr->scope_indexs.get_ticket();
                        if (InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(&scope_index._index), new_index, 0) > 0) {
                            root_ptr->scope_indexs.return_ticket(new_index);
                        }
                    }
                }
            }
            return scope_index._index;
        };
        std::string_view GetCurrentNamespace() const {
            if (this->this_m.is_namespace()) {
                return this->this_m.current_namespace->substr();
            }
            else {
                return this->namespace_m->this_m.current_namespace->substr();
            }
        };

        Breadcrumb(utilities::string&& name = {}, int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
            : this_m(std::move(name), scope_type_p)
            , parent_m(std::move(parent))
            , root_m{ nullptr }
            , namespace_m{ nullptr }
            , scope_index()
        {
            // ROOT
            if (parent_m) root_m = parent_m->root_m;
            else root_m = this;

            // NAMESPACE
            if (this_m.is_namespace()) namespace_m = this;
            else if (parent_m) namespace_m = parent_m->namespace_m;
            else namespace_m = this->root_m;

            // current_namespace
            if (this_m.scope_name.length() > 0) {
                if (parent_m) {
                    std::string temp_string = parent_m->this_m.current_namespace->to_string() + this_m.scope_name.to_string();
                    auto cleaned_temp_string = CleanUpScopeName(std::string_view(temp_string));
                    this_m.current_namespace = std::make_unique<utilities::string>(cleaned_temp_string);
                }
                else {
                    // this shouldn't happen...
                    this_m.current_namespace = std::make_unique<utilities::string>(CleanUpScopeName(this_m.scope_name));
                }
            }
            else {
                if (parent_m) { /*this_m.current_namespace = parent_m->this_m.current_namespace;*/ }
                else this_m.current_namespace = std::make_unique<utilities::string>(utilities::string::namespace_colons());
            }
        };
        Breadcrumb(Breadcrumb const&) = delete;
        Breadcrumb(Breadcrumb &&) = delete;
        Breadcrumb& operator=(Breadcrumb const&) = delete;
        Breadcrumb& operator=(Breadcrumb&&) = delete;
        ~Breadcrumb() = default;

    };

public:
    // Thread-safe access to a cache of data. While new caches are made, old caches may be deleted safely, protected by Epoch-controlled allocators. 
    template <int numCategories = 4> class Cache {
    private: // CacheVersion -> CacheCategory -> Inputs -> Result
        using ResultType = Breadcrumb*;
        using InputType = size_t;
        using ResultForInputType = concurrency::concurrent_unordered_map<InputType, ResultType>; // only emplaces, never deletes, so concurrent_unordered_map should be OK. 
        utilities::atomic_map<size_t, std::array<utilities::DelayedInstantiation<ResultForInputType>, numCategories>>
            _current_cache; // cache uses atomic_map since it may delete items as well as append items. Needs to be sorted since we "pop" the first item frequently. 

    public:
        Cache() = default;
        Cache(Cache const&) = delete;
        Cache(Cache &&) = delete;
        Cache& operator=(Cache const&) = delete;
        Cache& operator=(Cache&&) = delete;
        ~Cache() = default;

        void unsafe_unload() {
            _current_cache.unsafe_unload();
        };

        // Insert an item into the cache.
        template<int category> void EmplaceCache(size_t cache_version, size_t input_hash, Breadcrumb* result) {
            auto g{ _current_cache.ProtectCurrentEpoch() };
            bool success = false;
            while (!success) {
                if (!_current_cache.do_at_end([&](size_t curr_version, std::array<utilities::DelayedInstantiation<ResultForInputType>, numCategories>& cache) {
                    if (curr_version >= cache_version) {
                        InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&cache[category]->operator[](input_hash)), reinterpret_cast<PVOID>(result));
                        success = true;
                    }
                })) {};
                if (!success) {
                    (void)_current_cache.operator[](cache_version); // default-initializes the item at the specified index if it does not already exist. 
                    _current_cache.pop_front_if([&](size_t curr_version, std::array<utilities::DelayedInstantiation<ResultForInputType>, numCategories>& cache) -> bool {
                         return curr_version < cache_version;
                    });
                }
            }
        };

        // Try to copy an item from the cache.
        template<int category> Breadcrumb* TryGetCache(size_t cache_version, size_t input_hash) {
            auto g{ _current_cache.ProtectCurrentEpoch() };
            Breadcrumb* out{ nullptr };
            _current_cache.do_at_end([&](size_t curr_version, std::array<utilities::DelayedInstantiation<ResultForInputType>, numCategories>& cache) {
                if (curr_version >= cache_version) {
                    out = cache[category]->operator[](input_hash);
                }
            });
            return out;
        };

    };

    /// <summary>
    /// Foundational element of a scope. Should not be created on its own, and instead should be issued by a parent.
    /// </summary>
    class BasicScope {
    friend class NamespaceScope;
    friend class RootScope;
    friend class Breadcrumb;
    protected:
        Breadcrumb 
            breadcrumb_m;
        utilities::DelayedInstantiation<concurrency::concurrent_unordered_map<Breadcrumb*, utilities::Callback<NamespaceScope>::ScopedListener>>
            using_m; // NOTE: calling "using" should split a normal, BasicScope - e.g. using statements are appended staticly at compile time, NOT at runtime. 
        utilities::DelayedInstantiation<concurrency::concurrent_unordered_map< utilities::string, utilities::ObjectWrapper >>
            objects_m; // NOTE: adding objects should be appended staticly at compile time, NOT at runtime. E.g. the names are known, even if the types are not yet known. 

        virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) {};
        bool EmplaceObject_Impl(utilities::string const& sv, utilities::ObjectWrapper Obj, bool overwriteIfExists = true) {
            auto iter = objects_m->insert({ sv, Obj });
            if (iter.second) {
                this->invalidate_cache();
                return true;
            }
            else if (overwriteIfExists) {
                iter.first->second = Obj;
                this->invalidate_cache();
                return true;
            }
            else {
                return false;
            }
        };
        utilities::ObjectWrapper* GetObject_Impl(utilities::string const& sv) {
            if (objects_m) {
                if (auto f = objects_m->find(sv), e = objects_m->end(); f != e) {
                    return &f->second;
                }
            }
            return nullptr;
        };
        virtual void AddUsing_Impl(Breadcrumb* scope) {
            if (scope) {
                if (auto f = using_m->find(scope); f != using_m->end()) {
                    using_m->insert(std::pair<Breadcrumb*, utilities::Callback<NamespaceScope>::ScopedListener>{ scope, utilities::Callback<NamespaceScope>::ScopedListener() });
                    invalidate_cache();
                }
            }
        };

    protected:
        BasicScope(utilities::string&& name, int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
            : breadcrumb_m(std::move(name), scope_type_p, parent)
        {
            breadcrumb_m.this_m.scope = this;
        };

    public:
        BasicScope() = delete;
        virtual ~BasicScope() = default;

        /// <summary>
        /// Make a child scope from this scope. 
        /// Thread-safe, and allowed to make many children of this scope in parallel safely. 
        /// The created scope (and its children) are destroyed at the end of this C++ scope. 
        /// </summary>
        BasicScope make_scope() const {
            return BasicScope("", Scopes::ScopeType::Basic, const_cast<Breadcrumb*>(&this->breadcrumb_m));
        };
        
        /// <summary>
        /// Instruct this scope to "use" the provided namespace when searching for objects, functions, or other scopes by name. 
        /// If this scope is a namespace, this will reset the search cache.
        /// </summary>
        /// <param name="ptr"></param>
        /// <returns></returns>
        void add_using_here(NamespaceScope const& ptr) {
            if (auto p = static_cast<const BasicScope*>(&ptr)) {
                this->AddUsing_Impl(const_cast<Breadcrumb*>(&p->breadcrumb_m));
            }
        }
        
        /// <summary>
        /// Insert an object into this scope only if it does not yet exist. Does not search neighbors or review the object name.
        /// </summary>
        /// <param name="sv"></param>
        /// <param name="Obj"></param>
        /// <returns>bool</returns>
        bool insert_object_here(utilities::string const& sv, utilities::ObjectWrapper Obj) {
            return this->EmplaceObject_Impl(sv, std::move(Obj), false);
        };
        
        /// <summary>
        /// Emplace an object into this scope whether or not it exists. Does not search neighbors or review the object name.
        /// </summary>
        /// <param name="sv"></param>
        /// <param name="Obj"></param>
        /// <returns>bool</returns>
        bool emplace_object_here(utilities::string const& sv, utilities::ObjectWrapper Obj) {
            return this->EmplaceObject_Impl(sv, std::move(Obj), true);
        };
        
        /// <summary>
        /// Try to find an object in this scope. Does not search neighbors or review the object name.
        /// Since objects cannot be removed, it safely returns a pointer. 
        /// </summary>
        /// <returns>utilities::ObjectWrapper*</returns>
        utilities::ObjectWrapper* find_object_here(utilities::string const& sv) const {
            return const_cast<BasicScope*>(this)->GetObject_Impl(sv);
        };

    };

    /// <summary>
    /// A special type of scope that serves as the "nodes" in the script tree, hosting functions and the caches. 
    /// Should not be created on its own, and instead should be issued by a parent.
    /// </summary>
    class NamespaceScope : public BasicScope {
    friend class BasicScope;
    friend class RootScope;
    friend class Breadcrumb;
    protected:
        // explicit children namespaces, with strongly-held protections to their memory.
        utilities::atomic_map<size_t, std::shared_ptr<NamespaceScope>>
        // concurrency::concurrent_unordered_map<size_t, std::shared_ptr<NamespaceScope>>
            children;

    protected:
        Scopes::Cache<4> 
            search_cache; // while thread-safe, it does seem to singificantly decrease the performance of creating new BasicScope's, hence moving it here. 

    protected:
        utilities::Callback<NamespaceScope>
            sockets_for_cache_versions;
        concurrency::concurrent_vector<utilities::Callback<NamespaceScope>::ScopedListener>
            connection_for_cache_version;
    
        size_t
            object_or_function_versions;
    public:
        virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) override {
            InterlockedIncrement(static_cast<volatile size_t*>(&object_or_function_versions));
            sockets_for_cache_versions.speak(parent_alive, call_number);
        };

    protected:
        virtual void AddUsing_Impl(Breadcrumb* scope) override {
            if (scope) {
                if (auto* ns_ptr = scope->namespace_m) {
                    if (ns_ptr->this_m.is_namespace()) {
                        if (auto* p = dynamic_cast<NamespaceScope*>(ns_ptr->this_m.scope)) {
                            // suddenly, we require our scope index, now that we are "using" a namespace
                            using_m->insert(std::pair<Breadcrumb*, utilities::Callback<NamespaceScope>::ScopedListener>{ scope, p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this) });
                            invalidate_cache();
                        }
                    }
                }
            }
        };

    protected:
        // instancing a child namespace should only be done from an existing namespace
        NamespaceScope(utilities::string&& name, int scope_type_p = ScopeType::Basic & ScopeType::Namespace, Breadcrumb* parent = nullptr)
            : BasicScope(std::move(name), scope_type_p, parent)
            , children{}
            , search_cache{}
            , sockets_for_cache_versions(&NamespaceScope::invalidate_cache)
            , connection_for_cache_version{}
            , object_or_function_versions{ 0 }
        {
            if (this->breadcrumb_m.parent_m) {
                if (auto* p = dynamic_cast<NamespaceScope*>(this->breadcrumb_m.parent_m->namespace_m->this_m.scope)) {
                    connection_for_cache_version.push_back(p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this));
                }
            }
        };
        // unloads the connections to other namespaces before deletion, which can prevent a memory-access crash. 
        void unload() {
            for (auto& x : this->connection_for_cache_version) x = {};
            for (auto& x : *this->using_m) x.second = {};
            this->search_cache.unsafe_unload();
            this->using_m->clear();
            this->connection_for_cache_version.clear();
            for (auto& child : this->children) {
                child.second->unload();
            }
            this->children.unsafe_unload();
        };

    public:
        NamespaceScope() = delete;
        virtual ~NamespaceScope() {
            unload();
        };

        /// <summary>
        /// Finds or creates a new namespace scope as a child of this one, and keeps it in memory. 
        /// The created scope will survive for the life of this parent scope. 
        /// If a namespace already exists with the provided name, it will return the existing namespace without creating a new one or overwritting the existing one.
        /// </summary>
        /// <returns>NamespaceScope</returns>
        NamespaceScope& make_namespace(utilities::string const& name) {
            // return NamespaceScope((utilities::string)name, Scopes::ScopeType::Basic | Scopes::ScopeType::Namespace, const_cast<Breadcrumb*>(&this->breadcrumb_m));

            if (auto f = children.find(name.hash()); f != children.end()) {
                return *f->second;
            }
            else {
                return *children.insert(name.hash(), std::shared_ptr<NamespaceScope>(new NamespaceScope((utilities::string)name, Scopes::ScopeType::Basic | Scopes::ScopeType::Namespace, const_cast<Breadcrumb*>(&this->breadcrumb_m)))).second;
            }
        };


    };

    /// <summary>
    /// The only scope that should be instanced on its own. 
    /// </summary>
    class RootScope : public NamespaceScope {
    friend class BasicScope; 
    friend class NamespaceScope;     
    friend class Breadcrumb;
    protected:
        // When a scope is born it will get the smallest-possible unique index for itself. 
        // This "ticket" or unique index will be unique to 
        utilities::TicketDispensor 
            scope_indexs;

    public:
        RootScope() 
            : NamespaceScope("::", ScopeType::Basic & ScopeType::Namespace & ScopeType::Root, nullptr) 
        {};
        virtual ~RootScope() {
            this->unload();
        };

    };

};

int main() {
    using namespace utilities;
    using namespace ABA_Problem;
    Stopwatch sw;

    while (true) {
        print("STARTING LOOP: \n");

#if 0
        if (1) {
            ABA_Problem::Allocator<size_t>
                index_allocator;

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto* p = index_allocator.Alloc((size_t)i);
                //Sleep(1);
                index_allocator.Free(p);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            std::vector<size_t*> ptrs(1000000, nullptr);
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                ptrs[i] = index_allocator.Alloc((size_t)i);
                });
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(ptrs[i]);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        print("");
        if (1) {
            GoodLang::Allocator<size_t>
                index_allocator;

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                index_allocator.Free(index_allocator.Alloc(i));
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(index_allocator.Alloc(i));
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto* p = index_allocator.Alloc(i);
                //Sleep(1);
                index_allocator.Free(p);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            std::vector<size_t*> ptrs(1000000, nullptr);
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                ptrs[i] = index_allocator.Alloc((size_t)i);
                });
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(ptrs[i]);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        print("");
        if (0) {
            GoodLang::details::BTreeAllocator<size_t>
                index_allocator;

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                index_allocator.Free(index_allocator.Alloc(i));
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(index_allocator.Alloc(i));
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto* p = index_allocator.Alloc(i);
                // Sleep(1);
                index_allocator.Free(p);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            std::vector<size_t*> ptrs(1000000, nullptr);
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                ptrs[i] = index_allocator.Alloc((size_t)i);
                });
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(ptrs[i]);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        print("");
        if (1) {
            ABA_Problem::EpochAllocator<size_t>
                index_allocator;

            if (1) {
                auto iterator = GoodLang::CustomizedSequence(std::function([](size_t pos) -> std::pair<size_t, size_t> {
                    return { pos, pos };
                }), 1000000ull);
                utilities::atomic_map<size_t, size_t> map;
                map.insert_bulk(iterator.begin(), iterator.end());
                EXPECT_EQ(map.size(), 1000000ull);
            }

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto* p = index_allocator.Alloc((size_t)i);
                //Sleep(1);
                index_allocator.Free(p);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            std::vector<size_t*> ptrs(1000000, nullptr);
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                ptrs[i] = index_allocator.Alloc((size_t)i);
                });
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(ptrs[i]);
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        print("");
        if (1) {
            ABA_Problem::EpochAllocator<size_t>
                index_allocator;

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto g{ index_allocator.ProtectCurrentEpoch() };
                auto* p = index_allocator.Alloc((size_t)i);
                index_allocator.Free(p);
                ++* p;
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        print("");
        if (1) {
            GoodLang::EpochProtectedAllocator<size_t, 1>
                index_allocator;

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                index_allocator.Free(index_allocator.Alloc((size_t)i));
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto g{ index_allocator.CreateEpochGuard() };
                auto* p = index_allocator.Alloc((size_t)i);
                index_allocator.Free(p);
                ++* p;
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        print("");

        if (1) {
            ABA_Problem::EpochAllocator<size_t>
                index_allocator;
            auto g{ index_allocator.ProtectCurrentEpoch() };
            for (int i = 0; i < 1000000; ++i) {
                index_allocator.Free(index_allocator.Alloc());
            }
        }


        sw.Start();
        if (1) {
            BTree<std::string, size_t, 10> tree{};
            for (char c = 'a'; c <= 'z'; ++c) {
                EXPECT_EQ(nullptr, tree.Find((int)c));
            }
            for (char c = 'a'; c <= 'z'; ++c) {
                tree.Add(std::string(1, c), (int)c);
                //print(*tree.Find((int)c));
            }
            for (char c = 'Z'; c >= 'A'; --c) {
                EXPECT_EQ(nullptr, tree.Find((int)c));
            }
            for (char c = 'A'; c <= 'Z'; ++c) {
                tree.Add(std::string(1, c), (int)c);
               // print(*tree.Find((int)c));
            }
            for (char c = '0'; c <= '9'; ++c) {
                EXPECT_EQ(nullptr, tree.Find((int)c));
            }
            for (char c = '0'; c <= '9'; ++c) {
                tree.Add(std::string(1, c), (int)c);
                //print(*tree.Find((int)c));
            }
            for (char c = '0'; c <= '9'; ++c) {
                tree.Add(std::string("OVERWRITTEN: ") + std::string(1, c), (int)c);
                //print(*tree.Find((int)c));
            }
            for (char c = 'A'; c <= 'Z'; ++c) {
                tree.Remove(tree.NodeFind((int)c));
                EXPECT_EQ(nullptr, tree.Find((int)c));
            }
        }
        if (1) {
            BTree<std::string, size_t, 10> tree{};
            GoodLang::parallel::For(0, 255, [&](int i) {
                tree.Add(GoodLang::ToString(i), i);
                });
            auto g{ tree.ProtectCurrentEpoch() };
            for (auto* iter = tree.GetFirst(); iter; iter = tree.GetNextLeaf(iter)) {
                //print(iter->key);
            }
        }
        if (1) {
            BTree<std::string, size_t, 10> tree{};
            GoodLang::parallel::For(0, 255, [&](int i) {
                tree.Add(GoodLang::ToString(i), i);
                if (auto* p = tree.Find(i)) {
                    EXPECT_EQ(GoodLang::ToString(i), *p);
                }
                else {
                    EXPECT_EQ(true, false);
                }
            });
            auto g{ tree.ProtectCurrentEpoch() };
            for (auto* iter = tree.GetFirst(); iter; iter = tree.GetNextLeaf(iter)) {
                //print(iter->key);
            }
        }
        if (1) {
            BTree<std::string, size_t, 10> tree{};
            GoodLang::parallel::For(0, 255, [&](int i) {
                tree.Add(GoodLang::ToString(i), i);
                tree.Remove(tree.NodeFind(i));
                EXPECT_EQ(nullptr, tree.Find(i));
            });
            auto g{ tree.ProtectCurrentEpoch() };
            for (auto* iter = tree.GetFirst(); iter; iter = tree.GetNextLeaf(iter)) {
                //print(iter->key);
            }
        }
        if (1) {
            BTree<std::string, size_t, 10> tree{};
            if (auto thread_ptr = GoodLang::parallel::AsThread([&tree]() {
                for (int i = 0; i < 254; ++i) {
                    auto g{ tree.ProtectCurrentEpoch() };
                    tree.Remove(tree.NodeFind(i));
                };
            })) {
                GoodLang::parallel::For(0, 255, [&](int i) {
                    auto g{ tree.ProtectCurrentEpoch() };
                    tree.Add(GoodLang::ToString(i), i);
                    if (auto* p = tree.NodeFind(i)) {
                        // print(p->key);
                    }
                });
                thread_ptr = nullptr;
            }

            auto g{ tree.ProtectCurrentEpoch() };
            for (auto* iter = tree.GetFirst(); iter; iter = tree.GetNextLeaf(iter)) {
                //print(iter->key);
            }
        }
        print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            std::atomic<long> S{ 0 };
            sw.Start();
            GoodLang::parallel::While(
                [&](void)-> bool {
                    return tree.size() < 100;
                }, 
                [&](void) -> void {                    
                    long s = ++S;
                    tree.insert(s, GoodLang::printf("%i", (int)s));
                }
            );
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            std::atomic<long> S{ 0 };
            sw.Start();
            while (tree.size() < 1000000){
                long s = ++S;
                tree.insert(s, GoodLang::printf("%i", (int)s));
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            std::atomic<long> S{ 0 };
            sw.Start();
            GoodLang::parallel::While(
                [&](void)-> bool {
                    return tree.size() < 1000000;
                },
                [&](void) -> void {
                    long s = ++S;
                    tree.insert(s, GoodLang::printf("%i", (int)s));
                }
            );
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            std::atomic<long> S{ 0 };
            sw.Start();
            GoodLang::parallel::Until(
                [&](void) -> void {
                    long s = ++S;
                    tree.insert(s, GoodLang::printf("%i", (int)s));
                },
                [&](void)-> bool {
                    return tree.size() >= 1000000;
                }
            );
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        // Testing an extreme example of multithreading bullshit causing softlocks, that has hopefully been fixed. 
        if (1) {            
            utilities::atomic_map<size_t, std::string> tree{};
            sw.Start();
            if (auto thread_ptr = GoodLang::parallel::AsThread([&tree]() {
                // this thread is using one of the GoodLang threads, and will be highly performant
                for (int i = 0; i < 1000; ++i) {
                    (void)tree.insert(i, GoodLang::printf("%i", i));
                }
            })) {
                GoodLang::parallel::For(0, 10, [&tree](int i) {
                    GoodLang::parallel::For(0, 100, [i, &tree](int j) {
                        if (auto thread_ptr = GoodLang::parallel::AsThread([&tree]() {
                            // this thread is using the shared C++ thread for iterative AsThread calls, and is significantly less performant. 
                            for (int i = 0; i < 1000; ++i) {
                                (void)tree.erase(i);
                            }
                        })) {
                            GoodLang::parallel::For(0, 1000, [i, j, &tree](int k) {
                                tree.insert((i * 100 * 100) + (k * 100) + j, GoodLang::printf("%i", (i * 100 * 100) + (k * 100) + j));
                            });
                        }
                    });
                });
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        // technically works but runs into a major bottleneck.. AsThread running into AsThread during the less-optimum route causes a major bottleneck. 
        // Needs this version of the parallel atomic_map to better support it. 
        if (0) {
            sw.Start();
            if (auto thread_ptr = GoodLang::parallel::AsThread([]() {
                // Do Something...
            })) {
                // Meanwhile...
                GoodLang::parallel::For(0, 100, [](int i) {
                    if (auto thread_ptr2 = GoodLang::parallel::AsThread([]() {
                        // Do Something...
                    })) {
                    //    // Meanwhile...
                        GoodLang::parallel::For(0, 100, [&i](int j) {
                            if (auto thread_ptr3 = GoodLang::parallel::AsThread([]() {
                                // Do Something...
                            })) {
                    //            // Meanwhile...
                    //            GoodLang::parallel::For(0, 100, [&i, &j](int k) {
                    //            });
                                thread_ptr3 = nullptr;
                            }
                        });
                        thread_ptr2 = nullptr;
                    }
                });
                thread_ptr = nullptr;
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            for (char c = 'a'; c <= 'z'; ++c) {
                EXPECT_EQ(tree.find((int)c), tree.end());
            }
            for (char c = 'a'; c <= 'z'; ++c) {
                tree.insert((int)c, std::string(1, c));
            }
            for (char c = 'Z'; c >= 'A'; --c) {
                EXPECT_EQ(tree.find((int)c), tree.end());
            }
            for (char c = 'A'; c <= 'Z'; ++c) {
                tree.insert((int)c, std::string(1, c));
            }
            for (char c = '0'; c <= '9'; ++c) {
                EXPECT_EQ(tree.find((int)c), tree.end());
            }
            for (char c = '0'; c <= '9'; ++c) {
                tree.insert((int)c, std::string(1, c));
            }
            for (char c = '0'; c <= '9'; ++c) {
                tree.emplace((int)c, std::string("OVERWRITTEN: ") + std::string(1, c));
            }
            for (char c = 'A'; c <= 'Z'; ++c) {
                tree.erase((int)c);
                EXPECT_EQ(tree.find((int)c), tree.end());
            }
        }
        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            GoodLang::parallel::For(0, 255, [&](int i) {
                tree.insert(i, GoodLang::ToString(i));
            });
            auto g{ tree.ProtectCurrentEpoch() };
            for (auto& x : tree) {
                //print(iter->key);
            }
        }
        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            GoodLang::parallel::For(0, 255, [&](int i) {
                tree.insert(i, GoodLang::ToString(i));
            });
            auto g{ tree.ProtectCurrentEpoch() };
            for (auto& x : tree) {
                //print(iter->key);
            }
        }
        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            GoodLang::parallel::For(0, 255, [&](int i) {
                auto g{ tree.ProtectCurrentEpoch() };
                tree.insert(i, GoodLang::ToString(i));
                if (tree.erase(i)) {
                    // claims to have successfully erased it. 
                    EXPECT_EQ(tree.find(i), tree.end());
                }
                else {
                    // claims to have failed to erase it
                    EXPECT_NE(tree.find(i), tree.end());
                    tree.erase(i);
                }
                
            });
            auto g{ tree.ProtectCurrentEpoch() };
            for (auto& x : tree) {
                //print(iter->key);
            }
        }
        if (1) {
            utilities::atomic_map<size_t, std::string> tree{};
            if (auto thread_ptr = GoodLang::parallel::AsThread([&tree]() {
                for (int i = 0; i < 254; ++i) {
                    auto g{ tree.ProtectCurrentEpoch() };
                    tree.erase(i);
                };
            })) {
                GoodLang::parallel::For(0, 255, [&](int i) {
                    auto g{ tree.ProtectCurrentEpoch() };
                    tree.insert(i, GoodLang::ToString(i));
                    for (auto& x : tree) {
                        // print(p->key);
                    }
                });
                thread_ptr = nullptr;
            }

            auto g{ tree.ProtectCurrentEpoch() };
            for (auto& x : tree) {
                //print(iter->key);
            }
        }
#endif
        // Test atomic_map and atomic_unordered_map
        if (0) {
            utilities::atomic_map<size_t, std::string> tree{};
            std::atomic<long> S{ 0 };
            sw.Start();
            GoodLang::parallel::Until(
                [&](void) -> void {
                    long s = ++S;
                    tree.insert(s, GoodLang::printf("%i", (int)s));

                    if (s % 10000 == 0) {
                        for (auto& x : tree) {
                            (void)x.second.c_str();
                        }
                    }
                },
                [&](void)-> bool {
                    return tree.size() >= 100000;
                }
            );
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (0) {
            utilities::atomic_unordered_map<utilities::string, std::string> tree{};
            std::atomic<long> S{ 0 };
            sw.Start();
            GoodLang::parallel::Until(
                [&](void) -> void {
                    long s = ++S;
                    tree.insert(GoodLang::printf("%i", (int)s), GoodLang::printf("%i", (int)s));

                    if (s % 10000 == 0) {
                        for (auto& x : tree) {
                            (void)x.second.c_str();
                        }
                    }
                },
                [&](void)-> bool {
                    return tree.size() >= 100000;
                }
            );
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (0) {
            concurrency::concurrent_unordered_map<utilities::string, std::string> tree{};
            std::atomic<long> S{ 0 };
            sw.Start();
            GoodLang::parallel::Until(
                [&](void) -> void {
                    long s = ++S;
                    tree.insert(std::pair<utilities::string, std::string>{ GoodLang::printf("%i", (int)s), GoodLang::printf("%i", (int)s) });

                    if (s % 10000 == 0) {
                        for (auto& x : tree) {
                            (void)x.second.c_str();
                        }
                    }
                },
                [&](void)-> bool {
                    return tree.size() >= 100000;
                }
            );
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        if (1) {
            Scopes::Cache<4> cache;
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                cache.EmplaceCache<0>(1, 0, reinterpret_cast<Scopes::Breadcrumb*>(1));
                (void)cache.TryGetCache<0>(1, 0);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                cache.EmplaceCache<0>(1, i, reinterpret_cast<Scopes::Breadcrumb*>(i));
                (void)cache.TryGetCache<0>(1, i);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                cache.EmplaceCache<0>(i + 1, 0, reinterpret_cast<Scopes::Breadcrumb*>(1));
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                cache.EmplaceCache<0>(i + 1, 0, reinterpret_cast<Scopes::Breadcrumb*>(1));
                cache.TryGetCache<0>(i + 1, 0);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

#if 0
        for (int loopN = 0; loopN < 1000; loopN++) {
            Scopes::RootScope root;
            if (1) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                // EXPECT_EQ(*scope.breadcrumb_m.this_m.scope_index, 1);
                EXPECT_EQ(root.breadcrumb_m.this_m.scope_index._index, 0);
                EXPECT_EQ(scope.breadcrumb_m.root_m, &root.breadcrumb_m);
                EXPECT_EQ(scope.object_or_function_versions, 0);

                scope.UpdateObjectFunctionVersion();
                EXPECT_EQ(scope.object_or_function_versions, 1);
            }
            if (1) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                // EXPECT_EQ(*scope.breadcrumb_m.this_m.scope_index, 1);
                EXPECT_EQ(root.breadcrumb_m.this_m.scope_index._index, 0);
                EXPECT_EQ(scope.breadcrumb_m.root_m, &root.breadcrumb_m);
                EXPECT_EQ(scope.object_or_function_versions, 0);

                Scopes::BasicScope scope2("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                // EXPECT_EQ(*scope2.breadcrumb_m.this_m.scope_index, 2);
                EXPECT_EQ(root.breadcrumb_m.this_m.scope_index._index, 0);
                EXPECT_EQ(scope2.breadcrumb_m.root_m, &root.breadcrumb_m);
                EXPECT_EQ(scope2.object_or_function_versions, 0);

                scope.UpdateObjectFunctionVersion();
                EXPECT_EQ(scope.object_or_function_versions, 1);

                scope2.UpdateObjectFunctionVersion();
                EXPECT_EQ(scope.object_or_function_versions, 1);

                root.UpdateObjectFunctionVersion();
                EXPECT_EQ(root.object_or_function_versions, 1);
                EXPECT_EQ(scope.object_or_function_versions, 2);
                EXPECT_EQ(scope2.object_or_function_versions, 2);
            }
        }
#endif

        if (1) {
            Scopes::RootScope root; // successfully starts a new script root

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 10000, [&](int i) {
                auto& scope{ root.make_namespace("std") };
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 10000, [&](int i) {
                auto& scope{ root.make_namespace("std") };
                scope.invalidate_cache();
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.invalidate_cache();
            })) {
                GoodLang::parallel::For(0, 10000, [&](int i) {
                    auto& scope{ root.make_namespace("std") };
                    scope.invalidate_cache();
                });
                main_loop = nullptr;
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            // Test recursive update calls. Should only recurse one time until the "call num" saturates. 
            sw.Start();
            if (1) {
                auto& scope1{ root.make_namespace("std") };
                auto& scope2{ root.make_namespace("UI") };

                scope2.add_using_here(scope1);
                scope1.add_using_here(scope2);

                scope1.invalidate_cache();
                scope2.invalidate_cache();
                root.invalidate_cache();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));






            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
                scope.emplace_object_here(utilities::string(GoodLang::printf("%i", i)), utilities::ObjectWrapper(i, utilities::ObjectWrapper::ObjectState::Normal)); // x = 100.0;
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));


            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
                scope.emplace_object_here(utilities::string(GoodLang::printf("%i", i)), utilities::ObjectWrapper(i, utilities::ObjectWrapper::ObjectState::Normal)); // x = 100.0;
                if (auto* p = scope.find_object_here(utilities::string(GoodLang::printf("%i", i)))) {}
                else {
                    EXPECT_EQ(true, false);
                }
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));









            sw.Start();
            for(int i = 0; i < 1000000; ++i) {
                auto scope{ root.make_scope() };
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                auto scope{ root.make_scope() };
                //scope.UpdateObjectFunctionVersion();
                //EXPECT_EQ(scope.object_or_function_versions, 1);
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.invalidate_cache();
            })) {
                for (int i = 0; i < 1000000; ++i) {
                    auto scope{ root.make_scope() };
                    //scope.UpdateObjectFunctionVersion();
                    //EXPECT_EQ(true, scope.object_or_function_versions >= 1);
                };
                main_loop = nullptr;
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
                //scope.UpdateObjectFunctionVersion();
                //EXPECT_EQ(scope.object_or_function_versions, 1);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.invalidate_cache();
            })) {
                GoodLang::parallel::For(0, 1000000, [&](int i) {
                    auto scope{ root.make_scope() };
                    //scope.UpdateObjectFunctionVersion();                    
                    //EXPECT_EQ(true, scope.object_or_function_versions >= 1);
                });
                main_loop = nullptr;
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            // print(root.scope_indexs.num_tickets());
        }

        // In order of slowest to fastest way to manage strings using shared_string...
        if (1) { // Copying std::strings
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test = std::string("TEST");
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying std::string_views
            sw.Start();
            std::string sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test = utilities::string(sv);
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Creating a new reference from a globally constant string
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test{ "TEST" };
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying a reference to an existing shared_string
            sw.Start();
            std::string_view sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test{ std::string_view{ sv } };
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying a reference to an existing shared_string
            sw.Start();
            utilities::string sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test{ sv };
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // using std::string_view without scope guarrantees. (This particular example gets optimized-out entirely down to 0.00 seconds)
            sw.Start();
            std::string_view sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                std::string_view test{ sv };
                (void*)test.data();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        if (1) {
            utilities::string str1("::std::string::");
            str1 = str1.remove_leading_and_trailing(':');
            EXPECT_EQ(str1, utilities::string("std::string"));
        }
        if (1) {
            utilities::string str1("Hello World!");
            EXPECT_EQ(str1, utilities::string("Hello World!"));
        }
        if (1) {
            utilities::string str1("Hello World!\n");
            EXPECT_EQ(str1 != utilities::string("Hello World!"), true);
        }

        EXPECT_EQ(utilities::string("").hash(), compound_shared_string("").hash());
        EXPECT_EQ(utilities::string("Hello World!\n").hash(), compound_shared_string("Hello World!\n").hash());
    }

};
