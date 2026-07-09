#pragma once

#include "atomic_bag.h"
#include "atomic_epoch_allocator.h"
#include "epoch_map.h"
//#include "atomic_tree.h"

namespace GL {
    // non-atomic, non-thread-safe b_tree. 
    template< class objType, class keyType, int maxChildrenPerNode = 10 >
    class b_tree {
    public:
        class b_treeNode {
        public:
            keyType
                key;			// key used for sorting
            objType*
                object;			// if != NULL pointer to object stored in leaf node
            b_treeNode*
                parent;			// parent node
            b_treeNode*
                next;			// next sibling
            b_treeNode*
                prev;			// prev sibling
            int
                numChildren;	// number of children
            b_treeNode*
                firstChild;		// first child
            b_treeNode*
                lastChild;		// last child
        };

    public:
        b_tree(GL::fast_atomic_allocator<b_treeNode, 128>& allocator)
            : root{ nullptr }
            , nodeAllocator{ &allocator }
        {};
        b_tree()
            : root{ nullptr }
            , nodeAllocator{ nullptr }
        {};
        ~b_tree() {};

        __declspec(noinline) b_treeNode*
            Add(objType* object, keyType key) {
            b_treeNode
                *node, 
                *child, 
                *newNode;

            if (root == NULL) {
                root = AllocNode();
            }

            if (root->numChildren >= maxChildrenPerNode) {
                newNode = AllocNode();
                newNode->key = root->key;
                newNode->firstChild = root;
                newNode->lastChild = root;
                newNode->numChildren = 1;
                root->parent = newNode;
                SplitNode(root);
                root = newNode;
            }

            newNode = AllocNode();
            newNode->key = key;
            newNode->object = object;

            for (node = root; node->firstChild != NULL; node = child) {

                if (key > node->key) {
                    node->key = key;
                }

                // find the first child with a key larger equal to the key of the new node
                for (child = node->firstChild; child->next; child = child->next) {
                    if (key <= child->key) {
                        break;
                    }
                }

                if (child->numChildren == 0) {
                    if (key <= child->key) {
                        // insert new node before child
                        if (child->prev) {
                            child->prev->next = newNode;
                        }
                        else {
                            node->firstChild = newNode;
                        }
                        newNode->prev = child->prev;
                        newNode->next = child;
                        child->prev = newNode;
                    }
                    else {
                        // insert new node after child
                        if (child->next) {
                            child->next->prev = newNode;
                        }
                        else {
                            node->lastChild = newNode;
                        }
                        newNode->prev = child;
                        newNode->next = child->next;
                        child->next = newNode;
                    }

                    newNode->parent = node;
                    node->numChildren++;

                    return newNode;
                }

                // make sure the child has room to store another node
                if (child->numChildren >= maxChildrenPerNode) {
                    SplitNode(child);
                    if (key <= child->prev->key) {
                        child = child->prev;
                    }
                }
            }

            // we only end up here if the root node is empty
            newNode->parent = root;
            root->key = key;
            root->firstChild = newNode;
            root->lastChild = newNode;
            root->numChildren++;

            return newNode;
        };						// add an object to the tree
        __declspec(noinline) void
            Remove(b_treeNode* node) {
            b_treeNode* parent;

            // unlink the node from it's parent
            if (node->prev) {
                node->prev->next = node->next;
            }
            else {
                node->parent->firstChild = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
            else {
                node->parent->lastChild = node->prev;
            }
            node->parent->numChildren--;

            // make sure there are no parent nodes with a single child
            for (parent = node->parent; parent != root && parent->numChildren <= 1; parent = parent->parent) {

                if (parent->next) {
                    parent = MergeNodes(parent, parent->next);
                }
                else if (parent->prev) {
                    parent = MergeNodes(parent->prev, parent);
                }

                // a parent may not use a key higher than the key of it's last child
                if (parent->key > parent->lastChild->key) {
                    parent->key = parent->lastChild->key;
                }

                if (parent->numChildren > maxChildrenPerNode) {
                    SplitNode(parent);
                    break;
                }
            }
            for (; parent != NULL && parent->lastChild != NULL; parent = parent->parent) {
                // a parent may not use a key higher than the key of it's last child
                if (parent->key > parent->lastChild->key) {
                    parent->key = parent->lastChild->key;
                }
            }

            // free the node
            FreeNode(node);

            // remove the root node if it has a single internal node as child
            if ((root->numChildren == 1) && (root->firstChild->numChildren > 0)) {
                b_treeNode* oldRoot = root;
                root->firstChild->parent = NULL;
                root = root->firstChild;
                FreeNode(oldRoot);
            }

#ifdef BTREE_CHECK
            CheckTree();
#endif
        };				// remove an object node from the tree

        __declspec(noinline) b_treeNode*
            NodeFind(keyType key) const {
            b_treeNode* node;
            if (root) {
                for (node = root->firstChild; node != NULL; node = node->firstChild) {
                    while (node->next) {
                        if (node->key >= key) {
                            break;
                        }
                        node = node->next;
                    }
                    if (node->numChildren == 0) {
                        if (node->key == key) {
                            return node;
                        }
                        else {
                            return NULL;
                        }
                    }
                }
            }
            return NULL;
        };								// find an object using the given key
        __declspec(noinline) b_treeNode*
            NodeFindSmallestLargerEqual(keyType key) const {
            b_treeNode* node;
            if (root) {
                for (node = root->firstChild; node != NULL; node = node->firstChild) {
                    while (node->next) {
                        if (node->key >= key) {
                            break;
                        }
                        node = node->next;
                    }
                    if (node->numChildren == 0) {
                        if (node->key >= key) {
                            return node;
                        }
                        else {
                            return NULL;
                        }
                    }
                }
            }
            return NULL;
        };			// find an object with the smallest key larger equal the given key
        __declspec(noinline) b_treeNode*
            NodeFindLargestSmallerEqual(keyType key) const {
            b_treeNode* node;
            b_treeNode* smaller = NULL;
            if (root) {
                for (node = root->firstChild; node != NULL; node = node->firstChild) {
                    while (node->next) {
                        if (node->key >= key) {
                            break;
                        }
                        smaller = node;
                        node = node->next;
                    }
                    if (node->numChildren == 0) {
                        if (node->key <= key) {
                            return node;
                        }
                        else if (smaller == NULL) {
                            return NULL;
                        }
                        else {
                            node = smaller;
                            if (node->numChildren == 0) {
                                return node;
                            }
                        }
                    }
                }
            }
            return NULL;
        };			// find an object with the largest key smaller equal the given key

        __declspec(noinline) objType*
            Find(keyType key) const {
            b_treeNode* node = NodeFind(key);
            if (node == NULL) {
                return NULL;
            }
            else {
                return node->object;
            }
        };									// find an object using the given key
        __declspec(noinline) objType*
            FindSmallestLargerEqual(keyType key) const {
            b_treeNode* node = NodeFindSmallestLargerEqual(key);
            if (node == NULL) {
                return NULL;
            }
            else {
                return node->object;
            }
        };				// find an object with the smallest key larger equal the given key
        __declspec(noinline) objType*
            FindLargestSmallerEqual(keyType key) const {
            b_treeNode* node = NodeFindLargestSmallerEqual(key);
            if (node == NULL) {
                return NULL;
            }
            else {
                return node->object;
            }
        };				// find an object with the largest key smaller equal the given key

        __declspec(noinline) b_treeNode*
            GetRoot() const {
            return root;
        };											// returns the root node of the tree
        __declspec(noinline) int
            GetNodeCount() const {
            return nodeAllocator->GetAllocCount();
        };										// returns the total number of nodes in the tree
        __declspec(noinline) b_treeNode*
            GetNext(b_treeNode* node) const {
            if (node->firstChild) {
                return node->firstChild;
            }
            else {
                while (node && node->next == NULL) {
                    node = node->parent;
                }
                return node;
            }
        };		// goes through all nodes of the tree
        __declspec(noinline) b_treeNode*
            GetNextLeaf(b_treeNode* node) const {
            if (node->firstChild) {
                while (node->firstChild) {
                    node = node->firstChild;
                }
                return node;
            }
            else {
                while (node && node->next == NULL) {
                    node = node->parent;
                }
                if (node) {
                    node = node->next;
                    while (node->firstChild) {
                        node = node->firstChild;
                    }
                    return node;
                }
                else {
                    return NULL;
                }
            }
        };	// goes through all leaf nodes of the tree

    private:
        b_treeNode*
            root;
    public:
        GL::fast_atomic_allocator<b_treeNode, 128>*
            nodeAllocator;
    private:
        __declspec(noinline) b_treeNode*
            AllocNode() {
            b_treeNode* node = nodeAllocator->Alloc();
            node->key = 0;
            node->parent = NULL;
            node->next = NULL;
            node->prev = NULL;
            node->numChildren = 0;
            node->firstChild = NULL;
            node->lastChild = NULL;
            node->object = NULL;
            return node;
        };
        __declspec(noinline) void
            FreeNode(b_treeNode* node) {
            nodeAllocator->Free(node);
        };
        __declspec(noinline) void
            SplitNode(b_treeNode* node) {
            int i;
            b_treeNode* child, * newNode;

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

            child->next->prev = NULL;
            child->next = NULL;

            // add the new child to the parent before the split node
            if (node->prev) {
                node->prev->next = newNode;
            }
            else {
                node->parent->firstChild = newNode;
            }
            newNode->prev = node->prev;
            newNode->next = node;
            node->prev = newNode;

            node->parent->numChildren++;
        };
        __declspec(noinline) b_treeNode*
            MergeNodes(b_treeNode* node1, b_treeNode* node2) {
            b_treeNode* child;

            for (child = node1->firstChild; child->next; child = child->next) {
                child->parent = node2;
            }
            child->parent = node2;
            child->next = node2->firstChild;
            node2->firstChild->prev = child;
            node2->firstChild = node1->firstChild;
            node2->numChildren += node1->numChildren;

            // unlink the first node from the parent
            if (node1->prev) {
                node1->prev->next = node2;
            }
            else {
                node1->parent->firstChild = node2;
            }
            node2->prev = node1->prev;
            node2->parent->numChildren--;

            FreeNode(node1);

            return node2;
        };

    };

    // non-atomic, non-thread-safe allocator that can allocate arrays of items (e.g. 128 floats, 1024 strings, etc.).
    template<class type, int baseBlockSize = 1024 * sizeof(type)>
    class array_allocator {
    public:
        class dynamic_block {
        public:
            type*
                GetMemory() const { return (type*)(((::byte*)this) + sizeof(dynamic_block)); }
            int
                GetSize() const { return abs(size); }
            void
                SetSize(int s, bool isBaseBlock) { size = isBaseBlock ? -s : s; }
            bool
                IsBaseBlock() const { return (size < 0); }

            dynamic_block*
                prev = nullptr; // previous memory block
            dynamic_block*
                next = nullptr; // next memory block
            typename b_tree<dynamic_block, int>::b_treeNode*
                node = nullptr; // node in the B-Tree with free blocks
            int
                thread_id = 0;
            int
                allocated_block_index = 0;
            int
                size = 0; // size in bytes of the block
            int
                initialized_block_index = 0;
            int
                num = 0;
        };

    private:
        //GL::binary_search_tree< dynamic_block, int, 10>
        b_tree<dynamic_block, int>
            freeTree;   // B-Tree with free memory blocks
        GL::atomic_bag< dynamic_block* >
            allocated_blocks;
        GL::atomic_bag< dynamic_block* >
            initialized_blocks;

    public:
        void // required to set the node allocator before first use. 
            SetAllocator(GL::fast_atomic_allocator<typename b_tree<dynamic_block, int>::b_treeNode, 128>& allocator) {
            this->freeTree.nodeAllocator = &allocator;
        };
        static dynamic_block* // get the block for a given allocated pointer. 
            Block(type* ptr) {
            return (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));
        };

        array_allocator() = default;
        array_allocator(array_allocator const&) = delete;
        array_allocator(array_allocator&&) noexcept = delete;
        array_allocator& operator=(array_allocator const&) = delete;
        array_allocator& operator=(array_allocator&&) noexcept = delete;
        ~array_allocator() {
            if (!std::is_pod<type>::value) {
                initialized_blocks.unsafe_for_each([](dynamic_block* block) {
                    type* ptr{ block->GetMemory() };
                    for (int i = 0; i < block->num; ++i)
                        (ptr + i)->~type();
                    });
            }
            allocated_blocks.unsafe_for_each([](dynamic_block* block) {
                GL::mfree(block);
                });
        };

    public:
        template <typename Lock, typename Unlock> type*
            Alloc(const int num, Lock const& lock, Unlock const& unlock) {
            dynamic_block
                * block;
            type
                * ptr;

            if (num <= 0)
                return nullptr;

            lock();

            block = AllocInternal(num);
            if (block == nullptr)
                return nullptr;

            block = ResizeInternal(block, num);
            if (block == nullptr)
                return nullptr;

            unlock();

            ptr = block->GetMemory();

            if (std::is_pod<type>::value)
                ::memset((void*)ptr, 0, sizeof(type) * num);
            else {
                block->num = num;
                for (int i = 0; i < num; ++i) new (ptr + i) type();
                block->initialized_block_index = initialized_blocks.push_back(block);
            }

            return ptr;
        };
        template <typename Lock, typename Unlock> void
            Free(type* ptr, Lock const& lock, Unlock const& unlock) {
            if (!ptr) { return; }

            dynamic_block* block = (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));

            if (!std::is_pod<type>::value) {
                for (int i = 0; i < block->num; ++i) (ptr + i)->~type();
                initialized_blocks.erase(block->initialized_block_index);
            }

            lock();

            FreeInternal(block);

            unlock();
        };

    private:
        dynamic_block* // find a free block that is big enough for the request, otherwise manufacture it. 
            AllocInternal(const int num) {
            dynamic_block* block;
            int alignedBytes = (num * sizeof(type) + 15) & ~15; // request is aligned to 16 bytes

            block = freeTree.FindSmallestLargerEqual(alignedBytes);
            if (block) {
                UnlinkFreeInternal(block);
            }
            else {
                int allocSize = std::max(baseBlockSize, alignedBytes + (int)sizeof(dynamic_block));

                block = (dynamic_block*)GL::malloc((size_t)allocSize);

                block->SetSize(allocSize - (int)sizeof(dynamic_block), true);
                block->allocated_block_index = allocated_blocks.push_back(block);
                block->next = nullptr;
                block->prev = nullptr;
                block->node = nullptr;
            }
            block->thread_id = GL::util::get_thread_id();
            return block;
        };
        void
            UnlinkFreeInternal(dynamic_block* block) {
            freeTree.Remove(block->node);
            block->node = nullptr;
        };
        dynamic_block*
            ResizeInternal(dynamic_block* block, const int num) {
            dynamic_block* newBlock;
            int alignedBytes = (num * sizeof(type) + 15) & ~15;
            // if the new size is larger
            if (alignedBytes > block->GetSize()) {

                dynamic_block* nextBlock = block->next;

                // try to annexate the next block if it's free
                if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL &&
                    block->GetSize() + (int)sizeof(dynamic_block) + nextBlock->GetSize() >= alignedBytes) {

                    UnlinkFreeInternal(nextBlock);
                    block->SetSize(block->GetSize() + (int)sizeof(dynamic_block) + nextBlock->GetSize(), block->IsBaseBlock());
                    block->next = nextBlock->next;
                    if (nextBlock->next) {
                        nextBlock->next->prev = block;
                    }
                }
                else {
                    // allocate a new block and copy
                    dynamic_block* oldBlock = block;
                    block = AllocInternal(num);
                    if (block == NULL) {
                        return NULL;
                    }
                    ::memcpy(block->GetMemory(), oldBlock->GetMemory(), oldBlock->GetSize());
                    FreeInternal(oldBlock);
                }
            }

            // if the unused space at the end of this block is large enough to hold a block with at least one element
            if ((block->GetSize() - alignedBytes - (int)sizeof(dynamic_block)) < (int)sizeof(type))
                return block;

            newBlock = (dynamic_block*)(((::byte*)block) + (int)sizeof(dynamic_block) + alignedBytes);
            newBlock->SetSize(block->GetSize() - alignedBytes - (int)sizeof(dynamic_block), false);
            newBlock->next = block->next;
            newBlock->prev = block;
            if (newBlock->next != NULL) {
                newBlock->next->prev = newBlock;
            }
            newBlock->node = NULL;
            block->next = newBlock;
            block->SetSize(alignedBytes, block->IsBaseBlock());

            FreeInternal(newBlock);

            return block;
        };
        void
            FreeInternal(dynamic_block* block) {
            while (true) {
                // try to merge with a previous free block
                if (dynamic_block* prevBlock = block->prev; prevBlock && !prevBlock->IsBaseBlock() && prevBlock->node != NULL) {
                    UnlinkFreeInternal(prevBlock);
                    prevBlock->SetSize(prevBlock->GetSize() + (int)sizeof(dynamic_block) + block->GetSize(), prevBlock->IsBaseBlock());
                    prevBlock->next = block->next;
                    if (block->next) {
                        block->next->prev = prevBlock;
                    }
                    block = prevBlock;
                }
                // try to merge with a next free block
                else if (dynamic_block* nextBlock = block->next; nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL) {
                    UnlinkFreeInternal(nextBlock);
                    block->SetSize(nextBlock->GetSize() + (int)sizeof(dynamic_block) + block->GetSize(), block->IsBaseBlock());
                    block->next = nextBlock->next;
                    if (nextBlock->next) {
                        nextBlock->next->prev = block;
                    }
                }
                else if (dynamic_block* nextBlock = block->next, *prevBlock = block->prev; !nextBlock && !prevBlock) {
                    if (freeTree.FindSmallestLargerEqual(block->GetSize())) {
                        allocated_blocks.erase(block->allocated_block_index);
                        GL::mfree(block);
                    }
                    else {
                        LinkFreeInternal(block);
                    }
                    return;
                }
                else {
                    LinkFreeInternal(block);
                    return;
                }
            }
        };
        void
            LinkFreeInternal(dynamic_block* block) {
            block->node = freeTree.Add(block, block->GetSize());
        };

    };

    // thread-safe allocator that can allocate arrays of items (e.g. 128 floats, 1024 strings, etc.).
    template<class type, int baseBlockSize = 1024 * sizeof(type)>
    class parallel_array_allocator {
    protected:
        GL::fast_atomic_allocator<typename GL::b_tree<typename GL::array_allocator<type, baseBlockSize>::dynamic_block, int>::b_treeNode, 128>
            allocator;
        GL::thread_object_no_default < std::pair<GL::array_allocator<type, baseBlockSize>, GL::fast_exclusive_mutex> >
            alloc;

    public:
        parallel_array_allocator()
            : allocator()
            , alloc()
        {
            alloc._after_construction = [this](auto& tree) {
                tree.first.SetAllocator(this->allocator);
            };
        };
        parallel_array_allocator(parallel_array_allocator const&) = delete;
        parallel_array_allocator(parallel_array_allocator&&) noexcept = delete;
        parallel_array_allocator& operator=(parallel_array_allocator const&) = delete;
        parallel_array_allocator& operator=(parallel_array_allocator&&) noexcept = delete;
        ~parallel_array_allocator() = default;

    public:
        type*
            Alloc(const int num) {
            auto& Alloc = *alloc;
            return Alloc.first.Alloc(num, [&Alloc](void)->void {
                Alloc.second.lock();
                }, [&Alloc](void)->void {
                    Alloc.second.unlock();
                });
        };
        void
            Free(type* ptr) {
            auto& Alloc = alloc[GL::array_allocator<type, baseBlockSize>::Block(ptr)->thread_id];
            Alloc.first.Free(ptr, [&Alloc](void)->void {
                Alloc.second.lock();
                }, [&Alloc](void)->void {
                    Alloc.second.unlock();
                });
        };

    };

};