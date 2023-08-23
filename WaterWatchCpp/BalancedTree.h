/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "Iterator.h"
#include "LinkedList.h"

template< class objType, class keyType, int maxChildrenPerNode = 10 >
class cweeBalancedTree {
public:
	class cweeBalancedTreeNode {
	public:
		cweeBalancedTreeNode() : 
			key(0), 
			object(nullptr), 
			parent(nullptr), 
			next(nullptr), 
			prev(nullptr), 
			numChildren(0), 
			firstChild(nullptr), 
			lastChild(nullptr) 
		{};

		keyType							key;								// key used for sorting
		objType* object;						// if != NULL pointer to object stored in leaf node
		cweeBalancedTreeNode* parent;						// parent node
		cweeBalancedTreeNode* next;							// next sibling
		cweeBalancedTreeNode* prev;							// prev sibling
		long long						numChildren;						// number of children
		cweeBalancedTreeNode* firstChild;					// first child
		cweeBalancedTreeNode* lastChild;					// last child
	};
	INLINE cweeBalancedTreeNode* InitNode(cweeBalancedTreeNode* p) {
		// static constexpr bool isThisPOD = std::is_pod<cweeBalancedTree<double, u64>::cweeBalancedTreeNode>::value;
		p->key = 0;
		p->object = nullptr;
		p->parent = nullptr;
		p->next = nullptr;
		p->prev = nullptr;
		p->numChildren = 0;
		p->firstChild = nullptr;
		p->lastChild = nullptr;
		return p;
	};

public:
	typedef cweeBalancedTreeNode _iterType;
	struct it_state {
		_iterType _node;
		_iterType* node = &_node;
		inline void begin(const cweeBalancedTree* ref) {
			node = ref->GetFirst();
			if (!node) node = &_node;
		};
		inline void begin_at(const cweeBalancedTree* ref, keyType key) {
			node = ref->NodeFindLargestSmallerEqual(key);
			if (!node) this->begin(ref);
		};
		inline void next(const cweeBalancedTree* ref) {
			node = ref->GetNextLeaf(node);
			if (!node) node = &_node;
		};
		inline void end(const cweeBalancedTree* ref) {
			node = &_node;
		};
		inline _iterType& get(cweeBalancedTree* ref) {
			return *node;
		};
		inline bool cmp(const it_state& s) const {
#ifdef useCweeUnorderedListForBalancedTreeObjAlloc
			return (node->objectIndex == s.node->objectIndex) ? false : true;
#else
			return !(!node->object || (node->object == s.node->object));
			// return ((node->object == s.node->object) || (node->object == nullptr)) ? false : true;
#endif
		};

		inline long long distance(const it_state& s) const { throw(std::exception("Cannot evaluate distance")); }
		// Optional to allow operator--() and reverse iterators:
		inline void prev(const cweeBalancedTree* ref) {
			node = ref->GetPrevLeaf(node);
			if (!node) node = &_node;
		};
		// Optional to allow `const_iterator`:
		inline const _iterType& get(const cweeBalancedTree* ref) const { return *node; }
	};
	SETUP_STL_ITERATOR(cweeBalancedTree, _iterType, it_state);
	iterator begin_at(keyType key) { 
		AUTO iter = this->begin();
		iter.state.begin_at(this, key);
		return iter;
	};
	const_iterator begin_at(keyType key) const {
		AUTO iter = this->begin();
		iter.state.begin_at(this, key);
		return iter;
	};
	const_iterator cbegin_at(keyType key) const {
		AUTO iter = this->cbegin();
		iter.state.begin_at(this, key);
		return iter;
	};
private:
	// cweeLinkedList< objType, maxChildrenPerNode> objAllocator;
	cweeAlloc<objType, maxChildrenPerNode>	objAllocator;
	long long								Num;
	//cweeLinkedList< cweeBalancedTreeNode, maxChildrenPerNode>			nodeAllocator;
	cweeAlloc<cweeBalancedTreeNode, maxChildrenPerNode>			nodeAllocator;
	cweeBalancedTreeNode* root;
	cweeBalancedTreeNode* first;
	cweeBalancedTreeNode* last;

public:
	cweeBalancedTree& operator=(const cweeBalancedTree& obj) {
		Clear(); // empty out whatever this container had 

		for (auto& x : obj) {
			Add(*x.object, x.key, false);
		}

		return *this;
	};
	bool operator==(const cweeBalancedTree& obj) {
		return GetFirst() == obj.GetFirst() && GetLast() == obj.GetLast();
	};
	bool operator!=(const cweeBalancedTree& obj) { return !operator==(obj); };

	cweeBalancedTree() : Num(0), root(nullptr), first(nullptr), last(nullptr), objAllocator(), nodeAllocator() {
		static_assert(maxChildrenPerNode >= 4);
		Init();
	};
	cweeBalancedTree(int toReserve) : 
		Num(0), 
		root(nullptr), 
		first(nullptr), 
		last(nullptr), 
		objAllocator(toReserve),
		nodeAllocator(toReserve * 1.25)
	{
		static_assert(maxChildrenPerNode >= 4);
		Init();
		//objAllocator.Reserve(toReserve);
		//nodeAllocator.Reserve(toReserve * 1.25); // approximately 25% more for 'overage'
	};
	~cweeBalancedTree() {
		Clear();
	};

	void									Reserve(long long num) {
		objAllocator.Reserve(num);
		nodeAllocator.Reserve(num * 1.25); // approximately 25% more for 'overage'
	};

	void									Add(cweeThreadedList<std::pair<keyType, objType>> const& data, bool addUnique = true) {
		for (auto& source : data) {
			Add(source.second, source.first, addUnique);
		}
	};
	cweeBalancedTreeNode*					Add(objType const& object, keyType const& key, bool addUnique = true) {
		cweeBalancedTreeNode* node, * child, * newNode; objType* OBJ; long long index;

		if (root == nullptr) {
			root = AllocNode();
		}

		// check that the key does not already exist		
		if (addUnique) {
			node = NodeFind(key);
			if (node && node->object) {
				*node->object = const_cast<objType&>(object);
				return CheckLastNode(CheckFirstNode(node));
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

		for (node = root; node->firstChild != nullptr; node = child) {

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

				return CheckLastNode(CheckFirstNode(newNode));
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

		return CheckLastNode(CheckFirstNode(newNode));
	};

	void									Remove(cweeBalancedTreeNode* node) {
		if (!node) return;

		if (first == node) {
			first = this->GetNextLeaf(node);
		}

		if (last == node) {
			last = this->GetPrevLeaf(node);
		}

		cweeBalancedTreeNode* parent, * oldRoot;

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
		for (; parent != nullptr && parent->lastChild != nullptr; parent = parent->parent) {
			// a parent may not use a key higher than the key of it's last child
			if (parent->key > parent->lastChild->key) {
				parent->key = parent->lastChild->key;
			}
		}

		// free the node
		FreeNode(node);

		// remove the root node if it has a single internal node as child
		if (root->numChildren == 1 && root->firstChild->object == nullptr) {
			oldRoot = root;
			root->firstChild->parent = nullptr;
			root = root->firstChild;
			FreeNode(oldRoot);
		}
	};				// remove an object node from the tree
	void									Clear() {
		// while (first) { Remove(first); }
		
		// remove all
		nodeAllocator.Clear();
		objAllocator.Clear();
		root = nullptr;
		first = nullptr;
		last = nullptr;
		Num = 0;

		Init();
	};;
	cweeBalancedTreeNode*					NodeFind(keyType  const& key) const {
		return NodeFind(key, root);
	};								// find an object using the given key;
	cweeBalancedTreeNode*					NodeFindSmallestLargerEqual(keyType const& key) const {
		return NodeFindSmallestLargerEqual(key, root);
	};			// find an object with the smallest key larger equal the given key;
	cweeBalancedTreeNode*					NodeFindLargestSmallerEqual(keyType const& key) const {
		return NodeFindLargestSmallerEqual(key, root);
	};			// find an object with the largest key smaller equal the given key;

	static cweeBalancedTreeNode*			NodeFind(keyType  const& key, cweeBalancedTreeNode* root) {
		cweeBalancedTreeNode* node = NodeFindLargestSmallerEqual(key, root);
		if (node && node->object && node->key == key) return node;
		return nullptr;
	};								// find an object using the given key;
	static cweeBalancedTreeNode*			NodeFindSmallestLargerEqual(keyType const& key, cweeBalancedTreeNode* Root) {
		cweeBalancedTreeNode *node, *smaller;

		if (Root == nullptr) {
			return nullptr;
		}

		smaller = nullptr;
		for (node = Root->lastChild; node != nullptr; node = node->lastChild) {
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
				else if (smaller == nullptr) {
					return nullptr;
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
	};			// find an object with the smallest key larger equal the given key;
	static cweeBalancedTreeNode*			NodeFindLargestSmallerEqual(keyType const& key, cweeBalancedTreeNode* Root) {
		cweeBalancedTreeNode *node, *smaller;

		if (Root == nullptr) {
			return nullptr;
		}

		smaller = nullptr;
		for (node = Root->firstChild; node != nullptr; node = node->firstChild) {
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
				else if (smaller == nullptr) {
					return nullptr;
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
	};			// find an object with the largest key smaller equal the given key;

	objType*								Find(keyType  const& key) const {
		cweeBalancedTreeNode* node = NodeFind(key, root);
		if (node == nullptr) {
			return nullptr;
		}
		else {
			return node->object;
		}
	};									// find an object using the given key;
	objType*								FindSmallestLargerEqual(keyType const& key) const {
		cweeBalancedTreeNode* node = NodeFindSmallestLargerEqual(key, root);
		if (node == nullptr) {
			return nullptr;
		}
		else {
			return node->object;
		}
	};				// find an object with the smallest key larger equal the given key;
	objType*								FindLargestSmallerEqual(keyType const& key) const {
		cweeBalancedTreeNode* node = NodeFindLargestSmallerEqual(key, root);
		if (node == nullptr) {
			return nullptr;
		}
		else {
			return node->object;
		}
	};				// find an object with the largest key smaller equal the given key;

	cweeBalancedTreeNode*					GetFirst() const { return first; };
	cweeBalancedTreeNode*					GetLast() const { return last; };
	cweeBalancedTreeNode*					GetRoot() const {
		return root;
	};											// returns the root node of the tree;
	long long								GetNodeCount() const {
		return Num;
	};										// returns the total number of nodes in the tree;
	long long								GetReservedCount() const {
		return objAllocator.GetTotalCount();  // .Num(); //  
	};
	static cweeBalancedTreeNode*			GetNext(cweeBalancedTreeNode* node) {
		if (node) {
			if (node->firstChild) {
				node = node->firstChild;
			}
			else {
				while (node && node->next == nullptr) {
					node = node->parent;
				}
			}
		}
		return node;

		//if (!node) return nullptr; 
		//if (node->firstChild) {
		//	return node->firstChild;
		//}
		//else {
		//	while (node && node->next == nullptr) {
		//		node = node->parent;
		//	}
		//	return node;
		//}
	};		// goes through all nodes of the tree;

public:
	static cweeBalancedTreeNode*			GetNextLeaf(cweeBalancedTreeNode* node) {
		if (node) {
			if (node->firstChild) {
				while (node->firstChild) {
					node = node->firstChild;
				}
			}
			else {
				while (node && !node->next) {
					node = node->parent;
				}
				if (node) {
					node = node->next;
					while (node->firstChild) {
						node = node->firstChild;
					}
				}
				else {
					node = nullptr;
				}
			}
		}
		return node;

		//if (!node) return nullptr;
		//if (node->firstChild) {
		//	while (node->firstChild) {
		//		node = node->firstChild;
		//	}
		//	return node;
		//}
		//else {
		//	while (node && node->next == nullptr) {
		//		node = node->parent;
		//	}
		//	if (node) {
		//		node = node->next;
		//		while (node->firstChild) {
		//			node = node->firstChild;
		//		}
		//		return node;
		//	}
		//	else {
		//		return nullptr;
		//	}
		//}
	};	// goes through all leaf nodes of the tree;
	static cweeBalancedTreeNode*			GetPrevLeaf(cweeBalancedTreeNode* node) {
		if (!node) return nullptr; 
		if (node->lastChild) {
			while (node->lastChild) {
				node = node->lastChild;
			}
			return node;
		}
		else {
			while (node && node->prev == nullptr) {
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
				return nullptr;
			}
		}
	};	// goes through all leaf nodes of the tree;

private:
	cweeBalancedTreeNode*					CheckFirstNode(cweeBalancedTreeNode* newNode) {
		if (newNode && first) {
			if (newNode->key < first->key) {
				first = newNode;
			}
		}
		else {
			first = newNode;
		}
		return newNode;
	};
	cweeBalancedTreeNode*					CheckLastNode(cweeBalancedTreeNode* newNode) {
		if (newNode && last) {
			if (newNode->key > last->key) {
				last = newNode;
			}
		}
		else {
			last = newNode;
		}
		return newNode;
	};
	void									Init() {
		root = AllocNode();
		{ // helps init the objAllocator
			auto x = objAllocator.Alloc();
			objAllocator.Free(x);
		}
	};
	void									Shutdown() {
		nodeAllocator.Clear();
		
		objAllocator.Clear();
		root = nullptr;
		first = nullptr;
		last = nullptr;
		Num = 0;
	};
	cweeBalancedTreeNode*					AllocNode() {
		cweeBalancedTreeNode* node;

		node = nodeAllocator.Alloc();
		return InitNode(node);

		//node->key = 0;
		//node->parent = nullptr;
		//node->next = nullptr;
		//node->prev = nullptr;
		//node->numChildren = 0;
		//node->firstChild = nullptr;
		//node->lastChild = nullptr;
		//node->object = nullptr;

		//return node;
	};
	void									FreeNode(cweeBalancedTreeNode* node) {
		if (node && node->object) {
			objAllocator.Free(node->object);  // RemoveFast(node->object); // 
			Num--;
		}
		nodeAllocator.Free(node); // RemoveFast(node); //  
	};
	void									SplitNode(cweeBalancedTreeNode* node) {
		long long i;
		cweeBalancedTreeNode* child, * newNode;

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
	cweeBalancedTreeNode*					MergeNodes(cweeBalancedTreeNode* node1, cweeBalancedTreeNode* node2) {
		cweeBalancedTreeNode* child;

		assert(node1->parent == node2->parent);
		assert(node1->next == node2 && node2->prev == node1);
		assert(node1->object == nullptr && node2->object == nullptr);
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

};