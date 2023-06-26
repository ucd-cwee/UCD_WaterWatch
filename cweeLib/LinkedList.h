#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__


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
	SETUP_STL_ITERATOR(cweeLinkedListNode, _type_, it_state);

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
		inline void prev(const cweeParser* ref) { --pos; }
		inline const _type_& get(const _listType_* ref) const {
			if (!node) node = const_cast<cweeLinkedListNode<T, numChildren> * >(ref->GetNode(nodeIndex));
			if (pos < node->MaxIndex()) {
				return node->absoluteIndex(pos);
			}
			nodeIndex++;
			node = const_cast<cweeLinkedListNode<T, numChildren>*>(node->GetNext());
			return get(ref);
		}
	};
	SETUP_STL_ITERATOR(cweeLinkedList, _type_, it_state);
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
	operator					std::vector<T>() {
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
	operator					std::vector<T>() const {
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
		cweeThreadedList<_othertype_> out(cweeMath::max(16, GetGranularity()));
		for (auto& x : *this) {
			out.Append((_othertype_)x);
		}
		return out;
	};
	/*! Cast to cweeThreadedList */
	template< typename _othertype_ > operator cweeThreadedList<_othertype_>() {
		cweeThreadedList<_othertype_> out(cweeMath::max(16, GetGranularity()));
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
	cweeLinkedList&				operator=(const cweeLinkedList& other);
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
	cweeLinkedList&				Reverse();
	/*! Copies this list to a cweeThreadedList. */
	cweeThreadedList<T>			ToList() const;
	/*! Clears this list, and copies the content of 'inbound' to this list. */
	cweeLinkedList&				FromList(const cweeThreadedList<T>& inbound);
	/*! Appends the content of 'inbound' to this list. */
	cweeLinkedList&				AppendList(const cweeThreadedList<T>& inbound);
	/*! Converts the list to a contiguous list, sorts it, and returns to a linked list. Not memory-efficient. */
	cweeLinkedList&				Sort();

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
	T&							Alloc();
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
	/*! Returns a reference to the underlying list object if the provided object is found within the list. Nullptr otherwise. */
	T*							Find(T const& obj) const;

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
	cweeBlockAlloc<cweeLinkedListNode<T, numChildren>, 128>		allocator; // allocator for nodes used in linked list. Allows for safe memory handling and sudden "release" all all nodes used in this linked list.
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
		allocator.Shutdown();
	};
#pragma endregion

};

#pragma endregion

#endif