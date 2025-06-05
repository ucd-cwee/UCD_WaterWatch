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
    // all const-functions are thread-safe
    class shared_string {
    public:
        using size_type = std::string_view::size_type;
        static constexpr const auto npos = std::string_view::npos;

    private:
        std::shared_ptr<std::string> source;
        std::string_view data;
        size_type _hash{ npos };

    public:
        shared_string() {};
        shared_string(shared_string const&) = default;
        shared_string(shared_string&&) = default;
        shared_string& operator=(shared_string const&) = default;
        shared_string& operator=(shared_string&&) = default;
        ~shared_string() = default;

        template <size_t N> __forceinline shared_string(const char(&r)[N]) : source(nullptr), data(r) {};
        shared_string(std::string&& _Copy) : source(std::make_shared<std::string>(std::move(_Copy))) { data = *source; }
        shared_string(std::string_view const& _Copy) : source(std::make_shared<std::string>(_Copy)) { data = *source; }
        explicit shared_string(std::shared_ptr<std::string> const& _Orig, std::string_view _Copy) : source(_Orig), data(std::move(_Copy)) {}

        const char* c_str() const {
            return data.data();
        };
        shared_string substr(const size_type _Off = 0, size_type _Count = npos) const {
            return shared_string(source, data.substr(_Off, _Count));
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
        auto begin() const {
            return data.begin();
        };
        auto end() const {
            return data.end();
        };
        auto cbegin() const {
            return data.begin();
        };
        auto cend() const {
            return data.end();
        };
        const char& operator[](const size_type index) const {
            return data[index];
        }
        friend std::ostream& operator<<(std::ostream& os, shared_string const& obj) {
            os << obj.data; 
            return os;
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
        bool Equals(const shared_string& _Right, size_t _Count = npos) const {
            long long j = std::min<long long>(_Right.length(), length());
            if (_Count > j) return false;
            for (j = j - 1; (j >= 0) && (_Count > 0); --j, --_Count) if (_Right[j] != operator[](j)) return false;            
            return true;
        };        
        friend bool operator==(shared_string const& A, shared_string const& V) noexcept {
            if (A.length() != V.length()) return false;
            else if (A.length() > 16) return A.hash() == V.hash();            
            else return A.data == V.data;
        };
        friend bool operator<(shared_string const& A, shared_string const& V) {
            if (A.data.length() < V.length()) return true;            
            else if (A.data.length() > V.length()) return false;            
            else if(A.length() > 16) return A.hash() < V.hash();
            else return A.data < V.data;
        };
        friend bool operator<=(shared_string const& A, shared_string const& V) {
            if (A.data.length() < V.length()) return true;            
            else if (A.data.length() > V.length()) return false;            
            else if (A.length() > 16) return A.hash() <= V.hash();
            else return A.data <= V.data;
        };
        friend bool operator>(shared_string const& A, shared_string const& V) { return !operator<=(A, V); };
        friend bool operator>=(shared_string const& A, shared_string const& V) { return !operator<(A,V); };
        friend bool operator!=(shared_string const& A, shared_string const& V) noexcept { return !operator==(A,V); };   
        friend shared_string operator+(shared_string const& A, shared_string const& V) noexcept {
            return std::string(A.data) + std::string(V.data);
        };

        void remove_prefix(const size_type _Count) noexcept {
            data.remove_prefix(_Count);
        };
        void remove_suffix(const size_type _Count) noexcept {
            data.remove_suffix(_Count);
        };
        size_type find(const shared_string& _Right, long long _Off = 0, long long _End = -1) const {
            long long l, j, k;
            const char sample = _Right[0];
            if (_End == -1) _End = length();
            l = _End - (k = _Right.length());
            if (k <= 0 || (l - _Off) < 0) return npos;
            if (!sample) return (size_t)_Off;
            for (; _Off <= l; ++_Off) // starting at the search position ... 
                if (operator[](_Off) == sample)  // found a match for the first character ...
                    for (j = 1; ; ++j) { // for the remaining parts of the search text ... 
                        if (j >= k) return (size_t)_Off;
                        if (operator[](_Off + j) != _Right[j]) break;
                    }

            return npos;
        };
        size_type rfind(const shared_string& _Right) const {
            return data.rfind(_Right.c_str());
        };
        shared_string replace(const shared_string& from, const shared_string& to) const {
            if (from.empty() || (from == to)) return *this;
            size_t startPos;
            if (npos != (startPos = find(from))) {
                std::shared_ptr<std::string> string{ std::make_shared<std::string>(this->data) };
                shared_string out(string, *string);
                while (startPos != npos) {
                    string->replace(startPos, from.length(), to.c_str());
                    startPos = out.find(from, to.length() + startPos, string->length());
                }
                return shared_string(string, *string);
            }
            else return *this;
        }
        shared_string remove_trailing(char _Right) const {
            shared_string out{ *this };
            while (
                (out.length() > 0)
                && ((out.operator[](out.length() - 1) == _Right))
                ) {
                out.remove_suffix(1);
            }
            return out;
        };
        shared_string remove_leading(char _Right) const {
            shared_string out{ *this };
            while (
                (out.length() > 0)
                && (out.operator[](0) == _Right)
                ) {
                out.remove_prefix(1);
            }
            return out;
        };
        shared_string remove_leading_and_trailing(char _Right) const {
            shared_string out{ *this };
            return out.remove_trailing(_Right).remove_leading(_Right);
        };

    public:
        static const shared_string& empty_string() {
            static shared_string out{ "" };
            return out;
        };
        static const shared_string& namespace_colons() {
            static shared_string out{ "::" };
            return out;
        };
    };    
    // all const-functions are thread-safe
    class compound_shared_string {
    public:
        shared_string a;
        shared_string b;
        shared_string c;

    public:
        compound_shared_string(shared_string A = "", shared_string B = "", shared_string C = "") : a{ A }, b{ B }, c{ C } {};
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
            for (auto& x : a) {
                out ^= x + 0x9e3779b9 + (out << 6) + (out >> 2);
            }
            for (auto& x : b) {
                out ^= x + 0x9e3779b9 + (out << 6) + (out >> 2);
            }
            for (auto& x : c) {
                out ^= x + 0x9e3779b9 + (out << 6) + (out >> 2);
            }
            return out;
        }
        bool empty() const {
            return length() == 0;
        };
        size_t Cmpn(const shared_string& rhs, size_t n = std::string::npos) const {
            long long j = std::min<long long>(rhs.length(), length());
            if (n > j) return false;
            for (j = j - 1; (j >= 0) && (n > 0); --j, --n) {
                if (rhs[j] != operator[](j)) return false;
            }
            return true;
        };
        friend bool operator==(compound_shared_string const& lhs, const shared_string& rhs) {
            if (rhs.length() != lhs.length()) return false;
            for (long long j = lhs.length() - 1; j >= 0; --j) {
                if (rhs[j] != lhs[j]) return false;
            }
            return true;
        };
        friend bool operator==(const shared_string& rhs, compound_shared_string const& lhs) {
            if (rhs.length() != lhs.length()) return false;
            for (long long j = lhs.length() - 1; j >= 0; --j) {
                if (rhs[j] != lhs[j]) return false;
            }
            return true;
        };
        friend bool operator!=(compound_shared_string const& lhs, const shared_string& rhs) {
            return !operator==(lhs, rhs);
        };
        friend bool operator!=(const shared_string& rhs, compound_shared_string const& lhs) {
            return !operator==(lhs, rhs);
        };
        operator shared_string() const {
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
                for (int i = 0; i < BlockSize - 1; ++i) {
                    block->elements[i].m_pNext = &block->elements[i + 1];
                    std::memset(&block->elements[i].data[0], 0, sizeof(T));
                }
                block->elements[BlockSize - 1].m_pNext = nullptr;
                std::memset(&block->elements[BlockSize - 1].data[0], 0, sizeof(T));

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

    };


    // Allows adding and removing of listeners in parallel
    template <typename T = void*>
    class Callback {
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
            size{ 0 };
        concurrency::concurrent_vector<Wrap>
            listeners;
        std::function<void(T*, long*)>
            func;
        // add a listener to the list
        __declspec(noinline) void add_listener(size_t index, T* p) {
            if (size <= index) {
                if (listeners.size() <= index) (void)listeners.grow_to_at_least((index + 2) + ((index + 2) % 16));
                InterlockedExchange(static_cast<volatile size_t*>(&size), index);
            }
            Wrap& wrap = listeners[index - 1];
            wrap.ptr = p;
            InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1 << 8);
            InterlockedIncrement(static_cast<volatile long*>(&wrap.alive));
        };
        // remove a listener from the list
        __declspec(noinline) void remove_listener(size_t index) {
            Wrap& wrap = listeners[index - 1];
            InterlockedDecrement(static_cast<volatile long*>(&wrap.alive));
            if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), -(1 << 8)) == 0) {}
            else while (wrap.count != 0) if (!wrap.ptr) InterlockedExchange(static_cast<volatile long*>(&wrap.count), 0);
            wrap.ptr = nullptr;
        };

    public:
        Callback(std::function<void(T*, long*)>&& listener)
            : func{ std::move(listener) }
        {};
        __declspec(noinline) ScopedListener listener(size_t index, T* p) {
            add_listener(index, p);
            return ScopedListener(index, *this);
        };
        // callback performed on all listeners
        __declspec(noinline) void speak(long* parent_alive) {
            for (size_t i = 0; i < size; ++i) {
                Wrap& wrap = listeners[i];
                if (wrap.alive) {
                    if (!parent_alive || *parent_alive) {
                        if (InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1) >= (1 << 8))
                            func(wrap.ptr, &wrap.alive);
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
                : _index(nullptr), _parent(nullptr) {};
            ScopedTicket(size_t* index, TicketDispensor& parent)
                : _index(index), _parent(&parent) {};
            ScopedTicket(ScopedTicket const& rhs) = delete;
            ScopedTicket(ScopedTicket&& rhs)
                : _index(std::move(rhs._index)), _parent(std::move(rhs._parent))
            {
                rhs._index = nullptr;
            };
            ScopedTicket& operator=(ScopedTicket const& rhs) = delete;
            ScopedTicket& operator=(ScopedTicket&& rhs)
            {
                _index = std::move(rhs._index);
                _parent = std::move(rhs._parent);
                rhs._index = nullptr;
                return *this;
            };
            ~ScopedTicket() {
                if (_index)
                    _parent->return_ticket(_index);
            };

            size_t* _index;
        private:
            TicketDispensor* _parent;
        };
            
    public:
        ABA_Problem::BlockAlloc<size_t, 64, true>
        // ABA_Problem::Allocator<size_t, 32, 4, true>
            alloc;
        std::atomic<size_t>
            indexes{ 0 };
        std::atomic<size_t*> 
            last_free{ nullptr }; // avoids using  "free" queue for the first free scope. 

    protected:
        __declspec(noinline) size_t* get_ticket() {
            size_t* out{ last_free.exchange(nullptr) };
            if (!out) out = alloc.Alloc();
            if (*out == 0) *out = ++indexes;
            return out; 
        };
        __declspec(noinline) void return_ticket(size_t* ticket) {
            if (ticket = last_free.exchange(ticket)) alloc.Free(ticket);            
        };
    public:
        size_t num_tickets() const {
            return indexes.load() + 1;
        };
        __declspec(noinline) ScopedTicket get_scoped_ticket() {
            return ScopedTicket(get_ticket(), *this);
        };

    };
};
namespace std {
    template <> struct hash<utilities::shared_string> {
        std::size_t operator()(const utilities::shared_string& k) const {
            return k.hash();
        };
    };
    template <> struct less<utilities::shared_string> {
        std::size_t operator()(const utilities::shared_string& lhs, const utilities::shared_string& rhs) const {
            return lhs < rhs;
        };
    };
    template <> struct greater<utilities::shared_string> {
        std::size_t operator()(const utilities::shared_string& lhs, const utilities::shared_string& rhs) const {
            return lhs > rhs;
        };
    };
    template <> struct equal_to<utilities::shared_string> {
        std::size_t operator()(const utilities::shared_string& lhs, const utilities::shared_string& rhs) const {
            return lhs == rhs;
        };
    };

};




class Scopes {
private:
    // clean-up the name of a scope: e.g. "::" becomes "::", "Units" becomes "::Units::"
    static utilities::compound_shared_string CleanUpScopeName(utilities::shared_string const& x) {
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
    class BasicScope;
    class NamespaceScope;
    class ClassScope;
    class RootScope;

private:
    // Identity of an individual scope
    class ScopeID {
    public:
        utilities::shared_string
            scope_name; // e.g. "Color"
        Scopes::BasicScope*
            scope;
        int
            scope_type; // may be a compound of multiple types, e.g. a root is also a namespace
        utilities::TicketDispensor::ScopedTicket
            scope_index; // unique index of this scope for check_flags
        utilities::shared_string
            current_namespace; // e.g. "::" or "::UI::Color::"

        ScopeID(utilities::shared_string const& scope_name_p = "", int scope_type_p = ScopeType::Basic)
            : scope_name{ scope_name_p }
            , scope{ nullptr }
            , scope_type{ scope_type_p }
            , scope_index{}
            , current_namespace{}
        {}
        ScopeID(ScopeID&&) = default;
        ScopeID(ScopeID const&) = default;
        ScopeID& operator=(ScopeID&&) = default;
        ScopeID& operator=(ScopeID const&) = default;
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

        Breadcrumb(ScopeID&& thisNode, Breadcrumb* parent = nullptr)
            : this_m(std::forward<ScopeID>(thisNode))
            , parent_m(parent)
        {
            // ROOT
            if (parent_m) root_m = parent_m->root_m;
            else root_m = this;

            // NAMESPACE
            if (this_m.is_namespace()) namespace_m = this;
            else if (parent_m) namespace_m = parent_m->namespace_m;
            else namespace_m = this->root_m;

            // current_namespace
            if (this_m.scope_name != utilities::shared_string::empty_string()) {
                if (parent_m) this_m.current_namespace = CleanUpScopeName(parent_m->this_m.current_namespace + this_m.scope_name);
                else this_m.current_namespace = CleanUpScopeName(this_m.scope_name);
            }
            else {
                if (parent_m) this_m.current_namespace = parent_m->this_m.current_namespace;
                else this_m.current_namespace = utilities::shared_string::namespace_colons();
            }

            // scope_index
            if (parent_m) {
                if (auto* root_ptr = dynamic_cast<RootScope*>(root_m->this_m.scope)) {
                    this_m.scope_index = root_ptr->scope_indexs.get_scoped_ticket();
                }
            }

        };
        ~Breadcrumb() = default;

    };


public:
    class BasicScope {
    public:
        Breadcrumb 
            breadcrumb_m;
        concurrency::concurrent_vector< utilities::shared_string >
            using_m; // NOTE: calling "using" should split the scope - e.g. using statements are appended staticly at compile time, NOT at runtime. 
        GoodLang::SharedLockable<std::map< utilities::shared_string, utilities::ObjectWrapper>>
            objects_m; // NOTE: adding objects should be appended staticly at compile time, NOT at runtime. E.g. the names are known, even if the types are not yet known. 
        
        size_t
            object_or_function_versions{ 0 };
        utilities::Callback<BasicScope>
            children_listeners;
        utilities::Callback<BasicScope>::ScopedListener
            listener;

        void UpdateObjectFunctionVersion(long* parent_alive = nullptr) {
            InterlockedIncrementNoFence(static_cast<volatile size_t*>(&object_or_function_versions));
            children_listeners.speak(parent_alive);
        };

    public:
        BasicScope(utilities::shared_string const& name = "", int scope_type_p = ScopeType::Basic, Breadcrumb* parent = nullptr)
            : breadcrumb_m(ScopeID(name, scope_type_p), parent)
            , children_listeners(&BasicScope::UpdateObjectFunctionVersion) // [](BasicScope* ptr, long* parent_alive) -> void { ptr->UpdateObjectFunctionVersion(parent_alive); }
        {
            breadcrumb_m.this_m.scope = this;
            if (this->breadcrumb_m.parent_m) {
                listener = this->breadcrumb_m.parent_m->this_m.scope->children_listeners.listener(*this->breadcrumb_m.this_m.scope_index._index, this);
            }
        };
        virtual ~BasicScope() = default;
    };

    class NamespaceScope : public BasicScope {
    public:
        // explicit children namespaces, with strongly-held protections to their memory.
        concurrency::concurrent_unordered_map<size_t, std::shared_ptr<NamespaceScope>>
            children;

    public:
        NamespaceScope(utilities::shared_string const& name = "", int scope_type_p = ScopeType::Basic & ScopeType::Namespace, Breadcrumb* parent = nullptr)
            : BasicScope(name, scope_type_p, parent)
            , children{}
        {};
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
        {};
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
    }
    print("");


    if (1) {
        ABA_Problem::Allocator< Node<size_t> > alloc;
        THead<Node<size_t>> head; head.m_n64 = 0;
        for (int i = 0; i < 1000000; ++i) {
            auto* p = new Node<size_t>((size_t)i, nullptr);
            ABA_Problem::Push(head, p);
            if (auto* z = Pop(head)) {
                delete z;
            }
            else {
                print("Failure1");
            }
        }
        GoodLang::parallel::For(0, 1000000, [&](int i) {
            auto* p = new Node<size_t>((size_t)i, nullptr);
            ABA_Problem::Push(head, p);
            if (auto* z = ABA_Problem::Pop(head)) {
                delete z;
            }
            else {
                print("Failure2");
            }
        });
        while (auto* z = ABA_Problem::Pop(head)) {
            print(z->data);
            delete z;
        }
    }






    while (true) {
        print("");
        
        for (int loopN = 0; loopN < 1000; loopN++) {
            Scopes::RootScope root;
            if (1) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                // EXPECT_EQ(*scope.breadcrumb_m.this_m.scope_index, 1);
                EXPECT_EQ(root.breadcrumb_m.this_m.scope_index._index, nullptr);
                EXPECT_EQ(scope.breadcrumb_m.root_m, &root.breadcrumb_m);
                EXPECT_EQ(scope.object_or_function_versions, 0);

                scope.UpdateObjectFunctionVersion();
                EXPECT_EQ(scope.object_or_function_versions, 1);
            }
            if (1) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                // EXPECT_EQ(*scope.breadcrumb_m.this_m.scope_index, 1);
                EXPECT_EQ(root.breadcrumb_m.this_m.scope_index._index, nullptr);
                EXPECT_EQ(scope.breadcrumb_m.root_m, &root.breadcrumb_m);
                EXPECT_EQ(scope.object_or_function_versions, 0);

                Scopes::BasicScope scope2("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                // EXPECT_EQ(*scope2.breadcrumb_m.this_m.scope_index, 2);
                EXPECT_EQ(root.breadcrumb_m.this_m.scope_index._index, nullptr);
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

        if (1) {
            Scopes::RootScope root;

            sw.Start();
            for(int i = 0; i < 1000000; ++i) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                scope.UpdateObjectFunctionVersion();
                EXPECT_EQ(scope.object_or_function_versions, 1);
            };
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.UpdateObjectFunctionVersion();
            })) {
                for (int i = 0; i < 1000000; ++i) {
                    Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                    scope.UpdateObjectFunctionVersion();
                    EXPECT_EQ(true, scope.object_or_function_versions >= 2);
                };
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                scope.UpdateObjectFunctionVersion();
                EXPECT_EQ(scope.object_or_function_versions, 1);
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.UpdateObjectFunctionVersion();
            })) {
                GoodLang::parallel::For(0, 1000000, [&](int i) {
                    Scopes::BasicScope scope("", Scopes::ScopeType::Basic, &root.breadcrumb_m);
                    scope.UpdateObjectFunctionVersion();                    
                    EXPECT_EQ(true, scope.object_or_function_versions >= 2);
                });
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            print(root.scope_indexs.num_tickets());
        }

       
        if (1) {
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                shared_string test = shared_string("TEST");
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) {
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                shared_string test = shared_string(std::string("TEST"));
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) {
            sw.Start();
            std::string_view sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                shared_string test = shared_string(sv);
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) {
            sw.Start();
            shared_string sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                shared_string test = shared_string(sv);
                (void*)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        EXPECT_EQ(shared_string("::std::string::").remove_leading_and_trailing(':'), "std::string");
        EXPECT_EQ(shared_string("::std::").replace("::", ""), "std");
        EXPECT_EQ(shared_string("Hello World!"), "Hello World!");
        EXPECT_EQ(false, (shared_string("Hello World!\n") == shared_string("TEST")));

        if (0) {
            sw.Start();            
            concurrency::concurrent_unordered_map<std::string, size_t> Set;
            GoodLang::parallel::For(0, 1000000, [&Set](int i) {
                Set.insert({ GoodLang::ToString(i % 100), 0 });
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (0) {
            sw.Start();
            concurrency::concurrent_unordered_map<shared_string, size_t> Set;
            GoodLang::parallel::For(0, 1000000, [&Set](int i) {
                Set.insert({ GoodLang::ToString(i % 100), 0 });
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

        EXPECT_EQ(shared_string("").hash(), compound_shared_string("").hash());
        EXPECT_EQ(shared_string("Hello World!\n").hash(), compound_shared_string("Hello World!\n").hash());        
    }

};
