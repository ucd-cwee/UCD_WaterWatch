#pragma once
#include "atomic_vector.h"
#include "thread_object.h"

// atomic stack of uint.
namespace GL {
    // atomic, thread-safe stack of unsigned integers. Specialized for single-threaded access with occassional, lock-free multi-threaded access.
    class atomic_uint_stack {
    private:
        static constexpr unsigned int INVALID = 0xFFFFFFFF;

        // Fits into a standard 64-bit atomic integer on modern platforms
        union TaggedIndex {
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

            static unsigned int Finalize(unsigned int p) noexcept {
                TaggedIndex out;
                out.m_bits.m_pNode = (uint64_t)p;
                out.m_bits.m_nABA = 0;
                return (unsigned int)out.m_bits.m_pNode;
            };
            bool is_null() const noexcept {
                return (m_bits.m_nABA == 0) || ((unsigned int)m_bits.m_pNode == INVALID);
            };
            unsigned int Node() noexcept { return (unsigned int)m_bits.m_pNode; }
            // changeing Node bumps aba
            TaggedIndex* Node(unsigned int p) noexcept { m_bits.m_nABA++; m_bits.m_pNode = (uint64_t)p; return this; }

            static TaggedIndex Init(unsigned int rhs) {
                TaggedIndex out;
                out.m_n64 = 0;
                out.m_bits.m_pNode = rhs;
                return out;
            };
        };

        struct Node {
            unsigned int data;
            unsigned int next; // Stores index of next node, or INVALID
        };

        GL::atomic_constructable_batch_vector<Node> nodes;
        // std::atomic<TaggedIndex> 
        TaggedIndex
            head;
    public:
        //std::atomic<TaggedIndex> 
        TaggedIndex
            free_head;

        // pop pNode from head of list.
        unsigned int Pop(TaggedIndex& Head) noexcept {
            TaggedIndex Old, New; // Get an atomic copy of head and call it old.
            while (1) { // race loop                
                New.m_n64 = (Old.m_n64 = Head.m_n64);
                if (Old.is_null()) { break; }
                New.Node(nodes.get_or_make(Old.Node()).next); // change New's Node, which bumps internal aba                
                if (GL::interlocked::compare_exchange(Head.m_n64, Old.m_n64, New.m_n64))
                    return /*TaggedIndex::Finalize(*/Old.Node()/*)*/; // success      
            } // race, try again
            return INVALID; // Head.m_n64.m_pNode was nullptr ... e.g. nothing to pop
        };
        // push pNode onto head of list. 
        void Stack_Push(TaggedIndex& Head, unsigned int pNode) noexcept {
            TaggedIndex Old, New;
            while (1) { // race loop                
                New.m_n64 = (Old.m_n64 = Head.m_n64); // Get an atomic copy of head and call it old. Copy old and call it new.                
                nodes.get_or_make(pNode).next = Old.Node(); // Wire node t Head    
                New.Node(pNode); // change New's head ptr, which bumps internal aba
                if (GL::interlocked::compare_exchange(Head.m_n64, Old.m_n64, New.m_n64)) // compare and swap New with Head if it still matches Old.
                    break; // success           
            } // race, try again
        }


    public:
        // Initialize stack with a maximum capacity
        __declspec(noinline) atomic_uint_stack()
            : nodes(
                [](Node* ptr, size_t count, short blockN) -> void {
                    // size_t starting_position = (blockN == 0) ? 0 : decltype(nodes)::block_to_total_allocsize((blockN == 0) ? 0 : blockN - 1);
                    // for (size_t i = 0; i < count; ++i)
                        // ptr[i].next = (i == (count - 1)) ? INVALID : static_cast<unsigned int>(i + 1 + starting_position);
                },
                [](Node* ptr, size_t count, short blockN, void* _data) -> void {
                    auto* self = static_cast<atomic_uint_stack*>(_data);
                    size_t starting_position = (blockN == 0) ? 0 : decltype(nodes)::block_to_total_allocsize((blockN == 0) ? 0 : blockN - 1);
                    for (size_t i = 0; i < count; ++i)
                        self->Stack_Push(self->free_head, (unsigned int)(starting_position + i));
                },
                this
            )
            , head{ TaggedIndex::Init(INVALID) }
            , free_head{ TaggedIndex::Init(INVALID) }
        {
            // Link all nodes into the free list initially
            size_t
                i;
            for (i = 0; i < decltype(nodes)::block_to_total_allocsize(4); ++i)
                nodes.get_or_make(i).next = static_cast<unsigned int>(i + 1);
            nodes.get_or_make(i).next = INVALID;
            free_head.Node(0); // free_head.store(*temp.Node(0), std::memory_order_relaxed);
        }

        // Push a value onto the stack
        __declspec(noinline) bool push(unsigned int value) {
            while (true) {
                // 1. Grab an available node index from the freelist
                unsigned int node_idx = Pop(free_head);
                if (node_idx == INVALID) {
                    nodes.get_or_make(
                        decltype(nodes)::block_to_total_allocsize(decltype(nodes)::total_allocsize_to_block(nodes.size()) + 1) - 1
                    );
                    continue;
                }

                // 2. Assign the data
                nodes.get_or_make(node_idx).data = value;

                // 3. Push it onto the main stack
                Stack_Push(head, node_idx);
                return true;
            }
        }

        // Pop a value from the stack
        __declspec(noinline) bool try_pop(unsigned int& result) {
            // 1. Grab a node index from the main stack
            unsigned int node_idx = Pop(head);
            if (node_idx == INVALID) {
                return false; // Stack underflow (empty)
            }

            // 2. Extract data
            result = nodes.get_or_make(node_idx).data;

            // 3. Return the node index back to the freelist
            Stack_Push(free_head, node_idx);
            return true;
        }
    };

#if 0
    // atomic, thread-safe stack of unsigned integers. Specialized for frequent multithreaded-threaded lock-free access.
    class parallel_atomic_uint_stack {
    private:
        static constexpr unsigned int INVALID = 0xFFFFFFFF;

        // Fits into a standard 64-bit atomic integer on modern platforms
        union TaggedIndex {
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

            static unsigned int Finalize(unsigned int p) noexcept {
                TaggedIndex out;
                out.m_bits.m_pNode = (uint64_t)p;
                out.m_bits.m_nABA = 0;
                return (unsigned int)out.m_bits.m_pNode;
            };
            bool is_null() const noexcept {
                return (unsigned int)m_bits.m_pNode == INVALID;
            };
            unsigned int Node() noexcept { return (unsigned int)m_bits.m_pNode; }
            // changeing Node bumps aba
            TaggedIndex* Node(unsigned int p) noexcept { m_bits.m_nABA++; m_bits.m_pNode = (uint64_t)p; return this; }
        };

        struct Node {
            unsigned int data;
            unsigned int next; // Stores index of next node, or INVALID
        };

        GL::atomic_constructable_batch_vector<Node>
            nodes;
        GL::thread_object<TaggedIndex>
            submitted_head;
    public:
        GL::thread_object<TaggedIndex>
            free_head;

        // Helper to push to a generic list (either main stack or freelist)
        __declspec(noinline) void push_to_list(std::atomic<TaggedIndex>& list_head, unsigned int node_idx) {
            TaggedIndex old_head, new_head;
            do {
                new_head.m_n64 = (old_head.m_n64 = list_head.load(std::memory_order_relaxed).m_n64);
                nodes[node_idx].next = old_head.Node();
                new_head.Node(node_idx);
            } while (!list_head.compare_exchange_weak(
                old_head, new_head,
                std::memory_order_release,
                std::memory_order_relaxed
            ));
        };

        // Helper to pop from a generic list
        __declspec(noinline) unsigned int pop_from_list(std::atomic<TaggedIndex>& list_head) {
            TaggedIndex old_head, new_head;
            do {
                new_head.m_n64 = old_head.m_n64 = list_head.load(std::memory_order_relaxed).m_n64;
                if (old_head.Node() == INVALID)
                    return INVALID;
                new_head.Node(nodes[old_head.Node()].next);
            } while (!list_head.compare_exchange_weak(
                old_head, new_head,
                std::memory_order_acquire,
                std::memory_order_relaxed));

            return old_head.Node();
        };

        // pop pNode from head of list.
        unsigned int Pop(TaggedIndex& Head) noexcept {
            TaggedIndex Old, New; // Get an atomic copy of head and call it old.
            while (1) { // race loop                
                New.m_n64 = (Old.m_n64 = Head.m_n64);
                if (Old.is_null()) { break; }
                New.Node(nodes[Old.Node()].next); // change New's Node, which bumps internal aba                
                if (GL::interlocked::compare_exchange(Head.m_n64, Old.m_n64, New.m_n64))
                    return TaggedIndex::Finalize(Old.Node()); // success      
            } // race, try again
            return INVALID; // Head.m_n64.m_pNode was nullptr ... e.g. nothing to pop
        };
        // push pNode onto head of list. 
        void Stack_Push(TaggedIndex& Head, unsigned int pNode) noexcept {
            TaggedIndex Old, New;
            while (1) { // race loop                
                New.m_n64 = Old.m_n64 = Head.m_n64; // Get an atomic copy of head and call it old. Copy old and call it new.                
                nodes[pNode].next = Old.Node(); // Wire node t Head    
                New.Node(pNode); // change New's head ptr, which bumps internal aba
                if (GL::interlocked::compare_exchange(Head.m_n64, Old.m_n64, New.m_n64)) // compare and swap New with Head if it still matches Old.
                    break; // success           
            } // race, try again
        }


    public:
        // Initialize stack with a maximum capacity
        parallel_atomic_uint_stack()
            : nodes(
                [](Node* ptr, size_t count, short blockN) -> void {
                    //size_t starting_position = (blockN == 0) ? 0 : decltype(nodes)::block_to_total_allocsize((blockN == 0) ? 0 : blockN - 1);
                    //for (size_t i = 0; i < count; ++i)
                        //ptr[i].next = (i == (count - 1)) ? INVALID : static_cast<unsigned int>(i + 1 + starting_position);
                },
                [](Node* ptr, size_t count, short blockN, void* _data) -> void {
                    auto* self = static_cast<parallel_atomic_uint_stack*>(_data);
                    size_t starting_position = (blockN == 0) ? 0 : decltype(nodes)::block_to_total_allocsize((blockN == 0) ? 0 : blockN - 1);
                    for (size_t i = 0; i < count; ++i)
                        self->Stack_Push(*self->free_head, (unsigned int)(starting_position + i));
                },
                this
            )
        {
            const_cast<TaggedIndex&>(free_head._default).Node(INVALID);
            const_cast<TaggedIndex&>(submitted_head._default).Node(INVALID);

            // Link all nodes into the free list initially
            size_t
                i;
            for (i = 0; i < decltype(nodes)::block_to_total_allocsize(4); ++i)
                nodes.get_or_make(i).next = static_cast<unsigned int>(i + 1);
            nodes.get_or_make(i).next = INVALID;
            submitted_head->Node(INVALID);
            free_head->Node(0);
        }

        // Push a value onto the stack
        bool push(unsigned int value) {
            while (true) {
                // 1. Grab an available node index from the freelist
                unsigned int node_idx = INVALID;
                if (!free_head.for_each_cancellable([&node_idx, this](auto& _head) -> bool {
                    node_idx = this->Pop(_head);
                    return node_idx != INVALID;
                })) {
                    nodes.get_or_make(
                        decltype(nodes)::block_to_total_allocsize(decltype(nodes)::total_allocsize_to_block(nodes.size()) + 1) - 1
                    );
                    continue;
                }

                // 2. Assign the data
                nodes[node_idx].data = value;

                // 3. Push it onto the main stack
                Stack_Push(*submitted_head, node_idx);
                return true;
            }
        }

        // Pop a value from the stack
        bool try_pop(unsigned int& result) {
            // 1. Grab a node index from the main stack
            unsigned int node_idx = INVALID;
            submitted_head.for_each_cancellable([&node_idx, this](auto& _head) -> bool {
                node_idx = this->Pop(_head);
                return node_idx != INVALID;
            });
            if (node_idx == INVALID) return false; // Stack underflow (empty)

            // 2. Extract data
            result = nodes[node_idx].data;

            // 3. Return the node index back to the freelist
            Stack_Push(*free_head, node_idx);
            return true;
        }
    };
#endif
}