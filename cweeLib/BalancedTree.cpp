#ifndef __BALANCED_TREE_CPP__
#define __BALANCED_TREE_CPP__

#pragma region "INCLUDES"
#pragma hdrstop
#include "precompiled.h"
#pragma endregion

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void	cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Reserve(int num) {
	objAllocator.Reserve(num);
	nodeAllocator.Reserve(num);
};

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void	cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Add(cweeThreadedList<std::pair<keyType, objType>> const& data, bool addUnique) {
	cweeBalancedTreeNode<objType, keyType>* node, * child, * newNode; objType* OBJ; int index; bool exit;

	if (root == NULL) {
		root = AllocNode();
	}

	for (auto& source : data) {
		objType const& object = source.second;
		keyType const& key = source.first;

		// check that the key does not already exist		
		if (addUnique) {
			node = NodeFind(key);
			if (node && node->object) {
				*node->object = const_cast<objType&>(object);
				continue;
			}
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

		OBJ = nullptr;
		{
			OBJ = objAllocator.Alloc();
			*OBJ = const_cast<objType&>(object);
			Num++;
		}

		newNode->object = OBJ;
		exit = false;
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

			if (child->object) {

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

				exit = true;
				break;
			}

			// make sure the child has room to store another node
			if (child->numChildren >= maxChildrenPerNode) {
				SplitNode(child);
				if (key <= child->prev->key) {
					child = child->prev;
				}
			}
		}

		if (exit) {
			// we only end up here if the root node is empty
			newNode->parent = root;
			root->key = key;
			root->firstChild = newNode;
			root->lastChild = newNode;
			root->numChildren++;
		} 		
	}
};

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Add(objType const& object, keyType const& key, bool addUnique) {
	cweeBalancedTreeNode<objType, keyType>* node, * child, * newNode; objType* OBJ; int index;

	if (root == NULL) {
		root = AllocNode();
	}

	// check that the key does not already exist		
	if (addUnique) {
		node = NodeFind(key);
		if (node && node->object) {
			*node->object = const_cast<objType&>(object);
			return node;
		}
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

	OBJ = nullptr;
	{
		OBJ = objAllocator.Alloc();
		*OBJ = const_cast<objType&>(object);
		Num++;
	}

	newNode->object = OBJ;

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

		if (child->object) {

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

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Remove(cweeBalancedTreeNode<objType, keyType>* node) {
	if (!node) return;

	cweeBalancedTreeNode<objType, keyType>* parent, * oldRoot;

	//assert(node->object != NULL);

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
	if (root->numChildren == 1 && root->firstChild->object == NULL) {
		oldRoot = root;
		root->firstChild->parent = NULL;
		root = root->firstChild;
		FreeNode(oldRoot);
	}
};				// remove an object node from the tree

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Clear() {

	nodeAllocator.Shutdown();

	root = NULL;

	Num = 0;
	objAllocator.Shutdown();

	Init();
};

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::NodeFind(keyType  const& key) const {
	return NodeFind(key, root);
};								// find an object using the given key

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::NodeFindSmallestLargerEqual(keyType const& key) const {
	return NodeFindSmallestLargerEqual(key, root);
};			// find an object with the smallest key larger equal the given key

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::NodeFindLargestSmallerEqual(keyType const& key) const {
	return NodeFindLargestSmallerEqual(key, root);
};			// find an object with the largest key smaller equal the given key

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::NodeFind(keyType  const& key, cweeBalancedTreeNode<objType, keyType>* root) {
	cweeBalancedTreeNode<objType, keyType>* node = NodeFindLargestSmallerEqual(key, root);
	if (node && node->object && node->key == key) return node;
	return NULL;
};								// find an object using the given key

template< class objType, class keyType, int maxChildrenPerNode>
cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::NodeFindSmallestLargerEqual(keyType const& key, cweeBalancedTreeNode<objType, keyType>* Root) {
	cweeBalancedTreeNode<objType, keyType>* node, * smaller;

	if (Root == NULL) {
		return NULL;
	}

	smaller = NULL;
	for (node = Root->lastChild; node != NULL; node = node->lastChild) {
		//if (node->lastChild && node->firstChild) {
		//	if (node->firstChild->key > node->lastChild->key) {
		//		node = GetPrevLeaf(Root);
		//		break;
		//	}
		//}
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
			else if (smaller == NULL) {
				return NULL;
			}
			else {
				node = smaller;
				if (node->object) {
					break;
				}
			}
		}
	}

	//if (node) {
	//	while (node && !node->object) {
	//		node = cweeBalancedTree<objType, keyType, maxChildrenPerNode>::GetNextLeaf(node);
	//	}

	//	//if (!node->object) {
	//	//	node = cweeBalancedTree<objType, keyType, maxChildrenPerNode>::GetNextLeaf(node);
	//	//}
	//	//if (!node->object) {
	//	//	node = NULL;
	//	//}
	//}

	return node;
};			// find an object with the smallest key larger equal the given key

template< class objType, class keyType, int maxChildrenPerNode>
cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::NodeFindLargestSmallerEqual(keyType const& key, cweeBalancedTreeNode<objType, keyType>* Root) {
	cweeBalancedTreeNode<objType, keyType>* node, * smaller = nullptr;

	if (Root == NULL) {
		return smaller;
	}

	smaller = NULL;
	for (node = Root->firstChild; node != NULL; node = node->firstChild) {
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
			else if (smaller == NULL) {
				return NULL;
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
};			// find an object with the largest key smaller equal the given key

template< class objType, class keyType, int maxChildrenPerNode>
INLINE objType* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Find(keyType  const& key) const {
	cweeBalancedTreeNode<objType, keyType>* node = NodeFind(key, root);
	if (node == NULL) {
		return NULL;
	}
	else {
		return node->object;
	}
};									// find an object using the given key

template< class objType, class keyType, int maxChildrenPerNode>
INLINE objType* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::FindSmallestLargerEqual(keyType const& key) const {
	cweeBalancedTreeNode<objType, keyType>* node = NodeFindSmallestLargerEqual(key, root);
	if (node == NULL) {
		return NULL;
	}
	else {
		return node->object;
	}
};				// find an object with the smallest key larger equal the given key

template< class objType, class keyType, int maxChildrenPerNode>
INLINE objType* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::FindLargestSmallerEqual(keyType const& key) const {
	cweeBalancedTreeNode<objType, keyType>* node = NodeFindLargestSmallerEqual(key, root);
	if (node == NULL) {
		return NULL;
	}
	else {
		return node->object;
	}
};				// find an object with the largest key smaller equal the given key

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::GetRoot() const {
	return root;
};											// returns the root node of the tree

template< class objType, class keyType, int maxChildrenPerNode>
INLINE int	cweeBalancedTree<objType, keyType, maxChildrenPerNode>::GetNodeCount() const {
	return Num;
};										// returns the total number of nodes in the tree

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::GetNext(cweeBalancedTreeNode<objType, keyType>* node) {
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

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::GetNextLeaf(cweeBalancedTreeNode<objType, keyType>* node) {
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

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::GetPrevLeaf(cweeBalancedTreeNode<objType, keyType>* node) {
	if (node->lastChild) {
		while (node->lastChild) {
			node = node->lastChild;
		}
		return node;
	}
	else {
		while (node && node->prev == NULL) {
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
			return NULL;
		}
	}
};	// goes through all leaf nodes of the tree

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Init() {
	root = AllocNode();
	{ // helps init the objAllocator
		auto x = objAllocator.Alloc();
		objAllocator.Free(x);
	}
};

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void cweeBalancedTree<objType, keyType, maxChildrenPerNode>::Shutdown() {
	nodeAllocator.Shutdown();
	root = NULL;
	objAllocator.Shutdown();
};

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::AllocNode() {
	cweeBalancedTreeNode<objType, keyType>* node;

	node = nodeAllocator.Alloc();

	node->key = 0;
	node->parent = NULL;
	node->next = NULL;
	node->prev = NULL;
	node->numChildren = 0;
	node->firstChild = NULL;
	node->lastChild = NULL;
	node->object = nullptr;

	return node;
};

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void cweeBalancedTree<objType, keyType, maxChildrenPerNode>::FreeNode(cweeBalancedTreeNode<objType, keyType>* node) {
	if (node && node->object) {
		objAllocator.Free(node->object);
		Num--;
	}
	nodeAllocator.Free(node);
};

template< class objType, class keyType, int maxChildrenPerNode>
INLINE void cweeBalancedTree<objType, keyType, maxChildrenPerNode>::SplitNode(cweeBalancedTreeNode<objType, keyType>* node) {
	int i;
	cweeBalancedTreeNode<objType, keyType>* child, * newNode;

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
	assert(node->parent->numChildren < maxChildrenPerNode);

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

template< class objType, class keyType, int maxChildrenPerNode>
INLINE cweeBalancedTreeNode<objType, keyType>* cweeBalancedTree<objType, keyType, maxChildrenPerNode>::MergeNodes(cweeBalancedTreeNode<objType, keyType>* node1, cweeBalancedTreeNode<objType, keyType>* node2) {
	cweeBalancedTreeNode<objType, keyType>* child;

	assert(node1->parent == node2->parent);
	assert(node1->next == node2 && node2->prev == node1);
	assert(node1->object == NULL && node2->object == NULL);
	assert(node1->numChildren >= 1 && node2->numChildren >= 1);

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

#endif