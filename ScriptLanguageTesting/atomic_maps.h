#pragma once

#include <limits>
#include "atomic_allocator.h"
#include "atomic_queue.h"
#define NOMINMAX
#undef max
#undef min
#include <shared_mutex>

// Epoch Allocator
namespace GL {
    // Thread-safe, lock-free, okay-performance allocator that delays destruction until likely safe to do so. 
    // uses the real clock of the OS to time when enough periods have passed that a free'd pointer is likely forgotten. 
    template <typename _type_, typename AllocatorType = atomic_parallel_allocator<_type_>>
    class atomic_epoch_allocator {
    private:
        struct DeleteType {
            long long epoch;
            _type_* ptr;

            friend bool operator<(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch < rhs.epoch;
            };
            friend bool operator>(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch > rhs.epoch;
            };
            friend bool operator<=(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch <= rhs.epoch;
            };
            friend bool operator>=(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch >= rhs.epoch;
            };
            friend bool operator==(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch == rhs.epoch;
            };
            friend bool operator!=(DeleteType const& lhs, DeleteType const& rhs) {
                return lhs.epoch != rhs.epoch;
            };
        };

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
                atomic_epoch_allocator* _parent_parent;
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
                EpochGuard(atomic_epoch_allocator* parent_parent, TLS* parent, long long CurrentEpoch) : _parent_parent{ parent_parent }, _parent{ parent }, _CurrentEpoch{ CurrentEpoch } {
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
        AllocatorType // Allocator<_type_, 32> // , 32 // ABA_Problem::BlockAlloc<_type_, 32> // 
            _alloc;
        atomic_parallel_priority_queue<DeleteType>
            _delete_list; // note that these are NOT available for re-use yet -- these may still be being used by certain threads. 
        GL::thread_object_no_default<TLS>
            _TLS;
        long long
            _lastGC;

    public:
        // Performs the actual garbage collection. OK to call this over-and-over again, as it'll space itself out in time to prevent over-ambitous GC calls. 
        void RunGC() {
            static constexpr long long duration_ms{ 2 };

            long long curr_epoch{ GL::util::get_current_epoch() };
            long long previous_epoch{ _lastGC };
            long long _EpochLimit{ std::numeric_limits<long long>::max() };
            DeleteType out;

            if (((curr_epoch - previous_epoch) > duration_ms) && (InterlockedCompareExchange64(reinterpret_cast<volatile long long*>(&_lastGC), curr_epoch, previous_epoch) == previous_epoch)) {
                _TLS.for_each_alive([&_EpochLimit](TLS& _tls) {
                    if (long long L = _tls.EpochLimit; L >= 0 && L < _tls.Epoch_1) {
                        _EpochLimit = std::min<long long>(_EpochLimit, L);
                    }
                    });

                if ((_EpochLimit > 0) && (_EpochLimit < std::numeric_limits<long long>::max())) {
                    while (_delete_list.try_pop(out)) {
                        if (out.epoch < _EpochLimit) { // deemed safe to delete
                            _alloc.Free(out.ptr);
                        }
                        else { // deemed unsafe to delete just yet
                            _delete_list.push(out);
                            break;
                        }
                    }
                }
            }
        };

    public:
        using GuardType = typename TLS::EpochGuard;

        atomic_epoch_allocator()
            : _alloc{}
            , _delete_list{}
            , _TLS{}
            , _lastGC{ GL::util::get_current_epoch() }
        {};
        atomic_epoch_allocator(atomic_epoch_allocator const&) = delete;
        atomic_epoch_allocator(atomic_epoch_allocator&&) = delete;
        atomic_epoch_allocator& operator=(atomic_epoch_allocator const&) = delete;
        atomic_epoch_allocator& operator=(atomic_epoch_allocator&&) = delete;
        ~atomic_epoch_allocator() = default;

        void unsafe_unload() {
            _alloc.unsafe_unload();
        };

    public:
        GuardType ProtectCurrentEpoch() const {
            return TLS::EpochGuard(
                const_cast<atomic_epoch_allocator*>(this),
                const_cast<TLS*>(&*_TLS),
                GL::util::get_current_epoch()
            );
        };
        void ProtectCurrentEpoch_Fast() const {
            if (const_cast<TLS*>(&*_TLS)->_scope_count == 0) {
                const_cast<TLS*>(&*_TLS)->ForwardEpoch(GL::util::get_current_epoch());
            }
        };

        // Request a new memory pointer
        template <typename... TArgs> _type_* Alloc(TArgs &&... a) {
            return _alloc.Alloc(std::forward<TArgs>(a)...);
        };

        // Frees the memory pointer
        void Free(const _type_* element) {
            _delete_list.push({ GL::util::get_current_epoch(), const_cast<_type_*>(element) });
            if (_TLS->EpochCheck(GL::util::get_current_epoch())) {
                // will only succeed if we are in scope-level 0, which only happens if this thread has not made any protecting guards.
                RunGC();
            }
        };

    };

};

// Atomic Maps
namespace GL {
    // Thread-safe ordered B-Tree, which guarrantees valid and safe access to
    // pointers during erasure or modification of the tree when using the Epoch-guard
    // protection, which will delay actual deletion until the guard is satisfactorily old.
    template< class objType, class keyType, int maxChildrenPerNode = 10> class atomic_btree {
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
        deferred< atomic_epoch_allocator<objType> >
            objAllocator;
        atomic_epoch_allocator<_iterType>
            nodeAllocator;
        mutable std::shared_mutex
            mutex;

        class EpochGuard {
        private:
            typename atomic_epoch_allocator<objType>::GuardType guard_1;
            typename atomic_epoch_allocator<_iterType>::GuardType guard_2;

        public:
            EpochGuard(atomic_btree const* parent) : guard_1{ parent->objAllocator->ProtectCurrentEpoch() }, guard_2{ parent->nodeAllocator.ProtectCurrentEpoch() } {};
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

        atomic_btree()
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
        atomic_btree(atomic_btree const&) = delete;
        atomic_btree(atomic_btree&& rhs) = delete;
        atomic_btree& operator=(atomic_btree const&) = delete;
        atomic_btree& operator=(atomic_btree&&) = delete;
        ~atomic_btree() = default;

        void unsafe_unload() {
            if (objAllocator) objAllocator->unsafe_unload();
            nodeAllocator.unsafe_unload();
            root = first = last = nullptr;
            Num = 0;
        };

        template <bool EmplaceIfExists = true> _iterType*
            Add(objType object, keyType const& key) {
            _iterType
                * node,
                * child,
                * newNode;

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
                * child,
                * newNode;

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
        deferred<atomic_btree<ValueType, KeyType>>
            tree;

    public:
        class WrappedReference {
        private:
            typename atomic_btree<ValueType, KeyType>::GuardType
                guard;

        public:
            const KeyType&
                first;
            ValueType&
                second;

            // WrappedReference() = delete;
            WrappedReference(const KeyType& _first, ValueType& _second, atomic_btree<ValueType, KeyType>* _parent)
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
            using difference_type = ptrdiff_t;

            // data
            mutable typename atomic_btree<ValueType, KeyType>::_iterType*
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
        iterator // returns an iterator to an exact match
            find(const KeyType& _Keyval) const {
            auto g{ tree->ProtectCurrentEpoch() };
            auto iter = this->end();
            if (auto* p = this->tree->NodeFind(_Keyval)) {
                iter.state._ptr = p;
            }
            return iter;
        };
        iterator // returns an iterator to the nearest match that is smaller or equal to the requested value
            find_less_or_equal(const KeyType& _Keyval) const {
            auto g{ tree->ProtectCurrentEpoch() };
            auto iter = this->end();
            if (auto* p = this->tree->NodeFindLargestSmallerEqual(_Keyval)) {
                iter.state._ptr = p;
            }
            return iter;
        }
        iterator // returns an iterator to the nearest match that is larger or equal to the requested value
            find_larger_or_equal(const KeyType& _Keyval) const {
            auto g{ tree->ProtectCurrentEpoch() };
            auto iter = this->end();
            if (auto* p = this->tree->NodeFindSmallestLargerEqual(_Keyval)) {
                iter.state._ptr = p;
            }
            return iter;
        }

    };

    // fast, thread-safe sorted map, which sorts on key hash values rather than keys themselves. Allows simultaneous reading / writing / erasure. Slower than concurrent_unordered_map when erasure is not necessary. 
    template<class KeyType, class ValueType, typename HashType = std::hash<KeyType>> class atomic_hash_map {
        friend class it_state;
    protected:
        std::unique_ptr< deferred<atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>> >
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
            typename  atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>::GuardType
                guard;

        public:
            const KeyType&
                first;
            ValueType&
                second;

            WrappedReference(const KeyType& _first, ValueType& _second, atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>* _parent)
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
        atomic_hash_map()
            : tree{ std::make_unique< deferred<atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>>>() }, hasher{ HashType{} }
        {};
        atomic_hash_map(atomic_hash_map const& rhs) = delete;
        atomic_hash_map(atomic_hash_map&& rhs) : tree{ std::move(rhs.tree) }, hasher{ HashType{} }
        {};
        atomic_hash_map& operator=(atomic_hash_map const& rhs) = delete;
        atomic_hash_map& operator=(atomic_hash_map&& rhs) = delete;
        ~atomic_hash_map() = default;

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
            using thisType = atomic_hash_map;
            using value_type = WrappedReference;
            using iterator_category = std::forward_iterator_tag;
            using difference_type = typename std::iterator<iterator_category, value_type>::difference_type;

            // data
            mutable typename  atomic_btree<std::pair<KeyType, std::shared_ptr<ValueType>>, size_t>::_iterType*
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
        SETUP_ITERATOR(atomic_hash_map, it_state);
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

};