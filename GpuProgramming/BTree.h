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

// version of a b-tree that uses fine-grained locks to make it thread-safe. Survives single-threaded and multi-threaded tests. 
template< class objType, class keyType, int maxChildrenPerNode >
class cTree {
public:
	struct cTreeNode {
		keyType	// key used for sorting						
			key;
		objType // if != nullptr pointer to object stored in leaf node
			* object;
		cTreeNode // parent node
			* parent;
		cTreeNode // next sibling
			* next;
		cTreeNode // prev sibling
			* prev;
		int	// number of children							
			numChildren;
		cTreeNode // first child
			* firstChild;
		cTreeNode // last child
			* lastChild;
		std::shared_mutex // mutex
			mut;
	};

private:
	std::shared_mutex
		mut; // global tree lock. SHould only be held temporarily if at all possible. 
	cTreeNode*
		root; 
	GL::atomic_epoch_allocator< cTreeNode, GL::atomic_allocator<cTreeNode> >
		nodeAllocator; // necessary to use the Epoch allocator since we need the mutexes within nodes to survive slightly after the Free call is made on the node itself

	class // exclusive lock manager. Allows push'ing or pop'ing scoped mutex locks. 
		locker {
	public:
		std::deque<std::shared_ptr<std::scoped_lock<std::shared_mutex>>>
			locks;

		void push_back(std::shared_mutex& source) {
			locks.push_back(std::make_shared<std::scoped_lock<std::shared_mutex>>(source));
		};
		void pop_back() {
			locks.pop_back();
		};
		void pop_front() {
			locks.pop_front();
		};
	};
	
	class // shared lock manager. Allows push'ing or pop'ing shared mutex locks. 
		slocker {
	public:
		std::deque<std::shared_ptr<std::shared_lock<std::shared_mutex>>>
			locks;

		void push_back(std::shared_mutex& source) {
			locks.push_back(std::make_shared<std::shared_lock<std::shared_mutex>>(source));
		};
		void pop_back() {
			locks.pop_back();
		};
		void pop_front() {
			locks.pop_front();
		};
	};

public:
	cTree() 
		: nodeAllocator()
		, root{ nullptr } 
		, mut()
	{
		root = AllocNode();
	};
	cTree(cTree const&)
		= delete;
	cTree(cTree&&) noexcept
		= delete;
	cTree& operator=(cTree const&)
		= delete;
	cTree& operator=(cTree&&) noexcept
		= delete;
	~cTree() = default;

	cTreeNode* // add an object to the tree
		Add(objType* object, keyType key) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		cTreeNode
			*node,
			*child,
			*newNode;

		locker locking;
		locking.push_back(mut);

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

		locking.push_back(root->mut);
		for (node = root; node->firstChild != nullptr; node = child, locking.push_back(child->mut)) {			
			if (node == root) locking.pop_front();

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
	
	void // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
		Remove(cTreeNode* node) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		cTreeNode
			*Node,
			*parent,
			*oldRoot;

		// acquire all relevant locks before we perform the deletion
		locker locking;
		if (1) {
			locking.push_back(mut); // get the global tree lock
			if (!root || !root->firstChild) return;
			locking.push_back(root->mut);
			locking.pop_front(); // release the global tree lock
			locking.push_back(root->firstChild->mut);
			for (Node = root->firstChild; Node != nullptr; ) {
				while (Node->next) {
					if (Node->key >= node->key) break;
					Node = Node->next;
				}
				if (Node->object) {
					if (Node == node) break; // found
					else return; // doesn't exist
				}

				if (!Node) return; // doesn't exist
				if (!Node->firstChild) return; // doesn't exist

				locking.push_back(Node->firstChild->mut);

				Node = Node->firstChild;
			}
		}

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
			oldRoot = root;
			root->firstChild->parent = nullptr;
			root = root->firstChild;
			FreeNode(oldRoot);
		}
	};
	
	__declspec(noinline) cTreeNode* // find an object using the given key
		NodeFind(keyType key) {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		cTreeNode
			*node;

		slocker locking;
		locking.push_back(mut);
		if (!root || !root->firstChild) return nullptr;
		locking.push_back(root->mut);		
		locking.pop_front();
		locking.push_back(root->firstChild->mut);
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				node = node->next;
			}
			if (node->object) {
				if (node->key == key) return node;
				else return nullptr;
			}

			if (!node) return nullptr;
			if (!node->firstChild) return nullptr;

			locking.push_back(node->firstChild->mut);
			locking.pop_front();

			node = node->firstChild;			
		}
		return nullptr;
	};
	

	cTreeNode* // find an object with the smallest key larger equal the given key
		NodeFindSmallestLargerEqual(keyType key) {
		cTreeNode
			* node;

		slocker locking;
		locking.push_back(mut);
		if (!root || !root->firstChild) return nullptr;
		locking.push_back(root->mut);
		locking.pop_front();
		locking.push_back(root->firstChild->mut);
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				node = node->next;
			}
			if (node->object) {
				if (node->key >= key) return node;
				else return nullptr;
			}

			if (!node) return nullptr;
			if (!node->firstChild) return nullptr;

			locking.push_back(node->firstChild->mut);
			locking.pop_front();

			node = node->firstChild;
		}
		return nullptr;
	};

	cTreeNode* // find an object with the largest key smaller equal the given key
		NodeFindLargestSmallerEqual(keyType key) {
		cTreeNode
			* node,
			* smaller;

		slocker locking;
		locking.push_back(mut);
		if (!root || !root->firstChild) return nullptr;
		locking.push_back(root->mut);
		locking.pop_front();
		locking.push_back(root->firstChild->mut);

		for (node = root->firstChild, smaller = nullptr; node != nullptr; ) {
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

			if (!node) return nullptr;
			if (!node->firstChild) return nullptr;

			locking.push_back(node->firstChild->mut);
			// locking.pop_front();

			node = node->firstChild;
		}
		return nullptr;
	};

	objType* // find an object using the given key
		Find(keyType key) {
		cTreeNode
			* node;

		if (node = NodeFind(key)) return node->object;
		else return nullptr;
	};
	
	objType* // find an object with the smallest key larger equal the given key
		FindSmallestLargerEqual(keyType key) {
		cTreeNode
			* node;

		if (node = NodeFindSmallestLargerEqual(key)) return node->object;
		else return nullptr;
	};
	
	objType* // find an object with the largest key smaller equal the given key
		FindLargestSmallerEqual(keyType key) {
		cTreeNode
			* node;

		if (node = NodeFindLargestSmallerEqual(key)) return node->object;
		else return nullptr;
	};
	
	std::pair<cTreeNode*, slocker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
		GetRoot() {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		std::pair<cTreeNode*, slocker> out;
		out.second.push_back(mut);
		out.first = root;
		out.second.push_back(root->mut);
		out.second.pop_front();
		return out;
	};
	
	static cTreeNode* // goes through all nodes of the tree		
		GetNext(cTreeNode* node, slocker& locking) {
		if (node->firstChild) {
			locking.push_back(node->firstChild->mut);
			// locking.pop_front();
			return node->firstChild;
		}
		else {
			while (node && (node->next == nullptr)) {	
				locking.pop_back();
				node = node->parent;
			}
			return node;
		}

	};
	
	static cTreeNode* // goes through all leaf nodes of the tree		
		GetNextLeaf(cTreeNode* node, slocker& locking) {
		if (node->firstChild) {			
			while (node->firstChild) {
				locking.push_back(node->firstChild->mut);
				node = node->firstChild;
			}
			return node;
		}
		else {
			while (node && (node->next == nullptr)) {
				locking.pop_back();
				node = node->parent;
			}
			if (node) {
				node = node->next;
				while (node->firstChild) {
					locking.push_back(node->firstChild->mut);
					node = node->firstChild;
				}
				return node;
			}
			else return nullptr;
		}
	};




private:
	cTreeNode*
		AllocNode() {
		cTreeNode
			*node;

		node = nodeAllocator.Alloc();
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
		FreeNode(cTreeNode* node) {
		nodeAllocator.Free(node);
	};
	void
		SplitNode(cTreeNode* node) {
		int i;
		cTreeNode
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
	cTreeNode*
		MergeNodes(cTreeNode* node1, cTreeNode* node2) {
		cTreeNode
			* child;

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

