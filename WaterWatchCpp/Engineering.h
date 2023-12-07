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
#include "Strings.h"
#include "List.h"
#include "Parser.h"
#include "cwee_math.h"
#include "vec.h"
#include "Curve.h"
#include "cweeUnitedValue.h"
#include <cassert>
#include <memory>
#include <stdexcept>
#include <iterator>
#include <set>
#include <queue>
#include <algorithm>
#include <cstddef>

INLINE int vec3d_compare(const void* a, const void* b) {
	vec3d& aR = *((vec3d*)a);
	vec3d& bR = *((vec3d*)b);
	if (aR.x < bR.x) return 1;
	else return 0;
	//return -1;
}
INLINE void vec3d_swap(vec3d* a, vec3d* b, vec3d& temp) {
	temp = *a;
	*a = *b;
	*b = temp;
}
INLINE int vec3d_partition(vec3d* arr, int low, int high) {
	auto& pivot = arr[cweeRandomInt(low, high)];    // pivot // was 'high' -- randomizing the pivot should result in lower complexity when the array is already sorted. 
	int i = (low - 1);			// Index of smaller element
	vec3d temp;
	for (int j = low; j <= high - 1; j++)
	{
		// If current element is smaller than or
		// equal to pivot
		if (vec3d_compare(&arr[j], &pivot) <= 0)
		{
			i++;    // increment index of smaller element
			vec3d_swap(&arr[i], &arr[j], temp);
		}
	}
	vec3d_swap(&arr[i + 1], &arr[high], temp);
	return (i + 1);
}
INLINE void vec3d_quickSort(vec3d* arr, int low, int high) {
	if (low < high)
	{
		/* pi is partitioning index, arr[p] is now
		   at right place */
		int pi = vec3d_partition(arr, low, high);

		// Separately sort elements before
		// partition and after partition
		vec3d_quickSort(arr, low, pi - 1);
		vec3d_quickSort(arr, pi + 1, high);
	}
}

namespace treeNS
{
	//	STL-like templated tree class.
	//
	// Copyright (C) 2001-2015 Kasper Peeters <kasper@phi-sci.com>
	// Distributed under the GNU General Public License version 3.
	//
	// Special permission to use tree.hh under the conditions of a 
	// different license can be requested from the author.

	/** \mainpage tree.hh
		\author   Kasper Peeters
		\version  3.4
		\date     23-Jan-2016
		\see      http://tree.phi-sci.com/
		\see      http://tree.phi-sci.com/ChangeLog

	   The tree.hh library for C++ provides an STL-like container class
	   for n-ary trees, templated over the data stored at the
	   nodes. Various types of iterators are provided (post-order,
	   pre-order, and others). Where possible the access methods are
	   compatible with the STL or alternative algorithms are
	   available.
	*/

	/// A node in the tree, combining links to other nodes as well as the actual data.
	template<class T>
	class tree_node_ { // size: 5*4=20 bytes (on 32 bit arch), can be reduced by 8.
	public:
		tree_node_();
		tree_node_(const T&);
		tree_node_(T&&);

		tree_node_<T>* parent;
		tree_node_<T>* first_child, * last_child;
		tree_node_<T>* prev_sibling, * next_sibling;
		T data;
	};

	template<class T>
	tree_node_<T>::tree_node_()
		: parent(0), first_child(0), last_child(0), prev_sibling(0), next_sibling(0)
	{
	}

	template<class T>
	tree_node_<T>::tree_node_(const T& val)
		: parent(0), first_child(0), last_child(0), prev_sibling(0), next_sibling(0), data(val)
	{
	}

	template<class T>
	tree_node_<T>::tree_node_(T&& val)
		: parent(0), first_child(0), last_child(0), prev_sibling(0), next_sibling(0), data(val)
	{
	}

	template <class T, class tree_node_allocator = std::allocator<tree_node_<T> > >
	class tree {
	protected:
		typedef tree_node_<T> tree_node;
	public:
		/// Value of the data stored at a node.
		typedef T value_type;

		class iterator_base;
		class pre_order_iterator;
		class post_order_iterator;
		class sibling_iterator;
		class leaf_iterator;

		tree();                                         // empty constructor
		tree(const T&);                                 // constructor setting given element as head
		tree(const iterator_base&);
		tree(const tree<T, tree_node_allocator>&);      // copy constructor
		tree(tree<T, tree_node_allocator>&&);           // move constructor
		~tree();
		tree<T, tree_node_allocator>& operator=(const tree<T, tree_node_allocator>&);   // copy assignment
		tree<T, tree_node_allocator>& operator=(tree<T, tree_node_allocator>&&);        // move assignment

	  /// Base class for iterators, only pointers stored, no traversal logic.
#ifdef __SGI_STL_PORT
		class iterator_base : public stlport::bidirectional_iterator<T, ptrdiff_t> {
#else
		class iterator_base {
#endif
		public:
			typedef T                               value_type;
			typedef T* pointer;
			typedef T& reference;
			typedef size_t                          size_type;
			typedef ptrdiff_t                       difference_type;
			typedef std::bidirectional_iterator_tag iterator_category;

			iterator_base();
			iterator_base(tree_node*);

			T& operator*() const;
			T* operator->() const;

			/// When called, the next increment/decrement skips children of this node.
			void         skip_children();
			void         skip_children(bool skip);
			/// Number of children of the node pointed to by the iterator.
			unsigned int number_of_children() const;

			sibling_iterator begin() const;
			sibling_iterator end() const;

			tree_node* node;
		protected:
			bool skip_current_children_;
		};

		/// Depth-first iterator, first accessing the node, then its children.
		class pre_order_iterator : public iterator_base {
		public:
			pre_order_iterator();
			pre_order_iterator(tree_node*);
			pre_order_iterator(const iterator_base&);
			pre_order_iterator(const sibling_iterator&);

			bool    operator==(const pre_order_iterator&) const;
			bool    operator!=(const pre_order_iterator&) const;
			pre_order_iterator& operator++();
			pre_order_iterator& operator--();
			pre_order_iterator   operator++(int);
			pre_order_iterator   operator--(int);
			pre_order_iterator& operator+=(unsigned int);
			pre_order_iterator& operator-=(unsigned int);

			pre_order_iterator& next_skip_children();
		};

		/// Depth-first iterator, first accessing the children, then the node itself.
		class post_order_iterator : public iterator_base {
		public:
			post_order_iterator();
			post_order_iterator(tree_node*);
			post_order_iterator(const iterator_base&);
			post_order_iterator(const sibling_iterator&);

			bool    operator==(const post_order_iterator&) const;
			bool    operator!=(const post_order_iterator&) const;
			post_order_iterator& operator++();
			post_order_iterator& operator--();
			post_order_iterator   operator++(int);
			post_order_iterator   operator--(int);
			post_order_iterator& operator+=(unsigned int);
			post_order_iterator& operator-=(unsigned int);

			/// Set iterator to the first child as deep as possible down the tree.
			void descend_all();
		};

		/// Breadth-first iterator, using a queue
		class breadth_first_queued_iterator : public iterator_base {
		public:
			breadth_first_queued_iterator();
			breadth_first_queued_iterator(tree_node*);
			breadth_first_queued_iterator(const iterator_base&);

			bool    operator==(const breadth_first_queued_iterator&) const;
			bool    operator!=(const breadth_first_queued_iterator&) const;
			breadth_first_queued_iterator& operator++();
			breadth_first_queued_iterator   operator++(int);
			breadth_first_queued_iterator& operator+=(unsigned int);

		private:
			std::queue<tree_node*> traversal_queue;
		};

		/// The default iterator types throughout the tree class.
		typedef pre_order_iterator            iterator;
		typedef breadth_first_queued_iterator breadth_first_iterator;

		/// Iterator which traverses only the nodes at a given depth from the root.
		class fixed_depth_iterator : public iterator_base {
		public:
			fixed_depth_iterator();
			fixed_depth_iterator(tree_node*);
			fixed_depth_iterator(const iterator_base&);
			fixed_depth_iterator(const sibling_iterator&);
			fixed_depth_iterator(const fixed_depth_iterator&);

			bool    operator==(const fixed_depth_iterator&) const;
			bool    operator!=(const fixed_depth_iterator&) const;
			fixed_depth_iterator& operator++();
			fixed_depth_iterator& operator--();
			fixed_depth_iterator   operator++(int);
			fixed_depth_iterator   operator--(int);
			fixed_depth_iterator& operator+=(unsigned int);
			fixed_depth_iterator& operator-=(unsigned int);

			tree_node* top_node;
		};

		/// Iterator which traverses only the nodes which are siblings of each other.
		class sibling_iterator : public iterator_base {
		public:
			sibling_iterator();
			sibling_iterator(tree_node*);
			sibling_iterator(const sibling_iterator&);
			sibling_iterator(const iterator_base&);

			bool    operator==(const sibling_iterator&) const;
			bool    operator!=(const sibling_iterator&) const;
			sibling_iterator& operator++();
			sibling_iterator& operator--();
			sibling_iterator   operator++(int);
			sibling_iterator   operator--(int);
			sibling_iterator& operator+=(unsigned int);
			sibling_iterator& operator-=(unsigned int);

			tree_node* range_first() const;
			tree_node* range_last() const;
			tree_node* parent_;
		private:
			void set_parent_();
		};

		/// Iterator which traverses only the leaves.
		class leaf_iterator : public iterator_base {
		public:
			leaf_iterator();
			leaf_iterator(tree_node*, tree_node* top = 0);
			leaf_iterator(const sibling_iterator&);
			leaf_iterator(const iterator_base&);

			bool    operator==(const leaf_iterator&) const;
			bool    operator!=(const leaf_iterator&) const;
			leaf_iterator& operator++();
			leaf_iterator& operator--();
			leaf_iterator   operator++(int);
			leaf_iterator   operator--(int);
			leaf_iterator& operator+=(unsigned int);
			leaf_iterator& operator-=(unsigned int);
		private:
			tree_node* top_node;
		};

		/// Return iterator to the beginning of the tree.
		inline pre_order_iterator   begin() const;
		/// Return iterator to the end of the tree.
		inline pre_order_iterator   end() const;
		/// Return post-order iterator to the beginning of the tree.
		post_order_iterator  begin_post() const;
		/// Return post-order end iterator of the tree.
		post_order_iterator  end_post() const;
		/// Return fixed-depth iterator to the first node at a given depth from the given iterator.
		fixed_depth_iterator begin_fixed(const iterator_base&, unsigned int) const;
		/// Return fixed-depth end iterator.
		fixed_depth_iterator end_fixed(const iterator_base&, unsigned int) const;
		/// Return breadth-first iterator to the first node at a given depth.
		breadth_first_queued_iterator begin_breadth_first() const;
		/// Return breadth-first end iterator.
		breadth_first_queued_iterator end_breadth_first() const;
		/// Return sibling iterator to the first child of given node.
		sibling_iterator     begin(const iterator_base&) const;
		/// Return sibling end iterator for children of given node.
		sibling_iterator     end(const iterator_base&) const;
		/// Return leaf iterator to the first leaf of the tree.
		leaf_iterator   begin_leaf() const;
		/// Return leaf end iterator for entire tree.
		leaf_iterator   end_leaf() const;
		/// Return leaf iterator to the first leaf of the subtree at the given node.
		leaf_iterator   begin_leaf(const iterator_base& top) const;
		/// Return leaf end iterator for the subtree at the given node.
		leaf_iterator   end_leaf(const iterator_base& top) const;

		/// Return iterator to the parent of a node.
		template<typename	iter> static iter parent(iter);
		/// Return iterator to the previous sibling of a node.
		template<typename iter> static iter previous_sibling(iter);
		/// Return iterator to the next sibling of a node.
		template<typename iter> static iter next_sibling(iter);
		/// Return iterator to the next node at a given depth.
		template<typename iter> iter next_at_same_depth(iter) const;

		/// Erase all nodes of the tree.
		void     clear();
		/// Erase element at position pointed to by iterator, return incremented iterator.
		template<typename iter> iter erase(iter);
		/// Erase all children of the node pointed to by iterator.
		void     erase_children(const iterator_base&);
		/// Erase all siblings to the right of the iterator.
		void     erase_right_siblings(const iterator_base&);
		/// Erase all siblings to the left of the iterator.
		void     erase_left_siblings(const iterator_base&);

		/// Insert empty node as last/first child of node pointed to by position.
		template<typename iter> iter append_child(iter position);
		template<typename iter> iter prepend_child(iter position);
		/// Insert node as last/first child of node pointed to by position.
		template<typename iter> iter append_child(iter position, const T& x);
		template<typename iter> iter append_child(iter position, T&& x);
		template<typename iter> iter prepend_child(iter position, const T& x);
		template<typename iter> iter prepend_child(iter position, T&& x);
		/// Append the node (plus its children) at other_position as last/first child of position.
		template<typename iter> iter append_child(iter position, iter other_position);
		template<typename iter> iter prepend_child(iter position, iter other_position);
		/// Append the nodes in the from-to range (plus their children) as last/first children of position.
		template<typename iter> iter append_children(iter position, sibling_iterator from, sibling_iterator to);
		template<typename iter> iter prepend_children(iter position, sibling_iterator from, sibling_iterator to);

		/// Short-hand to insert topmost node in otherwise empty tree.
		pre_order_iterator set_head(const T& x);
		pre_order_iterator set_head(T&& x);
		/// Insert node as previous sibling of node pointed to by position.
		template<typename iter> iter insert(iter position, const T& x);
		template<typename iter> iter insert(iter position, T&& x);
		/// Specialisation of previous member.
		sibling_iterator insert(sibling_iterator position, const T& x);
		/// Insert node (with children) pointed to by subtree as previous sibling of node pointed to by position.
		/// Does not change the subtree itself (use move_in or move_in_below for that).
		template<typename iter> iter insert_subtree(iter position, const iterator_base& subtree);
		/// Insert node as next sibling of node pointed to by position.
		template<typename iter> iter insert_after(iter position, const T& x);
		template<typename iter> iter insert_after(iter position, T&& x);
		/// Insert node (with children) pointed to by subtree as next sibling of node pointed to by position.
		template<typename iter> iter insert_subtree_after(iter position, const iterator_base& subtree);

		/// Replace node at 'position' with other node (keeping same children); 'position' becomes invalid.
		template<typename iter> iter replace(iter position, const T& x);
		/// Replace node at 'position' with subtree starting at 'from' (do not erase subtree at 'from'); see above.
		template<typename iter> iter replace(iter position, const iterator_base& from);
		/// Replace string of siblings (plus their children) with copy of a new string (with children); see above
		sibling_iterator replace(sibling_iterator orig_begin, sibling_iterator orig_end,
			sibling_iterator new_begin, sibling_iterator new_end);

		/// Move all children of node at 'position' to be siblings, returns position.
		template<typename iter> iter flatten(iter position);
		/// Move nodes in range to be children of 'position'.
		template<typename iter> iter reparent(iter position, sibling_iterator begin, sibling_iterator end);
		/// Move all child nodes of 'from' to be children of 'position'.
		template<typename iter> iter reparent(iter position, iter from);

		/// Replace node with a new node, making the old node a child of the new node.
		template<typename iter> iter wrap(iter position, const T& x);

		/// Move 'source' node (plus its children) to become the next sibling of 'target'.
		template<typename iter> iter move_after(iter target, iter source);
		/// Move 'source' node (plus its children) to become the previous sibling of 'target'.
		template<typename iter> iter move_before(iter target, iter source);
		sibling_iterator move_before(sibling_iterator target, sibling_iterator source);
		/// Move 'source' node (plus its children) to become the node at 'target' (erasing the node at 'target').
		template<typename iter> iter move_ontop(iter target, iter source);

		/// Extract the subtree starting at the indicated node, removing it from the original tree.
		tree                         move_out(iterator);
		/// Inverse of take_out: inserts the given tree as previous sibling of indicated node by a 
		/// move operation, that is, the given tree becomes empty. Returns iterator to the top node.
		template<typename iter> iter move_in(iter, tree&);
		/// As above, but now make the tree a child of the indicated node.
		template<typename iter> iter move_in_below(iter, tree&);
		/// As above, but now make the tree the nth child of the indicated node (if possible).
		template<typename iter> iter move_in_as_nth_child(iter, size_t, tree&);

		/// Merge with other tree, creating new branches and leaves only if they are not already present.
		void     merge(sibling_iterator, sibling_iterator, sibling_iterator, sibling_iterator,
			bool duplicate_leaves = false);
		/// Sort (std::sort only moves values of nodes, this one moves children as well).
		void     sort(sibling_iterator from, sibling_iterator to, bool deep = false);
		template<class StrictWeakOrdering>
		void     sort(sibling_iterator from, sibling_iterator to, StrictWeakOrdering comp, bool deep = false);
		/// Compare two ranges of nodes (compares nodes as well as tree structure).
		template<typename iter>
		bool     equal(const iter& one, const iter& two, const iter& three) const;
		template<typename iter, class BinaryPredicate>
		bool     equal(const iter& one, const iter& two, const iter& three, BinaryPredicate) const;
		template<typename iter>
		bool     equal_subtree(const iter& one, const iter& two) const;
		template<typename iter, class BinaryPredicate>
		bool     equal_subtree(const iter& one, const iter& two, BinaryPredicate) const;
		/// Extract a new tree formed by the range of siblings plus all their children.
		tree     subtree(sibling_iterator from, sibling_iterator to) const;
		void     subtree(tree&, sibling_iterator from, sibling_iterator to) const;
		/// Exchange the node (plus subtree) with its sibling node (do nothing if no sibling present).
		void     swap(sibling_iterator it);
		/// Exchange two nodes (plus subtrees)
		void     swap(iterator, iterator);

		/// Count the total number of nodes.
		size_t   size() const;
		/// Count the total number of nodes below the indicated node (plus one).
		size_t   size(const iterator_base&) const;
		/// Check if tree is empty.
		bool     empty() const;
		/// Compute the depth to the root or to a fixed other iterator.
		static int depth(const iterator_base&);
		static int depth(const iterator_base&, const iterator_base&);
		/// Determine the maximal depth of the tree. An empty tree has max_depth=-1.
		int      max_depth() const;
		/// Determine the maximal depth of the tree with top node at the given position.
		int      max_depth(const iterator_base&) const;
		/// Count the number of children of node at position.
		static unsigned int number_of_children(const iterator_base&);
		/// Count the number of siblings (left and right) of node at iterator. Total nodes at this level is +1.
		unsigned int number_of_siblings(const iterator_base&) const;
		/// Determine whether node at position is in the subtrees with root in the range.
		bool     is_in_subtree(const iterator_base& position, const iterator_base& begin,
			const iterator_base& end) const;
		/// Determine whether the iterator is an 'end' iterator and thus not actually pointing to a node.
		bool     is_valid(const iterator_base&) const;
		/// Find the lowest common ancestor of two nodes, that is, the deepest node such that
		/// both nodes are descendants of it.
		iterator lowest_common_ancestor(const iterator_base&, const iterator_base&) const;

		/// Determine the index of a node in the range of siblings to which it belongs.
		unsigned int index(sibling_iterator it) const;
		/// Inverse of 'index': return the n-th child of the node at position.
		static sibling_iterator child(const iterator_base& position, unsigned int);
		/// Return iterator to the sibling indicated by index
		sibling_iterator sibling(const iterator_base& position, unsigned int);

		/// For debugging only: verify internal consistency by inspecting all pointers in the tree
		/// (which will also trigger a valgrind error in case something got corrupted).
		void debug_verify_consistency() const;

		/// Comparator class for iterators (compares pointer values; why doesn't this work automatically?)
		class iterator_base_less {
		public:
			bool operator()(const typename tree<T, tree_node_allocator>::iterator_base& one,
				const typename tree<T, tree_node_allocator>::iterator_base& two) const
			{
				return one.node < two.node;
			}
		};
		tree_node* head, * feet;    // head/feet are always dummy; if an iterator points to them it is invalid
	private:
		tree_node_allocator alloc_;
		void head_initialise_();
		void copy_(const tree<T, tree_node_allocator>& other);

		/// Comparator class for two nodes of a tree (used for sorting and searching).
		template<class StrictWeakOrdering>
		class compare_nodes {
		public:
			compare_nodes(StrictWeakOrdering comp) : comp_(comp) {};

			bool operator()(const tree_node* a, const tree_node* b)
			{
				return comp_(a->data, b->data);
			}
		private:
			StrictWeakOrdering comp_;
		};
		};

	//template <class T, class tree_node_allocator>
	//class iterator_base_less {
	//	public:
	//		bool operator()(const typename tree<T, tree_node_allocator>::iterator_base& one,
	//						  const typename tree<T, tree_node_allocator>::iterator_base& two) const
	//			{
	//			txtout << "operatorclass<" << one.node < two.node << std::endl;
	//			return one.node < two.node;
	//			}
	//};

	// template <class T, class tree_node_allocator>
	// bool operator<(const typename tree<T, tree_node_allocator>::iterator& one,
	// 					const typename tree<T, tree_node_allocator>::iterator& two)
	// 	{
	// 	txtout << "operator< " << one.node < two.node << std::endl;
	// 	if(one.node < two.node) return true;
	// 	return false;
	// 	}
	// 
	// template <class T, class tree_node_allocator>
	// bool operator==(const typename tree<T, tree_node_allocator>::iterator& one,
	// 					const typename tree<T, tree_node_allocator>::iterator& two)
	// 	{
	// 	txtout << "operator== " << one.node == two.node << std::endl;
	// 	if(one.node == two.node) return true;
	// 	return false;
	// 	}
	// 
	// template <class T, class tree_node_allocator>
	// bool operator>(const typename tree<T, tree_node_allocator>::iterator_base& one,
	// 					const typename tree<T, tree_node_allocator>::iterator_base& two)
	// 	{
	// 	txtout << "operator> " << one.node < two.node << std::endl;
	// 	if(one.node > two.node) return true;
	// 	return false;
	// 	}

	// Tree
	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::tree()
	{
		head_initialise_();
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::tree(const T& x)
	{
		head_initialise_();
		set_head(x);
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::tree(tree<T, tree_node_allocator>&& x)
	{
		head_initialise_();
		if (x.head->next_sibling != x.feet) { // move tree if non-empty only
			head->next_sibling = x.head->next_sibling;
			feet->prev_sibling = x.head->prev_sibling;
			x.head->next_sibling->prev_sibling = head;
			x.feet->prev_sibling->next_sibling = feet;
			x.head->next_sibling = x.feet;
			x.feet->prev_sibling = x.head;
		}
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::tree(const iterator_base& other)
	{
		head_initialise_();
		set_head((*other));
		replace(begin(), other);
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::~tree()
	{
		clear();
		alloc_.destroy(head);
		alloc_.destroy(feet);
		alloc_.deallocate(head, 1);
		alloc_.deallocate(feet, 1);
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::head_initialise_()
	{
		head = alloc_.allocate(1, 0); // MSVC does not have default second argument 
		feet = alloc_.allocate(1, 0);
		alloc_.construct(head, tree_node_<T>());
		alloc_.construct(feet, tree_node_<T>());

		head->parent = 0;
		head->first_child = 0;
		head->last_child = 0;
		head->prev_sibling = 0; //head;
		head->next_sibling = feet; //head;

		feet->parent = 0;
		feet->first_child = 0;
		feet->last_child = 0;
		feet->prev_sibling = head;
		feet->next_sibling = 0;
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>& tree<T, tree_node_allocator>::operator=(const tree<T, tree_node_allocator>& other)
	{
		if (this != &other)
			copy_(other);
		return *this;
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>& tree<T, tree_node_allocator>::operator=(tree<T, tree_node_allocator>&& x)
	{
		if (this != &x) {
			head->next_sibling = x.head->next_sibling;
			feet->prev_sibling = x.head->prev_sibling;
			x.head->next_sibling->prev_sibling = head;
			x.feet->prev_sibling->next_sibling = feet;
			x.head->next_sibling = x.feet;
			x.feet->prev_sibling = x.head;
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::tree(const tree<T, tree_node_allocator>& other)
	{
		head_initialise_();
		copy_(other);
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::copy_(const tree<T, tree_node_allocator>& other)
	{
		clear();
		pre_order_iterator it = other.begin(), to = begin();
		while (it != other.end()) {
			to = insert(to, (*it));
			it.skip_children();
			++it;
		}
		to = begin();
		it = other.begin();
		while (it != other.end()) {
			to = replace(to, it);
			to.skip_children();
			it.skip_children();
			++to;
			++it;
		}
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::clear()
	{
		if (head)
			while (head->next_sibling != feet)
				erase(pre_order_iterator(head->next_sibling));
	}

	template<class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::erase_children(const iterator_base& it)
	{
		//	std::cout << "erase_children " << it.node << std::endl;
		if (it.node == 0) return;

		tree_node* cur = it.node->first_child;
		tree_node* prev = 0;

		while (cur != 0) {
			prev = cur;
			cur = cur->next_sibling;
			erase_children(pre_order_iterator(prev));
			//		kp::destructor(&prev->data);
			alloc_.destroy(prev);
			alloc_.deallocate(prev, 1);
		}
		it.node->first_child = 0;
		it.node->last_child = 0;
		//	std::cout << "exit" << std::endl;
	}

	template<class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::erase_right_siblings(const iterator_base& it)
	{
		if (it.node == 0) return;

		tree_node* cur = it.node->next_sibling;
		tree_node* prev = 0;

		while (cur != 0) {
			prev = cur;
			cur = cur->next_sibling;
			erase_children(pre_order_iterator(prev));
			//		kp::destructor(&prev->data);
			alloc_.destroy(prev);
			alloc_.deallocate(prev, 1);
		}
		it.node->next_sibling = 0;
		if (it.node->parent != 0)
			it.node->parent->last_child = it.node;
	}

	template<class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::erase_left_siblings(const iterator_base& it)
	{
		if (it.node == 0) return;

		tree_node* cur = it.node->prev_sibling;
		tree_node* prev = 0;

		while (cur != 0) {
			prev = cur;
			cur = cur->prev_sibling;
			erase_children(pre_order_iterator(prev));
			//		kp::destructor(&prev->data);
			alloc_.destroy(prev);
			alloc_.deallocate(prev, 1);
		}
		it.node->prev_sibling = 0;
		if (it.node->parent != 0)
			it.node->parent->first_child = it.node;
	}

	template<class T, class tree_node_allocator>
	template<class iter>
	iter tree<T, tree_node_allocator>::erase(iter it)
	{
		tree_node* cur = it.node;
		assert(cur != head);
		iter ret = it;
		ret.skip_children();
		++ret;
		erase_children(it);
		if (cur->prev_sibling == 0) {
			cur->parent->first_child = cur->next_sibling;
		}
		else {
			cur->prev_sibling->next_sibling = cur->next_sibling;
		}
		if (cur->next_sibling == 0) {
			cur->parent->last_child = cur->prev_sibling;
		}
		else {
			cur->next_sibling->prev_sibling = cur->prev_sibling;
		}

		//	kp::destructor(&cur->data);
		alloc_.destroy(cur);
		alloc_.deallocate(cur, 1);
		return ret;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::begin() const
	{
		return pre_order_iterator(head->next_sibling);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::end() const
	{
		return pre_order_iterator(feet);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::breadth_first_queued_iterator tree<T, tree_node_allocator>::begin_breadth_first() const
	{
		return breadth_first_queued_iterator(head->next_sibling);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::breadth_first_queued_iterator tree<T, tree_node_allocator>::end_breadth_first() const
	{
		return breadth_first_queued_iterator();
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::begin_post() const
	{
		tree_node* tmp = head->next_sibling;
		if (tmp != feet) {
			while (tmp->first_child)
				tmp = tmp->first_child;
		}
		return post_order_iterator(tmp);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::end_post() const
	{
		return post_order_iterator(feet);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::begin_fixed(const iterator_base& pos, unsigned int dp) const
	{
		typename tree<T, tree_node_allocator>::fixed_depth_iterator ret;
		ret.top_node = pos.node;

		tree_node* tmp = pos.node;
		unsigned int curdepth = 0;
		while (curdepth < dp) { // go down one level
			while (tmp->first_child == 0) {
				if (tmp->next_sibling == 0) {
					// try to walk up and then right again
					do {
						if (tmp == ret.top_node)
							throw std::range_error("tree: begin_fixed out of range");
						tmp = tmp->parent;
						if (tmp == 0)
							throw std::range_error("tree: begin_fixed out of range");
						--curdepth;
					} while (tmp->next_sibling == 0);
				}
				tmp = tmp->next_sibling;
			}
			tmp = tmp->first_child;
			++curdepth;
		}

		ret.node = tmp;
		return ret;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::end_fixed(const iterator_base& pos, unsigned int dp) const
	{
		assert(1 == 0); // FIXME: not correct yet: use is_valid() as a temporary workaround 
		tree_node* tmp = pos.node;
		unsigned int curdepth = 1;
		while (curdepth < dp) { // go down one level
			while (tmp->first_child == 0) {
				tmp = tmp->next_sibling;
				if (tmp == 0)
					throw std::range_error("tree: end_fixed out of range");
			}
			tmp = tmp->first_child;
			++curdepth;
		}
		return tmp;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::begin(const iterator_base& pos) const
	{
		assert(pos.node != 0);
		if (pos.node->first_child == 0) {
			return end(pos);
		}
		return pos.node->first_child;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::end(const iterator_base& pos) const
	{
		sibling_iterator ret(0);
		ret.parent_ = pos.node;
		return ret;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::begin_leaf() const
	{
		tree_node* tmp = head->next_sibling;
		if (tmp != feet) {
			while (tmp->first_child)
				tmp = tmp->first_child;
		}
		return leaf_iterator(tmp);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::end_leaf() const
	{
		return leaf_iterator(feet);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::begin_leaf(const iterator_base& top) const
	{
		tree_node* tmp = top.node;
		while (tmp->first_child)
			tmp = tmp->first_child;
		return leaf_iterator(tmp, top.node);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::end_leaf(const iterator_base& top) const
	{
		return leaf_iterator(top.node, top.node);
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::parent(iter position)
	{
		assert(position.node != 0);
		return iter(position.node->parent);
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::previous_sibling(iter position)
	{
		assert(position.node != 0);
		iter ret(position);
		ret.node = position.node->prev_sibling;
		return ret;
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::next_sibling(iter position)
	{
		assert(position.node != 0);
		iter ret(position);
		ret.node = position.node->next_sibling;
		return ret;
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::next_at_same_depth(iter position) const
	{
		// We make use of a temporary fixed_depth iterator to implement this.

		typename tree<T, tree_node_allocator>::fixed_depth_iterator tmp(position.node);

		++tmp;
		return iter(tmp);

		//	assert(position.node!=0);
		//	iter ret(position);
		//
		//	if(position.node->next_sibling) {
		//		ret.node=position.node->next_sibling;
		//		}
		//	else { 
		//		int relative_depth=0;
		//	   upper:
		//		do {
		//			ret.node=ret.node->parent;
		//			if(ret.node==0) return ret;
		//			--relative_depth;
		//			} while(ret.node->next_sibling==0);
		//	   lower:
		//		ret.node=ret.node->next_sibling;
		//		while(ret.node->first_child==0) {
		//			if(ret.node->next_sibling==0)
		//				goto upper;
		//			ret.node=ret.node->next_sibling;
		//			if(ret.node==0) return ret;
		//			}
		//		while(relative_depth<0 && ret.node->first_child!=0) {
		//			ret.node=ret.node->first_child;
		//			++relative_depth;
		//			}
		//		if(relative_depth<0) {
		//			if(ret.node->next_sibling==0) goto upper;
		//			else                          goto lower;
		//			}
		//		}
		//	return ret;
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::append_child(iter position)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, tree_node_<T>());
		//	kp::constructor(&tmp->data);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node;
		if (position.node->last_child != 0) {
			position.node->last_child->next_sibling = tmp;
		}
		else {
			position.node->first_child = tmp;
		}
		tmp->prev_sibling = position.node->last_child;
		position.node->last_child = tmp;
		tmp->next_sibling = 0;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::prepend_child(iter position)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, tree_node_<T>());
		//	kp::constructor(&tmp->data);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node;
		if (position.node->first_child != 0) {
			position.node->first_child->prev_sibling = tmp;
		}
		else {
			position.node->last_child = tmp;
		}
		tmp->next_sibling = position.node->first_child;
		position.node->prev_child = tmp;
		tmp->prev_sibling = 0;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::append_child(iter position, const T& x)
	{
		// If your program fails here you probably used 'append_child' to add the top
		// node to an empty tree. From version 1.45 the top element should be added
		// using 'insert'. See the documentation for further information, and sorry about
		// the API change.
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, x);
		//	kp::constructor(&tmp->data, x);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node;
		if (position.node->last_child != 0) {
			position.node->last_child->next_sibling = tmp;
		}
		else {
			position.node->first_child = tmp;
		}
		tmp->prev_sibling = position.node->last_child;
		position.node->last_child = tmp;
		tmp->next_sibling = 0;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::append_child(iter position, T&& x)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp); // Here is where the move semantics kick in
		std::swap(tmp->data, x);

		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node;
		if (position.node->last_child != 0) {
			position.node->last_child->next_sibling = tmp;
		}
		else {
			position.node->first_child = tmp;
		}
		tmp->prev_sibling = position.node->last_child;
		position.node->last_child = tmp;
		tmp->next_sibling = 0;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::prepend_child(iter position, const T& x)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, x);
		//	kp::constructor(&tmp->data, x);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node;
		if (position.node->first_child != 0) {
			position.node->first_child->prev_sibling = tmp;
		}
		else {
			position.node->last_child = tmp;
		}
		tmp->next_sibling = position.node->first_child;
		position.node->first_child = tmp;
		tmp->prev_sibling = 0;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::prepend_child(iter position, T&& x)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp);
		std::swap(tmp->data, x);

		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node;
		if (position.node->first_child != 0) {
			position.node->first_child->prev_sibling = tmp;
		}
		else {
			position.node->last_child = tmp;
		}
		tmp->next_sibling = position.node->first_child;
		position.node->first_child = tmp;
		tmp->prev_sibling = 0;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::append_child(iter position, iter other)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		sibling_iterator aargh = append_child(position, value_type());
		return replace(aargh, other);
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::prepend_child(iter position, iter other)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		sibling_iterator aargh = prepend_child(position, value_type());
		return replace(aargh, other);
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::append_children(iter position, sibling_iterator from, sibling_iterator to)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		iter ret = from;

		while (from != to) {
			insert_subtree(position.end(), from);
			++from;
		}
		return ret;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::prepend_children(iter position, sibling_iterator from, sibling_iterator to)
	{
		assert(position.node != head);
		assert(position.node != feet);
		assert(position.node);

		iter ret = from;

		while (from != to) {
			insert_subtree(position.begin(), from);
			++from;
		}
		return ret;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::set_head(const T& x)
	{
		assert(head->next_sibling == feet);
		return insert(iterator(feet), x);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::set_head(T&& x)
	{
		assert(head->next_sibling == feet);
		return insert(iterator(feet), x);
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::insert(iter position, const T& x)
	{
		if (position.node == 0) {
			position.node = feet; // Backward compatibility: when calling insert on a null node,
								// insert before the feet.
		}
		assert(position.node != head); // Cannot insert before head.

		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, x);
		//	kp::constructor(&tmp->data, x);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node->parent;
		tmp->next_sibling = position.node;
		tmp->prev_sibling = position.node->prev_sibling;
		position.node->prev_sibling = tmp;

		if (tmp->prev_sibling == 0) {
			if (tmp->parent) // when inserting nodes at the head, there is no parent
				tmp->parent->first_child = tmp;
		}
		else
			tmp->prev_sibling->next_sibling = tmp;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::insert(iter position, T&& x)
	{
		if (position.node == 0) {
			position.node = feet; // Backward compatibility: when calling insert on a null node,
								// insert before the feet.
		}
		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp);
		std::swap(tmp->data, x); // Move semantics
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node->parent;
		tmp->next_sibling = position.node;
		tmp->prev_sibling = position.node->prev_sibling;
		position.node->prev_sibling = tmp;

		if (tmp->prev_sibling == 0) {
			if (tmp->parent) // when inserting nodes at the head, there is no parent
				tmp->parent->first_child = tmp;
		}
		else
			tmp->prev_sibling->next_sibling = tmp;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::insert(sibling_iterator position, const T& x)
	{
		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, x);
		//	kp::constructor(&tmp->data, x);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->next_sibling = position.node;
		if (position.node == 0) { // iterator points to end of a subtree
			tmp->parent = position.parent_;
			tmp->prev_sibling = position.range_last();
			tmp->parent->last_child = tmp;
		}
		else {
			tmp->parent = position.node->parent;
			tmp->prev_sibling = position.node->prev_sibling;
			position.node->prev_sibling = tmp;
		}

		if (tmp->prev_sibling == 0) {
			if (tmp->parent) // when inserting nodes at the head, there is no parent
				tmp->parent->first_child = tmp;
		}
		else
			tmp->prev_sibling->next_sibling = tmp;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::insert_after(iter position, const T& x)
	{
		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, x);
		//	kp::constructor(&tmp->data, x);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node->parent;
		tmp->prev_sibling = position.node;
		tmp->next_sibling = position.node->next_sibling;
		position.node->next_sibling = tmp;

		if (tmp->next_sibling == 0) {
			if (tmp->parent) // when inserting nodes at the head, there is no parent
				tmp->parent->last_child = tmp;
		}
		else {
			tmp->next_sibling->prev_sibling = tmp;
		}
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::insert_after(iter position, T&& x)
	{
		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp);
		std::swap(tmp->data, x); // move semantics
	//	kp::constructor(&tmp->data, x);
		tmp->first_child = 0;
		tmp->last_child = 0;

		tmp->parent = position.node->parent;
		tmp->prev_sibling = position.node;
		tmp->next_sibling = position.node->next_sibling;
		position.node->next_sibling = tmp;

		if (tmp->next_sibling == 0) {
			if (tmp->parent) // when inserting nodes at the head, there is no parent
				tmp->parent->last_child = tmp;
		}
		else {
			tmp->next_sibling->prev_sibling = tmp;
		}
		return tmp;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::insert_subtree(iter position, const iterator_base& subtree)
	{
		// insert dummy
		iter it = insert(position, value_type());
		// replace dummy with subtree
		return replace(it, subtree);
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::insert_subtree_after(iter position, const iterator_base& subtree)
	{
		// insert dummy
		iter it = insert_after(position, value_type());
		// replace dummy with subtree
		return replace(it, subtree);
	}

	// template <class T, class tree_node_allocator>
	// template <class iter>
	// iter tree<T, tree_node_allocator>::insert_subtree(sibling_iterator position, iter subtree)
	// 	{
	// 	// insert dummy
	// 	iter it(insert(position, value_type()));
	// 	// replace dummy with subtree
	// 	return replace(it, subtree);
	// 	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::replace(iter position, const T& x)
	{
		//	kp::destructor(&position.node->data);
		//	kp::constructor(&position.node->data, x);
		position.node->data = x;
		//	alloc_.destroy(position.node);
		//	alloc_.construct(position.node, x);
		return position;
	}

	template <class T, class tree_node_allocator>
	template <class iter>
	iter tree<T, tree_node_allocator>::replace(iter position, const iterator_base& from)
	{
		assert(position.node != head);
		tree_node* current_from = from.node;
		tree_node* start_from = from.node;
		tree_node* current_to = position.node;

		// replace the node at position with head of the replacement tree at from
	//	std::cout << "warning!" << position.node << std::endl;
		erase_children(position);
		//	std::cout << "no warning!" << std::endl;
		tree_node* tmp = alloc_.allocate(1, 0);
		alloc_.construct(tmp, (*from));
		//	kp::constructor(&tmp->data, (*from));
		tmp->first_child = 0;
		tmp->last_child = 0;
		if (current_to->prev_sibling == 0) {
			if (current_to->parent != 0)
				current_to->parent->first_child = tmp;
		}
		else {
			current_to->prev_sibling->next_sibling = tmp;
		}
		tmp->prev_sibling = current_to->prev_sibling;
		if (current_to->next_sibling == 0) {
			if (current_to->parent != 0)
				current_to->parent->last_child = tmp;
		}
		else {
			current_to->next_sibling->prev_sibling = tmp;
		}
		tmp->next_sibling = current_to->next_sibling;
		tmp->parent = current_to->parent;
		//	kp::destructor(&current_to->data);
		alloc_.destroy(current_to);
		alloc_.deallocate(current_to, 1);
		current_to = tmp;

		// only at this stage can we fix 'last'
		tree_node* last = from.node->next_sibling;

		pre_order_iterator toit = tmp;
		// copy all children
		do {
			assert(current_from != 0);
			if (current_from->first_child != 0) {
				current_from = current_from->first_child;
				toit = append_child(toit, current_from->data);
			}
			else {
				while (current_from->next_sibling == 0 && current_from != start_from) {
					current_from = current_from->parent;
					toit = parent(toit);
					assert(current_from != 0);
				}
				current_from = current_from->next_sibling;
				if (current_from != last) {
					toit = append_child(parent(toit), current_from->data);
				}
			}
		} while (current_from != last);

		return current_to;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::replace(
		sibling_iterator orig_begin,
		sibling_iterator orig_end,
		sibling_iterator new_begin,
		sibling_iterator new_end)
	{
		tree_node* orig_first = orig_begin.node;
		tree_node* new_first = new_begin.node;
		tree_node* orig_last = orig_first;
		while ((++orig_begin) != orig_end)
			orig_last = orig_last->next_sibling;
		tree_node* new_last = new_first;
		while ((++new_begin) != new_end)
			new_last = new_last->next_sibling;

		// insert all siblings in new_first..new_last before orig_first
		bool first = true;
		pre_order_iterator ret;
		while (1 == 1) {
			pre_order_iterator tt = insert_subtree(pre_order_iterator(orig_first), pre_order_iterator(new_first));
			if (first) {
				ret = tt;
				first = false;
			}
			if (new_first == new_last)
				break;
			new_first = new_first->next_sibling;
		}

		// erase old range of siblings
		bool last = false;
		tree_node* next = orig_first;
		while (1 == 1) {
			if (next == orig_last)
				last = true;
			next = next->next_sibling;
			erase((pre_order_iterator)orig_first);
			if (last)
				break;
			orig_first = next;
		}
		return ret;
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::flatten(iter position)
	{
		if (position.node->first_child == 0)
			return position;

		tree_node* tmp = position.node->first_child;
		while (tmp) {
			tmp->parent = position.node->parent;
			tmp = tmp->next_sibling;
		}
		if (position.node->next_sibling) {
			position.node->last_child->next_sibling = position.node->next_sibling;
			position.node->next_sibling->prev_sibling = position.node->last_child;
		}
		else {
			position.node->parent->last_child = position.node->last_child;
		}
		position.node->next_sibling = position.node->first_child;
		position.node->next_sibling->prev_sibling = position.node;
		position.node->first_child = 0;
		position.node->last_child = 0;

		return position;
	}


	template <class T, class tree_node_allocator>
	template <typename iter>
	iter tree<T, tree_node_allocator>::reparent(iter position, sibling_iterator begin, sibling_iterator end)
	{
		tree_node* first = begin.node;
		tree_node* last = first;

		assert(first != position.node);

		if (begin == end) return begin;
		// determine last node
		while ((++begin) != end) {
			last = last->next_sibling;
		}
		// move subtree
		if (first->prev_sibling == 0) {
			first->parent->first_child = last->next_sibling;
		}
		else {
			first->prev_sibling->next_sibling = last->next_sibling;
		}
		if (last->next_sibling == 0) {
			last->parent->last_child = first->prev_sibling;
		}
		else {
			last->next_sibling->prev_sibling = first->prev_sibling;
		}
		if (position.node->first_child == 0) {
			position.node->first_child = first;
			position.node->last_child = last;
			first->prev_sibling = 0;
		}
		else {
			position.node->last_child->next_sibling = first;
			first->prev_sibling = position.node->last_child;
			position.node->last_child = last;
		}
		last->next_sibling = 0;

		tree_node* pos = first;
		for (;;) {
			pos->parent = position.node;
			if (pos == last) break;
			pos = pos->next_sibling;
		}

		return first;
	}

	template <class T, class tree_node_allocator>
	template <typename iter> iter tree<T, tree_node_allocator>::reparent(iter position, iter from)
	{
		if (from.node->first_child == 0) return position;
		return reparent(position, from.node->first_child, end(from));
	}

	template <class T, class tree_node_allocator>
	template <typename iter> iter tree<T, tree_node_allocator>::wrap(iter position, const T& x)
	{
		assert(position.node != 0);
		sibling_iterator fr = position, to = position;
		++to;
		iter ret = insert(position, x);
		reparent(ret, fr, to);
		return ret;
	}

	template <class T, class tree_node_allocator>
	template <typename iter> iter tree<T, tree_node_allocator>::move_after(iter target, iter source)
	{
		tree_node* dst = target.node;
		tree_node* src = source.node;
		assert(dst);
		assert(src);

		if (dst == src) return source;
		if (dst->next_sibling)
			if (dst->next_sibling == src) // already in the right spot
				return source;

		// take src out of the tree
		if (src->prev_sibling != 0) src->prev_sibling->next_sibling = src->next_sibling;
		else                     src->parent->first_child = src->next_sibling;
		if (src->next_sibling != 0) src->next_sibling->prev_sibling = src->prev_sibling;
		else                     src->parent->last_child = src->prev_sibling;

		// connect it to the new point
		if (dst->next_sibling != 0) dst->next_sibling->prev_sibling = src;
		else                     dst->parent->last_child = src;
		src->next_sibling = dst->next_sibling;
		dst->next_sibling = src;
		src->prev_sibling = dst;
		src->parent = dst->parent;
		return src;
	}

	template <class T, class tree_node_allocator>
	template <typename iter> iter tree<T, tree_node_allocator>::move_before(iter target, iter source)
	{
		tree_node* dst = target.node;
		tree_node* src = source.node;
		assert(dst);
		assert(src);

		if (dst == src) return source;
		if (dst->prev_sibling)
			if (dst->prev_sibling == src) // already in the right spot
				return source;

		// take src out of the tree
		if (src->prev_sibling != 0) src->prev_sibling->next_sibling = src->next_sibling;
		else                     src->parent->first_child = src->next_sibling;
		if (src->next_sibling != 0) src->next_sibling->prev_sibling = src->prev_sibling;
		else                     src->parent->last_child = src->prev_sibling;

		// connect it to the new point
		if (dst->prev_sibling != 0) dst->prev_sibling->next_sibling = src;
		else                     dst->parent->first_child = src;
		src->prev_sibling = dst->prev_sibling;
		dst->prev_sibling = src;
		src->next_sibling = dst;
		src->parent = dst->parent;
		return src;
	}

	// specialisation for sibling_iterators
	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::move_before(sibling_iterator target,
		sibling_iterator source)
	{
		tree_node* dst = target.node;
		tree_node* src = source.node;
		tree_node* dst_prev_sibling;
		if (dst == 0) { // must then be an end iterator
			dst_prev_sibling = target.parent_->last_child;
			assert(dst_prev_sibling);
		}
		else dst_prev_sibling = dst->prev_sibling;
		assert(src);

		if (dst == src) return source;
		if (dst_prev_sibling)
			if (dst_prev_sibling == src) // already in the right spot
				return source;

		// take src out of the tree
		if (src->prev_sibling != 0) src->prev_sibling->next_sibling = src->next_sibling;
		else                     src->parent->first_child = src->next_sibling;
		if (src->next_sibling != 0) src->next_sibling->prev_sibling = src->prev_sibling;
		else                     src->parent->last_child = src->prev_sibling;

		// connect it to the new point
		if (dst_prev_sibling != 0) dst_prev_sibling->next_sibling = src;
		else                    target.parent_->first_child = src;
		src->prev_sibling = dst_prev_sibling;
		if (dst) {
			dst->prev_sibling = src;
			src->parent = dst->parent;
		}
		src->next_sibling = dst;
		return src;
	}

	template <class T, class tree_node_allocator>
	template <typename iter> iter tree<T, tree_node_allocator>::move_ontop(iter target, iter source)
	{
		tree_node* dst = target.node;
		tree_node* src = source.node;
		assert(dst);
		assert(src);

		if (dst == src) return source;

		//	if(dst==src->prev_sibling) {
		//
		//		}

			// remember connection points
		tree_node* b_prev_sibling = dst->prev_sibling;
		tree_node* b_next_sibling = dst->next_sibling;
		tree_node* b_parent = dst->parent;

		// remove target
		erase(target);

		// take src out of the tree
		if (src->prev_sibling != 0) src->prev_sibling->next_sibling = src->next_sibling;
		else                     src->parent->first_child = src->next_sibling;
		if (src->next_sibling != 0) src->next_sibling->prev_sibling = src->prev_sibling;
		else                     src->parent->last_child = src->prev_sibling;

		// connect it to the new point
		if (b_prev_sibling != 0) b_prev_sibling->next_sibling = src;
		else                  b_parent->first_child = src;
		if (b_next_sibling != 0) b_next_sibling->prev_sibling = src;
		else                  b_parent->last_child = src;
		src->prev_sibling = b_prev_sibling;
		src->next_sibling = b_next_sibling;
		src->parent = b_parent;
		return src;
	}


	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator> tree<T, tree_node_allocator>::move_out(iterator source)
	{
		tree ret;

		// Move source node into the 'ret' tree.
		ret.head->next_sibling = source.node;
		ret.feet->prev_sibling = source.node;
		source.node->parent = 0;

		// Close the links in the current tree.
		if (source.node->prev_sibling != 0)
			source.node->prev_sibling->next_sibling = source.node->next_sibling;

		if (source.node->next_sibling != 0)
			source.node->next_sibling->prev_sibling = source.node->prev_sibling;

		// Fix source prev/next links.
		source.node->prev_sibling = ret.head;
		source.node->next_sibling = ret.feet;

		return ret; // A good compiler will move this, not copy.
	}

	template <class T, class tree_node_allocator>
	template<typename iter> iter tree<T, tree_node_allocator>::move_in(iter loc, tree& other)
	{
		if (other.head->next_sibling == other.feet) return loc; // other tree is empty

		tree_node* other_first_head = other.head->next_sibling;
		tree_node* other_last_head = other.feet->prev_sibling;

		sibling_iterator prev(loc);
		--prev;

		prev.node->next_sibling = other_first_head;
		loc.node->prev_sibling = other_last_head;
		other_first_head->prev_sibling = prev.node;
		other_last_head->next_sibling = loc.node;

		// Adjust parent pointers.
		tree_node* walk = other_first_head;
		while (true) {
			walk->parent = loc.node->parent;
			if (walk == other_last_head)
				break;
			walk = walk->next_sibling;
		}

		// Close other tree.
		other.head->next_sibling = other.feet;
		other.feet->prev_sibling = other.head;

		return other_first_head;
	}

	template <class T, class tree_node_allocator>
	template<typename iter> iter tree<T, tree_node_allocator>::move_in_as_nth_child(iter loc, size_t n, tree& other)
	{
		if (other.head->next_sibling == other.feet) return loc; // other tree is empty

		tree_node* other_first_head = other.head->next_sibling;
		tree_node* other_last_head = other.feet->prev_sibling;

		if (n == 0) {
			if (loc.node->first_child == 0) {
				loc.node->first_child = other_first_head;
				loc.node->last_child = other_last_head;
				other_last_head->next_sibling = 0;
				other_first_head->prev_sibling = 0;
			}
			else {
				loc.node->first_child->prev_sibling = other_last_head;
				other_last_head->next_sibling = loc.node->first_child;
				loc.node->first_child = other_first_head;
				other_first_head->prev_sibling = 0;
			}
		}
		else {
			--n;
			tree_node* walk = loc.node->first_child;
			while (true) {
				if (walk == 0)
					throw std::range_error("tree: move_in_as_nth_child position out of range");
				if (n == 0)
					break;
				--n;
				walk = walk->next_sibling;
			}
			if (walk->next_sibling == 0)
				loc.node->last_child = other_last_head;
			else
				walk->next_sibling->prev_sibling = other_last_head;
			other_last_head->next_sibling = walk->next_sibling;
			walk->next_sibling = other_first_head;
			other_first_head->prev_sibling = walk;
		}

		// Adjust parent pointers.
		tree_node* walk = other_first_head;
		while (true) {
			walk->parent = loc.node;
			if (walk == other_last_head)
				break;
			walk = walk->next_sibling;
		}

		// Close other tree.
		other.head->next_sibling = other.feet;
		other.feet->prev_sibling = other.head;

		return other_first_head;
	}


	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::merge(sibling_iterator to1, sibling_iterator to2,
		sibling_iterator from1, sibling_iterator from2,
		bool duplicate_leaves)
	{
		sibling_iterator fnd;
		while (from1 != from2) {
			if ((fnd = std::find(to1, to2, (*from1))) != to2) { // element found
				if (from1.begin() == from1.end()) { // full depth reached
					if (duplicate_leaves)
						append_child(parent(to1), (*from1));
				}
				else { // descend further
					merge(fnd.begin(), fnd.end(), from1.begin(), from1.end(), duplicate_leaves);
				}
			}
			else { // element missing
				insert_subtree(to2, from1);
			}
			++from1;
		}
	}


	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::sort(sibling_iterator from, sibling_iterator to, bool deep)
	{
		std::less<T> comp;
		sort(from, to, comp, deep);
	}

	template <class T, class tree_node_allocator>
	template <class StrictWeakOrdering>
	void tree<T, tree_node_allocator>::sort(sibling_iterator from, sibling_iterator to,
		StrictWeakOrdering comp, bool deep)
	{
		if (from == to) return;
		// make list of sorted nodes
		// CHECK: if multiset stores equivalent nodes in the order in which they
		// are inserted, then this routine should be called 'stable_sort'.
		std::multiset<tree_node*, compare_nodes<StrictWeakOrdering> > nodes(comp);
		sibling_iterator it = from, it2 = to;
		while (it != to) {
			nodes.insert(it.node);
			++it;
		}
		// reassemble
		--it2;

		// prev and next are the nodes before and after the sorted range
		tree_node* prev = from.node->prev_sibling;
		tree_node* next = it2.node->next_sibling;
		typename std::multiset<tree_node*, compare_nodes<StrictWeakOrdering> >::iterator nit = nodes.begin(), eit = nodes.end();
		if (prev == 0) {
			if ((*nit)->parent != 0) // to catch "sorting the head" situations, when there is no parent
				(*nit)->parent->first_child = (*nit);
		}
		else prev->next_sibling = (*nit);

		--eit;
		while (nit != eit) {
			(*nit)->prev_sibling = prev;
			if (prev)
				prev->next_sibling = (*nit);
			prev = (*nit);
			++nit;
		}
		// prev now points to the last-but-one node in the sorted range
		if (prev)
			prev->next_sibling = (*eit);

		// eit points to the last node in the sorted range.
		(*eit)->next_sibling = next;
		(*eit)->prev_sibling = prev; // missed in the loop above
		if (next == 0) {
			if ((*eit)->parent != 0) // to catch "sorting the head" situations, when there is no parent
				(*eit)->parent->last_child = (*eit);
		}
		else next->prev_sibling = (*eit);

		if (deep) {	// sort the children of each node too
			sibling_iterator bcs(*nodes.begin());
			sibling_iterator ecs(*eit);
			++ecs;
			while (bcs != ecs) {
				sort(begin(bcs), end(bcs), comp, deep);
				++bcs;
			}
		}
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	bool tree<T, tree_node_allocator>::equal(const iter& one_, const iter& two, const iter& three_) const
	{
		std::equal_to<T> comp;
		return equal(one_, two, three_, comp);
	}

	template <class T, class tree_node_allocator>
	template <typename iter>
	bool tree<T, tree_node_allocator>::equal_subtree(const iter& one_, const iter& two_) const
	{
		std::equal_to<T> comp;
		return equal_subtree(one_, two_, comp);
	}

	template <class T, class tree_node_allocator>
	template <typename iter, class BinaryPredicate>
	bool tree<T, tree_node_allocator>::equal(const iter& one_, const iter& two, const iter& three_, BinaryPredicate fun) const
	{
		pre_order_iterator one(one_), three(three_);

		//	if(one==two && is_valid(three) && three.number_of_children()!=0)
		//		return false;
		while (one != two && is_valid(three)) {
			if (!fun(*one, *three))
				return false;
			if (one.number_of_children() != three.number_of_children())
				return false;
			++one;
			++three;
		}
		return true;
	}

	template <class T, class tree_node_allocator>
	template <typename iter, class BinaryPredicate>
	bool tree<T, tree_node_allocator>::equal_subtree(const iter& one_, const iter& two_, BinaryPredicate fun) const
	{
		pre_order_iterator one(one_), two(two_);

		if (!fun(*one, *two)) return false;
		if (number_of_children(one) != number_of_children(two)) return false;
		return equal(begin(one), end(one), begin(two), fun);
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator> tree<T, tree_node_allocator>::subtree(sibling_iterator from, sibling_iterator to) const
	{
		assert(from != to); // if from==to, the range is empty, hence no tree to return.

		tree tmp;
		tmp.set_head(value_type());
		tmp.replace(tmp.begin(), tmp.end(), from, to);
		return tmp;
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::subtree(tree& tmp, sibling_iterator from, sibling_iterator to) const
	{
		assert(from != to); // if from==to, the range is empty, hence no tree to return.

		tmp.set_head(value_type());
		tmp.replace(tmp.begin(), tmp.end(), from, to);
	}

	template <class T, class tree_node_allocator>
	size_t tree<T, tree_node_allocator>::size() const
	{
		size_t i = 0;
		pre_order_iterator it = begin(), eit = end();
		while (it != eit) {
			++i;
			++it;
		}
		return i;
	}

	template <class T, class tree_node_allocator>
	size_t tree<T, tree_node_allocator>::size(const iterator_base& top) const
	{
		size_t i = 0;
		pre_order_iterator it = top, eit = top;
		eit.skip_children();
		++eit;
		while (it != eit) {
			++i;
			++it;
		}
		return i;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::empty() const
	{
		pre_order_iterator it = begin(), eit = end();
		return (it == eit);
	}

	template <class T, class tree_node_allocator>
	int tree<T, tree_node_allocator>::depth(const iterator_base& it)
	{
		tree_node* pos = it.node;
		assert(pos != 0);
		int ret = 0;
		while (pos->parent != 0) {
			pos = pos->parent;
			++ret;
		}
		return ret;
	}

	template <class T, class tree_node_allocator>
	int tree<T, tree_node_allocator>::depth(const iterator_base& it, const iterator_base& root)
	{
		tree_node* pos = it.node;
		assert(pos != 0);
		int ret = 0;
		while (pos->parent != 0 && pos != root.node) {
			pos = pos->parent;
			++ret;
		}
		return ret;
	}

	template <class T, class tree_node_allocator>
	int tree<T, tree_node_allocator>::max_depth() const
	{
		int maxd = -1;
		for (tree_node* it = head->next_sibling; it != feet; it = it->next_sibling)
			maxd = std::max(maxd, max_depth(it));

		return maxd;
	}


	template <class T, class tree_node_allocator>
	int tree<T, tree_node_allocator>::max_depth(const iterator_base& pos) const
	{
		tree_node* tmp = pos.node;

		if (tmp == 0 || tmp == head || tmp == feet) return -1;

		int curdepth = 0, maxdepth = 0;
		while (true) { // try to walk the bottom of the tree
			while (tmp->first_child == 0) {
				if (tmp == pos.node) return maxdepth;
				if (tmp->next_sibling == 0) {
					// try to walk up and then right again
					do {
						tmp = tmp->parent;
						if (tmp == 0) return maxdepth;
						--curdepth;
					} while (tmp->next_sibling == 0);
				}
				if (tmp == pos.node) return maxdepth;
				tmp = tmp->next_sibling;
			}
			tmp = tmp->first_child;
			++curdepth;
			maxdepth = std::max(curdepth, maxdepth);
		}
	}

	template <class T, class tree_node_allocator>
	unsigned int tree<T, tree_node_allocator>::number_of_children(const iterator_base& it)
	{
		tree_node* pos = it.node->first_child;
		if (pos == 0) return 0;

		unsigned int ret = 1;
		//	  while(pos!=it.node->last_child) {
		//		  ++ret;
		//		  pos=pos->next_sibling;
		//		  }
		while ((pos = pos->next_sibling))
			++ret;
		return ret;
	}

	template <class T, class tree_node_allocator>
	unsigned int tree<T, tree_node_allocator>::number_of_siblings(const iterator_base& it) const
	{
		tree_node* pos = it.node;
		unsigned int ret = 0;
		// count forward
		while (pos->next_sibling &&
			pos->next_sibling != head &&
			pos->next_sibling != feet) {
			++ret;
			pos = pos->next_sibling;
		}
		// count backward
		pos = it.node;
		while (pos->prev_sibling &&
			pos->prev_sibling != head &&
			pos->prev_sibling != feet) {
			++ret;
			pos = pos->prev_sibling;
		}

		return ret;
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::swap(sibling_iterator it)
	{
		tree_node* nxt = it.node->next_sibling;
		if (nxt) {
			if (it.node->prev_sibling)
				it.node->prev_sibling->next_sibling = nxt;
			else
				it.node->parent->first_child = nxt;
			nxt->prev_sibling = it.node->prev_sibling;
			tree_node* nxtnxt = nxt->next_sibling;
			if (nxtnxt)
				nxtnxt->prev_sibling = it.node;
			else
				it.node->parent->last_child = it.node;
			nxt->next_sibling = it.node;
			it.node->prev_sibling = nxt;
			it.node->next_sibling = nxtnxt;
		}
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::swap(iterator one, iterator two)
	{
		// if one and two are adjacent siblings, use the sibling swap
		if (one.node->next_sibling == two.node) swap(one);
		else if (two.node->next_sibling == one.node) swap(two);
		else {
			tree_node* nxt1 = one.node->next_sibling;
			tree_node* nxt2 = two.node->next_sibling;
			tree_node* pre1 = one.node->prev_sibling;
			tree_node* pre2 = two.node->prev_sibling;
			tree_node* par1 = one.node->parent;
			tree_node* par2 = two.node->parent;

			// reconnect
			one.node->parent = par2;
			one.node->next_sibling = nxt2;
			if (nxt2) nxt2->prev_sibling = one.node;
			else     par2->last_child = one.node;
			one.node->prev_sibling = pre2;
			if (pre2) pre2->next_sibling = one.node;
			else     par2->first_child = one.node;

			two.node->parent = par1;
			two.node->next_sibling = nxt1;
			if (nxt1) nxt1->prev_sibling = two.node;
			else     par1->last_child = two.node;
			two.node->prev_sibling = pre1;
			if (pre1) pre1->next_sibling = two.node;
			else     par1->first_child = two.node;
		}
	}

	// template <class BinaryPredicate>
	// tree<T, tree_node_allocator>::iterator tree<T, tree_node_allocator>::find_subtree(
	// 	sibling_iterator subfrom, sibling_iterator subto, iterator from, iterator to, 
	// 	BinaryPredicate fun) const
	// 	{
	// 	assert(1==0); // this routine is not finished yet.
	// 	while(from!=to) {
	// 		if(fun(*subfrom, *from)) {
	// 			
	// 			}
	// 		}
	// 	return to;
	// 	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::is_in_subtree(const iterator_base& it, const iterator_base& begin,
		const iterator_base& end) const
	{
		// FIXME: this should be optimised.
		pre_order_iterator tmp = begin;
		while (tmp != end) {
			if (tmp == it) return true;
			++tmp;
		}
		return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::is_valid(const iterator_base& it) const
	{
		if (it.node == 0 || it.node == feet || it.node == head) return false;
		else return true;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::iterator tree<T, tree_node_allocator>::lowest_common_ancestor(
		const iterator_base& one, const iterator_base& two) const
	{
		std::set<iterator, iterator_base_less> parents;

		// Walk up from 'one' storing all parents.
		iterator walk = one;
		do {
			walk = parent(walk);
			parents.insert(walk);
		} while (is_valid(parent(walk)));

		// Walk up from 'two' until we encounter a node in parents.
		walk = two;
		do {
			walk = parent(walk);
			if (parents.find(walk) != parents.end()) break;
		} while (is_valid(parent(walk)));

		return walk;
	}

	template <class T, class tree_node_allocator>
	unsigned int tree<T, tree_node_allocator>::index(sibling_iterator it) const
	{
		unsigned int ind = 0;
		if (it.node->parent == 0) {
			while (it.node->prev_sibling != head) {
				it.node = it.node->prev_sibling;
				++ind;
			}
		}
		else {
			while (it.node->prev_sibling != 0) {
				it.node = it.node->prev_sibling;
				++ind;
			}
		}
		return ind;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::sibling(const iterator_base& it, unsigned int num)
	{
		tree_node* tmp;
		if (it.node->parent == 0) {
			tmp = head->next_sibling;
			while (num) {
				tmp = tmp->next_sibling;
				--num;
			}
		}
		else {
			tmp = it.node->parent->first_child;
			while (num) {
				assert(tmp != 0);
				tmp = tmp->next_sibling;
				--num;
			}
		}
		return tmp;
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::debug_verify_consistency() const
	{
		iterator it = begin();
		while (it != end()) {
			if (it.node->parent != 0) {
				if (it.node->prev_sibling == 0)
					assert(it.node->parent->first_child == it.node);
				else
					assert(it.node->prev_sibling->next_sibling == it.node);
				if (it.node->next_sibling == 0)
					assert(it.node->parent->last_child == it.node);
				else
					assert(it.node->next_sibling->prev_sibling == it.node);
			}
			++it;
		}
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::child(const iterator_base& it, unsigned int num)
	{
		tree_node* tmp = it.node->first_child;
		while (num--) {
			assert(tmp != 0);
			tmp = tmp->next_sibling;
		}
		return tmp;
	}




	// Iterator base

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::iterator_base::iterator_base()
		: node(0), skip_current_children_(false)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::iterator_base::iterator_base(tree_node* tn)
		: node(tn), skip_current_children_(false)
	{
	}

	template <class T, class tree_node_allocator>
	T& tree<T, tree_node_allocator>::iterator_base::operator*() const
	{
		return node->data;
	}

	template <class T, class tree_node_allocator>
	T* tree<T, tree_node_allocator>::iterator_base::operator->() const
	{
		return &(node->data);
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::post_order_iterator::operator!=(const post_order_iterator& other) const
	{
		if (other.node != this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::post_order_iterator::operator==(const post_order_iterator& other) const
	{
		if (other.node == this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::pre_order_iterator::operator!=(const pre_order_iterator& other) const
	{
		if (other.node != this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::pre_order_iterator::operator==(const pre_order_iterator& other) const
	{
		if (other.node == this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::sibling_iterator::operator!=(const sibling_iterator& other) const
	{
		if (other.node != this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::sibling_iterator::operator==(const sibling_iterator& other) const
	{
		if (other.node == this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::leaf_iterator::operator!=(const leaf_iterator& other) const
	{
		if (other.node != this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::leaf_iterator::operator==(const leaf_iterator& other) const
	{
		if (other.node == this->node && other.top_node == this->top_node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::iterator_base::begin() const
	{
		if (node->first_child == 0)
			return end();

		sibling_iterator ret(node->first_child);
		ret.parent_ = this->node;
		return ret;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::iterator_base::end() const
	{
		sibling_iterator ret(0);
		ret.parent_ = node;
		return ret;
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::iterator_base::skip_children()
	{
		skip_current_children_ = true;
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::iterator_base::skip_children(bool skip)
	{
		skip_current_children_ = skip;
	}

	template <class T, class tree_node_allocator>
	unsigned int tree<T, tree_node_allocator>::iterator_base::number_of_children() const
	{
		tree_node* pos = node->first_child;
		if (pos == 0) return 0;

		unsigned int ret = 1;
		while (pos != node->last_child) {
			++ret;
			pos = pos->next_sibling;
		}
		return ret;
	}



	// Pre-order iterator

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator()
		: iterator_base(0)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator(tree_node* tn)
		: iterator_base(tn)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator(const iterator_base& other)
		: iterator_base(other.node)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::pre_order_iterator::pre_order_iterator(const sibling_iterator& other)
		: iterator_base(other.node)
	{
		if (this->node == 0) {
			if (other.range_last() != 0)
				this->node = other.range_last();
			else
				this->node = other.parent_;
			this->skip_children();
			++(*this);
		}
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator++()
	{
		assert(this->node != 0);
		if (!this->skip_current_children_ && this->node->first_child != 0) {
			this->node = this->node->first_child;
		}
		else {
			this->skip_current_children_ = false;
			while (this->node->next_sibling == 0) {
				this->node = this->node->parent;
				if (this->node == 0)
					return *this;
			}
			this->node = this->node->next_sibling;
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator--()
	{
		assert(this->node != 0);
		if (this->node->prev_sibling) {
			this->node = this->node->prev_sibling;
			while (this->node->last_child)
				this->node = this->node->last_child;
		}
		else {
			this->node = this->node->parent;
			if (this->node == 0)
				return *this;
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::pre_order_iterator::operator++(int)
	{
		pre_order_iterator copy = *this;
		++(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::next_skip_children()
	{
		(*this).skip_children();
		(*this)++;
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator tree<T, tree_node_allocator>::pre_order_iterator::operator--(int)
	{
		pre_order_iterator copy = *this;
		--(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator+=(unsigned int num)
	{
		while (num > 0) {
			++(*this);
			--num;
		}
		return (*this);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::pre_order_iterator& tree<T, tree_node_allocator>::pre_order_iterator::operator-=(unsigned int num)
	{
		while (num > 0) {
			--(*this);
			--num;
		}
		return (*this);
	}



	// Post-order iterator

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator()
		: iterator_base(0)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator(tree_node* tn)
		: iterator_base(tn)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator(const iterator_base& other)
		: iterator_base(other.node)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::post_order_iterator::post_order_iterator(const sibling_iterator& other)
		: iterator_base(other.node)
	{
		if (this->node == 0) {
			if (other.range_last() != 0)
				this->node = other.range_last();
			else
				this->node = other.parent_;
			this->skip_children();
			++(*this);
		}
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator++()
	{
		assert(this->node != 0);
		if (this->node->next_sibling == 0) {
			this->node = this->node->parent;
			this->skip_current_children_ = false;
		}
		else {
			this->node = this->node->next_sibling;
			if (this->skip_current_children_) {
				this->skip_current_children_ = false;
			}
			else {
				while (this->node->first_child)
					this->node = this->node->first_child;
			}
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator--()
	{
		assert(this->node != 0);
		if (this->skip_current_children_ || this->node->last_child == 0) {
			this->skip_current_children_ = false;
			while (this->node->prev_sibling == 0)
				this->node = this->node->parent;
			this->node = this->node->prev_sibling;
		}
		else {
			this->node = this->node->last_child;
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::post_order_iterator::operator++(int)
	{
		post_order_iterator copy = *this;
		++(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator tree<T, tree_node_allocator>::post_order_iterator::operator--(int)
	{
		post_order_iterator copy = *this;
		--(*this);
		return copy;
	}


	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator+=(unsigned int num)
	{
		while (num > 0) {
			++(*this);
			--num;
		}
		return (*this);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::post_order_iterator& tree<T, tree_node_allocator>::post_order_iterator::operator-=(unsigned int num)
	{
		while (num > 0) {
			--(*this);
			--num;
		}
		return (*this);
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::post_order_iterator::descend_all()
	{
		assert(this->node != 0);
		while (this->node->first_child)
			this->node = this->node->first_child;
	}


	// Breadth-first iterator

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::breadth_first_queued_iterator::breadth_first_queued_iterator()
		: iterator_base()
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::breadth_first_queued_iterator::breadth_first_queued_iterator(tree_node* tn)
		: iterator_base(tn)
	{
		traversal_queue.push(tn);
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::breadth_first_queued_iterator::breadth_first_queued_iterator(const iterator_base& other)
		: iterator_base(other.node)
	{
		traversal_queue.push(other.node);
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator!=(const breadth_first_queued_iterator& other) const
	{
		if (other.node != this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator==(const breadth_first_queued_iterator& other) const
	{
		if (other.node == this->node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::breadth_first_queued_iterator& tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator++()
	{
		assert(this->node != 0);

		// Add child nodes and pop current node
		sibling_iterator sib = this->begin();
		while (sib != this->end()) {
			traversal_queue.push(sib.node);
			++sib;
		}
		traversal_queue.pop();
		if (traversal_queue.size() > 0)
			this->node = traversal_queue.front();
		else
			this->node = 0;
		return (*this);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::breadth_first_queued_iterator tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator++(int)
	{
		breadth_first_queued_iterator copy = *this;
		++(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::breadth_first_queued_iterator& tree<T, tree_node_allocator>::breadth_first_queued_iterator::operator+=(unsigned int num)
	{
		while (num > 0) {
			++(*this);
			--num;
		}
		return (*this);
	}



	// Fixed depth iterator

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator()
		: iterator_base()
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(tree_node* tn)
		: iterator_base(tn), top_node(0)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(const iterator_base& other)
		: iterator_base(other.node), top_node(0)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(const sibling_iterator& other)
		: iterator_base(other.node), top_node(0)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::fixed_depth_iterator::fixed_depth_iterator(const fixed_depth_iterator& other)
		: iterator_base(other.node), top_node(other.top_node)
	{
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::fixed_depth_iterator::operator==(const fixed_depth_iterator& other) const
	{
		if (other.node == this->node && other.top_node == top_node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	bool tree<T, tree_node_allocator>::fixed_depth_iterator::operator!=(const fixed_depth_iterator& other) const
	{
		if (other.node != this->node || other.top_node != top_node) return true;
		else return false;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator++()
	{
		assert(this->node != 0);

		if (this->node->next_sibling) {
			this->node = this->node->next_sibling;
		}
		else {
			int relative_depth = 0;
		upper:
			do {
				if (this->node == this->top_node) {
					this->node = 0; // FIXME: return a proper fixed_depth end iterator once implemented
					return *this;
				}
				this->node = this->node->parent;
				if (this->node == 0) return *this;
				--relative_depth;
			} while (this->node->next_sibling == 0);
		lower:
			this->node = this->node->next_sibling;
			while (this->node->first_child == 0) {
				if (this->node->next_sibling == 0)
					goto upper;
				this->node = this->node->next_sibling;
				if (this->node == 0) return *this;
			}
			while (relative_depth < 0 && this->node->first_child != 0) {
				this->node = this->node->first_child;
				++relative_depth;
			}
			if (relative_depth < 0) {
				if (this->node->next_sibling == 0) goto upper;
				else                          goto lower;
			}
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator--()
	{
		assert(this->node != 0);

		if (this->node->prev_sibling) {
			this->node = this->node->prev_sibling;
		}
		else {
			int relative_depth = 0;
		upper:
			do {
				if (this->node == this->top_node) {
					this->node = 0;
					return *this;
				}
				this->node = this->node->parent;
				if (this->node == 0) return *this;
				--relative_depth;
			} while (this->node->prev_sibling == 0);
		lower:
			this->node = this->node->prev_sibling;
			while (this->node->last_child == 0) {
				if (this->node->prev_sibling == 0)
					goto upper;
				this->node = this->node->prev_sibling;
				if (this->node == 0) return *this;
			}
			while (relative_depth < 0 && this->node->last_child != 0) {
				this->node = this->node->last_child;
				++relative_depth;
			}
			if (relative_depth < 0) {
				if (this->node->prev_sibling == 0) goto upper;
				else                            goto lower;
			}
		}
		return *this;

		//
		//
		//	assert(this->node!=0);
		//	if(this->node->prev_sibling!=0) {
		//		this->node=this->node->prev_sibling;
		//		assert(this->node!=0);
		//		if(this->node->parent==0 && this->node->prev_sibling==0) // head element
		//			this->node=0;
		//		}
		//	else {
		//		tree_node *par=this->node->parent;
		//		do {
		//			par=par->prev_sibling;
		//			if(par==0) { // FIXME: need to keep track of this!
		//				this->node=0;
		//				return *this;
		//				}
		//			} while(par->last_child==0);
		//		this->node=par->last_child;
		//		}
		//	return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::fixed_depth_iterator::operator++(int)
	{
		fixed_depth_iterator copy = *this;
		++(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator tree<T, tree_node_allocator>::fixed_depth_iterator::operator--(int)
	{
		fixed_depth_iterator copy = *this;
		--(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator-=(unsigned int num)
	{
		while (num > 0) {
			--(*this);
			--(num);
		}
		return (*this);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::fixed_depth_iterator& tree<T, tree_node_allocator>::fixed_depth_iterator::operator+=(unsigned int num)
	{
		while (num > 0) {
			++(*this);
			--(num);
		}
		return *this;
	}


	// Sibling iterator

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator()
		: iterator_base()
	{
		set_parent_();
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator(tree_node* tn)
		: iterator_base(tn)
	{
		set_parent_();
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator(const iterator_base& other)
		: iterator_base(other.node)
	{
		set_parent_();
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::sibling_iterator::sibling_iterator(const sibling_iterator& other)
		: iterator_base(other), parent_(other.parent_)
	{
	}

	template <class T, class tree_node_allocator>
	void tree<T, tree_node_allocator>::sibling_iterator::set_parent_()
	{
		parent_ = 0;
		if (this->node == 0) return;
		if (this->node->parent != 0)
			parent_ = this->node->parent;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator++()
	{
		if (this->node)
			this->node = this->node->next_sibling;
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator--()
	{
		if (this->node) this->node = this->node->prev_sibling;
		else {
			assert(parent_);
			this->node = parent_->last_child;
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::sibling_iterator::operator++(int)
	{
		sibling_iterator copy = *this;
		++(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator tree<T, tree_node_allocator>::sibling_iterator::operator--(int)
	{
		sibling_iterator copy = *this;
		--(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator+=(unsigned int num)
	{
		while (num > 0) {
			++(*this);
			--num;
		}
		return (*this);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::sibling_iterator& tree<T, tree_node_allocator>::sibling_iterator::operator-=(unsigned int num)
	{
		while (num > 0) {
			--(*this);
			--num;
		}
		return (*this);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::tree_node* tree<T, tree_node_allocator>::sibling_iterator::range_first() const
	{
		tree_node* tmp = parent_->first_child;
		return tmp;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::tree_node* tree<T, tree_node_allocator>::sibling_iterator::range_last() const
	{
		return parent_->last_child;
	}

	// Leaf iterator

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator()
		: iterator_base(0), top_node(0)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator(tree_node* tn, tree_node* top)
		: iterator_base(tn), top_node(top)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator(const iterator_base& other)
		: iterator_base(other.node), top_node(0)
	{
	}

	template <class T, class tree_node_allocator>
	tree<T, tree_node_allocator>::leaf_iterator::leaf_iterator(const sibling_iterator& other)
		: iterator_base(other.node), top_node(0)
	{
		if (this->node == 0) {
			if (other.range_last() != 0)
				this->node = other.range_last();
			else
				this->node = other.parent_;
			++(*this);
		}
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator++()
	{
		assert(this->node != 0);
		if (this->node->first_child != 0) { // current node is no longer leaf (children got added)
			while (this->node->first_child)
				this->node = this->node->first_child;
		}
		else {
			while (this->node->next_sibling == 0) {
				if (this->node->parent == 0) return *this;
				this->node = this->node->parent;
				if (top_node != 0 && this->node == top_node) return *this;
			}
			this->node = this->node->next_sibling;
			while (this->node->first_child)
				this->node = this->node->first_child;
		}
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator--()
	{
		assert(this->node != 0);
		while (this->node->prev_sibling == 0) {
			if (this->node->parent == 0) return *this;
			this->node = this->node->parent;
			if (top_node != 0 && this->node == top_node) return *this;
		}
		this->node = this->node->prev_sibling;
		while (this->node->last_child)
			this->node = this->node->last_child;
		return *this;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::leaf_iterator::operator++(int)
	{
		leaf_iterator copy = *this;
		++(*this);
		return copy;
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator tree<T, tree_node_allocator>::leaf_iterator::operator--(int)
	{
		leaf_iterator copy = *this;
		--(*this);
		return copy;
	}


	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator+=(unsigned int num)
	{
		while (num > 0) {
			++(*this);
			--num;
		}
		return (*this);
	}

	template <class T, class tree_node_allocator>
	typename tree<T, tree_node_allocator>::leaf_iterator& tree<T, tree_node_allocator>::leaf_iterator::operator-=(unsigned int num)
	{
		while (num > 0) {
			--(*this);
			--num;
		}
		return (*this);
	}
}

namespace cweeEng {
	using namespace cwee_units;

	/*!
	Assumes head on y-axis and flow on x-axis.
	*/
	static foot_t SamplePumpHeadCurve(Curve const& curve, gallon_per_minute_t flow) {
		switch (curve.GetNumValues()) {
		case 0: return foot_t(flow() / 5.0f);		
		case 1: {
			Curve newCurve = curve;
			float designHead = newCurve.GetValue(0);
			float designFlow = newCurve.GetTime(0);

			newCurve.AddUniqueValue(0, designHead * 1.33f); // epanet approach
			newCurve.AddUniqueValue(designFlow * 2.0f, 0); // epanet approach

			return newCurve.GetCurrentValue(flow()); // catmull_rom spline sample
		}
		case 2: {
			Curve newCurve = curve;

			float designHead = cweeMath::Faverage({ newCurve.GetValue(0), newCurve.GetValue(1) });
			float designFlow = cweeMath::Faverage({ (float)newCurve.GetTime(0), (float)newCurve.GetTime(1) });
			float maxFlow = cweeMath::Fmax(designFlow * 2.0f, 1.5f * cweeMath::Fmax((float)newCurve.GetTime(0), (float)newCurve.GetTime(1)));
			float maxHead = cweeMath::Fmax(designHead * 1.33f, 1.15f * cweeMath::Fmax((float)newCurve.GetValue(0), (float)newCurve.GetValue(1)));

			newCurve.AddUniqueValue(0, maxHead);
			newCurve.AddUniqueValue(maxFlow, 0);

			return newCurve.GetCurrentValue(flow()); // catmull_rom spline sample
		}
		default: return curve.GetCurrentValue(flow()); // catmull_rom spline sample		
		}
	}

	static constexpr AUTO FlowTimesHead(gallon_per_minute_t Flow, foot_t Head) {
		return Flow * Head;
	};

	static constexpr kilowatt_t CentrifugalPumpEnergyDemand_kW(gallon_per_minute_t Flow_gpm, foot_t Head_feet, scalar_t Efficiency_percent) {
		return FlowTimesHead(Flow_gpm, Head_feet) * units::constants::gd * 100.0 / Efficiency_percent;


		// return ((cubic_meter_per_hour_t)Flow_gpm) * units::constants::d * units::constants::g * ((meter_t)Head_feet) / (Efficiency_percent / 100.0);
		//return Flow_gpm * units::constants::d * units::constants::g * Head_feet / (Efficiency_percent / 100.0);
	};
	static constexpr square_foot_t SurfaceAreaCircle_ft2(foot_t Diameter_feet) {
		return units::math::cpow<2>(Diameter_feet) * (scalar_t)cweeMath::PI * (scalar_t)0.25;
	};
	static constexpr gallon_t VolumeCylinder_gal(foot_t Diameter_feet, foot_t Height_feet) {
		return SurfaceAreaCircle_ft2(Diameter_feet) * Height_feet; // converts from cubic_foot_t to gallon_t auto-magically. 
	};
	static constexpr feet_per_hour_t Cylinder_FlowRate_to_LevelRate_fph(gallon_per_minute_t FlowRate_gpm, foot_t Diameter_feet) {
		return cubic_foot_per_hour_t(FlowRate_gpm) / SurfaceAreaCircle_ft2(Diameter_feet);
	};
	static constexpr foot_t Cylinder_Volume_to_Level_f(cubic_foot_t volume, foot_t Diameter_feet) {
		return volume / SurfaceAreaCircle_ft2(Diameter_feet);
	};
	static constexpr pounds_per_square_inch_t Head_to_Pressure_psi(foot_t HydraulicHead_feet, foot_t BaseElevation_feet) {
		return (HydraulicHead_feet - BaseElevation_feet)() * 0.4329004329f;
	};
	static vec3 GetMadConversion(const measurement_t& from, const measurement_t& to) {
		return cweeUnits::GetMadConversion(from, to);
	};

	static vec3 RotatePositionXY(const vec3& start, float clockwiseRotationDegrees, const vec3& rotateAround = vec3(0, 0, 0)) {
		vec3 out;
		float angle = (360.0f - clockwiseRotationDegrees) * (cweeMath::PI / 180.0f);
		float x = cosf(angle);
		float y = sinf(angle);
		out.x = (start.x - rotateAround.x) * x - (start.y - rotateAround.y) * y + rotateAround.x;
		out.y = (start.y - rotateAround.y) * x + (start.x - rotateAround.x) * y + rotateAround.y;
		out.z = start.z;
		return out;
	};
	static vec3d RotatePositionXY(const vec3d& start, float clockwiseRotationDegrees, const vec3d& rotateAround = vec3d(0, 0, 0)) {
		vec3d out;
		double angle = (360.0f - clockwiseRotationDegrees) * (cweeMath::PI / 180.0f);
		double x = cosf(angle);
		double y = sinf(angle);
		out.x = (start.x - rotateAround.x) * x - (start.y - rotateAround.y) * y + rotateAround.x;
		out.y = (start.y - rotateAround.y) * x + (start.x - rotateAround.x) * y + rotateAround.y;
		out.z = start.z;
		return out;
	};

	// true only for the DW equation
	static inch_t EquivalentPipeDiameter_ConstantFlow(foot_t pipe1_length, foot_t pipe2_length, inch_t pipe1_diameter, inch_t pipe2_diameter) {
		// from: https://pdf.sciencedirectassets.com/278653/1-s2.0-S1877705817X00179/1-s2.0-S1877705817313991/main.pdf?X-Amz-Security-Token=IQoJb3JpZ2luX2VjEPD%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaCXVzLWVhc3QtMSJGMEQCIBvCRSY0I3te7nlj1iryyTb7MJTMPlVrOssmSDmmxV56AiAOJzRyuDwv%2BcxvCkzzBTbcYVnOlp9QuUNZXTAJtEbR9iq9AwjJ%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F8BEAMaDDA1OTAwMzU0Njg2NSIMFIToVymz2aRmF6wtKpEDd85poqyoqiweTRIQd8GtOo8E9%2B%2BWPFX4UGDtF%2BCf3CvpqKTvXU0urOQZ65lYpDzzBYxIKwU2PaQCY7m0iy1blMFKDOS7GfGqB2RcmXVnl2yXpzI%2FbFs2DM8jVvjkwSIJxf3aNcQQJJ5zKMbU3phJkvg55wxYdXAx5FpH6jOfJO02Uq98AE6S0cWS2mIgLlJ%2BGCpDfPXhadC1ZD8BuWQb8Eva26Ps68HXE30%2BXlVnWfhMqvccsYQtWB6Ch7K9wQ3LYrE7Xwts67%2B%2B42zO%2BZ419T0%2FDW7cRUTqGCe17Ur1EaAYCUmXXTZNNGmX7g7hOvms6eDeO%2F8WkllFbotUW0voTLq%2FERQsfZpDVXLSTuOY%2FqOGlIiqvM3VQ6XVpX3K4WxthnRzKQuhsuKn6eU0rhbSsUsrUNximKHemJV5Ve4h%2FChM8q3BvQWMHN2A84EAKzPJ2ukkwuAHYD6PrtwDPiFQqY1m1T43%2BHlc2PSgxWtjm1jnUGpw1wMh0oMYpT369NPfk8unN3QLIij8ZMCtpAVObjww2s2tgAY67AE7DyXRJTBae%2F92UFkxxo7R8PPoP0RsSrYHHpp5zxYei0NOlWNgvZN%2FK%2FTfxzBycZuSvkLmhNm7FIp4c8Khddt3YSbmk1%2BN01vlAI7TymZmGn%2Ff%2Fq8tUyW6hJsdjivJXCW%2BGDeJ8msALEthedb3a0L7Ziic%2Bb6YrmbmvbusHF0LutJ6EWiMa%2B48RK5NkdehNzqWj9MmmkfmTk93RYP80Ll4R6srvlhixx%2Bl9KHxCJPZz8vSki0LFu86P0QwfPfYdcqHyWS0dDkgs2j3CQeCHBtxwtv3y3C8qC4gDEILsS2iPGECN3jFtL4GSWkIFA%3D%3D&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20210123T014359Z&X-Amz-SignedHeaders=host&X-Amz-Expires=300&X-Amz-Credential=ASIAQ3PHCVTYSWFXPUFL%2F20210123%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Signature=ebf9629e15e043d99b422b093bfef1cc1af095fe76298a81bc4ce33b8c812e2a&hash=16c0bc952b2da6894978af4488940ed24c84691d41e1de87abd2012b16027cfa&host=68042c943591013ac2b2430a89b270f6af2c76d8dfd086a07176afe7c76c2c61&pii=S1877705817313991&tid=spdf-24b18965-1fee-4f69-bcb5-d2f705f42550&sid=6e4529fc8bf8934ce51b64a3dcc0320f0b1dgxrqa&type=client
		return ::pow((pipe1_length + pipe2_length)(), 0.2) / ::pow(((pipe1_length() / ::pow(pipe1_diameter(), 5.0)) + (pipe2_length() / ::pow(pipe2_diameter(), 5.0))), 0.2 );
	};

	static double EquivalentPipeRoughness(inch_t desiredDiameter, foot_t pipe1_length, foot_t pipe2_length, inch_t pipe1_diameter, inch_t pipe2_diameter, const double& pipe1_roughness, const double& pipe2_roughness) {
		// From: https://ecommons.udayton.edu/cgi/viewcontent.cgi?article=1013&context=cee_fac_pub
		// Eq. 3.4

		AUTO L_r = pipe1_length + pipe2_length;
		AUTO D_r = desiredDiameter; // cweeMath::Faverage({ pipe1_diameter, pipe2_diameter });

		if (pipe1_diameter == pipe2_diameter && pipe1_roughness == pipe2_roughness) return pipe1_roughness;

		double C_r =
			::pow(L_r() / (::pow(D_r(), 4.87)), 0.54)
			*
			::pow( (pipe1_length() / ((::pow(pipe1_diameter(), 4.87)) * (::pow(pipe1_roughness, 1.85)))) + (pipe2_length() / ((::pow(pipe2_diameter(), 4.87)) * (::pow(pipe2_roughness, 1.85)))) , -0.54);

		return C_r;
	};

	static constexpr float ReynoldsNumberInPipe(feet_per_second_t velocity_ftPerSec, inch_t diameter_inches, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return (velocity_ftPerSec * foot_t(diameter_inches))() / (kinematicViscosity_ft2PerSec == 0.0f ? 0.000001004 : kinematicViscosity_ft2PerSec);
	};

	static constexpr fahrenheit_t CelsiusToFahrenheit(celsius_t C) {
		return C; // return (C * 9.0f / 5.0f) + 32.0f;
	};
	static constexpr celsius_t FahrenheitToCelsius(fahrenheit_t F) {
		return F; // return (F - 32.0f) * 5.0f / 9.0f;
	};

	/*!
	Approximation based on regression.
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static double PipeRoughnessConversion_HW_to_DW(double HW_roughness) {
		return
			cweeMath::Fmin(1.5f,
				cweeMath::Fmax(0.028f,
					(float)((-0.0000028183185026) * (::pow(HW_roughness, 4.0))
					+ (0.0014243470892602) * (::pow(HW_roughness, 3.0))
					- (0.267451030006944) * (::pow(HW_roughness, 2.0))
					+ (22.072188172073) * HW_roughness
					- 673.672526526534)
				)
			);
	};
	/*!
	Approximation based on regression.
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static double PipeRoughnessConversion_DW_to_HW(double DW_roughness) {
		return
			cweeMath::Fmin(150.0f,
				cweeMath::Fmax(100.0f,
					(float)((-28.845364406152) * (::pow(DW_roughness, 4.0))
					+ (34.250225016229) * (::pow(DW_roughness, 3.0))
					- (62.591795817284) * (::pow(DW_roughness, 2.0))
					+ (103.276842824486) * DW_roughness
					- 149.373579659483)
				)
			);
	};

	/*!
	Approximation based on regression.
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static double PipeFrictionFactor_HW_to_DWFF(double HW_roughness, double ReynoldsNumber, inch_t diameter_inches) {
		return
			1016.610
			* ::pow(HW_roughness, -1.85)
			* ::pow(ReynoldsNumber, -0.148)
			* ::pow(diameter_inches(), -0.0158);
	};
	static double PipeFrictionFactor_DW_to_HWFF(double DW_roughness, double ReynoldsNumber, inch_t diameter_inches) {
		if (ReynoldsNumber < 2000) {
			return 64.0 / ReynoldsNumber;
		}
		else if (ReynoldsNumber > 4000) {
			return 0.25 / ::pow(::log( (DW_roughness / (3.7 * foot_t(diameter_inches)())) + (5.74 / ::pow(ReynoldsNumber, 0.9))), 2.0);
		}
		else {
			double X1, R, X2, X3, X4, FA, FB, Y2, Y3;
			R = ReynoldsNumber / 2000.0;
			Y2 = (DW_roughness / (3.7 * foot_t(diameter_inches)())) + (5.74 / (::pow(ReynoldsNumber, 0.9)));
			Y3 = -0.86859 * ::log(
				(DW_roughness / (3.7 * foot_t(diameter_inches)()))
				+ (5.74 / (::pow(4000.0, 0.9)))
			);
			FA = ::pow(Y3, -2.0);
			FB = FA * (
				2.0
				- (0.00514215 / (Y2 * Y3))
				);

			X1 = 7 * FA - FB;
			X2 = 0.128 - 17.0 * FA + 2.5 * FB;
			X3 = -0.128 + 13.0 * FA - 2.0 * FB;
			X4 = R * (0.032 - 3.0 * FA + 0.5 * FB);

			return (X1) + R * ((X2) + R * ((X3) + (X4)));
		}
	};
	static float PipeResistanceCoefficient_HW(float HW_roughness, inch_t diameter_inches, foot_t length_feet, feet_per_second_t velocity_ftPerSes, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return
			4.727
			* PipeFrictionFactor_HW_to_DWFF(HW_roughness, ReynoldsNumberInPipe(velocity_ftPerSes, diameter_inches, kinematicViscosity_ft2PerSec), diameter_inches)
			* ::pow(foot_t(diameter_inches)(), -4.871)
			* length_feet();
	};
	static float PipeResistanceCoefficient_HW_to_DW(float HW_roughness, inch_t diameter_inches, foot_t length_feet) {
		return
			0.0252
			* ::pow(HW_roughness, -1.852)
			* ::pow(foot_t(diameter_inches)(), -5)
			* length_feet();
	};
	static float PipeResistanceCoefficient_DW_to_HW(float DW_roughness, inch_t diameter_inches, foot_t length_feet, feet_per_second_t velocity_ftPerSes, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return
			0.0252
			* PipeFrictionFactor_DW_to_HWFF(DW_roughness, ReynoldsNumberInPipe(velocity_ftPerSes, diameter_inches, kinematicViscosity_ft2PerSec), diameter_inches)
			* ::pow(foot_t(diameter_inches)(), -5)
			* length_feet();
	};

	static float PipeHeadloss_HW(float resistanceCoefficient, cubic_foot_per_second_t flowrate_cfs) {
		return resistanceCoefficient * ::pow(flowrate_cfs(), 1.852);
	};
	static float PipeHeadloss_DW(float resistanceCoefficient, cubic_foot_per_second_t flowrate_cfs) {
		return resistanceCoefficient * ::pow(flowrate_cfs(), 2.0);
	};
	static foot_t Approximate_EquivalentPipeDiameter(float FrictionFactor, foot_t length_feet, foot_t totalHeadloss_feet, cubic_foot_per_second_t totalPipeFlow_cfs) {
		return ::pow(
			(8.0 * FrictionFactor * length_feet())
			* (totalPipeFlow_cfs() * totalPipeFlow_cfs())
			/ (cweeMath::PI * cweeMath::PI * cweeMath::G * totalHeadloss_feet())
			, 0.2);
	};
	static inch_t Find_EquivalentPipeDiameter_DW(float DW_roughness, feet_per_second_t velocity_ftPerSes, foot_t length_feet, foot_t totalHeadloss_feet, cubic_foot_per_second_t totalPipeFlow_cfs) {
		constexpr float eps = 0.001f;
		float err = 1.0f;
		int trials = 1000;
		// assume Deq
		inch_t Deq = 8.0f; // 8 inches
		inch_t Deq_Prime = 8.0f; // 8 inches
		while (cweeMath::Fabs(err * err) > eps && trials >= 0) {
			float Re = ReynoldsNumberInPipe(velocity_ftPerSes, Deq);
			float ff = PipeFrictionFactor_DW_to_HWFF(DW_roughness, Re, Deq);

			Deq_Prime = Approximate_EquivalentPipeDiameter(ff, length_feet, totalHeadloss_feet, totalPipeFlow_cfs);

			err = (Deq_Prime - Deq)();
			Deq = Deq_Prime;

			trials--;
		}
		return Deq;
	};

	static inch_t Find_EquivalentPipeDiameter_HW(float HW_roughness, feet_per_second_t velocity_ftPerSes, foot_t length_feet, foot_t totalHeadloss_feet, cubic_foot_per_second_t totalPipeFlow_cfs) {
		constexpr float eps = 0.001f;
		float err = 1.0f;
		int trials = 1000;
		// assume Deq
		inch_t Deq = 8.0f; // 8 inches
		inch_t Deq_Prime = 8.0f; // 8 inches
		while (cweeMath::Fabs(err * err) > eps && trials >= 0) {
			float Re = ReynoldsNumberInPipe(velocity_ftPerSes, Deq);
			float ff = PipeFrictionFactor_HW_to_DWFF(HW_roughness, Re, Deq);

			Deq_Prime = Approximate_EquivalentPipeDiameter(ff, length_feet, totalHeadloss_feet, totalPipeFlow_cfs);

			err = (Deq_Prime - Deq)();
			Deq = Deq_Prime;

			trials--;
		}

		return Deq;

	};

	/*	// implimentation of "https://www.iogp.org/wp-content/uploads/2019/09/373-07-02.pdf" Lambert Conic Conformal (2SP)
		var& inputLongLat = Pair(-96, 28.5);
		var& XY_Feet = Pair(2963503.91_ft, 254759.80_ft);
		double LatFirstStandardParallel = 0.49538262 / AngRad; // GET DEGREES
		double LatSecondStandardParallel = 0.52854388 / AngRad; // GET DEGREES
		double LatOrigin = 0.48578331 / AngRad; // GET DEGREES
		double LongOrigin = -1.72787596 / AngRad; // GET DEGREES
		foot FalseNorthing = 0.0_ft;
		foot FalseEasting = 2000000.00_ft;

		foot a = 20925832.16_ft;                        	// major radius of GRS 1980 ellipsoid, feet
		double Ec = 0.08227185;                  			// eccentricity of GRD 1980 ellipsoid
		double Ec2 = Ec ^ 2.0;                          	// eccentricity squared
		double EcD2 = Ec / 2.0;

		double Pi4 = PI / 4.0;                			// Pi / 4
		double Pi2 = PI / 2.0;							// Pi / 2
		double P0 = LatOrigin * AngRad;   						// latitude of origin
		double P1 = LatFirstStandardParallel * AngRad;			// latitude of first standard parallel
		double P2 = LatSecondStandardParallel * AngRad;		// latitude of second standard parallel
		var& Ef = FalseEasting.double;					// False easting of central meridian, map units
		var& Nf = FalseNorthing.double;					// False northing of central meridian, map units
		var& E = XY_Feet.first.double;
		var& N = XY_Feet.second.double;

		double m1 = cos(P1) / ((1.0 - (Ec2 * ((sin(P1))^2)))^0.5); // good
		double m2 = cos(P2) / ((1.0 - (Ec2 * ((sin(P2))^2)))^0.5); // good
		double t0 = tan(Pi4 - (P0 / 2.0)) / (((1.0 - Ec * sin(P0)) / (1.0 + Ec * sin(P0)))^EcD2);
		double t1 = tan(Pi4 - (P1 / 2.0)) / (((1.0 - Ec * sin(P1)) / (1.0 + Ec * sin(P1)))^EcD2);
		double t2 = tan(Pi4 - (P2 / 2.0)) / (((1.0 - Ec * sin(P2)) / (1.0 + Ec * sin(P2)))^EcD2);
		double n = log(m1 / m2) / log(t1 / t2); // good
		double F = m1 / (n * (t1^n));

		double degree = n*(inputLongLat.first*AngRad - LongOrigin*AngRad); // good
		double t =
			tan(Pi4 - (inputLongLat.second * AngRad / 2.0)) /
			((1.0 - Ec * sin(inputLongLat.second * AngRad)) / (1.0 + Ec * sin(inputLongLat.second * AngRad)))^EcD2; // good

		double r = a*F*t^n
		double r_F = a * F * (t0^n); // good

		return ["Easting":Ef+r*sin(degree), "Northing":Nf+r_F-r*cos(degree)];

		// calc longitude
		double r_prime = ((E - Ef)^2 + (r_F - (N - Nf))^2)^0.5;
		r_prime *= (n >= 0 ? 1.0 : -1.0); // taking the sign of n

		double t_prime = (r_prime / (a * F)) ^ (1.0 / n);

		double degree_prime = atan((E - Ef) / (r_F - (N - Nf)));

		double LonR = ((degree_prime/n)/AngRad + LongOrigin) * AngRad

		// Substitute the estimate into the iterative calculation that converges on the correct Latitude value.
		double LatR = Pi2 - (2.0 * atan(t_prime));
		for (var j = 0; j < 5; ++j) {


			LatR = Pi2 - (2.0 * atan(t_prime * (((1.0 - (Ec * sin(LatR))) / (1.0 + (Ec * sin(LatR))))^EcD2)));
		}
		// return Pair(LonR / AngRad, LatR / AngRad); // Convert from radians to degrees

		return ["Longitude (deg)":LonR/AngRad, "Latitude (deg)":LatR/AngRad]; // Convert from radians to degrees	
	*/

	static vec2d CoordinateConversion_FeetToLongLat(vec2d XY_Feet,
		double centralMeridian = -120.5000000000000000,
		double LatFirstStandardParallel = 37.0666666666666666667,
		double LatSecondStandardParallel = 38.433333333333333333,
		double LatOrigin = 36.500000000000000000,
		double FalseNorthing = 1640416.6666666666666,
		double FalseEasting = 6561666.6666666666666) {

		constexpr double A = 20925604.47;                        	// major radius of GRS 1980 ellipsoid, feet
		constexpr double Ec = 0.0818191910435;                  	// eccentricity of GRD 1980 ellipsoid
		constexpr double Ec2 = Ec * Ec;                          	// eccentricity squared
		constexpr double EcD2 = Ec / 2.0;

		double Pi4, M0, P1, P2, P0, X0, Y0, m1, m2, t1, t2, t0, n, F, rho0, x, y, Pi2, rho, t, LonR, LatR;
		int j;

		Pi4 = cweeMath::PI / 4.0;                			// Pi / 4
		Pi2 = cweeMath::PI / 2.0;							// Pi / 2
		M0 = centralMeridian * cweeMath::AngRad;			// central meridian
		P1 = LatFirstStandardParallel * cweeMath::AngRad;	// latitude of first standard parallel
		P2 = LatSecondStandardParallel * cweeMath::AngRad;	// latitude of second standard parallel
		P0 = LatOrigin * cweeMath::AngRad;   				// latitude of origin
		X0 = FalseEasting;									// False easting of central meridian, map units
		Y0 = FalseNorthing;									// False northing of central meridian, map units

		m1 = ::cos(P1) / ::sqrt(1 - (Ec2 * ::pow((::sin(P1)), 2.0)));
		m2 = ::cos(P2) / ::sqrt(1.0 - (Ec2 * ::pow((::sin(P2)), 2.0)));

		t1 = ::tan(Pi4 - (P1 / 2.0)) / ::pow((1.0 - Ec * ::sin(P1)) / (1.0 + Ec * ::sin(P1)), EcD2);
		t2 = ::tan(Pi4 - (P2 / 2.0)) / ::pow((1.0 - Ec * ::sin(P2)) / (1.0 + Ec * ::sin(P2)), EcD2);
		t0 = ::tan(Pi4 - (P0 / 2.0)) / ::pow((1.0 - Ec * ::sin(P0)) / (1.0 + Ec * ::sin(P0)), EcD2);
		n = ::log(m1 / m2) / ::log(t1 / t2);
		F = m1 / (n * ::pow(t1, n));
		rho0 = A * F * ::pow(t0, n);

		x = XY_Feet.x - X0;
		y = XY_Feet.y - Y0;

		// calc longitude		
		rho = ::sqrt(::pow(x, 2.0) + (::pow(rho0 - y, 2.0)));
		t = ::pow(rho / (A * F), (1.0 / n));
		LonR = ::atan(x / (rho0 - y)) / n + M0;

		// Substitute the estimate into the iterative calculation that converges on the correct Latitude value.	
		LatR = Pi2 - (2.0 * atan(t));
		for (j = 0; j < 5; ++j) {
			LatR = Pi2 - (2.0 * ::atan(t * ::pow((1.0 - (Ec * ::sin(LatR))) / (1.0 + (Ec * ::sin(LatR))), EcD2)));
		}

		// Convert from radians to degrees.
		return vec2d(LonR / cweeMath::AngRad, LatR / cweeMath::AngRad);
	};


	static cweeThreadedList<vec3> N_SidedCircle(int NumSides, const vec3& center, float radius) {
		cweeThreadedList<vec3> out;
		if (NumSides > 0) {
			out.SetGranularity(NumSides + 3);

			vec3 toAdd; toAdd.z = center.z;
			for (int n = 0; n < NumSides; n++) {
				toAdd.x = radius * ::cos(2.0f * cweeMath::PI * n / NumSides) + center.x;
				toAdd.y = radius * ::sin(2.0f * cweeMath::PI * n / NumSides) + center.y;
				out.Append(toAdd);
			}

		}
		return out;
	};
	static cweeThreadedList<vec3d> N_SidedCircle(int NumSides, const vec3d& center, float radius) {
		cweeThreadedList<vec3d> out;
		if (NumSides > 0) {
			out.SetGranularity(NumSides + 3);

			vec3d toAdd; toAdd.z = center.z;
			for (int n = 0; n < NumSides; n++) {
				toAdd.x = radius * ::cos(2.0f * cweeMath::PI * n / NumSides) + center.x;
				toAdd.y = radius * ::sin(2.0f * cweeMath::PI * n / NumSides) + center.y;
				out.Append(toAdd);
			}

		}
		return out;
	};

	static float AngleOffsetFromVerticle(const vec3& middle, const vec3& target)
	{
		double angle; double RadianToDegree = (360.0 / cweeMath::TWO_PI);
		// x and y args to atan2() swapped to rotate resulting angle 90 degrees
		// (Thus angle in respect to +Y axis instead of +X axis)
		angle = ::atan2(middle.x - target.x, target.y - middle.y) * RadianToDegree;

		// Ensure result is in interval [0, 360)
		// Subtract because positive degree angles go clockwise
		return ::fmod(360.0 - angle, 360.0);


		//vec3 vec = vec3((float)target.x - (float)middle.x, (float)target.y - (float)middle.y, 0);
		//vec3 verticle(0, 1, 0);
		//vec.Normalize();
		//auto dot = vec.x * verticle.x + vec.y * verticle.y;
		//auto det = vec.x * verticle.y - vec.y * verticle.x;
		//auto radian = ::atan2((double)det, (double)dot);
		//while (radian < 0) radian += cweeMath::TWO_PI;
		//return radian * (360.0f / cweeMath::TWO_PI); // radian to angle
	}
	static double AngleOffsetFromVerticle(const vec3d& middle, const vec3d& target)
	{
		double angle; double RadianToDegree = (360.0 / cweeMath::TWO_PI);
		angle = ::atan2(middle.x - target.x, target.y - middle.y) * RadianToDegree;
		return ::fmod(360.0 - angle, 360.0);
	}

	class pidLogic {
		/**
		* Copyright 2019 Bradley J. Snyder <snyder.bradleyj@gmail.com>
		*
		* Permission is hereby granted, free of charge, to any person obtaining a copy
		* of this software and associated documentation files (the "Software"), to deal
		* in the Software without restriction, including without limitation the rights
		* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
		* copies of the Software, and to permit persons to whom the Software is
		* furnished to do so, subject to the following conditions:
		*
		* The above copyright notice and this permission notice shall be included in
		* all copies or substantial portions of the Software.
		*
		* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
		* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
		* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
		* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
		* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
		* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
		* THE SOFTWARE.
		*/
	public:
		pidLogic(float maxOutput = 1, float minOutput = 0, float Kp = 0.1, float Kd = 0.1, float Ki = 0.5) :
			_Ku(1), 
			_max(maxOutput),
			_min(minOutput),			
			_Kp(Kp),
			_Kd(Kd),
			_Ki(Ki),
			_pre_error(0),
			_pre_output(1),
			_integral(0)
		{ };
		float calculate(float setpoint, float pv, const u64& dt = 360) {
			float error, Pv_0, Pv_1, Pv_2, Pout_0, Pout_1, Pout_2, expectedPv, Pout, output;

			// Tune the PID values
			tunePID();

			// State
			error = setpoint - pv;

			Pv_0 = setpoint - _pre_error;
			Pv_1 = pv;
			Pv_2 = (2.0f * pv) - setpoint + _pre_error;

			Pout_0 = _Kp * (setpoint - Pv_0) * _Ku;
			Pout_1 = _Kp * (setpoint - Pv_1) * _Ku;
			Pout_2 = _Kp * (setpoint - Pv_2) * _Ku;

			expectedPv = (((Pv_1 - Pv_0) / ((Pout_1 - Pout_0) == 0.0f ? 1 : (Pout_1 - Pout_0))) * (Pout_2 - Pout_1)) + Pv_2;

			Pout = _Kp * ((setpoint - Pv_2) + (setpoint - expectedPv)) * _Ku;

			// Calculate total output
			output = Pout;// ((dt == 0.0f) ? Pout : Pout + Dout);

			// Save error to previous error
			_pre_error = error;

			// adjustment
			output = output + _pre_output; // (output / setpoint) + _pre_output

			// Restrict to max/min
			if (output > _max)
				output = _max;
			else if (output < _min)
				output = _min;

			_pre_output = output;

			return output;
		};
		void setSettingsRange(float max, float min = cweeMath::EPSILON, float kU = cweeMath::EPSILON) {
			_max = max;
			_integral = 0;

			if (min != cweeMath::EPSILON) {
				_min = min;
			}
			if (kU != cweeMath::EPSILON) {
				_Ku = kU;
			}

			//check = 0;
			//_accum_error = -1;
		};
		cweeStr Serialize() {
			cweeStr delim = ":pidLogic:";
			cweeStr out;

			out.AddToDelimiter(_max, delim);
			out.AddToDelimiter(_min, delim);
			out.AddToDelimiter(_Ku, delim);
			out.AddToDelimiter(_Kp, delim);
			out.AddToDelimiter(_Kd, delim);
			out.AddToDelimiter(_Ki, delim);
			out.AddToDelimiter(_pre_error, delim);
			out.AddToDelimiter(_pre_output, delim);
			out.AddToDelimiter(_integral, delim);

			return out;
		};
		void Deserialize(const cweeStr& in) {
			cweeParser obj(in, ":pidLogic:", true);

			_max = (float)obj[0];
			_min = (float)obj[1];
			_Ku = (float)obj[2];
			_Kp = (float)obj[3];
			_Kd = (float)obj[4];
			_Ki = (float)obj[5];
			_pre_error = (float)obj[6];
			_pre_output = (float)obj[7];
			_integral = 0;
		};
		float _Ku;
	private:
		float _max;
		float _min;
		float _Kp;
		float _Kd;
		float _Ki;
		float _pre_error;
		float _pre_output;
		float _integral;

		//cweeThreadedList<float> selfTuner;
		//Pattern kU_trials;
		//float _accum_error = -1;
		//int check = 0;

		void tunePID() {
			// automatically self-tune the PID if (and as) necessary		

			//if (check == 100000) {
			//	check = -1; // no more 'calibration'

			//	float bestError = kU_trials.GetMinValue();
			//	_Ku = kU_trials.TimeForIndex(kU_trials.FindExactY(bestError));
			//	kU_trials.Clear();
			//	kU_trials.AddValue(_Ku, bestError);

			//}else if (check >= 0){
			//	selfTuner.Append(_pre_error);
			//	check++;
			//	if (selfTuner.Num() >= 10) {
			//		if (_accum_error < 0) {
			//			float min = cweeMath::INF; float max = -cweeMath::INF;
			//			_accum_error = 0; for (auto& x : selfTuner) {
			//				_accum_error += x;
			//				if (x < min) min = x;
			//				if (x > max) max = x;
			//			}
			//			_accum_error += (max - min); // the "range" of error should be minimized as well.
			//			kU_trials.AddValue(_Ku, cweeMath::Fabs(_accum_error));
			//		}
			//		else {
			//			float min = cweeMath::INF; float max = -cweeMath::INF;
			//			_accum_error = 0; for (auto& x : selfTuner) {
			//				_accum_error += x;
			//				if (x < min) min = x;
			//				if (x > max) max = x;
			//			}
			//			_accum_error += (max - min); // the "range" of error should be minimized as well.

			//			float t = cweeMath::Fabs(_accum_error);
			//			kU_trials.AddValue(_Ku, t);

			//			if (t >= _accum_error) {
			//				// we could do better. 					
			//				if (check % 100 == 0) {
			//					float bestError = kU_trials.GetMinValue();
			//					_Ku = kU_trials.TimeForIndex(kU_trials.FindExactY(bestError));
			//					kU_trials.Clear();
			//					kU_trials.AddValue(_Ku, bestError);
			//				}
			//				else {
			//					_Ku += cweeRandomFloat(-2.0f / (1.0f + (((float)check) / 1000.0f)), 2.0f / (1.0f + (((float)check) / 1000.0f)));
			//				}
			//			}

			//			_accum_error = t;
			//		}
			//		selfTuner.Clear();
			//	}
			//}

		};

	};

	class Solution {
	public:
		static bool mycmpA(vec3& a, vec3& b) {
			return a.x < b.x;
		}
		static int testSide(vec3& a, vec3& b, vec3& c) {
			// cross product of (AB and AC vectors)
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}
		static double distvec3Line(vec3& A, vec3& B, vec3& C) {
			// dist(line: ax+by+c=0, and vec3(x0, y0)): (a*x0 + b*y0 + c)/sqrt(a^2+b^2)
			// line: (y2-y1)*x - (x2-x1)*y + x2*y1 - y2*x1 = 0
			int a = B.y - A.y, b = B.x - A.x;
			return abs((a * C.x - b * C.y + B.x * A.y - B.y * A.x) / sqrt(a * a + b * b));
		}
		static void FindHull(std::vector<vec3>& vec3s, vec3& A, vec3& B, std::vector<vec3>& ret) {
			if (vec3s.empty())
				return;
			int idx = 0;
			double dist = distvec3Line(A, B, vec3s[0]);
			for (size_t i = 1; i < vec3s.size(); i++) {
				if (distvec3Line(A, B, vec3s[i]) > dist) {
					dist = distvec3Line(A, B, vec3s[i]);
					idx = i;
				}
			}
			ret.push_back(vec3s[idx]);
			std::vector<vec3> R, T;
			for (size_t i = 0; i < vec3s.size(); i++) {
				if (i != idx) {
					int tmp = testSide(A, vec3s[idx], vec3s[i]);
					if (tmp >= 0)
						R.push_back(vec3s[i]);
					else {
						tmp = testSide(vec3s[idx], B, vec3s[i]);
						if (tmp >= 0)
							T.push_back(vec3s[i]);
					}
				}
			}
			FindHull(R, A, vec3s[idx], ret);
			FindHull(T, vec3s[idx], B, ret);
			return;
		}
		static std::vector<vec3> outerTrees(std::vector<vec3>& vec3s) {
			std::vector<vec3> ret;

			// find the convex hull; use QuickHull algorithm
			if (vec3s.size() <= 1)
				return vec3s;
			// find the left most and right most two vec3s
			std::sort(vec3s.begin(), vec3s.end(), mycmpA);
			ret.push_back(vec3s[0]);
			ret.push_back(vec3s.back());
			// test whether a vec3 on the left side right side or on the line
			std::vector<vec3> Left, Right, Online;
			for (size_t i = 1; i < vec3s.size() - 1; i++) {
				int tmp = testSide(vec3s[0], vec3s.back(), vec3s[i]);
				if (tmp < 0)
					Right.push_back(vec3s[i]);
				else if (tmp > 0)
					Left.push_back(vec3s[i]);
				else
					Online.push_back(vec3s[i]);
			}
			// if Upper or Down is empty, Online should be pushed into ret
			if (Left.empty() || Right.empty())
				for (size_t i = 0; i < Online.size(); i++)
					ret.push_back(Online[i]);
			FindHull(Left, vec3s[0], vec3s.back(), ret);
			FindHull(Right, vec3s.back(), vec3s[0], ret);
			return ret;
		}
		static int testSide(const vec3d& a, const vec3d& b, const vec3d& c) {
			// cross product of (AB and AC vectors)
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}
		static double distvec3dLine(const vec3d& A, const vec3d& B, const vec3d& C) {
			// dist(line: ax+by+c=0, and vec3d(x0, y0)): (a*x0 + b*y0 + c)/sqrt(a^2+b^2)
			// line: (y2-y1)*x - (x2-x1)*y + x2*y1 - y2*x1 = 0
			double a = B.y - A.y, b = B.x - A.x; // was int
			return abs((a * C.x - b * C.y + B.x * A.y - B.y * A.x) / sqrt(a * a + b * b));
		}
		static void FindHull(const cweeThreadedList<vec3d>& vec3ds, const vec3d& A, const vec3d& B, cweeThreadedList<vec3d>& ret) {
			if (vec3ds.Num() <= 0) return;

			int idx = 0;
			double dist = distvec3dLine(A, B, vec3ds[0]);
			double C;
			for (int i = 1; i < vec3ds.Num(); i++) {
				C = distvec3dLine(A, B, vec3ds[i]);
				if (C > dist) {
					dist = C;
					idx = i;
				}
			}
			ret.Append(vec3ds[idx]);
			cweeThreadedList<vec3d> R(vec3ds.Num() + 16);
			cweeThreadedList<vec3d> T(vec3ds.Num() + 16);
			int tmp;
			for (int i = 0; i < vec3ds.Num(); i++) {
				if (i != idx) {
					tmp = testSide(A, vec3ds[idx], vec3ds[i]);
					if (tmp >= 0)
						R.Append(vec3ds[i]);
					else {
						tmp = testSide(vec3ds[idx], B, vec3ds[i]);
						if (tmp >= 0)
							T.Append(vec3ds[i]);
					}
				}
			}
			FindHull(R, A, vec3ds[idx], ret);
			FindHull(T, vec3ds[idx], B, ret);
			return;
		}
		static void outerTrees(cweeThreadedList<vec3d>& vec3ds, cweeThreadedList<vec3d>& ret) {
			ret.SetGranularity(vec3ds.Num() + 16);

			int n = vec3ds.Num();

			// find the convex hull; use QuickHull algorithm
			if (n <= 1) {
				ret = vec3ds;
				return;
			}

			// find the left most and right most two vec3ds		
			vec3d_quickSort(vec3ds.Ptr(), 0, vec3ds.Num() - 1);
			ret.Append(vec3ds[0]);
			ret.Append(vec3ds[n - 1]);

			// test whether a vec3d on the left side right side or on the line
			cweeThreadedList<vec3d> Left(n + 16);
			cweeThreadedList<vec3d> Right(n + 16);
			cweeThreadedList<vec3d> Online(n + 16);
			int tmp;
			for (int i = 1; i < n - 1; i++) {
				tmp = testSide(vec3ds[0], vec3ds[n - 1], vec3ds[i]);
				if (tmp < 0)
					Right.Append(vec3ds[i]);
				else if (tmp > 0)
					Left.Append(vec3ds[i]);
				else
					Online.Append(vec3ds[i]);
			}

			// if Upper or Down is empty, Online should be pushed into ret
			if (Left.Num() <= 0 || Right.Num() <= 0)
				for (int i = 0; i < Online.Num(); i++)
					ret.Append(Online[i]);

			FindHull(Left, vec3ds[0], vec3ds[n - 1], ret);
			FindHull(Right, vec3ds[n - 1], vec3ds[0], ret);

			return;
		}
	};

	// reorders / drops points such that this can be drawn as a simple polygon without the lines self-crossing 
	static void ReorderConvexHull(cweeThreadedList<vec3>& points) {
		{ // get the convex hull
			std::vector<vec3> v; v = points;

			points.Clear();

			for (auto& x : Solution::outerTrees(v))
				points.Append(x);
		}
		{ // guarrantee the draw order (Clockwise)
			vec3 middle(0, 0, 0);
			int numSamplesX = 0; int numSamplesY = 0;
			for (auto& point : points)
			{
				cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
				cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
			}
			middle.z = 0;

	
			cweeCurve<vec3> ordered;
			float key;
			for (int i = 0; i < points.Num(); i++)
			{
				key = AngleOffsetFromVerticle(middle, points[i]);
				key = ::fmod(360.0 - key, 360.0);
				key += 360.0;
				ordered.AddUniqueValue(key, points[i]);
			}
			points.Clear();
			for (auto& x : ordered.GetKnotSeries()) points.Append(x.second);


		}
	};

	static void ReorderConvexHull(cweeThreadedList<vec3d>& points) {

		// get the convex hull
		cweeThreadedList<vec3d> returned;
		Solution::outerTrees(points, returned);

		{ // guarrantee the draw order (Clockwise)
			vec3d middle(0, 0, 0);
			int numSamplesX = 0; int numSamplesY = 0;
			for (auto& point : points)
			{
				cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
				cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
			}
			middle.z = 0;

			cweeCurve<vec3d> ordered; ordered.SetGranularity(returned.Num() + 16);
			double key;
			for (int i = 0; i < returned.Num(); i++)
			{
				key = AngleOffsetFromVerticle(middle, returned[i]);
				key = ::fmod(360.0 - key, 360.0);
				key += 360.0;
				ordered.AddUniqueValue(key, returned[i]);
			}

			int n = ordered.GetNumValues();
			points.SetNum(n);
			for (int i = 0; i < n; i++) points[i] = *ordered.GetValueAddress(i);
		}
	};

	static bool IsPointInPolygon(const cweeThreadedList<vec3>& polygon, const vec3& testPoint)
	{
		bool result = false;
		int j = polygon.Num() - 1;
		for (int i = 0; i < polygon.Num(); i++)
		{
			if (
				(polygon[i].y < testPoint.y 
				&& 
				polygon[j].y >= testPoint.y) 
				|| 
				(polygon[j].y < testPoint.y 
				&& 
				polygon[i].y >= testPoint.y)
				)
			{
				if (polygon[i].x + (testPoint.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x) < testPoint.x)
				{
					result = !result;
				}
			}
			j = i;
		}
		return result;
	};
	static bool IsPointInPolygon(const cweeThreadedList<vec3d>& polygon, const vec3d& testPoint)
	{
		bool result = false;
		int j = polygon.Num() - 1;
		for (int i = 0; i < polygon.Num(); i++)
		{
			if (
				(polygon[i].y < testPoint.y 
				&& 
				polygon[j].y >= testPoint.y) 
				|| 
				(polygon[j].y < testPoint.y 
				&& 
				polygon[i].y >= testPoint.y)
				)
			{
				if (polygon[i].x + (testPoint.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x) < testPoint.x)
				{
					result = !result;
				}
			}
			j = i;
		}
		return result;
	};
	static bool IsPointInPolygon(const cweeThreadedList<vec2d>& polygon, const vec2d& testPoint)
	{
		bool result = false;
		int j = polygon.Num() - 1;
		for (int i = 0; i < polygon.Num(); i++)
		{
			if (
				(polygon[i].y < testPoint.y
					&&
					polygon[j].y >= testPoint.y)
				||
				(polygon[j].y < testPoint.y
					&&
					polygon[i].y >= testPoint.y)
				)
			{
				if (polygon[i].x + (testPoint.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x) < testPoint.x)
				{
					result = !result;
				}
			}
			j = i;
		}
		return result;
	};

	static int WhichSide(const cweeThreadedList<vec3>& C, const vec2& D, const vec3& V)
	{
		int i; float t;
		// C vertices are projected to the form V+t*D.
		// Return value is +1 if all t > 0, -1 if all t < 0, 0 otherwise, in
		// which case the line splits the polygon.
		int positive = 0, negative = 0;
		for (i = 0; i < C.Num(); i++)
		{
			t = D.Dot((C[i] - V).GetVec2());
			if (t > 0) positive++; else if (t < 0) negative++;
			if (positive && negative) return 0;
		}
		return (positive ? +1 : -1);
	};
	static int WhichSide(const cweeThreadedList<vec3d>& C, const vec2d& D, const vec3d& V)
	{
		int i; float t;
		// C vertices are projected to the form V+t*D.
		// Return value is +1 if all t > 0, -1 if all t < 0, 0 otherwise, in
		// which case the line splits the polygon.
		int positive = 0, negative = 0;
		for (i = 0; i < C.Num(); i++)
		{
			t = D.Dot((C[i] - V).GetVec2());
			if (t > 0) positive++; else if (t < 0) negative++;
			if (positive && negative) return 0;
		}
		return (positive ? +1 : -1);
	};

	static bool ObjectsIntersect(const cweeThreadedList<vec3>& C0, const cweeThreadedList<vec3>& C1)
	{
		int i0, i1;
		vec2 E, D;

		// Test edges of C0 for separation. Because of the counterclockwise ordering,
		// the projection interval for C0 is [m,0] where m <= 0. Only try to determine
		// if C1 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C0.Num() - 1; i0 < C0.Num(); i1 = i0, i0++)
		{
			E = (C0[i0] - C0[i1]).GetVec2(); // or precompute edges if desired
			D = vec2(E.y, -E.x);
			if (WhichSide(C1, D, C0[i0]) > 0)
			{ // C1 is entirely on ‘positive’ side of line C0.V(i0)+t*D
				return false;
			}
		}
		// Test edges of C1 for separation. Because of the counterclockwise ordering,
		// the projection interval for C1 is [m,0] where m <= 0. Only try to determine
		// if C0 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C1.Num() - 1; i0 < C1.Num(); i1 = i0, i0++)
		{
			E = (C1[i0] - C1[i1]).GetVec2(); // or precompute edges if desired
			D = vec2(E.y, -E.x);
			if (WhichSide(C0, D, C1[i0]) > 0)
			{ // C0 is entirely on ‘positive’ side of line C1.V(i0)+t*D
				return false;
			}
		}
		return true;
	};
	static bool ObjectsIntersect(const cweeThreadedList<vec3d>& C0, const cweeThreadedList<vec3d>& C1)
	{
		int i0, i1;
		vec2d E, D;

		// Test edges of C0 for separation. Because of the counterclockwise ordering,
		// the projection interval for C0 is [m,0] where m <= 0. Only try to determine
		// if C1 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C0.Num() - 1; i0 < C0.Num(); i1 = i0, i0++)
		{
			E = (C0[i0] - C0[i1]).GetVec2(); // or precompute edges if desired
			D = vec2d(E.y, -E.x);
			if (WhichSide(C1, D, C0[i0]) > 0)
			{ // C1 is entirely on ‘positive’ side of line C0.V(i0)+t*D
				return false;
			}
		}
		// Test edges of C1 for separation. Because of the counterclockwise ordering,
		// the projection interval for C1 is [m,0] where m <= 0. Only try to determine
		// if C0 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C1.Num() - 1; i0 < C1.Num(); i1 = i0, i0++)
		{
			E = (C1[i0] - C1[i1]).GetVec2(); // or precompute edges if desired
			D = vec2d(E.y, -E.x);
			if (WhichSide(C0, D, C1[i0]) > 0)
			{ // C0 is entirely on ‘positive’ side of line C1.V(i0)+t*D
				return false;
			}
		}
		return true;
	};

	static bool PolygonsOverlap(const cweeThreadedList<vec3>& polygon1, const cweeThreadedList<vec3>& polygon2) {
		return ObjectsIntersect(polygon1, polygon2);
	};
	static bool PolygonsOverlap(const cweeThreadedList<vec3d>& polygon1, const cweeThreadedList<vec3d>& polygon2) {
		return ObjectsIntersect(polygon1, polygon2);
	};

	template<typename T>
	static AUTO R_Squared(const cweeList<T>& Real, const cweeList<T>& Estimate) {		
		T avg_real; { avg_real = 0; int numSamples = 0;
			for (auto& x : Real) cweeMath::rollingAverageRef(avg_real, x, numSamples);
		}


		int N = cweeMath::min(Real.Num(), Estimate.Num());
		double SumErrReal = 0;
		double SumErrEstimate = 0;

		for (int i = 0; i < N; i++) {
			double real_error = (double)(Real[i] - avg_real);
			double estimate_error = (double)(Real[i] - Estimate[i]);
			SumErrReal += real_error * real_error;
			SumErrEstimate += estimate_error * estimate_error;
		};

		AUTO out = avg_real / avg_real; 
		out = 1.0 - (SumErrEstimate / SumErrReal);
		return out;
	};

};




