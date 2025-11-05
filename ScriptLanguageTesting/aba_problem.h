#pragma once
#include <ShlDisp.h>
#include <winnt.h>

// Good Language namespace
namespace GL {
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

        static bool CAS(size_t* Destination, size_t& Comperand, size_t& Exchange) {
            return InterlockedCompareExchange(reinterpret_cast<volatile size_t*>(Destination), Exchange, Comperand) == Comperand;
        };

        template<class T>
        static bool CAS(THead<T>& Head, T* Comperand, T* Exchange) {
            THead<T> Old, New; // Get an atomic copy of head and call it old.
            if (1) { // race loop
                New.m_n64 = (Old.m_n64 = Head.m_n64);       
                if (Old.Node() != Comperand) return false;
                New.Node(Exchange);
                if (CAS(&Head.m_n64, Old.m_n64, New.m_n64))
                    return true;
            } // race, try again
            return false; 
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

        // Thread-safe, lock-free, good-performance queue with LIFO functionality. Lower memory footprint than atomic_parallel_stack.
        template <typename T>
        class stack {
            struct element_t {
                T
                    data;
                element_t*
                    m_pNext;
            };
            aba_problem::THead<element_t>
                head;
        public:
            stack() = default;
            stack(stack const&) = delete;
            stack(stack&&) = delete;
            stack& operator=(stack const&) = delete;
            stack& operator=(stack&&) = delete;
            ~stack() {
                while (true) {
                    if (element_t* ptr = aba_problem::Pop(head)) {
                        delete ptr;
                    }
                    else {
                        break;
                    }
                }
            };
        public:
            void push(T const& obj) {
                element_t* new_ptr = new element_t();
                new_ptr->data = obj;
                new_ptr->m_pNext = nullptr;
                aba_problem::Stack_Push(head, new_ptr);
            };
            void push(T&& obj) {
                element_t* new_ptr = new element_t();
                new_ptr->data = std::move(obj);
                new_ptr->m_pNext = nullptr;
                aba_problem::Stack_Push(head, new_ptr);
            };
            bool try_pop(T& out) {
                if (element_t* ptr = aba_problem::Pop(head)) {
                    if constexpr (std::is_move_assignable<T>::value) {
                        out = std::move(ptr->data);
                    }
                    else {
                        out = ptr->data;
                    }
                    delete ptr;
                    return true;
                }
                return false;
            };
        };

    };

};