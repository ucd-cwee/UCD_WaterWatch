#pragma once
#include "dynamic_allocator.h"
#include "../ScriptLanguageTesting/atomic_allocator.h"
#include <variant>

template< class objType, class keyType, int maxChildrenPerNode >
class bTree {
public:
	struct bTreeNode {
		keyType	// key used for sorting						
			key;			
		objType // if != nullptr pointer to object stored in leaf node
			*object;		
		bTreeNode // parent node
			*parent;		
		bTreeNode // next sibling
			*next;			
		bTreeNode // prev sibling
			*prev;			
		int	// number of children							
			numChildren;	
		bTreeNode // first child
			*firstChild;	
		bTreeNode // last child
			*lastChild;		
	};

protected:
	bTreeNode*
		root;
	GL::atomic_allocator< bTreeNode >
		nodeAllocator;

public:
	bTree() {
		root = AllocNode();
	};
	~bTree() {
		root = nullptr;
	};

	// add an object to the tree
	bTreeNode* Add(objType* object, keyType key) {
		bTreeNode
			*node, 
			*child, 
			*newNode;

		if (root == nullptr) root = AllocNode();
		

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

		for (node = root; node->firstChild != nullptr; node = child) {
			if (key > node->key) node->key = key;
			
			// find the first child with a key larger equal to the key of the new node
			for (child = node->firstChild; child->next; child = child->next) 
				if (key <= child->key) 
					break;		

			if (child->object) {
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
				node->numChildren++;

				return newNode;
			}

			// make sure the child has room to store another node
			if (child->numChildren >= maxChildrenPerNode) {
				SplitNode(child);
				if (key <= child->prev->key) child = child->prev;				
			}
		}

		// we only end up here if the root node is empty
		newNode->parent = root;
		root->key = key;
		root->firstChild = newNode;
		root->lastChild = newNode;
		root->numChildren++;

		return newNode;
	};	
	// remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
	void Remove(bTreeNode* node) {
		bTreeNode* parent;

		// unlink the node from it's parent
		if (node->prev) node->prev->next = node->next;		
		else node->parent->firstChild = node->next;		
		if (node->next) node->next->prev = node->prev;		
		else node->parent->lastChild = node->prev;		
		node->parent->numChildren--;

		// make sure there are no parent nodes with a single child
		for (parent = node->parent; (parent != root) && (parent->numChildren <= 1); parent = parent->parent) {
			if (parent->next) parent = MergeNodes(parent, parent->next);			
			else if (parent->prev) parent = MergeNodes(parent->prev, parent);
			
			// a parent may not use a key higher than the key of its last child
			if (parent->key > parent->lastChild->key) parent->key = parent->lastChild->key;
			
			if (parent->numChildren > maxChildrenPerNode) {
				SplitNode(parent);
				break;
			}
		}
		// a parent may not use a key higher than the key of it's last child
		for (; (parent != nullptr) && (parent->lastChild != nullptr); parent = parent->parent)		
			if (parent->key > parent->lastChild->key) 
				parent->key = parent->lastChild->key;			
		
		// actually free the node
		FreeNode(node);

		// remove the root node if it has a single internal node as child
		if ((root->numChildren == 1) && (root->firstChild->object == nullptr)) {
			bTreeNode* oldRoot = root;
			root->firstChild->parent = nullptr;
			root = root->firstChild;
			FreeNode(oldRoot);
		}
	};				
	// find an object using the given key
	bTreeNode* NodeFind(keyType key) const {
		for (bTreeNode* node = root->firstChild; node != nullptr; node = node->firstChild) {
			while (node->next) {
				if (node->key >= key) break;				
				node = node->next;
			}
			if (node->object) {
				if (node->key == key) return node;				
				else return nullptr;				
			}
		}
		return nullptr;
	};								
	// find an object with the smallest key larger equal the given key
	bTreeNode* NodeFindSmallestLargerEqual(keyType key) const {
		if (root == nullptr) return nullptr;
		for (bTreeNode* node = root->firstChild; node != nullptr; node = node->firstChild) {
			while (node->next) {
				if (node->key >= key) break;				
				node = node->next;
			}
			if (node->object) {
				if (node->key >= key) return node;				
				else return nullptr;				
			}
		}
		return nullptr;
	};;			
	// find an object with the largest key smaller equal the given key
	bTreeNode* NodeFindLargestSmallerEqual(keyType key) const {
		bTreeNode
			*node, 
			*smaller;

		if (root == nullptr) return nullptr;
		for (node = root->firstChild, smaller = nullptr; node != nullptr; node = node->firstChild) {
			while (node->next) {
				if (node->key >= key) break;				
				smaller = node;
				node = node->next;
			}
			if (node->object) {
				if (node->key <= key) return node;				
				else if (smaller == nullptr) return nullptr;				
				else {
					node = smaller;
					if (node->object) return node;					
				}
			}
		}
		return nullptr;
	};	
	// find an object using the given key
	objType* Find(keyType key) const {
		if (bTreeNode* node = NodeFind(key)) return node->object;		
		else return nullptr;		
	};									
	// find an object with the smallest key larger equal the given key
	objType* FindSmallestLargerEqual(keyType key) const {
		if (bTreeNode* node = NodeFindSmallestLargerEqual(key)) return node->object;		
		else return nullptr;
	};		
	// find an object with the largest key smaller equal the given key
	objType* FindLargestSmallerEqual(keyType key) const {
		if (bTreeNode* node = NodeFindLargestSmallerEqual(key)) return node->object;		
		else return nullptr;		
	};		
	// returns the root node of the tree
	bTreeNode*  GetRoot() const {
		return root;
	}; 
	// goes through all nodes of the tree
	static bTreeNode*  GetNext(bTreeNode* node) {
		if (node->firstChild) return node->firstChild;		
		else {
			while (node && (node->next == nullptr)) node = node->parent;			
			return node;
		}
	};	
	// goes through all leaf nodes of the tree
	static bTreeNode*  GetNextLeaf(bTreeNode* node) {
		if (node->firstChild) {
			while (node->firstChild) node = node->firstChild;			
			return node;
		}
		else {
			while (node && (node->next == nullptr)) node = node->parent;			
			if (node) {
				node = node->next;
				while (node->firstChild) node = node->firstChild;				
				return node;
			}
			else return nullptr;			
		}
	};

private:
	bTreeNode* AllocNode() {
		bTreeNode* node = nodeAllocator.Alloc();
		node->key = 0;
		node->parent = nullptr;
		node->next = nullptr;
		node->prev = nullptr;
		node->numChildren = 0;
		node->firstChild = nullptr;
		node->lastChild = nullptr;
		node->object = nullptr;
		return node;
	};
	void FreeNode(bTreeNode* node) {
		nodeAllocator.Free(node);
	};
	void SplitNode(bTreeNode* node) {
		int i;
		bTreeNode* child, * newNode;

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
	};;
	bTreeNode* MergeNodes(bTreeNode* node1, bTreeNode* node2) {
		bTreeNode* child;

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

template< class objType, class keyType, int maxChildrenPerNode >
class atomic_tree {
public:
	struct tree_node;
	struct branch_data {
		tree_node // first child
			*firstChild;
		tree_node // last child
			*lastChild;
		int	// number of children							
			numChildren;
	};
	struct leaf_data {
		objType // if != nullptr pointer to object stored in leaf node
			*object;
	};
	struct tree_node {
		tree_node // parent node
			*parent;
		tree_node // next sibling
			*next;
		tree_node // prev sibling
			*prev;
		union {
			branch_data // branches have children nodes
				branch; 
			leaf_data // leafs do not have children, but do have stored data
				leaf;
		} data;
		const keyType // key used for sorting
			key;
		bool
			is_leaf;
	};
	class locked_tree_node {
		friend class atomic_tree;
	private:
		tree_node
			*node;
		const atomic_tree
			*parent;

	protected:
		locked_tree_node(const atomic_tree* _p, tree_node* _n) 
			: node{ _n }
			, parent{ _p } 
			, object{ nullptr }
			, key{ nullptr }
		{
			if (parent) parent->mutex.lock_shared();
			if (node && node->is_leaf) object = node->data.leaf.object;
			if (node) key = &node->key;
		}

	public:
		objType
			*object;
		const keyType
			*key;

		operator bool() const {
			return object;
		};
		locked_tree_node next() const {
			return locked_tree_node(parent, GetNextLeaf(node));
		};
		locked_tree_node prev() const {
			return locked_tree_node(parent, GetPrevLeaf(node));
		};

		locked_tree_node()
			: node{ nullptr }
			, parent{ nullptr }
			, object{ nullptr }
			, key{ nullptr }
		{};
		locked_tree_node(locked_tree_node const& rhs)
			: node{ rhs.node }
			, parent{ rhs.parent }
			, object{ rhs.object }
			, key{ rhs.key }
		{
			if (parent) {
				parent->mutex.lock_shared();
			}
		};
		locked_tree_node(locked_tree_node && rhs)
			: node{ rhs.node }
			, parent{ rhs.parent }
			, object{ rhs.object }
			, key{ rhs.key }
		{
			rhs.node = nullptr;
			rhs.parent = nullptr;
			rhs.object = nullptr;
			rhs.key = nullptr;
		};
		locked_tree_node& operator=(locked_tree_node const& rhs) {
			if (parent) parent->mutex.unlock_shared();
			node = rhs.node;
			parent = rhs.parent;
			object = rhs.object;
			key = rhs.key;
			if (parent) parent->mutex.lock_shared();
			return *this;
		};
		locked_tree_node& operator=(locked_tree_node&& rhs) {
			if (parent) parent->mutex.unlock_shared();
			node = rhs.node;
			parent = rhs.parent;
			object = rhs.object;
			key = rhs.key;			
			rhs.node = nullptr;
			rhs.parent = nullptr;
			rhs.object = nullptr;
			rhs.key = nullptr;
			return *this;
		};
		~locked_tree_node() {
			if (parent) {
				parent->mutex.unlock_shared();
			}
		};
	};

private:
	GL::atomic_allocator< tree_node > // will automatically recover memory if unreleased
		nodeAllocator;
	tree_node* // will be recovered by the nodeAllocator on destruction. 
		root;
	mutable std::shared_mutex
		mutex;

public:
	atomic_tree() 
		: root{ nullptr }
		, nodeAllocator()
		, mutex()
	{};
	atomic_tree(atomic_tree const&) = delete;
	atomic_tree(atomic_tree &&) = delete;
	atomic_tree& operator=(atomic_tree const&) = delete;
	atomic_tree& operator=(atomic_tree&&) = delete;
	~atomic_tree() = default;

	locked_tree_node // find an iterator for the atomic_tree that can be progressed forwards or backwards. 
		Find(keyType key) const {
		std::shared_lock locked(mutex);
		if (tree_node* n = NodeFind(key)) {
			return locked_tree_node(this, n);
		}
		else {
			return locked_tree_node();
		}
	};
	locked_tree_node // find an iterator for the atomic_tree with a key value less than or equal to the provided key
		FindLEQ(keyType key) const {
		std::shared_lock locked(mutex);
		if (tree_node* n = NodeFindLargestSmallerEqual(key)) {
			return locked_tree_node(this, n);
		}
		else {
			return locked_tree_node();
		}
	};
	locked_tree_node // find an iterator for the atomic_tree with a key value greater than or equal to the provided key
		FindGEQ(keyType key) const {
		std::shared_lock locked(mutex);
		if (tree_node* n = NodeFindSmallestLargerEqual(key)) {
			return locked_tree_node(this, n);
		}
		else {
			return locked_tree_node();
		}
	};

	void // add an object to the tree
		Add(objType* object, keyType key) {
		tree_node
			* node = nullptr,
			* child = nullptr,
			* newNode;

		std::scoped_lock 
			locked(mutex);

		if (!root) {
			root = AllocNode(key);
		}

		if (root->data.branch.numChildren >= maxChildrenPerNode) {
			newNode = AllocNode(root->key);
			newNode->data.branch.firstChild = root;
			newNode->data.branch.lastChild = root;
			newNode->data.branch.numChildren = 1;
			root->parent = newNode;
			SplitNode(root); // to-do
			root = newNode;
		}

		newNode = AllocNode(key);
		newNode->data.leaf.object = object;
		newNode->is_leaf = true;

		for (node = root; (!node->is_leaf) && (node->data.branch.firstChild); node = child) {
			// since a parent key may not be smaller than the largest child key...
			if (key > node->key) const_cast<keyType&>(node->key) = key;
			
			// find the first child with a key larger equal to the key of the new node
			for (child = node->data.branch.firstChild; child->next; child = child->next)
				if (key <= child->key)
					break;			

			if (child->is_leaf) {
				if (key <= child->key) {
					// insert new node before child
					if (child->prev) child->prev->next = newNode;					
					else node->data.branch.firstChild = newNode;
					newNode->prev = child->prev;
					newNode->next = child;
					child->prev = newNode;
				}
				else {
					// insert new node after child
					if (child->next) child->next->prev = newNode;
					else node->data.branch.lastChild = newNode;
					newNode->prev = child;
					newNode->next = child->next;
					child->next = newNode;
				}

				newNode->parent = node;
				node->data.branch.numChildren++;

				return;
			}

			// make sure the child has room to store another node
			if (child->data.branch.numChildren >= maxChildrenPerNode) {
				SplitNode(child);
				if (key <= child->prev->key) child = child->prev;
			}
		}

		// we only end up here if the root node is empty
		newNode->parent = root;
		// const_cast<keyType&>(root->key) = key;
		root->data.branch.firstChild = newNode;
		root->data.branch.lastChild = newNode;
		root->data.branch.numChildren++;
	};	
	void // remove an object node from the tree if the key is found. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
		Remove(keyType key) {
		std::scoped_lock 
			locked(mutex);

		NodeRemove(NodeFind(key));
	};	
	bool // remove an object node from the tree if the key and object match. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
		Remove(objType* object, keyType key) {
		std::scoped_lock 
			locked(mutex);

		if (tree_node* node = NodeFindSmallestLargerEqual(key)) {
			while (node) {
				if (node->key == key) {
					if (node->is_leaf && (node->data.leaf.object == object)) {
						NodeRemove(node);
						return true;
					}
					else {
						node = GetNextLeaf(node);
					}
				}
				else {
					return false;
				}
			}
		}
		return false;
	};
	template <class Lambda> __declspec(noinline) bool // removes an object if the Lambda function returns true. 
		RemoveIf(keyType search_start, Lambda const& F) {
		std::scoped_lock 
			locked(mutex);
		if (auto tree_node = NodeFindSmallestLargerEqual(search_start)) {
			while (tree_node) {
				if (tree_node->is_leaf) {
					if (F(tree_node)) {
						NodeRemove(tree_node);
						return true;
					}
				}
				tree_node = GetNextLeaf(tree_node);
			}
		}
		return false;
	};
	objType* // Returns the pointer to the underlying object if found at the provided key.
		At(keyType key) const {
		std::shared_lock locked(mutex);
		if (auto* node = NodeFind(key)) {
			if (node->is_leaf)
				return node->data.leaf.object;
		}
		return nullptr;
	};
	template <class Lambda> void // Iterates over each Leaf node and calls the lambda function.
		ForEach(Lambda const& F) const {
		std::shared_lock locked(mutex);
		if (auto* n = GetRoot()) {
			while (n) {
				if (n->is_leaf) {
					F(n);
				}
				n = GetNextLeaf(n);
			}
		}
	};



private:	
	void // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
		NodeRemove(tree_node* node) {
		tree_node
			*parent;

		if (node) {
			// unlink the node from it's parent
			if (node->prev) node->prev->next = node->next;
			else node->parent->data.branch.firstChild = node->next;
			if (node->next) node->next->prev = node->prev;
			else node->parent->data.branch.lastChild = node->prev;
			node->parent->data.branch.numChildren--;

			// make sure there are no parent nodes with a single child
			for (parent = node->parent; (parent != root) && (parent->data.branch.numChildren <= 1); parent = parent->parent) {
				if (parent->next) parent = MergeNodes(parent, parent->next);
				else if (parent->prev) parent = MergeNodes(parent->prev, parent);

				// a parent may not use a key higher than the key of its last child
				if (parent->key > parent->data.branch.lastChild->key) const_cast<keyType&>(parent->key) = parent->data.branch.lastChild->key;

				if (parent->data.branch.numChildren > maxChildrenPerNode) {
					SplitNode(parent);
					break;
				}
			}
			// a parent may not use a key higher than the key of it's last child
			for (; (parent != nullptr) && (parent->data.branch.lastChild != nullptr); parent = parent->parent)
				if (parent->key > parent->data.branch.lastChild->key) const_cast<keyType&>(parent->key) = parent->data.branch.lastChild->key;

			// actually free the node
			FreeNode(node);

			// remove the root node if it has a single internal node as child
			if ((root->data.branch.numChildren == 1) && (!root->data.branch.firstChild->is_leaf)) {
				tree_node* oldRoot = root;
				root->data.branch.firstChild->parent = nullptr;
				root = root->data.branch.firstChild;
				FreeNode(oldRoot);
			}
		}
	};	
	tree_node* // find an object using the given key
		NodeFind(keyType key) const {
		if (root) {
			for (tree_node* node = root->data.branch.firstChild; node != nullptr; node = ((node->is_leaf) ? (tree_node*)nullptr : node->data.branch.firstChild)) {
				while (node->next) {
					if (node->key >= key) break;
					node = node->next;
				}
				if (node->is_leaf) {
					if (node->key == key) return node;
					else return nullptr;
				}
			}
		}
		return nullptr;
	};	
	tree_node* // find an object with the smallest key larger equal the given key
		NodeFindSmallestLargerEqual(keyType key) const {
		if (root == nullptr) return nullptr;
		for (tree_node* node = root->data.branch.firstChild; node != nullptr; node = ((node->is_leaf) ? (tree_node*)nullptr : node->data.branch.firstChild)) {
			while (node->next) {
				if (node->key >= key) break;
				node = node->next;
			}
			if (node->is_leaf) {
				if (node->key >= key) return node;
				else return nullptr;
			}
		}
		return nullptr;
	};;		
	tree_node* // find an object with the largest key smaller equal the given key
		NodeFindLargestSmallerEqual(keyType key) const {
		tree_node
			* node,
			* smaller;

		if (!root) return nullptr;
		for (node = root->data.branch.firstChild, smaller = nullptr; node != nullptr; node = ((node->is_leaf) ? (tree_node*)nullptr : node->data.branch.firstChild)) {
			while (node->next) {
				if (node->key >= key) break;
				smaller = node;
				node = node->next;
			}
			if (node->is_leaf) {
				if (node->key <= key) return node;
				else if (smaller == nullptr) return nullptr;
				else {
					node = smaller;
					if (node->is_leaf) return node;
				}
			}
		}
		return nullptr;
	};
	objType* // find an object with the smallest key larger equal the given key
		FindSmallestLargerEqual(keyType key) const {
		if (tree_node* node = NodeFindSmallestLargerEqual(key)) return node->data.leaf.object;
		else return nullptr;
	};	
	objType* // find an object with the largest key smaller equal the given key
		FindLargestSmallerEqual(keyType key) const {
		if (tree_node* node = NodeFindLargestSmallerEqual(key)) return node->data.leaf.object;
		else return nullptr;
	};
	tree_node* // returns the root node of the tree
		GetRoot() const {
		return root;
	};	
	static tree_node* // goes through all nodes of the tree
		GetNext(tree_node* node) {
		if (!node->is_leaf) {
			if (node->data.branch.firstChild) return node->data.branch.firstChild;
		}
		while (node && (node->next == nullptr)) node = node->parent;
		return node;		
	};	
	static tree_node* // goes through all leaf nodes of the tree
		GetNextLeaf(tree_node* node) {
		if (!node->is_leaf) {
			if (node->data.branch.firstChild) {
				while (node->data.branch.firstChild) node = node->data.branch.firstChild;
				return node;
			}
		}		
		while (node && (node->next == nullptr)) node = node->parent;
		if (node) {
			node = node->next;
			while (!node->is_leaf && node->data.branch.firstChild) node = node->data.branch.firstChild;
			return node;
		}
		else return nullptr;		
	};
	static tree_node* // goes through all leaf nodes of the tree
		GetPrevLeaf(tree_node* node) {
		if (!node) return nullptr;
		if (!node->is_leaf) {
			if (node->data.branch.lastChild) {
				while (node->data.branch.lastChild) node = node->data.branch.lastChild;
				return node;
			}
		}
		
		while (node && node->prev == nullptr) node = node->parent;
		if (node) {
			node = node->prev;
			while (!node->is_leaf && node->data.branch.lastChild) node = node->data.branch.lastChild;
			return node;
		}
		else return nullptr;		
	};

private:
	tree_node* 
		AllocNode(keyType key) {
		return nodeAllocator.Alloc(tree_node{
			nullptr, nullptr, nullptr, { nullptr }, key, false
		});
	};
	void 
		FreeNode(tree_node* node) {
		nodeAllocator.Free(node);
	};
	void 
		SplitNode(tree_node* node) {
		int i;
		tree_node
			*child, 
			*newNode;

		// allocate a new node
		newNode = AllocNode(keyType{}); // wrong key -- guessing
		newNode->parent = node->parent;

		// divide the children over the two nodes
		child = node->data.branch.firstChild;
		child->parent = newNode;
		for (i = 3; child && (i < node->data.branch.numChildren); i += 2) {
			child = child->next;
			child->parent = newNode;
		}
		
		const_cast<keyType&>(newNode->key) = child->key;
		newNode->data.branch.numChildren = node->data.branch.numChildren / 2;
		newNode->data.branch.firstChild = node->data.branch.firstChild;
		newNode->data.branch.lastChild = child;

		node->data.branch.numChildren -= newNode->data.branch.numChildren;
		node->data.branch.firstChild = child->next;

		child->next->prev = nullptr;
		child->next = nullptr;

		if (node->prev) node->prev->next = newNode;		
		else node->parent->data.branch.firstChild = newNode;

		newNode->prev = node->prev;
		newNode->next = node;
		node->prev = newNode;

		node->parent->data.branch.numChildren++;
	};;
	tree_node* 
		MergeNodes(tree_node* node1, tree_node* node2) {
		tree_node
			*child;

		for (child = node1->data.branch.firstChild; child->next; child = child->next) child->parent = node2;
		child->parent = node2;
		child->next = node2->data.branch.firstChild;
		node2->data.branch.firstChild->prev = child;
		node2->data.branch.firstChild = node1->data.branch.firstChild;
		node2->data.branch.numChildren += node1->data.branch.numChildren;

		// unlink the first node from the parent
		if (node1->prev) node1->prev->next = node2;		
		else node1->parent->data.branch.firstChild = node2;
		
		node2->prev = node1->prev;
		node2->parent->data.branch.numChildren--;

		FreeNode(node1);

		return node2;
	};

};