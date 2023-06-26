
#ifndef __LIST_H__
#define __LIST_H__

/*
===============================================================================
	List template - Does not allocate memory until the first item is added.
===============================================================================
*/

template< class type >
INLINE int cweeListSortCompare(const type* a, const type* b) {
	return *a - *b;
}

template< class type >
INLINE type* cweeListNewElement(void) {
	return new type;
}

template< class type >
INLINE void listSwap(type& a, type& b) {
	type c = a;
	a = b;
	b = c;
}

template< class type >
class cweeList {
public:

	typedef int		cmp_t(const type*, const type*);
	typedef type	new_t(void);

	cweeList(int newgranularity = 16);
	cweeList(const cweeList<type>& other);
	~cweeList<type>(void);

	void			Clear(void);									// clear the list
	int				Num(void) const;								// returns number of elements in list
	int				NumAllocated(void) const;						// returns number of elements allocated for
	void			SetGranularity(int newgranularity);				// set new granularity
	int				GetGranularity(void) const;						// get the current granularity

	size_t			Allocated(void) const;							// returns total size of allocated memory
	size_t			Size(void) const;								// returns total size of allocated memory including size of list type
	size_t			MemoryUsed(void) const;							// returns size of the used elements in the list

	cweeList<type>& operator=(const cweeList<type>& other);
	const type& operator[](int index) const;
	type& operator[](int index);

	void			Condense(void);									// resizes list to exactly the number of elements it contains
	void			Resize(int newsize);							// resizes list to the given number of elements
	void			Resize(int newsize, int newgranularity);		// resizes list and sets new granularity
	void			SetNum(int newnum, bool resize = true);			// set number of elements in list and resize to exactly this number if necessary
	void			AssureSize(int newSize);						// assure list has given number of elements, but leave them uninitialized
	void			AssureSize(int newSize, const type& initValue);	// assure list has given number of elements and initialize any new elements
	void			AssureSizeAlloc(int newSize, new_t* allocator);	// assure the pointer list has the given number of elements and allocate any new elements

	type*			Ptr(void);										// returns a pointer to the list
	const type*		Ptr(void) const;								// returns a pointer to the list
	type&			Alloc(void);									// returns reference to a new data element at the end of the list
	int				Append(const type& obj);						// append element
	int				Append(const cweeList<type>& other);			// append list
	int				AddUnique(const type& obj);						// add unique element
	int				Insert(const type& obj, int index = 0);			// insert the element at the given index
	int				FindIndex(const type& obj) const;				// find the index for the given element
	type*			Find(type const& obj) const;					// find pointer to the given element
	int				FindNull(void) const;							// find the index for the first NULL pointer in the list
	int				IndexOf(const type* obj) const;					// returns the index for the pointer to an element in the list
	bool			RemoveIndex(int index);							// remove the element at the given index
	bool			Remove(const type& obj);						// remove the element
	void			Sort(cmp_t* compare = (cmp_t*)& cweeListSortCompare<type>);
	void			SortSubSection(int startIndex, int endIndex, cmp_t* compare = (cmp_t*)& cweeListSortCompare<type>);
	void			Swap(cweeList<type>& other);					// swap the contents of the lists
	void			DeleteContents(bool clear);						// delete the contents of the list

private:
	int				num;
	int				size;
	int				granularity;
	type* list;
};

template< class type >
INLINE cweeList<type>::cweeList(int newgranularity) {
	assert(newgranularity > 0);

	list = NULL;
	granularity = newgranularity;
	Clear();
}

template< class type >
INLINE cweeList<type>::cweeList(const cweeList<type>& other) {
	list = NULL;
	*this = other;
}

template< class type >
INLINE cweeList<type>::~cweeList(void) {
	Clear();
}

template< class type >
INLINE void cweeList<type>::Clear(void) {
	if (list) {
		delete[] list;
	}

	list = NULL;
	num = 0;
	size = 0;
}

/*
================
cweeList<type>::DeleteContents

Calls the destructor of all elements in the list.  Conditionally frees up memory used by the list.
Note that this only works on lists containing pointers to objects and will cause a compiler error
if called with non-pointers.  Since the list was not responsible for allocating the object, it has
no information on whether the object still exists or not, so care must be taken to ensure that
the pointers are still valid when this function is called.  Function will set all pointers in the
list to NULL.
================
*/
template< class type >
INLINE void cweeList<type>::DeleteContents(bool clear) {
	int i;

	for (i = 0; i < num; i++) {
		delete list[i];
		list[i] = NULL;
	}

	if (clear) {
		Clear();
	}
	else {
		memset(list, 0, size * sizeof(type));
	}
}

/*
================
cweeList<type>::Allocated

return total memory allocated for the list in bytes, but doesn't take into account additional memory allocated by type
================
*/
template< class type >
INLINE size_t cweeList<type>::Allocated(void) const {
	return size * sizeof(type);
}

/*
================
cweeList<type>::Size

return total size of list in bytes, but doesn't take into account additional memory allocated by type
================
*/
template< class type >
INLINE size_t cweeList<type>::Size(void) const {
	return sizeof(cweeList<type>) + Allocated();
}

/*
================
cweeList<type>::MemoryUsed
================
*/
template< class type >
INLINE size_t cweeList<type>::MemoryUsed(void) const {
	return num * sizeof(*list);
}

/*
================
cweeList<type>::Num

Returns the number of elements currently contained in the list.
Note that this is NOT an indication of the memory allocated.
================
*/
template< class type >
INLINE int cweeList<type>::Num(void) const {
	return num;
}

/*
================
cweeList<type>::NumAllocated

Returns the number of elements currently allocated for.
================
*/
template< class type >
INLINE int cweeList<type>::NumAllocated(void) const {
	return size;
}

/*
================
cweeList<type>::SetNum

Resize to the exact size specified irregardless of granularity
================
*/
template< class type >
INLINE void cweeList<type>::SetNum(int newnum, bool resize) {
	assert(newnum >= 0);
	if (resize || newnum > size) {
		Resize(newnum);
	}
	num = newnum;
}

/*
================
cweeList<type>::SetGranularity

Sets the base size of the array and resizes the array to match.
================
*/
template< class type >
INLINE void cweeList<type>::SetGranularity(int newgranularity) {
	int newsize;

	assert(newgranularity > 0);
	granularity = newgranularity;

	if (list) {
		// resize it to the closest level of granularity
		newsize = num + granularity - 1;
		newsize -= newsize % granularity;
		if (newsize != size) {
			Resize(newsize);
		}
	}
}

/*
================
cweeList<type>::GetGranularity

Get the current granularity.
================
*/
template< class type >
INLINE int cweeList<type>::GetGranularity(void) const {
	return granularity;
}

/*
================
cweeList<type>::Condense

Resizes the array to exactly the number of elements it contains or frees up memory if empty.
================
*/
template< class type >
INLINE void cweeList<type>::Condense(void) {
	if (list) {
		if (num) {
			Resize(num);
		}
		else {
			Clear();
		}
	}
}

/*
================
cweeList<type>::Resize

Allocates memory for the amount of elements requested while keeping the contents intact.
Contents are copied using their = operator so that data is correnctly instantiated.
================
*/
template< class type >
INLINE void cweeList<type>::Resize(int newsize) {
	type* temp;
	int		i;

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

	temp = list;
	size = newsize;
	if (size < num) {
		num = size;
	}

	// copy the old list into our new one
	list = new type[size];
	for (i = 0; i < num; i++) {
		list[i] = temp[i];
	}

	// delete the old list if it exists
	if (temp) {
		delete[] temp;
	}
}

/*
================
cweeList<type>::Resize

Allocates memory for the amount of elements requested while keeping the contents intact.
Contents are copied using their = operator so that data is correnctly instantiated.
================
*/
template< class type >
INLINE void cweeList<type>::Resize(int newsize, int newgranularity) {
	type* temp;
	int		i;

	assert(newsize >= 0);

	assert(newgranularity > 0);
	granularity = newgranularity;

	// free up the list if no data is being reserved
	if (newsize <= 0) {
		Clear();
		return;
	}

	temp = list;
	size = newsize;
	if (size < num) {
		num = size;
	}

	// copy the old list into our new one
	list = new type[size];
	for (i = 0; i < num; i++) {
		list[i] = temp[i];
	}

	// delete the old list if it exists
	if (temp) {
		delete[] temp;
	}
}

/*
================
cweeList<type>::AssureSize

Makes sure the list has at least the given number of elements.
================
*/
template< class type >
INLINE void cweeList<type>::AssureSize(int newSize) {
	int newNum = newSize;

	if (newSize > size) {

		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}

		newSize += granularity - 1;
		newSize -= newSize % granularity;
		Resize(newSize);
	}

	num = newNum;
}

/*
================
cweeList<type>::AssureSize

Makes sure the list has at least the given number of elements and initialize any elements not yet initialized.
================
*/
template< class type >
INLINE void cweeList<type>::AssureSize(int newSize, const type& initValue) {
	int newNum = newSize;

	if (newSize > size) {

		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}

		newSize += granularity - 1;
		newSize -= newSize % granularity;
		num = size;
		Resize(newSize);

		for (int i = num; i < newSize; i++) {
			list[i] = initValue;
		}
	}

	num = newNum;
}

/*
================
cweeList<type>::AssureSizeAlloc

Makes sure the list has at least the given number of elements and allocates any elements using the allocator.

NOTE: This function can only be called on lists containing pointers. Calling it
on non-pointer lists will cause a compiler error.
================
*/
template< class type >
INLINE void cweeList<type>::AssureSizeAlloc(int newSize, new_t* allocator) {
	int newNum = newSize;

	if (newSize > size) {

		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}

		newSize += granularity - 1;
		newSize -= newSize % granularity;
		num = size;
		Resize(newSize);

		for (int i = num; i < newSize; i++) {
			list[i] = (*allocator)();
		}
	}

	num = newNum;
}

/*
================
cweeList<type>::operator=

Copies the contents and size attributes of another list.
================
*/
template< class type >
INLINE cweeList<type>& cweeList<type>::operator=(const cweeList<type>& other) {
	int	i;

	Clear();

	num = other.num;
	size = other.size;
	granularity = other.granularity;

	if (size) {
		list = new type[size];
		for (i = 0; i < num; i++) {
			list[i] = other.list[i];
		}
	}

	return *this;
}

/*
================
cweeList<type>::operator[] const

Access operator.  Index must be within range or an assert will be issued in debug builds.
Release builds do no range checking.
================
*/
template< class type >
INLINE const type& cweeList<type>::operator[](int index) const {
	assert(index >= 0);
	assert(index < num);

	return list[index];
}

/*
================
cweeList<type>::operator[]

Access operator.  Index must be within range or an assert will be issued in debug builds.
Release builds do no range checking.
================
*/
template< class type >
INLINE type& cweeList<type>::operator[](int index) {
	assert(index >= 0);
	assert(index < num);

	return list[index];
}

/*
================
cweeList<type>::Ptr

Returns a pointer to the begining of the array.  Useful for iterating through the list in loops.

Note: may return NULL if the list is empty.

FIXME: Create an iterator template for this kind of thing.
================
*/
template< class type >
INLINE type* cweeList<type>::Ptr(void) {
	return list;
}

/*
================
cweeList<type>::Ptr

Returns a pointer to the begining of the array.  Useful for iterating through the list in loops.

Note: may return NULL if the list is empty.

FIXME: Create an iterator template for this kind of thing.
================
*/
template< class type >
const INLINE type* cweeList<type>::Ptr(void) const {
	return list;
}

/*
================
cweeList<type>::Alloc

Returns a reference to a new data element at the end of the list.
================
*/
template< class type >
INLINE type& cweeList<type>::Alloc(void) {
	if (!list) {
		Resize(granularity);
	}

	if (num == size) {
		Resize(size + granularity);
	}

	return list[num++];
}

/*
================
cweeList<type>::Append

Increases the size of the list by one element and copies the supplied data into it.

Returns the index of the new element.
================
*/
template< class type >
INLINE int cweeList<type>::Append(type const& obj) {
	if (!list) {
		Resize(granularity);
	}

	if (num == size) {
		int newsize;

		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}
		newsize = size + granularity;
		Resize(newsize - newsize % granularity);
	}

	list[num] = obj;
	num++;

	return num - 1;
}


/*
================
cweeList<type>::Insert

Increases the size of the list by at leat one element if necessary
and inserts the supplied data into it.

Returns the index of the new element.
================
*/
template< class type >
INLINE int cweeList<type>::Insert(type const& obj, int index) {
	if (!list) {
		Resize(granularity);
	}

	if (num == size) {
		int newsize;

		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}
		newsize = size + granularity;
		Resize(newsize - newsize % granularity);
	}

	if (index < 0) {
		index = 0;
	}
	else if (index > num) {
		index = num;
	}
	for (int i = num; i > index; --i) {
		list[i] = list[i - 1];
	}
	num++;
	list[index] = obj;
	return index;
}

/*
================
cweeList<type>::Append

adds the other list to this one

Returns the size of the new combined list
================
*/
template< class type >
INLINE int cweeList<type>::Append(const cweeList<type>& other) {
	if (!list) {
		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}
		Resize(granularity);
	}

	int n = other.Num();
	for (int i = 0; i < n; i++) {
		Append(other[i]);
	}

	return Num();
}

/*
================
cweeList<type>::AddUnique

Adds the data to the list if it doesn't already exist.  Returns the index of the data in the list.
================
*/
template< class type >
INLINE int cweeList<type>::AddUnique(type const& obj) {
	int index;

	index = FindIndex(obj);
	if (index < 0) {
		index = Append(obj);
	}

	return index;
}

/*
================
cweeList<type>::FindIndex

Searches for the specified data in the list and returns it's index.  Returns -1 if the data is not found.
================
*/
template< class type >
INLINE int cweeList<type>::FindIndex(type const& obj) const {
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
cweeList<type>::Find

Searches for the specified data in the list and returns it's address. Returns NULL if the data is not found.
================
*/
template< class type >
INLINE type* cweeList<type>::Find(type const& obj) const {
	int i;

	i = FindIndex(obj);
	if (i >= 0) {
		return &list[i];
	}

	return NULL;
}

/*
================
cweeList<type>::FindNull

Searches for a NULL pointer in the list.  Returns -1 if NULL is not found.

NOTE: This function can only be called on lists containing pointers. Calling it
on non-pointer lists will cause a compiler error.
================
*/
template< class type >
INLINE int cweeList<type>::FindNull(void) const {
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
cweeList<type>::IndexOf

Takes a pointer to an element in the list and returns the index of the element.
This is NOT a guarantee that the object is really in the list.
Function will assert in debug builds if pointer is outside the bounds of the list,
but remains silent in release builds.
================
*/
template< class type >
INLINE int cweeList<type>::IndexOf(type const* objptr) const {
	int index;

	index = objptr - list;

	assert(index >= 0);
	assert(index < num);

	return index;
}

/*
================
cweeList<type>::RemoveIndex

Removes the element at the specified index and moves all data following the element down to fill in the gap.
The number of elements in the list is reduced by one.  Returns false if the index is outside the bounds of the list.
Note that the element is not destroyed, so any memory used by it may not be freed until the destruction of the list.
================
*/
template< class type >
INLINE bool cweeList<type>::RemoveIndex(int index) {
	int i;

	assert(list != NULL);
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
================
cweeList<type>::Remove

Removes the element if it is found within the list and moves all data following the element down to fill in the gap.
The number of elements in the list is reduced by one.  Returns false if the data is not found in the list.  Note that
the element is not destroyed, so any memory used by it may not be freed until the destruction of the list.
================
*/
template< class type >
INLINE bool cweeList<type>::Remove(type const& obj) {
	int index;

	index = FindIndex(obj);
	if (index >= 0) {
		return RemoveIndex(index);
	}

	return false;
}

/*
================
cweeList<type>::Sort

Performs a qsort on the list using the supplied comparison function.  Note that the data is merely moved around the
list, so any pointers to data within the list may no longer be valid.
================
*/
template< class type >
INLINE void cweeList<type>::Sort(cmp_t* compare) {
	if (!list) {
		return;
	}
	typedef int cmp_c(const void*, const void*);

	cmp_c* vCompare = (cmp_c*)compare;
	qsort((void*)list, (size_t)num, sizeof(type), vCompare);
}

/*
================
cweeList<type>::SortSubSection

Sorts a subsection of the list.
================
*/
template< class type >
INLINE void cweeList<type>::SortSubSection(int startIndex, int endIndex, cmp_t* compare) {
	if (!list) {
		return;
	}
	if (startIndex < 0) {
		startIndex = 0;
	}
	if (endIndex >= num) {
		endIndex = num - 1;
	}
	if (startIndex >= endIndex) {
		return;
	}
	typedef int cmp_c(const void*, const void*);

	cmp_c* vCompare = (cmp_c*)compare;
	qsort((void*)(&list[startIndex]), (size_t)(endIndex - startIndex + 1), sizeof(type), vCompare);
}

/*
================
cweeList<type>::Swap

Swaps the contents of two lists
================
*/
template< class type >
INLINE void cweeList<type>::Swap(cweeList<type>& other) {
	listSwap(num, other.num);
	listSwap(size, other.size);
	listSwap(granularity, other.granularity);
	listSwap(list, other.list);
}

#endif /* !__LIST_H__ */
