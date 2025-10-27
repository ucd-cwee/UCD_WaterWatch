#pragma once
#include "dynamic_allocator.h"
#include "../ScriptLanguageTesting/atomic_allocator.h"


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

//template< class objType, class keyType, int maxChildrenPerNode >
class atomic_tree {
public:
	using objType = int;
	using keyType = int;
	static constexpr int maxChildrenPerNode = 10;

	class atomic_treeNode {
	public:
		keyType	// key used for sorting						
			key;
		objType // if != nullptr pointer to object stored in leaf node
			*object;
		atomic_treeNode // parent node
			*parent;
		atomic_treeNode // next sibling
			*next;
		atomic_treeNode // prev sibling
			*prev;
		int	// number of children							
			numChildren;
		atomic_treeNode // first child
			*firstChild;
		atomic_treeNode // last child
			*lastChild;
		std::recursive_mutex
			mutex;
	};

public:
	atomic_tree() {
		root = AllocNode();
	};
	~atomic_tree() {
		root = nullptr;
	};

	// add an object to the tree
	atomic_treeNode* Add(objType* object, keyType key) {
		atomic_treeNode
			*node = nullptr,
			*child,
			*newNode;

		if (!root) {
			newNode = AllocNode();
			if (!root.compare_exchange_weak(node, newNode)) {
				FreeNode(newNode);
				newNode = nullptr;
			}
		}
		std::scoped_lock root_lock(root.load()->mutex);

		// continue from here...

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
	void Remove(atomic_treeNode* node) {
		atomic_treeNode* parent;

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
			atomic_treeNode* oldRoot = root;
			root->firstChild->parent = nullptr;
			root = root->firstChild;
			FreeNode(oldRoot);
		}
	};
	// find an object using the given key
	atomic_treeNode* NodeFind(keyType key) const {
		for (atomic_treeNode* node = root->firstChild; node != nullptr; node = node->firstChild) {
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
	atomic_treeNode* NodeFindSmallestLargerEqual(keyType key) const {
		if (root == nullptr) return nullptr;
		for (atomic_treeNode* node = root->firstChild; node != nullptr; node = node->firstChild) {
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
	// find an object with the smallest key larger equal the given key
	objType* FindSmallestLargerEqual(keyType key) const {
		if (atomic_treeNode* node = NodeFindSmallestLargerEqual(key)) return node->object;
		else return nullptr;
	};
	// returns the root node of the tree
	atomic_treeNode* GetRoot() const {
		return root;
	};
	// goes through all nodes of the tree
	static atomic_treeNode* GetNext(atomic_treeNode* node) {
		if (node->firstChild) return node->firstChild;
		else {
			while (node && (node->next == nullptr)) node = node->parent;
			return node;
		}
	};
	// goes through all leaf nodes of the tree
	static atomic_treeNode* GetNextLeaf(atomic_treeNode* node) {
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
	std::atomic<atomic_treeNode*> // will be recovered by the nodeAllocator on destruction. 
		root;
	GL::atomic_allocator< atomic_treeNode > // will automatically recover memory if unreleased
		nodeAllocator; 

	atomic_treeNode*
		AllocNode() {
		atomic_treeNode* node = nodeAllocator.Alloc();
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
		FreeNode(atomic_treeNode* node) {
		nodeAllocator.Free(node);
	};
	void
		SplitNode(atomic_treeNode* node) {
		int i;
		atomic_treeNode* child, * newNode;

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
	atomic_treeNode*
		MergeNodes(atomic_treeNode* node1, atomic_treeNode* node2) {
		atomic_treeNode* child;

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