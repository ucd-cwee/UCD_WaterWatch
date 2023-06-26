#ifndef __BALANCED_TREE_H__
#define __BALANCED_TREE_H__

#pragma region "INCLUDES"
#pragma hdrstop
#include "precompiled.h"
#pragma endregion

template< class objType, class keyType >
class cweeBalancedTreeNode {
public:
	keyType							key = 0;								// key used for sorting
	objType*						object = nullptr;						// if != NULL pointer to object stored in leaf node
	cweeBalancedTreeNode*			parent = nullptr;						// parent node
	cweeBalancedTreeNode*			next = nullptr;							// next sibling
	cweeBalancedTreeNode*			prev = nullptr;							// prev sibling
	int								numChildren = 0;						// number of children
	cweeBalancedTreeNode*			firstChild = nullptr;					// first child
	cweeBalancedTreeNode*			lastChild = nullptr;					// last child
};

template< class objType, class keyType, int maxChildrenPerNode = 10 >
class cweeBalancedTree {
public:
	typedef cweeBalancedTreeNode<objType, keyType> _iterType;
	struct it_state {
		_iterType _node;
		_iterType* node = &_node;
		inline void begin(const cweeBalancedTree* ref) {
			node = ref->GetNextLeaf(ref->GetRoot());
			if (!node) node = &_node;
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
			return ((node->object == s.node->object) || (node->object == nullptr)) ? false : true;
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

	cweeBalancedTree& operator=(const cweeBalancedTree& obj) {
		Clear(); // empty out whatever this container had 

		for (auto& x : obj) {
			Add(*x.object, x.key, false);
		}

		return *this;
	};

	cweeBalancedTree() {
		static_assert(maxChildrenPerNode >= 4);
		Init();
	};
	~cweeBalancedTree() {
		Shutdown();
	};

	void									Reserve(int num);

	void									Add(cweeThreadedList<std::pair<keyType, objType>> const& data, bool addUnique = true);
	cweeBalancedTreeNode<objType, keyType>* Add(objType const& object, keyType const& key, bool addUnique = true);
	void									Remove(cweeBalancedTreeNode<objType, keyType>* node);
	void									Clear();
	cweeBalancedTreeNode<objType, keyType>* NodeFind(keyType  const& key) const;
	cweeBalancedTreeNode<objType, keyType>* NodeFindSmallestLargerEqual(keyType const& key) const;
	cweeBalancedTreeNode<objType, keyType>* NodeFindLargestSmallerEqual(keyType const& key) const;

	static cweeBalancedTreeNode<objType, keyType>* NodeFind(keyType  const& key, cweeBalancedTreeNode<objType, keyType>* root);
	static cweeBalancedTreeNode<objType, keyType>* NodeFindSmallestLargerEqual(keyType const& key, cweeBalancedTreeNode<objType, keyType>* Root);
	static cweeBalancedTreeNode<objType, keyType>* NodeFindLargestSmallerEqual(keyType const& key, cweeBalancedTreeNode<objType, keyType>* Root);

	objType* Find(keyType  const& key) const;
	objType* FindSmallestLargerEqual(keyType const& key) const;
	objType* FindLargestSmallerEqual(keyType const& key) const;

	cweeBalancedTreeNode<objType, keyType>* GetRoot() const;
	int										GetNodeCount() const;
	static cweeBalancedTreeNode<objType, keyType>* GetNext(cweeBalancedTreeNode<objType, keyType>* node);
	static cweeBalancedTreeNode<objType, keyType>* GetNextLeaf(cweeBalancedTreeNode<objType, keyType>* node);
	static cweeBalancedTreeNode<objType, keyType>* GetPrevLeaf(cweeBalancedTreeNode<objType, keyType>* node);

private:
    cweeBlockAlloc<objType, 128>										objAllocator;
    int																	Num = 0;
    cweeBlockAlloc<cweeBalancedTreeNode<objType, keyType>, 128>			nodeAllocator;
    cweeBalancedTreeNode<objType, keyType>*								root = nullptr;

	void									Init();
	void									Shutdown();
	cweeBalancedTreeNode<objType, keyType>* AllocNode();
	void									FreeNode(cweeBalancedTreeNode<objType, keyType>* node);
	void									SplitNode(cweeBalancedTreeNode<objType, keyType>* node);
	cweeBalancedTreeNode<objType, keyType>* MergeNodes(cweeBalancedTreeNode<objType, keyType>* node1, cweeBalancedTreeNode<objType, keyType>* node2);
};


#endif