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
#include "cweeThreadedMap.h"
#include "Voronoi.h"


class cweeBoundary {
public:
	vec2d topRight;
	vec2d bottomLeft;

	cweeBoundary() : topRight(-cweeMath::INF, -cweeMath::INF), bottomLeft(cweeMath::INF, cweeMath::INF) {};
	cweeBoundary(cweeBoundary const&) = default;
	cweeBoundary(cweeBoundary&&) = default;
	cweeBoundary& operator=(cweeBoundary const&) = default;
	cweeBoundary& operator=(cweeBoundary&&) = default;

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

	vec2d Center() { return vec2d((topRight.x + bottomLeft.x) / 2.0, (topRight.y + bottomLeft.y) / 2.0); };

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

template <class objType, cweeBoundary(*coordinateLookupFunctor)(objType const&)>
class RTree {
public:
	class TreeNode {
	public:
		cweeList< TreeNode* >
			children;
		cweeBoundary
			bound;
		cweeSharedPtr<objType> 
			object;
		TreeNode*
			parent;
		int 
			parentsChildIndex;

		TreeNode() : children(), bound(), object(nullptr), parent(nullptr), parentsChildIndex(-1) {};
		TreeNode(TreeNode const&) = default;
		TreeNode(TreeNode&&) = default;
		TreeNode& operator=(TreeNode const&) = default;
		TreeNode& operator=(TreeNode&&) = default;

		TreeNode* Next() {
			TreeNode* out{ nullptr };
			if (parent && (parentsChildIndex >= 0)) {
				if (parent->children.Num() > (parentsChildIndex + 1)) {
					out = parent->children[parentsChildIndex + 1];
				}
			}
			return out;
		};
		TreeNode* Prev() {
			TreeNode* out{ nullptr };
			if (parent && (parentsChildIndex >= 1)) {
				if ((parentsChildIndex - 1) >= 0) {
					out = parent->children[parentsChildIndex - 1];
				}
			}
			return out;
		};

		void AddChild(TreeNode* ptr) {
			if (ptr) {
				children.Append(ptr);
				if (bound.topRight.x < ptr->bound.topRight.x) bound.topRight.x = ptr->bound.topRight.x;
				if (bound.topRight.y < ptr->bound.topRight.y) bound.topRight.y = ptr->bound.topRight.y;
				if (bound.bottomLeft.x > ptr->bound.bottomLeft.x) bound.bottomLeft.x = ptr->bound.bottomLeft.x;
				if (bound.bottomLeft.y > ptr->bound.bottomLeft.y) bound.bottomLeft.y = ptr->bound.bottomLeft.y;
			}
		};
	};
	cweeAlloc<TreeNode, 10> 
		nodeAllocator;
	cweeList<cweeSharedPtr<objType>>
		objects;
	TreeNode*
		root;

public:
	RTree() : nodeAllocator(), objects(), root(nullptr)  { ReloadTree(); };
	RTree(RTree const& obj) : nodeAllocator(), objects(obj.objects), root(nullptr) { ReloadTree(); };
	RTree& operator=(const RTree& obj) { objects = obj.objects; ReloadTree(); return *this; };
	RTree& operator=(RTree&& obj) { objects = obj.objects; ReloadTree(); return *this; };
	~RTree() { nodeAllocator.Clear(); };
	
	static cweeList<vec2d> kmeans(int k, std::vector<vec2d> const& data) {
		int m = data.size();
		int n = 2;
		int i, j, l;
		double min_dist, dist;
		bool converged;
		int label;
		std::vector<vec2d> centers(k, vec2d());
		std::vector<int> labels(m);
		std::vector<std::vector<double>> new_centers(k, std::vector<double>(n));
		std::vector<int> counts(k);

		for (i = 0; i < k; ++i) centers[i] = data[i];
		while (true) {
			for (i = 0; i < k; ++i) for (j = 0; j < n; ++j) new_centers[i][j] = 0;
			for (i = 0; i < k; ++i) counts[i] = 0;
			for (i = 0; i < m; ++i) {
				min_dist = std::numeric_limits<double>::max();
				label = -1;
				for (j = 0; j < k; ++j) {
					dist = 0;
					for (l = 0; l < n; ++l) {
						dist += std::pow(data[i][l] - centers[j][l], 2);
					}
					if (dist < min_dist) {
						min_dist = dist;
						label = j;
					}
				}
				labels[i] = label;
				counts[label]++;
				for (l = 0; l < n; ++l) {
					new_centers[label][l] += data[i][l];
				}
			}
			converged = true;
			for (i = 0; i < k; ++i) {
				if (counts[i] == 0) {
					continue;
				}
				for (l = 0; l < n; ++l) {
					new_centers[i][l] /= counts[i];
					if (new_centers[i][l] != centers[i][l]) {
						converged = false;
					}
					centers[i][l] = new_centers[i][l];
				}
			}
			if (converged) {
				break;
			}
		}

		cweeList<vec2d> out;
		out = centers;
		return out;
	};
	static cweeList< cweeList<cweeSharedPtr<objType>> > Cluster(int numClusters, cweeList<cweeSharedPtr<objType>> const& objs) {
		cweeList< cweeList<cweeSharedPtr<objType>> > out;
		{
			cweeList<vec2d> coord_data;
			vec2d c;
			int childCount = 0;
			for (auto& x : objs) {
				if (x) {
					cweeBoundary bound = coordinateLookupFunctor(*x);
					coord_data.Append(bound.Center());
					childCount++;
				}
				else {
					throw(std::runtime_error("RTree object was empty."));
				}
			}

			auto newCenters = kmeans(cweeMath::min(cweeMath::max(1, childCount / 10), numClusters), coord_data);
			auto voronoi{ Voronoi(newCenters) };
			int cellN = 0;
			for (auto& cell : voronoi.GetCells()) {
				cweeList<cweeSharedPtr<objType>> cellChildren;

				for (auto& x : objs) {
					if (x) {
						cweeBoundary bound = coordinateLookupFunctor(*x);
						if (cell.overlaps(bound.Center())) {
							cellChildren.Append(x);
						}
					}
					else {
						throw(std::runtime_error("RTree object was empty."));
					}
				}

				out.Append(cellChildren);
			}
		}
		return out;
	};

	void CreateNode(TreeNode* parent, TreeNode* node, cweeList<cweeSharedPtr<objType>> const& objs) {
		if (node && objs.Num() > 0) {
			if (objs.Num() == 1) {
				node->object = objs[0];
				node->parent = parent;
				if (parent) {
					node->parentsChildIndex = parent->children.Num();
				}
				else {
					node->parentsChildIndex = -1;
				}
			} 
			else {
				cweeList< cweeList<cweeSharedPtr<objType>> > clusters = Cluster(10, objs);
				for (cweeList<cweeSharedPtr<objType>>& cluster : clusters) {
					AUTO child_node = nodeAllocator.Alloc();					
					child_node->parent = parent;
					CreateNode(node, child_node, cluster);
					node->AddChild(child_node);
					child_node->parentsChildIndex = node->children.Num() - 1;
				}
			}
		}
	};
	AUTO ReloadTree() {
		nodeAllocator.Clear();
		root = nodeAllocator.Alloc();
		return CreateNode(nullptr, root, objects);
	};

	void Add(cweeSharedPtr<objType> const& obj) {
		objects.Append(obj);
		root = nullptr;
	};
	void Remove(cweeSharedPtr<objType> const& obj) {
		objects.Remove(obj);
		root = nullptr; 
	};

	/* goes through all nodes of the tree */
	TreeNode* GetRoot() {
		if (!root) ReloadTree();
		return root;
	};
	static TreeNode* GetNext(TreeNode* node) {
		if (node) {
			if (node->children.Num() > 0) {
				node = node->children[0];
			}
			else {
				while (node && node->Next() == nullptr) {
					node = node->parent;
				}
				if (node) node = node->Next();
			}
		}
		return node;
	};
	static TreeNode* GetNextLeaf(TreeNode* node) {
		node = GetNext(node);
		while (node && !node->object) {
			node = GetNext(node);
		}
		return node;
	};
	cweeSharedPtr<objType> TryFindObject(std::function<bool(objType const&)> search) {
		TreeNode* child = GetRoot();
		child = GetNextLeaf(child);
		while (child) {
			if (child->object && search(*child->object)) {
				return child->object;
			}
			child = GetNextLeaf(child);
		}
		return nullptr;
	};


};

template <class objType, vec2d(*coordinateLookupFunctor)(objType const&)>
class cweeRTree {
public:
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

		static cweeList<vec2d> kmeans(int k, std::vector<vec2d> const& data) {
			int m = data.size();
			int n = 2;
			int i, j, l;
			double min_dist, dist;
			bool converged;
			int label;
			std::vector<vec2d> centers(k, vec2d());
			std::vector<int> labels(m);
			std::vector<std::vector<double>> new_centers(k, std::vector<double>(n));
			std::vector<int> counts(k);

			for (i = 0; i < k; ++i) centers[i] = data[i];
			while (true) {
				for (i = 0; i < k; ++i) for (j = 0; j < n; ++j) new_centers[i][j] = 0;
				for (i = 0; i < k; ++i) counts[i] = 0;
				for (i = 0; i < m; ++i) {
					min_dist = std::numeric_limits<double>::max();
					label = -1;
					for (j = 0; j < k; ++j) {
						dist = 0;
						for (l = 0; l < n; ++l) {
							dist += std::pow(data[i][l] - centers[j][l], 2);
						}
						if (dist < min_dist) {
							min_dist = dist;
							label = j;
						}
					}
					labels[i] = label;
					counts[label]++;
					for (l = 0; l < n; ++l) {
						new_centers[label][l] += data[i][l];
					}
				}
				converged = true;
				for (i = 0; i < k; ++i) {
					if (counts[i] == 0) {
						continue;
					}
					for (l = 0; l < n; ++l) {
						new_centers[i][l] /= counts[i];
						if (new_centers[i][l] != centers[i][l]) {
							converged = false;
						}
						centers[i][l] = new_centers[i][l];
					}
				}
				if (converged) {
					break;
				}
			}

			cweeList<vec2d> out;
			out = centers;
			return out;
		};

		std::vector<std::vector<TreeNode*>> cluster_children() {
			std::vector<std::vector<TreeNode*>> out(2, std::vector<TreeNode*>());
			{
				cweeList<vec2d> coord_data;
				vec2d c;
				auto* child = this->firstChild;
				int childCount = 0;
				while (child) { coord_data.Append(child->boundary.Center()); child = child->next; }

				auto newCenters = kmeans(2, coord_data);
				auto voronoi{ Voronoi(newCenters) };
				int cellN = 0;
				for (auto& cell : voronoi.GetCells()) {
					if (cellN < out.size()) {
						child = this->firstChild;
						while (child) {
							if (child->boundary.topRight == child->boundary.bottomLeft) {
								if (cell.overlaps(child->boundary.topRight)) {
									out[cellN].push_back(child);
								}
							}
							else {
								if (cell.overlaps(child->boundary.topRight, child->boundary.bottomLeft)) {
									out[cellN].push_back(child);
								}
							}
							child = child->next;
						}
						cellN++;
					}
				}
			}
			return out;
		};

	};

private:
	long long	
		Num;
	TreeNode*	
		root;
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
	cweeSharedPtr<objType> TryFindObject(std::function<bool(objType const&)> search) {
		auto* child = GetRoot();
		child = GetNextLeaf(child);
		while (child) {
			if (child->object && search(*child->object)) {
				return child->object;
			}
			child = GetNextLeaf(child);
		}
		return nullptr;
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