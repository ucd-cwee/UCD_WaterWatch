#pragma region "Includes"
#pragma once
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

        string(std::shared_ptr<std::string> _d, std::string_view d) : _data(std::move(_d)), data(std::move(d)) {};

    public:
        string() {};
        string(string const&) = default;
        string(string&&) = default;
        string& operator=(string const&) = default;
        string& operator=(string&&) = default;
        virtual ~string() = default;

        template <size_t N> __forceinline string(const char(&r)[N]) : data(r) {};
        string(std::string&& _Copy) : _data(std::make_shared<std::string>(std::move(_Copy))) {
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
        friend std::ostream& operator<<(std::ostream& os, string const& obj) {
            os << obj.data;
            return os;
        };
        friend string operator+(string const& A, string const& B) { return string(std::string(A.data) + std::string(B.data)); };
    private:
        static size_type	        FindString(std::string_view const& str, std::string_view const& text, bool casesensitive = true, long long start = 0, long long end = -1) {
            long long l, j, k;
            k = text.length();
            if (end == -1) {
                end = str.length();
            }
            l = end - k;

            if (k <= 0 || (l - start) < 0) return std::string::npos;

            if (casesensitive) {
                const char sample = text[0];
                if (!sample) return (size_t)start;
                for (; start <= l; ++start) // starting at the search position ... 
                    if (str[start] == sample)  // found a match for the first character ...
                        for (j = 1; ; ++j) { // for the remaining parts of the search text ... 
                            if (j >= k) return start;
                            if (str[start + j] != text[j]) break;
                        }
            }
            else {
                for (; start <= l; ++start)
                    for (j = 0;; j++) {
                        if (j >= k) return (size_t)start;
                        if (::toupper(str[start + j]) != ::toupper(text[j]))
                            break;
                    }
            }
            return std::string::npos;
        };
        static bool                 ReplaceString(string& String, const std::string_view& from, const std::string_view& to) {
            size_t startPos;
            bool ret;

            ret = false;
            if (from.empty() || (from == to)) return ret;

            startPos = FindString(String.data, from, true, 0);
            if (startPos != std::string::npos) String = string(std::string(String.data)); // make a new copy of the data
            while (startPos != std::string::npos) {
                ret = true;
                String._data->replace(startPos, from.length(), to);
                String.data = *String._data;
                startPos = FindString(String.data, from, true, to.length() + startPos);
            }
            return ret;
        };

    public:
        std::string to_string() const {
            return std::string(data);
        };
        std::string_view const& c_str() const {
            return data;
        };
        string substr(const size_type _Off = 0, size_type _Count = npos) const {
            return string(this->_data, data.substr(_Off, _Count));
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
        string& remove_prefix(const size_type _Count) noexcept {
            data.remove_prefix(_Count);
            return *this;
        };
        string& remove_suffix(const size_type _Count) noexcept {
            data.remove_suffix(_Count);
            return *this;
        };
        string& remove_prefix(const string& prefix) noexcept {
            if (left(prefix.size()) == prefix) data.remove_prefix(prefix.size());
            return *this;
        };
        string& remove_suffix(const string& suffix) noexcept {
            if (right(suffix.size()) == suffix) data.remove_suffix(suffix.size());
            return *this;
        };
        size_type rfind(const string& _Right) const {
            return data.rfind(_Right.data);
        };
        size_type find(const string& FIND, bool casesensitive = true, long long start = 0, long long end = -1) const {
            if (end == -1) {
                end = this->length();
            }
            if (this->length() == 0 || FIND.length() == 0) return std::string::npos;
            if (FIND.length() > this->length()) return std::string::npos;
            return FindString(this->data, FIND.data, casesensitive, start, end);
        };
        string replace(const string& what, const string& with) const {
            string out(*this);
            (void)ReplaceString(out, what.data, with.data);
            return out;
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
        string right(size_type _Count) const {
            if (_Count >= length()) return *this;
            else return string(this->_data, this->data.substr(this->data.length() - _Count, _Count));
        };
        string left(size_type _Count) const {
            if (_Count >= length()) return *this;
            else return string(this->_data, this->data.substr(0, _Count));
        };
        // returns the part of this string that is left of the searched content, if found. Otherwise returns everything.
        string left_of(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return substr(0, p);
            }
            else {
                return *this;
            }
        };
        // returns the part of this string that is right of the searched content, if found. Otherwise returns everything.
        string right_of(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return substr(p + what.length());
            }
            else {
                return *this;
            }
        };
        // returns the part of this string that is left of the searched content, if found. Otherwise returns everything.
        string left_of_last(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return substr(0, p);
            }
            else {
                return *this;
            }
        };
        // returns the part of this string that is right of the searched content, if found. Otherwise returns everything.
        string right_of_last(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return substr(p + what.length());
            }
            else {
                return *this;
            }
        };
        std::pair<string, string> left_and_right_of(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return std::pair<string, string>{ substr(0, p), substr(p + what.length()) };
            }
            else {
                return std::pair<string, string>{ *this, "" };
            }
        };
        std::pair<string, string> left_and_right_of_last(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return std::pair<string, string>{ substr(0, p), substr(p + what.length()) };
            }
            else {
                return std::pair<string, string>{ *this, "" };
            }
        };
        bool ends_with(const string& what) const {
            if (auto p = this->rfind(what); p != npos) {
                return (p + what.length()) == this->length();
            }
            else {
                return false;
            }
        };
        bool begins_with(const string& what) const {
            if (auto p = this->find(what); p != npos) {
                return p == 0;
            }
            else {
                return false;
            }
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

namespace utilities {
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
            std::string out;
            out.append(a.c_str().data(), a.c_str().length());
            out.append(b.c_str().data(), b.c_str().length());
            out.append(c.c_str().data(), c.c_str().length());
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
        template<class T> __declspec(noinline) void Push(THead<T>& Head, T* pNode) {
            THead<T> Old, New;
            while (1) { // race loop                
                New.m_n64 = Old.m_n64 = Head.m_n64; // Get an atomic copy of head and call it old. Copy old and call it new.                
                pNode->m_pNext = New.Node(); // Wire node t Head                
                New.Node(pNode); // change New's head ptr, which bumps internal aba                
                if (CAS(&Head.m_n64, Old.m_n64, New.m_n64)) // compare and swap New with Head if it still matches Old.
                    break; // success                
            } // race, try again
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
                block_t& block = *blocks.grow_by(1);

                // add the new elements to the list
                // std::memset(&block->elements[0], 0, sizeof(block_t));
                for (int i = 0; i < BlockSize - 1; ++i) block.elements[i].m_pNext = &block.elements[i + 1];
                block.elements[BlockSize - 1].m_pNext = nullptr;

                // push pNode onto head of list.
                uint64_t old;
                THead<element_t> New;
                while (true) { // race loop
                    // Get an atomic copy of head and call it old.
                    // Copy old and call it new.                    
                    old = New.m_n64 = free.m_n64;

                    // Wire the tail of this block to connect to the old head ptr
                    block.elements[BlockSize - 1].m_pNext = New.Node();

                    // change New's head ptr, which bumps internal aba
                    New.Node(&block.elements[0]); // head shall be the start of this block

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
                        for (auto& element : block.elements) {
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

        template <typename _type_, typename DeleteListType = concurrency::concurrent_queue<std::pair<long long, _type_*>>>
        class EpochAllocator {
        private:
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
            Allocator<_type_, 32> // , 32 // ABA_Problem::BlockAlloc<_type_, 32> // 
                _alloc;
            DeleteListType
                _delete_list; // note that these are NOT available for re-use yet -- these may still be being used by certain threads. 
            GoodLang::ThreadLocalInstance<TLS>
                _TLS;
            long long 
                _lastGC;

        public:
            // Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
            void RunGC()  {
                static constexpr long long duration_ms{ 5 };
                static thread_local std::pair<long long, _type_*> out{};
                if ((GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch() - _lastGC) > duration_ms) {
                    InterlockedExchange64(reinterpret_cast<volatile long long*>(&_lastGC), GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch());

                    long long _EpochLimit{ std::numeric_limits<long long>::max() };

                    _TLS.for_each([&_EpochLimit](TLS& _tls) {
                        long long L = _tls.EpochLimit;
                        if (L >= 0) {
                            _EpochLimit = std::min<long long>(_EpochLimit, L);
                        }
                    });

                    if ((_EpochLimit > 0) && (_EpochLimit < std::numeric_limits<long long>::max())) {
                        while (_delete_list.try_pop(out)) {
                            if (out.first < _EpochLimit) { // deemed safe to delete                                
                                _alloc.Free(out.second);
                            }
                            else { // deemed unsafe to delete just yet                                
                                _delete_list.push(out); // pushing to the end of the queue is lazy deferred sorting -- literally wasting time and hoping it'll be sorted later-on.
                                break;
                            }
                        }
                    }
                }

            };

        public:
            using GuardType = typename TLS::EpochGuard;

            EpochAllocator() 
                : _alloc{}
                , _delete_list{}
                , _TLS{}
                , _lastGC{ 0 }
            {};
            EpochAllocator(EpochAllocator const&) = delete;
            EpochAllocator(EpochAllocator&&) = delete;
            EpochAllocator& operator=(EpochAllocator const&) = delete;
            EpochAllocator& operator=(EpochAllocator &&) = delete;
            ~EpochAllocator() = default;

            void unsafe_unload() {
                _alloc.unsafe_unload();
            };

        public:
            GuardType ProtectCurrentEpoch() const {
                return TLS::EpochGuard(
                    const_cast<EpochAllocator*>(this), 
                    const_cast<TLS*>(&*_TLS), 
                    GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch()
                );
            };
            void ProtectCurrentEpoch_Fast() const {
                const_cast<TLS*>(&*_TLS)->ForwardEpoch(GoodLang::EpochGarbageCollectorImpl::ThreadManager::GetCurrentEpoch());
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

    /// <summary>
    /// Thread-safe and fiber-safe wrapper for atomic operations on pointers, without having to utilize std_atomic(T*)
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template< typename T>
    struct atomic_ptr {
    private:
        // pop pNode from head of list.
        static T* Pop(ABA_Problem::THead<T>& Head) {
            ABA_Problem::THead<T> Old, New;
            while (1) { // race loop
                // Get an atomic copy of head and call it old.
                Old.m_n64 = Head.m_n64;
                if (Old.is_null()) return nullptr;
                // 
                New.m_n64 = Old.m_n64;
                New.Node(Old.Node());
                // compare and swap New with Head if it still matches Old.
                if (ABA_Problem::CAS(&Head.m_n64, Old.m_n64, New.m_n64)) return ABA_Problem::THead<T>::Finalize(Old.Node()); // success                
                // race, try again
            }
        }

        // push pNode onto head of list.
        static T* Push(ABA_Problem::THead<T>& Head, T* pNode) {
            ABA_Problem::THead<T> Old, New;
            while (1) { // race loop
                // Get an atomic copy of head and call it old.
                // Copy old and call it new.
                New.m_n64 = Old.m_n64 = Head.m_n64;
                // change New's head ptr, which bumps internal aba
                New.Node(pNode);
                // compare and swap New with Head if it still matches Old.
                if (ABA_Problem::CAS(&Head.m_n64, Old.m_n64, New.m_n64)) break; // success
                // race, try again
            }
            return Old.Node();
        }

    public:
        atomic_ptr() noexcept {
            ptr.m_n64 = 0;
        };
        atomic_ptr(T* newSource) noexcept {
            ptr.Node(newSource);
        };
        atomic_ptr(const atomic_ptr& other) noexcept {
            ptr.Node(other.ptr.Node());
        };
        atomic_ptr& operator=(const atomic_ptr& other) noexcept { Set(other.Get()); return *this; };
        atomic_ptr& operator=(T* newSource) noexcept { Set(newSource); return *this; };
        ~atomic_ptr() = default;

        explicit operator bool() { return !ptr.is_null(); };
        explicit operator bool() const { return !ptr.is_null(); };

        operator T* () noexcept { return Pop(ptr); };
        operator const T* () const noexcept { return Pop(ptr); };

        /* atomically sets the pointer and returns the previous pointer value */
        T* Set(T* newPtr) noexcept {
            return Push(ptr, newPtr);
        };
        T* Get() noexcept { return Pop(ptr); };
        T* Get() const noexcept { return Pop(ptr); };
        T* load() noexcept { return Get(); };
        T* load() const noexcept { return Get(); };
        T* operator->() noexcept { return Get(); };
        const T* operator->() const noexcept { return Get(); };

    protected:
        mutable ABA_Problem::THead<T> ptr;
    };

    template <typename T>
    class DelayedInstantiation {
    private:
        static ABA_Problem::BlockAlloc<T, 8>& shared_alloc() {
            static ABA_Problem::BlockAlloc<T, 8> alloc;
            return alloc;
        };
        T* ptr{ nullptr };

    public:
        DelayedInstantiation() = default;
        DelayedInstantiation(T const& data) : ptr(shared_alloc().Alloc(data)) {};
        DelayedInstantiation(T && data) : ptr(shared_alloc().Alloc(std::move(data))) {};
        DelayedInstantiation(DelayedInstantiation const&) = delete;
        DelayedInstantiation(DelayedInstantiation&& rhs) : ptr(std::move(rhs.ptr)) { rhs.ptr = nullptr; };
        DelayedInstantiation& operator=(DelayedInstantiation const&) = delete;
        DelayedInstantiation& operator=(DelayedInstantiation&& rhs) {
            if (ptr) shared_alloc().Free(ptr);
            ptr = std::move(rhs.ptr);
            rhs.ptr = nullptr;
            return *this;
        };
        ~DelayedInstantiation() {
            if (ptr) shared_alloc().Free(ptr);
        };

        bool valid() const {
            return ptr;
        };
        T* operator->() const {
            if (!ptr) {
                if (auto* newPtr = shared_alloc().Alloc()) {
                    if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(reinterpret_cast<PVOID*>(const_cast<T**>(&ptr))), newPtr, nullptr) != nullptr) {
                        shared_alloc().Free(newPtr);
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
                Wrap& wrap = _listeners[index/* - 1*/];
                InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&wrap.ptr), static_cast<PVOID>(p));
                InterlockedAdd(static_cast<volatile long*>(&wrap.count), 1 << 8);
                InterlockedIncrement(static_cast<volatile long*>(&wrap.alive));
            }
        };
        // remove a listener from the list
        __declspec(noinline) void remove_listener(size_t index) {
            if (alive.load() && _listeners.size() >= index) {
                Wrap& wrap = _listeners[index/* - 1*/];
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

        ObjectWrapper()
            : object_state{ nullptr }
        {};
        ObjectWrapper(GoodLang::Any obj, int s = 0)
            : object_state{ GoodLang::make_shared<std::pair<GoodLang::Any, int>>(std::move(obj), s) }
        {
            if (auto copy = object_state) {
                if (copy->first.GetFlag(GoodLang::AnyData::Flag::constant)) {
                    copy->second |= Constant;
                }
                if (copy->second & Constant) {
                    copy->first.SetFlag(GoodLang::AnyData::Flag::constant, true);
                }
            }
        };
        ObjectWrapper(ObjectWrapper const&) = default;
        ObjectWrapper(ObjectWrapper &&) = default;
        ObjectWrapper& operator=(ObjectWrapper const&) = default;
        ObjectWrapper& operator=(ObjectWrapper&&) = default;
        ~ObjectWrapper() = default;
    private:
        GoodLang::shared_ptr< std::pair<GoodLang::Any, int> > object_state;

    public:
        GoodLang::Any* operator->() const { 
            if (auto* p =object_state.get()) {
                return &p->first;
            }
            else {
                return nullptr;
            }
        };
        GoodLang::Any& operator*() const {
            return *operator->();
        };

        bool is_const() const {
            if (auto copy = object_state) {
                return copy->second & Constant;
            }
            return false;
        };
        bool is_static() const {
            if (auto copy = object_state) {
                return copy->second & Static;
            }
            return false;
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
        DelayedInstantiation< utilities::ABA_Problem::EpochAllocator<objType> >
            objAllocator;
        utilities::ABA_Problem::EpochAllocator<_iterType>
            nodeAllocator;
        GoodLang::fast_shared_mutex // mutable std::shared_mutex // 
            mutex;

        class EpochGuard {
        private:
            typename utilities::ABA_Problem::EpochAllocator<objType>::GuardType guard_1;
            typename utilities::ABA_Problem::EpochAllocator<_iterType>::GuardType guard_2;

        public:
            EpochGuard(BTree const* parent) : guard_1{ parent->objAllocator->ProtectCurrentEpoch() }, guard_2{ parent->nodeAllocator.ProtectCurrentEpoch() } {};
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
        BTree(BTree const&) = delete;
        BTree(BTree&& rhs) = delete;
        BTree& operator=(BTree const&) = delete;
        BTree& operator=(BTree&&) = delete;
        ~BTree() = default;

        void unsafe_unload() {
            if (objAllocator) objAllocator->unsafe_unload();
            nodeAllocator.unsafe_unload();
            root = first = last = nullptr;
            Num = 0;
        };

        template <bool EmplaceIfExists = true> _iterType* 
            Add(objType object, keyType const& key) {
            _iterType
                *node, 
                *child, 
                *newNode; 

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

    // fast, thread-safe sorted map. Allows simultaneous reading / writing / erasure. Slower than concurrent_unordered_map when erasure is not necessary. 
    template<class KeyType, class ValueType> class atomic_map {
        friend class it_state;
    protected:
        DelayedInstantiation<BTree<ValueType, KeyType>>
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
            using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

            // data
            mutable typename BTree<ValueType, KeyType>::_iterType*
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

    // fast, thread-safe unsorted map. Allows simultaneous reading / writing / erasure. Slower than concurrent_unordered_map when erasure is not necessary. 
    template<class KeyType, class ValueType, typename HashType = std::hash<KeyType>> class atomic_unordered_map {
        friend class it_state;
    protected:
        std::unique_ptr< DelayedInstantiation<BTree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>> >
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
            typename  BTree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>::GuardType
                guard;

        public:
            const KeyType&
                first;
            ValueType&
                second;

            WrappedReference(const KeyType& _first, ValueType& _second, BTree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>* _parent)
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
            : tree{ std::make_unique< DelayedInstantiation<BTree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>>>() }, hasher{ HashType{} }
        {};
        atomic_unordered_map(atomic_unordered_map const& rhs) = delete;
        atomic_unordered_map(atomic_unordered_map&& rhs) : tree{ std::move(rhs.tree) }, hasher{ HashType{} } 
        {};
        atomic_unordered_map& operator=(atomic_unordered_map const& rhs) = delete;
        atomic_unordered_map& operator=(atomic_unordered_map&& rhs) = delete;
        ~atomic_unordered_map() = default;

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
            using thisType = atomic_unordered_map;
            using value_type = WrappedReference;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

            // data
            mutable typename  BTree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>::_iterType*
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
        SETUP_ITERATOR(atomic_unordered_map, it_state);
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





    // To-Do, need to roll my own Any, Params, etc.

    // class any;

    // To-Do, update the onversion functions for scripted types once everything is figured out. 
    // type records the type of either built-in or scripted, runtime types    
    class type {
    public:
        enum Modifiers {
            Const = 1,
            Reference = 2,
            Temporary = 4,
            Any = 8,
            Void = 16
        };

    protected:        
        size_t 
            underlying_hash; 
        size_t
            hash;
        unsigned short 
            modifiers = 0;        
        utilities::string 
            name; // for scripted classes, this shall be the full namespace path, e.g. ::std::string::
        std::function<GoodLang::Any(GoodLang::Any const&)>* 
            copy_constructor; 
        std::function<GoodLang::Any(GoodLang::Any const&)>* 
            constructor_from_value;

    public:
        size_t const& get_hash() const { return hash; };
        type(size_t underlying_hash_p = 0, size_t modifiers_p = Modifiers::Void, utilities::string const& name_p = "", std::function<GoodLang::Any(GoodLang::Any const&)>* copy_constructor_p = nullptr, std::function<GoodLang::Any(GoodLang::Any const&)>* constructor_from_value_p = nullptr) noexcept
            : underlying_hash(underlying_hash_p)
            , hash(underlying_hash_p ^ (modifiers_p + 0x9e3779b9 + (underlying_hash_p << 6) + (underlying_hash_p >> 2)))
            , modifiers((unsigned short)modifiers_p)
            , name(std::move(name_p))
            , copy_constructor(copy_constructor_p)
            , constructor_from_value(constructor_from_value_p)
        {};
        type(type const&) = default;
        type(type&&) = default;
        type& operator=(type const&) = default;
        type& operator=(type&&) = default;
        ~type() = default;

        // Returns true if the types are similar enough to be casted for free (0 cost)
        static bool can_free_cast(type const& from, type const& to) {
            if (from.underlying_hash == to.underlying_hash) {
                // anything can convert into const T&
                if (to.is_const_ref()) return true;

                // cannot cast-away the const-ness
                if (from.is_const() && !to.is_const()) return false;

                // temporary (T&&) can be used for a base
                if (from.is_temp() && to.is_base()) return true;

                // temporary (T&&) cannot be used as const-less references (T&)
                if (from.is_temp() && to.is_ref()) return false;

                // T& cannot cast to T or T&& without a conversion function
                if (from.is_ref() && (to.is_temp() || to.is_base())) return false;

                // Otherwise OK
                return true;
            }
            return false;            
        };
        // Returns true if the types are similar enough to be casted for free (0 cost)
        bool can_free_cast(type const& to) const { return can_free_cast(*this, to); };

        //// Operators
        friend bool operator==(const type& a, const type& b) noexcept { return a.get_hash() == b.get_hash(); };
        friend bool operator!=(const type& a, const type& b) noexcept { return a.get_hash() != b.get_hash(); };
        friend bool operator<(const type& a, const type& b) noexcept { return a.get_hash() < b.get_hash(); };
        friend bool operator<=(const type& a, const type& b) noexcept { return a.get_hash() <= b.get_hash(); };
        friend bool operator>(const type& a, const type& b) noexcept { return a.get_hash() > b.get_hash(); };
        friend bool operator>=(const type& a, const type& b) noexcept { return a.get_hash() >= b.get_hash(); };
        bool is_temp() const noexcept { return modifiers & Modifiers::Temporary; };
        bool is_const() const noexcept { return is_temp() ? false : (modifiers & Modifiers::Const); };        
        bool is_ref() const noexcept { return is_temp() ? false : (modifiers & Modifiers::Reference); };
        bool is_const_ref() const noexcept { return is_temp() ? false : (modifiers & (Modifiers::Const | Modifiers::Reference)); };
        bool is_base() const noexcept { return modifiers == 0; };
        bool is_any() const noexcept { return modifiers & Modifiers::Any; };
        bool is_void() const noexcept { return modifiers & Modifiers::Void; };

        utilities::string get_name() const {
            auto out{ name.remove_leading_and_trailing(':').remove_suffix(" __cdecl(void)") };
            if (is_temp()) return out + "&&";
            else if (is_const() && is_ref()) return "const " + out + "&";
            else if (is_const() && !is_ref()) return "const " + out;
            else if (!is_const() && is_ref()) return out + "&";
            else return out;
        };

        type operator+(Modifiers modifier) const {
            return type(this->underlying_hash, this->modifiers | modifier, this->name, this->copy_constructor, this->constructor_from_value);
        };
        type operator-(Modifiers modifier) const {
            return type(this->underlying_hash, this->modifiers & ~modifier, this->name, this->copy_constructor, this->constructor_from_value);
        };

        // const This& to This&&
        std::function<GoodLang::Any(GoodLang::Any const&)> const& GetCopyConstructor() const {
            if (copy_constructor) {
                return *copy_constructor;
            }
            else {
                static std::function<GoodLang::Any(GoodLang::Any const&)> passthrough = [](GoodLang::Any const& p) -> GoodLang::Any { return p; };
                return passthrough;
            }
        }; 
        // const value_t& to This&&
        std::function<GoodLang::Any(GoodLang::Any const&)> const& GetConstructorFromValue() const {
            if (constructor_from_value) {
                return *constructor_from_value;
            }
            else {
                static std::function<GoodLang::Any(GoodLang::Any const&)> passthrough = [](GoodLang::Any const& p) -> GoodLang::Any { return p; };
                return passthrough;
            }
        };

    };
    template<typename T> static type const& type_of() noexcept {
        using base_type = typename std::decay<T>::type; // e.g. int&& -> int, or const int& -> int, or int* const& -> int*
        static auto const& underlying_type = GoodLang::impl::TypeId<base_type>();
        static auto const& void_type = GoodLang::impl::TypeId<void>();
        static auto const& any_type = GoodLang::impl::TypeId<GoodLang::Any>();

        static auto const const_modifier = std::is_const<typename std::remove_pointer_t<typename std::remove_reference_t<T>>>::value ? type::Modifiers::Const : 0;
        static auto const ref_modifier = std::is_reference<typename std::remove_pointer<T>::type>::value ? type::Modifiers::Reference : 0;
        static auto const void_modifier = (void_type.hash_code() == underlying_type.hash_code()) ? type::Modifiers::Void : 0;
        static auto const any_modifier = (any_type.hash_code() == underlying_type.hash_code()) ? type::Modifiers::Any : 0;

        static std::function<GoodLang::Any(GoodLang::Any const&)> copy_constructor = [](GoodLang::Any const& from) -> GoodLang::Any {  
            if constexpr (std::is_copy_constructible_v<base_type>) 
                if (void_modifier == 0) 
                    return base_type{ from.cast<base_type>() };

            // To-Do, the return type should be set to "temporary" to improve type engine

            /* scripted objects -->
            auto& dynObj = from.cast<DynamicObject>();
            return DynamicObject(dynObj);
            <-- scripted objects */

            return from;
        };
        static std::function<GoodLang::Any(GoodLang::Any const&)> constructor_from_value = [](GoodLang::Any const& from) -> GoodLang::Any {
            if constexpr (std::is_constructible_v<base_type, GoodLang::Units::value&>)
                if (from.IsTypeOf<GoodLang::Units::value>()) 
                    return base_type{ from.cast<GoodLang::Units::value&>() };
            
            // To-Do, the return type should be set to "temporary" to improve type engine

            return from;
        };
        static utilities::type out(underlying_type.hash_code(), const_modifier | ref_modifier | void_modifier | any_modifier, utilities::string(std::string_view(underlying_type.name())), &copy_constructor, &constructor_from_value);
        return out;
    };
    static type type_of(utilities::string const& full_namespace_path) {
        if (full_namespace_path.empty()) {
            return type_of<void>();
        }
        else {
            size_t underlying_hash = full_namespace_path.hash();
            static std::function<GoodLang::Any(GoodLang::Any const&)> copy_constructor = [](GoodLang::Any const& from) -> GoodLang::Any {
                
                /* scripted objects -->
                auto& dynObj = from.cast<DynamicObject>();
                return DynamicObject(dynObj);
                <-- scripted objects */

                return from;
            };
            static std::function<GoodLang::Any(GoodLang::Any const&)> constructor_from_value = [](GoodLang::Any const& from) -> GoodLang::Any {
                // To-Do, the return type should be set to "temporary" to improve type engine
                return from;
            };
            return type(underlying_hash, 0, full_namespace_path, &copy_constructor, &constructor_from_value);
        }
    };

    // Collection of one or more types, which can be appended or added together to create a types collection. operator= is not thread-safe. 
    class types {
    private:
        utilities::DelayedInstantiation<concurrency::concurrent_vector<type>>
            types_m;
        mutable size_t hash = 0;

        types(concurrency::concurrent_vector<type> const& d) : types_m(d) {};
        types(std::vector<type> const& d) : types_m(concurrency::concurrent_vector<type>{ d.begin(), d.end() }) {};
    public:
        types() = default;
        types(type const& d) : types_m(concurrency::concurrent_vector<type>(1, d)) {};
        types(types const& rhs) : types_m() {
            if (rhs.types_m) {
                *types_m = *rhs.types_m;
            }
        };
        types(types&&) = default;
        // not thread-safe
        types& operator=(types const& rhs) {
            if (rhs.types_m) {
                *types_m = *rhs.types_m;
            }
            else if (types_m) {
                types_m->clear();
            }
            return *this;
        };
        // not thread-safe
        types& operator=(types&&) = default;
        ~types() = default;

        friend bool operator==(const types& a, const types& b) noexcept { return a.get_hash() == b.get_hash(); };
        friend bool operator!=(const types& a, const types& b) noexcept { return a.get_hash() != b.get_hash(); };
        friend bool operator<(const types& a, const types& b) noexcept { return a.get_hash() < b.get_hash(); };
        friend bool operator<=(const types& a, const types& b) noexcept { return a.get_hash() <= b.get_hash(); };
        friend bool operator>(const types& a, const types& b) noexcept { return a.get_hash() > b.get_hash(); };
        friend bool operator>=(const types& a, const types& b) noexcept { return a.get_hash() >= b.get_hash(); };

        // thread-safe
        friend types operator+(types const& lhs, types const& rhs) {
            if (lhs.types_m) {
                if (rhs.types_m) {
                    std::vector<type> out;
                    out.reserve(lhs.types_m->size() + rhs.types_m->size());
                    out.insert(out.end(), lhs.types_m->begin(), lhs.types_m->end());
                    out.insert(out.end(), rhs.types_m->begin(), rhs.types_m->end());
                    return out;
                }
                else {
                    return *lhs.types_m;
                }
            }
            else if (rhs.types_m) {
                return *rhs.types_m;
            }
            else {
                return {};
            }
        };
        // thread-safe
        types& operator+=(types const& rhs) {
            if (rhs.types_m) {
                for (auto& x : *rhs.types_m) types_m->push_back(x);
            }
            return *this;
        };
        // thread-safe
        const type& operator[](size_t index) const {
            if (types_m && types_m->size() > index) {
                return types_m->operator[](index);
            }
            else {
                return type_of<void>();
            }
        };
        // thread-safe
        size_t size() const {
            if (types_m) {
                return types_m->size();
            }
            else {
                return 0;
            }
        };
        // thread-safe
        size_t get_hash() const {
            if ((hash == 0) && types_m) {
                size_t out = 0;
                for (auto& x : *types_m) {
                    out ^= x.get_hash() + 0x9e3779b9 + (out << 6) + (out >> 2);
                }
                InterlockedExchange(reinterpret_cast<volatile size_t*>(&hash), out);
            }
            return hash;
        };

    };


    class shared_ptr_base {
    public:
        struct aux {
            long long // strong (first short), weak (second short), destroy flag (third short), delete flag (fourth short)
                Strong_Weak_Destroy_Delete{ 1 }; // strong = 1, weak = 0, destroy = 0, delete = 0
            void*
                p;

            aux()
                : Strong_Weak_Destroy_Delete{ 1 }
                , p{ nullptr }
            {};
            aux(void* pu)
                : Strong_Weak_Destroy_Delete{ 1 }
                , p{ pu }
            {}
            aux(aux const&) = default;
            aux(aux &&) = default;
            aux& operator=(aux const&) = default;
            aux& operator=(aux&&) = default;
            virtual ~aux() = default;

            void* ptr() const { return p; };
            virtual utilities::type const& type() const = 0;
            virtual void /*std::shared_ptr<void>*/ protect_aux() = 0;
            virtual void destroy_aux() = 0;
            virtual void destroy_obj() = 0;
        };

        static __declspec(noinline) aux* inc_strong(GoodLang::atomic_ptr<aux> const& pa) {
            aux
                * pa_ptr{ nullptr };
            long long
                read;

            if (pa_ptr = pa.load()) {
                (void)pa_ptr->protect_aux();
            }
            while (pa_ptr = pa.load()) {
                read = InterlockedAdd64(reinterpret_cast<volatile long long*>(&pa_ptr->Strong_Weak_Destroy_Delete), 1); // increments the strong count, regardless of the others
                if (
                    (reinterpret_cast<short*>(&read)[0] >= 1)
                    && (reinterpret_cast<long*>(&read)[1] == 0)
                ) { // if NOT being destroyed or deleted...
                    return pa_ptr; // remember - I am still locked from deletion.
                }
                else {
                    InterlockedAdd64(reinterpret_cast<volatile long long*>(&pa_ptr->Strong_Weak_Destroy_Delete), -1); // failure
                }
            }
            return nullptr;
        };
        static __declspec(noinline) void dec_strong(aux* pa_ptr) {
            long long
                read,
                planned;

            while (pa_ptr) {
                read = pa_ptr->Strong_Weak_Destroy_Delete;
                if (!reinterpret_cast<long*>(&read)[1]) { // if NOT being destroyed or deleted...
                    planned = read;
                    --reinterpret_cast<short*>(&planned)[0];
                    if (reinterpret_cast<short*>(&planned)[0] <= 0) {
                        // flag that we plan on deleting the data!
                        reinterpret_cast<short*>(&planned)[2] = 1;
                    }
                    if (reinterpret_cast<short*>(&planned)[1] <= 0) {
                        // flag that we plan on deleting the mem_block!
                        reinterpret_cast<short*>(&planned)[3] = 1;
                    }
                    if (InterlockedCompareExchange64(reinterpret_cast<volatile long long*>(&pa_ptr->Strong_Weak_Destroy_Delete), planned, read) == read) { // success!
                        if (reinterpret_cast<short*>(&planned)[2] == 1) {
                            pa_ptr->destroy_obj();
                        }
                        if (reinterpret_cast<short*>(&planned)[3] == 1) {
                            pa_ptr->destroy_aux();
                        }
                        break;
                    }
                }
                else {
                    break;
                }
            }
        };
        static __declspec(noinline) aux* inc_weak(GoodLang::atomic_ptr<aux> const& pa) {
            aux
                * pa_ptr{ nullptr };
            long long
                read,
                planned;

            if (pa_ptr = pa.load()) {
                (void)pa_ptr->protect_aux();
            }
            while (pa_ptr = pa.load()) {
                read = pa_ptr->Strong_Weak_Destroy_Delete;
                planned = read;
                if (!reinterpret_cast<long*>(&read)[1]) { // if NOT being destroyed or deleted...
                    // add to the weak count
                    ++reinterpret_cast<short*>(&planned)[1];
                    if (InterlockedCompareExchange64(reinterpret_cast<volatile long long*>(&pa_ptr->Strong_Weak_Destroy_Delete), planned, read) == read) { // success!
                        return pa_ptr;
                    }
                }
                else {
                    break;
                }
            }
            return nullptr;
        };
        static __declspec(noinline) void dec_weak(aux* pa_ptr) {
            long long
                read,
                planned;

            while (pa_ptr) {
                read = pa_ptr->Strong_Weak_Destroy_Delete;                
                if (!reinterpret_cast<short*>(&read)[3]) { // if NOT being destroyed...
                    planned = read;
                    --reinterpret_cast<short*>(&planned)[1];
                    if (reinterpret_cast<long*>(&planned)[0] <= 0) {
                        // flag that we plan on deleting the mem_block!
                        reinterpret_cast<short*>(&planned)[3] = 1;
                    }
                    if (InterlockedCompareExchange64(reinterpret_cast<volatile long long*>(&pa_ptr->Strong_Weak_Destroy_Delete), planned, read) == read) { // success!
                        if (reinterpret_cast<short*>(&planned)[3] == 1) {
                           pa_ptr->destroy_aux();
                        }
                        break;
                    }
                }
                else {
                    break;
                }
            }
        };
    };

    template<class T> class weak_ptr; // forward-decl
    /// <summary>
    /// Thread-safe implimentation of std::shared_ptr. Slower in single-thread cases, faster (and race-free) in multi-threaded cases. weak_ptr dereferencing is particularly slow here. 
    /// </summary>
    /// <returns></returns>
    template<class T> class shared_ptr : public shared_ptr_base {
        friend class weak_ptr<T>;
    protected:
        template<class U>
        struct aux_default final : public aux {
            aux_default(U* pu = nullptr) : aux(static_cast<void*>(pu)) {}
            aux_default(aux_default &&) = default;
            aux_default& operator=(aux_default&&) = default;
            aux_default(aux_default const&) = default;
            aux_default& operator=(aux_default const&) = default;
            ~aux_default() = default;

            virtual utilities::type const& type() const override { return utilities::type_of<U>(); };
            virtual void protect_aux() override { (void)shared_ptr<U>::aux_allocator().ProtectCurrentEpoch_Fast(); };
            virtual void destroy_aux() override { 
                (void)shared_ptr<U>::aux_allocator().Free(this);  
                shared_ptr<U>::aux_allocator().RunGC();
                --shared_ptr<U>::aux_allocations();
            };
            virtual void destroy_obj() override { delete static_cast<T*>(this->p); };
        };
        static auto& aux_allocator() {
            static utilities::ABA_Problem::EpochAllocator<aux_default<T>, moodycamel::ConcurrentQueue<std::pair<long long, aux_default<T>*>>> alloc{};            
            return alloc;
        };
        static auto& aux_allocations() {
            static std::atomic<size_t> alloc{ 0 };
            return alloc;
        };


        T* 
            ptr;
        GoodLang::atomic_ptr<shared_ptr_base::aux>
            paux;

        static T* get(shared_ptr const& p) {
            if (p.ptr) return p.ptr;

            T*
                out{ nullptr };
            aux
                * pa_ptr{ nullptr };
            long long
                read;

            if (pa_ptr = p.paux.load()) {
                (void)pa_ptr->protect_aux();
            }
            while (pa_ptr) {
                read = pa_ptr->Strong_Weak_Destroy_Delete;
                if (!reinterpret_cast<short*>(&read)[2] && !reinterpret_cast<short*>(&read)[3]) { // if NOT being destroyed or deleted...
                    out = static_cast<T*>(pa_ptr->ptr());
                    return out;
                }                              
            }
            return out;
        };

    public:
        auto const& GetPaux() const { return paux; };

        static size_t num_allocations() {
            return aux_allocations().load();
        };

        template<class U> explicit shared_ptr(U* pu) : paux(static_cast<aux*>(shared_ptr<U>::aux_allocator().Alloc(pu))), ptr(reinterpret_cast<T*>(pu)) {
            ++aux_allocations();
        };

        shared_ptr() : paux(nullptr), ptr(nullptr) {}
        shared_ptr(std::nullptr_t) : paux(nullptr), ptr(nullptr) {}
        explicit shared_ptr(aux* pa_p, T* pt_p, bool) : paux(pa_p), ptr(pt_p) {}
        template<class U> shared_ptr(shared_ptr<U> const& s) : paux(shared_ptr_base::inc_strong(s.GetPaux())), ptr(nullptr) { // create from a different shared_ptr                        
            if (this->paux) ptr = static_cast<T*>(paux->ptr());
        };
        shared_ptr(shared_ptr<T> const& s) : paux(shared_ptr_base::inc_strong(s.paux)), ptr(nullptr) {
            if (this->paux) ptr = static_cast<T*>(paux->ptr());
        };
        ~shared_ptr() {
            shared_ptr_base::dec_strong(paux.load());
        }

        shared_ptr& operator=(const shared_ptr& s) {
            if (this != &s) {
                InterlockedExchangePointer(reinterpret_cast<void**>(&ptr), nullptr);
                shared_ptr_base::dec_strong(paux.Set(shared_ptr_base::inc_strong(s.paux)));
            }
            return *this;
        };
        shared_ptr& single_threaded_assignment(const shared_ptr& s) {
            if (this != &s) {
                shared_ptr_base::dec_strong(paux.Set(shared_ptr_base::inc_strong(s.paux)));
                InterlockedExchangePointer(reinterpret_cast<void**>(&ptr), paux->ptr());
            }
            return *this;
        };
        shared_ptr& operator=(std::nullptr_t) {
            InterlockedExchangePointer(reinterpret_cast<void**>(&ptr), nullptr);
            shared_ptr_base::dec_strong(paux.Set(nullptr));            
            return *this;
        };

        T* get() const {
            return get(*this);
        };
        operator bool() const {
            return get();
        };
        T* operator->() const {
            return get();
        };
        T& operator*() const {
            return *get();
        };

        friend bool operator==(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() == b.get(); };
        friend bool operator!=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() != b.get(); };
        friend bool operator<(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() < b.get(); };
        friend bool operator<=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() <= b.get(); };
        friend bool operator>(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() > b.get(); };
        friend bool operator>=(const shared_ptr& a, const shared_ptr& b) noexcept { return a.get() >= b.get(); };
        friend bool operator==(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() == nullptr; };
        friend bool operator!=(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() != nullptr; };
        friend bool operator<(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() < nullptr; };
        friend bool operator<=(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() <= nullptr; };
        friend bool operator>(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() > nullptr; };
        friend bool operator>=(const shared_ptr& a, std::nullptr_t) noexcept { return a.get() >= nullptr; };
        friend bool operator==(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr == a.get(); };
        friend bool operator!=(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr != a.get(); };
        friend bool operator<(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr < a.get(); };
        friend bool operator<=(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr <= a.get(); };
        friend bool operator>(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr > a.get(); };
        friend bool operator>=(std::nullptr_t, const shared_ptr& a) noexcept { return nullptr >= a.get(); };
    };

    /// <summary>
    /// Thread-safe implimentation of std::weak_ptr. Slower in single-thread cases, faster (and race-free) in multi-threaded cases. weak_ptr dereferencing is particularly slow here if locks are not needed. 
    /// </summary>
    /// <returns></returns>
    template<class T> class weak_ptr {
        GoodLang::atomic_ptr<shared_ptr_base::aux> pa; // pointer to shared memory block

    public:
        weak_ptr() : pa(nullptr) {}
        weak_ptr(std::nullptr_t) : pa(nullptr) {}
        weak_ptr(shared_ptr<T> const& r) : pa(shared_ptr_base::inc_weak(r.paux)) {};
        weak_ptr(const weak_ptr& r) : pa(shared_ptr_base::inc_weak(r.pa)) {};
        ~weak_ptr() {
            shared_ptr_base::dec_weak(pa.load());
        }

        operator bool() const {
            return !expired();
        };

        weak_ptr& operator=(const weak_ptr& s) {
            if (this != &s) shared_ptr_base::dec_weak(pa.Set(shared_ptr_base::inc_weak(s.pa)));            
            return *this;
        };
        weak_ptr& operator=(std::nullptr_t) {
            shared_ptr_base::dec_weak(pa.Set(nullptr));
            return *this;
        };

        shared_ptr<T> lock() const {
            auto* Pa = shared_ptr_base::inc_strong(pa);
            if (Pa) {
                if (auto* ptr = Pa->ptr()) {
                    return shared_ptr<T>(Pa, static_cast<T*>(ptr), true);
                }
            }
            return shared_ptr<T>();
        };
        bool expired() {
            if (auto* pa_ptr = pa.load()) {
                auto read = pa_ptr->Strong_Weak_Destroy_Delete.load();
                if ((reinterpret_cast<short*>(&read)[0] < std::numeric_limits<short>::max()) && !reinterpret_cast<short*>(&read)[2] && !reinterpret_cast<short*>(&read)[3]) { // if NOT being destroyed or deleted...
                    return false;
                }
                else {
                    return true;
                }
            }
            return true;
        };
    };

};

namespace std {
    template <> struct hash<utilities::type> {
        std::size_t operator()(const utilities::type& k) const {
            return k.get_hash();
        };
    };    
    template <> struct hash<utilities::types> {
        std::size_t operator()(const utilities::types& k) const {
            return k.get_hash();
        };
    };
};

namespace utilities{
    class FunctionWrapper {
    public:
        enum FunctionState {
            Normal = 0, // default -- meaningless. 
            Static = 1, // whether the function is a static function or not. 
            Constant = 2, // whether this function commits to making no changes to the underlying object. Often, const should also be async, but not always. 
            Async = 4, // whether this function can be safely called asynchronously or not
            Template = 8, // whether the function is a template 
            Explicit = 16, // whether the function is explicit and the input params must exactly match (does not allow conversion)
            Cached = 32 // whether the function is a cache from another function, for performance reasons. 
        };

        FunctionWrapper(GoodLang::Proxy_Function obj = nullptr, int s = 0, std::vector<GoodLang::Any> defaults = {})
            : function{ std::move(obj) }
            , state{ std::move(s) }
            , default_values(std::move(defaults))
        {};

        GoodLang::Proxy_Function 
            function{ nullptr };
        int 
            state{ 0 }; // combination(s) of FunctionState(s)
        mutable GoodLang::Units::value
            cost{ std::numeric_limits<double>::max() }; // often associated to a cache
        std::vector<GoodLang::Any>
            default_values; // default "empty" values are considered as not having been provided at all. 

        bool is_const() const {
            return state & Constant;
        };
        bool is_static() const {
            return state & Static;
        };
        bool is_async() const {
            return state & Async;
        };
        bool is_template() const {
            return state & Template;
        };
        bool is_explicit() const {
            return state & Explicit;
        };      
        bool is_cached() const {
            return state & Cached;
        };

        // Given index (lef tto right), will return the default value for it, if available. Otherwise nullptr.
        const GoodLang::Any* get_default(size_t index) const {
            if ((index < default_values.size()) && default_values[index])
                return &default_values[index];            
            return nullptr;
        };

        double determine_cost(std::vector<std::shared_ptr<GoodLang::Type_Info>> const& from_types, GoodLang::TypeConverter& m_typeConverters) const {
            if (function) {
                if (function->GetSignature().Arguments().size() <= from_types.size()) {
                    // perfection
                    return function->conversion_cost_fast(from_types, m_typeConverters);
                }
                else if (default_values.size() >= function->GetSignature().Arguments().size()) {
                    // we can use the defaults
                    std::vector<std::shared_ptr<GoodLang::Type_Info>> temp(function->GetSignature().Arguments().size(), nullptr);
                    int i = 0;
                    double penalty_count = 0;
                    for (; i < temp.size() && i < from_types.size(); i++) temp[i] = from_types[i];
                    for (; i < temp.size() && i < default_values.size(); i++) {
                        if (auto* p = get_default(i)) {
                            ++penalty_count;
                            temp[i] = p->TypePtr();
                        }
                        else {
                            // something went wrong
                            return function->conversion_cost_fast(from_types, m_typeConverters);
                        }
                    }
                    return function->conversion_cost_fast(temp, m_typeConverters) + penalty_count; // * GoodLang::details::TypeConversionWorstCaseCost / 2.0;
                }
                else {
                    // we simply do not have enough parameters. Learn the hard way.
                    return function->conversion_cost_fast(from_types, m_typeConverters);
                }
            }
            else {
                return std::numeric_limits<double>::max();
            }
        };
        GoodLang::Any call(std::vector<GoodLang::Any> const& params, GoodLang::TypeConverter& m_typeConverters) const  {
            if (function) {
                if (function->GetSignature().Arguments().size() <= params.size()) {
                    // perfection
                    return function->operator()(const_cast<std::vector<GoodLang::Any>&>(params), m_typeConverters);
                }
                else if (default_values.size() >= function->GetSignature().Arguments().size()) {
                    // we can use the defaults
                    std::vector<GoodLang::Any> temp(function->GetSignature().Arguments().size(), GoodLang::Any());
                    int i = 0;
                    for (; i < temp.size() && i < params.size(); i++) temp[i] = params[i];
                    for (; i < temp.size() && i < default_values.size(); i++) {
                        if (auto* p = get_default(i)) {
                            temp[i] = *p;
                        }
                        else {
                            // something went wrong
                            return function->operator()(const_cast<std::vector<GoodLang::Any>&>(params), m_typeConverters);
                        }
                    }
                    return function->operator()(temp, m_typeConverters);
                }
                else {
                    // we simply do not have enough parameters. Learn the hard way.
                    return function->operator()(const_cast<std::vector<GoodLang::Any>&>(params), m_typeConverters);
                }
            }
            else {
                throw std::runtime_error("Attempted to call a null-defined FunctionWrapper");
            }
        };

    };

    class Functions {
    public:
        Functions() = default;
        Functions(Functions const& rhs) = delete;
        Functions(Functions&& rhs) = delete;
        Functions& operator=(Functions const& rhs) = delete;
        Functions& operator=(Functions&& rhs) = delete;
        ~Functions() = default;

    public:
        typedef FunctionWrapper
            FunctionPtr;
        typedef atomic_map<GoodLang::ParamTypes, FunctionPtr>
            FunctionSort; // key may NOT be the function's underlying params, but just params that were previously searched... 
        typedef atomic_map<utilities::string, FunctionSort>
            FunctionMap; // name

    public:
        static constexpr size_t numV = ((int)('Z') - (int)('A') + 1) + ((int)('z') - (int)('a') + 1) + 1;
        static constexpr size_t CharToIndex(char firstChar) {
            if (firstChar >= 'a' && firstChar <= 'z') {
                return ((int)firstChar - (int)('a')) + 27;
            }
            else if (firstChar >= 'A' && firstChar <= 'Z') {
                return ((int)firstChar - (int)('A')) + 1;
            }
            else {
                return 0;
            }
        };
        FunctionMap
            m_functions;

    private:
        FunctionPtr const& at(utilities::string const& key, GoodLang::ParamTypes const& params) const {
            static Functions::FunctionPtr out;
            if (auto functionMapPtr = m_functions.find(key), e = m_functions.end(); functionMapPtr != e) {
                if (auto FunctionSortPtr = functionMapPtr->second.find(params), e2 = functionMapPtr->second.end(); FunctionSortPtr != e2) {
                    return FunctionSortPtr->second;
                }
            }            
            return out;
        };

    public:
        FunctionPtr const& operator()(utilities::string const& key, GoodLang::ParamTypes const& params) const {
            return at(key, params);
        };

        FunctionPtr const& emplace(utilities::string const& key, GoodLang::ParamTypes const& params, FunctionWrapper const& func) {
            auto ptr{ m_functions[key].insert(params, (FunctionWrapper)func) };
            return ptr.second;
        };
        FunctionPtr const& emplace(utilities::string const& key, FunctionWrapper const& func) {
            return emplace(key, func.function->Arguments().Types(), func);
        };

        /* Given a function name and call parameters, will attempt to find an exact-match function, variadic instantiation, or convertable function call, or return nullptr. */
        FunctionWrapper const& BuildMatch(utilities::string const& functionName, GoodLang::ParamTypes const& params, GoodLang::TypeConverter& m_typeConverters, bool AllowTemplateInstantiation = true, bool AllowTypeConversion = true, double* finalCost = nullptr) {
            static FunctionWrapper null_func;

            std::vector<std::shared_ptr<GoodLang::Type_Info>> paramTypes;
            paramTypes.reserve(params.size());
            for (auto& x : params) paramTypes.push_back(x.lock());

            if (functionName.length() == 0) return null_func;
#if 1
            if (auto& func = at(functionName, params); func.function) {
                bool isTemplateFunc = func.function->GetSignature().IsTemplate();
                bool isExplicitFunc = func.is_explicit();

                if (isTemplateFunc) {
                    if (AllowTemplateInstantiation) {
                        if (finalCost) {
                            //if (func.cost >= GoodLang::details::TypeConversionWorstCaseCost) {
                            //    func.cost = func.determine_cost(paramTypes, m_typeConverters);
                            //}
                            *finalCost = func.determine_cost(paramTypes, m_typeConverters);// func.cost();
                        }
                        return func.function;
                    }
                }
                else {
                    if (finalCost) {
                        //if (func.cost >= GoodLang::details::TypeConversionWorstCaseCost) {
                        //    func.cost = func.determine_cost(paramTypes, m_typeConverters);
                        //}
                        *finalCost = func.determine_cost(paramTypes, m_typeConverters);// func.cost();
                    }
                    return func;
                }                
            }
#endif
            if (1) {
                // Three sorted groups of candidates. 
                // Group 1 = exact matches, Group 2 = type conversions, Group 3 = template functions
                thread_local static std::map< size_t, std::array<std::pair<double, FunctionPtr*>, 3>, std::greater<size_t>>
                    candidates;
                defer(candidates.clear());

                // Create candidates.
                {               
                    if (functionName.size() > 0) {
                        for (auto& function : m_functions[functionName]) {
                            if (!function.second.function) continue; // not valid
                            if (function.second.is_cached()) continue; // ignoring pre-cached functions. Only interested in "true" functions. 

                            bool isTemplateFunc = function.second.function->GetSignature().IsTemplate();
                            bool isExplicitFunc = function.second.is_explicit();

                            auto conversionCost = function.second.determine_cost(paramTypes, m_typeConverters);

                            // try to early exit...
                            if (params.size() == function.second.function->Arguments().size()) {
                                if (!isTemplateFunc) {
                                    if (conversionCost == 0) {
                                        if (function.second.function->Arguments().Types().hash() == params.hash()) {
                                            if (finalCost) *finalCost = 0;
                                            return function.second;
                                        }
                                        else {
                                            FunctionWrapper FunctionToCache(function.second.function, function.second.state | FunctionWrapper::FunctionState::Cached, function.second.default_values);
                                            FunctionToCache.cost = 0;
                                            if (auto& func = this->emplace(functionName, params, FunctionToCache); func.function) {
                                                if (finalCost) *finalCost = 0;
                                                return func;
                                            }
                                        }
                                    }
                                }
                            }

                            if (isTemplateFunc) {
                                if (AllowTemplateInstantiation) {
                                    auto& pair = candidates[function.second.function->NumArguments()][2];
                                    if (pair.second) {
                                        if (pair.first > conversionCost) {
                                            pair.first = conversionCost;
                                            pair.second = &function.second;
                                        }
                                        else if (pair.first == conversionCost) {
                                            // This indicates that one of these functions is "unclear" to be better or worse for this set of parameters...
                                            // C++ would have thrown an error due to the ambiguity, to encourage the user to write more clear code.

                                            // To-Do, improve the error code to be more explicit, or try to find another way to distinguish functions? 
                                            auto& incomingFunctionArgs = function.second.function->GetSignature().Arguments();
                                            auto& existingFunctionArgs = pair.second->function->GetSignature().Arguments();
                                            if (existingFunctionArgs.size() > incomingFunctionArgs.size()) {
                                                pair.first = conversionCost;
                                                pair.second = &function.second;
                                                continue;
                                            }

                                            if (existingFunctionArgs.IsTemplate() && !incomingFunctionArgs.IsTemplate()) {
                                                pair.first = conversionCost;
                                                pair.second = &function.second;
                                                continue;
                                            }

                                            if ((incomingFunctionArgs.size() > 0) && (paramTypes.size() > 0) && (existingFunctionArgs.size() > 0)) {
                                                if (auto p = incomingFunctionArgs.Type(0).lock()) {
                                                    if (auto p2 = existingFunctionArgs.Type(0).lock()) {
                                                        if (p->underlyingHash == paramTypes[0]->underlyingHash) {
                                                            if (p2->underlyingHash != paramTypes[0]->underlyingHash) {
                                                                pair.first = conversionCost;
                                                                pair.second = &function.second;
                                                                continue;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        pair.first = conversionCost;
                                        pair.second = &function.second;
                                    }
                                }
                            }
                            else {
                                if (conversionCost == 0) {
                                    auto& pair = candidates[function.second.function->NumArguments()][0];
                                    if (pair.second) {
                                        if (pair.first > conversionCost) {
                                            pair.first = conversionCost;
                                            pair.second = &function.second;
                                        }
                                        else if (pair.first == conversionCost) {
                                            // This indicates that one of these functions is "unclear" to be better or worse for this set of parameters...
                                            // C++ would have thrown an error due to the ambiguity, to encourage the user to write more clear code.
                                            
                                            // To-Do, improve the error code to be more explicit, or try to find another way to distinguish functions? 
                                            auto& incomingFunctionArgs = function.second.function->GetSignature().Arguments();
                                            auto& existingFunctionArgs = pair.second->function->GetSignature().Arguments();
                                            if (existingFunctionArgs.size() > incomingFunctionArgs.size()) {
                                                pair.first = conversionCost;
                                                pair.second = &function.second;
                                                continue;
                                            }

                                            if (existingFunctionArgs.IsTemplate() && !incomingFunctionArgs.IsTemplate()) {
                                                pair.first = conversionCost;
                                                pair.second = &function.second;
                                                continue;
                                            }

                                            if ((incomingFunctionArgs.size() > 0) && (paramTypes.size() > 0) && (existingFunctionArgs.size() > 0)) {
                                                if (auto p = incomingFunctionArgs.Type(0).lock()) {
                                                    if (auto p2 = existingFunctionArgs.Type(0).lock()) {
                                                        if (p->underlyingHash == paramTypes[0]->underlyingHash) {
                                                            if (p2->underlyingHash != paramTypes[0]->underlyingHash) {
                                                                pair.first = conversionCost;
                                                                pair.second = &function.second;
                                                                continue;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else {
                                        pair.first = conversionCost;
                                        pair.second = &function.second;
                                    }
                                }
                                else if (AllowTypeConversion && !isExplicitFunc) {
                                    auto& pair = candidates[function.second.function->NumArguments()][1];
                                    if (pair.second) {
                                        if (pair.first > conversionCost) {
                                            pair.first = conversionCost;
                                            pair.second = &function.second;
                                        }
                                        else if (pair.first == conversionCost) {
                                            // This indicates that one of these functions is "unclear" to be better or worse for this set of parameters...
                                            // C++ would have thrown an error due to the ambiguity, to encourage the user to write more clear code.

                                            // To-Do, improve the error code to be more explicit, or try to find another way to distinguish functions? 
                                            auto& incomingFunctionArgs = function.second.function->GetSignature().Arguments();
                                            auto& existingFunctionArgs = pair.second->function->GetSignature().Arguments();
                                            if (existingFunctionArgs.size() > incomingFunctionArgs.size()) {
                                                pair.first = conversionCost;
                                                pair.second = &function.second;
                                                continue;
                                            }

                                            if (existingFunctionArgs.IsTemplate() && !incomingFunctionArgs.IsTemplate()) {
                                                pair.first = conversionCost;
                                                pair.second = &function.second;
                                                continue;
                                            }

                                            if ((incomingFunctionArgs.size() > 0) && (paramTypes.size() > 0) && (existingFunctionArgs.size() > 0)) {
                                                if (auto p = incomingFunctionArgs.Type(0).lock()) {
                                                    if (auto p2 = existingFunctionArgs.Type(0).lock()) {
                                                        if (p->underlyingHash == paramTypes[0]->underlyingHash) {
                                                            if (p2->underlyingHash != paramTypes[0]->underlyingHash) {
                                                                pair.first = conversionCost;
                                                                pair.second = &function.second;
                                                                continue;
                                                            }
                                                        }
                                                    }
                                                }
                                            }  
                                        }
                                    }
                                    else {
                                        pair.first = conversionCost;
                                        pair.second = &function.second;
                                    }
                                }
                            }
                        }
                    }
                }

                // Get the "cheapest" or fastest conversion option available at this scope, with the largest number of arguments, in order of group (e.g. preference).
                for (size_t num_param = params.size(); num_param < 16; ++num_param) {
                    if (auto f = candidates.find(num_param); f != candidates.end()) {
                        for (auto& candidate : f->second) {
                            if (candidate.first >= std::numeric_limits<double>::max()) continue;
                            // if (candidate.first >= GoodLang::details::TypeConversionWorstCaseCost) continue;
                            if (!candidate.second) continue;

                            if (candidate.second->function->Arguments().Types().hash() == params.hash()) {
                                if (finalCost) *finalCost = candidate.first;
                                return *candidate.second;
                            }
                            else {
                                FunctionWrapper FunctionToCache(candidate.second->function, candidate.second->state | FunctionWrapper::FunctionState::Cached, candidate.second->default_values);
                                FunctionToCache.cost = candidate.first;
                                if (auto& func = this->emplace(functionName, params, FunctionToCache); func.function) {
                                    if (finalCost) *finalCost = candidate.first;
                                    return func;
                                }
                            }


                        }
                    }                    
                }
                for (long long num_param = (long long)(params.size()) - 1; num_param >= 0; --num_param) {
                    if (auto f = candidates.find(num_param); f != candidates.end()) {
                        for (auto& candidate : f->second) {
                            if (candidate.first >= std::numeric_limits<double>::max()) continue;
                            // if (candidate.first >= GoodLang::details::TypeConversionWorstCaseCost) continue;
                            if (!candidate.second) continue;

                            if (candidate.second->function->Arguments().Types().hash() == params.hash()) {
                                if (finalCost) *finalCost = candidate.first;
                                return *candidate.second;
                            }
                            else {
                                FunctionWrapper FunctionToCache(candidate.second->function, candidate.second->state | FunctionWrapper::FunctionState::Cached, candidate.second->default_values);
                                FunctionToCache.cost = candidate.first;
                                if (auto& func = this->emplace(functionName, params, FunctionToCache); func.function) {
                                    if (finalCost) *finalCost = candidate.first;
                                    return func;
                                }
                            }
                        }
                    }
                }
            }
            return null_func;        
        };
        GoodLang::Any Call(utilities::string const& functionName, std::vector<GoodLang::Any> const& params, GoodLang::TypeConverter& m_typeConverters) {
            if (auto const& f = BuildMatch(functionName, GoodLang::ParamTypes(params), m_typeConverters); f.function) {
                return f.call(params, m_typeConverters);
            }
            else {
                std::string params_str;
                for (auto& p : params) {
                    std::string className = p.TypeName(); {
                        //if (auto classPtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(p.Type()))) {
                        //	className = classPtr->GetName();
                        //}
                    }

                    if (params_str.empty()) {
                        params_str += className;
                    }
                    else {
                        params_str += ", ";
                        params_str += className;
                    }
                }
                throw GoodLang::exception::not_found_error(GoodLang::printf("`%s`(%s)", functionName.c_str(), params_str.c_str()));
            }


        };
        GoodLang::Any Call(FunctionWrapper const& f, std::vector<GoodLang::Any> const& params, GoodLang::TypeConverter& m_typeConverters) {
            return f.call(params, m_typeConverters);
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
        utilities::string
            current_namespace; // e.g. "::" or "::UI::Color::"
        int
            scope_type; // may be a compound of multiple types, e.g. a root is also a namespace

    public:
        ScopeID(utilities::string && scope_name_p = {}, int scope_type_p = ScopeType::Basic)
            : scope_name{ std::move(scope_name_p) }
            , scope{ nullptr }            
            , current_namespace{ utilities::string::empty_string() }
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
                        else {
                            if (this_m.is_namespace()) {
                                // Since basic_scopes can be created and deleted without much notice,
                                // we limit the caching to namespaces to help guarrantee that looping over the list
                                // will likely be protected from the lifetime perspective.                                 
                                root_ptr->scopes.grow_to_at_least(new_index + 1);
                                root_ptr->scopes[new_index] = this;
                            }
                        }
                    }
                }
            }
            return scope_index._index;
        };
        utilities::string const& GetCurrentNamespace() const {
            if (this->this_m.is_namespace()) {
                return this->this_m.current_namespace;
            }
            else {
                return this->namespace_m->this_m.current_namespace;
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
            if (parent_m) {
                if (this_m.scope_name.length() > 0) {
                    std::string temp_string = parent_m->this_m.current_namespace.to_string() + this_m.scope_name.to_string();
                    auto cleaned_temp_string = CleanUpScopeName(std::string_view(temp_string));
                    this_m.current_namespace = cleaned_temp_string;
                }
            }
            else {
                this_m.current_namespace = utilities::string::namespace_colons();
            }
        };
        Breadcrumb(Breadcrumb const&) = delete;
        Breadcrumb(Breadcrumb &&) = delete;
        Breadcrumb& operator=(Breadcrumb const&) = delete;
        Breadcrumb& operator=(Breadcrumb&&) = delete;
        ~Breadcrumb() {
            if (this_m.is_namespace() && (scope_index._index > 0) && root_m) if (auto* root_ptr = dynamic_cast<RootScope*>(root_m->this_m.scope))
                root_ptr->scopes[scope_index._index] = nullptr;
        }
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
                    if (cache[category].valid()) {
                        if (auto f = cache[category]->find(input_hash); f != cache[category]->end()) {
                            out = f->second;
                        }
                    }

                    // out = cache[category]->operator[](input_hash);
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
        concurrency::concurrent_unordered_map<Breadcrumb*, utilities::Callback<NamespaceScope>::ScopedListener>
            using_m; // NOTE: calling "using" should split a normal, BasicScope - e.g. using statements are appended staticly at compile time, NOT at runtime. 
        utilities::atomic_map<utilities::string, utilities::ObjectWrapper>
            objects_m; // NOTE: adding objects should be appended staticly at compile time, NOT at runtime. E.g. the names are known, even if the types are not yet known. 

        virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) {};
        template <bool overwriteIfExists> bool EmplaceObject_Impl(utilities::string const& sv, utilities::ObjectWrapper && Obj) {            
            if constexpr (overwriteIfExists)
                objects_m.emplace_fast(sv, std::move(Obj));
            else
                objects_m.insert_fast(sv, std::move(Obj));
            return true;
        };
        utilities::ObjectWrapper* GetObject_Impl(utilities::string const& sv) {
            if (auto f = objects_m.find(sv), e = objects_m.end(); f != e) 
                return &f->second;            
            return nullptr;
        };
        virtual void AddUsing_Impl(Breadcrumb* scope) {
            if (scope) {
                if (scope->this_m.is_namespace()) {
                    //if (auto f = using_m->find(scope); f == using_m->end()) {
                    using_m.insert(std::pair<Breadcrumb*, utilities::Callback<NamespaceScope>::ScopedListener>{ scope, utilities::Callback<NamespaceScope>::ScopedListener() });
                        invalidate_cache(); // does nothing for normal scopes
                    //}
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
        // Returns true if this scope is a namespace scope
        bool is_namespace() const {
            return this->breadcrumb_m.this_m.is_namespace();
        };
        // Returns true if this scope is a class scope
        bool is_class() const {
            return this->breadcrumb_m.this_m.is_class();
        };
        // Returns true if this scope is a root scope
        bool is_root() const {
            return this->breadcrumb_m.this_m.is_root();
        };
        // Get the immediate parent, if one exists.
        BasicScope* GetParent() const {
            if (this->breadcrumb_m.parent_m) {
                return this->breadcrumb_m.parent_m->this_m.scope;
            }
            else {
                return nullptr;
            }
        };
        // Get the current namespace (for inserting functions, etc)
        NamespaceScope* GetNamespace() const {
            if (this->breadcrumb_m.namespace_m) {
                return static_cast<NamespaceScope*>(this->breadcrumb_m.namespace_m->this_m.scope);
            }
            else {
                return nullptr;
            }
        };
        // Get the root of the entire scope tree
        RootScope* GetRoot() const {
            if (this->breadcrumb_m.root_m) {
                return static_cast<RootScope*>(this->breadcrumb_m.root_m->this_m.scope);
            }
            else {
                return nullptr;
            }
        };
       
    protected:
        enum CheckFlagState {
            none = 0,
            self = 1,
            all = 2
        };
        enum SearchState {
            SearchingParents = 1,
            SearchingUsings = 2,
            SearchingChildren = 4,
            SearchUpHitNamespace = 8,
            SkipChildren = 16,
            SkipParent = 32
        };
        enum SearchResult {
            Failure = 1,
            Success = 2,
            StaticFailure = 4
        };
        using check_cache = std::vector<short>;
        static check_cache& GetCheckMap() {
            thread_local check_cache out;
            out.clear();
            return out;
        };
        virtual Breadcrumb* FindNearestScopeWhere(
            std::function<int(Breadcrumb*, int)> const& func,
            Breadcrumb* SecondaryPriortyScope = nullptr,
            int searchState = 0,
            check_cache& check_flags = GetCheckMap(),
            int depth = 0
        ) const {
            auto& selfPtr = const_cast<Breadcrumb&>(this->breadcrumb_m);
            Breadcrumb* finalResult = nullptr;
            if (depth == 0) {
                if (auto numTickets = GetRoot()->scope_indexs.num_tickets(); check_flags.size() < numTickets) {
                    check_flags.resize(numTickets);
                }
                for (auto& x : check_flags) x = CheckFlagState::none;
            }

            // Prevent Duplication
            if (check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::all) {
                finalResult = nullptr;
                return finalResult;
            }
            if (searchState & SkipChildren) {
                check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::all;
            }

            // test myself directly	
            if (!(check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::self)) {
                check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::self;

                auto res = func(&selfPtr, searchState);
                if (res & SearchResult::Success) {
                    finalResult = &selfPtr;               
                    return finalResult;
                }
                else if (res & SearchResult::StaticFailure) {
                    finalResult = nullptr;
                    return finalResult;
                }
            }

            // test my personal "using" namespaces completely
            if (using_m.size() > 0ull){
                for (auto& childNamespace : using_m) {
                    if (check_flags[childNamespace.first->GetScopeIndex()] & CheckFlagState::all) { continue; }
                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                        return finalResult;
                    }  
                }
            }

            // test all of my parents directly -- hoping to quickly find "it"
            if (!(searchState & SkipParent)) {
                Breadcrumb* thisParent = &selfPtr;
                while (thisParent = thisParent->parent_m) {
                    auto& flag = check_flags[thisParent->GetScopeIndex()];
                    if (flag & CheckFlagState::self) break;
                    else {
                        flag |= CheckFlagState::self;
                    }
                    if (thisParent->this_m.is_namespace()) {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    else {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    // check the using statements of the parent.
                    if (thisParent->this_m.scope->using_m.size() > 0) {
                        for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                            auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                            if (flag2 & CheckFlagState::all) continue;
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                    }
                }
            }

            // test the SecondaryPriortyScope, often the class of the first param provided in a function call
            if ((depth == 0) && SecondaryPriortyScope) {
                Breadcrumb* thisParent = SecondaryPriortyScope;
                if (thisParent) {
                    auto& flag1 = check_flags[thisParent->GetScopeIndex()];
                    flag1 = CheckFlagState::none;

                    // test myself directly
                    if (!(flag1 & CheckFlagState::self)) {
                        flag1 |= CheckFlagState::self;

                        auto res = func(thisParent, searchState);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            finalResult = nullptr;
                            return finalResult;
                        }
                    }

                    // test my personal "using" namespaces completely
                    if (thisParent->this_m.scope->using_m.size() > 0) {
                        for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                            auto& flag = check_flags[childNamespace.first->GetScopeIndex()];
                            if (flag & CheckFlagState::all) { continue; }
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
                            }     
                        }
                    }
                }
                while (thisParent = thisParent->parent_m) {
                    auto& flag = check_flags[thisParent->GetScopeIndex()];
                    if (flag & CheckFlagState::self) break;
                    else {
                        flag |= CheckFlagState::self;
                    }
                    if (thisParent->this_m.is_namespace()) {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    else {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    // check the using statements of the parent.
                    if (thisParent->this_m.scope->using_m.size() > 0) {
                        for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                            auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                            if (flag2 & CheckFlagState::all) continue;
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
                            }   
                        }
                    }
                }
            }

            // Test my parent completely.
            if (!(searchState & SkipParent)) {
                if (selfPtr.parent_m) {
                    auto& flag = check_flags[selfPtr.parent_m->GetScopeIndex()];
                    if (!(flag & CheckFlagState::all)) {
                        if (selfPtr.parent_m->this_m.is_namespace()) {
                            if (finalResult = selfPtr.parent_m->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingParents | SearchUpHitNamespace, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                        else {
                            if (finalResult = selfPtr.parent_m->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingParents, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                        
                    }
                }
            }
            return finalResult;
        };

        static __declspec(noinline) Scopes::Breadcrumb* FindNamespace(utilities::compound_shared_string const& Name, Scopes::Breadcrumb* start) {
            if (!start) return nullptr;
            if (Name.empty()) return start->root_m;

            static thread_local size_t len;
            len = Name.length();
            static thread_local std::set< size_t> target_hash; {
                target_hash.clear();
                target_hash.insert(Name.hash());
                auto temp = Name.remove_leading(':');
                auto* BC = start;
                while (BC) {
                    target_hash.insert(temp.hash(BC->GetCurrentNamespace().hash()));
                    BC = BC->parent_m;
                }
            }

            if (target_hash.count(utilities::string::namespace_colons().hash()) > 0) {
                return start->root_m;
            }
            else if (Breadcrumb* BC = start->this_m.scope->FindNearestScopeWhere([stringified = utilities::string(Name)](Breadcrumb* namespacePtr, int search_state)-> int {
                if (!namespacePtr->this_m.is_namespace()) return SearchResult::Failure;
                utilities::string const& currNS{ namespacePtr->GetCurrentNamespace() };
                if (target_hash.count(currNS.hash()) > 0) return SearchResult::Success;
                else if (search_state & SearchingChildren) {
                    if (len < currNS.length()) return SearchResult::Failure | SearchResult::StaticFailure;
                    else if (stringified.find(currNS/*, true, stringified.length() - currNS.length()*/) == utilities::string::npos) return SearchResult::Failure | SearchResult::StaticFailure;
                    else return SearchResult::Failure;
                }
                else return SearchResult::Failure;
            })) {
                return BC;
            }
            else {
                return nullptr;
            }
        };

    public:
        BasicScope() = delete;
        virtual ~BasicScope() = default;

        /// <summary>
        /// Get the index that is unique to this scope, which will remain unique for the life of the scope. May be re-used after the scope ends. 
        /// </summary>
        /// <returns>size_t</returns>
        size_t get_unique_index() const {
            return const_cast<BasicScope*>(this)->breadcrumb_m.GetScopeIndex();
        };

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
                if (this == p) return; // may not "use" yourself.
                this->AddUsing_Impl(const_cast<Breadcrumb*>(&p->breadcrumb_m));
            }
        }
        
        /// <summary>
        /// Insert an object into this scope only if it does not yet exist. Does not search neighbors or review the object name.
        /// </summary>
        /// <param name="sv"></param>
        /// <param name="Obj"></param>
        /// <returns>bool</returns>
        bool insert_object_here(utilities::string const& sv, utilities::ObjectWrapper && Obj) {
            return this->EmplaceObject_Impl<false>(sv, std::move(Obj));
        };
        
        /// <summary>
        /// Emplace an object into this scope whether or not it exists. Does not search neighbors or review the object name.
        /// </summary>
        /// <param name="sv"></param>
        /// <param name="Obj"></param>
        /// <returns>bool</returns>
        bool emplace_object_here(utilities::string const& sv, utilities::ObjectWrapper && Obj) {
            return this->EmplaceObject_Impl<true>(sv, std::move(Obj));
        };
        
        /// <summary>
        /// Try to find an object in this scope. Does not search neighbors or review the object name. Since objects cannot be removed, it safely returns a pointer. 
        /// </summary>
        /// <returns>ObjectWrapper</returns>
        utilities::ObjectWrapper* find_object_here(utilities::string const& sv) const {
            return const_cast<BasicScope*>(this)->GetObject_Impl(sv);
        };

        // Searches for a namespace that best fits the provided information, starting from this scope's namespace or position.
        Scopes::Breadcrumb* find_namespace(utilities::string const& Name) const {
            auto* NS = this->GetNamespace();
            if (auto* cache = NS->search_cache.TryGetCache<0>(NS->cache_version, Name.hash())) {
                return cache;
            }
            
            if (auto* out = FindNamespace(
                utilities::compound_shared_string("::", Name.remove_leading_and_trailing(':'), "::"), // "std" or "::std" or "::std::" -> "::std::" 
                &const_cast<BasicScope*>(this)->breadcrumb_m // where
            )) {
                NS->search_cache.EmplaceCache<0>(NS->cache_version, Name.hash(), out);
                return out;                
            }
            else {
                return nullptr;
            }
        };
    private:
        Scopes::Breadcrumb* FindNamespaceImpl(utilities::string const& Name, Scopes::Breadcrumb*& nearest_scope) const {
            if (auto* out = find_namespace(Name)) {
                nearest_scope = out;
                return out;
            }
            else {
                if (!nearest_scope) nearest_scope = this->breadcrumb_m.root_m;
                const auto& [left, right] = Name.remove_leading_and_trailing(':').left_and_right_of_last("::");
                if (right.length() > 0) {                    
                    if (auto* out = FindNamespaceImpl(left, nearest_scope)) {
                        nearest_scope = out;
                        return out->this_m.scope->FindNamespaceImpl(right, nearest_scope);
                    }
                    else {
                        return nullptr;
                    }                    
                }
                else {
                    // no colons inside
                    return nullptr;
                }
            }
        };
    public:
        // Searches for a namespace while also specifying the "closest" it was able to get to the requested namespace. Useful for debugging where the search last ended. 
        Scopes::Breadcrumb* find_namespace(utilities::string const& Name, Scopes::Breadcrumb*& nearest_scope) const {
            Scopes::Breadcrumb* out = FindNamespaceImpl(Name, nearest_scope);
            if (out) {
                return out->this_m.scope->find_namespace(Name);
            }
            return nullptr;
        };

    public:
        // User is allowed to request a scoped object, e.g. "x" or "::x" or "::std::string::npos"
        utilities::ObjectWrapper* find_object(utilities::string const& PossiblyScopedName, Scopes::Breadcrumb* search_from = nullptr) const {
            utilities::ObjectWrapper* p{ nullptr }; 
            if (search_from) {
                // we have a scope with a specific object name
                // PossiblyScopedName should NOT have colons in this case. 
                if (Breadcrumb* BC = search_from->this_m.scope->FindNearestScopeWhere([&](Breadcrumb* namespacePtr, int search_state)-> int {
                    // we should not be able to find objects in children scopes
                    // search_state & Scopes::BasicScope::
                    if (SearchState::SearchingChildren & search_state) {
                        return SearchResult::Failure | SearchResult::StaticFailure;
                    }

                    if (p = namespacePtr->this_m.scope->find_object_here(PossiblyScopedName)) {
                        return SearchResult::Success;
                    }
                    else {
                        return SearchResult::Failure;
                    }                    
                }, nullptr, SearchState::SkipChildren)) {
                    return p;
                }
                else {
                    return nullptr;
                }
            }
            else {
                auto* NS = this->GetNamespace();
                if (auto* cache = NS->search_cache.TryGetCache<1>(NS->cache_version, PossiblyScopedName.hash())) {
                    p = reinterpret_cast<utilities::ObjectWrapper*>(cache);
                    return p;
                }

                // we don't have a scope (yet)
                const auto& [optionalScope, optionalName] = PossiblyScopedName.left_and_right_of_last("::");
                if (optionalName.length() == 0) {
                    // We only have an object name -- just do the normal search from here.
                    p = find_object(optionalScope, &const_cast<BasicScope*>(this)->breadcrumb_m);
                }
                else {
                    Scopes::Breadcrumb* closest_scope{ nullptr };
                    if (optionalScope.length() == 0) {
                        p = find_object(optionalName, this->breadcrumb_m.root_m);
                    }
                    else if (auto nameSpace = find_namespace(optionalScope, closest_scope)) {
                        p = find_object(optionalName, nameSpace);
                    }
                    else if (closest_scope) { // namespace was not found                        
                        p = nullptr; //  throw GoodLang::exception::not_found_error(GoodLang::printf("Could not located object '%s' in namespace '%s'", optionalName.c_str().data(), closest_scope->GetCurrentNamespace().c_str().data()));
                    }
                    else { // namespace was not found AND no nearest was discovered     
                        p = nullptr; // throw GoodLang::exception::not_found_error(GoodLang::printf("Could not located object '%s'", optionalName.c_str().data()));
                    }
                }
                if (p) NS->search_cache.EmplaceCache<1>(NS->cache_version, PossiblyScopedName.hash(), reinterpret_cast<Scopes::Breadcrumb*>(p));
                return p;
            }
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
        concurrency::concurrent_unordered_map<size_t, std::shared_ptr<NamespaceScope>>
            children; // children cannot be removed at runtime, so using the concurrent_unordered_map is the higher-performance option. 

    protected:
        Scopes::Cache<4> 
            search_cache; // while thread-safe, it does seem to singificantly decrease the performance of creating new BasicScope's, hence moving it here. 

    protected:
        utilities::Callback<NamespaceScope>
            sockets_for_cache_versions; // socket(s) for others to connect to for listening to changes to THIS scope. Thread-safe. 
        utilities::Callback<NamespaceScope>::ScopedListener
            connection_for_cache_version; // socket connection for this scope to its parent, to listen to changes to THEIR scope. Not thread-safe.
        size_t
            cache_version; // the current cache version of this scope. Thread-safe to read. Will be updated during every call to "invalidate_cache()"
    public:
        virtual void invalidate_cache(long* parent_alive = nullptr, size_t call_number = 0) override {
            InterlockedIncrement(static_cast<volatile size_t*>(&cache_version));
            sockets_for_cache_versions.speak(parent_alive, call_number);
        };

    protected:
        virtual void AddUsing_Impl(Breadcrumb* scope) override {
            if (scope) {
                if (scope->this_m.is_namespace()) {
                    if (auto* p = dynamic_cast<NamespaceScope*>(scope->this_m.scope)) {
                        using_m.insert(std::pair<Breadcrumb*, utilities::Callback<NamespaceScope>::ScopedListener>{ scope, p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this) });
                        invalidate_cache();
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
            , cache_version{ 0 }
        {
            if (this->breadcrumb_m.parent_m) {
                if (auto* p = dynamic_cast<NamespaceScope*>(this->breadcrumb_m.parent_m->namespace_m->this_m.scope)) {
                    connection_for_cache_version = p->sockets_for_cache_versions.listener(this->breadcrumb_m.GetScopeIndex(), this);
                }
            }
        };
        // unloads the connections to other namespaces before deletion, which can prevent a memory-access crash. 
        void unload() {
            this->connection_for_cache_version = {};
            for (auto& x : this->using_m) x.second = {};
            for (auto& child : this->children) child.second->unload();            
            this->children.clear(); 
        };

    protected:
        virtual Breadcrumb* FindNearestScopeWhere(
            std::function<int(Breadcrumb*, int)> const& func,
            Breadcrumb* SecondaryPriortyScope = nullptr,
            int searchState = 0,
            check_cache& check_flags = GetCheckMap(),
            int depth = 0
        ) const override {
            auto& selfPtr = const_cast<Breadcrumb&>(this->breadcrumb_m);
            Breadcrumb* finalResult = nullptr;
            if (depth == 0) {
                if (auto numTickets = GetRoot()->scope_indexs.num_tickets(); check_flags.size() < numTickets) {
                    check_flags.resize(numTickets);
                }
                for (auto& x : check_flags) x = CheckFlagState::none;
            }

            // Prevent Duplication
            if (check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::all) {
                finalResult = nullptr;
                return finalResult;
            }
            if (searchState & SkipChildren) {
                check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::all;
            }

            // test myself directly	
            if (!(check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::self)) {
                check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::self;

                auto res = func(&selfPtr, searchState);
                if (res & SearchResult::Success) {
                    finalResult = &selfPtr;
                    return finalResult;
                }
                else if (res & SearchResult::StaticFailure) {
                    finalResult = nullptr;
                    return finalResult;
                }
            }

            bool RequestedSkipChildren = check_flags[selfPtr.GetScopeIndex()] & CheckFlagState::all;
            if (!(searchState & SkipChildren)) {
                check_flags[selfPtr.GetScopeIndex()] |= CheckFlagState::all;
            }

            // test my personal "using" namespaces completely
            if (using_m.size() > 0ull) {
                for (auto& childNamespace : using_m) {
                    if (check_flags[childNamespace.first->GetScopeIndex()] & CheckFlagState::all) { continue; }
                    if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                        return finalResult;
                    }
                }
            }

            // test all of my parents directly -- hoping to quickly find "it"
            if (!(searchState & SkipParent)) {
                Breadcrumb* thisParent = &selfPtr;
                while (thisParent = thisParent->parent_m) {
                    auto& flag = check_flags[thisParent->GetScopeIndex()];
                    if (flag & CheckFlagState::self) break;
                    else {
                        flag |= CheckFlagState::self;
                    }
                    if (thisParent->this_m.is_namespace()) {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    else {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    // check the using statements of the parent.
                    if (thisParent->this_m.scope->using_m.size() > 0) {
                        for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                            auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                            if (flag2 & CheckFlagState::all) continue;
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                    }
                }
            }

            // test the SecondaryPriortyScope, often the class of the first param provided in a function call
            if ((depth == 0) && SecondaryPriortyScope) {
                Breadcrumb* thisParent = SecondaryPriortyScope;
                if (thisParent) {
                    auto& flag1 = check_flags[thisParent->GetScopeIndex()];
                    flag1 = CheckFlagState::none;

                    // test myself directly
                    if (!(flag1 & CheckFlagState::self)) {
                        flag1 |= CheckFlagState::self;

                        auto res = func(thisParent, searchState);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            finalResult = nullptr;
                            return finalResult;
                        }
                    }

                    // test my personal "using" namespaces completely
                    if (thisParent->this_m.scope->using_m.size() > 0) {
                        for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                            auto& flag = check_flags[childNamespace.first->GetScopeIndex()];
                            if (flag & CheckFlagState::all) { continue; }
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                    }
                }
                while (thisParent = thisParent->parent_m) {
                    auto& flag = check_flags[thisParent->GetScopeIndex()];
                    if (flag & CheckFlagState::self) break;
                    else {
                        flag |= CheckFlagState::self;
                    }
                    if (thisParent->this_m.is_namespace()) {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren | SearchUpHitNamespace);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    else {
                        auto res = func(thisParent, searchState | SearchingParents | SkipChildren);
                        if (res & SearchResult::Success) {
                            finalResult = thisParent;
                            return finalResult;
                        }
                        else if (res & SearchResult::StaticFailure) {
                            flag |= CheckFlagState::all;
                        }
                    }
                    // check the using statements of the parent.
                    if (thisParent->this_m.scope->using_m.size() > 0) {
                        for (auto& childNamespace : thisParent->this_m.scope->using_m) {
                            auto& flag2 = check_flags[childNamespace.first->GetScopeIndex()];
                            if (flag2 & CheckFlagState::all) continue;
                            if (finalResult = childNamespace.first->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingUsings, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                    }
                }
            }

            // Test my children themselves. 
            if (!RequestedSkipChildren && (!(searchState & SkipChildren)) && this->children.size() > 0ull) {
                for (auto& child : this->children) {
                    auto* child_bc = &child.second->breadcrumb_m;
                    auto& flag = check_flags[child_bc->GetScopeIndex()];

                    if (flag & CheckFlagState::self) continue;
                    
                    flag |= CheckFlagState::self;

                    auto res = func(child_bc, searchState | SearchingChildren | SkipChildren | SkipParent);
                    if (res & SearchResult::Success) {
                        finalResult = child_bc;
                        return finalResult;
                    }
                    else if (res & SearchResult::StaticFailure) {
                        flag |= CheckFlagState::all;
                    }
                }
            }

            // Test my parent completely.
            if (!(searchState & SkipParent)) {
                if (selfPtr.parent_m) {
                    auto& flag = check_flags[selfPtr.parent_m->GetScopeIndex()];
                    if (!(flag & CheckFlagState::all)) {
                        if (selfPtr.parent_m->this_m.is_namespace()) {
                            if (finalResult = selfPtr.parent_m->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingParents | SearchUpHitNamespace, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }
                        else {
                            if (finalResult = selfPtr.parent_m->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingParents, check_flags, depth + 1)) {
                                return finalResult;
                            }
                        }

                    }
                }
            }

            // Test my children completely. 
            if (!RequestedSkipChildren && (!(searchState & SkipChildren)) && this->children.size() > 0ull) {
                for (auto& child : this->children) {
                    auto* child_bc = &child.second->breadcrumb_m;
                    auto& flag = check_flags[child_bc->GetScopeIndex()];

                    if (flag & CheckFlagState::all) continue;

                    if (finalResult = child_bc->this_m.scope->FindNearestScopeWhere(func, SecondaryPriortyScope, searchState | SearchingChildren | SkipParent, check_flags, depth + 1)) {
                        return finalResult;
                    }
                }
            }

            return finalResult;
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
            if (auto f = children.find(name.hash()); f != children.end()) {
                return *f->second;
            }
            else {
                return *children.insert(
                    { name.hash(), std::shared_ptr<NamespaceScope>(new NamespaceScope((utilities::string)name, Scopes::ScopeType::Basic | Scopes::ScopeType::Namespace, const_cast<Breadcrumb*>(&this->breadcrumb_m))) }
                ).first->second;
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
        // This "ticket" or unique index will be unique to the scope for its life, after which it returns the ticket to here.
        utilities::TicketDispensor
            scope_indexs;
        concurrency::concurrent_vector<Breadcrumb*>
            scopes; // namespaces and classes may add themselves to this list (order not guarranteed) to help with debugging or other activities. 

    public:
        RootScope() 
            : NamespaceScope("::", ScopeType::Basic & ScopeType::Namespace & ScopeType::Root, nullptr)
        {};
        virtual ~RootScope() {
            this->unload(); // must call the namespace's unload function BEFORE this destroys itself, otherwise connections are unable to resolve themselves. 
        };

    };

};

class stackThing {
public:
    std::string varName;
    bool perform_cout;

public:
    stackThing() : varName(), perform_cout{ true }{};
    stackThing(std::string const& name) : varName(name), perform_cout{ true } {};
    stackThing(std::string const& name, bool DoCout) : varName(name), perform_cout{ DoCout } {};
    stackThing(stackThing const& r) = default;
    stackThing(stackThing&& r) = default;
    stackThing& operator=(stackThing const& r) = default;
    stackThing& operator=(stackThing&& r) = default;
    ~stackThing() {
        if (perform_cout && (!varName.empty())) {
            std::cout << GoodLang::printf("DELETING %s\n", varName.c_str()) << std::endl;
        }
    };

    int length() const { return varName.length(); };
    std::string& get_var_name() { return varName; };
    bool operator==(stackThing const& a) const { return varName == a.varName; };
    bool operator!=(stackThing const& a) const { return varName != a.varName; };
};


int main() {
    using namespace utilities;
    using namespace ABA_Problem;
    Stopwatch sw;

    if (true) {
        print("STARTING LOOP: \n");

        // Testing utilities::shared_ptr
#if 1
        // utilities::shared_pointer is slower than the GoodLang::shared_pointer, BUT has a much lower memory footprint. 
        print("");
        if (1) {
            using shared_ptr = utilities::shared_ptr<utilities::string>;
            using weak_ptr = utilities::weak_ptr<utilities::string>;

            sw.Start();
            EXPECT_EQ(0, shared_ptr::num_allocations());
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                EXPECT_EQ(ptr, true);
            }
            EXPECT_EQ(0, shared_ptr::num_allocations());
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2{ ptr };
                EXPECT_EQ(ptr2.get(), ptr.get());
                EXPECT_EQ(*ptr2.get(), *ptr.get());
            }
            EXPECT_EQ(0, shared_ptr::num_allocations());
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2;
                ptr2 = ptr;
                EXPECT_EQ(ptr2, true);
                EXPECT_EQ(ptr2.get(), ptr.get());
            }
            EXPECT_EQ(0, shared_ptr::num_allocations());
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                weak_ptr ptr2{ ptr };
                ptr = ptr2.lock();
                EXPECT_EQ(ptr, true);
            }
            EXPECT_EQ(0, shared_ptr::num_allocations());
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                ptr = nullptr;
            });
            EXPECT_EQ(0, shared_ptr::num_allocations());
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2{ ptr };
                ptr = nullptr;
            });
            EXPECT_EQ(0, shared_ptr::num_allocations());
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2;
                ptr2 = ptr;
                ptr = nullptr;
            });
            EXPECT_EQ(0, shared_ptr::num_allocations());
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                weak_ptr ptr2{ ptr };
                ptr = ptr2.lock();
                ptr = nullptr;
                ptr2 = shared_ptr();
            });
            EXPECT_EQ(0, shared_ptr::num_allocations());
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            if (1) {
                shared_ptr temp_ptr{ new utilities::string(GoodLang::ToString(0)) };
                EXPECT_EQ(1, shared_ptr::num_allocations());
                GoodLang::parallel::For(1, 1000000, [&](int i) {
                    temp_ptr = shared_ptr{ new utilities::string(GoodLang::ToString(i)) };
                    // print(temp_ptr->get_var_name());
                });
            }
            EXPECT_EQ(0, utilities::shared_ptr<utilities::string>::num_allocations());
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        print("");
        if (0) {
            using shared_ptr = GoodLang::shared_ptr<utilities::string>;
            using weak_ptr = GoodLang::weak_ptr<utilities::string>;

            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                EXPECT_EQ(ptr, true);
            }
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2{ ptr };
                EXPECT_EQ(ptr2.get(), ptr.get());
                EXPECT_EQ(*ptr2.get(), *ptr.get());
            }
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2;
                ptr2 = ptr;
                EXPECT_EQ(ptr2, true);
                EXPECT_EQ(ptr2.get(), ptr.get());
            }
            for (int i = 0; i < 1000000; ++i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                weak_ptr ptr2{ ptr };
                ptr = ptr2.lock();
                EXPECT_EQ(ptr, true);
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                ptr = nullptr;
                });
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2{ ptr };
                ptr = nullptr;
                });
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                shared_ptr ptr2;
                ptr2 = ptr;
                ptr = nullptr;
                });
            GoodLang::parallel::For(0, 1000000, [](int i) {
                shared_ptr ptr{ new utilities::string(GoodLang::ToString(i)) };
                weak_ptr ptr2{ ptr };
                ptr = ptr2.lock();
                //ptr = nullptr;
                //ptr2 = shared_ptr();
                });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
            sw.Start();
            if (1) {
                shared_ptr temp_ptr{ new utilities::string(GoodLang::ToString(0)) };
                GoodLang::parallel::For(0, 1000000, [&](int i) {
                    temp_ptr = shared_ptr{ new utilities::string(GoodLang::ToString(i)) };
                    });
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }

#endif

        // Testing utilities::type_of
#if 1
        EXPECT_EQ(true, utilities::type_of<int>().is_base());
        EXPECT_EQ(false, utilities::type_of<int>().is_ref());
        EXPECT_EQ(false, utilities::type_of<int>().is_const());
        EXPECT_EQ(false, utilities::type_of<int>().is_void());
        EXPECT_EQ(false, utilities::type_of<int>().is_temp());
        EXPECT_EQ(utilities::type_of<int>().get_name(), "int");

        EXPECT_EQ(false, utilities::type_of<void>().is_base());
        EXPECT_EQ(false, utilities::type_of<void>().is_ref());
        EXPECT_EQ(false, utilities::type_of<void>().is_const());
        EXPECT_EQ(true, utilities::type_of<void>().is_void());
        EXPECT_EQ(false, utilities::type_of<void>().is_temp());
        EXPECT_EQ(utilities::type_of<void>().get_name(), "void");

        EXPECT_EQ(false, utilities::type_of<GoodLang::Any>().is_base());
        EXPECT_EQ(false, utilities::type_of<GoodLang::Any>().is_ref());
        EXPECT_EQ(false, utilities::type_of<GoodLang::Any>().is_const());
        EXPECT_EQ(false, utilities::type_of<GoodLang::Any>().is_void());
        EXPECT_EQ(false, utilities::type_of<GoodLang::Any>().is_temp());
        EXPECT_EQ(true, utilities::type_of<GoodLang::Any>().is_any());
        EXPECT_EQ(utilities::type_of<GoodLang::Any>().get_name(), "class GoodLang::Any");

        EXPECT_EQ(false, utilities::type_of<int&>().is_base());
        EXPECT_EQ(true, utilities::type_of<int&>().is_ref());
        EXPECT_EQ(false, utilities::type_of<int&>().is_const());
        EXPECT_EQ(false, utilities::type_of<int&>().is_void());
        EXPECT_EQ(false, utilities::type_of<int&>().is_temp());
        EXPECT_EQ(utilities::type_of<int&>().get_name(), "int&");

        EXPECT_EQ(false, utilities::type_of<int const&>().is_base());
        EXPECT_EQ(true, utilities::type_of<int const&>().is_ref());
        EXPECT_EQ(true, utilities::type_of<int const&>().is_const());
        EXPECT_EQ(true, utilities::type_of<int const&>().is_const_ref());
        EXPECT_EQ(false, utilities::type_of<int const&>().is_void());
        EXPECT_EQ(false, utilities::type_of<int const&>().is_temp());
        EXPECT_EQ(utilities::type_of<int const&>().get_name(), "const int&");
        EXPECT_EQ((utilities::type_of<int>() + utilities::type::Temporary).get_name(), "int&&");

        EXPECT_EQ(utilities::type_of<int const&>().get_hash(), utilities::type_of<int const&>().get_hash());
        EXPECT_NE(utilities::type_of<int const&>().get_hash(), utilities::type_of<int&>().get_hash());
        EXPECT_EQ((utilities::type_of<int>() + utilities::type::Const).get_hash(), utilities::type_of<int const>().get_hash());
        EXPECT_EQ((utilities::type_of<int>() + utilities::type::Const + utilities::type::Reference).get_hash(), utilities::type_of<int const&>().get_hash());
        EXPECT_EQ((utilities::type_of<int const&>() - utilities::type::Const - utilities::type::Reference).get_hash(), utilities::type_of<int>().get_hash());

        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int const&>(), utilities::type_of<int const&>()));
        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int&>(), utilities::type_of<int const&>()));
        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int>(), utilities::type_of<int const&>()));
        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int const>(), utilities::type_of<int const&>()));
        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int>(), utilities::type_of<int&>()));
        EXPECT_EQ(false, utilities::type::can_free_cast(utilities::type_of<int&>(), utilities::type_of<int>()));
        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int>() + utilities::type::Temporary, utilities::type_of<int const&>()));
        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int>() + utilities::type::Temporary, utilities::type_of<int>()));
        EXPECT_EQ(true, utilities::type::can_free_cast(utilities::type_of<int>() + utilities::type::Temporary - utilities::type::Temporary, utilities::type_of<int>()));

        EXPECT_EQ("100", GoodLang::ToString(utilities::type_of<int>().GetCopyConstructor()(100)));

        utilities::types Types; 
        EXPECT_EQ(0, Types.size());
        Types += utilities::type_of<int>() + utilities::type::Const + utilities::type::Reference;
        Types += utilities::type_of<int>() + utilities::type::Temporary;
        EXPECT_EQ(2, Types.size());
        EXPECT_EQ(true, Types[0].is_const_ref());
        EXPECT_EQ(true, Types[1].is_temp());
        EXPECT_EQ(true, Types[2].is_void());

        if (1) {
            utilities::atomic_map<utilities::type, std::string> tree;
            tree[utilities::type_of<int>()] = "int";
            tree[utilities::type_of<int const&>()] = "const int&";
            tree[utilities::type_of<int>() + utilities::type::Temporary] = "int&&";
        }
        if (1) {
            utilities::atomic_unordered_map<utilities::type, std::string> tree;
            tree[utilities::type_of<int>()] = "int";
            tree[utilities::type_of<int const&>()] = "const int&";
            tree[utilities::type_of<int>() + utilities::type::Temporary] = "int&&";
        }






#endif // << NO LEAK

        // Testing atomic_ptr
#if 0
        sw.Start();
        if (1) {
            int* old_ptr;
            atomic_ptr<int> ptr{ nullptr };
            old_ptr = ptr.Set(reinterpret_cast<int*>((size_t)std::rand() + 1));
            EXPECT_EQ(old_ptr, nullptr);

            GoodLang::parallel::For(0, 1000000, [&](int i) {
                old_ptr = ptr.Set(reinterpret_cast<int*>((size_t)std::rand() + 1));
                EXPECT_NE(old_ptr, nullptr);
            });

            old_ptr = ptr.Set(nullptr);
            EXPECT_NE(old_ptr, nullptr);            
        }
        print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        sw.Start();
        if (1) {
            int* old_ptr;
            GoodLang::atomic_ptr<int> ptr{ nullptr };
            old_ptr = ptr.Set(reinterpret_cast<int*>((size_t)std::rand() + 1));
            EXPECT_EQ(old_ptr, nullptr);

            GoodLang::parallel::For(0, 1000000, [&](int i) {
                old_ptr = ptr.Set(reinterpret_cast<int*>((size_t)std::rand() + 1));
                EXPECT_NE(old_ptr, nullptr);
                });

            old_ptr = ptr.Set(nullptr);
            EXPECT_NE(old_ptr, nullptr);
        }
        print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        sw.Start();
        if (1) {
            int* old_ptr;
            std::atomic<int*> ptr{ nullptr };
            old_ptr = ptr.exchange(reinterpret_cast<int*>((size_t)std::rand() + 1));
            EXPECT_EQ(old_ptr, nullptr);

            GoodLang::parallel::For(0, 1000000, [&](int i) {
                old_ptr = ptr.exchange(reinterpret_cast<int*>((size_t)std::rand() + 1));
                EXPECT_NE(old_ptr, nullptr);
                });

            old_ptr = ptr.exchange(nullptr);
            EXPECT_NE(old_ptr, nullptr);
        }
        print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
#endif // << NO LEAK

        // Testing Allocator
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
#endif // << NO LEAK

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

        // Testing Scopes::Cache
#if 0
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
#endif // << NO LEAK

        // Testing Scopes::Scopes
#if 1
        if (1) {
            Scopes::RootScope root; // successfully starts a new script root

       // >> TEST SCOPES
#if 1
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto scope{ root.make_scope() };
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto& scope{ root.make_namespace("std") };
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                auto& scope{ root.make_namespace("std") };
                scope.invalidate_cache();
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            if (auto main_loop = GoodLang::parallel::AsThread([&]() {
                root.invalidate_cache();
            })) {
                GoodLang::parallel::For(0, 1000000, [&](int i) {
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

#if 1
            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                switch (i % 3) {
                case 0: {
#if 1
                    auto& scope1{ root.make_namespace("std") };
                    auto& scope2{ scope1.make_namespace("impl") };
                    auto scope3{ scope2.make_scope() };

                    scope3.add_using_here(scope2);
                    scope3.add_using_here(scope1);
                    scope3.add_using_here(root);

                    auto scope5{ scope3.make_scope() };
                    scope5.get_unique_index();
#endif 
                    break;
                }
                case 1: {
#if 1
                    auto& scope1{ root.make_namespace("std") };
                    auto& scope2{ scope1.make_namespace("string") };
                    auto& scope3{ scope2.make_namespace("impl") };
                    auto scope4{ scope3.make_scope() };

                    scope2.emplace_object_here("npos", GoodLang::Any(100)); // slow due to conflict with GoodLang::shared_ptr... 

                    scope4.add_using_here(scope3);
                    scope4.add_using_here(scope2);
                    scope4.add_using_here(scope1);
                    scope4.add_using_here(root);

                    auto scope5{ scope3.make_scope() };
                    auto scope6{ scope4.make_scope() };
                    scope5.get_unique_index();
                    scope6.get_unique_index();

#endif 
                    break;
                }
                case 2: {
#if 1
                    auto& scope1{ root.make_namespace("string") };
                    auto& scope2{ scope1.make_namespace("impl") };
                    auto scope3{ scope2.make_scope() };

                    scope1.emplace_object_here("npos", GoodLang::Any(200));

                    scope3.add_using_here(scope2);
                    scope3.add_using_here(scope1);
                    scope3.add_using_here(root);

                    auto scope5{ scope3.make_scope() };
                    scope5.get_unique_index();
#endif
                    break;
                }
                }
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
#endif // << NO LEAK

#if 1
            auto s = utilities::string("::std::string::");
            print(s.left_of("d::").c_str());
            print(s.right_of("d::").c_str());
            print(s.left_of("::std").c_str());
            print(s.right_of("::std").c_str());
            print(s.left_of("string::").c_str());
            print(s.right_of("string::").c_str());

            print(s.left_of_last("d::").c_str());
            print(s.right_of_last("d::").c_str());
            print(s.left_of_last("::std").c_str());
            print(s.right_of_last("::std").c_str());
            print(s.left_of_last("string::").c_str());
            print(s.right_of_last("string::").c_str());

            EXPECT_EQ(true, s.ends_with("::"));
            EXPECT_EQ(true, s.begins_with("::"));

            EXPECT_NE(nullptr, root.find_namespace(utilities::string("")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("::")));            
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("::std::")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("::std::string::")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("::std::string::impl::")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("::string::")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("::string::impl::")));

            EXPECT_EQ(nullptr, root.find_namespace(utilities::string("impl"))); // could not find "impl" from the root, which is (arguably) correct!             
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("std")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("std::string")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("std::string::impl")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("string"))->this_m.scope->find_namespace(utilities::string("impl")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("string")));
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("string::impl")));

            EXPECT_NE(nullptr, root.find_namespace(utilities::string("::std::string::"))->this_m.scope->find_object_here("npos"));
            EXPECT_NE(nullptr, root.find_object("::std::string::npos"));
            EXPECT_EQ(nullptr, root.find_object("npos")); // should not be successfully found.
            EXPECT_NE(nullptr, root.find_object("std::string::npos"));
            EXPECT_EQ("100", GoodLang::ToString(**root.find_object("std::string::npos")));
            EXPECT_NE(nullptr, root.find_object("::string::npos"));
            EXPECT_EQ("200", GoodLang::ToString(**root.find_object("::string::npos"))); 
            EXPECT_EQ(nullptr, root.find_object("::npos")); // should not be successfully found.

            EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("npos"));
            EXPECT_NE(nullptr, root.find_namespace("std")->this_m.scope->find_object("string::npos"));
            EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("string"));
            EXPECT_EQ(nullptr, root.find_namespace("std")->this_m.scope->find_object("string2::npos")); // this namespace does not exist and will not be found. 
            EXPECT_EQ(nullptr, root.find_object("std::npos")); // should not be successfully found.
            EXPECT_EQ(nullptr, root.find_object("std::string")); // should not be successfully found.
            EXPECT_EQ(nullptr, root.find_object("std::string2::npos")); // should not be successfully found.

            EXPECT_NE(nullptr, root.find_namespace("::string::impl::")->this_m.scope->find_object("npos"));
            EXPECT_NE(nullptr, root.find_namespace("std::string::impl::")->this_m.scope->find_object("npos"));

            EXPECT_EQ("100", GoodLang::ToString(**root.find_namespace("std::string::impl::")->this_m.scope->find_object("npos")));
            EXPECT_EQ("200", GoodLang::ToString(**root.find_namespace("::string::impl::")->this_m.scope->find_object("npos")));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                EXPECT_NE(nullptr, root.find_namespace("std")->this_m.scope->find_object("string::npos"));
                EXPECT_NE(nullptr, root.find_object("std::string::npos"));
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            sw.Start();
            GoodLang::parallel::For(0, 1000000, [&](int i) {
                EXPECT_EQ(nullptr, root.find_object("std::string2::npos")); // should not be successfully found.
            });
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));

            if (1) {
                auto scope1{ root.make_scope() };
                scope1.add_using_here(*scope1.find_namespace("::std::string::")->this_m.scope->GetNamespace());
                EXPECT_NE(nullptr, scope1.find_object("npos")); // should be successfully found now, due to the using statement.
            }

            if (1) {
                root.add_using_here(*root.find_namespace("::std::string::")->this_m.scope->GetNamespace());
                EXPECT_NE(nullptr, root.find_object("npos")); // should be successfully found now, due to the using statement.
            }
            EXPECT_NE(nullptr, root.find_object("::std::string::npos"));
            EXPECT_NE(nullptr, root.find_object("::npos"));
            EXPECT_NE(nullptr, root.find_namespace("UI")->this_m.scope->find_object("npos"));

#endif // << NO LEAK
#endif // << NO LEAK

       // >> TEST FUNCTION CALLS
#if 1
            Functions funcs;
            funcs.emplace("a", utilities::FunctionWrapper(GoodLang::make_callable([](void) -> int { return 0; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("a", utilities::FunctionWrapper(GoodLang::make_callable([](int i) -> int { return i; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("b", utilities::FunctionWrapper(GoodLang::make_callable([](int i) -> int { return i; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("c", utilities::FunctionWrapper(GoodLang::make_callable([](int i, int j) -> int { return i+j; }), utilities::FunctionWrapper::FunctionState::Normal, {}));
            funcs.emplace("d", utilities::FunctionWrapper(GoodLang::make_callable([](int i, int j, int k) -> int { return i+j+k; }), utilities::FunctionWrapper::FunctionState::Normal, { 10, 10, 10 })); // has defaults!
            
            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j, int const& k) -> std::string { return "3 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal, 
                { 10, 10, 10 }));

            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j) -> std::string { return "2 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal, 
                { 10, 10 }));

            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i) -> std::string { return "1 param"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10 }));

            funcs.emplace("example", utilities::FunctionWrapper(GoodLang::make_callable(
                []() -> std::string { return "no params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                {}));

            funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j, int const& k) -> std::string { return "3 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10.0, 10, 10.0 }));

            funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                [](int const& i, int const& j) -> std::string { return "2 params"; }
            ), utilities::FunctionWrapper::FunctionState::Normal,
                { 10.0, 10 }));

            funcs.emplace("example2", utilities::FunctionWrapper(GoodLang::make_callable(
                [](double const& i, double const& j) -> std::string { return "2 param doubles!"; }
            ), utilities::FunctionWrapper::FunctionState::Normal));

            GoodLang::TypeConverter converter;
            converter.AddConverter<bool, int>();
            converter.AddConverter<int, bool>();
            converter.AddConverter<double, int>();
            converter.AddConverter<int, double>();
            converter.AddConverter<float, int>();
            converter.AddConverter<int, float>();
            converter.AddConverter<bool, float>();
            converter.AddConverter<float, bool>();
            converter.AddConverter<double, float>();
            converter.AddConverter<float, double>();
            converter.AddConverter<bool, double>();
            converter.AddConverter<double, bool>();

            // including these conversion checks "fixes" it. IDK why. 
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<int>(), GoodLang::user_type_shared_ptr<double>()));
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<int>(), GoodLang::user_type_shared_ptr<double>()->MakeConstRef().lock()));
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<double>(), GoodLang::user_type_shared_ptr<int>()));
            print(converter.ConversionCost_Fast(GoodLang::user_type_shared_ptr<double>(), GoodLang::user_type_shared_ptr<int>()->MakeConstRef().lock()));





            EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes(), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>(), GoodLang::user_type_shared<int>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("a", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function); // test providing more params than needed
            EXPECT_NE(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);

            print(GoodLang::ToString(funcs.Call("a", {}, converter)));
            print(GoodLang::ToString(funcs.Call("b", { 100.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("c", { 200.0, 200.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("d", { 500.0, 50, true }, converter)));
            print(GoodLang::ToString(funcs.Call("a", { 100, 200.0 }, converter)));
            EXPECT_EQ(nullptr, funcs.BuildMatch("b", GoodLang::ParamTypes(), converter).function);
            EXPECT_EQ(nullptr, funcs.BuildMatch("c", GoodLang::ParamTypes(), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>(), GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes({ GoodLang::user_type_shared<float>() }), converter).function);
            EXPECT_NE(nullptr, funcs.BuildMatch("d", GoodLang::ParamTypes(), converter).function);
            print(GoodLang::ToString(funcs.Call("d", {}, converter)));

            print(GoodLang::ToString(funcs.Call("example", {}, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10, 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10, 10, 10, 10 }, converter)));

            print(GoodLang::ToString(funcs.Call("example", { 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example", { 10.0, 10.0, 10.0, 10.0 }, converter)));

            print(GoodLang::ToString(funcs.Call("example2", { 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10, 10, 10 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10.0, 10.0, 10.0 }, converter)));
            print(GoodLang::ToString(funcs.Call("example2", {}, converter)));
            print(GoodLang::ToString(funcs.Call("example2", { 10.0, 10 }, converter))); // prefers the double-type since it keeps the first type
            print(GoodLang::ToString(funcs.Call("example2", { 10, 10.0 }, converter))); // prefers the int-type since it keeps the first type

#endif // << NO LEAK

       // TEST SEARCHING FOR SCOPES
#if 1
            Scopes::Breadcrumb* nearest;

            nearest = nullptr;
            EXPECT_EQ(nullptr, root.find_namespace(utilities::string("impl"), nearest)); // does not find it, but returns the root as the nearest location
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            nearest = nullptr;
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("std::string::impl"), nearest)); // successfully finds it
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());
            
            nearest = nullptr;
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("std::impl"), nearest)); // successfully finds it
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            nearest = nullptr;
            EXPECT_NE(nullptr, root.find_namespace(utilities::string("string::impl"), nearest)); // successfully finds it
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

            nearest = nullptr;
            EXPECT_EQ(nullptr, root.find_namespace(utilities::string("string::impl::impl"), nearest)); // does not find it, but does locate the nearest location
            EXPECT_NE(nullptr, nearest);
            if (nearest) print(nearest->GetCurrentNamespace().c_str());

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
#endif // << NO LEAK
        }
#endif // << NO LEAK

        // Testing utilities::string
#if 0
        // In order of slowest to fastest way to manage strings using shared_string...
        if (1) { // Copying std::strings
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test = std::string("TEST");
                (void)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying std::string_views
            sw.Start();
            std::string sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test = utilities::string(sv);
                (void)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Creating a new reference from a globally constant string
            sw.Start();
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test{ "TEST" };
                (void)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying a reference to an existing shared_string
            sw.Start();
            std::string_view sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test{ std::string_view{ sv } };
                (void)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // Copying a reference to an existing shared_string
            sw.Start();
            utilities::string sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                utilities::string test{ sv };
                (void)test.c_str();
            }
            print(GoodLang::ToString(GoodLang::Units::second(sw.Stop_s())) + " @ " + GoodLang::ToString(__LINE__));
        }
        if (1) { // using std::string_view without scope guarrantees. (This particular example gets optimized-out entirely down to 0.00 seconds)
            sw.Start();
            std::string_view sv{ "TEST" };
            for (int i = 0; i < 1000000; ++i) {
                std::string_view test{ sv };
                (void)test.data();
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
#endif
    }

};
