
#ifndef __STATICLIST_H__
#define __STATICLIST_H__

#include "List.h"


/*
===============================================================================

	Static list template
	A non-growing, memset-able list using no memory allocation.

===============================================================================
*/

template<class type, int size>
class cweeStaticList {
public:

	cweeStaticList();
	cweeStaticList(const cweeStaticList<type, size>& other);
	~cweeStaticList<type, size>();

	void				Clear();										// marks the list as empty.  does not deallocate or intialize data.
	int					Num() const;									// returns number of elements in list
	int					Max() const;									// returns the maximum number of elements in the list
	void				SetNum(int newnum);								// set number of elements in list

	
	void				SetNum(int newNum, const type& initValue);		// sets the number of elements in list and initializes any newly allocated elements to the given value

	size_t				Allocated() const;								// returns total size of allocated memory
	size_t				Size() const;									// returns total size of allocated memory including size of list type
	size_t				MemoryUsed() const;								// returns size of the used elements in the list

	const type&			operator[](int index) const;
	type&				operator[](int index);

	type*				Ptr();											// returns a pointer to the list
	const type*			Ptr() const;									// returns a pointer to the list
	type*				Alloc();										// returns reference to a new data element at the end of the list.  returns NULL when full.
	int					Append(const type& obj);						// append element
	int					Append(const cweeStaticList<type, size>& other);	// append list
	int					AddUnique(const type& obj);						// add unique element
	int					Insert(const type& obj, int index = 0);			// insert the element at the given index
	int					FindIndex(const type& obj) const;				// find the index for the given element
	type*				Find(type const& obj) const;					// find pointer to the given element
	int					FindNull() const;								// find the index for the first NULL pointer in the list
	int					IndexOf(const type* obj) const;					// returns the index for the pointer to an element in the list
	bool				RemoveIndex(int index);							// remove the element at the given index
	bool				RemoveIndexFast(int index);						// remove the element at the given index
	bool				Remove(const type& obj);						// remove the element
	void				Swap(cweeStaticList<type, size>& other);			// swap the contents of the lists
	void				DeleteContents(bool clear);						// delete the contents of the list

	void				Sort(const cweeSort<type>& sort = cweeSort_QuickDefault<type>());

private:
	int					num;
	type 				list[size];
	void				Resize(int newsize); // resizes list to the given number of elements
	
};

/*
================
cweeStaticList<type,size>::cweeStaticList()
================
*/
template<class type, int size>
INLINE cweeStaticList<type, size>::cweeStaticList() {
	num = 0;
}

/*
================
cweeStaticList<type,size>::cweeStaticList( const cweeStaticList<type,size> &other )
================
*/
template<class type, int size>
INLINE cweeStaticList<type, size>::cweeStaticList(const cweeStaticList<type, size>& other) {
	*this = other;
}

/*
================
cweeStaticList<type,size>::~cweeStaticList<type,size>
================
*/
template<class type, int size>
INLINE cweeStaticList<type, size>::~cweeStaticList() {
}

/*
================
cweeStaticList<type,size>::Clear

Sets the number of elements in the list to 0.  Assumes that type automatically handles freeing up memory.
================
*/
template<class type, int size>
INLINE void cweeStaticList<type, size>::Clear() {
	num = 0;
}

/*
========================
idList<_type_,_tag_>::Sort

Performs a QuickSort on the list using the supplied sort algorithm.

Note:	The data is merely moved around the list, so any pointers to data within the list may
		no longer be valid.
========================
*/
template< class type, int size >
INLINE void cweeStaticList<type, size>::Sort(const cweeSort<type>& sort) {
	if (list == NULL) {
		return;
	}
	sort.Sort(Ptr(), Num());
}

/*
================
cweeStaticList<type,size>::DeleteContents

Calls the destructor of all elements in the list.  Conditionally frees up memory used by the list.
Note that this only works on lists containing pointers to objects and will cause a compiler error
if called with non-pointers.  Since the list was not responsible for allocating the object, it has
no information on whether the object still exists or not, so care must be taken to ensure that
the pointers are still valid when this function is called.  Function will set all pointers in the
list to NULL.
================
*/
template<class type, int size>
INLINE void cweeStaticList<type, size>::DeleteContents(bool clear) {
	int i;

	for (i = 0; i < num; i++) {
		delete list[i];
		list[i] = NULL;
	}

	if (clear) {
		Clear();
	}
	else {
		memset(list, 0, sizeof(list));
	}
}

/*
================
cweeStaticList<type,size>::Num

Returns the number of elements currently contained in the list.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::Num() const {
	return num;
}

/*
================
cweeStaticList<type,size>::Num

Returns the maximum number of elements in the list.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::Max() const {
	return size;
}

/*
================
cweeStaticList<type>::Allocated
================
*/
template<class type, int size>
INLINE size_t cweeStaticList<type, size>::Allocated() const {
	return size * sizeof(type);
}

/*
================
cweeStaticList<type>::Size
================
*/
template<class type, int size>
INLINE size_t cweeStaticList<type, size>::Size() const {
	return sizeof(cweeStaticList<type, size>) + Allocated();
}

/*
================
cweeStaticList<type,size>::Num
================
*/
template<class type, int size>
INLINE size_t cweeStaticList<type, size>::MemoryUsed() const {
	return num * sizeof(list[0]);
}

/*
================
cweeStaticList<type,size>::SetNum

Set number of elements in list.
================
*/
template<class type, int size>
INLINE void cweeStaticList<type, size>::SetNum(int newnum) {
	assert(newnum >= 0);
	assert(newnum <= size);
	num = newnum;
}

/*
========================
cweeStaticList<_type_,_tag_>::SetNum
========================
*/
template< class type, int size >
INLINE void cweeStaticList<type, size>::SetNum(int newNum, const type& initValue) {
	assert(newNum >= 0);
	newNum = Min(newNum, size);
	assert(newNum <= size);
	for (int i = num; i < newNum; i++) {
		list[i] = initValue;
	}
	num = newNum;
}

/*
================
cweeStaticList<type,size>::operator[] const

Access operator.  Index must be within range or an assert will be issued in debug builds.
Release builds do no range checking.
================
*/
template<class type, int size>
INLINE const type& cweeStaticList<type, size>::operator[](int index) const {
	assert(index >= 0);
	assert(index < num);

	return list[index];
}

/*
================
cweeStaticList<type,size>::operator[]

Access operator.  Index must be within range or an assert will be issued in debug builds.
Release builds do no range checking.
================
*/
template<class type, int size>
INLINE type& cweeStaticList<type, size>::operator[](int index) {
	assert(index >= 0);
	assert(index < num);

	return list[index];
}

/*
================
cweeStaticList<type,size>::Ptr

Returns a pointer to the begining of the array.  Useful for iterating through the list in loops.

Note: may return NULL if the list is empty.

FIXME: Create an iterator template for this kind of thing.
================
*/
template<class type, int size>
INLINE type* cweeStaticList<type, size>::Ptr() {
	return &list[0];
}

/*
================
cweeStaticList<type,size>::Ptr

Returns a pointer to the begining of the array.  Useful for iterating through the list in loops.

Note: may return NULL if the list is empty.

FIXME: Create an iterator template for this kind of thing.
================
*/
template<class type, int size>
INLINE const type* cweeStaticList<type, size>::Ptr() const {
	return &list[0];
}

/*
================
cweeStaticList<type,size>::Alloc

Returns a pointer to a new data element at the end of the list.
================
*/
template<class type, int size>
INLINE type* cweeStaticList<type, size>::Alloc() {
	if (num >= size) {
		return NULL;
	}

	return &list[num++];
}

/*
================
cweeStaticList<type,size>::Append

Increases the size of the list by one element and copies the supplied data into it.

Returns the index of the new element, or -1 when list is full.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::Append(type const& obj) {
	assert(num < size);
	if (num < size) {
		list[num] = obj;
		num++;
		return num - 1;
	}

	return -1;
}


/*
================
cweeStaticList<type,size>::Insert

Increases the size of the list by at leat one element if necessary
and inserts the supplied data into it.

Returns the index of the new element, or -1 when list is full.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::Insert(type const& obj, int index) {
	int i;

	assert(num < size);
	if (num >= size) {
		return -1;
	}

	assert(index >= 0);
	if (index < 0) {
		index = 0;
	}
	else if (index > num) {
		index = num;
	}

	for (i = num; i > index; --i) {
		list[i] = list[i - 1];
	}

	num++;
	list[index] = obj;
	return index;
}

/*
================
cweeStaticList<type,size>::Append

adds the other list to this one

Returns the size of the new combined list
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::Append(const cweeStaticList<type, size>& other) {
	int i;
	int n = other.Num();

	if (num + n > size) {
		n = size - num;
	}
	for (i = 0; i < n; i++) {
		list[i + num] = other.list[i];
	}
	num += n;
	return Num();
}

/*
================
cweeStaticList<type,size>::AddUnique

Adds the data to the list if it doesn't already exist.  Returns the index of the data in the list.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::AddUnique(type const& obj) {
	int index;

	index = FindIndex(obj);
	if (index < 0) {
		index = Append(obj);
	}

	return index;
}

/*
================
cweeStaticList<type,size>::FindIndex

Searches for the specified data in the list and returns it's index.  Returns -1 if the data is not found.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::FindIndex(type const& obj) const {
	int i;

	for (i = 0; i < num; i++) {
		if (list[i] == obj) {
			return i;
		}
	}

	// Not found
	return -1;
}

/*
================
cweeStaticList<type,size>::Find

Searches for the specified data in the list and returns it's address. Returns NULL if the data is not found.
================
*/
template<class type, int size>
INLINE type* cweeStaticList<type, size>::Find(type const& obj) const {
	int i;

	i = FindIndex(obj);
	if (i >= 0) {
		return (type*)& list[i];
	}

	return NULL;
}

/*
================
cweeStaticList<type,size>::FindNull

Searches for a NULL pointer in the list.  Returns -1 if NULL is not found.

NOTE: This function can only be called on lists containing pointers. Calling it
on non-pointer lists will cause a compiler error.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::FindNull() const {
	int i;

	for (i = 0; i < num; i++) {
		if (list[i] == NULL) {
			return i;
		}
	}

	// Not found
	return -1;
}

/*
================
cweeStaticList<type,size>::IndexOf

Takes a pointer to an element in the list and returns the index of the element.
This is NOT a guarantee that the object is really in the list.
Function will assert in debug builds if pointer is outside the bounds of the list,
but remains silent in release builds.
================
*/
template<class type, int size>
INLINE int cweeStaticList<type, size>::IndexOf(type const* objptr) const {
	int index;

	index = objptr - list;

	assert(index >= 0);
	assert(index < num);

	return index;
}

/*
================
cweeStaticList<type,size>::RemoveIndex

Removes the element at the specified index and moves all data following the element down to fill in the gap.
The number of elements in the list is reduced by one.  Returns false if the index is outside the bounds of the list.
Note that the element is not destroyed, so any memory used by it may not be freed until the destruction of the list.
================
*/
template<class type, int size>
INLINE bool cweeStaticList<type, size>::RemoveIndex(int index) {
	int i;

	assert(index >= 0);
	assert(index < num);

	if ((index < 0) || (index >= num)) {
		return false;
	}

	num--;
	for (i = index; i < num; i++) {
		list[i] = list[i + 1];
	}

	return true;
}

/*
========================
idList<_type_,_tag_>::RemoveIndexFast

Removes the element at the specified index and moves the last element into its spot, rather
than moving the whole array down by one. Of course, this doesn't maintain the order of
elements! The number of elements in the list is reduced by one.

return:	bool	- false if the data is not found in the list.

NOTE:	The element is not destroyed, so any memory used by it may not be freed until the
		destruction of the list.
========================
*/
template< typename _type_, int size >
INLINE bool cweeStaticList<_type_, size>::RemoveIndexFast(int index) {

	if ((index < 0) || (index >= num)) {
		return false;
	}

	num--;
	if (index != num) {
		list[index] = list[num];
	}

	return true;
}

/*
================
cweeStaticList<type,size>::Remove

Removes the element if it is found within the list and moves all data following the element down to fill in the gap.
The number of elements in the list is reduced by one.  Returns false if the data is not found in the list.  Note that
the element is not destroyed, so any memory used by it may not be freed until the destruction of the list.
================
*/
template<class type, int size>
INLINE bool cweeStaticList<type, size>::Remove(type const& obj) {
	int index;

	index = FindIndex(obj);
	if (index >= 0) {
		return RemoveIndex(index);
	}

	return false;
}

/*
================
cweeStaticList<type,size>::Swap

Swaps the contents of two lists
================
*/
template<class type, int size>
INLINE void cweeStaticList<type, size>::Swap(cweeStaticList<type, size>& other) {
	cweeStaticList<type, size> temp = *this;
	*this = other;
	other = temp;
}

// debug tool to find uses of idlist that are dynamically growing
// Ideally, most lists on shipping titles will explicitly set their size correctly
// instead of relying on allocate-on-add
void BreakOnListGrowth();
void BreakOnListDefault();

/*
========================
idList<_type_,_tag_>::Resize

Allocates memory for the amount of elements requested while keeping the contents intact.
Contents are copied using their = operator so that data is correctly instantiated.
========================
*/
template< class type, int size >
INLINE void cweeStaticList<type, size>::Resize(int newsize) {

	assert(newsize >= 0);

	// free up the list if no data is being reserved
	if (newsize <= 0) {
		Clear();
		return;
	}

	if (newsize == size) {
		// not changing the size, so just exit
		return;
	}

	assert(newsize < size);
	return;
}




#endif