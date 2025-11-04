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

		aTreeNode* // next sibling
			next(aTreeNode* parent, short parent_index) const {
			if (parent) {
				if ((parent_index + 1) < parent->numChildren) {
					return parent->children[parent_index + 1];
				}
			}
			return nullptr;
		};
		aTreeNode* // prev sibling
			prev(aTreeNode* parent, short parent_index) const {
			if (parent) {
				if (parent_index > 0) {
					return parent->children[parent_index - 1];
				}
			}
			return nullptr;
		};	
		void // assumes there is room. Inserting should invalidate any and all search histories
			insert_child(aTreeNode* node) {
			short i = 0;
			for (i = numChildren - 1; i >= 0; --i) {
				if (children[i]->key > node->key) {
					children[i + 1] = children[i];
				}
				else {
					children[i + 1] = node;
					++numChildren;
					return;
				}
			}
			// failed to do insert for some reason
			children[0] = node;
			++numChildren;
			return;
		};
		void // assumes the node exists in the list of children. Removing should invalidate any and all search histories
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
		};

		int	// number of children
			numChildren;
		aTreeNode
			*children[maxChildrenPerNode + 1];
		long
			marked; // if != 0 then the node is queued for replacement and the search needs to back-up or re-start
	};

private:
	aTreeNode
		*root;
	GL::atomic_epoch_allocator< aTreeNode, GL::atomic_allocator<aTreeNode> >
		nodeAllocator;

public:
	struct search_history {
		aTreeNode 
			*parent;
		short 
			parent_index;
	};
	using history_stack = std::deque< search_history >;

public:
	// goes through all leaf nodes of the tree
	__declspec(noinline) static aTreeNode* 
		GetNextLeaf(aTreeNode* node, history_stack& history) {
		while (true) {
			if (node->numChildren > 0) {
				while (node->numChildren > 0) {
					history.push_back({ node, 0 });
					node = node->children[0];
				}
				return node;
			}
			else {
				// if there is no history, then we are calling this on the root with no children

				if (history.empty()) return nullptr;
				if (!node) return nullptr;

				if (node->next(history.back().parent, history.back().parent_index)) {
					node = node->next(history.back().parent, history.back().parent_index);
					history.back().parent_index++;
					while (node->numChildren > 0) {
						history.push_back({ node, 0 });
						node = node->children[0];
					}
					return node;
				}
				else {
					// continue from parent
					if (history.empty()) return nullptr;
					
					while (!history.empty() && !node->next(history.back().parent, history.back().parent_index)) {
						node = history.back().parent;
						history.pop_back();
					}
					if (history.empty()) return nullptr;
					node = node->next(history.back().parent, history.back().parent_index);
					history.back().parent_index++;
				}
			}
		}
	};

	aTree() 
		: nodeAllocator()
		, root{ AllocNode() } 
	{};
	aTree(aTree const&) = delete;
	aTree(aTree &&) noexcept = delete;
	aTree& operator=(aTree const&) = delete;
	aTree& operator=(aTree&&) noexcept = delete;
	~aTree() = default;

	// add an object to the tree
	__declspec(noinline) aTreeNode* 
		Add(objType* object, keyType key) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		aTreeNode
			*node,
			*child,
			*newNode,
			*rootCopy;

		bool need_retry = false;
		while (true) {
			node = child = newNode = nullptr;
			rootCopy = root;
			need_retry = false;

			// Ensures the root & rootCopy is valid & alive
			if (rootCopy == nullptr) {
				newNode = AllocNode();
				if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), newNode, rootCopy) == rootCopy) {
					// exchange worked
				}
				else {					
					FreeNode(newNode); // undo allocation
					need_retry = true;
					continue; // try again
				}			
			}

			// if the root is too large, it must be split
			if (rootCopy->numChildren >= maxChildrenPerNode) {
				SplitRoot(); // will be repeated as-needed until successful. 				
				continue;
			}

			newNode = AllocNode();
			newNode->key = key;
			newNode->object = object;

			history_stack history;
			for (node = rootCopy; node->numChildren > 0; node = child) {
				if (key > node->key) node->key = key; // race condition
				
	            // check for conflict with existing operation
				//if (node->marked > 0) { need_retry = true; FreeNode(newNode); break; }

				// find the first child with a key larger equal to the key of the new node
				history.push_back({ node, 0 });
				for (child = node->children[0]; 
					child->next(history.back().parent, history.back().parent_index); 
					child = child->next(history.back().parent, history.back().parent_index), history.back().parent_index++)
					if (key <= child->key) break;

				// check for conflict with existing operation
				//if (node->marked > 0) { need_retry = true; FreeNode(newNode); break; }

				if (child->object) {
					auto* copiedNode = CopyNode(node);
					copiedNode->insert_child(newNode);
					if (history.size() <= 1) {
						// performing insert at root. 
						// copiedNode is therefore a copy of root, with an inserted child. Simply swap-in this new root. 
						if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), copiedNode, rootCopy) == rootCopy) {
							// successful swap
							FreeNode(node);
							return newNode;
						}
						else {
							// failure to swap-in
							FreeNode(copiedNode);
							FreeNode(newNode);
							need_retry = true;
							break;
						}
					}
					else {
						history.pop_back();
						auto parent_index_to_swap_at = history.back().parent_index;
						auto* parent_to_swap_at = history.back().parent;

						if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&parent_to_swap_at->children[parent_index_to_swap_at]), copiedNode, node) == node) {
							// successful swap
							FreeNode(node);
							return newNode;
						}
						else {
							// failure to swap-in
							FreeNode(copiedNode);
							FreeNode(newNode);
							need_retry = true;
							break;
						}
					}
				}

				// make sure the child has room to store another node. If not, expand and start all over.
				if (child->numChildren >= maxChildrenPerNode) {
					SplitNode(child); // reevaluate this
					FreeNode(newNode);
					need_retry = true;
					break;
				}
			}

			if (!need_retry) {
				// InterlockedIncrement(reinterpret_cast<volatile long*>(&rootCopy->marked));

				auto* copiedRoot = CopyNode(rootCopy);
				copiedRoot->key = key;
				copiedRoot->children[0] = newNode;
				copiedRoot->numChildren = 1;

				if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), copiedRoot, rootCopy) == rootCopy) {
					// success
					FreeNode(rootCopy);
					return newNode;
				}
				else {
					// failure -- try again
					FreeNode(copiedRoot);
					FreeNode(newNode);
				}
			}
		}
	};
	// remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
	__declspec(noinline) void 
		Remove(aTreeNode* node) {
		aTreeNode* start_node = node;
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		if (!node || !root || (root->numChildren <= 0)) return;

		// make sure there are no parent nodes with a single child
		bool retry = false;
		while (true) {
			node = start_node;
			retry = false;	

			bool found = false;
			// step 1, find the node
			history_stack history;
			history.push_back({ root, 0 });
			aTreeNode* Node;
			for (Node = root->children[0]; Node && (Node->numChildren > 0);
				history.push_back({ Node, 0 }), 
				Node = Node->children[0]				
			) {
				while (Node->next(history.back().parent, history.back().parent_index) && (Node->key < node->key)) {
					Node = Node->next(history.back().parent, history.back().parent_index);
					history.back().parent_index++;
				}
			}
			while (Node->next(history.back().parent, history.back().parent_index) && (Node != node)) {
				if (Node == node) {
					break;
				}
				Node = Node->next(history.back().parent, history.back().parent_index);
				history.back().parent_index++;
			}	
					
			while (!history.empty() && (history.back().parent != root)) {
				// we are removing this node from a deeper child node (e.g. not the root)
				if (history.back().parent->numChildren >= 2) {
					// removing this node should still keep the parent node "valid", therefore this is the "normal" path
					auto origParentPtr = history.back().parent;
					auto copiedParent = CopyNode(origParentPtr);
					copiedParent->remove_child(node);

					history.pop_back();

					if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&history.back().parent->children[history.back().parent_index]), copiedParent, origParentPtr) == origParentPtr) {
						// success
						FreeNode(origParentPtr);
						FreeNode(node); // actually free the node
						return;
					}
					else {
						// failure
						FreeNode(copiedParent);
						retry = true;
						continue;
					}
				}
				else {
					// removing this node will result in the parent node being empty, 
					// and therefore we need to actually remove this parent from it's own parent node.
					node = history.back().parent;
					history.pop_back();
				}
			}

			if (history.back().parent == root) {
				auto copiedRoot = CopyNode(history.back().parent);
				// we are removing this node from the root.
				if (copiedRoot->numChildren > 2) {
					// removing this node should still keep the root "valid", therefore this is the "normal" path
					copiedRoot->remove_child(node);
					if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), copiedRoot, history.back().parent) == history.back().parent) {
						// success
						FreeNode(history.back().parent); // free the old root
						FreeNode(node); // actually free the node
						return;
					}
					else {
						// failure -- try again
						retry = true;
						FreeNode(copiedRoot);
						continue;
					}
				}
				else if (copiedRoot->numChildren == 2) {
					// removing this node will result in the root having one child. 
					// Therefore make the child the root instead (if it is a branch node)
					short new_root_index = 1 - history.back().parent_index;
					if (!history.back().parent->children[new_root_index]->object) {
						auto newRoot = CopyNode(history.back().parent->children[new_root_index]);
						if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), newRoot, history.back().parent) == history.back().parent) {
							// success
							FreeNode(copiedRoot);
							FreeNode(node); // actually free the node
							FreeNode(history.back().parent); // free the old root							
							return;
						}
						else {
							// failure -- try again
							retry = true;
							FreeNode(copiedRoot);
							FreeNode(newRoot);
							continue;
						}
					}
					else {
						// removing this node should still keep the root "valid", therefore this is the "normal" path
						copiedRoot->remove_child(node);
						if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), copiedRoot, history.back().parent) == history.back().parent) {
							// success
							FreeNode(history.back().parent); // free the old root
							FreeNode(node); // actually free the node
							return;
						}
						else {
							// failure -- try again
							retry = true;
							FreeNode(copiedRoot);
							continue;
						}
					}
				}
				if (1) {
					// removing this node will result in an empty root. 
					copiedRoot->remove_child(node);
					if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), copiedRoot, history.back().parent) == history.back().parent) {
						// success
						FreeNode(history.back().parent); // free the old root
						FreeNode(node); // actually free the node
						return;
					}
					else {
						// failure -- try again
						retry = true;
						FreeNode(copiedRoot);
						continue;
					}
				}
			}

			if (!retry) return;
		}
	};
	// find an object using the given key
	__declspec(noinline) aTreeNode* 
		NodeFind(keyType key) const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		if (root == nullptr) return nullptr;

		history_stack history;
		history.push_back({ root, 0 });
		for (aTreeNode* node = root->children[0]; node; history.push_back({ node, 0 }), node = node->children[0]) {
			while (node->next(history.back().parent, history.back().parent_index)) {
				if (node->key >= key) break;
				node = node->next(history.back().parent, history.back().parent_index);
				history.back().parent_index++;
			}
			if (node->object) {
				if (node->key == key) return node;
				else return nullptr;
			}
		}
		return nullptr;
	};
	// find an object with the smallest key larger equal the given key
	aTreeNode* 
		NodeFindSmallestLargerEqual(keyType key) const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		if (root == nullptr) return nullptr;

		history_stack history;
		history.push_back({ root, 0 });
		for (aTreeNode* node = root->children[0]; node; history.push_back({ node, 0 }), node = node->children[0]) {
			while (node->next(history.back().parent, history.back().parent_index)) {
				if (node->key >= key) break;
				node = node->next(history.back().parent, history.back().parent_index);
				history.back().parent_index++;
			}
			if (node->object) {
				if (node->key >= key) return node;
				else return nullptr;
			}
		}
		return nullptr;
	};
	// find an object with the largest key smaller equal the given key
	aTreeNode* 
		NodeFindLargestSmallerEqual(keyType key) const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		aTreeNode
			* node,
			* smaller;
		if (root == nullptr) return nullptr;

		history_stack history;
		history.push_back({ root, 0 });
		for (node = root->children[0], smaller = nullptr; node; history.push_back({ node, 0 }), node = node->children[0]) {
			while (node->next(history.back().parent, history.back().parent_index)) {
				if (node->key >= key) break;
				smaller = node;
				node = node->next(history.back().parent, history.back().parent_index);
				history.back().parent_index++;
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
	objType* 
		Find(keyType key) const {
		if (aTreeNode* node = NodeFind(key)) return node->object;
		else return nullptr;
	};
	// find an object with the smallest key larger equal the given key
	objType* 
		FindSmallestLargerEqual(keyType key) const {
		if (aTreeNode* node = NodeFindSmallestLargerEqual(key)) return node->object;
		else return nullptr;
	};
	// find an object with the largest key smaller equal the given key
	objType* 
		FindLargestSmallerEqual(keyType key) const {
		if (aTreeNode* node = NodeFindLargestSmallerEqual(key)) return node->object;
		else return nullptr;
	};
	// returns the root node of the tree
	aTreeNode* 
		GetRoot() const {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };
		return root;
	};

private:
	aTreeNode* 
		AllocNode() {
		aTreeNode* node = nodeAllocator.Alloc();
		node->key = 0;
		for (short i = 0; i <= maxChildrenPerNode; ++i) node->children[i] = nullptr;
		node->numChildren = 0;
		node->object = nullptr;
		node->marked = 0;
		return node;
	};
	aTreeNode* 
		CopyNode(aTreeNode* copy) {
		aTreeNode* node = nodeAllocator.Alloc();
		node->key = copy->key;
		for (short i = 0; i <= maxChildrenPerNode; ++i) node->children[i] = copy->children[i];
		node->numChildren = copy->numChildren;
		node->object = copy->object;
		node->marked = 0;
		return node;
	};
	void 
		FreeNode(aTreeNode* node) {
		nodeAllocator.Free(node);
	};
	void // splits a typical child node. This has not yet been converted to be atomic. 
		SplitNode(aTreeNode* node) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		int i;
		aTreeNode* child, * newNode;
		bool found = false;

		// step 1, find the node
		history_stack history;
		history.push_back({ root, 0 });
		for (aTreeNode* Node = root->children[0]; Node;
			history.push_back({ Node, 0 }),
			Node = Node->children[0]
			) {
			if (Node == node) { found = true; break; }
			while (Node->next(history.back().parent, history.back().parent_index)) {
				if (Node == node) { found = true; break; }
				Node = Node->next(history.back().parent, history.back().parent_index);
				history.back().parent_index++;
			}
			if (Node == node) { found = true; break; }
		}
		if (!found) {
			return;
		}

		// allocate a new node
		newNode = AllocNode();
		//newNode->parent = node->parent;

		// divide the children over the two nodes
		int sz = node->numChildren / 2;
		for (int i = 0; i < sz; ++i) {
			newNode->children[i] = node->children[0];		
			node->remove_child(node->children[0]);
			newNode->key = newNode->children[i]->key;
			newNode->numChildren++;
		}

		history.back().parent->insert_child(newNode);
		// node->parent->insert_child(newNode);
	};
	void // splits the root node exlusively
		SplitRoot() {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		int i;
		aTreeNode 
			*newNode,
			*oldRoot,
			*rootCopy,
			*newRoot
			;
		bool found = false;
		oldRoot = root;
		rootCopy = CopyNode(oldRoot);

		if (rootCopy->numChildren >= maxChildrenPerNode) {
			// InterlockedIncrement(reinterpret_cast<volatile long*>(&oldRoot->marked)); // marks the node for eventual erasure

			// allocate a new node
			newNode = AllocNode();
			// divide the children over the two nodes
			int sz = rootCopy->numChildren / 2;
			for (int i = 0; i < sz; ++i) {
				newNode->children[i] = rootCopy->children[0];
				rootCopy->remove_child(rootCopy->children[0]);
				newNode->key = newNode->children[i]->key;
				newNode->numChildren++;
			}

			newRoot = AllocNode();
			newRoot->key = rootCopy->key;
			newRoot->children[0] = newNode;
			newRoot->children[1] = rootCopy;
			newRoot->numChildren = 2;

			if (InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&root), newRoot, oldRoot) == oldRoot) {
				// success
				FreeNode(oldRoot);
			}
			else {
				FreeNode(newNode);
				FreeNode(rootCopy);
				FreeNode(newRoot);
			}
		}
		else {
			FreeNode(rootCopy);
		}
	};

};