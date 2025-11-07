#pragma once
#include "dynamic_allocator.h"
#include "../ScriptLanguageTesting/atomic_allocator.h"
#include "../ScriptLanguageTesting/atomic_maps.h"

// Single-threaded version of the B-Tree. Fastest version, but is not thread-safe. Nodes are invalidated if removed.
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

// Multi-threaded version of a B-Tree that uses many fine-grained locks and an epoch-based GC to make it thread-safe. Nodes are temporarily valid even after removal, so long as the current thread does not continue to do work on the tree.
template< class objType, class keyType, int maxChildrenPerNode >
class parallel_fine_bTree {
public:
	using lock_type = std::shared_mutex;
	struct parallel_fine_bTreeNode {
		keyType	// key used for sorting						
			key;
		objType // if != nullptr pointer to object stored in leaf node
			*object;
		parallel_fine_bTreeNode // parent node
			*parent;
		parallel_fine_bTreeNode // next sibling
			*next;
		parallel_fine_bTreeNode // prev sibling
			*prev;
		int	// number of children							
			numChildren;
		parallel_fine_bTreeNode // first child
			*firstChild;
		parallel_fine_bTreeNode // last child
			*lastChild;
		lock_type // mutex
			mut;
	};

private:
	lock_type
		mut; // global tree lock. Should only be held temporarily if at all possible. 
	parallel_fine_bTreeNode*
		root; 
	GL::atomic_epoch_allocator< parallel_fine_bTreeNode, GL::atomic_parallel_allocator<parallel_fine_bTreeNode, 128, false> >
		nodeAllocator; // necessary to use the Epoch allocator since we need the mutexes within nodes to survive slightly after the Free call is made on the node itself
	std::atomic<long>
		count;
	class // exclusive lock manager. Allows push'ing or pop'ing scoped mutex locks. 
		locker {
	public:
		std::deque<std::shared_ptr<void>>
			locks;
	public:
		void // store a shared lock
			push_back(lock_type& source) {
			locks.push_back(std::static_pointer_cast<void>(std::make_shared<std::scoped_lock<lock_type>>(source)));
		};
		void // store a shared lock
			push_back_shared(lock_type& source) {
			locks.push_back(std::static_pointer_cast<void>(std::make_shared<std::shared_lock<lock_type>>(source)));
		};
		void // store a shared lock
			push_pop(lock_type& source) {
			locks.back() = std::static_pointer_cast<void>(std::make_shared<std::scoped_lock<lock_type>>(source));
		};
		void // store a shared lock
			push_pop_shared(lock_type& source) {
			locks.back() = std::static_pointer_cast<void>(std::make_shared<std::shared_lock<lock_type>>(source));
		};
		void // remove the youngest lock
			pop_back() {
			locks.pop_back();
		};
		void // remove the oldest lock
			pop_front() {
			locks.pop_front();
		};
		size_t // count of locks
			size() const {
			return locks.size();
		};
		void // clear all locks
			clear() {
			locks.clear();
		};
	};	

public:
	parallel_fine_bTree() 
		: nodeAllocator()
		, root{ nullptr } 
		, mut()
		, count{ 0 }
	{ 
		root = AllocNode(); 
	};
	parallel_fine_bTree(parallel_fine_bTree const&)
		= delete;
	parallel_fine_bTree(parallel_fine_bTree&&) noexcept
		= delete;
	parallel_fine_bTree& operator=(parallel_fine_bTree const&)
		= delete;
	parallel_fine_bTree& operator=(parallel_fine_bTree&&) noexcept
		= delete;
	~parallel_fine_bTree() 
		= default;
	parallel_fine_bTreeNode* // add an object to the tree
		Add(objType* object, keyType key) {
		parallel_fine_bTreeNode
			*node,
			*child,
			*newNode;
		auto 
			guarded = nodeAllocator.ProtectCurrentEpoch();
		locker
			locking;
		bool
			optimistic = true; // (count.load() / maxChildrenPerNode) > (maxChildrenPerNode / 2);
		bool
			completed = true;
		newNode 
			= AllocNode();
		newNode->key 
			= key;
		newNode->object 
			= object;

		while (true) {
			completed = true;

			if (locking.size() > 0) locking.clear();

			locking.push_back(mut);

			if (root == nullptr) {
				root = AllocNode();
				continue;
			}

			locking.push_back(root->mut);

			if (root->numChildren >= maxChildrenPerNode) {
				node = AllocNode();
				node->key = root->key;
				node->firstChild = root;
				node->lastChild = root;
				node->numChildren = 1;
				root->parent = node;
				SplitNode(root);
				root = node;
				node = nullptr;
				continue;
			}

			if (root->key < newNode->key) optimistic = false;

			for (node = root; node->firstChild; node = child) {
				if (node == root) locking.pop_front();

				if (key > node->key) {
					if (optimistic) {
						optimistic = false;
						completed = false;
						break;
					}
					node->key = key;
				}

				// find the first child with a key larger equal to the key of the new node
				for (child = node->firstChild; child->next; child = child->next)
					if (key <= child->key)
						break;

				// we are inside of a branch of leafs -- we will do the insert.
				if (child->object) {
					while (locking.size() >= 3) locking.pop_front(); // only holds the lock on the parent for this child & newNode
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
					++count;
					return newNode;
				}
				else if (child->numChildren >= maxChildrenPerNode) {
					if (optimistic) {
						optimistic = false;
						completed = false;
						break;
					}
					SplitNode(child);
					if (key <= child->prev->key) child = child->prev;
				}

				locking.push_back(child->mut);
				if (optimistic) {
					while (locking.size() >= 3) locking.pop_front();
				}
			}

			if (completed) {
				// we only end up here if the root node is empty
				newNode->parent = root;
				root->key = key;
				root->firstChild = newNode;
				root->lastChild = newNode;
				root->numChildren++;
				++count;
				return newNode;
			}
		}
	};	
	bool // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
		Remove(parallel_fine_bTreeNode* node, locker const& Locking = locker()) {
		parallel_fine_bTreeNode
			*Node,
			*parent,
			*oldRoot;
		auto 
			guarded = nodeAllocator.ProtectCurrentEpoch();
		locker&
			locking = const_cast<locker&>(Locking);

		// acquire all relevant locks before we perform the deletion
		if (locking.size() == 0) {			
			locking.push_back(mut); // get the global tree lock
			if (!root || !root->firstChild) return false;
			locking.push_back(root->mut);
			// locking.pop_front(); // release the global tree lock
			locking.push_back(root->firstChild->mut);
			for (Node = root->firstChild; Node != nullptr; ) {
				while (Node->next) {
					if (Node->key >= node->key) break;
					Node = Node->next;
				}
				if (Node->object) {
					if (Node->object == node->object) break; // found
					else return false; // doesn't exist
				}

				if (!Node) return false; // doesn't exist
				if (!Node->firstChild) return false; // doesn't exist

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
		for (parent = node->parent; (parent != root) && (parent->numChildren <= 1); parent = parent->parent/*, locking.pop_back()*/) {
			if (parent->next) parent = MergeNodes(parent, parent->next);
			else if (parent->prev) parent = MergeNodes(parent->prev, parent);

			// a parent may not use a key higher than the key of its last child
			if ((parent->numChildren > 0) && parent->lastChild)
				if (parent->key > parent->lastChild->key) 
					parent->key = parent->lastChild->key;

			if (parent->numChildren > maxChildrenPerNode) {
				SplitNode(parent);
				break;
			}
		}
		// a parent may not use a key higher than the key of it's last child
		for (; parent && parent->lastChild; parent = parent->parent) {
			if (parent->key > parent->lastChild->key)
				parent->key = parent->lastChild->key;
			/*if (locking.size() > 2) locking.pop_back();*/
		}

		// actually free the node
		--count;
		FreeNode(node);

		// while (locking.size() > 2) locking.pop_back();

		// remove the root node if it has a single internal node as child		
		if ((root->numChildren == 1) && (root->firstChild->object == nullptr)) {
			oldRoot = root;

			locking.push_back(root->firstChild->mut);

			root->firstChild->parent = nullptr;
			root = root->firstChild;
			FreeNode(oldRoot);
		}

		return true;
	};	
	std::pair<parallel_fine_bTreeNode*, locker> // find an object using the given key
		NodeFind(keyType key) {
		auto guarded{
			nodeAllocator.ProtectCurrentEpoch() };
		parallel_fine_bTreeNode
			*nxt;
		std::pair<parallel_fine_bTreeNode*, locker>
			out;
		parallel_fine_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back_shared(mut);
		if (!root || !root->firstChild) {
			locking.clear();
			node = nullptr;
			return out;
		}
		locking.push_back_shared(root->mut);
		locking.pop_front();
		locking.push_back_shared(root->firstChild->mut);
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				nxt = node->next;
				locking.push_pop_shared(node->next->mut);
				node = nxt;
			}
			if (locking.size() >= 3) locking.pop_front();

			if (node->object) {
				if (node->key == key) return out;
				else {
					locking.clear();
					node = nullptr;
					return out;
				}
			}
			if (!node || !node->firstChild) {
				locking.clear();
				node = nullptr;
				return out;
			}

			locking.push_back_shared(node->firstChild->mut);
			node = node->firstChild;
		}		
		locking.clear();
		node = nullptr;
		return out;		
	};	
	std::pair<parallel_fine_bTreeNode*, locker> // find an object using the given key
		NodeFind_ForRemoval(keyType key) {	
		auto guarded{
			nodeAllocator.ProtectCurrentEpoch() };
		std::pair<parallel_fine_bTreeNode*, locker>
			out;
		parallel_fine_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			locking.clear();
			node = nullptr;
			return out;
		}
		locking.push_back(root->mut);
		// locking.pop_front();
		locking.push_back(root->firstChild->mut);
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				node = node->next;
			}
			if (node->object) {
				if (node->key == key) return out;
				else {
					locking.clear();
					node = nullptr;
					return out;
				}
			}

			if (!node || !node->firstChild) {
				locking.clear();
				node = nullptr;
				return out;
			}

			locking.push_back(node->firstChild->mut);

			node = node->firstChild;
		}		
		locking.clear();
		node = nullptr;
		return out;		
	};
	std::pair<parallel_fine_bTreeNode*, locker> // find an object with the smallest key larger equal the given key
		NodeFindSmallestLargerEqual(keyType key) {
		auto 
			guarded = nodeAllocator.ProtectCurrentEpoch();
		parallel_fine_bTreeNode
			*nxt;
		std::pair<parallel_fine_bTreeNode*, locker>
			out;
		parallel_fine_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back_shared(mut);
		if (!root || !root->firstChild) {
			locking.clear();
			node = nullptr;
			return out;
		}
		locking.push_back_shared(root->mut);
		locking.pop_front();
		locking.push_back_shared(root->firstChild->mut);
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				nxt = node->next;
				locking.push_pop_shared(nxt->mut);
				node = nxt;
			}
			if (locking.size() >= 3) locking.pop_front();
			if (node->object) {
				if (node->key >= key) return out;
				else {
					locking.clear();
					node = nullptr;
					return out;
				}
			}

			if (!node || !node->firstChild) {
				locking.clear();
				node = nullptr;
				return out;
			}

			locking.push_back_shared(node->firstChild->mut);

			node = node->firstChild;
		}
		{
			locking.clear();
			node = nullptr;
			return out;
		}
	};
	std::pair<parallel_fine_bTreeNode*, locker> // find an object with the largest key smaller equal the given key
		NodeFindLargestSmallerEqual(keyType key) {
		auto
			guarded = nodeAllocator.ProtectCurrentEpoch();
		parallel_fine_bTreeNode
			*smaller;
		std::pair<parallel_fine_bTreeNode*, locker>
			out;
		parallel_fine_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			locking.clear();
			node = nullptr;
			return out;
		}
		locking.push_back_shared(root->mut);
		locking.pop_front();
		locking.push_back_shared(root->firstChild->mut);

		for (node = root->firstChild, smaller = nullptr; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				smaller = node;
				node = node->next;
			}
			if (node->object) {
				if (node->key <= key) return out;
				else if (smaller == nullptr) {
					locking.clear();
					node = nullptr;
					return out;
				}
				else {
					node = smaller;
					if (node->object) return out;
				}
			}
			if (!node || !node->firstChild) {
				locking.clear();
				node = nullptr;
				return out;
			}
			locking.push_back_shared(node->firstChild->mut);
			node = node->firstChild;
		}		
		locking.clear();
		node = nullptr;
		return out;		
	};

	std::pair<parallel_fine_bTreeNode*, locker> // find an object with the smallest key larger equal the given key
		NodeFindSmallestLargerEqual_ForRemoval(keyType key) {
		auto
			guarded = nodeAllocator.ProtectCurrentEpoch();
		parallel_fine_bTreeNode
			* nxt;
		std::pair<parallel_fine_bTreeNode*, locker>
			out;
		parallel_fine_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			locking.clear();
			node = nullptr;
			return out;
		}
		locking.push_back(root->mut);
		// locking.pop_front();
		locking.push_back(root->firstChild->mut);
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				nxt = node->next;
				locking.push_pop(nxt->mut);
				node = nxt;
			}
			// if (locking.size() >= 3) locking.pop_front();
			if (node->object) {
				if (node->key >= key) return out;
				else {
					locking.clear();
					node = nullptr;
					return out;
				}
			}

			if (!node || !node->firstChild) {
				locking.clear();
				node = nullptr;
				return out;
			}

			locking.push_back(node->firstChild->mut);

			node = node->firstChild;
		}
		{
			locking.clear();
			node = nullptr;
			return out;
		}
	};
	std::pair<parallel_fine_bTreeNode*, locker> // find an object with the largest key smaller equal the given key
		NodeFindLargestSmallerEqual_ForRemoval(keyType key) {
		auto
			guarded = nodeAllocator.ProtectCurrentEpoch();
		parallel_fine_bTreeNode
			* smaller;
		std::pair<parallel_fine_bTreeNode*, locker>
			out;
		parallel_fine_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			locking.clear();
			node = nullptr;
			return out;
		}
		locking.push_back(root->mut);
		// locking.pop_front();
		locking.push_back(root->firstChild->mut);

		for (node = root->firstChild, smaller = nullptr; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				smaller = node;
				node = node->next;
			}
			if (node->object) {
				if (node->key <= key) return out;
				else if (smaller == nullptr) {
					locking.clear();
					node = nullptr;
					return out;
				}
				else {
					node = smaller;
					if (node->object) return out;
				}
			}
			if (!node || !node->firstChild) {
				locking.clear();
				node = nullptr;
				return out;
			}
			locking.push_back(node->firstChild->mut);
			node = node->firstChild;
		}
		locking.clear();
		node = nullptr;
		return out;
	};

	std::pair<parallel_fine_bTreeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
		GetRoot() {
		auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		std::pair<parallel_fine_bTreeNode*, locker> out;
		out.second.push_back(mut);
		out.first = root;
		out.second.push_back(root->mut);
		out.second.pop_front();
		return out;
	};	
	static parallel_fine_bTreeNode* // goes through all nodes of the tree		
		GetNext(parallel_fine_bTreeNode* node, locker& locking) {
		if (node->firstChild) {
			locking.push_back_shared(node->firstChild->mut);
			// locking.pop_front();
			return node->firstChild;
		}
		else {
			while (node && (node->next == nullptr)) {	
				locking.pop_back_shared();
				node = node->parent;
			}
			return node;
		}

	};	
	static parallel_fine_bTreeNode* // goes through all leaf nodes of the tree		
		GetNextLeaf(parallel_fine_bTreeNode* node, locker& locking) {
		parallel_fine_bTreeNode*
			nxt;
		if (node->firstChild) {			
			while (node->firstChild) {
				locking.push_back_shared(node->firstChild->mut);
				node = node->firstChild;
			}
			return node;
		}
		else {
			while (node && (node->next == nullptr)) {
				nxt = node->parent;
				locking.pop_back();
				node = nxt;
			}
			if (node) {
				nxt = node->next;
				locking.push_pop_shared(nxt->mut);
				node = nxt;
				while (node->firstChild) {
					locking.push_back_shared(node->firstChild->mut);
					node = node->firstChild;
				}
				return node;
			}
			else return nullptr;
		}
	};

private:
	parallel_fine_bTreeNode*
		AllocNode() {
		parallel_fine_bTreeNode
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
		FreeNode(parallel_fine_bTreeNode* node) {
		nodeAllocator.Free(node);
	};
	void // does not lock
		SplitNode(parallel_fine_bTreeNode* node) {
		int 
			i;
		parallel_fine_bTreeNode
			*child, 
			*newNode;

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
	};;
	parallel_fine_bTreeNode* // does not lock
		MergeNodes(parallel_fine_bTreeNode* node1, parallel_fine_bTreeNode* node2) {
		parallel_fine_bTreeNode
			* child;

		for (child = node1->firstChild; child->next; child = child->next) child->parent = node2;		
		child->parent = node2;
		child->next = node2->firstChild;
		node2->firstChild->prev = child;
		node2->firstChild = node1->firstChild;
		node2->numChildren += node1->numChildren;
		if (node1->prev) node1->prev->next = node2; // unlink the first node from the parent
		else node1->parent->firstChild = node2;	// unlink the first node from the parent	
		node2->prev = node1->prev;
		node2->parent->numChildren--;
		FreeNode(node1);
		return node2;
	};

};

// Multi-threaded version of a B-Tree that uses a course-grained lock with parallel allocator to make it thread-safe. Nodes are at-risk of disposal once the lock is returned.
template< class objType, class keyType, int maxChildrenPerNode >
class parallel_course_bTree {
public:
	using lock_type = std::shared_mutex;
	struct parallel_course_bTreeNode {
		keyType	// key used for sorting						
			key;
		objType // if != nullptr pointer to object stored in leaf node
			*object;
		parallel_course_bTreeNode // parent node
			*parent;
		parallel_course_bTreeNode // next sibling
			*next;
		parallel_course_bTreeNode // prev sibling
			*prev;
		int	// number of children							
			numChildren;
		parallel_course_bTreeNode // first child
			*firstChild;
		parallel_course_bTreeNode // last child
			*lastChild;
	};

private:
	lock_type
		mut; // global tree lock. Should only be held temporarily if at all possible. 
	parallel_course_bTreeNode*
		root;
	GL::atomic_parallel_allocator< parallel_course_bTreeNode, 128, true >
		nodeAllocator;
	std::atomic<long>
		count;
	class // exclusive lock manager. Allows push'ing or pop'ing scoped mutex locks. 
		locker {
	public:
		std::deque<std::shared_ptr<void>>
			locks;
	public:
		void // store a shared lock
			push_back(lock_type& source) {
			locks.push_back(std::static_pointer_cast<void>(std::make_shared<std::scoped_lock<lock_type>>(source)));
		};
		void // store a shared lock
			push_back_shared(lock_type& source) {
			locks.push_back(std::static_pointer_cast<void>(std::make_shared<std::shared_lock<lock_type>>(source)));
		};
		void // store a shared lock
			push_pop(lock_type& source) {
			locks.back() = std::static_pointer_cast<void>(std::make_shared<std::scoped_lock<lock_type>>(source));
		};
		void // store a shared lock
			push_pop_shared(lock_type& source) {
			locks.back() = std::static_pointer_cast<void>(std::make_shared<std::shared_lock<lock_type>>(source));
		};
		void // remove the youngest lock
			pop_back() {
			locks.pop_back();
		};
		void // remove the oldest lock
			pop_front() {
			locks.pop_front();
		};
		size_t // count of locks
			size() const {
			return locks.size();
		};
		void // clear all locks
			clear() {
			locks.clear();
		};
	};

public:
	parallel_course_bTree()
		: nodeAllocator()
		, root{ nullptr }
		, mut()
		, count{ 0 }
	{
		root = AllocNode();
	};
	parallel_course_bTree(parallel_course_bTree const&)
		= delete;
	parallel_course_bTree(parallel_course_bTree&&) noexcept
		= delete;
	parallel_course_bTree& operator=(parallel_course_bTree const&)
		= delete;
	parallel_course_bTree& operator=(parallel_course_bTree&&) noexcept
		= delete;
	~parallel_course_bTree()
		= default;

	parallel_course_bTreeNode* // add an object to the tree
		Add(objType* object, keyType key) {
		parallel_course_bTreeNode
			* node,
			* child,
			* newNode;
		//auto guarded = nodeAllocator.ProtectCurrentEpoch();
		locker
			locking;
		newNode
			= AllocNode();
		newNode->key
			= key;
		newNode->object
			= object;

		locking.push_back(mut);

		if (root == nullptr) root = AllocNode();			

		if (root->numChildren >= maxChildrenPerNode) {
			node = AllocNode();
			node->key = root->key;
			node->firstChild = root;
			node->lastChild = root;
			node->numChildren = 1;
			root->parent = node;
			SplitNode(root);
			root = node;
			node = nullptr;
		}

		for (node = root; node->firstChild; node = child) {
			if (key > node->key) node->key = key;				

			// find the first child with a key larger equal to the key of the new node
			for (child = node->firstChild; child->next; child = child->next)
				if (key <= child->key)
					break;

			// we are inside of a branch of leafs -- we will do the insert.
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
				++count;
				return newNode;
			}
			else if (child->numChildren >= maxChildrenPerNode) {
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
		++count;
		return newNode;			
		
	};
	bool // remove an object node from the tree. Assumes the user cannot remove branch nodes, and can only request to remove leafs.
		Remove(parallel_course_bTreeNode* node, locker const& Locking = locker()) {
		parallel_course_bTreeNode
			* Node,
			* parent,
			* oldRoot;
		// auto guarded = nodeAllocator.ProtectCurrentEpoch();
		locker&
			locking = const_cast<locker&>(Locking);

		// acquire all relevant locks before we perform the deletion
		if (locking.size() == 0) {
			locking.push_back(mut); // get the global tree lock
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
			if ((parent->numChildren > 0) && parent->lastChild)
				if (parent->key > parent->lastChild->key)
					parent->key = parent->lastChild->key;

			if (parent->numChildren > maxChildrenPerNode) {
				SplitNode(parent);
				break;
			}
		}
		// a parent may not use a key higher than the key of it's last child
		for (; parent && parent->lastChild; parent = parent->parent) {
			if (parent->key > parent->lastChild->key)
				parent->key = parent->lastChild->key;
		}

		// actually free the node
		--count;
		FreeNode(node);

		// remove the root node if it has a single internal node as child		
		if ((root->numChildren == 1) && (root->firstChild->object == nullptr)) {
			oldRoot = root;
			root->firstChild->parent = nullptr;
			root = root->firstChild;
			FreeNode(oldRoot);
		}

		return true;
	};
	std::pair<parallel_course_bTreeNode*, locker> // find an object using the given key
		NodeFind(keyType key) {
		parallel_course_bTreeNode
			* nxt;
		std::pair<parallel_course_bTreeNode*, locker>
			out;
		parallel_course_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back_shared(mut);
		if (!root || !root->firstChild) {
			node = nullptr;
			locking.clear();
			return out;
		}
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				nxt = node->next;
				node = nxt;
			}

			if (node->object) {
				if (node->key == key) return out;
				else {
					node = nullptr;
					locking.clear();
					return out;
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
	std::pair<parallel_course_bTreeNode*, locker> // find an object using the given key
		NodeFind_ForRemoval(keyType key) {
		std::pair<parallel_course_bTreeNode*, locker>
			out;
		parallel_course_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) return out;
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				node = node->next;
			}
			if (node->object) {
				if (node->key == key) return out;
				else return out;
			}

			if (!node) return out;
			if (!node->firstChild) return out;

			node = node->firstChild;
		}
		return out;
	};
	std::pair<parallel_course_bTreeNode*, locker> // find an object with the smallest key larger equal the given key
		NodeFindSmallestLargerEqual(keyType key) {
		parallel_course_bTreeNode
			* nxt;
		std::pair<parallel_course_bTreeNode*, locker>
			out;
		parallel_course_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back_shared(mut);
		if (!root || !root->firstChild) {
			node = nullptr;
			locking.clear();
			return out;
		}
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				nxt = node->next;
				node = nxt;
			}
			if (node->object) {
				if (node->key >= key) return out;
				else {
					node = nullptr;
					locking.clear();
					return out;
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
	std::pair<parallel_course_bTreeNode*, locker> // find an object with the largest key smaller equal the given key
		NodeFindLargestSmallerEqual(keyType key) {
		parallel_course_bTreeNode
			* smaller;
		std::pair<parallel_course_bTreeNode*, locker>
			out;
		parallel_course_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			node = nullptr;
			locking.clear();
			return out;
		}

		for (node = root->firstChild, smaller = nullptr; node != nullptr; ) {
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
	std::pair<parallel_course_bTreeNode*, locker> // find an object with the smallest key larger equal the given key
		NodeFindSmallestLargerEqual_ForRemoval(keyType key) {
		parallel_course_bTreeNode
			* nxt;
		std::pair<parallel_course_bTreeNode*, locker>
			out;
		parallel_course_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			node = nullptr;
			locking.clear();
			return out;
		}
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				nxt = node->next;
				node = nxt;
			}
			if (node->object) {
				if (node->key >= key) return out;
				else {
					node = nullptr;
					locking.clear();
					return out;
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
	std::pair<parallel_course_bTreeNode*, locker> // find an object with the largest key smaller equal the given key
		NodeFindLargestSmallerEqual_ForRemoval(keyType key) {
		parallel_course_bTreeNode
			* smaller;
		std::pair<parallel_course_bTreeNode*, locker>
			out;
		parallel_course_bTreeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			node = nullptr;
			locking.clear();
			return out;
		}

		for (node = root->firstChild, smaller = nullptr; node != nullptr; ) {
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

	std::pair<parallel_course_bTreeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
		GetRoot() {
		// auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		std::pair<parallel_course_bTreeNode*, locker> out;
		out.second.push_back(mut);
		out.first = root;
		return out;
	};
	static parallel_course_bTreeNode* // goes through all nodes of the tree		
		GetNext(parallel_course_bTreeNode* node, locker& locking) {
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
	static parallel_course_bTreeNode* // goes through all leaf nodes of the tree		
		GetNextLeaf(parallel_course_bTreeNode* node, locker& locking) {
		parallel_course_bTreeNode*
			nxt;
		if (node->firstChild) {
			while (node->firstChild) {
				node = node->firstChild;
			}
			return node;
		}
		else {
			while (node && (node->next == nullptr)) {
				nxt = node->parent;
				node = nxt;
			}
			if (node) {
				nxt = node->next;
				node = nxt;
				while (node->firstChild) {
					node = node->firstChild;
				}
				return node;
			}
			else return nullptr;
		}
	};

private:
	parallel_course_bTreeNode*
		AllocNode() {
		parallel_course_bTreeNode
			* node;

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
		FreeNode(parallel_course_bTreeNode* node) {
		nodeAllocator.Free(node);
	};
	void // does not lock
		SplitNode(parallel_course_bTreeNode* node) {
		int
			i;
		parallel_course_bTreeNode
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
	};;
	parallel_course_bTreeNode* // does not lock
		MergeNodes(parallel_course_bTreeNode* node1, parallel_course_bTreeNode* node2) {
		parallel_course_bTreeNode
			* child;

		for (child = node1->firstChild; child->next; child = child->next) child->parent = node2;
		child->parent = node2;
		child->next = node2->firstChild;
		node2->firstChild->prev = child;
		node2->firstChild = node1->firstChild;
		node2->numChildren += node1->numChildren;
		if (node1->prev) node1->prev->next = node2; // unlink the first node from the parent
		else node1->parent->firstChild = node2;	// unlink the first node from the parent	
		node2->prev = node1->prev;
		node2->parent->numChildren--;
		FreeNode(node1);
		return node2;
	};

};

// Multi-threaded version of a B-Tree that uses a course-grained lock with parallel allocator to make it thread-safe. Nodes are at-risk of disposal once the lock is returned.
// Attempts to speed-up searching using a binomial search within BTree nodes. In theory should benefit from larger maxChildrenPerNode values. 
template< class objType, class keyType, int maxChildrenPerNode >
class parallel_binomial_search_tree {
public:
	using lock_type = std::shared_mutex;
	struct parallel_binomial_search_treeNode {
		keyType	// key used for sorting						
			key;
		objType // if != nullptr pointer to object stored in leaf node
			* object;
		parallel_binomial_search_treeNode // parent node
			* parent;
		int	// number of children							
			numChildren;

		// To-Do, make this array a pointer that is not utilized when object != null, e.g. is a leaf node
		parallel_binomial_search_treeNode* 
			children[maxChildrenPerNode];
		int
			parent_index;




		parallel_binomial_search_treeNode* // next sibling
			next() const {
			if (parent && (parent->numChildren > (parent_index + 1))) {
				return parent->children[parent_index + 1];
			}
			else {
				return nullptr;
			}
		};
		parallel_binomial_search_treeNode* // prev sibling
			prev() const {
			if (parent && (parent_index >= 1)) {
				return parent->children[parent_index - 1];
			}
			else {
				return nullptr;
			}
		};
		parallel_binomial_search_treeNode* // first child
			firstChild() const {
			if (numChildren == 0) return nullptr;
			return children[0];
		};
		parallel_binomial_search_treeNode* // last child
			lastChild() const {
			if (numChildren == 0) return nullptr;
			return children[numChildren - 1];
		};
		void 
			add_child(parallel_binomial_search_treeNode* p) {
			p->parent = this;
			p->parent_index = numChildren;
			children[numChildren] = p;
			++numChildren;
			if (this->key < p->key) this->key = p->key;
		}
		void 
			add_child_at(parallel_binomial_search_treeNode* p, int i) {
			if (i >= numChildren) add_child(p);
			else {
				p->parent = this;
				p->parent_index = i;
				for (int j = numChildren; j > i; --j) {
					children[j] = children[j - 1];
					children[j]->parent_index = j;
				}
				children[i] = p;
				++numChildren;
				if (this->key < p->key) this->key = p->key;
			}
		}
		parallel_binomial_search_treeNode* 
			pop_front_child() {
			if (numChildren <= 0) {
				return nullptr;
			}
			else {
				auto* out = children[0];
				int i = 0;
				for (i = 0; i < (numChildren - 1); ++i) {
					children[i] = children[i + 1];
					children[i]->parent_index = i;
				}
				children[i] = nullptr;
				--numChildren;
				return out;
			}
		}
		void 
			pop_front_children(int n) {
			if (numChildren >= n) {
				int i = 0;
				for (i = 0; i < (numChildren - n); ++i) {
					children[i] = children[i + n];
					children[i]->parent_index = i;
				}
				for (; i < maxChildrenPerNode; ++i) {
					children[i] = nullptr;
				}
				numChildren -= n;
			}
		}
		parallel_binomial_search_treeNode* 
			pop_child(int i) {
			if (numChildren <= 0) {
				return nullptr;
			}
			else {
				auto* out = children[i];
				for (; i < (numChildren - 1); ++i) {
					children[i] = children[i + 1];
					children[i]->parent_index = i;
				}
				children[i] = nullptr;
				--numChildren;
				if (numChildren > 0) this->key = children[numChildren - 1]->key;
				return out;
			}
		}
		parallel_binomial_search_treeNode* 
			pop_back_child() {
			if (numChildren <= 0) {
				return nullptr;
			}
			else {
				auto* out = children[numChildren - 1];
				children[numChildren - 1] = nullptr;
				--numChildren;
				if (numChildren > 0) this->key = children[numChildren - 1]->key;
				return out;
			}
		};
	};

private:
	lock_type
		mut; // global tree lock. Should only be held temporarily if at all possible. 
	parallel_binomial_search_treeNode*
		root;
	GL::atomic_parallel_allocator< parallel_binomial_search_treeNode, 128, true >
		nodeAllocator;
	std::atomic<long>
		count;
	class // exclusive lock manager. Since this is a course-grained type, though, it can only ever hold one lock at a time. 
		locker {
	public:
		std::shared_ptr<void>
			locks;
	public:
		void // store a shared lock
			push_back(lock_type& source) {
			locks = std::static_pointer_cast<void>(std::make_shared<std::scoped_lock<lock_type>>(source));
		};
		void // store a shared lock
			push_back_shared(lock_type& source) {
			locks = std::static_pointer_cast<void>(std::make_shared<std::shared_lock<lock_type>>(source));
		};
		void // store a shared lock
			push_pop(lock_type& source) {
			locks = std::static_pointer_cast<void>(std::make_shared<std::scoped_lock<lock_type>>(source));
		};
		void // store a shared lock
			push_pop_shared(lock_type& source) {
			locks = std::static_pointer_cast<void>(std::make_shared<std::shared_lock<lock_type>>(source));
		};
		void // remove the youngest lock
			pop_back() {
			locks = nullptr;
		};
		void // remove the oldest lock
			pop_front() {
			locks = nullptr;
		};
		size_t // count of locks
			size() const {
			return locks ? 1 : 0;
		};
		void // clear all locks
			clear() {
			locks = nullptr;
		};
	};

public:
	parallel_binomial_search_tree()
		: nodeAllocator()
		, root{ nullptr }
		, mut()
		, count{ 0 }
	{
		root = AllocNode();
	};
	parallel_binomial_search_tree(parallel_binomial_search_tree const&)
		= delete;
	parallel_binomial_search_tree(parallel_binomial_search_tree&&) noexcept
		= delete;
	parallel_binomial_search_tree& operator=(parallel_binomial_search_tree const&)
		= delete;
	parallel_binomial_search_tree& operator=(parallel_binomial_search_tree&&) noexcept
		= delete;
	~parallel_binomial_search_tree()
		= default;

	__declspec(noinline) parallel_binomial_search_treeNode* // add an object to the tree
		Add(objType* object, keyType key) {
		parallel_binomial_search_treeNode
			* node,
			* child,
			* newNode;
		locker
			locking;
		newNode
			= AllocNode();
		newNode->key
			= key;
		newNode->object
			= object;

		locking.push_back(mut); // locked

		if (root == nullptr) root = AllocNode(); // start fresh

		if (root->numChildren >= maxChildrenPerNode) { // make a new root and split
			node = AllocNode();
			node->key = root->key;
			node->add_child(root);
			SplitNode(root);
			root = node;
			node = nullptr;
		}

		for (node = root; node->numChildren > 0; node = child) {
			if (key > node->key) node->key = key; // in prep for the insertion

			// To-Do, upgrade this to the binomial search algorithm
			// find the first child with a key larger equal to the key of the new node
			for (child = node->firstChild(); child->next(); child = child->next()) 
				if (key <= child->key) 
					break;

			// we are inside of a branch of leafs -- we will do the insert.
			if (child->object) {
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
		Remove(parallel_binomial_search_treeNode* node, locker const& Locking = locker()) {
		parallel_binomial_search_treeNode
			* Node,
			* parent,
			* oldRoot;
		locker&
			locking = const_cast<locker&>(Locking);

		// acquire all relevant locks before we perform the deletion
		if (locking.size() == 0) locking.push_back(mut); // get the global tree lock		

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

			// a parent may not use a key higher than the key of its last child
			if (parent->numChildren > 0) 
				if (Node = parent->lastChild()) 
					if (parent->key > Node->key)
						parent->key = Node->key;

			if (parent->numChildren > maxChildrenPerNode) {
				SplitNode(parent);
				break;
			}
		}
		
		// a parent may not use a key higher than the key of it's last child
		for (; parent && (parent->numChildren > 0); parent = parent->parent) 
			if (Node = parent->lastChild()) 
				if (parent->key > Node->key) 
					parent->key = Node->key;		
		
		// actually free the node
		--count;
		FreeNode(node);

		// remove the root node if it has a single internal node as child		
		if ((root->numChildren == 1) && !root->firstChild()->object) {
			oldRoot = root;

			root = oldRoot->firstChild();
			root->parent = nullptr;
			root->parent_index = 0;

			FreeNode(oldRoot);
		}

		return true;
	};
	std::pair<parallel_binomial_search_treeNode*, locker> // find an object using the given key
		NodeFind(keyType key, bool for_removal = false) {
		parallel_binomial_search_treeNode
			* nxt;
		std::pair<parallel_binomial_search_treeNode*, locker>
			out;
		parallel_binomial_search_treeNode*&
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
		for (node = root->firstChild(); node; ) {
			while (node->next()) {
				if (node->key >= key) break;
				nxt = node->next();
				node = nxt;
			}

			if (node->object) {
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
			node = node->firstChild();
		}
		node = nullptr;
		locking.clear();
		return out;
	};
	std::pair<parallel_binomial_search_treeNode*, locker> // find an object using the given key
		NodeFind_ForRemoval(keyType key) {
		return NodeFind(key, true);
	};

#if 0
	std::pair<parallel_binomial_search_treeNode*, locker> // find an object with the smallest key larger equal the given key
		NodeFindSmallestLargerEqual(keyType key) {
		parallel_binomial_search_treeNode
			* nxt;
		std::pair<parallel_binomial_search_treeNode*, locker>
			out;
		parallel_binomial_search_treeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back_shared(mut);
		if (!root || !root->firstChild) {
			node = nullptr;
			locking.clear();
			return out;
		}
		for (node = root->firstChild; node; ) {
			while (node->next) {
				if (node->key >= key) break;
				nxt = node->next;
				node = nxt;
			}
			if (node->object) {
				if (node->key >= key) return out;
				else {
					node = nullptr;
					locking.clear();
					return out;
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
	std::pair<parallel_binomial_search_treeNode*, locker> // find an object with the largest key smaller equal the given key
		NodeFindLargestSmallerEqual(keyType key) {
		parallel_binomial_search_treeNode
			* smaller;
		std::pair<parallel_binomial_search_treeNode*, locker>
			out;
		parallel_binomial_search_treeNode*&
			node = out.first;
		locker&
			locking = out.second;

		locking.push_back(mut);
		if (!root || !root->firstChild) {
			node = nullptr;
			locking.clear();
			return out;
		}

		for (node = root->firstChild, smaller = nullptr; node != nullptr; ) {
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
	std::pair<parallel_binomial_search_treeNode*, locker> // returns the root node of the tree, with a locker that can be used for iteration. The locker has already locked the root. 
		GetRoot() {
		// auto guarded{ nodeAllocator.ProtectCurrentEpoch() };

		std::pair<parallel_binomial_search_treeNode*, locker> out;
		out.second.push_back(mut);
		out.first = root;
		return out;
	};
#if 0
	static parallel_binomial_search_treeNode* // goes through all nodes of the tree		
		GetNext(parallel_binomial_search_treeNode* node, locker& locking) {
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
	static parallel_binomial_search_treeNode* // goes through all leaf nodes of the tree		
		GetNextLeaf(parallel_binomial_search_treeNode* node, locker& locking) {
		parallel_binomial_search_treeNode*
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

private:
	parallel_binomial_search_treeNode*
		AllocNode() {
		parallel_binomial_search_treeNode
			* node;

		node = nodeAllocator.Alloc();
		for (int i = 0; i < maxChildrenPerNode; ++i) node->children[i] = nullptr;
		node->key = 0;
		node->parent = nullptr;
		node->parent_index = 0;
		node->numChildren = 0;
		node->object = nullptr;
		return node;
	};
	void
		FreeNode(parallel_binomial_search_treeNode* node) {
		nodeAllocator.Free(node);
	};
	void // will split node by creating a neighbor next to it in the parent node and sharing half its children
		SplitNode(parallel_binomial_search_treeNode* node) {
		int
			i, j;
		parallel_binomial_search_treeNode
			* child,
			* newNode;

		// allocate a new node
		newNode = AllocNode();
		newNode->parent = node->parent;

		// divide the children over the two nodes
		child = node->firstChild();
		newNode->children[0] = child;
		for (j = 1, i = 3; i < node->numChildren; i += 2, j++) {			
			child = child->next();
			newNode->children[j] = child;
		}
		newNode->key = child->key;
		newNode->numChildren = node->numChildren / 2;
		for (i = 0; i < newNode->numChildren; ++i) {
			newNode->children[i]->parent = newNode;
			newNode->children[i]->parent_index = i;
		}

		newNode->parent_index = node->parent_index;
		node->pop_front_children(newNode->numChildren);
		node->parent->add_child_at(newNode, newNode->parent_index);
	};;
	parallel_binomial_search_treeNode* // node1 will be deleted and its children appended to node2
		MergeNodes(parallel_binomial_search_treeNode* node1, parallel_binomial_search_treeNode* node2) {		
		for (int i = 0; i < node1->numChildren; ++i) 
			node2->add_child_at(node1->children[i], i);		
		(void)node1->parent->pop_child(node1->parent_index);
		FreeNode(node1);
		return node2;
	};

};