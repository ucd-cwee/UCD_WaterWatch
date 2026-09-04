#pragma once
#include "atomic_epoch_allocator.h"

namespace GL {
    // Multi-threaded version of a B-Tree that uses a course-grained lock with parallel allocator to make it thread-safe. Nodes are at-risk of disposal once the lock is returned.
    // Attempts to speed-up searching using a binomial search within BTree nodes. In theory should benefit from larger maxChildrenPerNode values. 
    template< class objType, class keyType, int maxChildrenPerNode = 10>
    class epoch_btree {
    private:
        fast_atomic_allocator< objType >
            objAllocator;

    public:
        using lock_type = GL::fast_shared_mutex; // std::shared_mutex; // fast_shared_mutex; //  
        class epoch_btreeNode {
        public:
            epoch_btreeNode() = default;
            epoch_btreeNode(epoch_btreeNode const&) = delete;
            epoch_btreeNode(epoch_btreeNode&&) = delete;
            epoch_btreeNode& operator=(epoch_btreeNode const&) = delete;
            epoch_btreeNode& operator=(epoch_btreeNode&&) = delete;
            __declspec(noinline) ~epoch_btreeNode() noexcept {
                if (is_leaf()) {
                    father->objAllocator.Free(ptr);
                    ptr = nullptr;
                }
            };

            objType
                * ptr{ nullptr };
            std::array<epoch_btreeNode*, maxChildrenPerNode>
                data;
            epoch_btreeNode // parent node
                * parent{ nullptr };
            epoch_btree
                * father{ nullptr };
            keyType	// key used for sorting						
                key;
            int	// number of children							
                numChildren{ 0 };
            int
                parent_index{ 0 };

            bool
                is_leaf() const {
                return ptr;
            };
            template <typename... Args>
            void instantiate_object(Args&&... args) {
                ptr = father->objAllocator.Alloc(std::move(args)...);
            };
            objType* const&
                object() {
                return ptr;
            };
            epoch_btreeNode**
                children() {
                return &data[0];
            };

            epoch_btreeNode* // next sibling
                next() {
                if (parent && (parent->numChildren > (parent_index + 1))) {
                    return parent->children()[parent_index + 1];
                }
                else {
                    return nullptr;
                }
            };
            epoch_btreeNode* // prev sibling
                prev() {
                if (parent && (parent_index >= 1)) {
                    return parent->children()[parent_index - 1];
                }
                else {
                    return nullptr;
                }
            };
            epoch_btreeNode* // first child
                firstChild() {
                if (numChildren == 0) return nullptr;
                return children()[0];
            };
            epoch_btreeNode* // last child
                lastChild() {
                if (numChildren == 0) return nullptr;
                return children()[numChildren - 1];
            };
            void
                add_child(epoch_btreeNode* p) {
                p->parent = this;
                p->parent_index = numChildren;
                children()[numChildren] = p;
                ++numChildren;
                if (this->key < p->key) this->key = p->key;
            }
            __declspec(noinline) void
                add_child_at(epoch_btreeNode* p, int i) {
                if (i >= numChildren) add_child(p);
                else {
                    epoch_btreeNode**
                        ch = children();
                    int
                        j;

                    if (this->key < p->key) this->key = p->key;
                    p->parent = this;
                    p->parent_index = i;
                    // shift everything forward

#if 1
                    for (j = numChildren; j > i; --j) {
                        ch[j] = ch[j - 1];
                        ch[j]->parent_index = j;
                    }
                    ch[i] = p;
#else
                    std::memmove(&ch[i + 1], &ch[i], sizeof(epoch_btreeNode*) * (size_t)(numChildren - i));
                    ch[i] = p;
                    ch = &ch[i];
                    for (j = i + 1; j <= numChildren; ++j) {
                        ++ch;
                        (*ch)->parent_index = j;
                    }
#endif
                    ++numChildren;
                }
            }
            epoch_btreeNode*
                pop_front_child() {
                if (numChildren <= 0) {
                    return nullptr;
                }
                else {
                    auto* out = children()[0];
                    int i = 0;
                    for (i = 0; i < (numChildren - 1); ++i) {
                        children()[i] = children()[i + 1];
                        children()[i]->parent_index = i;
                    }
                    children()[i] = nullptr;
                    --numChildren;
                    return out;
                }
            }
            void
                pop_front_children(int n) {
                if (numChildren >= n) {
                    int i = 0;
                    for (i = 0; i < (numChildren - n); ++i) {
                        children()[i] = children()[i + n];
                        children()[i]->parent_index = i;
                    }
                    for (; i < maxChildrenPerNode; ++i) {
                        children()[i] = nullptr;
                    }
                    numChildren -= n;
                }
            }
            epoch_btreeNode*
                pop_child(int i) {
                if (numChildren <= 0) {
                    return nullptr;
                }
                else {
                    epoch_btreeNode**
                        ch = children();
                    epoch_btreeNode*
                        out = ch[i];
                    int
                        j = numChildren - 1;

#if 1
                    for (; i < (numChildren - 1); ++i) {
                        ch[i] = ch[i + 1];
                        ch[i]->parent_index = i;
                    }
                    ch[i] = nullptr;
                    --numChildren;
                    if (numChildren > 0) this->key = ch[numChildren - 1]->key;
#else
                    std::memmove(&ch[i], &ch[i + 1], sizeof(epoch_btreeNode*) * (size_t)((numChildren - i) - 1));
                    if (numChildren > 1) this->key = ch[numChildren - 2]->key;
                    ch[j] = nullptr;
                    ch = &ch[i];
                    for (; (i < j) && ch; ++i) {
                        (*ch)->parent_index = i;
                        ++ch;
                    }
                    --numChildren;
#endif
                    return out;
                }
            }
            epoch_btreeNode*
                pop_back_child() {
                if (numChildren <= 0) {
                    return nullptr;
                }
                else {
                    auto* out = children()[numChildren - 1];
                    children()[numChildren - 1] = nullptr;
                    --numChildren;
                    if (numChildren > 0) this->key = children()[numChildren - 1]->key;
                    return out;
                }
            };
            __declspec(noinline) epoch_btreeNode*
                binomial_search_smallest_greater_equal_to(keyType const& K) {
#if 0
                epoch_btreeNode* child = this->firstChild();
                for (; child->next(); child = child->next()) {
                    if (K <= child->key)
                        break;
                }
                return child;
#else
                //if (K >= children()[numChildren - 1]->key) {
                //	// it will be one of the final children
                //	for (int i = numChildren - 2; i >= 0; --i) {
                //		if (children()[i]->key < K) return children()[i + 1];
                //	}
                //	// worst-case we searched them all...
                //	return children()[0];
                //}
                //else {
                int
                    len,
                    mid,
                    offset;
                bool
                    res;
                epoch_btreeNode
                    * sample;
                epoch_btreeNode
                    ** childrens;
                if (numChildren == 0)
                    return nullptr;
                if (numChildren == 1)
                    return this->children()[0];

                if (numChildren >= maxChildrenPerNode) {
                    // std::cout << "Something went wrong 1...\n";
                    // throw std::runtime_error(std::to_string(numChildren) + " - Bad index");
                }

                childrens = this->children();
                len = numChildren;
                mid = len;
                offset = 0;
                res = false;

                while (mid > 0) {
                    mid = len >> 1;
                    if (((offset + mid) < 0) || ((offset + mid) >= maxChildrenPerNode)) {
                        // std::cout << "Something went wrong 2...\n";
                        // return binomial_search_smallest_greater_equal_to(K);
                        return nullptr;
                        // throw std::runtime_error(std::to_string(offset + mid) + " - Bad index");
                    }
                    sample = childrens[std::min<int>(numChildren - 1, offset + mid)];
                    if (!sample) {
                        // std::cout << "Something went wrong 3...\n";
                        // return binomial_search_smallest_greater_equal_to(K);
                        return nullptr;
                    }
                    if (K >= sample->key) {
                        offset += mid;
                        len -= mid;
                        res = true;
                        if (K == sample->key) return sample;
                    }
                    else {
                        len -= mid;
                        res = false;
                    }
                }
                mid = offset + (int)res;
                if (mid == numChildren) return childrens[std::min<int>(numChildren - 1, offset)];
                else return childrens[std::min<int>(numChildren - 1, mid)];
                //}
#endif
            };

        };

    private:
        mutable lock_type
            mut; // global tree lock. Should only be held temporarily if at all possible. 
        epoch_btreeNode*
            root;
        epoch_btreeNode*
            first;
        epoch_btreeNode*
            last;
        fast_atomic_epoch_allocator< epoch_btreeNode >
            nodeAllocator;
        long
            count;
    public:
        class // exclusive lock manager. Since this is a course-grained type, though, it can only ever hold one lock at a time. 
            locker {
        public:
            lock_type*
                locked;
            bool
                hard_locked;

            locker() : locked{ nullptr }, hard_locked{ false } {};
            locker(locker const&) = delete;
            locker(locker&& rhs) noexcept : locked{ rhs.locked }, hard_locked{ rhs.hard_locked } { rhs.locked = nullptr; };
            locker& operator=(locker const&) = delete;
            locker& operator=(locker&& rhs) noexcept {
                clear();
                locked = rhs.locked;
                hard_locked = rhs.hard_locked;
                rhs.locked = nullptr;
                return *this;
            };
            ~locker() {
                clear();
            };
            operator bool() const {
                return locked;
            };

        public:
            __declspec(noinline) bool // store a shared lock
                try_push_back(lock_type& source) {
                clear();
                locked = &source;
                if (locked->try_lock()) {
                    hard_locked = true;
                    return true;
                }
                else {
                    hard_locked = false;
                    locked = nullptr;
                    return false;
                }
            };
            void // store a shared lock
                push_back(lock_type& source) {
                clear();
                locked = &source;
                locked->lock();
                hard_locked = true;
            };
            void // store a shared lock
                push_back_shared(lock_type& source) {
                clear();
                locked = &source;
                locked->lock_shared();
                hard_locked = false;
            };
            __declspec(noinline) bool // store a shared lock
                try_push_back_shared(lock_type& source) {
                clear();
                locked = &source;
                if (locked->try_lock_shared()) {
                    hard_locked = false;
                    return true;
                }
                else {
                    hard_locked = false;
                    locked = nullptr;
                    return false;
                }
            };
            void // store a shared lock
                push_pop(lock_type& source) {
                source.lock();
                clear();
                locked = &source;
                hard_locked = true;
            };
            void // store a shared lock
                push_pop_shared(lock_type& source) {
                source.lock_shared();
                clear();
                locked = &source;
                hard_locked = false;
            };
            void // remove the youngest lock
                pop_back() {
                clear();
            };
            void // remove the oldest lock
                pop_front() {
                clear();
            };
            size_t // count of locks
                size() const {
                return (locked) ? 1 : 0;
            };
            void // clear all locks
                clear() {
                if (locked) {
                    if (hard_locked) locked->unlock();
                    else locked->unlock_shared();
                    locked = nullptr;
                }
            };
        };
    public:
        class EpochGuard {
        private:
            typename typename decltype(nodeAllocator)::GuardType guard_1;

        public:
            EpochGuard(epoch_btree* parent) : guard_1{ parent->nodeAllocator.guard_critical_section() } {};
            EpochGuard(EpochGuard const&) = delete;
            EpochGuard(EpochGuard&& rhs) = delete;
            EpochGuard& operator=(EpochGuard const&) = delete;
            EpochGuard& operator=(EpochGuard&&) = delete;
            ~EpochGuard() = default;
        };

        using GuardType = typename EpochGuard;
        [[nodiscard]] GuardType guard_critical_section() const {
            return EpochGuard(const_cast<epoch_btree*>(this));
        };

        epoch_btree()
            : objAllocator()
            , nodeAllocator()
            , root{ nullptr }
            , first{ nullptr }
            , last{ nullptr }
            , mut()
            , count{ 0 }
        {
            root = AllocNode(false);
        };
        epoch_btree(epoch_btree const&)
            = delete;
        epoch_btree(epoch_btree&&) noexcept
            = delete;
        epoch_btree& operator=(epoch_btree const&)
            = delete;
        epoch_btree& operator=(epoch_btree&&) noexcept
            = delete;
        ~epoch_btree() = default;

        epoch_btreeNode* // add an object to the tree
            GetOrInstance(keyType const& key) {
            if (auto [try_found, locked] = NodeFindSmallestLargerEqual(key, false); try_found) {
                return try_found;
            }
            else {
                epoch_btreeNode
                    * node,
                    * child,
                    * newNode;
                locker
                    locking;
                newNode
                    = AllocNode(true);
                newNode->key
                    = key;
                // newNode->instantiate_object(const_cast<epoch_btree*>(this));

                if (!locking) {
                    locking.push_back(mut); // locked
                }

                if (root == nullptr) root = AllocNode(false); // start fresh
                if (root == nullptr) throw std::runtime_error("Root should not have been nullptr");

                if (root->numChildren >= maxChildrenPerNode) { // make a new root and split
                    node = AllocNode(false);
                    node->key = root->key;
                    node->add_child(root);
                    SplitNode(root);
                    root = node;
                    node = nullptr;
                }

                for (node = root; node->numChildren > 0; node = child) {
                    if (key > node->key) node->key = key; // in prep for the insertion

                    // find the first child with a key larger equal to the key of the new node
                    child = node->binomial_search_smallest_greater_equal_to(key);

                    // we are inside of a branch of leafs -- we will do the insert.
                    if (child->object()) {
                        if (key <= child->key) {
                            if (key == child->key) {
                                // *child->object() = std::move(*newNode->object());
                                FreeNode(newNode);
                                return child;
                            }

                            // insert new node before child
                            newNode->instantiate_object();
                            node->add_child_at(newNode, child->parent_index);
                        }
                        else {
                            // insert new node after child
                            newNode->instantiate_object();
                            node->add_child_at(newNode, child->parent_index + 1);
                        }

                        if (!first || (first->key > newNode->key)) first = newNode;
                        if (!last || (last->key < newNode->key)) last = newNode;

                        ++count;
                        return newNode;
                    }
                    else if (child->numChildren >= maxChildrenPerNode) {
                        SplitNode(child);
                        if (key <= child->prev()->key) child = child->prev();
                    }
                }

                // we only end up here if the root node is empty
                newNode->instantiate_object();
                root->add_child(newNode);

                if (!first || (first->key > newNode->key)) first = newNode;
                if (!last || (last->key < newNode->key)) last = newNode;

                ++count;
                return newNode;
            }
        };
        objType& operator[](keyType const& key) {
            return *GetOrInstance(key)->object();
        };

        __declspec(noinline) epoch_btreeNode* // add an object to the tree
            Add(objType&& object, keyType const& key, locker const& Locking = locker(), bool unique = true) {
            epoch_btreeNode
                * node,
                * child,
                * newNode;
            locker&
                locking = const_cast<locker&>(Locking);
            newNode
                = AllocNode(true);
            newNode->key
                = key;
            newNode->instantiate_object(std::move(object));

            if (!locking) {
                locking.push_back(mut); // locked
            }

            if (root == nullptr) root = AllocNode(false); // start fresh
            if (root == nullptr) throw std::runtime_error("Root should not have been nullptr");

            if (root->numChildren >= maxChildrenPerNode) { // make a new root and split
                node = AllocNode(false);
                node->key = root->key;
                node->add_child(root);
                SplitNode(root);
                root = node;
                node = nullptr;
            }

            for (node = root; node->numChildren > 0; node = child) {
                if (key > node->key) node->key = key; // in prep for the insertion

                // find the first child with a key larger equal to the key of the new node
                child = node->binomial_search_smallest_greater_equal_to(key);

                // we are inside of a branch of leafs -- we will do the insert.
                if (child->object()) {
                    if (key <= child->key) {
                        if (unique && (key == child->key)) {
                            // *child->object() = std::move(*newNode->object());
                            FreeNode(newNode);
                            return child;
                        }

                        // insert new node before child
                        node->add_child_at(newNode, child->parent_index);
                    }
                    else {
                        // insert new node after child
                        node->add_child_at(newNode, child->parent_index + 1);
                    }

                    if (!first || (first->key > newNode->key)) first = newNode;
                    if (!last || (last->key < newNode->key)) last = newNode;

                    ++count;
                    return newNode;
                }
                else if (child->numChildren >= maxChildrenPerNode) {
                    SplitNode(child);
                    if (key <= child->prev()->key) child = child->prev();
                }
            }

            // we only end up here if the root node is empty
            root->add_child(newNode);

            if (!first || (first->key > newNode->key)) first = newNode;
            if (!last || (last->key < newNode->key)) last = newNode;

            ++count;
            return newNode;

        };
        bool // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
            Remove(epoch_btreeNode* node, locker const& Locking = locker()) {
            epoch_btreeNode
                * Node,
                * parent,
                * oldRoot;
            locker&
                locking = const_cast<locker&>(Locking);

            // acquire all relevant locks before we perform the deletion
            if (locking.size() == 0) locking.push_back(mut); // get the global tree lock		

            if (first == node) first = GetNextLeaf(node, locking);
            if (last == node) last = GetPrevLeaf(node, locking);

            // unlink the node from it's parent
            parent = node->parent;
            parent->pop_child(node->parent_index);

            // make sure there are no parent nodes with a single child
            for (; (parent != root) && (parent->numChildren <= 1); parent = parent->parent) {
                while (true) {
                    if (Node = parent->next()) {
                        if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
                            SplitNode(Node);
                            continue;
                        }
                        parent = MergeNodes(parent, Node);
                        break;
                    }
                    else if (Node = parent->prev()) {
                        if ((parent->numChildren + Node->numChildren) > maxChildrenPerNode) {
                            SplitNode(Node);
                            continue;
                        }
                        parent = MergeNodes(Node, parent);
                        break;
                    }
                }

                if (parent->numChildren > maxChildrenPerNode) {
                    SplitNode(parent);
                    break;
                }
            }

            // a parent may not use a key higher than the key of it's last child. Work backwards and make sure this is true. 
            for (; parent && (parent->numChildren > 0); parent = parent->parent)
                if (Node = parent->children()[parent->numChildren - 1])
                    if (parent->key > Node->key)
                        parent->key = Node->key;

            // actually free the node
            --count;
            FreeNode(node);

            // remove the root node if it has a single internal node as child		
            if ((root->numChildren == 1) && !root->firstChild()->object()) {
                oldRoot = root;

                root = oldRoot->firstChild();
                root->parent = nullptr;
                root->parent_index = 0;

                FreeNode(oldRoot);
            }

            return true;
        };
        std::pair<epoch_btreeNode*, locker> // find an object using the given key
            NodeFind(keyType const& key, bool for_removal = false) {
            std::pair<epoch_btreeNode*, locker>
                out;
            epoch_btreeNode*&
                node = out.first;
            locker&
                locking = out.second;

            if (!for_removal) locking.push_back_shared(mut);
            else locking.push_back(mut);
            if (!root || (root->numChildren <= 0)) {
                node = nullptr;
                locking.clear();
                return out;
            }

            for (node = root; node; ) {
                node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                if (node && node->object()) {
                    if (node->key == key) return out;
                    else {
                        node = nullptr;
                        locking.clear();
                        return out;
                    }
                }
                if (!node || (node->numChildren <= 0)) {
                    node = nullptr;
                    locking.clear();
                    return out;
                }
            }
            node = nullptr;
            locking.clear();
            return out;
        };
        std::pair<epoch_btreeNode*, locker> // find an object using the given key
            NodeFind_ForRemoval(keyType const& key) {
            return NodeFind(key, true);
        };
        locker
            try_lock() {
            locker out;
            out.try_push_back(mut);
            return out;
        };
        locker
            lock(bool do_hard_lock = true) {
            locker out;
            if (do_hard_lock) out.push_back(mut);
            else out.push_back_shared(mut);
            return out;
        };
        locker
            lock_shared() {
            locker out;
            out.push_back_shared(mut);
            return out;
        };
        locker
            try_lock_shared() {
            locker out;
            out.try_push_back_shared(mut);
            return out;
        };

        std::pair<epoch_btreeNode*, locker> // find an object with the smallest key larger equal the given key
            NodeFindSmallestLargerEqual(keyType const& key, bool for_removal = false) {
            std::pair<epoch_btreeNode*, locker>
                out;
            epoch_btreeNode*&
                node = out.first;
            locker&
                locking = out.second;

            if (!for_removal) locking.push_back_shared(mut);
            else locking.push_back(mut);
            if (!root || (root->numChildren <= 0)) {
                node = nullptr;
                locking.clear();
                return out;
            }
            for (node = root; node; ) {
                node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                if (node && node->object()) {
                    if (node->key >= key) return out;
                    else {
                        node = nullptr;
                        locking.clear();
                        return out;
                    }
                }
                if (!node || (node->numChildren <= 0)) {
                    node = nullptr;
                    locking.clear();
                    return out;
                }
            }
            node = nullptr;
            locking.clear();
            return out;
        };
        epoch_btreeNode* // find an object with the smallest key larger equal the given key
            NodeFindSmallestLargerEqual_Locked(keyType const& key, locker const& locked) {
            epoch_btreeNode*
                node = nullptr;

            if (!root || (root->numChildren <= 0)) {
                node = nullptr;
                return node;
            }
            for (node = root; node; ) {
                node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                if (node && node->object()) {
                    if (node->key >= key) return node;
                    else {
                        node = nullptr;
                        return node;
                    }
                }
                if (!node || (node->numChildren <= 0)) {
                    node = nullptr;
                    return node;
                }
            }
            node = nullptr;
            return node;
        };
        std::pair<epoch_btreeNode*, locker> // find an object with the smallest key larger equal the given key
            NodeFindSmallestLargerEqual_ForRemoval(keyType const& key) {
            return NodeFindSmallestLargerEqual(key, true);
        }

#if 0
        std::pair<epoch_btreeNode*, locker> // find an object with the largest key smaller equal the given key
            NodeFindLargestSmallerEqual(keyType key) {
            epoch_btreeNode
                * smaller;
            std::pair<epoch_btreeNode*, locker>
                out;
            epoch_btreeNode*&
                node = out.first;
            locker&
                locking = out.second;

            locking.push_back(mut);
            if (!root || !root->firstChild) {
                node = nullptr;
                locking.clear();
                return out;
            }

            for (node = root->firstChild, smaller = nullptr; node; ) {
                while (node->next) {
                    if (node->key >= key) break;
                    smaller = node;
                    node = node->next;
                }
                if (node->object) {
                    if (node->key <= key) return node;
                    else if (smaller == nullptr) {
                        node = nullptr;
                        locking.clear();
                        return out;
                    }
                    else {
                        node = smaller;
                        if (node->object) return out;
                    }
                }

                if (!node || !node->firstChild) {
                    node = nullptr;
                    locking.clear();
                    return out;
                }

                node = node->firstChild;
            }
            node = nullptr;
            locking.clear();
            return out;
        };
#endif
        std::pair<epoch_btreeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
            GetRoot() {
            std::pair<epoch_btreeNode*, locker> out;
            out.second.push_back(mut);
            out.first = root;
            return out;
        };
        std::pair<epoch_btreeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
            GetRootShared() {
            std::pair<epoch_btreeNode*, locker> out;
            out.second.push_back_shared(mut);
            out.first = root;
            return out;
        };
        epoch_btreeNode* // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
            GetRoot(locker const& locked) {
            return root;
        };

#if 0
        static epoch_btreeNode* // goes through all nodes of the tree		
            GetNext(epoch_btreeNode* node, locker& locking) {
            if (node->firstChild) {
                return node->firstChild;
            }
            else {
                while (node && (node->next == nullptr)) {
                    node = node->parent;
                }
                return node;
            }

        };
#endif

        static epoch_btreeNode* // goes through all leaf nodes of the tree		
            GetNextLeaf(epoch_btreeNode* node, locker& locking) {
            if (!node) return nullptr;

            epoch_btreeNode*
                nxt;
            if (nxt = node->firstChild()) {
                while (nxt) {
                    node = nxt;
                    nxt = node->firstChild();
                }
                return node;
            }
            else {
                while (node && (node->next() == nullptr)) {
                    nxt = node->parent;
                    node = nxt;
                }
                if (node) {
                    nxt = node->next();
                    node = nxt;
                    nxt = node->firstChild();
                    while (nxt) {
                        node = nxt;
                        nxt = node->firstChild();
                    }
                    return node;
                }
                else return nullptr;
            }
        };
        static epoch_btreeNode*
            GetPrevLeaf(epoch_btreeNode* node, locker& locking) {
            if (!node) return nullptr;
            epoch_btreeNode*
                prev;
            if (node->lastChild()) {
                while (true) {
                    if (prev = node->lastChild(); prev) {
                        node = prev;
                    }
                    else {
                        break;
                    }
                }
                return node;
            }
            else {
                while (node && (node->prev() == nullptr)) {
                    node = node->parent;
                }
                if (node) {
                    node = node->prev();
                    while (true) {
                        if (prev = node->lastChild(); prev) {
                            node = prev;
                        }
                        else {
                            break;
                        }
                    }
                    return node;
                }
                else {
                    return nullptr;
                }
            }
        };	// goes through all leaf nodes of the tree;

        size_t
            size() const {
            auto locked{ std::shared_lock(mut) };
            return (size_t)count;
        };
        void
            clear() {
            while (true) {
                auto [node, locked] = this->GetRoot();
                if (!node) { break; }
                node = GetNextLeaf(node, locked);
                if (!node) { break; }
                Remove(node, locked);
            }
        };
        template <typename Func> epoch_btreeNode* // same as operator[], except it will call the provided function to initialize the value if no value was found. 
            get_or_make(const keyType& time, Func const& func, bool* ExistedAlready = nullptr) {
            auto g{ guard_critical_section() };
            if (auto [node, locked] = NodeFind(time, false); node) {
                if (ExistedAlready) *ExistedAlready = true;
                return node;
            }
            if (ExistedAlready) *ExistedAlready = false;
            return this->Add(func(), time);
        };
        template <typename Func> __declspec(noinline) bool // removes the first (smallest key) node in the map if func(key, object) returns true
            pop_front_if(Func const& func) {
            auto g{ guard_critical_section() };

            if (1) {
                if (epoch_btreeNode* node = first; node) {
                    if (func(node->key, *node->object())) {
                        // need to do removal
                    }
                    else {
                        return false;
                    }
                }

            }

            if (1) {
                auto locked = lock();
                if (epoch_btreeNode* node = first; node) {
                    if (func(node->key, *node->object())) {
                        Remove(node, locked);
                        return true;
                    }
                    else {
                        return false;
                    }
                }
            }

            return false;
        };
        template <typename Func> bool // calls func(key, object) on the last (largest key) node in the map
            do_at_end(Func const& func) {
            auto g{ guard_critical_section() };
            if (auto* L = last; L) {
                func(L->key, *L->object());
                return true;
            }
            else {
                return false;
            }

            //auto locked = lock_shared();
            //if (last) {
            //	func(last->key, *last->object());
            //	return true;
            //}
            //else {
            //	return false;
            //}
        };
    private:
        epoch_btreeNode*
            AllocNode(bool is_leaf) {
            epoch_btreeNode
                * node;

            node = nodeAllocator.Alloc();
            if (is_leaf) {

            }
            else {
                for (int i = 0; i < maxChildrenPerNode; ++i) node->children()[i] = nullptr;
            }

            node->key = {};
            node->parent = nullptr;
            node->father = this;
            node->parent_index = 0;
            node->numChildren = 0;

            return node;
        };
        __declspec(noinline) void
            FreeNode(epoch_btreeNode* node) {
            if (node) {
                nodeAllocator.Free(node);
            }
        };
        void // will split node by creating a neighbor next to it in the parent node and sharing half its children
            SplitNode(epoch_btreeNode* node) {
            int
                i, j;
            epoch_btreeNode
                * child,
                * newNode;

            // allocate a new node
            newNode = AllocNode(false);
            newNode->parent = node->parent;

            // divide the children over the two nodes
            child = node->firstChild();
            newNode->children()[0] = child;
            for (j = 1, i = 3; i < node->numChildren; i += 2, j++) {
                child = child->next();
                newNode->children()[j] = child;
            }
            newNode->key = child->key;
            newNode->numChildren = node->numChildren / 2;
            for (i = 0; i < newNode->numChildren; ++i) {
                newNode->children()[i]->parent = newNode;
                newNode->children()[i]->parent_index = i;
            }

            newNode->parent_index = node->parent_index;
            node->pop_front_children(newNode->numChildren);
            node->parent->add_child_at(newNode, newNode->parent_index);
            node->key = node->children()[node->numChildren - 1]->key;
        };;
        epoch_btreeNode* // node1 will be deleted and its children appended to node2
            MergeNodes(epoch_btreeNode* node1, epoch_btreeNode* node2) {
            for (int i = 0; i < node1->numChildren; ++i)
                node2->add_child_at(node1->children()[i], i);
            (void)node1->parent->pop_child(node1->parent_index);
            FreeNode(node1);
            return node2;
        };

    };

    template< class objType, class keyType>
    class epoch_btree_map {
    private:
        mutable epoch_btree<objType, keyType, 10>
            tree;
        using GuardType = typename decltype(tree)::GuardType;

    public:
        [[nodiscard]] GuardType guard_critical_section() const {
            return tree.guard_critical_section();
        };

        class WrappedReference {
        private:
            GuardType guard;

        public:
            const keyType&
                first;
            objType&
                second;

            WrappedReference(
                const keyType& _first,
                objType& _second,
                const epoch_btree_map* _parent)
                : first{ _first }
                , second{ _second }
                , guard{ _parent->guard_critical_section() }
            {};
            WrappedReference(WrappedReference const&) = delete;
            WrappedReference(WrappedReference&&) = delete;
            WrappedReference& operator=(WrappedReference const&) = delete;
            WrappedReference& operator=(WrappedReference&&) = delete;
            ~WrappedReference() = default;
        };
        class WrappedReferenceFast {
        public:
            const keyType&
                first;
            objType&
                second;

            WrappedReferenceFast(const keyType* _first, objType* _second)
                : first{ *_first }
                , second{ *_second }
            {};
            WrappedReferenceFast(WrappedReferenceFast const&) = delete;
            WrappedReferenceFast(WrappedReferenceFast&&) = delete;
            WrappedReferenceFast& operator=(WrappedReferenceFast const&) = delete;
            WrappedReferenceFast& operator=(WrappedReferenceFast&&) = delete;
            ~WrappedReferenceFast() = default;
        };

        epoch_btree_map() = default;
        epoch_btree_map(epoch_btree_map const& rhs) = delete;
        epoch_btree_map(epoch_btree_map&& rhs) = delete;
        epoch_btree_map& operator=(epoch_btree_map const& rhs) = delete;
        epoch_btree_map& operator=(epoch_btree_map&& rhs) = delete;
        ~epoch_btree_map() = default;

        WrappedReference
            insert(const keyType& time, objType&& value) {
            auto g = guard_critical_section();
            auto* node_ptr = tree.Add(std::move(value), time);
            return WrappedReference(node_ptr->key, *node_ptr->object(), this);
        };
        void
            insert_fast(const keyType& time, objType&& value) {
            (void)tree.Add(std::move(value), time);
        };
        objType& // throws if the key is not found. 
            at(const keyType& time) const {
            auto g{ guard_critical_section() };
            if (auto [node, locker] = tree.NodeFind(time); node) {
                return *node->object();
            }
            throw std::range_error("Could not find key");
        };
        objType* // returns nullptr if the key is not found. 
            try_at(const keyType& time) const {
            auto g{ guard_critical_section() };
            if (auto [node, locker] = tree.NodeFind(time); node) {
                return node->ptr; //  object();
            }
            return nullptr;
        };
        objType* // returns nullptr if the key is not found. 
            try_at_smallest_larger_or_equal(const keyType& time) const {
            auto g{ guard_critical_section() };
            if (auto [node, locker] = tree.NodeFindSmallestLargerEqual(time); node) return node->ptr;
            if (auto [node, locker] = tree.NodeFind_ForRemoval(time); node) return node->ptr;
            else {
                if constexpr (std::is_copy_constructible_v< objType > && std::is_constructible_v< objType >) {
                    if (node = tree.Add({}, time, locker); node)
                        return node->ptr;
                }
            }
            throw std::range_error("Could not find key");
            return nullptr;
        };
        objType& // if already exists, returns the value. Otherwise, creates the value (default init) and returns the value. May throw under heavy conflict. 
            operator[](const keyType& time) {
            auto g = guard_critical_section();
            if (auto [node, locker] = tree.NodeFind(time); node) return *node->object();
            if (auto [node, locker] = tree.NodeFind_ForRemoval(time); node) return *node->object();
            else {
                if constexpr (std::is_copy_constructible_v< objType > && std::is_constructible_v< objType >) {
                    if (node = tree.Add({}, time, locker); node)
                        return *node->object();
                }
            }
            throw std::range_error("Could not find key");
        };
        template <typename F>
        objType& // if already exists, returns the value. Otherwise, creates the value (using function) and returns the value. May throw under heavy conflict. 
            get_or_make(const keyType& time, F const& func) {
            auto g = guard_critical_section();
            if (auto [node, locker] = tree.NodeFind(time); node) return *node->object();
            //if (auto [node, locker] = tree.NodeFind_ForRemoval(time); node) return *node->object();
            //else {
            if constexpr (std::is_copy_constructible_v< objType > && std::is_constructible_v< objType >) {
                if (auto node = tree.Add(func(), time); node)
                    return *node->object();
            }
            //}
            throw std::range_error("Could not find key or not construct the new object");
        };
        bool // optionally get a copy of the object being deleted. 
            erase(const keyType& time, objType* out = nullptr) const {
            auto g = guard_critical_section();
            if (auto [node, locker] = tree.NodeFind(time, true); node) {
                if (out) *out = *node->object();
                return tree.Remove(node, locker);
            }
            return false;
        };
        void
            clear() {
            auto g = guard_critical_section();
            auto locked = tree.lock();
            while (true) {
                if (auto* p = tree.GetRoot(locked)) {
                    if (p = tree.GetNextLeaf(p, locked); p) {
                        tree.Remove(p, locked);
                    }
                    else {
                        break;
                    }
                }
                else {
                    break;
                }
            }

        };
        size_t
            size() const {
            return tree.size();
        };
        template <typename Func> bool // calls func(key, object) on the last (largest key) node in the map
            do_at_end(Func const& func) {
            auto g{ guard_critical_section() };
            return tree.do_at_end(func);
        };
        template <typename Func> bool // removes the first (smallest key) node in the map if func(key, object) returns true
            pop_front_if(Func const& func) {
            auto g{ guard_critical_section() };
            return tree.pop_front_if(func);            
        };
        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair< const keyType*, objType* >;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;
            using iter_type = typename decltype(tree)::epoch_btreeNode;
            using lock_type = typename decltype(tree)::locker;

            Iterator(epoch_btree_map* parent = nullptr, iter_type* ptr = nullptr) : _parent{ parent }, _ptr(ptr), _data(nullptr, nullptr), _lock((parent&& ptr) ? parent->tree.lock_shared() : lock_type{}) {
                if (_ptr) {
                    _data = { &_ptr->key, _ptr->object() };
                }
            };
            Iterator(const Iterator& rhs) : _parent(rhs._parent), _ptr(rhs._ptr), _data(nullptr, nullptr) {
                if (_ptr) {
                    _data = { &_ptr->key, _ptr->object() };
                }
            };

            inline reference operator*() { return _data; }
            inline pointer operator->() { return &_data; }
            inline const reference operator*() const { return _data; }
            inline const pointer operator->() const { return &_data; }

            inline Iterator& operator++() {
                _ptr = typename decltype(tree)::GetNextLeaf(_ptr, _lock);
                if (_ptr) {
                    _data = { &_ptr->key, _ptr->object() };
                }
                else {
                    _data = { nullptr, nullptr };
                }
                return *this;
            }
            inline Iterator operator++(int) { Iterator tmp(*this); this->operator++(); return tmp; }

            inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
            inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
            inline bool operator>(const Iterator& rhs) const {
                if (_ptr == rhs._ptr) return false;
                if (!_ptr) return false;
                if (!rhs._ptr) return true;
                return _ptr->key > rhs._ptr->key;
            };
            inline bool operator>=(const Iterator& rhs) const {
                if (_ptr == rhs._ptr) return true;
                if (!_ptr) return false;
                if (!rhs._ptr) return true;
                return _ptr->key >= rhs._ptr->key;
            };
            inline bool operator<(const Iterator& rhs) const { return !operator>=(rhs); };
            inline bool operator<=(const Iterator& rhs) const { return !operator>(rhs); };

        protected:
            std::pair< const keyType*, objType* >
                _data;
            iter_type*
                _ptr;
            epoch_btree_map*
                _parent;
            lock_type
                _lock;
        };

        using iterator = Iterator;
        using const_iterator = iterator;

        auto begin() {
            auto locked = this->tree.lock_shared();
            return Iterator(this, this->tree.GetNextLeaf(this->tree.GetRoot(locked), locked));
        };
        auto end() {
            return Iterator(nullptr, nullptr);
        };
        auto cbegin() const { return const_cast<epoch_btree_map*>(this)->begin(); };
        auto cend() const { return const_cast<epoch_btree_map*>(this)->end(); };
        auto begin() const { return const_cast<epoch_btree_map*>(this)->begin(); };
        auto end() const { return const_cast<epoch_btree_map*>(this)->end(); };

    };
};