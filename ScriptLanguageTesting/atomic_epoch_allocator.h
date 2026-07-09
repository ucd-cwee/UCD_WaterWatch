#pragma once

#include "atomic_vector.h"
#include "ticket_dispensor.h"
#include "thread_object.h"
#include <vector>
#include <concurrent_priority_queue.h>
#include <queue>

namespace GL {
    template <size_t SZ, size_t BlockSize> struct block;
    // actual element data is located at the start. The inner, hidden data is located at the footer. 
    template <size_t SZ, size_t BlockSize> struct element {
        unsigned char
            data[(((SZ + sizeof(element*) + sizeof(block<SZ, BlockSize>*) + sizeof(long long)) + 15) & ~15) - sizeof(element*) - sizeof(block<SZ, BlockSize>*) - sizeof(long long)]; // wrapped to 16-byte blocks for the entire element_t
        element*
            m_pNext;
        block<SZ, BlockSize>*
            m_block;
        long long
            epoch;
    };
    // collection of elements and footer info.
    template <size_t SZ, size_t BlockSize> struct block {
        element<SZ, BlockSize>
            elements[BlockSize];
        block<SZ, BlockSize>*
            m_pNext;
        unsigned long long
            count_free;
        size_t
            block_position;
        long long
            youngest_epoch;
        size_t // the thread this block is intended to be assigned to. 
            parent_thread;
    };

    // Allocator that re-uses entire blocks of memory simultaneously. Each thread uses its own free list.
    // In rare cases, if the entire block is not free'd, the memory cannot be re-used. 
    // Handles the edge-case of the destruction of a thread without releasing it's free list - that list is released to a global list.
    template <typename T, size_t BlockSize = 256> class fast_atomic_allocator {
    private:
        using element_t = element<sizeof(T), BlockSize>;
        using block_t = block<sizeof(T), BlockSize>;

        static block_t* PushBlock() {
            block_t* p = reinterpret_cast<block_t*>(GL::malloc(sizeof(block_t)));
            if (p) std::memset(p, 0, sizeof(block_t));
            return p;
        };
        static void PopBlock(block_t* p) {
            GL::mfree(p);
        };

        // Allocate one new block of contiguous elements. These elements will be unique to this thread and unaccessible elsewhere. 
        void AllocBlock() {
            block_t* new_block_ptr = PushBlock();
            blocks.get_or_make(new_block_ptr->block_position = blocks_tickets.get_ticket()) = new_block_ptr;
            block_t& block = *new_block_ptr;
            new_block_ptr->parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].m_pNext = &block.elements[i + 1];
                block.elements[i].m_block = new_block_ptr;
            }
            block.elements[BlockSize - 1].m_pNext = nullptr;
            block.elements[BlockSize - 1].m_block = new_block_ptr;
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Allocate one old block of contiguous elements back onto the free list
        void ReallocBlock(block_t* existing_block_ptr) {
            block_t& block = *existing_block_ptr;
            block.parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].epoch = 0;
                block.elements[i].m_pNext = &block.elements[i + 1];
                ((T*)&block.elements[i].data[0])->~T();
            }
            block.elements[BlockSize - 1].epoch = 0;
            block.elements[BlockSize - 1].m_pNext = nullptr;
            ((T*)&block.elements[BlockSize - 1].data[0])->~T();
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Release memory held by this block
        void ReleaseBlock(block_t* ptr) noexcept {
            if constexpr (!std::is_pod_v<T>) {
                if (ptr) {
                    for (int element_i = 0; element_i < BlockSize; ++element_i) {
                        auto& element = ptr->elements[element_i];
                        if (element.epoch > 0) {
                            reinterpret_cast<T*>(&element.data[0])->~T();
                            element.epoch = 0;
                        }
                    }
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
            else {
                if (ptr) {
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
        };

        // Release all memory held by all blocks
        void ReleaseBlocks() noexcept {
            for (block_t*& ptr : blocks) {
                if (ptr) {
                    if constexpr (!std::is_pod_v<T>) {
                        for (int element_i = 0; element_i < BlockSize; ++element_i) {
                            auto& element = ptr->elements[element_i];
                            if (element.epoch > 0) {
                                reinterpret_cast<T*>(&element.data[0])->~T();
                                element.epoch = 0;
                            }
                        }
                    }
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                    ptr = nullptr;
                }
            }
        };

    public:
        fast_atomic_allocator()
            : blocks()
            , blocks_tickets()
            , m_free()
            , count_allocated()
        {
            const_cast<size_t&>(count_allocated._default) = 0;
            const_cast<element_t*&>(m_free._default) = nullptr;
            m_free._before_destruction = [this](element_t*& old_thread) {
                std::set<block_t*> blockss;
                element_t* element{ nullptr };
                while (old_thread) {
                    element = old_thread;
                    old_thread = element->m_pNext;
                    blockss.insert(element->m_block);
                }
                for (auto& x : blockss)
                    ReleaseBlock(x);
            };
        };
        fast_atomic_allocator(fast_atomic_allocator const&) = delete;
        fast_atomic_allocator(fast_atomic_allocator&&) = delete;
        fast_atomic_allocator& operator=(fast_atomic_allocator const&) = delete;
        fast_atomic_allocator& operator=(fast_atomic_allocator&&) = delete;
        ~fast_atomic_allocator() noexcept {
            m_free._before_destruction = nullptr;
            ReleaseBlocks();
        };

        // Acquire a new element from the Free list and construct it.
        template <typename... TArgs> __declspec(noinline) T* Alloc(TArgs &&... a) {
            element_t* element{ nullptr };
            element_t*& free{ *m_free };
            while (1) {
                if (element = free) {
                    free = element->m_pNext;
                    element->epoch = 1ll; // std::numeric_limits<long long>::max(); // indicates it's been initiated
                    T* data{ (T*)&element->data[0] };
                    if constexpr (std::is_pod<T>::value) {
                        if constexpr (sizeof...(a) > 0) {
                            new (data) T(std::forward<TArgs>(a)...);
                        }
                        else {
                            std::memset(data, 0, sizeof(T));
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

        // Destroys the element and return its memory to the Free list
        void Free(T* element) {
            element_t* t = (element_t*)(element);
            // GL::interlocked::compare_exchange(t->epoch, 1ll, GL::util::get_current_epoch());
            t->epoch = GL::util::get_current_epoch();

            if (GL::interlocked::decrement(t->m_block->count_free) == 0) {
                auto* Where = &count_allocated[t->m_block->parent_thread];
                while (Where) {
                    auto old = *Where;
                    if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                        old = *count_allocated;
                        if ((old / BlockSize) > 10) {
                            ReleaseBlock(t->m_block);
                        }
                        else {
                            ReallocBlock(t->m_block);
                        }
                        break;
                    }
                }
            }
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

    private:
        GL::atomic_vector<block_t*>
            blocks; // vector of all blocks. May or may not be nullptr. 
        GL::ticket_dispensor<false>
            blocks_tickets; // ticket dispensor to re-use blocks indexes and minimize the size of blocks
        GL::thread_object<element_t*>
            m_free; // since each thread is guarranteed to access their free-list without conflict, it does not need to be managed by an aba-protector.
        GL::thread_object<size_t>
            count_allocated; // this exists as part of a fix for thread_local allocations being free-d on other threads (e.g. consumer-producer pattern). 
    };

    // Allocator that re-uses entire blocks of memory simultaneously. Each thread uses its own free list.
    // In rare cases, if the entire block is not free'd, the memory cannot be re-used. 
    // Handles the edge-case of the destruction of a thread without releasing it's free list - that list is released to a global list.
    template <typename T, size_t BlockSize = 256> class fast_atomic_epoch_allocator {
    private:
        using element_t = element<sizeof(T), BlockSize>;
        using block_t = block<sizeof(T), BlockSize>;

        static block_t* PushBlock() {
            block_t* p = reinterpret_cast<block_t*>(GL::malloc(sizeof(block_t)));
            if (p) std::memset(p, 0, sizeof(block_t));
            return p;
        };
        static void PopBlock(block_t* p) {
            GL::mfree(p);
        };

        // Allocate one new block of contiguous elements onto the free list
        void AllocBlock() {
            block_t* new_block_ptr = PushBlock();
            blocks.get_or_make(new_block_ptr->block_position = blocks_tickets.get_ticket()) = new_block_ptr;
            block_t& block = *new_block_ptr;
            new_block_ptr->parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].m_pNext = &block.elements[i + 1];
                block.elements[i].m_block = new_block_ptr;
            }
            block.elements[BlockSize - 1].m_pNext = nullptr;
            block.elements[BlockSize - 1].m_block = new_block_ptr;
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Allocate one old block of contiguous elements back onto the free list. This requires that it has been retired and is safe to reclaim. 
        void ReallocBlock(block_t* existing_block_ptr) {
            block_t& block = *existing_block_ptr;
            block.parent_thread = GL::util::get_thread_id();

            // add the new elements to the list
            for (int i = 0; i < BlockSize - 1; ++i) {
                block.elements[i].epoch = 0;
                block.elements[i].m_pNext = &block.elements[i + 1];
                ((T*)&block.elements[i].data[0])->~T();
            }
            block.elements[BlockSize - 1].epoch = 0;
            block.elements[BlockSize - 1].m_pNext = nullptr;
            ((T*)&block.elements[BlockSize - 1].data[0])->~T();
            block.count_free = BlockSize;

            // push pNode onto head of list.
            *count_allocated += BlockSize;
            block.elements[BlockSize - 1].m_pNext = *m_free;
            *m_free = &block.elements[0];
        };

        // Release memory held by this block
        void ReleaseBlock(block_t* ptr) noexcept {
            if constexpr (!std::is_pod_v<T>) {
                if (ptr) {
                    for (int element_i = 0; element_i < BlockSize; ++element_i) {
                        auto& element = ptr->elements[element_i];
                        if (element.epoch > 0) {
                            reinterpret_cast<T*>(&element.data[0])->~T();
                            element.epoch = 0;
                        }
                    }
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
            else {
                if (ptr) {
                    this->blocks[ptr->block_position] = nullptr;
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                }
            }
        };

        enum class ReclamationResult {
            NoRetiredBlocks,
            FailedToReclaimAnyRetiredBlocks,
            ReclaimedRetiredBlocks
        };
        ReclamationResult TryReclaimRetiredBlocks() {
            block_t* block{ nullptr };
            ReclamationResult out = ReclamationResult::NoRetiredBlocks;
            bool failed_local = false;
            bool failed_global = false;
            auto& local_q = *retired_blocks;
            int allowed_repeatitions = 1;
            while (true) {
                if (!failed_local && (local_q.size() > 0)) {
                    block = local_q.top();
                    if ((block->youngest_epoch + 3) < this->current_epoch) {
                        local_q.pop();
                        if (out != ReclamationResult::ReclaimedRetiredBlocks || (--allowed_repeatitions > 0)) {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    if ((old / BlockSize) > 10) {
                                        ReleaseBlock(block);
                                    }
                                    else {
                                        ReallocBlock(block);
                                        out = ReclamationResult::ReclaimedRetiredBlocks;
                                    }
                                    break;
                                }
                            }
                        }
                        else {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    ReleaseBlock(block);
                                    break;
                                }
                            }
                        }
                    }
                    else {
                        if (out == ReclamationResult::NoRetiredBlocks) out = ReclamationResult::FailedToReclaimAnyRetiredBlocks;
                        failed_local = true;
                    }
                }
                else if (!failed_global && global_retired_blocks.try_pop(block)) {
                    if ((block->youngest_epoch + 3) < this->current_epoch) {
                        if (out != ReclamationResult::ReclaimedRetiredBlocks || (--allowed_repeatitions > 0)) {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    if ((old / BlockSize) > 10) {
                                        ReleaseBlock(block);
                                    }
                                    else {
                                        ReallocBlock(block);
                                        out = ReclamationResult::ReclaimedRetiredBlocks;
                                    }
                                    break;
                                }
                            }
                        }
                        else {
                            auto* Where = &count_allocated[block->parent_thread];
                            while (Where) {
                                auto old = *Where;
                                if (GL::interlocked::compare_exchange(*Where, old, old - BlockSize)) {
                                    old = *count_allocated;
                                    ReleaseBlock(block);
                                    break;
                                }
                            }
                        }
                    }
                    else {
                        if (out == ReclamationResult::NoRetiredBlocks) out = ReclamationResult::FailedToReclaimAnyRetiredBlocks;
                        local_q.push(block);
                        failed_global = true;
                    }
                }
                else {
                    return out;
                }
            }
        };

        // Release all memory held by all blocks
        void ReleaseBlocks() noexcept {
            for (block_t*& ptr : blocks) {
                if (ptr) {
                    if constexpr (!std::is_pod_v<T>) {
                        for (int element_i = 0; element_i < BlockSize; ++element_i) {
                            auto& element = ptr->elements[element_i];
                            if (element.epoch > 0) {
                                reinterpret_cast<T*>(&element.data[0])->~T();
                                element.epoch = 0;
                            }
                        }
                    }
                    this->blocks_tickets.return_ticket(ptr->block_position);
                    PopBlock(ptr);
                    ptr = nullptr;
                }
            }
        };

        class ThreadState {
        public:
            mutable long long
                epoch_2,
                epoch_1,
                epoch;
            mutable size_t
                epoch_depth;
            mutable long
                epoch_protected{ 0 };
            mutable long long
                queued_epoch;
            mutable long long
                delete_if_older_than;
            fast_atomic_epoch_allocator*
                parent;
            mutable int
                deferrment{ 0 };

            __declspec(noinline) void enter_critical_section() const {
                if (++epoch_depth == 1) {
                    GL::interlocked::exchange(epoch_protected, 1l);
                    queued_epoch = parent->current_epoch + 1;
                    if (deferrment <= 0) deferrment = 1'000;
                }
            };
            __declspec(noinline) void exit_critical_section() const {
                if (--epoch_depth == 0) {
                    GL::interlocked::exchange(epoch_protected, 0l);

                    // from the perspective of this thread, we are now OK to free pointers older than "epoch";
                    GL::interlocked::exchange(delete_if_older_than, epoch);
                    epoch = epoch_1;
                    epoch_1 = epoch_2;
                    epoch_2 = queued_epoch;

                    if (--deferrment == 0) {
                        // review the main thread to update the epoch number
                        long long old_epoch = parent->current_epoch;
                        long long currentEpoch = std::numeric_limits<long long>::max();
                        parent->states.for_each([&currentEpoch](auto& state) {
                            if (state.epoch_protected) {
                                currentEpoch = std::min<long long>(currentEpoch, state.delete_if_older_than);
                            }
                            });
                        if (old_epoch != currentEpoch) {
                            if (GL::interlocked::compare_exchange(parent->current_epoch, old_epoch, currentEpoch)) {
                                // std::cout << GL::printf("Updating the Current Epoch to %zu\n", (size_t)currentEpoch);
                            }
                        }
                    }
                }
            };
            auto guard_critical_section() const {
                class wrap {
                    const ThreadState* p;
                public:
                    wrap(const ThreadState* P) : p{ P } {};
                    wrap(wrap const&) = delete;
                    wrap(wrap&&) = delete;
                    wrap& operator=(wrap const&) = delete;
                    wrap& operator=(wrap&&) = delete;
                    ~wrap() {
                        p->exit_critical_section();
                    };
                };
                enter_critical_section();
                return wrap(this);
            };

            ThreadState() noexcept :
                epoch_2{ 0/*GL::util::get_current_epoch()*/ },
                epoch_1{ 0/*GL::util::get_current_epoch()*/ },
                epoch{ 0/*GL::util::get_current_epoch()*/ },
                delete_if_older_than{ 0/*GL::util::get_current_epoch()*/ },
                epoch_depth{ 0ull },
                queued_epoch{ 0/*GL::util::get_current_epoch()*/ },
                parent{ nullptr }
            {};
            ThreadState(ThreadState const&) = default;
            ThreadState(ThreadState&&) noexcept = default;
            ThreadState& operator=(ThreadState const&) = default;
            ThreadState& operator=(ThreadState&&) noexcept = default;
            ~ThreadState() {};
        };

    public:
        fast_atomic_epoch_allocator()
            : blocks()
            , m_free()
            , global_free{ 0ull }
            , retired_blocks()
            , global_retired_blocks{ 0ull }
            , current_epoch{ 0 }
            , states()
        {
            m_free._after_construction = [this](element_t*& new_thread) {
                new_thread = nullptr;
            };
            m_free._before_destruction = [this](element_t* old_thread) {
                while (old_thread) {
                    element_t* element{ old_thread };
                    if (element) {
                        old_thread = element->m_pNext;
                        GL::aba_problem::Stack_Push(global_free, element);
                    }
                }
            };
            global_free.m_n64 = 0;

            retired_blocks._before_destruction = [this](std::priority_queue<block_t*, std::vector<block_t*>, cmp>& old_thread) {
                while (old_thread.size() > 0) {
                    global_retired_blocks.push(old_thread.top());
                    old_thread.pop();
                }
            };

            states._after_construction = [this](ThreadState& state) {
                state.parent = this;
                state.epoch = state.epoch_1 = state.epoch_2 = state.delete_if_older_than = state.queued_epoch = this->current_epoch;
            };
            states._before_destruction = [this](ThreadState& state) {
                state.epoch_protected = 0;
            };
        };
        fast_atomic_epoch_allocator(fast_atomic_epoch_allocator const&) = delete;
        fast_atomic_epoch_allocator(fast_atomic_epoch_allocator&&) = delete;
        fast_atomic_epoch_allocator& operator=(fast_atomic_epoch_allocator const&) = delete;
        fast_atomic_epoch_allocator& operator=(fast_atomic_epoch_allocator&&) = delete;
        ~fast_atomic_epoch_allocator() noexcept {
            m_free._after_construction = nullptr;
            m_free._before_destruction = nullptr;
            retired_blocks._after_construction = nullptr;
            retired_blocks._before_destruction = nullptr;
            ReleaseBlocks();
        };

        // Acquire a new element from the Free list and construct it.
        template <typename... TArgs> T* Alloc(TArgs &&... a) {
            element_t* element{ nullptr };
            auto*& freeP = *m_free;
            T* data;
            ReclamationResult result;
            while (1) {
                if (freeP) {
                    element = freeP;
                    freeP = element->m_pNext;
                }
                else element = GL::aba_problem::Pop(global_free);
                if (element) {
                    element->epoch = 1ll; // std::numeric_limits<long long>::max(); // indicates it's been initiated
                    data = (T*)&element->data[0];
                    if constexpr (std::is_pod<T>::value) {
                        if constexpr (sizeof...(a) > 0) {
                            new (data) T(std::forward<TArgs>(a)...);
                        }
                        else {
                            std::memset(data, 0, sizeof(T));
                        }
                    }
                    else {
                        new (data) T(std::forward<TArgs>(a)...);
                    }
                    return data;
                }
                else {
                    result = TryReclaimRetiredBlocks();
                    if (result != ReclamationResult::ReclaimedRetiredBlocks) {
                        AllocBlock();
                    }
                }
            }
        };
        // Destroys the element and return its memory to the Free list
        void Free(T* element) {
            element_t* t = (element_t*)(element);
            t->epoch = current_epoch; // GL::util::get_current_epoch();
            // GL::interlocked::compare_exchange(t->epoch, 1ll, current_epoch);

            if (GL::interlocked::decrement(t->m_block->count_free) == 0) {
                // by definition, the most recent (youngest) epoch will be the one we just did that successfully retired the block...
                t->m_block->youngest_epoch = t->epoch;

                // queue the retired block
                retired_blocks->push(t->m_block);
            }
        };
        template <typename... TArgs> std::shared_ptr< T > AllocShared(TArgs&&... a) {
            return std::shared_ptr<T>(Alloc(std::forward<TArgs>(a)...), [this](T* p) { Free(p); });
        };

        typedef typename decltype(GL::details::detail::function_signature(&ThreadState::guard_critical_section))::Return_Type GuardType;
        [[nodiscard]] GuardType guard_critical_section() {
            return states->guard_critical_section();
        };

    private:
        struct cmp {
            constexpr bool operator()(block_t* const& lhs, block_t* const& rhs) {
                return lhs->youngest_epoch >= rhs->youngest_epoch;
            };
        };
        GL::atomic_vector<block_t*>
            blocks; // vector of all blocks currently allocated and alive
        GL::ticket_dispensor<false>
            blocks_tickets; // ticket dispensor to re-use blocks indexes and minimize the size of blocks
        GL::thread_object_no_default<element_t*>
            m_free; // thread-local free list of elements
        GL::aba_problem::THead<element_t>
            global_free; // shared free list of elements
        GL::thread_object_no_default<std::priority_queue<block_t*, std::vector<block_t*>, cmp>>
            retired_blocks; // retired (but alive) blocks, sorted by their youngest element's epoch. This is the thread-local queue.
        concurrency::concurrent_priority_queue<block_t*, cmp>
            global_retired_blocks; // retired (but alive) blocks, sorted by their youngest element's epoch. This is the shared queue.
        GL::thread_object_no_default< ThreadState >
            states; // thread states. Used to manage the scope guard and lifetime of objects. 
        long long
            current_epoch; // the current epoch that has been reached by the allocator. 
        GL::thread_object<size_t>
            count_allocated; // this exists as part of a fix for thread_local allocations being free-d on other threads (e.g. consumer-producer pattern). 
    };
};