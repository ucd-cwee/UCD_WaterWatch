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
    class string_view {
    public:
        using size_type = std::string_view::size_type;
        static constexpr const auto npos = std::string_view::npos;

    protected:
        std::string_view data;
        size_type _hash{ npos };

    public:
        string_view() {};
        string_view(string_view const&) = default;
        string_view(string_view&&) = default;
        string_view& operator=(string_view const&) = default;
        string_view& operator=(string_view&&) = default;
        virtual ~string_view() = default;

        template <size_t N> __forceinline string_view(const char(&r)[N]) : data(r) {};
        string_view(std::string_view const& _Copy) : data(_Copy) {};

        friend bool operator==(string_view const& A, string_view const& V) noexcept {
            if (A.data.length() != V.data.length()) return false;
            else if (A.data.length() > 1) return A.hash() == V.hash();
            else return A.data == V.data;
        };
        friend bool operator<(string_view const& A, string_view const& V) {
            if (A.data.length() < V.data.length()) return true;
            else if (A.data.length() > V.data.length()) return false;
            else /*if (A.length() > 1)*/ return A.hash() < V.hash();
            //else return A.data < V.data;
        };
        friend bool operator<=(string_view const& A, string_view const& V) {
            if (A.data.length() < V.data.length()) return true;
            else if (A.data.length() > V.data.length()) return false;
            else /*if (A.length() > 1)*/ return A.hash() <= V.hash();
            //else return A.data <= V.data;
        };
        friend bool operator>(string_view const& A, string_view const& V) { return !operator<=(A, V); };
        friend bool operator>=(string_view const& A, string_view const& V) { return !operator<(A, V); };
        friend bool operator!=(string_view const& A, string_view const& V) noexcept { return !operator==(A, V); };

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
        size_type rfind(const string_view& _Right) const {
            return data.rfind(_Right.data);
        };
        string_view remove_trailing(char _Right) const {
            string_view out{ *this };
            while (
                (out.length() > 0)
                && ((out.operator[](out.length() - 1) == _Right))
                ) {
                out.remove_suffix(1);
            }
            return out;
        };
        string_view remove_leading(char _Right) const {
            string_view out{ *this };
            while (
                (out.length() > 0)
                && (out.operator[](0) == _Right)
                ) {
                out.remove_prefix(1);
            }
            return out;
        };
        string_view remove_leading_and_trailing(char _Right) const {
            string_view out{ *this };
            return out.remove_trailing(_Right).remove_leading(_Right);
        };

        static const string_view& empty_string() {
            static string_view out{ "" };
            return out;
        };
        static const string_view& namespace_colons() {
            static string_view out{ "::" };
            return out;
        };
    };

    class string : public string_view {
    private:
        std::string _data;

    public:
        string() {};
        string(string const&) = default;
        string(string&&) = default;
        string& operator=(string const&) = default;
        string& operator=(string&&) = default;
        virtual ~string() = default;

        template <size_t N> __forceinline string(const char(&r)[N]) : _data() { this->data = r; };
        string(std::string && _Copy) : _data(std::move(_Copy)) {
            this->data = _data;
        };

    };

    // all const-functions are thread-safe
    class compound_shared_string {
    public:
        string_view a;
        string_view b;
        string_view c;

    public:
        compound_shared_string(string_view A = "", string_view B = "", string_view C = "") : a{ A }, b{ B }, c{ C } {};
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
        size_t Cmpn(const string_view& rhs, size_t n = std::string::npos) const {
            long long j = std::min<long long>(rhs.length(), length());
            if (n > j) return false;
            for (j = j - 1; (j >= 0) && (n > 0); --j, --n) {
                if (rhs[j] != operator[](j)) return false;
            }
            return true;
        };
        friend bool operator==(compound_shared_string const& lhs, const string_view& rhs) {
            if (rhs.length() != lhs.length()) return false;
            for (long long j = lhs.length() - 1; j >= 0; --j) {
                if (rhs[j] != lhs[j]) return false;
            }
            return true;
        };
        friend bool operator==(const string_view& rhs, compound_shared_string const& lhs) {
            if (rhs.length() != lhs.length()) return false;
            for (long long j = lhs.length() - 1; j >= 0; --j) {
                if (rhs[j] != lhs[j]) return false;
            }
            return true;
        };
        friend bool operator!=(compound_shared_string const& lhs, const string_view& rhs) {
            return !operator==(lhs, rhs);
        };
        friend bool operator!=(const string_view& rhs, compound_shared_string const& lhs) {
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
                block_t* block = &*blocks.grow_by(1);

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
                    for (auto& block : blocks) for (auto& element : block.elements) if (element.initialized) {
                        reinterpret_cast<T*>(&element.data[0])->~T();
                        element.initialized = false;
                    }
                }
            };

        public:
            BlockAlloc() : blocks{}, free{} {
                free.m_n64 = 0;
                // AllocBlock(); 
            };
            ~BlockAlloc() { ReleaseBlocks(); };

            // Acquire a new element from the free list and construct it.
            template <typename... TArgs> __declspec(noinline) T* Alloc(TArgs &&... a) {
                element_t* element{ nullptr };
                while (1) {
                    if (element = Pop(free)) {
                        if constexpr (std::is_pod<T>::value) element->initialized = true;
                        T* data{ (T*)&element->data[0] };
                        if constexpr (!std::is_pod<T>::value || !skipInitialization) new (data) T(std::forward<TArgs>(a)...);                        
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
                    element->~T();
                    t->initialized = false;
                }
                           
                Push(free, t);
            };

            concurrency::concurrent_vector<block_t>
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
                _type_ T;
                size_t threadID;
            };

            std::array<ABA_Problem::BlockAlloc<innerType, num_items, skipInitialization>, num_parallel_allocators> TLS_arr{};
            static auto GetThreadID() { return IDManager::GetThreadID(); };

        public:
            template <typename... TArgs> __declspec(noinline) _type_* Alloc(TArgs&&... a) {
                size_t thisThreadIndex = GetThreadID() % num_parallel_allocators;
                auto& TLS = TLS_arr[thisThreadIndex];
                innerType* out{ TLS.Alloc(innerType{ _type_{std::forward<TArgs>(a)...}, thisThreadIndex }) };
                return (_type_*)(out);
            };
            __declspec(noinline) void Free(const _type_* t) {
                innerType* impl = static_cast<innerType*>(static_cast<void*>(const_cast<_type_*>(t)));
                auto& TLS = TLS_arr[impl->threadID];
                TLS.Free(impl);
            };
            template <typename... TArgs> std::shared_ptr< _type_ > AllocShared(TArgs&&... a) {
                return std::shared_ptr<_type_>(Alloc(std::forward<TArgs>(a)...), [this](_type_* p) { Free(p); });
            };
        };


        /// <summary>
        /// Allocator that can prevent deletion of memory until after it is safe to do so, leveraging Epoch (e.g. time of deletion) to determine safety. 
        /// Typically, three epochs (or iterations) within a thread must pass before a memory recollection is valid. 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        template <typename T, uint64_t CleanupFrequencyMilliseconds = 1> 
        class EpochProtectedAllocator final : public GoodLang::EpochGarbageCollectorImpl {
        private:
            using EpochStorageType = typename ThreadLocalStorage::EpochStorageType;
            using EpochQueueType = std::pair<EpochStorageType, T*>;

            std::array<ThreadLocalStorage, ThreadManager::kMaxThreadNum> TLS_arr;
            std::atomic<EpochStorageType> lastGC;
            BlockAlloc<T, sizeof(T) << 4> _alloc;

            static auto GetThreadID() { return IDManager::GetThreadID(); };
            auto& GetTLS() { return TLS_arr[GetThreadID()]; };
            moodycamel::ConcurrentQueue< EpochQueueType > DeleteList;

        public:
            // Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
            void RunGC() override {
                static constexpr auto duration{ std::chrono::milliseconds(CleanupFrequencyMilliseconds) };
                static thread_local EpochQueueType out{};
                EpochStorageType _EpochLimit{ std::numeric_limits<EpochStorageType>::max() };
                auto currentGC{ std::chrono::milliseconds(ThreadManager::GetCurrentEpoch()) };

                if ((currentGC - std::chrono::milliseconds(lastGC.load())) > duration) {
                    lastGC.store(currentGC.count());

                    for (auto& tls : TLS_arr)
                        if (tls.Active > 0)
                            _EpochLimit = std::min< EpochStorageType>(_EpochLimit, tls.EpochLimit.load());

                    if (_EpochLimit > 0)
                        while (DeleteList.try_pop(out)) // while jobs are available...
                            if (out.first < _EpochLimit) // ...if the data is in the correct time period for reclamation...
                                _alloc.Free(out.second); // ... then do the clean-up, and try again (hoping that the list is semi-sorted).
                            else
                            {
                                DeleteList.push(out);
                                return;
                            }// ... otherwise push to the end of the queue, which is a form of lazy sorting (also prevents endless looping without additional checks / handles). 
                }
            };

        public:
            // Request a new memory pointer
            template <typename... TArgs> T* Alloc(TArgs &&... a) {
                return _alloc.Alloc(std::forward<TArgs>(a)...);
            };

            // Frees the memory pointer
            void				Free(const T* element) {
                DeleteList.push({ ThreadManager::GetCurrentEpoch(), const_cast<T*>(element) });
                RunGC();
            };

        public:
            EpochProtectedAllocator() 
                : EpochGarbageCollectorImpl()
                , TLS_arr{}
                , lastGC{ 0 }
                , DeleteList{} 
            { 
                for (auto& tls : TLS_arr) tls.parent = this; 
            };
            EpochProtectedAllocator(EpochProtectedAllocator const&) = delete;
            EpochProtectedAllocator(EpochProtectedAllocator&&) = delete;
            EpochProtectedAllocator& operator=(EpochProtectedAllocator const&) = delete;
            EpochProtectedAllocator& operator=(EpochProtectedAllocator&&) = delete;
            virtual ~EpochProtectedAllocator() = default;

        public:
            // Stalls deallocation / free calls made after this guard until at least after this guard expires.
            [[nodiscard]] const auto CreateEpochGuard() {
                return GetTLS().ProtectCurrentEpoch();
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
        };

        size_t
            _size{ 0 };
        concurrency::concurrent_vector<Wrap>
            _listeners;
        void (T::*_callback)(long*);
        
        // add a listener to the list
        void add_listener(size_t index, T* p) {
            if (_size <= index) {
                if (_listeners.size() <= index) (void)_listeners.grow_to_at_least((index + 2) + ((index + 2) % 16));
                InterlockedExchange(static_cast<volatile size_t*>(&_size), index);
            }
            Wrap& wrap = _listeners[index - 1];
            InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&wrap.ptr), static_cast<PVOID>(p)); 
            InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1 << 8);
            InterlockedIncrement(static_cast<volatile long*>(&wrap.alive));
        };
        // remove a listener from the list
        void remove_listener(size_t index) {
            Wrap& wrap = _listeners[index - 1];
            InterlockedDecrement(static_cast<volatile long*>(&wrap.alive));
            if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), -(1 << 8)) == 0) {}
            else while (wrap.count != 0) if (!wrap.ptr) InterlockedExchange(static_cast<volatile long*>(&wrap.count), 0);
            InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&wrap.ptr), static_cast<PVOID>(nullptr)); 
        };

    public:
        Callback(void (T::*listener)(long*))
            : _callback{ listener }
        {};
        ScopedListener listener(size_t index, T* p) {
            add_listener(index, p);
            return ScopedListener(index, *this);
        };
        // callback performed on all listeners
        void speak(long* parent_alive) {
            for (size_t i = 0; i < _size; ++i) {
                Wrap& wrap = _listeners[i];
                if (wrap.alive) {
                    if (!parent_alive || *parent_alive) {
                        if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1) >= (1 << 8))
                            (wrap.ptr->*_callback)(&wrap.alive); // _callback(wrap.ptr, &wrap.alive);
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

    template <typename T>
    class DelayedInstantiation {
    private:
        std::unique_ptr<T> ptr{ nullptr };
        GoodLang::fast_shared_mutex mut;
        bool initialized{ 0 };       
        
    public:
        bool valid() const {
            return initialized;
        };
        T* operator->() {
            if (!initialized) {
                mut.lock();
                if (!initialized) {
                    ptr = std::make_unique<T>();
                    InterlockedIncrement(reinterpret_cast<volatile long*>(&initialized));
                }
                mut.unlock();                
            }
            return ptr.get();
        };
        T& operator*() {
            return *operator->();
        };
    };

};
namespace std {
    template <> struct hash<utilities::string_view> {
        std::size_t operator()(const utilities::string_view& k) const {
            return k.hash();
        };
    };
    template <> struct less<utilities::string_view> {
        std::size_t operator()(const utilities::string_view& lhs, const utilities::string_view& rhs) const {
            return lhs < rhs;
        };
    };
    template <> struct greater<utilities::string_view> {
        std::size_t operator()(const utilities::string_view& lhs, const utilities::string_view& rhs) const {
            return lhs > rhs;
        };
    };
    template <> struct equal_to<utilities::string_view> {
        std::size_t operator()(const utilities::string_view& lhs, const utilities::string_view& rhs) const {
            return lhs == rhs;
        };
    };

};




class Scopes {
private:
    // clean-up the name of a scope: e.g. "::" becomes "::", "Units" becomes "::Units::"
    static utilities::compound_shared_string CleanUpScopeName(utilities::string_view const& x) {
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
        utilities::string_view
            scope_name; // e.g. "Color"
        Scopes::BasicScope*
            scope;
    private:
        std::unique_ptr<utilities::string_view>
            current_namespace; // e.g. "::" or "::UI::Color::"
        int
            scope_type; // may be a compound of multiple types, e.g. a root is also a namespace

    public:
        ScopeID(utilities::string_view && scope_name_p = {}, int scope_type_p = ScopeType::Basic)
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


        Breadcrumb(utilities::string_view&& name = {}, int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
            : this_m(std::move(name), scope_type_p)
            , parent_m(std::move(parent))
            , root_m{ nullptr }
            , namespace_m{ nullptr }
            , scope_index{}
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
                    this_m.current_namespace = std::make_unique<utilities::string_view>(CleanUpScopeName(this_m.scope_name));
                }
            }
            else {
                if (parent_m) { /*this_m.current_namespace = parent_m->this_m.current_namespace;*/ }
                else this_m.current_namespace = std::make_unique<utilities::string_view>(utilities::string_view::namespace_colons());
            }
        };
        Breadcrumb(Breadcrumb const&) = delete;
        Breadcrumb(Breadcrumb &&) = delete;
        Breadcrumb& operator=(Breadcrumb const&) = delete;
        Breadcrumb& operator=(Breadcrumb&&) = delete;
        ~Breadcrumb() = default;

    };


public:
    template <int numCategories = 4> class Cache {
    private: // CacheVersion -> CacheCategory -> Inputs -> Result
        using ResultType = Breadcrumb*;
        using InputType = size_t;
        using ResultForInputType = concurrency::concurrent_unordered_map<InputType, ResultType>;
        using CachedCategory = std::pair<size_t, std::array<ResultForInputType, numCategories>>;
        
        utilities::ABA_Problem::EpochProtectedAllocator< CachedCategory > _alloc;
        std::atomic<CachedCategory*> _current_cache;

    public:
        template<int category>
        void EmplaceCache(size_t cache_version, size_t input_hash, Breadcrumb* result) {
            auto g{ _alloc.CreateEpochGuard() };
            
            CachedCategory* cached{ nullptr }, *new_ptr{ nullptr };
            while (true) {
                cached = _current_cache.load();
                if (cached && cached->first >= cache_version) {
                    break;
                }
                else {
                    new_ptr = _alloc.Alloc(std::pair<size_t, std::array<ResultForInputType, numCategories>>{ cache_version, std::array<ResultForInputType, numCategories>() });
                    if (_current_cache.compare_exchange_strong(cached, new_ptr)) {
                        if (cached) _alloc.Free(cached);
                        cached = new_ptr;
                        break;
                    }
                    else {
                        _alloc.Free(new_ptr);
                    }
                }
            }

            InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&cached->second[category][input_hash]), reinterpret_cast<PVOID>(result));
        };

        template<int category>
        Breadcrumb* TryGetCache(size_t cache_version, size_t input_hash) {
            auto g{ _alloc.CreateEpochGuard() };
            CachedCategory* cached{ _current_cache.load() };
            if (cached && cached->first >= cache_version) {
                return cached->second[category][input_hash];
            }
            else {
                return nullptr;
            }
        };

    };

    class BasicScope {
    public:
        Breadcrumb 
            breadcrumb_m;
        utilities::DelayedInstantiation<concurrency::concurrent_vector< utilities::string_view >>
            using_m; // NOTE: calling "using" should split the scope - e.g. using statements are appended staticly at compile time, NOT at runtime. 
        utilities::DelayedInstantiation<concurrency::concurrent_unordered_map< utilities::string_view, utilities::ObjectWrapper>>
            objects_m; // NOTE: adding objects should be appended staticly at compile time, NOT at runtime. E.g. the names are known, even if the types are not yet known. 

    public:
        BasicScope(utilities::string_view&& name = {}, int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
            : breadcrumb_m(std::move(name), scope_type_p, parent)
        {
            breadcrumb_m.this_m.scope = this;
        };
        virtual ~BasicScope() = default;
    };

    class NamespaceScope : public BasicScope {
    public:
        // explicit children namespaces, with strongly-held protections to their memory.
        concurrency::concurrent_unordered_map<size_t, std::shared_ptr<NamespaceScope>>
            children;
    private:
        //Scopes::Cache<4> 
            //search_cache; // while thread-safe, it does seem to singificantly decrease the performance of creating new BasicScope's
    private:
        utilities::Callback<NamespaceScope>
            sockets_for_obj_or_func_versions;
        utilities::Callback<NamespaceScope>::ScopedListener
            connection_for_obj_or_func_version;
    public:
        size_t
            object_or_function_versions;
        void UpdateObjectFunctionVersion(long* parent_alive = nullptr) {
            InterlockedIncrement(static_cast<volatile size_t*>(&object_or_function_versions));
            sockets_for_obj_or_func_versions.speak(parent_alive);
        };

    private:
        utilities::Callback<NamespaceScope>
            sockets_for_using_or_children_versions;
        utilities::Callback<NamespaceScope>::ScopedListener
            connection_for_using_or_children_version;
    public:
        size_t
            using_or_children_versions;
        void UpdateUsingChildrenVersion(long* parent_alive = nullptr) {
            InterlockedIncrement(static_cast<volatile size_t*>(&using_or_children_versions));
            sockets_for_using_or_children_versions.speak(parent_alive);
        };

    public:
        NamespaceScope(utilities::string_view&& name = {}, int scope_type_p = ScopeType::Basic & ScopeType::Namespace, Breadcrumb* parent = nullptr)
            : BasicScope(std::move(name), scope_type_p, parent)
            , children{}
            //, search_cache{}
            , sockets_for_obj_or_func_versions(&NamespaceScope::UpdateObjectFunctionVersion)
            , connection_for_obj_or_func_version{}
            , object_or_function_versions{ 0 }
            , sockets_for_using_or_children_versions(&NamespaceScope::UpdateUsingChildrenVersion)
            , connection_for_using_or_children_version{}
            , using_or_children_versions{ 0 }
        {
            if (this->breadcrumb_m.parent_m) {
                if (auto* p = dynamic_cast<NamespaceScope*>(this->breadcrumb_m.parent_m->namespace_m->this_m.scope)) {
                    connection_for_obj_or_func_version = p->sockets_for_obj_or_func_versions.listener(this->breadcrumb_m.GetScopeIndex(), this);
                    connection_for_using_or_children_version = p->sockets_for_using_or_children_versions.listener(this->breadcrumb_m.GetScopeIndex(), this);
                }
            }
        };
        virtual ~NamespaceScope() = default;


    };

    class RootScope : public NamespaceScope {
    public:
        // when a scope is born it will get the smallest-possible unique index for itself. 
        utilities::TicketDispensor 
            scope_indexs;

    public:
        RootScope() 
            : NamespaceScope("::", ScopeType::Basic & ScopeType::Namespace & ScopeType::Root, nullptr) 
        {
            // scope_indexs.reserve(100);
        };
        virtual ~RootScope() = default;

    };

};



int main() {
    using namespace utilities;
    using namespace ABA_Problem;
    Stopwatch sw;

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
    if (1) {
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

    while (true) {
        print("");

        if (0) {
            Scopes::Cache<4> cache;
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                cache.EmplaceCache<0>(1, 0, reinterpret_cast<Scopes::Breadcrumb*>(1));
                EXPECT_EQ(cache.TryGetCache<0>(1, 0), reinterpret_cast<Scopes::Breadcrumb*>(1));
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                cache.EmplaceCache<0>(1, i, reinterpret_cast<Scopes::Breadcrumb*>(i));
                EXPECT_EQ(cache.TryGetCache<0>(1, i), reinterpret_cast<Scopes::Breadcrumb*>(i));
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
            Scopes::RootScope root;

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                Scopes::NamespaceScope scope("std", Scopes::ScopeType::Basic | Scopes::ScopeType::Namespace, &root.breadcrumb_m);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                Scopes::NamespaceScope scope("std", Scopes::ScopeType::Basic | Scopes::ScopeType::Namespace, &root.breadcrumb_m);
                scope.UpdateObjectFunctionVersion();
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.UpdateObjectFunctionVersion();
            })) {
                GoodLang::parallel::For(0, 1000000, [&](int i) {
                    Scopes::NamespaceScope scope("std", Scopes::ScopeType::Basic | Scopes::ScopeType::Namespace, &root.breadcrumb_m);
                    scope.UpdateObjectFunctionVersion();
                });
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));





            sw.Start();
            for(int i = 0; i < 1000000; ++i) {
                Scopes::BasicScope scope({}, Scopes::ScopeType::Basic, & root.breadcrumb_m);
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                Scopes::BasicScope scope({}, Scopes::ScopeType::Basic, & root.breadcrumb_m);
                //scope.UpdateObjectFunctionVersion();
                //EXPECT_EQ(scope.object_or_function_versions, 1);
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.UpdateObjectFunctionVersion();
            })) {
                for (int i = 0; i < 1000000; ++i) {
                    Scopes::BasicScope scope({}, Scopes::ScopeType::Basic, & root.breadcrumb_m);
                    //scope.UpdateObjectFunctionVersion();
                    //EXPECT_EQ(true, scope.object_or_function_versions >= 1);
                };
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                Scopes::BasicScope scope({}, Scopes::ScopeType::Basic, &root.breadcrumb_m);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                Scopes::BasicScope scope({}, Scopes::ScopeType::Basic, & root.breadcrumb_m);
                //scope.UpdateObjectFunctionVersion();
                //EXPECT_EQ(scope.object_or_function_versions, 1);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.UpdateObjectFunctionVersion();
            })) {
                GoodLang::parallel::For(0, 1000000, [&](int i) {
                    Scopes::BasicScope scope({}, Scopes::ScopeType::Basic, & root.breadcrumb_m);
                    //scope.UpdateObjectFunctionVersion();                    
                    //EXPECT_EQ(true, scope.object_or_function_versions >= 1);
                });
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            print(root.scope_indexs.num_tickets());
        }

        // In order of slowest to fastest way to manage strings using shared_string...
        if (1) { // Copying std::strings
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test = utilities::string(std::string("TEST"));
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying std::string_views
            sw.Start();
            std::string sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string_view test = utilities::string_view(sv);
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Creating a new reference from a globally constant string
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                utilities::string_view test = utilities::string_view("TEST");
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying a reference to an existing shared_string
            sw.Start();
            std::string_view sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string_view test = utilities::string_view(sv);
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying a reference to an existing shared_string
            sw.Start();
            utilities::string sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string_view test = utilities::string_view(sv);
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // using std::string_view without scope guarrantees. (This particular example gets optimized-out entirely down to 0.00 seconds)
            sw.Start();
            std::string_view sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                std::string_view test = std::string_view(sv);
                (void*)test.data();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        if (1) {
            utilities::string_view str1("::std::string::");
            str1 = str1.remove_leading_and_trailing(':');
            EXPECT_EQ(str1, utilities::string_view("std::string"));
        }
        if (1) {
            utilities::string_view str1("Hello World!");
            EXPECT_EQ(str1, utilities::string_view("Hello World!"));
        }
        if (1) {
            utilities::string_view str1("Hello World!\n");
            EXPECT_EQ(str1 != utilities::string_view("Hello World!"), true);
        }

        EXPECT_EQ(utilities::string_view("").hash(), compound_shared_string("").hash());
        EXPECT_EQ(utilities::string_view("Hello World!\n").hash(), compound_shared_string("Hello World!\n").hash());
    }

};
