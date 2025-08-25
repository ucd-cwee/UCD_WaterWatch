#pragma once

#include <memory>
#include <functional>
#include "aba_problem.h"
#include "atomic_maps.h"

namespace GL {
    namespace impl {
        template<class T>
        union TFlaggedHead {
        public:
            struct bitset {
            public:
                uint64_t // must sum to 64
                    m_nFlags : 2,
                    m_nABA : 10, // 8, 12, and 18 work. Larger = less likelihood of crashing due to ABA bug.
                    m_pNode : 52; // Windows only supports 44 bits addressing anyway.
            };
            uint64_t
                m_n64; // for CAS
            bitset
                m_bits;

            static T* Finalize(T* p) {
                TFlaggedHead<T> out;
                out.m_bits.m_pNode = (uint64_t)p;
                out.m_bits.m_nFlags = 0;
                out.m_bits.m_nABA = 0;
                return (T*)out.m_bits.m_pNode;
            };

            T* Finalize() {
                return Finalize(Node());
            };
            bool is_null() const {
                return m_bits.m_pNode == 0;
            };

            // this constructor will make an atomic copy on intel 
            T* Node() const { return reinterpret_cast<T*>(m_bits.m_pNode); }
            char Flag() const { return static_cast<const char>(m_bits.m_nFlags); }

            // changeing Node bumps aba
            TFlaggedHead* Node(T* p) { m_bits.m_nABA++; m_bits.m_pNode = (uint64_t)p; return this; }

            TFlaggedHead* Node(T* p, char flag_option) { m_bits.m_nABA++; m_bits.m_nFlags = (uint64_t)flag_option; m_bits.m_pNode = (uint64_t)p; return this; }

            TFlaggedHead* Node(TFlaggedHead const& p) {
                uint64_t copied_m_n64{ p.m_bits };
                m_bits.m_nABA++; m_bits.m_nFlags = copied_m_n64.m_nFlags; m_bits.m_pNode = copied_m_n64.m_pNode;
                return this;
            }

        };
        
        /// <summary>
        /// Thread-safe and fiber-safe wrapper for atomic operations on pointers, without having to utilize std_atomic(T*)
        /// </summary>
        /// <typeparam name="T"></typeparam>
        template< typename T>
        struct atomic_ptr {
        private:
            // pop head of the list
            template< typename F>
            static std::pair<TFlaggedHead<T>, bool> Pop_If(TFlaggedHead<T>& Head, F const& func) {
                TFlaggedHead<T> Old, New;
                while (1) { // race loop
                    // Get an atomic copy of head and call it old.
                    Old.m_n64 = Head.m_n64;
                    if (!func(Old)) return { TFlaggedHead<T>{}, false };
                    New.m_n64 = Old.m_n64;
                    New.Node(Old.Node(), Old.Flag());
                    // compare and swap New with Head if it still matches Old.
                    if (aba_problem::CAS(&Head.m_n64, Old.m_n64, New.m_n64)) {
                        return { Old, true }; // success                
                    }
                    // race, try again
                }
            }
            // pop head of the list
            static TFlaggedHead<T> Pop(TFlaggedHead<T>& Head) {
                TFlaggedHead<T> Old, New;
                while (1) { // race loop
                    // Get an atomic copy of head and call it old.
                    Old.m_n64 = Head.m_n64;
                    New.m_n64 = Old.m_n64;
                    New.Node(Old.Node(), Old.Flag());
                    // compare and swap New with Head if it still matches Old.
                    if (aba_problem::CAS(&Head.m_n64, Old.m_n64, New.m_n64)) {
                        return Old; // success                
                    }
                    // race, try again
                }
            }
            // push pNode onto head of list, recieve old head
            static TFlaggedHead<T> Push(TFlaggedHead<T>& Head, T* pNode, char flag_option) {
                TFlaggedHead<T> Old, New;
                while (1) { // race loop
                    // Get an atomic copy of head and call it old.
                    // Copy old and call it new.
                    New.m_n64 = Old.m_n64 = Head.m_n64;
                    // change New's head ptr, which bumps internal aba
                    New.Node(pNode, flag_option);
                    // compare and swap New with Head if it still matches Old.
                    if (aba_problem::CAS(&Head.m_n64, Old.m_n64, New.m_n64)) break; // success
                    // race, try again
                }
                return Old;
            }
            // push pNode onto head of list, recieve old head
            template< typename F>
            static std::pair<TFlaggedHead<T>, bool> Push_If(TFlaggedHead<T>& Head, T* pNode, char flag_option, F const& func) {
                TFlaggedHead<T> Old, New;
                while (1) { // race loop
                    // Get an atomic copy of head and call it old.
                    // Copy old and call it new.
                    New.m_n64 = Old.m_n64 = Head.m_n64;

                    if (!func(Old)) return { TFlaggedHead<T>{}, false };

                    // change New's head ptr, which bumps internal aba
                    New.Node(pNode, flag_option);
                    // compare and swap New with Head if it still matches Old.
                    if (aba_problem::CAS(&Head.m_n64, Old.m_n64, New.m_n64)) break; // success
                    // race, try again
                }
                return { Old, true };
            }

        public:
            atomic_ptr() noexcept {
                ptr.m_n64 = 0;
            };
            atomic_ptr(T* newSource, char flag_option) noexcept {
                ptr.Node(newSource, flag_option);
            };
            atomic_ptr(const atomic_ptr& other) noexcept {
                ptr.Node(other.ptr);
            };
            atomic_ptr& operator=(const atomic_ptr& other) noexcept { ptr.Node(other.ptr); return *this; };
            atomic_ptr& operator=(T* newSource) noexcept { ptr.Node(newSource); return *this; };
            ~atomic_ptr() = default;

            explicit operator bool() { return !ptr.is_null(); };
            explicit operator bool() const { return !ptr.is_null(); };

            std::tuple<T*, char, bool> load_if(char f) const {
                auto current_head = Pop_If(ptr, [f](TFlaggedHead<T> const& old) -> bool {
                    return old.Flag() == f;
                });
                return { current_head.first.Finalize(), current_head.first.Flag(), current_head.second };
            };
            std::pair<T*, char> load() const {
                auto current_head = Pop(ptr);
                return { current_head.Finalize(), current_head.Flag() };
            };
            std::pair<T*, char> exchange(T* p, char f) const {
                auto current_head = Push(ptr, p, f);
                return { current_head.Finalize(), current_head.Flag() };
            };
            std::tuple<T*, char, bool> exchange_if(T* p, char f) const {
                auto current_head = Push_If(ptr, p, f, [f](TFlaggedHead<T> const& old) -> bool {
                    return old.Flag() == f;
                });
                return { current_head.first.Finalize(), current_head.first.Flag(), current_head.second };
            };

        protected:
            mutable TFlaggedHead<T> ptr;
        };






    }





};


