#pragma once
#include "dynamic_allocator.h"
#include "../ScriptLanguageTesting/atomic_allocator.h"
#include "../ScriptLanguageTesting/atomic_maps.h"

template< class objType, class keyType, int maxChildrenPerNode >
class bTree {
public:
	struct bTreeNode {
		keyType	// key used for sorting						
			key;
		objType // if != nullptr pointer to object stored in leaf node
			* object;
		bTreeNode // parent node
			* parent;
		bTreeNode // next sibling
			* next;
		bTreeNode // prev sibling
			* prev;
		int	// number of children							
			numChildren;
		bTreeNode // first child
			* firstChild;
		bTreeNode // last child
			* lastChild;
	};

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
			* node,
			* child,
			* newNode;

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
			* node,
			* smaller;

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
	bTreeNode* GetRoot() const {
		return root;
	};
	// goes through all nodes of the tree
	static bTreeNode* GetNext(bTreeNode* node) {
		if (node->firstChild) return node->firstChild;
		else {
			while (node && (node->next == nullptr)) node = node->parent;
			return node;
		}
	};
	// goes through all leaf nodes of the tree
	static bTreeNode* GetNextLeaf(bTreeNode* node) {
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
	bTreeNode* root;
	GL::atomic_allocator< bTreeNode > nodeAllocator;

	bTreeNode*
		AllocNode() {
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
	void
		FreeNode(bTreeNode* node) {
		nodeAllocator.Free(node);
	};
	void
		SplitNode(bTreeNode* node) {
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
	bTreeNode*
		MergeNodes(bTreeNode* node1, bTreeNode* node2) {
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


// template< class objType, class keyType, int maxChildrenPerNode >
class aTree {
public:
	using objType = int;
	using keyType = int;
	static constexpr int maxChildrenPerNode = 10;

	struct aTreeNode {
		keyType	// key used for sorting						
			key;
		objType // if != nullptr pointer to object stored in leaf node
			* object;

		aTreeNode // parent node
			* parent;
		short
			parent_index;

		aTreeNode* // next sibling
			next() const {
			if (parent) {
				if ((parent_index + 1) < parent->numChildren) {
					return parent->children[parent_index + 1];
				}
			}
			return nullptr;
		};
		aTreeNode* // prev sibling
			prev() const {
			if (parent) {
				if (parent_index > 0) {
					return parent->children[parent_index - 1];
				}
			}
			return nullptr;
		};	
		void 
			update_children_indices() {
			for (short i = 0; i < numChildren; ++i) children[i]->parent_index = i;			
		};		
		void // assumes there is room
			insert_child(aTreeNode* node) {
			short i = 0;
			for (i = numChildren - 1; i >= 0; --i) {
				if (children[i]->key > node->key) {
					children[i + 1] = children[i];
					children[i + 1]->parent_index = i + 1;
				}
				else {
					node->parent = this;
					children[i + 1] = node;
					children[i + 1]->parent_index = i + 1;
					++numChildren;
					return;
				}
			}
			// failed to do insert for some reason
			node->parent = this;
			children[0] = node;
			children[0]->parent_index = 0;
			++numChildren;
			return;
		};
		void // assumes the node exists in the list of children
			remove_child(aTreeNode* node) {
			short i = 0;
			for (i = 0; i < numChildren; ++i) {
				if (node) { // still looking					
					if (children[i] == node) {
						// start removal
						children[i] = children[i + 1];
						node = nullptr;
					}
					else continue;					
				}
				else { // moving
					children[i] = children[i + 1];
				}
			}
			if (node) throw std::runtime_error("Could not locate requested node");
			--numChildren;
			for (i = numChildren; i <= maxChildrenPerNode; ++i) children[i] = nullptr;
			update_children_indices();
		};


		int	// number of children							
			numChildren;
		aTreeNode
			*children[maxChildrenPerNode + 1];
		long
			marked; // if != 0 then the node is queued for replacement and the search needs to back-up or re-start
	};
	struct search_history {
		aTreeNode 
			*parent;
		short 
			parent_index;
	};
	using history_stack = std::deque< search_history >;
public:
	// goes through all leaf nodes of the tree
	static aTreeNode* GetNextLeaf(aTreeNode* node/*, history_stack& history*/) {
		//if (node->numChildren > 0) {
		//	while (node->numChildren > 0) { 
		//		history.push_back({ node, 0 });
		//		node = node->children[0]; 
		//	}
		//	return node;
		//}
		//else {
		//	// no 'next' means that we are at the end of this segment of children and need to jump to the parent 
		//	while (node && !node->next()) {
		//		auto& most_recent_parent = history.back();
		//		node = most_recent_parent.parent;
		//		history.pop_back();
		//	}


		//	while (node && !node->next()) {
		//		
		//	}
		//	if (node) {
		//		node = node->next();
		//		while (node->numChildren > 0) { node = node->children[0]; }
		//		return node;
		//	}
		//	else return nullptr;
		//}









		if (node->numChildren > 0) {
			while (node->numChildren > 0) { node = node->children[0]; }
			return node;
		}
		else {
			while (node && !node->next()) node = node->parent;
			if (node) {
				node = node->next();
				while (node->numChildren > 0) { node = node->children[0]; }
				return node;
			}
			else return nullptr;
		}
	};

	aTree() : nodeAllocator(), root{ AllocNode() } {};
	aTree(aTree const&) = delete;
	aTree(aTree &&) noexcept = delete;
	aTree& operator=(aTree const&) = delete;
	aTree& operator=(aTree&&) noexcept = delete;
	~aTree() = default;

	// add an object to the tree
	__declspec(noinline) aTreeNode* Add(objType* object, keyType key) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		aTreeNode
			*node,
			*child,
			*newNode,
			*rootCopy
			;
		bool complete = false;
		bool need_retry = false;
		while (!complete) {
			node = child = newNode = nullptr;
			rootCopy = root;
			need_retry = false;

			// Ensures the root & rootCopy is valid & alive
			if (rootCopy == nullptr) {
				root = AllocNode();
				continue;
			}

			// if the root is too large, it must be split
			if (rootCopy->numChildren >= maxChildrenPerNode) {
				newNode = AllocNode();
				newNode->key = rootCopy->key;
				newNode->children[0] = rootCopy;
				newNode->numChildren = 1;
				rootCopy->parent_index = 0;				
				rootCopy->parent = newNode;
				SplitNode(rootCopy);
				root = newNode;
				continue;
			}

			newNode = AllocNode();
			newNode->key = key;
			newNode->object = object;

			for (node = rootCopy; node->numChildren > 0; node = child) {
				if (key > node->key) node->key = key; // race condition

				// find the first child with a key larger equal to the key of the new node
				for (child = node->children[0]; child->next(); child = child->next())
					if (key <= child->key) break;

				if (child->object) {
					newNode->parent = node;
					node->insert_child(newNode);
					return newNode;
				}

				// make sure the child has room to store another node
				if (child->numChildren >= maxChildrenPerNode) {
					SplitNode(child);
					need_retry = true;
					break;
				}
			}

			if (!need_retry) {
				// we only end up here if the root node is empty
				newNode->parent = rootCopy;
				rootCopy->key = key;
				rootCopy->children[0] = newNode;
				rootCopy->numChildren = 1;
				return newNode;
			}
		}
		// return newNode;
	};
	// remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
	void Remove(aTreeNode* node) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		aTreeNode* parent;

		// unlink the node from it's parent
		node->parent->remove_child(node);

		// make sure there are no parent nodes with a single child
		for (parent = node->parent; (parent != root) && (parent->numChildren <= 1); parent = parent->parent) {
			if (parent->next()) parent = MergeNodes(parent, parent->next());
			else if (parent->prev()) parent = MergeNodes(parent->prev(), parent);

			// a parent may not use a key higher than the key of its last child
			if (parent->key > parent->children[parent->numChildren - 1]->key) 
				parent->key = parent->children[parent->numChildren - 1]->key;

			if (parent->numChildren > maxChildrenPerNode) {
				SplitNode(parent);
				break;
			}
		}
		// a parent may not use a key higher than the key of it's last child
		for (; parent && parent->children[parent->numChildren - 1]; parent = parent->parent)
			if (parent->key > parent->children[parent->numChildren - 1]->key)
				parent->key = parent->children[parent->numChildren - 1]->key;

		// actually free the node
		FreeNode(node);

		// remove the root node if it has a single internal node as child
		if ((root->numChildren == 1) && (root->children[0]->object == nullptr)) {
			aTreeNode* oldRoot = root;
			root->children[0]->parent = nullptr;
			root = root->children[0];
			FreeNode(oldRoot);
		}
	};
	// find an object using the given key
	__declspec(noinline) aTreeNode* NodeFind(keyType key) const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		for (aTreeNode* node = root->children[0]; node; node = node->children[0]) {
			while (node->next()) {
				if (node->key >= key) break;
				node = node->next();
			}
			if (node->object) {
				if (node->key == key) return node;
				else return nullptr;
			}
		}
		return nullptr;
	};
	// find an object with the smallest key larger equal the given key
	aTreeNode* NodeFindSmallestLargerEqual(keyType key) const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		if (root == nullptr) return nullptr;
		for (aTreeNode* node = root->children[0]; node; node = node->children[0]) {
			while (node->next()) {
				if (node->key >= key) break;
				node = node->next();
			}
			if (node->object) {
				if (node->key >= key) return node;
				else return nullptr;
			}
		}
		return nullptr;
	};
	// find an object with the largest key smaller equal the given key
	aTreeNode* NodeFindLargestSmallerEqual(keyType key) const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		aTreeNode
			* node,
			* smaller;

		if (root == nullptr) return nullptr;
		for (node = root->children[0], smaller = nullptr; node; node = node->children[0]) {
			while (node->next()) {
				if (node->key >= key) break;
				smaller = node;
				node = node->next();
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
		if (aTreeNode* node = NodeFind(key)) return node->object;
		else return nullptr;
	};
	// find an object with the smallest key larger equal the given key
	objType* FindSmallestLargerEqual(keyType key) const {
		if (aTreeNode* node = NodeFindSmallestLargerEqual(key)) return node->object;
		else return nullptr;
	};
	// find an object with the largest key smaller equal the given key
	objType* FindLargestSmallerEqual(keyType key) const {
		if (aTreeNode* node = NodeFindLargestSmallerEqual(key)) return node->object;
		else return nullptr;
	};
	// returns the root node of the tree
	aTreeNode* GetRoot() const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		return root;
	};


private:
	aTreeNode
		*root;
	GL::atomic_epoch_allocator< aTreeNode, GL::atomic_allocator<aTreeNode> >
		nodeAllocator;

	aTreeNode* AllocNode() {
		aTreeNode* node = nodeAllocator.Alloc();
		node->key = 0;
		node->parent = nullptr;
		for (short i = 0; i <= maxChildrenPerNode; ++i) node->children[i] = nullptr;
		node->numChildren = 0;
		node->parent_index = 0;
		node->object = nullptr;
		node->marked = 0;
		return node;
	};
	//aTreeNode* CopyNode(aTreeNode* to_copy) {
	//	aTreeNode* node = nodeAllocator.Alloc();
	//	node->key = to_copy->key;
	//	node->parent = to_copy->parent;
	//	node->numChildren = to_copy->numChildren;
	//	node->parent_index = to_copy->parent_index;
	//	node->object = to_copy->object;
	//	node->marked = 0;
	//	return node;
	//};
	void FreeNode(aTreeNode* node) {
		nodeAllocator.Free(node);
	};
	void
		SplitNode(aTreeNode* node) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		int i;
		aTreeNode* child, * newNode;

		// allocate a new node
		newNode = AllocNode();



		newNode->parent = node->parent;

		// divide the children over the two nodes
		int sz = node->numChildren / 2;
		for (int i = 0; i < sz; ++i) {
			newNode->children[i] = node->children[0];		
			node->remove_child(node->children[0]);
			newNode->children[i]->parent_index = i;
			newNode->children[i]->parent = newNode;
			newNode->key = newNode->children[i]->key;
			newNode->numChildren++;
		}
		node->parent->insert_child(newNode);
	};
	aTreeNode*
		MergeNodes(aTreeNode* node1, aTreeNode* node2) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		aTreeNode* child;
		for (int i = node1->numChildren - 1; i >= 0; --i) {
			child = node1->children[i];
			node2->insert_child(child);
			node1->remove_child(child);
		}
		node1->parent->remove_child(node1);
		FreeNode(node1);
		return node2;
	};

};



#if 0
// template<class objType, class keyType, int degree>
class tree {
public:
	using objType = int;
	using keyType = int;
	static constexpr int degree = 10;

	class treeNode {
	public:
		keyType // local copy of the keys
			keys[degree - 1];
		std::atomic<treeNode*> // pointers to children nodes
			ptrs[degree];
		int // number of keys
			count;
		bool // true if leaf node
			is_leaf;
		std::atomic<int> // != 0 if being accessed
			marked;
		std::mutex // lock for node (delete)
			nodeLock;
	};
	GL::atomic_epoch_allocator< treeNode, GL::atomic_allocator<treeNode> >
		nodeAllocator;

	void queue_for_deletion(const treeNode* p) {
		nodeAllocator.Free(const_cast<treeNode*>(p));
	};
	treeNode*
		root;

	// takes a root node, moves the left-most items to a new child, moves the right-most items to a new child, and makes a new root
	void SplitRootNode() {
		const treeNode
			* currRoot = root;
		treeNode
			* newRoot = nullptr,
			* sib1 = nullptr,
			* sib2 = nullptr;
		int
			siz = 0;
		auto delayed_node_deletion
			= nodeAllocator.ProtectCurrentEpoch();

		// create the sibling nodes
		sib1 = nodeAllocator.Alloc();
		sib1->is_leaf = currRoot->is_leaf;
		sib2 = nodeAllocator.Alloc();
		sib2->is_leaf = currRoot->is_leaf;

		// set size based on degree
		siz = (degree - 1) / 2;

		// copy the first (degree-1)/2 keys of current root to sibling 1
		for (int j = 0; j < siz; j++)
			sib1->keys[j] = currRoot->keys[j];

		// copy the last (degree-1)/2 keys of current root to sibling 2
		for (int j = 0; j < siz; j++)
			sib2->keys[j] = currRoot->keys[j + siz + 1];

		// copy the first degree/2 ptrs of current root over to sibling 1
		if (!currRoot->is_leaf)
			for (int j = 0; j < degree / 2; j++)
				sib1->ptrs[j] = currRoot->ptrs[j].load();

		// copy the last degree/2 ptrs of current root over to sibling 2
		if (!currRoot->is_leaf)
			for (int j = 0; j < degree / 2; j++)
				sib2->ptrs[j] = currRoot->ptrs[j + degree / 2].load();

		// set sibling sizes
		sib1->count = siz;
		sib2->count = siz;

		// create and populate new root
		newRoot = nodeAllocator.Alloc();
		newRoot->is_leaf = false;
		newRoot->ptrs[0] = sib1;
		newRoot->ptrs[1] = sib2;
		newRoot->keys[0] = currRoot->keys[siz];
		newRoot->count = 1;

		// perform the CAS
		if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), newRoot, const_cast<treeNode*>(currRoot)) != currRoot) {
			queue_for_deletion(newRoot);
			queue_for_deletion(sib1);
			queue_for_deletion(sib2);
			// we failed the split. Hopefully the caller learns this the hard way and tries again.
		}
		else {
			// we successfully performed the split. The old root should be queued for deletion.
			queue_for_deletion(currRoot);
		}
	};
	treeNode* CloneNode(const treeNode* to_clone) {
		auto delayed_node_deletion
			= nodeAllocator.ProtectCurrentEpoch();

		treeNode* out = nodeAllocator.Alloc();
		out->count = to_clone->count;
		out->is_leaf = to_clone->is_leaf;
		for (int i = 0; i < out->count; ++i) out->keys[i] = to_clone->keys[i];
		for (int i = 0; i <= out->count; ++i) out->ptrs[i] = to_clone->ptrs[i].load();
		return out;
	};

	// note, user must queue for deletion the newParent (returned), sib1, and sib2 if the CAS fails.
	treeNode* SplitChildNode(const treeNode* parent, const treeNode* child, treeNode*& sib1, treeNode*& sib2) {
		auto delayed_node_deletion
			= nodeAllocator.ProtectCurrentEpoch();

		treeNode
			* newParent;
		int
			siz,
			idx;

		newParent = CloneNode(parent);

		// create new sibling nodes 
		sib1 = nodeAllocator.Alloc();
		sib1->is_leaf = child->is_leaf;

		sib2 = nodeAllocator.Alloc();
		sib2->is_leaf = child->is_leaf;

		// set size based on degree 
		siz = (degree - 1) / 2;

		// copy the first (degree-1)/2 keys of child to sibling 1 
		for (int j = 0; j < siz; j++)
			sib1->keys[j] = child->keys[j];

		// copy the last (degree-1)/2 keys of child to sibling 2 
		for (int j = 0; j < siz; j++)
			sib2->keys[j] = child->keys[j + siz + 1];

		// copy the first degree/2 ptrs of child over to sibling 1 
		if (!child->is_leaf)
			for (int j = 0; j < degree / 2; j++)
				sib1->ptrs[j] = child->ptrs[j].load();

		// set sibling size 
		sib1->count = siz;

		// copy the last degree/2 ptrs of child over to sibling 2 
		if (!child->is_leaf)
			for (int j = 0; j < degree / 2; j++)
				sib2->ptrs[j] = child->ptrs[j + degree / 2].load();

		// set sibling size 
		sib2->count = siz;

		// find where new key is going in new parent 
		idx = 0;
		while ((idx < newParent->count) && (newParent->keys[idx] < child->keys[siz]))
			idx++;

		// slide new parent child ptrs over to make room for new child 
		for (int j = newParent->count; j >= idx + 1; j--)
			newParent->ptrs[j + 1] = newParent->ptrs[j].load();

		// slide new parent child keys over to make room for new key 
		for (int j = newParent->count - 1; j >= idx; j--)
			newParent->keys[j + 1] = newParent->keys[j];

		// add new siblings to new parent 
		newParent->ptrs[idx] = sib1;
		newParent->ptrs[idx + 1] = sib2;

		// copy middle key of child to new parent 
		newParent->keys[idx] = child->keys[siz];

		// increment count of keys in new parent 
		newParent->count += 1;

		return newParent;
	};

	// see https://oasis.library.unlv.edu/cgi/viewcontent.cgi?article=4733&context=thesesdissertations
	void insert(keyType key) {
		treeNode
			* currPtr = nullptr,
			* prevPtr = nullptr,
			* newRoot = nullptr,
			* currChild = nullptr,
			* newCurrChild = nullptr,
			* newCurrPtr = nullptr,
			* newLeafPtr = nullptr,
			* sib1 = nullptr,
			* sib2 = nullptr;
		int
			currIndex = 0,
			prevIdx = 0;
		bool
			insertDone = false;
		auto delayed_node_deletion
			= nodeAllocator.ProtectCurrentEpoch();

		while (!insertDone) {
			// if root is NULL, create new root
			if (root == nullptr) {
				newRoot = nodeAllocator.Alloc();
				currPtr = nullptr;
				if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), newRoot, nullptr) != nullptr) {
					queue_for_deletion(newRoot);
					newRoot = nullptr;
				}
				continue; // try again from the top
			}

			// if root is full, tree grows in height.
			if (root->count == (degree - 1)) {
				SplitRootNode();
				continue; // try again from the top
			}

			// traverse downward to find applicable leaf node, split full nodes along the way
			currPtr = root;
			prevPtr = nullptr;
			prevIdx = -1;
			while (currPtr && !currPtr->is_leaf) {
				// find/set currIndex for appropriate child based on key
				while ((currIndex < currPtr->count) && (key > currPtr->keys[currIndex])) {
					currIndex++;
				}
				currChild = currPtr->ptrs[currIndex].load();

				if (currChild->count == (degree - 1)) { // must split child node (figure 25)
					// set flags for impacted nodes to indicate updates in process 
					set  currPtr->markedand currChild->marked;

					// split child node (figure 26) 
					// creates new current and child nodes 
					newCurrPtr = splitChildNode(currPtr, currChild, sib1, sib2);

					// attempt to cut-in split node into previous level (figure 27) // if at root, must update at root 
					if (prevPtr == NULL) {// at root?
						CAS in new  newCurrNode  into root
							// fail => delete nodes newCurrPtr, newChild, //  sb1, and sb2 // continue from top (while insert not done)
							// success => enqueue old nodes for deletion
					}
					else { // not at root
						// note, if marked, abandon changes, continue from top
						if (prevPtr->marked) {
							CAS in new  newCurrPtr  into  prevPtr[prexIdx] // fail => delete newCurrPtr and newChild // continue from top // success => enqueue related nodes for deletion
						}
					}
					... re - find currIndex for appropriate child based on key...


				}
				// move down tree to next level, track previous level 
				prevPtr = currPtr;
				prevIdx = currIndex;
				set  currNode  to appropriate child based on currIndex;
			} // end while

			// attempt to cut-in split node into previous level (figure 29) // if at root, must update at root 
			if (prevPtr == NULL) { // at root? 
				CAS in  newLeafPtr  into  root
					fail = > delete  newLeafPtr
					continue from top(while insert not done)
					success = > set  insertDone = true
			}
			else {
				// note, if marked, abandon changes, continue from top
				if  prevPtr->marked
					not CAS in newLeafPtr into prevPtr at prevIdx
					// fail => delete newLeafPtr 
					//     continue from top (while insert not done) 
					// success => enqueue released nodes for deletion set insertDone=true

					set insertDone = true
			}




		}  // end while not insertDone
	} // end insert





};
#endif