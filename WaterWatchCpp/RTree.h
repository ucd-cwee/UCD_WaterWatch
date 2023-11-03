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

#if 1
#pragma once
#include "Precompiled.h"
//#include "BalancedTree.h"
//#include "cwee_math.h"
#include "vec.h"
//#include "Engineering.h"
#include "cweeJob.h"
#include "SharedPtr.h"
#include "InterpolatedMatrix.h"

#if 0

template< class objType, int maxChildrenPerNode = 10 >
class cweeRTree {
public:
	using keyType = cweeBoundary;
	class cweeRTreeNode {
	public:
		cweeRTreeNode() :
			key(),
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
		cweeRTreeNode* parent;						// parent node
		cweeRTreeNode* next;							// next sibling
		cweeRTreeNode* prev;							// prev sibling
		long long						numChildren;						// number of children
		cweeRTreeNode* firstChild;					// first child
		cweeRTreeNode* lastChild;					// last child
	};
	INLINE cweeRTreeNode* InitNode(cweeRTreeNode* p) {
		// static constexpr bool isThisPOD = std::is_pod<cweeRTree<double, u64>::cweeRTreeNode>::value;
		p->key = keyType();
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
	typedef cweeRTreeNode _iterType;
	struct it_state {
		_iterType _node;
		_iterType* node = &_node;
		inline void begin(const cweeRTree* ref) {
			node = ref->GetFirst();
			if (!node) node = &_node;
		};
		inline void begin_at(const cweeRTree* ref, keyType key) {
			node = ref->NodeFindLargestSmallerEqual(key);
			if (!node) this->begin(ref);
		};
		inline void next(const cweeRTree* ref) {
			node = ref->GetNextLeaf(node);
			if (!node) node = &_node;
		};
		inline void end(const cweeRTree* ref) {
			node = &_node;
		};
		inline _iterType& get(cweeRTree* ref) {
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
		inline void prev(const cweeRTree* ref) {
			node = ref->GetPrevLeaf(node);
			if (!node) node = &_node;
		};
		// Optional to allow `const_iterator`:
		inline const _iterType& get(const cweeRTree* ref) const { return *node; }
	};
	SETUP_STL_ITERATOR(cweeRTree, _iterType, it_state);
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
	cweeAlloc<objType, maxChildrenPerNode>	objAllocator;
	long long								Num;
	cweeAlloc<cweeRTreeNode, maxChildrenPerNode>			nodeAllocator;
	cweeRTreeNode* root;
	cweeRTreeNode* first;
	cweeRTreeNode* last;

public:
	cweeRTree& operator=(const cweeRTree& obj) {
		Clear(); // empty out whatever this container had 

		for (auto& x : obj) {
			Add(*x.object, x.key, false);
		}

		return *this;
	};
	~cweeRTree() {
		Clear();
	};
	bool operator==(const cweeRTree& obj) {
		return GetFirst() == obj.GetFirst() && GetLast() == obj.GetLast();
	};
	bool operator!=(const cweeRTree& obj) { return !operator==(obj); };

	cweeRTree() : Num(0), root(nullptr), first(nullptr), last(nullptr), objAllocator(), nodeAllocator() {
		static_assert(maxChildrenPerNode >= 4);
		Init();
	};
	cweeRTree(int toReserve) :
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


	void									Reserve(long long num) {
		objAllocator.Reserve(num);
		nodeAllocator.Reserve(num * 1.25); // approximately 25% more for 'overage'
	};

	void									Add(cweeThreadedList<std::pair<keyType, objType>> const& data, bool addUnique = true) {
		for (auto& source : data) {
			Add(source.second, source.first, addUnique);
		}
	};



	cweeRTreeNode*							NodeFind(keyType  const& key) const {
		return NodeFind(key, root);
	};								// find an object using the given key;
	cweeRTreeNode*							NodeFindSmallestLargerEqual(keyType const& key) const {
		return NodeFindSmallestLargerEqual(key, root);
	};			// find an object with the smallest key larger equal the given key;
	cweeRTreeNode*							NodeFindLargestSmallerEqual(keyType const& key) const {
		return NodeFindLargestSmallerEqual(key, root);
	};			// find an object with the largest key smaller equal the given key;




	static cweeRTreeNode* NodeFind(keyType  const& key, cweeRTreeNode* root) {
		cweeRTreeNode* node = NodeFindLargestSmallerEqual(key, root);
		if (node && node->object && node->key == key) return node;
		return nullptr;
	};								// find an object using the given key;
	static cweeRTreeNode* NodeFindSmallestLargerEqual(keyType const& key, cweeRTreeNode* Root) {
		cweeRTreeNode* node, * smaller;

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
	static cweeRTreeNode* NodeFindLargestSmallerEqual(keyType const& key, cweeRTreeNode* Root) {
		cweeRTreeNode* node, * smaller;

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








};

#endif

template <class objType, vec2d(*coordinateLookupFunctor)(objType const&)>
class cweeRTree {
public:
	class cweeBoundary {
	public:
		vec2d topRight;
		vec2d bottomLeft;
		vec2d& operator[](int i) {
			switch (i) {
			case 0: return topRight;
			case 1: return bottomLeft;
			default: throw(std::runtime_error("Bad Index for Boundary"));
			}
		};
		const vec2d& operator[](int i) const {
			switch (i) {
			case 0: return topRight;
			case 1: return bottomLeft;
			default: throw(std::runtime_error("Bad Index for Boundary"));
			}
		};

		vec2d Center() { return vec2d((topRight.x + bottomLeft.x)/2.0, (topRight.y + bottomLeft.y) / 2.0); };
		
		static bool Contains(cweeBoundary const& DoesThis, vec2d const& ContainThis) {
			return (DoesThis.topRight >= ContainThis) && (DoesThis.bottomLeft <= ContainThis);
		};
		static bool Contains(cweeBoundary const& DoesThis, cweeBoundary const& ContainThis) {
			return (DoesThis.topRight >= ContainThis.topRight) && (DoesThis.bottomLeft <= ContainThis.bottomLeft);
		};
		static bool Overlaps(cweeBoundary const& DoesThis, cweeBoundary const& OverlapThis) {
			// we overlap if we contain any of the corners, OR if the line connecting those corners lie inside. 
			return ObjectsIntersect(DoesThis, OverlapThis);
		};

		bool Contains(vec2d const& a) const {
			return Contains(*this, a);
		};
		bool Contains(cweeBoundary const& a) const {
			return Contains(*this, a);
		};
		bool Overlaps(cweeBoundary const& a) const {
			return Overlaps(*this, a);
		};

	private:
		static int WhichSide(const cweeBoundary& C, const vec2d& D, const vec2d& V)
		{
			int i; float t;
			// C vertices are projected to the form V+t*D.
			// Return value is +1 if all t > 0, -1 if all t < 0, 0 otherwise, in
			// which case the line splits the polygon.
			int positive = 0, negative = 0;
			for (i = 0; i < 2; i++)
			{
				t = D.Dot(C[i] - V);
				if (t > 0) positive++; else if (t < 0) negative++;
				if (positive && negative) return 0;
			}
			return (positive ? +1 : -1);
		};
		static bool ObjectsIntersect(const cweeBoundary& C0, const cweeBoundary& C1)
		{
			int i0, i1;
			vec2d E, D;

			// Test edges of C0 for separation. Because of the counterclockwise ordering,
			// the projection interval for C0 is [m,0] where m <= 0. Only try to determine
			// if C1 is on the ‘positive’ side of the line.
			for (i0 = 0, i1 = 2 - 1; i0 < 2; i1 = i0, i0++)
			{
				E = (C0[i0] - C0[i1]); // or precompute edges if desired
				D = vec2d(E.y, -E.x);
				if (WhichSide(C1, D, C0[i0]) > 0)
				{ // C1 is entirely on ‘positive’ side of line C0.V(i0)+t*D
					return false;
				}
			}
			// Test edges of C1 for separation. Because of the counterclockwise ordering,
			// the projection interval for C1 is [m,0] where m <= 0. Only try to determine
			// if C0 is on the ‘positive’ side of the line.
			for (i0 = 0, i1 = 2 - 1; i0 < 2; i1 = i0, i0++)
			{
				E = (C1[i0] - C1[i1]); // or precompute edges if desired
				D = vec2d(E.y, -E.x);
				if (WhichSide(C0, D, C1[i0]) > 0)
				{ // C0 is entirely on ‘positive’ side of line C1.V(i0)+t*D
					return false;
				}
			}
			return true;
		};
	};
	class TreeNode {
	public:
		cweeBoundary
			boundary;					// node's basic, rectangular boundary 
		long long
			numChildren;				// number of children
		cweeSharedPtr<objType> 
			object;						// if != NULL pointer to object stored in leaf node
		TreeNode*
			parent;						// parent node
		TreeNode*
			next;						// next sibling
		TreeNode*
			prev;						// prev sibling
		TreeNode*
			firstChild;					// first child
		TreeNode*
			lastChild;					// last child

		// Update Boundary When Any Children Change
		void UpdateBoundary(bool selfOnly = false) {
			cweeSharedPtr<objType> o = object;
			if (o) {
				// my boundary IS my object.
				vec2d coord = GetCoordinate(*o);
				boundary.topRight = coord;
				boundary.bottomLeft = coord;
			} 
			else if (numChildren > 0) {
				TreeNode* p = this->firstChild;
				int count = 0;
				while (p) {
					if (!selfOnly) p->UpdateBoundary();
					if (0 == count++) {
						boundary = p->boundary;
					} 
					else {
						boundary.topRight.x = ::Max(boundary.topRight.x, p->boundary.topRight.x);
						boundary.topRight.y = ::Max(boundary.topRight.y, p->boundary.topRight.y);
						boundary.bottomLeft.x = ::Min(boundary.bottomLeft.x, p->boundary.bottomLeft.x);
						boundary.bottomLeft.y = ::Min(boundary.bottomLeft.y, p->boundary.bottomLeft.y);
					}					
					p = p->next;
				}
			}
			else {
				// empty object with no children and no object -- do nothing
			}

			// update the parents
			{
				TreeNode* p = parent;
				while (p) {
					p->UpdateBoundary(true);
					p = p->parent;
				}
			}
		};
		// Identify if a point lies within the boundary of this node
		bool Overlaps(vec2d const& a) {
			return boundary.Contains(a);
		};
		// Identify if a boundary overlaps with this node
		bool Overlaps(cweeBoundary const& a) {
			return boundary.Overlaps(a);
		};
		// Identify if a node overlaps with this node
		bool Overlaps(TreeNode const& a) {
			return boundary.Overlaps(a.boundary);
		};
		// Initialize a new node
		static TreeNode* InitNode(TreeNode* p) {
			p->boundary = cweeBoundary();
			p->object = nullptr;
			p->parent = nullptr;
			p->next = nullptr;
			p->prev = nullptr;
			p->numChildren = 0;
			p->firstChild = nullptr;
			p->lastChild = nullptr;
			return p;
		};
		std::vector<std::vector<TreeNode*>> cluster_children() {
			// split this 2d box into two smaller 2d boxes. 
			// one fast way to do this is to pick two corners opposite of each other and pair based on distance. 			
			std::vector<std::vector<TreeNode*>> out(2, std::vector<TreeNode*>());
			vec2d c;
			double distance;
			std::map<double, TreeNode*> sortedByDistance;
			auto* child = this->firstChild;
			int childCount = 0;
			while (child) {
				childCount++;
				c = child->boundary.Center();
				distance = c.Distance(boundary.topRight);
				while (sortedByDistance.count(distance) > 0) {
					distance += 1;
				}
				sortedByDistance.emplace(distance, child);
				child = child->next;
			}

			childCount /= 2;
			for (auto& childIter : sortedByDistance) {
				if (childCount-- > 0) {
					out[0].push_back(childIter.second);
				}
				else {
					out[1].push_back(childIter.second);
				}				
			}
			return out;
		};



	};

private:
	long long	Num;
	TreeNode*	root;
	cweeAlloc<TreeNode, 10>			
				nodeAllocator;

public: // Desired Interface
	void AddObject(objType const& toCopy) { Add(make_cwee_shared< objType>(toCopy)); };
	void AddObject(objType&& toMove) { Add(make_cwee_shared< objType>(std::forward<objType>(toMove))); };
	void AddObject(cweeSharedPtr<objType> const& toCopyRef) { Add(toCopyRef); };
	void RemoveObject(objType const& toRemove) {
		auto* child = GetRoot();
		child = GetNextLeaf(child);
		while (child) {
			if (child->object && *child->object == toRemove) {
				RemoveNode(child);
				break;
			}
			child = GetNextLeaf(child);
		}
	};
	void RemoveObject(cweeSharedPtr<objType> const& toRemoveRef) {
		auto* child = GetRoot();
		child = GetNextLeaf(child);
		while (child) {
			if (child->object && child->object == toRemoveRef) {
				RemoveNode(child);
				break;
			}
			child = GetNextLeaf(child);
		}
	};
	//cweeList<cweeSharedPtr<objType>> FindObjectsAt(vec2d const& location);
	//cweeList<cweeSharedPtr<objType>> FindObjectsIn(vec2d const& topRight, vec2d const& bottomLeft);

public:
	cweeRTree() : Num(0), root(nullptr), nodeAllocator() { Init(); };
	cweeRTree(cweeRTree const& obj) : Num(0), root(nullptr), nodeAllocator() { 
		Init();
		for (auto* p = obj.GetRoot(); p; p = GetNextLeaf(p)) {
			if (p && p->object) {
				Add(*p->object);
			}
		}
	};
	cweeRTree& operator=(const cweeRTree& obj) {
		Clear(); // empty out whatever this container had 
		
		for (auto* p = obj.GetRoot(); p; p = GetNextLeaf(p)) {
			if (p && p->object) {
				Add(*p->object);
			}
		}

		return *this;
	};
	~cweeRTree() {
		Clear();
	};

public:
	/* returns the total number of nodes in the tree; */
	long long GetNodeCount() const {
		return Num;
	};
	/* returns the root node of the tree */
	TreeNode* GetRoot() const { return root; };
	/* Clears the tree */
	void Clear() {
		nodeAllocator.Clear();
		root = nullptr;
		Num = 0;
		Init();
	};
	/*  remove an object node from the tree */
	void RemoveNode(TreeNode* node) {
		if (!node) return;

		TreeNode* parent, * oldRoot;

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

		node->parent->UpdateBoundary(true);

		// make sure there are no parent nodes with a single child
		for (parent = node->parent; parent != root && parent->numChildren <= 1; parent = parent->parent) {

			if (parent->next) {
				parent = MergeNodes(parent, parent->next);
			}
			else if (parent->prev) {
				parent = MergeNodes(parent->prev, parent);
			}

			if (parent->numChildren > 10) {
				SplitNode(parent);
				break;
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
	};
	/* add an object to the tree */
	TreeNode* Add(cweeSharedPtr<objType> const& object) {
		TreeNode* node, * child, * newNode; cweeSharedPtr<objType> OBJ; long long index;

		if (root == nullptr) {
			root = AllocNode();
		}

		if (root->numChildren >= 10) {
			newNode = AllocNode();
			newNode->firstChild = root;
			newNode->lastChild = root;
			newNode->numChildren = 1;
			root->parent = newNode;
			SplitNode(root);
			root = newNode;
		}

		newNode = AllocNode();
		newNode->object = object;
		
		Num++;

		for (node = root; node->firstChild != nullptr; node = child) {
			// find the first child that can contain this new node
			for (child = node->firstChild; child->next; child = child->next) {
				if (child->boundary.Contains(newNode->boundary)) {
					break;
				}
			}

			if (child->object) {
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
				
				newNode->parent = node;
				node->numChildren++;

				newNode->UpdateBoundary(true);

				return newNode;
			}

			// make sure the child has room to store another node
			if (child->numChildren >= 10) {
				SplitNode(child);
				child = child->prev;				
			}
		}

		// we only end up here if the root node is empty
		newNode->parent = root;
		root->firstChild = newNode;
		root->lastChild = newNode;
		root->numChildren++;
		newNode->UpdateBoundary(true);

		return newNode;
	};
	/* goes through all leaf nodes of the tree */
	static TreeNode* GetNextLeaf(TreeNode* node) {
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
	};
	/* goes through all leaf nodes of the tree */
	static TreeNode* GetPrevLeaf(TreeNode* node) {
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
	};
	/* goes through all nodes of the tree */
	static TreeNode* GetNext(TreeNode* node) {
		if (node) {
			if (node->firstChild) {
				node = node->firstChild;
			}
			else {
				if (node && node->next == nullptr) {
					while (node && node->next == nullptr) {
						node = node->parent;
					}
					
				}
				if (node) node = node->next;
			}
		}
		return node;
	};
	
private:
	/* Get the coordinate from the stored object, similar to a std::Set */
	static vec2d GetCoordinate(objType const& obj) { return coordinateLookupFunctor(obj); };
	/* Initialize the tree */ 
	void Init() { root = AllocNode(); };
	/* Shutdown the tree */
	void Shutdown() {
		nodeAllocator.Clear();
		root = nullptr;
		Num = 0;
	};
	/* Clears a node that may have been previously made */ 
	TreeNode* AllocNode() { return TreeNode::InitNode(nodeAllocator.Alloc()); };
	void FreeNode(TreeNode* node) {
		if (node && node->object) {
			node->object = nullptr;
			--Num;
		}
		nodeAllocator.Free(node);
	};
	/* Splits an existing node into two nodes */
	void SplitNode(TreeNode* node) {
		long long i;
		TreeNode* child, * newNode;

		// allocate a new node
		newNode = AllocNode();
		newNode->parent = node->parent; // share parents, as this will become a sibling

		// divide the children over the two nodes. 
		std::vector<std::vector<TreeNode*>> clustered_children = node->cluster_children();
		int numChildren1 = clustered_children[0].size();
		int numChildren2 = clustered_children[1].size();
		clustered_children[0].push_back(nullptr);
		clustered_children[1].push_back(nullptr);

		// first set -- append and assign
		for (i = 0; i < numChildren1; i++) {
			if (i == 0) {
				newNode->firstChild = clustered_children[0][i];
				clustered_children[0][i]->prev = nullptr;
				clustered_children[0][i]->next = clustered_children[0][i+1];
				newNode->numChildren = 1;
				clustered_children[0][i]->parent = newNode;
			}
			else if (i == numChildren1 - 1) {
				newNode->lastChild = clustered_children[0][i];
				clustered_children[0][i]->next = nullptr;
				clustered_children[0][i]->prev = clustered_children[0][i - 1];
				newNode->numChildren++;
				clustered_children[0][i]->parent = newNode;
			}
			else {
				clustered_children[0][i]->next = clustered_children[0][i + 1];
				clustered_children[0][i]->prev = clustered_children[0][i - 1];
				newNode->numChildren++;
				clustered_children[0][i]->parent = newNode;
			}
		}
		// second set -- append and assign
		for (i = 0; i < numChildren2; i++) {
			if (i == 0) {
				node->firstChild = clustered_children[1][i];
				clustered_children[1][i]->prev = nullptr;
				clustered_children[1][i]->next = clustered_children[1][i + 1];
				node->numChildren = 1;
				clustered_children[1][i]->parent = node;
			}
			else if (i == numChildren2 - 1) {
				node->lastChild = clustered_children[1][i];
				clustered_children[1][i]->next = nullptr;
				clustered_children[1][i]->prev = clustered_children[1][i - 1];
				node->numChildren++;
				clustered_children[1][i]->parent = node;
			}
			else {
				clustered_children[1][i]->next = clustered_children[1][i + 1];
				clustered_children[1][i]->prev = clustered_children[1][i - 1];
				node->numChildren++;
				clustered_children[1][i]->parent = node;
			}
		}

		newNode->UpdateBoundary(true);
		node->UpdateBoundary(true);

		if (node->prev) {
			node->prev->next = newNode;
		}
		else {
			node->parent->firstChild = newNode;
		}
		newNode->prev = node->prev;
		newNode->next = node;
		node->prev = newNode;

		node->parent->numChildren++; // parent boundary should not have changed.
	};
	/* Merge two nodes together into a single node */
	TreeNode* MergeNodes(TreeNode* node1, TreeNode* node2) {
		TreeNode* child;

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

		node2->UpdateBoundary(true); 

		return node2;
	};

	static TreeNode* NodeFindOverlap(cweeBoundary const& bound, TreeNode* Root) {
		TreeNode* node, * smaller;
		if (Root == nullptr) return nullptr;
		
		smaller = nullptr;
		for (node = Root->firstChild; node != nullptr; node = node->firstChild) {
			while (node->next) {
				if (!node->boundary.Overlaps(bound)) {
					if (!smaller) {
						smaller = GetNextLeaf(Root);
					}
					break;
				}
				smaller = node;
				node = node->next;
			}
			if (node->object) {
				if (node->boundary.Overlaps(bound)) {
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
	};

};

#endif