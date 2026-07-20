#pragma once

#include "atomic_bag.h"
#include "atomic_epoch_allocator.h"
#include "epoch_map.h"
#include "atomic_tree.h"
#include "atomic_vector.h"
#include <array>
#include <execution>
#include <memory>
#include <concurrent_unordered_map.h>


namespace GL {
    namespace impl{
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

        // Multi-threaded version of a B-Tree that uses a course-grained lock with parallel allocator to make it thread-safe. Nodes are at-risk of disposal once the lock is returned.
        // Attempts to speed-up searching using a binomial search within BTree nodes. In theory should benefit from larger maxChildrenPerNode values. 
        template< class objType, class keyType, int maxChildrenPerNode = 10 >
        class binary_search_b_tree {
        public:
            struct binary_search_b_treeNode {
                keyType	// key used for sorting						
                    key;
                binary_search_b_treeNode // parent node
                    * parent;
                int	// number of children							
                    numChildren;
                int
                    parent_index;
                void* // std::variant< objType*, std::array<binary_search_b_treeNode*, maxChildrenPerNode>* >		
                    data;
                bool
                    is_leaf;
                objType*
                    object() {
                    if (numChildren > 0 || !is_leaf) return nullptr;
                    return static_cast<objType*>(data);
                };
                binary_search_b_treeNode**
                    children() {
                    return &static_cast<std::array<binary_search_b_treeNode*, maxChildrenPerNode>*>(data)->operator[](0);
                };

                binary_search_b_treeNode* // next sibling
                    next() {
                    if (parent && (parent->numChildren > (parent_index + 1))) {
                        return parent->children()[parent_index + 1];
                    }
                    else {
                        return nullptr;
                    }
                };
                binary_search_b_treeNode* // prev sibling
                    prev() {
                    if (parent && (parent_index >= 1)) {
                        return parent->children()[parent_index - 1];
                    }
                    else {
                        return nullptr;
                    }
                };
                binary_search_b_treeNode* // first child
                    firstChild() {
                    if (numChildren == 0) return nullptr;
                    return children()[0];
                };
                binary_search_b_treeNode* // last child
                    lastChild() {
                    if (numChildren == 0) return nullptr;
                    return children()[numChildren - 1];
                };
                void
                    add_child(binary_search_b_treeNode* p) {
                    p->parent = this;
                    p->parent_index = numChildren;
                    children()[numChildren] = p;
                    ++numChildren;
                    if (this->key < p->key) this->key = p->key;
                }
                __declspec(noinline) void
                    add_child_at(binary_search_b_treeNode* p, int i) {
                    if (i >= numChildren) add_child(p);
                    else {
                        binary_search_b_treeNode**
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
                        std::memmove(&ch[i + 1], &ch[i], sizeof(binary_search_b_treeNode*) * (numChildren - i));
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
                binary_search_b_treeNode*
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
                binary_search_b_treeNode*
                    pop_child(int i) {
                    if (numChildren <= 0) {
                        return nullptr;
                    }
                    else {
                        binary_search_b_treeNode**
                            ch = children();
                        binary_search_b_treeNode*
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
                        std::memmove(&ch[i], &ch[i + 1], sizeof(binary_search_b_treeNode*) * ((numChildren - i) - 1));
                        if (numChildren > 1) this->key = ch[numChildren - 2]->key;
                        ch[j] = nullptr;
                        ch = &ch[i];
                        for (; i < j; ++i) {
                            (*ch)->parent_index = i;
                            ++ch;
                        }
                        --numChildren;
    #endif
                        return out;
                    }
                }
                binary_search_b_treeNode*
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
                binary_search_b_treeNode*
                    binomial_search_smallest_greater_equal_to(keyType K) {
    #if 0
                    binary_search_b_treeNode* child = this->firstChild();
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
                    binary_search_b_treeNode
                        * sample;
                    if (numChildren == 0)
                        return nullptr;
                    if (numChildren == 1)
                        return children()[0];

                    len = numChildren;
                    mid = len;
                    offset = 0;
                    res = false;

                    while (mid > 0) {
                        mid = len >> 1;
                        sample = children()[offset + mid];
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
                    if (mid == numChildren) return children()[offset];
                    else return children()[mid];
                    //}
    #endif
                };

            };

        private:
            binary_search_b_treeNode*
                root;
        public:
            GL::fast_atomic_allocator<binary_search_b_treeNode, 256>*
                nodeAllocator;
            GL::fast_atomic_allocator< std::array<binary_search_b_treeNode*, maxChildrenPerNode>, 32 >*
                nodeChildrenAllocator;
        private:
            long
                count;

        public:
            binary_search_b_tree()
                : nodeAllocator{ nullptr }
                , nodeChildrenAllocator{ nullptr }
                , root{ nullptr }
                , count{ 0 }
            {};
            binary_search_b_tree(binary_search_b_tree const&)
                = delete;
            binary_search_b_tree(binary_search_b_tree&&) noexcept
                = delete;
            binary_search_b_tree& operator=(binary_search_b_tree const&)
                = delete;
            binary_search_b_tree& operator=(binary_search_b_tree&&) noexcept
                = delete;
            ~binary_search_b_tree()
                = default;

            __declspec(noinline) binary_search_b_treeNode* // add an object to the tree
                Add(objType* object, keyType key) {
                binary_search_b_treeNode
                    * node,
                    * child,
                    * newNode;
                newNode
                    = AllocNode(true);
                newNode->key
                    = key;
                newNode->data
                    = object;

                if (root == nullptr) root = AllocNode(false); // start fresh

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
                            // insert new node before child
                            node->add_child_at(newNode, child->parent_index);
                        }
                        else {
                            // insert new node after child
                            node->add_child_at(newNode, child->parent_index + 1);
                        }

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

                ++count;
                return newNode;

            };
            __declspec(noinline) bool // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
                Remove(binary_search_b_treeNode* node) {
                binary_search_b_treeNode
                    * Node,
                    * parent,
                    * oldRoot;

                if (node == nullptr) return false;

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
            __declspec(noinline) binary_search_b_treeNode* // find an object using the given key
                NodeFind(keyType key) {
                binary_search_b_treeNode*
                    node;

                if (!root || (root->numChildren <= 0)) return nullptr;

                for (node = root; node; ) {
                    node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                    if (node->object()) {
                        if (node->key == key) return node;
                        else return nullptr;
                    }
                    if (!node || (node->numChildren <= 0)) return nullptr;
                }
                return nullptr;
            };
            __declspec(noinline) binary_search_b_treeNode* // find an object with the smallest key larger equal the given key
                NodeFindSmallestLargerEqual(keyType key) {
                binary_search_b_treeNode
                    * nxt;
                binary_search_b_treeNode*
                    node;

                if (!root || (root->numChildren <= 0)) return nullptr;
                for (node = root; node; ) {
                    node = node->binomial_search_smallest_greater_equal_to(key); // returns the child with a node->key >= provided key. 
                    if (node->object()) {
                        if (node->key >= key) return node;
                        else return nullptr;
                    }
                    if (!node || (node->numChildren <= 0)) return nullptr;
                }
                return nullptr;
            };

            __declspec(noinline) objType*
                Find(keyType key) {
                binary_search_b_treeNode* node = NodeFind(key);
                if (node == nullptr) {
                    return nullptr;
                }
                else {
                    return node->object();
                }
            }; // find an object using the given key
            __declspec(noinline) objType*
                FindSmallestLargerEqual(keyType key) {
                binary_search_b_treeNode* node = NodeFindSmallestLargerEqual(key);
                if (node == nullptr) {
                    return nullptr;
                }
                else {
                    return node->object();
                }
            }; // find an object with the smallest key larger equal the given key

    #if 0
            std::pair<binary_search_b_treeNode*, locker> // find an object with the largest key smaller equal the given key
                NodeFindLargestSmallerEqual(keyType key) {
                binary_search_b_treeNode
                    * smaller;
                std::pair<binary_search_b_treeNode*, locker>
                    out;
                binary_search_b_treeNode*&
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
            binary_search_b_treeNode* // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
                GetRoot() {
                return root;
            };
    #if 0
            static binary_search_b_treeNode* // goes through all nodes of the tree		
                GetNext(binary_search_b_treeNode* node, locker& locking) {
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
            static binary_search_b_treeNode* // goes through all leaf nodes of the tree		
                GetNextLeaf(binary_search_b_treeNode* node) {
                if (!node) return nullptr;

                binary_search_b_treeNode*
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
            size_t
                size() const {
                return (size_t)count;
            };

        private:
            __declspec(noinline) binary_search_b_treeNode*
                AllocNode(bool is_leaf) {
                binary_search_b_treeNode
                    * node;

                node = nodeAllocator->Alloc();
                if (is_leaf) {
                    node->is_leaf = true;
                    node->data = nullptr;
                }
                else {
                    node->is_leaf = false;
                    node->data = nodeChildrenAllocator->Alloc();
                    for (int i = 0; i < maxChildrenPerNode; ++i) node->children()[i] = nullptr;
                }

                node->key = 0;
                node->parent = nullptr;
                node->parent_index = 0;
                node->numChildren = 0;

                return node;
            };
            __declspec(noinline) void
                FreeNode(binary_search_b_treeNode* node) {
                if (node) {
                    if (!node->is_leaf) {
                        nodeChildrenAllocator->Free(static_cast<std::array<binary_search_b_treeNode*, maxChildrenPerNode>*>(node->data));
                    }
                    nodeAllocator->Free(node);
                }
            };
            __declspec(noinline) void // will split node by creating a neighbor next to it in the parent node and sharing half its children
                SplitNode(binary_search_b_treeNode* node) {
                int
                    i, j;
                binary_search_b_treeNode
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
            __declspec(noinline) binary_search_b_treeNode* // node1 will be deleted and its children appended to node2
                MergeNodes(binary_search_b_treeNode* node1, binary_search_b_treeNode* node2) {
                for (int i = 0; i < node1->numChildren; ++i)
                    node2->add_child_at(node1->children()[i], i);
                (void)node1->parent->pop_child(node1->parent_index);
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
                typename binary_search_b_tree<dynamic_block, int>::binary_search_b_treeNode*
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
            GL::impl::binary_search_b_tree< dynamic_block, int>
                freeTree;   // B-Tree with free memory blocks
            GL::atomic_bag< dynamic_block* >
                allocated_blocks;
            GL::atomic_bag< dynamic_block* >
                initialized_blocks;

        public:
            void // required to set the node allocator before first use. 
                SetAllocator(GL::fast_atomic_allocator<typename binary_search_b_tree<dynamic_block, int>::binary_search_b_treeNode, 256>& node_allocator, GL::fast_atomic_allocator< std::array<typename binary_search_b_tree<dynamic_block, int>::binary_search_b_treeNode*, 10>, 32 >& node_children_allocator) {
                this->freeTree.nodeAllocator = &node_allocator;
                this->freeTree.nodeChildrenAllocator = &node_children_allocator;
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
                        std::destroy(ptr, ptr + block->num);
                        // for (int i = 0; i < block->num; ++i) (ptr + i)->~type();
                    });
                }
                allocated_blocks.unsafe_for_each([](dynamic_block* block) {
                    GL::mfree(block);
                });
            };

        public:
            template <typename Lock, typename Unlock> __declspec(noinline) type*
                Alloc(const int num, Lock const& lock, Unlock const& unlock, bool cleared_alloc = false) {
                dynamic_block
                    * block;
                type
                    * ptr;

                if (num <= 0)
                    return nullptr;

                lock();

                block = AllocInternal(num);
                if (block == nullptr) {
                    unlock();
                    return nullptr;
                }

                block = ResizeInternal(block, num);
                if (block == nullptr) {
                    unlock();
                    return nullptr;
                }

                unlock();

                ptr = block->GetMemory();

                if constexpr (std::is_pod<type>::value) {             
                    if (cleared_alloc) {
                        // std::uninitialized_fill_n(ptr, num, type{});
                        ::memset((void*)ptr, 0, sizeof(type) * num);
                    }
                }
                else {
                    block->num = num;
                    std::uninitialized_default_construct(ptr, ptr + num);
                    // for (int i = 0; i < num; ++i) new (ptr + i) type();
                    block->initialized_block_index = initialized_blocks.push_back(block);
                }

                return ptr;
            };
            template <typename Lock, typename Unlock> __declspec(noinline) void
                Free(type* ptr, Lock const& lock, Unlock const& unlock) {
                if (!ptr) { return; }

                dynamic_block* block = (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));

                if (!std::is_pod<type>::value) {
                    std::destroy(ptr, ptr + block->num);
                    // for (int i = 0; i < block->num; ++i) (ptr + i)->~type();
                    initialized_blocks.erase(block->initialized_block_index);
                }

                lock();

                FreeInternal(block);

                unlock();
            };

        private:
            __declspec(noinline) dynamic_block* // find a free block that is big enough for the request, otherwise manufacture it. 
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
            __declspec(noinline) void
                UnlinkFreeInternal(dynamic_block* block) {
                freeTree.Remove(block->node);
                block->node = nullptr;
            };
            __declspec(noinline) dynamic_block*
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
            __declspec(noinline) void
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
            __declspec(noinline) void
                LinkFreeInternal(dynamic_block* block) {
                block->node = freeTree.Add(block, block->GetSize());
            };

        };

        // non-atomic, non-thread-safe allocator that can allocate arrays of items (e.g. 128 floats, 1024 strings, etc.).
        template <int baseBlockSize = 8 << 12>
        class generic_array_allocator {
        public:
            class dynamic_block {
            public:
                void*
                    GetMemory() const { return (void*)(((::byte*)this) + sizeof(dynamic_block)); }
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
                typename binary_search_b_tree<dynamic_block, int>::binary_search_b_treeNode*
                    node = nullptr; // node in the B-Tree with free blocks
                void (*deleter)(void*, size_t) = nullptr; 
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
            GL::impl::binary_search_b_tree< dynamic_block, int>
                freeTree;   // B-Tree with free memory blocks
            GL::atomic_bag< dynamic_block* >
                allocated_blocks;
            GL::atomic_bag< dynamic_block* >
                initialized_blocks;

        public:
            void // required to set the node allocator before first use. 
                SetAllocator(GL::fast_atomic_allocator<typename binary_search_b_tree<dynamic_block, int>::binary_search_b_treeNode, 256>& node_allocator, GL::fast_atomic_allocator< std::array<typename binary_search_b_tree<dynamic_block, int>::binary_search_b_treeNode*, 10>, 32 >& node_children_allocator) {
                this->freeTree.nodeAllocator = &node_allocator;
                this->freeTree.nodeChildrenAllocator = &node_children_allocator;
            };
            static dynamic_block* // get the block for a given allocated pointer. 
                Block(void* ptr) {
                return (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));
            };

            generic_array_allocator() = default;
            generic_array_allocator(generic_array_allocator const&) = delete;
            generic_array_allocator(generic_array_allocator&&) noexcept = delete;
            generic_array_allocator& operator=(generic_array_allocator const&) = delete;
            generic_array_allocator& operator=(generic_array_allocator&&) noexcept = delete;
            ~generic_array_allocator() {            
                initialized_blocks.unsafe_for_each([](dynamic_block* block) {
                    if (block->deleter) {
                        block->deleter(block->GetMemory(), block->num);
                    }
                });            
                allocated_blocks.unsafe_for_each([](dynamic_block* block) {
                    GL::mfree(block);
                });
            };

        public:
            template <typename T, typename Lock, typename Unlock> __declspec(noinline) T*
                Alloc(const int num, Lock const& lock, Unlock const& unlock, void(*deleter)(void*, size_t), bool cleared_alloc = false) {
                dynamic_block
                    * block;
                T
                    * ptr;

                if (num <= 0)
                    return nullptr;

                lock();

                block = AllocInternal(num * sizeof(T));
                if (block == nullptr) {
                    unlock();
                    return nullptr;
                }

                block = ResizeInternal(block, num * sizeof(T));
                if (block == nullptr) {
                    unlock();
                    return nullptr;
                }

                unlock();

                ptr = reinterpret_cast<T*>(block->GetMemory());
                block->deleter = deleter;
                block->num = num;
                if constexpr (std::is_pod<T>::value) {
                    if (cleared_alloc) ::memset((void*)ptr, 0, sizeof(T) * num);                
                }
                else {
                    std::uninitialized_default_construct(ptr, ptr + num);                    
                }
                block->initialized_block_index = initialized_blocks.push_back(block);

                return ptr;
            };
            template <typename Lock, typename Unlock> __declspec(noinline) void
                Free(void* ptr, Lock const& lock, Unlock const& unlock) {
                if (!ptr) { return; }

                dynamic_block* block = (dynamic_block*)(((::byte*)ptr) - (int)sizeof(dynamic_block));

                if (block->deleter) {
                    block->deleter(ptr, block->num);
                    initialized_blocks.erase(block->initialized_block_index);
                }

                lock();

                FreeInternal(block);

                unlock();
            };

        private:
            __declspec(noinline) dynamic_block* // find a free block that is big enough for the request, otherwise manufactures it. 
                AllocInternal(const int num) {
                dynamic_block* block;
                int alignedBytes = (num + 15) & ~15; // request is aligned to 16 bytes

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
            __declspec(noinline) void
                UnlinkFreeInternal(dynamic_block* block) {
                freeTree.Remove(block->node);
                block->node = nullptr;
            };
            __declspec(noinline) dynamic_block*
                ResizeInternal(dynamic_block* block, const int num) {
                dynamic_block* newBlock;
                int alignedBytes = (num + 15) & ~15;
                // if the new size is larger
                if (alignedBytes > block->GetSize()) {

                    dynamic_block* nextBlock = block->next;

                    // try to annexate the next block if it's free
                    if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != nullptr &&
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
                        if (block == nullptr) {
                            return nullptr;
                        }
                        ::memcpy(block->GetMemory(), oldBlock->GetMemory(), oldBlock->GetSize());
                        FreeInternal(oldBlock);
                    }
                }

                // if the unused space at the end of this block is large enough to hold a block with at least one element
                if ((block->GetSize() - alignedBytes - (int)sizeof(dynamic_block)) < (int)64)
                    return block;

                newBlock = (dynamic_block*)(((::byte*)block) + (int)sizeof(dynamic_block) + alignedBytes);
                newBlock->SetSize(block->GetSize() - alignedBytes - (int)sizeof(dynamic_block), false);
                newBlock->next = block->next;
                newBlock->prev = block;
                if (newBlock->next != nullptr) {
                    newBlock->next->prev = newBlock;
                }
                newBlock->node = nullptr;
                block->next = newBlock;
                block->SetSize(alignedBytes, block->IsBaseBlock());

                FreeInternal(newBlock);

                return block;
            };
            __declspec(noinline) void
                FreeInternal(dynamic_block* block) {
                // short max_help = 1;
                while (true) {
                    // try to merge with a previous free block
                    if (dynamic_block* prevBlock = block->prev; prevBlock && !prevBlock->IsBaseBlock() && prevBlock->node != nullptr) {
                        UnlinkFreeInternal(prevBlock);
                        prevBlock->SetSize(prevBlock->GetSize() + (int)sizeof(dynamic_block) + block->GetSize(), prevBlock->IsBaseBlock());
                        prevBlock->next = block->next;
                        if (block->next) {
                            block->next->prev = prevBlock;
                        }
                        block = prevBlock;
                    }
                    // try to merge with a next free block
                    else if (dynamic_block* nextBlock = block->next; nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != nullptr) {
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
            __declspec(noinline) void
                LinkFreeInternal(dynamic_block* block) {
                block->node = freeTree.Add(block, block->GetSize());
            };

        };
    }

    // thread-safe allocator that can allocate arrays of items of the specified type (e.g. 128 floats, 1024 strings, etc.).
    // all items will be of the same type using this allocator. Allocates an array of items at a time. 
    // On destruction of the allocator, all memory will be collected properly. 
    template<class type, int baseBlockSize = 1024 * sizeof(type)>
    class parallel_array_allocator {
    protected:
        GL::fast_atomic_allocator<typename impl::binary_search_b_tree<typename GL::impl::array_allocator<type, baseBlockSize>::dynamic_block, int>::binary_search_b_treeNode, 256>
            node_allocator_m;
        GL::fast_atomic_allocator< std::array<typename impl::binary_search_b_tree<typename GL::impl::array_allocator<type, baseBlockSize>::dynamic_block, int>::binary_search_b_treeNode*, 10>, 32 >
            node_children_allocator_m;
        GL::thread_object_no_default < std::pair<GL::impl::array_allocator<type, baseBlockSize>, GL::fast_exclusive_mutex> >
            alloc_m;

    public:
        parallel_array_allocator()
            : node_allocator_m()
            , node_children_allocator_m()
            , alloc_m()
        {
            alloc_m._after_construction = [this](auto& tree) {
                tree.first.SetAllocator(this->node_allocator_m, this->node_children_allocator_m);
            };
        };
        parallel_array_allocator(parallel_array_allocator const&) = delete;
        parallel_array_allocator(parallel_array_allocator&&) noexcept = delete;
        parallel_array_allocator& operator=(parallel_array_allocator const&) = delete;
        parallel_array_allocator& operator=(parallel_array_allocator&&) noexcept = delete;
        ~parallel_array_allocator() = default;

    public:        
        // allocate N items as an array. POD-types are not cleared and may be garbage data. 
        __declspec(noinline) type* 
            alloc(const int num) {
            auto& Alloc = *alloc_m;
            return Alloc.first.Alloc(num, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            });
        };
        
        // allocate N items as an array. POD-types are cleared to 0. 
        __declspec(noinline) type* 
            calloc(const int num) {
            auto& Alloc = *alloc_m;
            return Alloc.first.Alloc(num, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            }, true);
        };
        
        // free a pointer to an array of items. 
        __declspec(noinline) void 
            free(void* ptr) {
            auto& Alloc = alloc_m[GL::impl::array_allocator<type, baseBlockSize>::Block((type*)ptr)->thread_id];
            Alloc.first.Free((type*)ptr, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            });
        };

    };

    // thread-safe allocator that can allocate arrays of any types of items (e.g. 128 floats, 1024 strings, etc.).
    // allowed to mix-and-match types using this allocator. Allocates an array of items at a time. 
    // On destruction of the allocator, all memory will be collected properly. 
    template <int baseBlockSize = 8 << 12>
    class parallel_generic_array_allocator {
    // public:
        // static constexpr int baseBlockSize = 8 << 12;

    protected:
        GL::fast_atomic_allocator<typename impl::binary_search_b_tree<typename GL::impl::generic_array_allocator<baseBlockSize>::dynamic_block, int>::binary_search_b_treeNode, 256>
            node_allocator_m;
        GL::fast_atomic_allocator< std::array<typename impl::binary_search_b_tree<typename GL::impl::generic_array_allocator<baseBlockSize>::dynamic_block, int>::binary_search_b_treeNode*, 10>, 32 >
            node_children_allocator_m;
        GL::thread_object_no_default < std::pair<GL::impl::generic_array_allocator<baseBlockSize>, GL::fast_exclusive_mutex> >
            alloc_m;

    public:
        parallel_generic_array_allocator()
            : node_allocator_m()
            , node_children_allocator_m()
            , alloc_m()
        {
            alloc_m._after_construction = [this](auto& tree) {
                tree.first.SetAllocator(this->node_allocator_m, this->node_children_allocator_m);
            };
        };
        parallel_generic_array_allocator(parallel_generic_array_allocator const&) = delete;
        parallel_generic_array_allocator(parallel_generic_array_allocator&&) noexcept = delete;
        parallel_generic_array_allocator& operator=(parallel_generic_array_allocator const&) = delete;
        parallel_generic_array_allocator& operator=(parallel_generic_array_allocator&&) noexcept = delete;
        ~parallel_generic_array_allocator() = default;

    public:
        // allocate N bytes.
        __declspec(noinline) void*
            alloc(const size_t bytes, void(*destroy)(void*, size_t)) {
            auto& Alloc = *alloc_m;
            return Alloc.first.Alloc<unsigned char>(bytes / sizeof(unsigned char), [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            }, destroy, true);
        };

        // allocate N items as an array. POD-types are not cleared and may be garbage data. 
        template <typename T> __declspec(noinline) T*
            alloc(const int num) {
            auto& Alloc = *alloc_m;
            return Alloc.first.Alloc<T>(num, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            }, [](void* ptr, size_t num) -> void {
                std::destroy(reinterpret_cast<T*>(ptr), reinterpret_cast<T*>(ptr) + num);
            }, false);
        };

        // allocate N items as an array. POD-types are cleared to 0. 
        template <typename T> __declspec(noinline) T*
            calloc(const int num) {
            auto& Alloc = *alloc_m;
            return Alloc.first.Alloc<T>(num, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            }, [](void* ptr, size_t num) -> void {
                std::destroy(reinterpret_cast<T*>(ptr), reinterpret_cast<T*>(ptr) + num);
            }, true);
        };

        // free a pointer to an array of items. 
        __declspec(noinline) void
            free(void* ptr) {
            auto& Alloc = alloc_m[GL::impl::generic_array_allocator<baseBlockSize>::Block(ptr)->thread_id];
            Alloc.first.Free(ptr, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            });
        };

    };

#if 1
    // thread-safe allocator that can allocate any type of item (e.g. float, string, etc.).
    // allowed to mix-and-match types using this allocator. Only allocates one item at a time. 
    // On destruction of the allocator, all memory will be collected properly. 
    class parallel_generic_singleton_array_allocator {
    public:
        static constexpr int baseBlockSize = 8 << 12;

    protected:
        GL::fast_atomic_allocator<typename impl::binary_search_b_tree<typename GL::impl::generic_array_allocator<baseBlockSize>::dynamic_block, int>::binary_search_b_treeNode, 256>
            node_allocator_m;
        GL::fast_atomic_allocator< std::array<typename impl::binary_search_b_tree<typename GL::impl::generic_array_allocator<baseBlockSize>::dynamic_block, int>::binary_search_b_treeNode*, 10>, 32 >
            node_children_allocator_m;
        GL::thread_object_no_default < std::pair<GL::impl::generic_array_allocator<baseBlockSize>, GL::fast_exclusive_mutex> >
            alloc_m;

    public:
        parallel_generic_singleton_array_allocator()
            : node_allocator_m()
            , node_children_allocator_m()
            , alloc_m()
        {
            alloc_m._after_construction = [this](auto& tree) {
                tree.first.SetAllocator(this->node_allocator_m, this->node_children_allocator_m);
            };
        };
        parallel_generic_singleton_array_allocator(parallel_generic_singleton_array_allocator const&) = delete;
        parallel_generic_singleton_array_allocator(parallel_generic_singleton_array_allocator&&) noexcept = delete;
        parallel_generic_singleton_array_allocator& operator=(parallel_generic_singleton_array_allocator const&) = delete;
        parallel_generic_singleton_array_allocator& operator=(parallel_generic_singleton_array_allocator&&) noexcept = delete;
        ~parallel_generic_singleton_array_allocator() = default;

    public:
        // allocate N items as an array. POD-types are not cleared and may be garbage data. 
        template <typename T> __declspec(noinline) T*
            alloc() {
            auto& Alloc = *alloc_m;
            T* out = reinterpret_cast<T*>(Alloc.first.Alloc<unsigned char>(sizeof(T), [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            }, [](void* ptr, size_t num) -> void {
                reinterpret_cast<T*>(ptr)->~T();
            }, false));
            if constexpr (!std::is_pod_v<T>) {
                new (out) T();
            }
            return out;
        };

        // allocate N items as an array. POD-types are cleared to 0. 
        template <typename T> __declspec(noinline) T*
            calloc() {
            auto& Alloc = *alloc_m;
            T* out = reinterpret_cast<T*>(Alloc.first.Alloc<unsigned char>(sizeof(T), [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            }, [](void* ptr, size_t num) -> void {
                reinterpret_cast<T*>(ptr)->~T();
            }, true));
            if constexpr (!std::is_pod_v<T>) {
                new (out) T();
            }
            return out;
        };

        // free a pointer to an array of items. 
        __declspec(noinline) void
            free(void* ptr) {
            auto& Alloc = alloc_m[GL::impl::generic_array_allocator<baseBlockSize>::Block(ptr)->thread_id];
            Alloc.first.Free(ptr, [&Alloc](void)->void {
                Alloc.second.lock();
            }, [&Alloc](void)->void {
                Alloc.second.unlock();
            });
        };

    };
#else
    class parallel_generic_singleton_array_allocator {
    private:
        struct wrap {
            wrap* m_pNext;
            size_t type_index;
        };
        GL::epoch_map<size_t, GL::type> 
            type_to_index;
        GL::ticket_dispensor<false>
            tickets;
        GL::atomic_vector< GL::aba_problem::THead<wrap> >
            stack_heads;
        parallel_generic_array_allocator
            allocator;

    public:

        template <typename T> __declspec(noinline) T*
            alloc() {        
            size_t index = type_to_index.get_or_make(GL::type_of<T>(), [this]() -> size_t {
                return this->tickets.get_ticket();
            });
            GL::aba_problem::THead<wrap>& head = stack_heads.get_or_make(index);
            while (true) {
                if (wrap* p = GL::aba_problem::Pop(head)) {
                    T* out = reinterpret_cast<T*>(&p[0] + 2);
                    return out;
                }
                else {
                    static constexpr size_t Num = 1024;
                    void* ptr = allocator.alloc(Num * (sizeof(T) + sizeof(wrap)), [](void* ptr, size_t num) -> void {
                        if constexpr (!std::is_pod_v<T>) {
                            for (int i = 0; i < num; ++i) {
                                auto* p = reinterpret_cast<wrap*>((reinterpret_cast<unsigned char*>(ptr) + ((sizeof(T) + sizeof(wrap)) * i)));
                                T* out = reinterpret_cast<T*>(&p[0] + 2);
                                out->~T();
                            }
                        }
                    });
                    for (int i = 0; i < Num; ++i) {
                        auto* this_wrap = reinterpret_cast<wrap*>((reinterpret_cast<unsigned char*>(ptr) + ((sizeof(T) + sizeof(wrap)) * i)));
                        this_wrap->type_index = index;
                        T* out = reinterpret_cast<T*>(&this_wrap[0] + 2);
                        new (out) T();
                        GL::aba_problem::Stack_Push(head, this_wrap);
                    }
                }
            }
        };

        // free a pointer to an array of items. 
        __declspec(noinline) void
            free(void* ptr) {
            wrap* this_ptr = reinterpret_cast<wrap*>(&reinterpret_cast<wrap*>(ptr)[0] - 2);
            GL::aba_problem::Stack_Push(stack_heads.get_or_make(this_ptr->type_index), this_ptr);
        };

    };


#endif


};