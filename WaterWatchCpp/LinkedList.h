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
#include "List.h"

// #define cweeLinkedListUseRecursiveRelativeExploration

template <typename T, int numChildren = 100>
class cweeLinkedList;

template <typename T, int numChildren = 100>
class cweeLinkedListNode;

#pragma region "cweeLinkedListNode"

template <typename T, int numChildren>
class cweeLinkedListNode {
public:
	typedef T _type_;
	typedef  cweeLinkedListNode _listType_;
	struct it_state {
		int pos = 0;
		inline void begin(const _listType_* ref) { pos = 0; }
		inline void next(const _listType_* ref) { ++pos; }
		inline void end(const _listType_* ref) { pos = ref->Num(); }
		inline _type_& get(_listType_* ref) { return ref->relativeIndex(pos); }
		inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
		inline long long distance(const it_state& s) const { return pos - s.pos; };

		// Optional to allow operator--() and reverse iterators:
		inline void prev(const _listType_* ref) { --pos; }
		// Optional to allow `const_iterator`:
		inline const _type_& get(const _listType_* ref) const { return ref->relativeIndex(pos); }
	};

	using ParentClass = cweeLinkedListNode;
	using IterType = _type_;
	using StateType = it_state;

	typedef std::ptrdiff_t difference_type;
	typedef size_t size_type; typedef IterType value_type; typedef IterType* pointer; typedef const IterType* const_pointer;
	typedef IterType& reference;
	typedef const IterType& const_reference;
	class iterator : public std::iterator<std::random_access_iterator_tag, value_type> {
	public: const ParentClass* ref;	mutable StateType state;
		  using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;
		  iterator() : ref(nullptr), state() {};
		  iterator(const ParentClass* parent) : ref(parent), state() {};
		  inline iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };
		  inline iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };
		  inline value_type& operator*() const { return state.get(ref); }
		  inline value_type* operator->() const { return &state.get(ref); }
		  inline value_type& operator[](difference_type rhs) const { return *(*this + rhs); }
		  inline iterator& operator++() { state.next(ref); return *this; };
		  inline iterator& operator--() { state.prev(ref); return *this; };
		  inline iterator operator++(int) const { iterator retval = *this; ++(*this); return retval; };
		  inline iterator operator--(int) const { iterator retval = *this; --(*this); return retval; };
		  inline difference_type operator-(iterator const& other) const { return state.distance(other.state); };
		  inline iterator operator+(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };
		  inline iterator operator-(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };
		  friend inline iterator operator+(difference_type lhs, const iterator& rhs) { return rhs + lhs; }
		  friend inline iterator operator-(difference_type lhs_pos, const iterator& rhs) { iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++) { ++retval; } return retval; }
		  inline bool operator==(const iterator& other) const { return !(operator!=(other)); }
		  inline bool operator!=(const iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }
		  inline bool operator>(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }
		  inline bool operator<(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos; }
		  inline bool operator>=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos; }
		  inline bool operator<=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos; }
		  iterator& begin() { state.begin(ref); return *this; };
		  iterator& end() { state.end(ref); return *this; };
	};
	iterator begin() { return iterator(this).begin(); };
	iterator end() { return iterator(this).end(); };
	class const_iterator : public std::iterator<std::random_access_iterator_tag, value_type> {
	public: const ParentClass* ref;	mutable StateType state;
		  using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;
		  const_iterator() : ref(nullptr), state() {};
		  const_iterator(const ParentClass* parent) : ref(parent), state() {};
		  inline const_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };
		  inline const_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };
		  inline const value_type& operator*() const { return state.get(ref); }
		  inline const value_type* operator->() const { return &state.get(ref); }
		  inline const value_type& operator[](difference_type rhs) const { return *(*this + rhs); }
		  inline const_iterator& operator++() { state.next(ref); return *this; };
		  inline const_iterator& operator--() { state.prev(ref); return *this; };
		  inline const_iterator operator++(int) const { const_iterator retval = *this; ++(*this); return retval; };
		  inline const_iterator operator--(int) const { const_iterator retval = *this; --(*this); return retval; };
		  inline difference_type operator-(const_iterator const& other) const { return state.distance(other.state); };
		  inline const_iterator operator+(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };
		  inline const_iterator operator-(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };
		  friend inline const_iterator operator+(difference_type lhs, const const_iterator& rhs) { return rhs + lhs; }
		  friend inline const_iterator operator-(difference_type lhs_pos, const const_iterator& rhs) { const_iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++) { ++retval; } return retval; }
		  inline bool operator==(const const_iterator& other) const { return !(operator!=(other)); }
		  inline bool operator!=(const const_iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }
		  inline bool operator>(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }
		  inline bool operator<(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos; }
		  inline bool operator>=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos; }
		  inline bool operator<=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos; }
		  const_iterator& begin() { state.begin(ref); return *this; };
		  const_iterator& end() { state.end(ref); return *this; };
	};
	const_iterator cbegin() const { return const_iterator(this).begin(); };
	const_iterator cend() const { return const_iterator(this).end(); };
	const_iterator begin() const { return cbegin(); };
	const_iterator end() const { return cend(); };

	// SETUP_STL_ITERATOR(cweeLinkedListNode, _type_, it_state);

	cweeLinkedListNode() : parent(nullptr), parentIndex(0), prev(nullptr), next(nullptr), _relativeIndex(0), childrenPerNode(numChildren) {
		_data.SetGranularity(childrenPerNode + 1);
	};
	cweeLinkedListNode(cweeLinkedList<T, numChildren>* owner, int ParentIndex, int childrenNum) : parent(owner), parentIndex(ParentIndex), prev(nullptr), next(nullptr), _relativeIndex(0), childrenPerNode(childrenNum) {
		_data.SetGranularity(childrenPerNode + 1);
	};
	~cweeLinkedListNode() = default;
	cweeLinkedListNode(cweeLinkedListNode&& other) : parent(other.parent), parentIndex(other.parentIndex), prev(other.prev), next(other.next), _relativeIndex(other._relativeIndex), _data(other._data), childrenPerNode(other.childrenPerNode) {
		other.parent = nullptr;
		other.parentIndex = 0;
		other.prev = nullptr;
		other.next = nullptr;
		other._relativeIndex = 0;
		other._data.Clear();
		other.childrenPerNode = numChildren;
	};
	cweeLinkedListNode(const cweeLinkedListNode& other) : parent(other.parent), parentIndex(other.parentIndex), prev(other.prev), next(other.next), _relativeIndex(other._relativeIndex), _data(other._data), childrenPerNode(other.childrenPerNode) {};

	cweeLinkedListNode& operator=(const cweeLinkedListNode& other);
	cweeLinkedListNode& operator=(cweeLinkedListNode&& other);

	/*! Add new data to this node at the end of its list */
	void Append();

	/*! Add new data to this node at the end of its list */
	void Append(const T& obj);

	/*! Insert new data into this node somewhere in the middle of the list */
	void Insert(const T& obj, int relativeIndex);

	/*! Remove data from this node somewhere in the middle of the list */
	bool Remove(int relativeIndex);

	/*! Remove data from this node somewhere in the middle of the list. Does not maintain list order. */
	bool RemoveFast(int relativeIndex);

	/*! Remove data from this node somewhere in the middle of the list */
	bool Extract(int relativeIndex, T& out);

	/*! Remove data from this node somewhere in the middle of the list. Does not maintain list order. */
	bool ExtractFast(int relativeIndex, T& out);

	/*! Merge the right-list into this list. Opposit of a split. */
	void AbsorbTheRight();

	/*! Split this node into 2 (this and a new node to its right) and distribute the values between them. */
	void Split();

	/*! returns the value from this node from the perspective of the parent */
	T& absoluteIndex(int absoluteIndex);
	/*! returns the value from this node from the perspective of the parent */
	const T& absoluteIndex(int absoluteIndex) const;

	/*! returns the value from this node from the perspective of itself */
	T& relativeIndex(int relativeIndex);
	/*! returns the value from this node from the perspective of itself */
	const T& relativeIndex(int relativeIndex) const;

	int Num() const;

	int NumRef() const;

	int MinIndex() const;

	int MaxIndex() const;

	cweeThreadedList<T>& Data();
	const cweeThreadedList<T>& Data() const;

	cweeLinkedListNode* GetNext();
	const cweeLinkedListNode* GetNext() const;


protected:
	cweeLinkedList<T, numChildren>* parent;
	cweeLinkedListNode* prev;
	cweeLinkedListNode* next;
	int	childrenPerNode = numChildren;
	int parentIndex;
	mutable int	_relativeIndex;
	cweeThreadedList<T>	_data;

private:
#ifdef cweeLinkedListUseRecursiveRelativeExploration
	void IncrementDownstreamRelativeIndexes(cweeLinkedListNode* which) {
		if (!which) return;
		which->_relativeIndex++;
		IncrementDownstreamRelativeIndexes(which->next);
	};
	void DecrementDownstreamRelativeIndexes(cweeLinkedListNode* which) {
		if (!which) return;
		which->_relativeIndex--;
		DecrementDownstreamRelativeIndexes(which->next);
	};
#endif



};

#pragma endregion

#pragma region "cweeLinkedList"

/*! Nested list of lists with a shared master index. Used like a regular vector, but benefited by significantly improved removal/insert speeds. */
template <typename T, int numChildren>
class cweeLinkedList {
public:

#pragma region Iterators
	typedef T _type_;
	typedef cweeLinkedList _listType_;
	struct it_state {
		mutable int pos = 0;
		mutable cweeLinkedListNode<T, numChildren>* node = nullptr;
		mutable int nodeIndex = 0;

		inline void begin(const _listType_* ref) { pos = 0; }
		inline void next(const _listType_* ref) { ++pos; }
		inline void end(const _listType_* ref) { pos = ref->Num(); }
		inline _type_& get(_listType_* ref) {
			if (!node) node = ref->GetNode(nodeIndex);
			if (pos < node->MaxIndex()) {
				return node->absoluteIndex(pos);
			}
			nodeIndex++;
			node = node->GetNext();
			return get(ref);
		}
		inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
		inline long long distance(const it_state& s) const { return pos - s.pos; };
		// Optional to allow `const_iterator`:
		inline void prev(const _listType_* ref) { --pos; }
		inline const _type_& get(const _listType_* ref) const {
			if (!node) node = const_cast<cweeLinkedListNode<T, numChildren> *>(ref->GetNode(nodeIndex));
			if (pos < node->MaxIndex()) {
				return node->absoluteIndex(pos);
			}
			nodeIndex++;
			node = const_cast<cweeLinkedListNode<T, numChildren>*>(node->GetNext());
			return get(ref);
		}
	};

#if 0
	using ParentClass = cweeLinkedList;
	using IterType = _type_;
	using StateType = it_state;
	typedef std::ptrdiff_t difference_type;
	typedef size_t size_type; typedef IterType value_type; typedef IterType* pointer; typedef const IterType* const_pointer;
	typedef IterType& reference;
	typedef const IterType& const_reference;
	class iterator : public std::iterator<std::random_access_iterator_tag, value_type> {
	public: ParentClass* ref;	mutable StateType state;
		  using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;
		  iterator() : ref(nullptr), state() {};
		  iterator(ParentClass* parent) : ref(parent), state() {};
		  inline iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };
		  inline iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };
		  inline value_type& operator*() { return state.get(ref); }
		  inline value_type* operator->() { return &state.get(ref); }
		  inline const value_type& operator*() const { return state.get(ref); }
		  inline const value_type* operator->() const { return &state.get(ref); }
		  inline value_type& operator[](difference_type rhs) const { return *(*this + rhs); }
		  inline iterator& operator++() { state.next(ref); return *this; };
		  inline iterator& operator--() { state.prev(ref); return *this; };
		  inline iterator operator++(int) { iterator retval = *this; ++(*this); return retval; };
		  inline iterator operator--(int) { iterator retval = *this; --(*this); return retval; };
		  inline difference_type operator-(iterator const& other) const { return state.distance(other.state); };
		  inline iterator operator+(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };
		  inline iterator operator-(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };
		  friend inline iterator operator+(difference_type lhs, const iterator& rhs) { return rhs + lhs; }
		  friend inline iterator operator-(difference_type lhs_pos, const iterator& rhs) { iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++) { ++retval; } return retval; }
		  inline bool operator==(const iterator& other) const { return !(operator!=(other)); }
		  inline bool operator!=(const iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }
		  inline bool operator>(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }
		  inline bool operator<(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos; }
		  inline bool operator>=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos; }
		  inline bool operator<=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos; }
		  iterator& begin() { state.begin(ref); return *this; };
		  iterator& end() { state.end(ref); return *this; };
	};
	iterator begin() { return iterator(this).begin(); };
	iterator end() { return iterator(this).end(); };
	class const_iterator : public std::iterator<std::random_access_iterator_tag, value_type> {
	public: const ParentClass* ref;	mutable StateType state;
		  using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;
		  const_iterator() : ref(nullptr), state() {};
		  const_iterator(const ParentClass* parent) : ref(parent), state() {};
		  inline const_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };
		  inline const_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };
		  inline const value_type& operator*() const { return state.get(ref); }
		  inline const value_type* operator->() const { return &state.get(ref); }
		  inline const value_type& operator[](difference_type rhs) const { return *(*this + rhs); }
		  inline const_iterator& operator++() { state.next(ref); return *this; };
		  inline const_iterator& operator--() { state.prev(ref); return *this; };
		  inline const_iterator operator++(int) { const_iterator retval = *this; ++(*this); return retval; };
		  inline const_iterator operator--(int) { const_iterator retval = *this; --(*this); return retval; };
		  inline difference_type operator-(const_iterator const& other) const { return state.distance(other.state); };
		  inline const_iterator operator+(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };
		  inline const_iterator operator-(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };
		  friend inline const_iterator operator+(difference_type lhs, const const_iterator& rhs) { return rhs + lhs; }
		  friend inline const_iterator operator-(difference_type lhs_pos, const const_iterator& rhs) { const_iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++) { ++retval; } return retval; }
		  inline bool operator==(const const_iterator& other) const { return !(operator!=(other)); }
		  inline bool operator!=(const const_iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }
		  inline bool operator>(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }
		  inline bool operator<(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos; }
		  inline bool operator>=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos; }
		  inline bool operator<=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos; }
		  const_iterator& begin() { state.begin(ref); return *this; };
		  const_iterator& end() { state.end(ref); return *this; };
	};
	const_iterator cbegin() const { return const_iterator(this).begin(); };
	const_iterator cend() const { return const_iterator(this).end(); };
	const_iterator begin() const { return cbegin(); };
	const_iterator end() const { return cend(); };
#else
	SETUP_STL_ITERATOR(cweeLinkedList, _type_, it_state);
#endif

#pragma endregion

#pragma region Initializers
	/*! Empty initializer. */
	cweeLinkedList() : num(0), currentIndex(0) { InitializeAllocator(); SetGranularity(childrenPerNode * 100); };
	/*! Granulator initializer. */
	cweeLinkedList(int newgranularity) : num(0), currentIndex(0) { InitializeAllocator(); SetGranularity(newgranularity); };
	/*! Copy initializer. */
	cweeLinkedList(const cweeLinkedList& other) : num(0), currentIndex(0) { InitializeAllocator(); this->operator=(other); };
	/*! Destructor. */
	~cweeLinkedList() { ShutdownAllocator(); };
#pragma endregion

#pragma region Casts
	/*! Cast to std::vector */
	operator std::vector<T>() {
		std::vector<T> out;
		if (num > 0) {
			out.reserve(num);
			for (auto& x : *this) {
				out.Append(x);
			}
		}
		return out;
	};
	/*! Cast to std::vector */
	operator std::vector<T>() const {
		std::vector<T> out;
		if (num > 0) {
			out.reserve(num);
			for (auto& x : *this) {
				out.Append(x);
			}
		}
		return out;
	};

	/*! Cast to cweeThreadedList */
	template< typename _othertype_ > operator cweeThreadedList<_othertype_>() const {
		cweeThreadedList<_othertype_> out(16 > GetGranularity() ? 16 : GetGranularity());
		for (auto& x : *this) {
			out.Append((_othertype_)x);
		}
		return out;
	};
	/*! Cast to cweeThreadedList */
	template< typename _othertype_ > operator cweeThreadedList<_othertype_>() {
		cweeThreadedList<_othertype_> out(16 > GetGranularity() ? 16 : GetGranularity());
		for (auto& x : *this) {
			out.Append((_othertype_)x);
		}
		return out;
	};

#pragma endregion

#pragma region Operators
	/*! Random index access operator. Index must exist in the list, otherwise undefined behavior. */
	const T& operator[](int index) const;
	/*! Const random index access operator. Index must exist in the list, otherwise undefined behavior. */
	T& operator[](int index);
	/*! Copy operator */
	cweeLinkedList& operator=(const cweeLinkedList& other);
	/*! returns true if 'object is found in the list. false otherwise. */
	bool						operator==(const T& other);
	/*! returns true is the lists are equivalent. False otherwise. */
	friend bool					operator==(const  cweeLinkedList& a, const  cweeLinkedList& other) {
		if (a.Num() != other.Num()) return false;

		int n = a.Num();
		for (int i = 0; i < n; i++) {
			if (!(a[i] == other[i]))
				return false;
		}
		return true;
	};
	/*! returns false is the lists are equivalent. True otherwise. */
	friend bool					operator!=(const  cweeLinkedList& a, const  cweeLinkedList& b) {
		return !(a == b);
	};
#pragma endregion

#pragma region "List Management or Manipulation"
	/*! Clears the list. */
	void						Clear();
	/*! Set the granularity of the list, which is similar to reservation with the std::vector. */
	void						SetGranularity(int newgranularity);
	void						Reserve(int newgranularity);
	/*! returns the current granularity of the list */
	int							GetGranularity() const;
	/*! returns the number of objects contained in the list. */
	int							Num() const;
	/*! returns the number of objects contained in the list. */
	int							NumRef() const;
	/*! returns the size of the main node list. */
	size_t						Size() const;
	/*! recover unused memory from the list. Does not remove or move any underlying data. */
	void						Condense();
	/*! Reverses the order of the contents in the list. Requires 2-3 times the amount of memory to perform the operation. */
	cweeLinkedList& Reverse();
	/*! Copies this list to a cweeThreadedList. */
	cweeThreadedList<T>			ToList() const;
	/*! Clears this list, and copies the content of 'inbound' to this list. */
	cweeLinkedList& FromList(const cweeThreadedList<T>& inbound);
	/*! Appends the content of 'inbound' to this list. */
	cweeLinkedList& AppendList(const cweeThreadedList<T>& inbound);
	/*! Converts the list to a contiguous list, sorts it, and returns to a linked list. Not memory-efficient. */
	cweeLinkedList& Sort();

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<const _type_*> Select(std::function<bool(const _type_&)> predicate) const;
	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<_type_*>	Select(std::function<bool(const _type_&)> predicate);
	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<int>		SelectIndexes(std::function<bool(const _type_&)> predicate) const;

#pragma endregion

#pragma region "Data Insertion, Removal, and Searches"
	/*! Appends a new object to the end of the list and returns a reference to it. */
	T& Alloc();
	/*! Appends a new object to the end of the list and returns its index. */
	int							Append();
	/*! Appends the provided object to the end of the list and returns its index. */
	int							Append(const T& obj);
	/*! Appends the provided object to the end of the list if it does not already exist. Otherwise, it overrides the object with the new object and returns its index. */
	int							AddUnique(const T& obj);
	/*! Inserts the provided object into the list at the specified index. All downstread objects (even at this index) are pushed up by one. */
	int							Insert(const T& obj, int index = 0);
	/*! Will always succeed. If the index exists, the object will be overridedn with the new object. Otherwise, the list is extended with Append() calls until the index  exists, and the new object is inserted there anyways. */
	int							Emplace(const T& obj, int index = 0);
	/*! Returns the index to the specified object. Optionally, a reference to the underlying list object can be returned as a pointer-to-pointer. */
	int							FindIndex(const T& obj, T** ptr = nullptr) const;
	/*! Returns the index to the specified object. Optionally, a reference to the underlying list object can be returned as a pointer-to-pointer. */
	int							FindIndex(const T* obj, T** ptr = nullptr) const;
	/*! Returns a reference to the underlying list object if the provided object is found within the list. Nullptr otherwise. */
	T* Find(T const& obj) const;

	/*! Deletes the object at the specified index and pulls all downstream objects down by one to fill the gap. */
	bool						RemoveIndex(int index);
	/*! Delete the list of indexes in one swoop. Typically constant speed. Returns true if successful. False otherwise. */
	void						RemoveIndexes(const cweeThreadedList<int>& indexes);
	/*!  Deletes the object at the specified index and swaps another asset into it's place. Order is not maintained. Returns true if successful. False otherwise. */
	bool						RemoveIndexFast(int index);
	/*! Deletes the object at the specified index and pulls all downstream objects down by one to fill the gap. */
	bool						ExtractIndex(int index, T& out);
	/*!  Deletes the object at the specified index and swaps another asset into it's place. Order is not maintained. Returns true if successful. False otherwise. */
	bool						ExtractIndexFast(int index, T& out);
	/*! Removes the provided object from the list if found, and pulls all downstream assets down by one to fill the gap. Returns true if successful. False otherwise. */
	bool						Remove(const _type_& obj);
	/*! Removes the provided object from the list if found and swaps another asset into it's place. Order is not maintained. Returns true if successful. False otherwise. */
	bool						RemoveFast(const _type_& obj);
	/*! Removes the provided object from the list if found, and pulls all downstream assets down by one to fill the gap. Returns true if successful. False otherwise. */
	bool						Remove(const _type_* obj);
	/*! Removes the provided object from the list if found, and pulls all downstream assets down by one to fill the gap. Returns true if successful. False otherwise. */
	bool						RemoveFast(const _type_* obj);
#pragma endregion

#pragma region "Linked Node Management"
	/*! -- Not intended for public use -- */
	cweeLinkedListNode<T, numChildren>* InsertNode(int nodeIndex);
	/*! -- Not intended for public use -- */
	cweeLinkedListNode<T, numChildren>* AppendNode();
	/*! -- Not intended for public use -- */
	void						FreeNode(int nodeIndex);
	/*! -- Not intended for public use -- */
	int							NumNodes() const;
	/*! -- Not intended for public use -- */
	cweeLinkedListNode<T, numChildren>* GetNode(int index);
	/*! -- Not intended for public use -- */
	const cweeLinkedListNode<T, numChildren>* GetNode(int index) const;
#pragma endregion

#pragma region Data
protected:
	int															childrenPerNode = numChildren; // number of children per node
	mutable int													num; // current number of assets in the linked list
	mutable int													currentIndex; // optimization index for bisection search
	cweeThreadedList<cweeLinkedListNode<T, numChildren>*>		data; // actual linked list of pointers to nodes. Uses pointers to fastest possible insert/removal of nodes as the linked list grows and shrinks.
	cweeAlloc<cweeLinkedListNode<T, numChildren>, 128>			allocator; // allocator for nodes used in linked list. Allows for safe memory handling and sudden "release" all all nodes used in this linked list.
#pragma endregion

#pragma region "Private Member Functions"
private:
	/*! -- Not intended for public use -- */
	cweeLinkedListNode<T, numChildren>* FindNodeWithIndex(int index) const;
	/*! -- Not intended for public use -- */
	void						InitializeAllocator() {
		allocator.Free(allocator.Alloc());
	};
	/*! -- Not intended for public use -- */
	void						ShutdownAllocator() {
		allocator.Clear();
	};
#pragma endregion

};

#pragma endregion

#pragma region "cweeLinkedListNode definitions"

template <typename T, int numChildren>
INLINE cweeLinkedListNode<T, numChildren>& cweeLinkedListNode<T, numChildren>::operator=(const cweeLinkedListNode<T, numChildren>& other) {
	childrenPerNode = other.childrenPerNode;
	parent = other.parent;
	parentIndex = other.parentIndex;
	prev = other.prev;
	next = other.next;
	_relativeIndex = other._relativeIndex;
	_data = other._data;

	return *this;
};

template <typename T, int numChildren>
INLINE cweeLinkedListNode<T, numChildren>& cweeLinkedListNode<T, numChildren>::operator=(cweeLinkedListNode<T, numChildren>&& other) {
	childrenPerNode = other.childrenPerNode;
	parent = other.parent;
	parentIndex = other.parentIndex;
	prev = other.prev;
	next = other.next;
	_relativeIndex = other._relativeIndex;
	_data = other._data;

	other.parent = nullptr;
	other.parentIndex = 0;
	other.prev = nullptr;
	other.next = nullptr;
	other._relativeIndex = 0;
	other._data.Clear();

	return *this;
};

template <typename T, int numChildren>
INLINE void cweeLinkedListNode<T, numChildren>::Append() {
	// if we are about to extend beyond our allotment, then we need to split.
	if (childrenPerNode < NumRef() + 1) {
		if (next) {
			// split the node
			Split();

			// add the data to our next neighbor instead, as was originally intended
			{
				next->Append();
			}
		}
		else {
			next = parent->AppendNode();
			next->_relativeIndex = _relativeIndex + NumRef();
			next->prev = this;
			next->Append();
		}
	}
	else {
		// add the data 
		{
			_data.Alloc() = T(); // initialize it, just in case
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			IncrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex++;
			}
#endif
		}
	}
};

template <typename T, int numChildren>
INLINE void cweeLinkedListNode<T, numChildren>::Append(const T& obj) {
	// if we are about to extend beyond our allotment, then we need to split.
	if (childrenPerNode < NumRef() + 1) {
		if (next) {
			// split the node
			Split();

			// add the data to our next neighbor instead, as was originally intended
			{
				next->Append(obj);
			}
		}
		else {
			next = parent->AppendNode();
			next->_relativeIndex = _relativeIndex + NumRef();
			next->prev = this;
			next->Append(obj);
		}
	}
	else {
		// add the data 
		{
			_data.Append(obj);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			IncrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex++;
			}
#endif
		}
	}
};

template <typename T, int numChildren>
INLINE void cweeLinkedListNode<T, numChildren>::Insert(const T& obj, int relativeIndex) {
	// if we are about to extend beyond our allotment, then we need to split.
	if (relativeIndex < NumRef()) {
		// if needed, split and try again.
		{
			if (childrenPerNode < NumRef() + 1) {
				Split();
				Insert(obj, relativeIndex);
				return;
			}
		}

		// insert it here 
		{
			_data.Insert(obj, relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			IncrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex++;
			}
#endif
		}
	}
	else {
		// insert it downstream
		if (next) {
			next->Insert(obj, relativeIndex - NumRef());
		}
		else {
			Append(obj);
		}
	}
};

template <typename T, int numChildren>
INLINE bool cweeLinkedListNode<T, numChildren>::Remove(int relativeIndex) {
	if (relativeIndex < 0) {
		return false;
	}

	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndex(relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->Remove(relativeIndex - NumRef());
	}
	return false;
};

template <typename T, int numChildren>
INLINE bool cweeLinkedListNode<T, numChildren>::RemoveFast(int relativeIndex) {
	if (relativeIndex < 0) {
		return false;
	}
#if 0
	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndexFast(relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->RemoveFast(relativeIndex - NumRef());
	}
#else
	if (relativeIndex < NumRef()) {
		// override the object at "relativeIndex" with the value of the VERY LAST OBJECT in the entire dataset. Instead of finding it by navigating the "next" ptrs, use the parents to teleport there. 
		cweeLinkedListNode<T, numChildren>* lastNode = parent->GetNode(parent->NumNodes() - 1); // worst-case, will return this node
		{
			T& lastObj = lastNode->_data[lastNode->_data.NumRef() - 1];
			_data[relativeIndex] = lastObj;
		}
		// delete the very last object in the entire list
		lastNode->Remove(lastNode->NumRef() - 1);

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->RemoveFast(relativeIndex - NumRef());
	}
#endif

	return false;
};

template <typename T, int numChildren>
INLINE bool cweeLinkedListNode<T, numChildren>::Extract(int relativeIndex, T& out) {
	if (relativeIndex < 0) {
		return false;
	}

	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndex(relativeIndex);
			out = _data[relativeIndex];
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->Extract(relativeIndex - NumRef(), out);
	}
	return false;
};

template <typename T, int numChildren>
INLINE bool cweeLinkedListNode<T, numChildren>::ExtractFast(int relativeIndex, T& out) {
	if (relativeIndex < 0) {
		return false;
	}
#if 0
	if (relativeIndex < NumRef()) {
		// remove the data
		{
			_data.RemoveIndexFast(relativeIndex);
		}

		// update the relative index of our downstream neighbors
		{
#ifdef cweeLinkedListUseRecursiveRelativeExploration
			DecrementDownstreamRelativeIndexes(next);
#else
			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->_relativeIndex--;
			}
#endif
		}

		// if this node is now empty, it must be removed.
		{
			if (NumRef() <= 0) {
				if (prev) {
					prev->AbsorbTheRight();
				}
				else {
					if (next) {
						next->prev = nullptr;
						for (cweeLinkedListNode* node = next; node; node = node->next) {
							node->parentIndex--;
						}
						parent->FreeNode(0); // this action should not 'delete' this node right away - the allocator will allow it to live for a moment, which allows this to be legal as its last move.
					}
					else {
						// nothing needs to be done. This node is empty but that's OK as it serves as a Root. 								
					}
				}
			}
		}

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->RemoveFast(relativeIndex - NumRef());
	}
#else
	if (relativeIndex < NumRef()) {
		// override the object at "relativeIndex" with the value of the VERY LAST OBJECT in the entire dataset. Instead of finding it by navigating the "next" ptrs, use the parents to teleport there. 
		cweeLinkedListNode<T, numChildren>* lastNode = parent->GetNode(parent->NumNodes() - 1); // worst-case, will return this node
		{
			T& lastObj = lastNode->_data[lastNode->_data.NumRef() - 1];
			out = _data[relativeIndex];
			_data[relativeIndex] = lastObj;
		}
		// delete the very last object in the entire list
		lastNode->Remove(lastNode->NumRef() - 1);

		return true;
	}
	else if (next) {
		// try the neighbor - this index was too large
		return next->ExtractFast(relativeIndex - NumRef(), out);
	}
#endif

	return false;
};

template <typename T, int numChildren>
INLINE void cweeLinkedListNode<T, numChildren>::AbsorbTheRight() {
	if (next) {
		cweeLinkedListNode* toDelete = next;

		// move the data
		{
			_data.Append(toDelete->_data);

			//for (T& x : toDelete->_data) {
			//	_data.Append(x);
			//}
			//toDelete->_data.Clear();
		}

		// Change the relationships
		{
			next = toDelete->next;
			if (next) {
				next->prev = this;
			}

			for (cweeLinkedListNode* node = next; node; node = node->next) {
				node->parentIndex--;
			}
		}

		// delete the node
		{
			parent->FreeNode(toDelete->parentIndex);
		}
	}
};

template <typename T, int numChildren>
INLINE void cweeLinkedListNode<T, numChildren>::Split() {
	if (parent && NumRef() >= 2) {
		// create a node to my right and change assignments 
		{
			if (next) {
				for (cweeLinkedListNode* node = next; node; node = node->next) {
					node->parentIndex++;
				}

				cweeLinkedListNode* newRightNeighbor = parent->InsertNode(parentIndex + 1);

				newRightNeighbor->next = next;
				newRightNeighbor->prev = this;

				next->prev = newRightNeighbor;
				next = newRightNeighbor;
			}
			else {
				cweeLinkedListNode* newRightNeighbor = parent->AppendNode();

				newRightNeighbor->next = nullptr;
				newRightNeighbor->prev = this;

				next = newRightNeighbor;
			}
		}

		// move half of my data to the node to my right
		{
			int start = NumRef() / 2;
			int end = NumRef();
			for (int i = start; i < end; i++) {
				next->_data.Append(_data[i]);
			}
			_data.Resize(_data.NumRef() - (end - start));
		}

		// update the relative index of the node to my right
		{
			next->_relativeIndex = _relativeIndex + NumRef();
		}
	}
};

template <typename T, int numChildren>
INLINE T& cweeLinkedListNode<T, numChildren>::absoluteIndex(int absoluteIndex) {
	return _data[absoluteIndex - _relativeIndex];
};

template <typename T, int numChildren>
INLINE const T& cweeLinkedListNode<T, numChildren>::absoluteIndex(int absoluteIndex) const {
	return _data[absoluteIndex - _relativeIndex];
};

template <typename T, int numChildren>
INLINE T& cweeLinkedListNode<T, numChildren>::relativeIndex(int relativeIndex) {
	return _data[relativeIndex];
};

template <typename T, int numChildren>
INLINE const T& cweeLinkedListNode<T, numChildren>::relativeIndex(int relativeIndex) const {
	return _data[relativeIndex];
};

template <typename T, int numChildren>
INLINE int cweeLinkedListNode<T, numChildren>::Num() const {
	return _data.Num();
};

template <typename T, int numChildren>
INLINE int cweeLinkedListNode<T, numChildren>::NumRef() const {
	return _data.NumRef();
};

template <typename T, int numChildren>
INLINE int cweeLinkedListNode<T, numChildren>::MinIndex() const {
	return _relativeIndex;
};

template <typename T, int numChildren>
INLINE int cweeLinkedListNode<T, numChildren>::MaxIndex() const {
	return _relativeIndex + NumRef();
};

template <typename T, int numChildren>
INLINE cweeThreadedList<T>& cweeLinkedListNode<T, numChildren>::Data() {
	return _data;
};

template <typename T, int numChildren>
INLINE const cweeThreadedList<T>& cweeLinkedListNode<T, numChildren>::Data() const {
	return _data;
};

template <typename T, int numChildren>
INLINE cweeLinkedListNode<T, numChildren>* cweeLinkedListNode<T, numChildren>::GetNext() {
	return next;
};

template <typename T, int numChildren>
INLINE const cweeLinkedListNode<T, numChildren>* cweeLinkedListNode<T, numChildren>::GetNext() const {
	return next;
};


#pragma endregion

#pragma region "cweeLinkedList definitions"

template <typename T, int numChildren>
INLINE cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::operator=(const cweeLinkedList<T, numChildren>& other) {
	Clear();

	SetGranularity(16 > GetGranularity() ? 16 : GetGranularity());

	for (auto& x : other)
		Append(x);

	return *this;
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::operator==(const T& other) {
	return Find(other);
};

template <typename T, int numChildren>
INLINE cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::Reverse() {
#if 1

	int n = NumRef();
	for (int i = 0; i < n; i++) { this->Append(this->operator[](i)); } // copies all of the data in the list to the end of the list, in order
	for (int i = 0; i < n; i++) { this->RemoveIndexFast(i); } // replaces the objects in the initial list with the opposite end's value
	return *this;

#else	
	cweeThreadedList<T> list = ToList().Reverse();
	int i = 0;
	for (auto& x : *this) {
		x = list[i];
		i++;
	}
	return *this;
#endif
};

template <typename T, int numChildren>
INLINE void						cweeLinkedList<T, numChildren>::Clear() {
	data.Clear();
	allocator.Clear();
	num = 0;
	currentIndex = 0;
};

template <typename T, int numChildren>
INLINE void						cweeLinkedList<T, numChildren>::SetGranularity(int newgranularity) {
	data.SetGranularity(16 > newgranularity / 100 ? 16 : newgranularity / 100);
};				// set new granularity

template <typename T, int numChildren>
INLINE void						cweeLinkedList<T, numChildren>::Reserve(int newgranularity) {
	data.SetGranularity(16 > newgranularity / 100 ? 16 : newgranularity / 100);
};				// set new granularity


template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::GetGranularity() const {
	return data.GetGranularity() * 100;
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::Num() const {
	return num;
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::NumRef() const {
	return num;
};

template <typename T, int numChildren>
INLINE size_t						cweeLinkedList<T, numChildren>::Size() const {
	size_t out = data.Size();
	for (cweeLinkedListNode<T, numChildren>* node : data) {
		out += node->Data().Size();
	}
	return out;
};

template <typename T, int numChildren>
INLINE void						cweeLinkedList<T, numChildren>::Condense() {
	for (cweeLinkedListNode<T, numChildren>* ptr : data) {
		ptr->Data().Condense();
	};

	data.Condense();
};

template <typename T, int numChildren>
INLINE const T& cweeLinkedList<T, numChildren>::operator[](int index) const {
	if (index < num) {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);
		return node->absoluteIndex(index);
	}
	else {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(num - 1);
		return node->absoluteIndex(num - 1);
	}
};

template <typename T, int numChildren>
INLINE T& cweeLinkedList<T, numChildren>::operator[](int index) {
	if (index < num) {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);
		return node->absoluteIndex(index);
	}
	else {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(num - 1);
		return node->absoluteIndex(num - 1);
	}
};

template <typename T, int numChildren>
INLINE cweeThreadedList<T>			cweeLinkedList<T, numChildren>::ToList() const {
	cweeThreadedList<T> out(num + 16);
	for (cweeLinkedListNode<T, numChildren>* node : data) {
		for (auto& x : node->Data()) {
			out.Append(x);
		}
	}
	return out;
};

template <typename T, int numChildren>
INLINE cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::FromList(const cweeThreadedList<T>& inbound) {
	Clear();
	SetGranularity(inbound.GetGranularity() + 16);
	for (auto& x : inbound) {
		Append(x);
	}
	return *this;
};

template <typename T, int numChildren>
INLINE cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::AppendList(const cweeThreadedList<T>& inbound) {
	if (GetGranularity() < inbound.GetGranularity() + 16) SetGranularity(inbound.GetGranularity() + 16);
	for (auto& x : inbound) {
		Append(x);
	}
	return *this;
};

template <typename T, int numChildren>
INLINE T& cweeLinkedList<T, numChildren>::Alloc() {
	return operator[](Append());
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::Append() {
	if (data.NumRef() <= 0) AppendNode();
	data[data.NumRef() - 1]->Append();
	return num++;
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::Append(const T& obj) {
	if (data.NumRef() <= 0) AppendNode();
	data[data.NumRef() - 1]->Append(obj);
	return num++;
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::AddUnique(const T& obj) {
	int i;
	T* ptr = nullptr;
	i = FindIndex(obj, &ptr);
	if (i >= 0) {
		*ptr = obj;
		return i;
	}
	return Append(obj);
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::Insert(const T& obj, int index) {
	if (index >= num)
		return Append(obj);

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);
	node->Insert(obj, index - node->MinIndex());
	return num++;
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::Emplace(const T& obj, int index) {
	while (index >= num) {
		Append(T());
	}
	operator[](index) = obj;
	return index;
};		// insert the element at the given index

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::FindIndex(const T& obj, T** ptr) const {
	int i = 0;
	for (auto& x : *this) {
		if (x == obj) {
			if (ptr) *ptr = const_cast<T*>(&x);
			return i;
		}
		i++;
	}
	return -1;
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::FindIndex(const T* obj, T** ptr) const {
	int i = 0;
	for (auto& x : *this) {
		if (&x == obj) {
			if (ptr) *ptr = const_cast<T*>(&x);
			return i;
		}
		i++;
	}
	return -1;
};


template <typename T, int numChildren>
INLINE T* cweeLinkedList<T, numChildren>::Find(T const& obj) const {
	int i;
	T* ptr = nullptr;
	i = FindIndex(obj, &ptr);
	if (i >= 0) {
		return ptr;
	}
	return ptr;
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::RemoveIndex(int index) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->Remove(index - node->MinIndex())) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
INLINE void						cweeLinkedList<T, numChildren>::RemoveIndexes(const cweeThreadedList<int>& indexes) {
	if (indexes.NumRef() <= 0) return; // nothing to remove.

	std::vector<int> temp = indexes;
	std::sort(temp.begin(), temp.end());
	temp.push_back(-1);

	num -= indexes.NumRef();
	if (num <= 0) {
		Clear();
		return;
	}

	int removalProgress = 1;
	int currentRemoveIndex = temp[0];
	int copyIndex = currentRemoveIndex + 1;
	int nextRemoveIndex = temp[removalProgress];
	int sizeRemove = indexes.NumRef();
	for (int i = currentRemoveIndex; i < num;) {
		if (nextRemoveIndex > copyIndex || nextRemoveIndex == -1) {
			operator[](i) = operator[](copyIndex);
			copyIndex++;
		}
		else { // the next copy location is actually a removal location
			copyIndex++;
			removalProgress++;
			nextRemoveIndex = temp[removalProgress];
			continue;
		}
		i++;
	}

	for (int i = num + indexes.NumRef() - 1; i >= num; i--) {
		cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(i);
		node->Remove(i - node->MinIndex());
	}
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::RemoveIndexFast(int index) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->RemoveFast(index - node->MinIndex())) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::Remove(const T& obj) {
	int f = FindIndex(obj);
	if (f >= 0) return RemoveIndex(f);
	return false;
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::RemoveFast(const T& obj) {
	int f = FindIndex(obj);
	if (f >= 0) return RemoveIndexFast(f);
	return false;
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::Remove(const T* obj) {
	int f = FindIndex(obj);
	if (f >= 0) return RemoveIndex(f);
	return false;
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::RemoveFast(const T* obj) {
	int f = FindIndex(obj);
	if (f >= 0) return RemoveIndexFast(f);
	return false;
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::ExtractIndex(int index, T& out) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->Extract(index - node->MinIndex(), out)) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
INLINE bool						cweeLinkedList<T, numChildren>::ExtractIndexFast(int index, T& out) {
	if ((index < 0) || (index >= num)) {
		return false;
	}

	cweeLinkedListNode<T, numChildren>* node = FindNodeWithIndex(index);

	if (node->ExtractFast(index - node->MinIndex(), out)) {
		num--;
		return true;
	}
	else {
		return false;
	}
};

template <typename T, int numChildren>
INLINE cweeLinkedList<T, numChildren>& cweeLinkedList<T, numChildren>::Sort() {
	auto list = ToList();
	list.Sort();
	FromList(list);
	return *this;
};

template <typename T, int numChildren>
INLINE cweeThreadedList<const T*> cweeLinkedList<T, numChildren>::Select(std::function<bool(const T&)> predicate) const {
	cweeThreadedList<const T*> out;

	for (int i = 0; i < num; ++i) {
		if (predicate(operator[](i))) {
			out.Append(&operator[](i));
		}
	}
	return out;
};

template <typename T, int numChildren>
INLINE cweeThreadedList<T*> cweeLinkedList<T, numChildren>::Select(std::function<bool(const T&)> predicate) {
	cweeThreadedList<T*> out;
	for (int i = 0; i < num; ++i) {
		if (predicate(operator[](i))) {
			out.Append(&operator[](i));
		}
	}
	return out;
};

template <typename T, int numChildren>
INLINE cweeThreadedList<int> cweeLinkedList<T, numChildren>::SelectIndexes(std::function<bool(const T&)> predicate) const {
	cweeThreadedList<int> out;
	for (int i = 0; i < num; ++i) {
		if (predicate(operator[](i))) {
			out.Append(i);
		}
	}
	return out;
};


template <typename T, int numChildren>
INLINE cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::InsertNode(int nodeIndex) {
	cweeLinkedListNode<T, numChildren>* out = allocator.Alloc();

	*out = cweeLinkedListNode<T, numChildren>(this, nodeIndex, childrenPerNode > num / 100 ? childrenPerNode : num / 100);

	data.Insert(out, nodeIndex);

	return out;
};

template <typename T, int numChildren>
INLINE cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::AppendNode() {
	cweeLinkedListNode<T, numChildren>* out = allocator.Alloc();

	*out = cweeLinkedListNode<T, numChildren>(this, data.NumRef(), childrenPerNode > num / 100 ? childrenPerNode : num / 100);

	data.Append(out);

	return out;
};

template <typename T, int numChildren>
INLINE void						cweeLinkedList<T, numChildren>::FreeNode(int nodeIndex) {
	cweeLinkedListNode<T, numChildren>* out = data[nodeIndex];
	data.RemoveIndex(nodeIndex);
	allocator.Free(out);
};

template <typename T, int numChildren>
INLINE int							cweeLinkedList<T, numChildren>::NumNodes() const {
	return data.NumRef();
};

template <typename T, int numChildren>
INLINE cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::GetNode(int index) {
	if (data.Num() > index)
		return data[index];
	return nullptr;
};

template <typename T, int numChildren>
INLINE const cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::GetNode(int index) const {
	if (data.Num() > index)
		return data[index];
	return nullptr;
};

template <typename T, int numChildren>
INLINE cweeLinkedListNode<T, numChildren>* cweeLinkedList<T, numChildren>::FindNodeWithIndex(int index) const {
	// binary search the list
	if (data.NumRef() == 1) return data[0];

	int len, mid, offset, t;
	bool res;
	cweeLinkedListNode<T, numChildren>* sample = nullptr;

#if 0

	len = data.NumRef();
	offset = 0;

	mid = index;
	mid /= numChildren;
	mid = cweeMath::min(mid, len - 1);
	sample = data[mid];

	while (sample && mid >= 0)
	{
		if (sample->MinIndex() <= index) {
			// the index is to the right of (or on top of) this sample's minimum
			if (sample->GetNext() && sample->GetNext()->MinIndex() <= index) {
				// the index is upstream of this sample and is not within this sample
				// move to the right
				offset = mid + 1;
				t = len - offset;
				t = t >> 1;
				if (!t) {
					t = 1;
				}
				mid += t;

			}
			else {
				return sample;
			}
		}
		else {
			// the index is to the left of this sample and is not within this sample 
			// move to the left
			len = mid - 1;
			t = len - offset;
			t = t >> 1;
			if (!t) {
				t = 1;
			}
			mid -= t;
		}
		sample = data[mid];
	}
	return sample;

#else
	// check the currentIndex (optimization)
	if (currentIndex < data.NumRef() && currentIndex >= 0) {
		sample = data[currentIndex];
		len = sample->MinIndex();
		len += sample->NumRef();

		if (len > index) {
			if (index >= sample->MinIndex()) { // user is using the [] operator to explore within the same bucket at the moment
				return sample;
			}
			else if (index >= (sample->MinIndex() - 1)) { // user is navigating backwards down the chain with the [] operator by -1 at a time
				currentIndex--;
#if 0
				return FindNodeWithIndex(index);
#else
				if (currentIndex < data.NumRef() && currentIndex >= 0) {
					sample = data[currentIndex];
					len = sample->MinIndex();
					len += sample->NumRef();

					if (len > index) {
						if (index >= sample->MinIndex()) { // user is using the [] operator to explore within the same bucket at the moment
							return sample;
						}
					}
				}
#endif
			}
		}
		else if ((len + 1) > index) { // user is navigating up the chain with [] operator by 1 at a time
			currentIndex++;
#if 0
			return FindNodeWithIndex(index);
#else
			if (currentIndex < data.NumRef() && currentIndex >= 0) {
				sample = data[currentIndex];
				len = sample->MinIndex();
				len += sample->NumRef();

				if (len > index) {
					if (index >= sample->MinIndex()) { // user is using the [] operator to explore within the same bucket at the moment
						return sample;
					}
				}
			}
#endif
		}
	}

	// use binary search to find the correct bucket
	{

#if 1 // assumes no knowledge and will begin the bisection in the middle of the list
		len = data.NumRef();
		mid = len;
		offset = 0;
		res = false;

		while (mid > 0) {
			mid = len >> 1;
			sample = data[offset + mid];
			if (index >= sample->MinIndex())
			{
				offset += mid;
				len -= mid;
				res = true;
				if (index < sample->MaxIndex()) {
					currentIndex = offset;
					return sample;
				}
			}
			else
			{
				len -= mid;
				res = false;
			}
		}
		currentIndex = offset + (int)res;
		sample = data[currentIndex];
		return sample;

#else
		len = data.NumRef();
		mid = currentIndex;
		offset = 0;

		mid = index;
		mid /= numChildren;
		mid = cweeMath::min(mid, len - 1);
		sample = data[mid];


		while (sample && mid >= 0)
		{
			if (sample->MinIndex() <= index) {
				// the index is to the right of (or on top of) this sample's minimum
				if (sample->GetNext() && sample->GetNext()->MinIndex() <= index) {
					// the index is upstream of this sample and is not within this sample
					offset = mid + 1;
					t = len - offset;
					t = t >> 1;
					if (!t) t++;
					mid += t; // move to the right
				}
				else {
					// we've arrived
					currentIndex = mid;
					return sample;
				}
			}
			else {
				// the index is to the left of this sample and is not within this sample 
				len = mid - 1; // reduce search space for good. 
				t = len - offset;
				t = t >> 1;
				if (!t) t++;
				mid -= t; // move to the left
			}
			sample = data[mid];
		}
		currentIndex = mid;
		return sample;
#endif
	}
#endif

};

#pragma endregion


