
#ifndef __THREADED_LIST_H__
#define __THREADED_LIST_H__

#include <new>
#include <functional>

#define useMemMoveForThreadedList

INLINE int cweeStr_compare(const void* a, const void* b)
{
	cweeStr& aR = *((cweeStr*)a);
	cweeStr& bR = *((cweeStr*)b);
	return cweeStr::Cmp(aR, bR);
}
INLINE void cweeStr_swap(cweeStr* a, cweeStr* b)
{
	cweeStr t = *a;
	*a = *b;
	*b = t;
}
INLINE int cweeStr_partition(cweeStr* arr, int low, int high)
{
	auto& pivot = arr[high];    // pivot
	int i = (low - 1);			// Index of smaller element

	for (int j = low; j < high; j++)
	{
		// If current element is smaller than or
		// equal to pivot
		if (cweeStr_compare(&arr[j], &pivot) <= 0)
		{
			i++;    // increment index of smaller element
			cweeStr_swap(&arr[i], &arr[j]);
		}
	}
	cweeStr_swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}
INLINE void cweeStr_quickSort(cweeStr* arr, int low, int high)
{
	if (low < high)
	{
		/* pi is partitioning index, arr[p] is now
		   at right place */
		int pi = cweeStr_partition(arr, low, high);

		// Separately sort elements before
		// partition and after partition
		cweeStr_quickSort(arr, low, pi - 1);
		cweeStr_quickSort(arr, pi + 1, high);
	}
}

INLINE int int_compare(const void* a, const void* b)
{
	int& aR = *((int*)a);
	int& bR = *((int*)b);
	if (aR == bR) return 0;
	return aR > bR ? 1 : -1;
}
INLINE void int_swap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}
INLINE int int_partition(int* arr, int low, int high)
{
	auto& pivot = arr[high];    // pivot
	int i = (low - 1);			// Index of smaller element

	for (int j = low; j < high; j++)
	{
		// If current element is smaller than or
		// equal to pivot
		if (int_compare(&arr[j], &pivot) <= 0)
		{
			i++;    // increment index of smaller element
			int_swap(&arr[i], &arr[j]);
		}
	}
	int_swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}
INLINE void int_quickSort(int* arr, int low, int high)
{
	if (low < high)
	{
		int pi = int_partition(arr, low, high);
		int_quickSort(arr, low, pi - 1);
		int_quickSort(arr, pi + 1, high);
	}
}

INLINE void float_swap(float* const& a, float* const& b, float* const& tempF) {
	*tempF = *a;
	*a = *b;
	*b = *tempF;
};
INLINE int float_partition(float* const& arr, int const& low, int const& high, float* const& tempF, int& i, int& j) {
	i = (low - 1);			// Index of smaller element
	for (j = low; j < high; j++) {
		// If current element is smaller than or equal to pivot
		if (arr[j] <= arr[high]) {
			i++;    // increment index of smaller element
			*tempF = arr[i];
			arr[i] = arr[j];
			arr[j] = *tempF;
		}
	}
	*tempF = arr[i + 1];
	arr[i + 1] = arr[high];
	arr[high] = *tempF;
	return i;
};
template<bool retlow = true> INLINE int float_quickSort_Impl(float* const& arr, int const& low, int const& high, float* const& tempF, int* const& i, int* const& j) {
	float* first(&arr[0]);
	float* last(first + (high+1));
	std::sort(first, last);
	return 0;

	//if (low < high) {
	//	/* pi is partitioning index, arr[p] is now at right place */
	//	float_quickSort_Impl<true>(
	//		arr, 
	//		float_quickSort_Impl<false>(
	//			arr, 
	//			low, 
	//			float_partition(arr, low, high, tempF, *i, *j),// - 1, 
	//			tempF, i, j) + 2,
	//		high, 
	//		tempF, i, j);
	//}
	//if constexpr (retlow) {
	//	return low;
	//}
	//else {
	//	return high;
	//}
};
INLINE void float_quickSort(float* const& arr, int const& low, int const& high) {
	// float temp = 0; int i = 0, j = 0;

	float* first(&arr[0]);
	float* last(first + (high + 1));
	std::sort(first, last);

	// float_quickSort_Impl(arr, low, high, &temp, &i, &j);
};
INLINE void double_quickSort(double* const& arr, int const& low, int const& high) {
	double* first(&arr[0]);
	double* last(first + (high + 1));
	std::sort(first, last);
};
INLINE void u64_quickSort(u64* const& arr, int const& low, int const& high) {
	u64* first(&arr[0]);
	u64* last(first + (high + 1));
	std::sort(first, last);
};

template< typename _type_, memTag_t _tag_ = TAG_LIST >
class cweeThreadedList {
public:
	/*
	========================
	cweeThreadedListArrayNew
	========================
	*/
	_type_* cweeThreadedListArrayNew(size_t num, bool zeroBuffer) {
		constexpr bool isPod = std::is_pod<_type_>::value;
		_type_* ptr = nullptr;
		try {
			if (zeroBuffer) {
#ifdef useMemMoveForThreadedList
				if constexpr (isPod) {
					ptr = (_type_*)operator new[](sizeof(_type_)* num);
					SIMDProcessor->Memset((void*)ptr, 0, sizeof(_type_) * num);
				}
				else
					ptr = (_type_*)Mem_ClearedAlloc((size_t)(sizeof(_type_) * num), _tag_);
#else
				ptr = (_type_*)Mem_ClearedAlloc((size_t)(sizeof(_type_) * num), _tag_);
#endif
			}
			else {
#ifdef useMemMoveForThreadedList
				if constexpr (isPod) {
					ptr = (_type_*)operator new[](sizeof(_type_)* num);
					::memset((void*)ptr, 0, sizeof(_type_) * num);
				}
				else
					ptr = (_type_*)Mem_Alloc((size_t)(sizeof(_type_) * num), _tag_);
#else
				ptr = (_type_*)Mem_Alloc((size_t)(sizeof(_type_) * num), _tag_);
#endif
			}
			if (ptr) {
				if constexpr (!isPod) {
					for (size_t i = 0; i < num; i++) {
						new (&ptr[i]) _type_;
					}
				}
			}
			else {
				// problems! 
				ptr = nullptr;
			}
		}
		catch (...) {
			cweeStackTrace::GetTrace();	// will report to output window 
		}
		return ptr;
	};
	/*
	========================
	cweeThreadedListArrayDelete
	========================
	*/
	_type_* cweeThreadedListArrayDelete(_type_* ptr, size_t num) {
		constexpr bool isPod = std::is_pod<_type_>::value;

		// Call the destructors on all the elements
		try {
#ifdef useMemMoveForThreadedList
			if constexpr (isPod) {
				_type_* todelete = (_type_*)ptr;
				operator delete[](todelete);
				ptr = nullptr;
			}
			else {
				for (size_t i = 0; i < num; i++) {
					((_type_*)ptr)[i].~_type_();
				}
				Mem_Free(ptr);
			}
#else
			for (size_t i = 0; i < num; i++) {
				((_type_*)ptr)[i].~_type_();
			}
			Mem_Free(ptr);
#endif
		}
		catch (...) {
			cweeStackTrace::GetTrace();	// will report to output window 
		}
		return nullptr;
	};
	/*
	========================
	cweeThreadedListArrayResize
	========================
	*/
	_type_* cweeThreadedListArrayResize(_type_* voldptr, size_t oldNum, size_t newNum, bool zeroBuffer) {
		constexpr bool isPod = std::is_pod<_type_>::value;

		_type_* oldptr = voldptr;
		_type_* newptr = nullptr;
		if (newNum > 0) {
			newptr = cweeThreadedListArrayNew((size_t)newNum, zeroBuffer);
			size_t overlap = ::Min(oldNum, newNum); // Min(
			try {
				if (newptr) {
					//#pragma loop(hint_parallel(8))
					if (oldptr) {
#ifdef useMemMoveForThreadedList
						if constexpr (isPod) {
							if (overlap > 0) {
								::memmove((void*)&newptr[0], (void*)&oldptr[0], sizeof(_type_) * overlap);
								operator delete[](oldptr);
							}
							else {
								for (size_t i = 0; i < overlap; i++) {
									newptr[i] = oldptr[i];
								}
								if (oldptr) {
									voldptr = cweeThreadedListArrayDelete(voldptr, oldNum);
								}
							}
						}
						else {
							for (size_t i = 0; i < overlap; i++) {
								newptr[i] = oldptr[i];
							}
						}
#else
						for (size_t i = 0; i < overlap; i++) {
							newptr[i] = oldptr[i];
						}
#endif
					}
					else {
						voldptr = nullptr;
					}
				}
				else {
					newptr = nullptr;
				}
			}
			catch (...) {
				cweeStackTrace::GetTrace();	// will report to output window 
			}
		}
#ifndef useMemMoveForThreadedList
		if (oldptr) oldptr = cweeThreadedListArrayDelete<_type_>(voldptr, oldNum);
#else
		if constexpr (!isPod) {
			if (oldptr) oldptr = cweeThreadedListArrayDelete(voldptr, oldNum);
		}
#endif
		return newptr;
	};

	struct it_state {
		mutable int pos = 0;
		inline void begin(const cweeThreadedList* ref) { pos = 0; }
		inline void next(const cweeThreadedList* ref) { ++pos; }
		inline void end(const cweeThreadedList* ref) { pos = ref->Num(); }
		inline _type_& get(cweeThreadedList* ref) { return ref->list[pos]; }
		inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
		inline long long distance(const it_state& s) const { return pos - s.pos; };
		// Optional to allow operator--() and reverse iterators:
		inline void prev(const cweeThreadedList* ref) { --pos; }
		// Optional to allow `const_iterator`:
		inline const _type_& get(const cweeThreadedList* ref) const { return ref->list[pos]; }
	};
	SETUP_STL_ITERATOR(cweeThreadedList, _type_, it_state);

	/**/		/**/		/**/
	typedef int		cmp_t(const _type_*, const _type_*);
	typedef _type_	new_t();

	constexpr cweeThreadedList() : granularity(16), memTag(_tag_), list(nullptr), _size(0), num(0) {};
	constexpr cweeThreadedList(int newgranularity) : granularity(newgranularity), memTag(_tag_), list(nullptr), _size(0), num(0) {};
	cweeThreadedList(const cweeThreadedList& other) : granularity(16), memTag(_tag_), list(nullptr), _size(0), num(0) {
		*this = other;
	};
	cweeThreadedList(const std::vector<_type_>& input) : granularity(16), memTag(_tag_), list(nullptr), _size(0), num(0) {
		granularity = input.size() + 16;
		memTag = _tag_;
		Clear();
		for (auto& x : input) {
			Append(x);
		}
	};

	template< typename _othertype_, typename = std::enable_if_t<!std::is_same_v<_othertype_, std::decay_t<_type_>>> >
	cweeThreadedList(const std::vector<_othertype_>& input) : granularity(16), memTag(_tag_), list(nullptr), _size(0), num(0) {
		granularity = input.size() + 16;
		memTag = _tag_;
		Clear();
		for (auto& x : input) {
			Append((_type_)x);
		}
	};

	~cweeThreadedList() {
		Clear();
	};
	cweeThreadedList<_type_, _tag_>& operator=(const cweeThreadedList<_type_, _tag_>& other) {
		constexpr bool isPod = std::is_pod<_type_>::value;

		int	i;

		Clear();

		num = other.num;
		_size = other._size;
		granularity = other.granularity;
		memTag = other.memTag;

		if (_size) {
			try {
				list = cweeThreadedListArrayNew((size_t)_size, false);
				if (list && other.list) {

#ifdef useMemMoveForThreadedList
					if constexpr (isPod) {
						if (num > 0) {
							::memcpy((void*)&list[0], (void*)&other.list[0], sizeof(_type_) * num);
						}
					}
					else {
						//#pragma loop(hint_parallel(8))
						for (i = 0; i < num; i++) {
							list[i] = other.list[i];
						}
					}
#else
					//#pragma loop(hint_parallel(8))
					for (i = 0; i < num; i++) {
						list[i] = other.list[i];
					}
#endif

				}
			}
			catch (...) {
				cweeStackTrace::GetTrace();	// will report to output window 
			}
		}

		return *this;
	};

	bool								operator==(const _type_& other);
	friend bool							operator==(const  cweeThreadedList<_type_, _tag_>& a, const  cweeThreadedList<_type_, _tag_>& other) {
		constexpr bool isPod = std::is_pod<_type_>::value;
		if (a.Num() != other.Num()) return false;
		if (a.Num() == 0) return true;

#ifdef useMemMoveForThreadedList
		int n;
		n = ::memcmp((void*)&a.Ptr()[0], (void*)&other.Ptr()[0], sizeof(_type_)*a.Num());
		return (n == 0);
#else	
		int n = a.Num();
		for (int i = 0; i < n; i++) {
			if (!(a[i] == other[i]))
				return false;
		}
		return true;
#endif			
	};
	friend bool							operator!=(const  cweeThreadedList<_type_, _tag_>& a, const  cweeThreadedList<_type_, _tag_>& b) {
		return !(a==b);
	};

	void			Clear() {
		if (list) 
		{
			list = cweeThreadedListArrayDelete(list, _size);
		}
		num = 0;
		_size = 0;
	};											// clear the list
	int				Num() const {
		return num;
	};										// returns number of elements in list
	int				NumRef() const {
		return num;
	};
	cweeThreadedList<_type_, _tag_>& Reverse() {
#if 0	
		cweeThreadedList<_type_, _tag_> a = *this;
		Clear();
		for (int i = num-1; i >= 0; i--) {
			Append(a[i]);
		}
		return *this;
#else
		int n = Num();
		for (int i = 0; i < n; i++) { Append(list[i]); } // copies all of the data in the list to the end of the list
		for (int i = 0; i < n; i++) { RemoveIndexFast(i); }
		return *this;
#endif
	};
	int				NumAllocated() const;								// returns number of elements allocated for
	void			SetGranularity(int newgranularity) {
		int newsize;

		assert(newgranularity > 0);
		granularity = ::Max(4, newgranularity);

		if (list) {
			// resize it to the closest level of granularity
			newsize = num + granularity - 1;
			newsize -= newsize % granularity;
			if (newsize != _size) {
				Resize(newsize);
			}
		}
	};				// set new granularity
	int				GetGranularity() const {
		return granularity;
	};								// get the current granularity

	size_t			Allocated() const;									// returns total size of allocated memory
	size_t			Size() const;										// returns total size of allocated memory including size of list _type_
	size_t			MemoryUsed() const;									// returns size of the used elements in the list

	template< typename _othertype_, typename = std::enable_if_t<!std::is_same_v<_othertype_, std::decay_t<_type_>>> >
	operator cweeThreadedList<_othertype_>() const {
		cweeThreadedList<_othertype_> out; out.SetGranularity(num + 16);
		
		for (int i = 0; i < num; i++)
			out.Append((_othertype_)list[i]);

		return out;
	};
	template< typename _othertype_, typename = std::enable_if_t<!std::is_same_v<_othertype_, std::decay_t<_type_>>> >
	operator cweeThreadedList<_othertype_>() {
		cweeThreadedList<_othertype_> out; out.SetGranularity(num + 16);

		for (int i = 0; i < num; i++)
			out.Append((_othertype_)list[i]);

		return out;
	};

	operator		std::vector<_type_> () {
		constexpr bool isPod = std::is_pod<_type_>::value;
#ifdef useMemMoveForThreadedList
		if constexpr (isPod) {
			std::vector<_type_> out(list, list + num);
			return out;
		}
		else {
			std::vector<_type_> out;
			out.reserve(num);
			for (int i = 0; i < num; ++i) {
				out.push_back(this->operator[](i));
			}
			return out;
		}
#else
		std::vector<_type_> out;
		out.reserve(num);
		for (int i = 0; i < num; ++i) {
			out.push_back(this->operator[](i));
		}
		return out;
#endif
	};
	operator		std::vector<_type_>() const {
		constexpr bool isPod = std::is_pod<_type_>::value;
#ifdef useMemMoveForThreadedList
		if constexpr (isPod) {
			std::vector<_type_> out(list, list + num);
			return out;
		}
		else {
			std::vector<_type_> out;
			out.reserve(num);
			for (int i = 0; i < num; ++i) {
				out.push_back(this->operator[](i));
			}
			return out;
		}
#else
		std::vector<_type_> out;
		out.reserve(num);
		for (int i = 0; i < num; ++i) {
			out.push_back(this->operator[](i));
		}
		return out;
#endif
	};

	template< typename _othertype_, typename = std::enable_if_t<!std::is_same_v<_othertype_, std::decay_t<_type_>>> >
	explicit operator std::vector<_othertype_>() const {
		std::vector<_othertype_> out; out.reserve(num + 16);

		for (int i = 0; i < num; i++)
			out.push_back((_othertype_)list[i]);

		return out;
	};
	template< typename _othertype_, typename = std::enable_if_t<!std::is_same_v<_othertype_, std::decay_t<_type_>>> >
	explicit operator std::vector<_othertype_>() {
		std::vector<_othertype_> out; out.reserve(num + 16);

		for (int i = 0; i < num; i++)
			out.push_back((_othertype_)list[i]);

		return out;
	};


	cweeThreadedList<_type_, _tag_>& operator=(const std::vector<_type_>& input) {
		constexpr bool isPod = std::is_pod<_type_>::value; 
		
		Clear();
		SetGranularity(input.size() + 16);

#ifdef useMemMoveForThreadedList
		if constexpr (isPod) {
			int	i;

			num = input.size();
			_size = num;
			granularity = 16;

			if (_size) {
				list = (_type_*)cweeThreadedListArrayNew((size_t)_size, false);
				if (list && num > 0) {
					::memcpy((void*)&list[0], input.data(), sizeof(_type_) * num);						
				}				
			}
		}
		else {
			for (auto x : input) {
				Append(x);
			}
		}
#else
		for (auto x : input) {
			Append(x);
		}
#endif
		return *this;
	};

	constexpr const _type_& operator[](int index) const {
		if (index < 0 || index >= num) { throw(std::exception("Bad Index")); }
		return list[index];
	};
	constexpr _type_& operator[](int index) {
		if (index < 0 || index >= num) { throw(std::exception("Bad Index")); }
		return list[index];
	};

	/*
================
cweeThreadedList<_type_,_tag_>::Condense

Resizes the array to exactly the number of elements it contains or frees up memory if empty.
================
*/
	void			Condense() {
		if (list) {
			if (num) {
				Resize(num);
			}
			else {
				Clear();
			}
		}
	};											// resizes list to exactly the number of elements it contains
	void			Resize(int newsize) {
		assert(newsize >= 0);

		// free up the list if no data is being reserved
		if (newsize <= 0) {
			Clear();
			return;
		}

		if (newsize == _size) {
			// not changing the size, so just exit
			return;
		}
		
		list = cweeThreadedListArrayResize(list, (size_t)_size, (size_t)newsize, false);
		_size = newsize;
		if (_size < num) {
			num = _size;
		}
	};								// resizes list to the given number of elements
	void			Resize(int newsize, int newgranularity) {
		assert(newsize >= 0);

		assert(newgranularity > 0);
		granularity = newgranularity;

		// free up the list if no data is being reserved
		if (newsize <= 0) {
			Clear();
			return;
		}

		list = cweeThreadedListArrayResize(list, (size_t)_size, (size_t)newsize, false);
		_size = newsize;
		if (_size < num) {
			num = _size;
		}
	};			// resizes list and sets new granularity
	void			SetNum(int newnum);								// set number of elements in list and resize to exactly this number if needed
	void			AssureSize(int newSize);							// assure list has given number of elements, but leave them uninitialized
	void			AssureSize(int newSize, const _type_& initValue);	// assure list has given number of elements and initialize any new elements
	void			AssureSizeAlloc(int newSize, new_t* allocator);	// assure the pointer list has the given number of elements and allocate any new elements

	_type_*			Ptr();												// returns a pointer to the list
	const _type_*	Ptr() const;										// returns a pointer to the list
	_type_&			Alloc();											// returns reference to a new data element at the end of the list
	int				Append(const _type_& obj) {
		try {
			if (!list) {
				Resize(granularity);
			}

			EnsureCapacity();

			list[num] = obj;
			num++;

			return num - 1;
		}
		catch (...) {
			cweeStackTrace::GetTrace();	// will report to output window 
			return 0;
		}
	};						// append element
	int				Append(_type_&& obj) {
		try {
			if (!list) {
				Resize(granularity);
			}

			EnsureCapacity();
	
			list[num] = std::move(obj);
			num++;

			return num - 1;
		}
		catch (...) {
			cweeStackTrace::GetTrace();	// will report to output window 
			return 0;
		}
	};						// append element
	int				Append(const cweeThreadedList& other);						// append list
	int				AddUnique(const _type_& obj);					// add unique element

	void			EnsureCapacity() {
		if (num == _size) {
			int newsize;

			if (granularity == 0) {	// this is a hack to fix our memset classes
				granularity = 16;
			}
			if ((_size + 1) >= (granularity * GRANULARITY_SCALER)) granularity = granularity * GRANULARITY_SCALER;
			newsize = _size + granularity;
			Resize(newsize - newsize % granularity);
		}
	};
	
	/*
	================
	cweeThreadedList<_type_,_tag_>::Insert

	Increases the size of the list by at leat one element if necessary
	and inserts the supplied data into it.

	Returns the index of the new element.
	================
	*/
	int				Insert(const _type_& obj, int index = 0) {
		constexpr bool isPod = std::is_pod<_type_>::value;

		if (!list) {
			Resize(granularity);
		}

		EnsureCapacity();

		if (index < 0) {
			index = 0;
		}
		else if (index > num) {
			index = num;
		}

#ifdef useMemMoveForThreadedList
		if constexpr (isPod) {
			try {
				if (num > index) {
					::memmove((void*)&list[index + 1], (void*)&list[index], sizeof(_type_) * (num - index));
				}
				::memcpy((void*)&list[index], (void*)&obj, sizeof(_type_));
				num++;
				return index;
			}
			catch (...) {
				for (int i = num; i > index; --i) {
					list[i] = list[i - 1];
				}
				num++;
				list[index] = obj;
				return index;
			}
		}
		else {
			for (int i = num; i > index; --i) {
				list[i] = list[i - 1];
			}
			num++;
			list[index] = obj;
			return index;
		}
#else
		for (int i = num; i > index; --i) {
			list[i] = list[i - 1];
		}
		num++;
		list[index] = obj;
		return index;
#endif
	};		// insert the element at the given index

	/*
	================
	cweeThreadedList<_type_,_tag_>::Emplace

	Places the obj at the requested location is possible. The list will grow to meet the emplace need, if greater than the current Num.

	Returns the index of the new element.
	================
	*/
	int				Emplace(const _type_& obj, int index = 0) {
		if (!list) {
			Resize(granularity);
		}

		EnsureCapacity();

		if (index < 0) {
			index = 0;
		}
		else if (index >= num) {
			// grow to fit 	
			while (index >= num) {
				this->Append(_type_());
			}
		}
		list[index] = obj;

		return index;
	};		// insert the element at the given index

	int				FindIndex(const _type_& obj) const;				// find the index for the given element
	_type_*			Find(_type_ const& obj) const;					// find pointer to the given element
	int				FindNull() const;									// find the index for the first nullptr pointer in the list
	int				IndexOf(const _type_* obj) const;					// returns the index for the pointer to an element in the list
	bool			RemoveIndex(int index);							// remove the element at the given index
	void			RemoveIndexes(const cweeThreadedList<int>& indexes) {
#if 0

		if (indexes.Num() <= 0) return; // nothing to remove.
		std::vector<int> temp = indexes;
		std::sort(temp.begin(), temp.end());
		cweeThreadedList<int> temp = removeList;

		int copyIndex = indexes[0] + 1;
		int findIndex;
		num -= indexes.Num();
		if (num <= 0) Clear();

		for (int i = indexes[0]; i < num; i++) {
			findIndex = indexes.FindIndex(i);
			if (findIndex >= 0) {
				indexes.RemoveIndexFast(findIndex);
				for (;;) {
					if (indexes.FindIndex(copyIndex) < 0) {
						indexes.Append(copyIndex);
						list[i] = list[copyIndex];
						copyIndex++;
						break;
					}
					else {
						copyIndex++;
					}
				}
			}
		}

#elif 1

		if (indexes.Num() <= 0) return; // nothing to remove.
		std::vector<int> temp = indexes;		
		std::sort(temp.begin(), temp.end());
		temp.push_back(-1);

		num -= indexes.Num();
		if (num <= 0) Clear();

		int removalProgress = 1;
		int currentRemoveIndex = temp[0];
		int copyIndex = currentRemoveIndex + 1;
		int nextRemoveIndex = temp[removalProgress];
		int sizeRemove = indexes.Num();
		for (int i = currentRemoveIndex; i < num;) {
			if (nextRemoveIndex > copyIndex || nextRemoveIndex == -1) {
				list[i] = list[copyIndex];
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
#else
		int sizeRemove = indexes.Num();

		if (sizeRemove <= 0) return; // nothing to remove.
		else if (sizeRemove == 1) { // only one index to be moved - do it the old fashioned way
			RemoveIndex(indexes[0]);
			return;
		}
		// at least two indexes to be removes

		if ((num - sizeRemove) <= 0) // we were going to clear this list
		{ 
			Clear();
			return;
		}
		int newNum = num - sizeRemove;

		cweeThreadedList<int> indexesC = indexes;

		indexesC.Sort<int, TAG_LIST>();

		int removalProgress = 1;
		int currentRemoveIndex = indexesC[0];
		int copyIndex = currentRemoveIndex + 1;
		int nextRemoveIndex = indexesC[removalProgress];
		bool finished = false;
		for (int i = currentRemoveIndex; i < newNum;) {
			if (nextRemoveIndex > copyIndex || finished) {
				list[i] = list[copyIndex];
				copyIndex++;
			}
			else { // the next copy location is actually a removal location
				copyIndex++;
				removalProgress++;
				if (removalProgress >= sizeRemove) {
					finished = true;
				}
				else {
					nextRemoveIndex = indexesC[removalProgress];
				}				
				continue;
			}
			i++;
		}

		Resize(newNum);
#endif
	};	// remove the elements at the given indexes
	// removes the element at the given index and places the last element into its spot - DOES NOT PRESERVE LIST ORDER
	bool			RemoveIndexFast(int index);
	// remove the element
	bool			Remove(const _type_& obj);						
	bool			RemoveFast(const _type_& obj);
	// swap the contents of the lists
	void			Swap(cweeThreadedList& other) {
		cweeThreadedList other2 = *this;		
		*this = other;
		other = other2;
	};
	// delete the contents of the list
	void			DeleteContents(bool clear = true);				

	template< typename _type_, memTag_t _tag_ > 
	cweeThreadedList<_type_, _tag_>& Sort(void* quickSortMethod = nullptr ) {
		if (num >= 2) {
			if (quickSortMethod) {
				//Condense();
				quickSortMethod(list, 0, num - 1);
			}
			else {
				auto* first = &list[0];
				auto* last = first + num;
				std::sort(first, last);
				// throw std::runtime_error("User must provide an explicit template specialization, or, pass a quickSortMethod that can sort the ptr array.");
			}
		}
		return *this;
	};

	template<> 
	cweeThreadedList<cweeStr, TAG_LIST>& Sort<cweeStr, TAG_LIST>(void* quickSortMethod) {
		if (num >= 2) cweeStr_quickSort(list, 0, num - 1);
		return *this;
	};

	template<>
	cweeThreadedList<int, TAG_LIST>& Sort<int, TAG_LIST>(void* quickSortMethod) {
		if (num >= 2) int_quickSort(list, 0, num - 1);
		return *this;
	};

	template<>
	cweeThreadedList<float, TAG_LIST>& Sort<float, TAG_LIST>(void* quickSortMethod) {
		if (num >= 2) float_quickSort(list, 0, num - 1);
		return *this;
	};

	template<>
	cweeThreadedList<double, TAG_LIST>& Sort<double, TAG_LIST>(void* quickSortMethod) {
		if (num >= 2) double_quickSort(list, 0, num - 1);
		return *this;
	};

	template<>
	cweeThreadedList<u64, TAG_LIST>& Sort<u64, TAG_LIST>(void* quickSortMethod) {
		if (num >= 2) u64_quickSort(list, 0, num - 1);
		return *this;
	};

	//------------------------
	// auto-cast to other cweeThreadedList types with a different memory tag
	//------------------------
	template< memTag_t _t_ >
	operator cweeThreadedList<_type_, _t_>& () {
		return *reinterpret_cast<cweeThreadedList<_type_, _t_>*>(this);
	}

	template< memTag_t _t_>
	operator const cweeThreadedList<_type_, _t_>& () const {
		return *reinterpret_cast<const cweeThreadedList<_type_, _t_>*>(this);
	}

	/* 
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;  
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); }); 
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl; 
	}
	*/
	cweeThreadedList<const _type_*, _tag_> Select(std::function<bool(const _type_&)> predicate) const {
		cweeThreadedList<const _type_*, _tag_> out;
		for (int i = 0; i < num; ++i) {
			if (predicate(list[i])) {
				out.Append(&list[i]);
			}
		}
		return out;
	};

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
	cweeThreadedList<_type_*, _tag_> Select(std::function<bool(const _type_&)> predicate) {
		cweeThreadedList<_type_*, _tag_> out;
		for (int i = 0; i < num; ++i) {
			if (predicate(list[i])) {
				out.Append(&list[i]);
			}
		}
		return out;
	};

	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<int, _tag_> SelectIndexes(std::function<bool(const _type_&)> predicate) const {
		cweeThreadedList<int, _tag_> out;
		for (int i = 0; i < num; ++i) {
			if (predicate(list[i])) {
				out.Append(i);
			}
		}
		return out;
	};

	//------------------------
	// memTag
	//
	// Changing the memTag when the list has an allocated buffer will
	// result in corruption of the memory statistics.
	//------------------------
	memTag_t		GetMemTag() const { return (memTag_t)memTag; };
	void			SetMemTag(memTag_t tag_) { memTag = (unsigned char)tag_; }; // byte

#pragma region "STD VECTOR METHODS"
	// typedef int size_type;
	typedef _type_ T;
	typedef std::ptrdiff_t difference_type;  
	typedef size_t size_type;                
	typedef T value_type;                    
	typedef T* pointer;                      
	typedef const T* const_pointer;          
	typedef T& reference;                    
	typedef const T& const_reference;

	// access the first element
	reference front() {
		return at(0);
	};
	reference back() {
		return at(Num()-1);
	};
	const_reference front() const {
		return at(0);
	};
	const_reference back() const {
		return at(Num() - 1);
	};
	// access specified element with bounds checking
	reference at(size_type pos) {
		return this->operator[](pos);
	};
	// access specified element with bounds checking
	const_reference  at(size_type pos) const {
		return this->operator[](pos);
	};
	// Returns pointer to the underlying array serving as element storage.
	T* data() {
		return list;
	};
	// Returns pointer to the underlying array serving as element storage.
	const T* data() const {
		return list;
	};
	// Checks if the container has no elements, i.e. whether begin() == end().
	bool empty() const {
		return Num() <= 0;
	};
	// Returns the number of elements in the container, i.e. std::distance(begin(), end()).
	size_type size() const {
		return Num();
	};
	// Returns the maximum number of elements the container is able to hold due to system or library implementation limitations, i.e. std::distance(begin(), end()) for the largest container.
	size_type max_size() const {
		return INT_MAX;
	};
	// Increase the capacity of the vector
	void reserve(size_type new_cap) {
		SetGranularity(new_cap);
	};
	// Returns the number of elements that the container has currently allocated space for.
	size_type capacity() const {
		return GetGranularity();
	};
	// Requests the removal of unused capacity.
	void shrink_to_fit() {
		Condense();
	};
	// Erases all elements from the container. After this call, size() returns zero.
	void clear() {
		Clear();
	};
	// inserts value before pos
	iterator insert(iterator pos, const T& value) {
		Insert(value, pos.state.pos);
		return pos;
	};
	// inserts count copies of the value before pos
	void insert(iterator pos, size_type count, const T& value) {
		for (size_type i = 0; i < count; i++) {
			pos = insert(pos, value);
		}
	};
	// inserts elements from range [first, last) before pos.
	template< class InputIt > void insert(iterator pos, InputIt first, InputIt last) {
		for (auto& x = first; x != last; x++) {
			pos = insert(pos, x);
		}
	};
	// Inserts a new element into the container directly before pos.
	template< class... Args > iterator emplace(const_iterator pos, Args&&... args) {		
		std::vector<T> temp;
		temp.emplace_back(std::forward<Args>(args)...);
		Insert(temp[0], pos.state.pos);

		iterator x;
		for (x = this->begin(); x != this->end(); x++) {
			if (x.state.pos == pos.state.pos) {
				return x;
			}
		}
		return x;

	};
	// Removes the element at pos.
	iterator erase(iterator pos) {
		RemoveIndex(pos.state.pos);
		return pos;
	};
	// Removes the elements in the range [first, last).
	iterator erase(iterator first, iterator last) {
		for (auto& x = first; x != last; x++) {
			erase(x);
		}
		return first;
	};
	// The new element is initialized as a copy of value.
	void push_back(const T& value) {
		Append(value);
	};
	// value is moved into the new element.
	void push_back(T&& value) {
		Append(value);
	};
	// Appends a new element to the end of the container.
	template< class... Args > void emplace_back(Args&&... args) {
		std::vector<T> temp;
		temp.emplace_back(std::forward<Args>(args)...);
		Append(temp[0]);
	};
	// Removes the last element of the container.
	void pop_back() {
		RemoveIndex(Num()-1);
	};
	// Resizes the container to contain count elements.
	size_type resize(size_type count) {
		Resize(count);
		return size();
	};
	// Resizes the container to contain count elements.
	size_type resize(size_type count, const T& value) {
		int n = Num();
		Resize(count);
		for (int i = n; i < Num(); i++) {
			this->operator[](i) = value;
		}
		return size();
	};
	// Exchanges the contents of the container with those of other.
	void swap(cweeThreadedList& other) {
		Swap(other);
	};
	// The element exists in the list
	bool contains(const T& value) {
		return FindIndex(value) >= 0;
	};
	iterator find(const T& value) {
		iterator x;
		for (x = this->begin(); x != this->end(); x++) {
			if (x.get() == value) {
				return x;
			}
		}
		return x;
	};
	size_type count(const T& value) {
		size_type i = 0;

		for (auto& x : *this) {
			if (x == value) i++;
		}

		return i;
	};


#pragma endregion










private:
	mutable int		num;
	int				_size;
	int				granularity;
	_type_*			list;
	unsigned char	memTag; // byte
};

/*
================
cweeThreadedList<_type_,_tag_>::DeleteContents

Calls the destructor of all elements in the list -  Conditionally frees up memory used by the list -
Note that this only works on lists containing pointers to objects and will cause a compiler error
if called with non-pointers.  Since the list was not responsible for allocating the object, it has
no information on whether the object still exists or not, so care must be taken to ensure that
the pointers are still valid when this function is called.  Function will set all pointers in the
list to nullptr.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE void cweeThreadedList<_type_, _tag_>::DeleteContents(bool clear) {
	int i;

	for (i = 0; i < num; i++) {
		delete list[i];
		list[i] = nullptr;
	}

	if (clear) {
		Clear();
	}
	else {
		memset(list, 0, size * sizeof(_type_));
	}
}

/*
================
cweeThreadedList<_type_,_tag_>::Allocated

return total memory allocated for the list in bytes, but doesn't take into account additional memory allocated by _type_
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE size_t cweeThreadedList<_type_, _tag_>::Allocated() const {
	return _size * sizeof(_type_);
}

/*
================
cweeThreadedList<_type_,_tag_>::Size

return total size of list in bytes, but doesn't take into account additional memory allocated by _type_
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE size_t cweeThreadedList<_type_, _tag_>::Size() const {
	return sizeof(cweeThreadedList< _type_, _tag_ >) + Allocated();
}

/*
================
cweeThreadedList<_type_,_tag_>::MemoryUsed
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE size_t cweeThreadedList<_type_, _tag_>::MemoryUsed() const {
	return num * sizeof(*list);
}

/*
================
cweeThreadedList<_type_,_tag_>::NumAllocated

Returns the number of elements currently allocated for.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE int cweeThreadedList<_type_, _tag_>::NumAllocated() const {
	return _size;
}

/*
================
cweeThreadedList<_type_,_tag_>::SetNum
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE void cweeThreadedList<_type_, _tag_>::SetNum(int newnum) {
	assert(newnum >= 0);
	if (newnum > _size) {
		Resize(newnum);
	}
	num = newnum;
}

/*
================
cweeThreadedList<_type_,_tag_>::AssureSize

Makes sure the list has at least the given number of elements.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE void cweeThreadedList<_type_, _tag_>::AssureSize(int newSize) {
	int newNum = newSize;

	if (newSize > _size) {

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
cweeThreadedList<_type_,_tag_>::AssureSize

Makes sure the list has at least the given number of elements and initialize any elements not yet initialized.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE void cweeThreadedList<_type_, _tag_>::AssureSize(int newSize, const _type_& initValue) {
	int newNum = newSize;

	if (newSize > _size) {

		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}

		newSize += granularity - 1;
		newSize -= newSize % granularity;
		num = _size;
		Resize(newSize);

//#pragma loop(hint_parallel(8))
		for (int i = num; i < newSize; i++) {
			list[i] = initValue;
		}
	}

	num = newNum;
}

/*
================
cweeThreadedList<_type_,_tag_>::AssureSizeAlloc

Makes sure the list has at least the given number of elements and allocates any elements using the allocator.

NOTE: This function can only be called on lists containing pointers. Calling it
on non-pointer lists will cause a compiler error.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE void cweeThreadedList<_type_, _tag_>::AssureSizeAlloc(int newSize, new_t* allocator) {
	int newNum = newSize;

	if (newSize > _size) {

		if (granularity == 0) {	// this is a hack to fix our memset classes
			granularity = 16;
		}

		newSize += granularity - 1;
		newSize -= newSize % granularity;
		num = _size;
		Resize(newSize);

		for (int i = num; i < newSize; i++) {
			list[i] = (*allocator)();
		}
	}

	num = newNum;
}

/*
================
cweeThreadedList<_type_,_tag_>::operator=

Copies the contents and size attributes of another list
================
*/
//template< typename _type_, memTag_t _tag_ >
//INLINE cweeThreadedList<_type_, _tag_>& cweeThreadedList<_type_, _tag_>::operator=(const cweeThreadedList<_type_, _tag_>& other) {
//	int	i;
//
//	Clear();
//
//	num = other.num;
//	size = other.size;
//	granularity = other.granularity;
//	memTag = other.memTag;
//
//	if (size) {
//		list = (_type_*)cweeThreadedListArrayNew< _type_, _tag_ >((size_t)size, false);
//		for (i = 0; i < num; i++) {
//			list[i] = other.list[i];
//		}
//	}
//
//	return *this;
//}

template< typename _type_, memTag_t _tag_ >
INLINE bool cweeThreadedList<_type_, _tag_>::operator==(const _type_& other) {
	int sze = Num();
	bool passed = true;
	for (int i = 0; passed && i < sze; i++) {
		passed = (other == list[i]);
	}
	return passed;
}

/*
================
cweeThreadedList<_type_,_tag_>::Ptr

Returns a pointer to the begining of the array.  Useful for iterating through the list in loops.

Note: may return nullptr if the list is empty.

FIXME: Create an iterator template for this kind of thing.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE _type_* cweeThreadedList<_type_, _tag_>::Ptr() {
	return list;
}

/*
================
cweeThreadedList<_type_,_tag_>::Ptr

Returns a pointer to the begining of the array.  Useful for iterating through the list in loops.

Note: may return nullptr if the list is empty.

FIXME: Create an iterator template for this kind of thing.
================
*/
template< typename _type_, memTag_t _tag_ >
const INLINE _type_* cweeThreadedList<_type_, _tag_>::Ptr() const {
	return list;
}

/*
================
cweeThreadedList<_type_,_tag_>::Alloc

Returns a reference to a new data element at the end of the list
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE _type_& cweeThreadedList<_type_, _tag_>::Alloc() {
	if (!list) {
		Resize(granularity);
	}

	EnsureCapacity();

	return list[num++]; // error handeling from MVS19 suggests to change this from list[num++] to &list[num++]
}

/*
================
cweeThreadedList<_type_,_tag_>::Append

adds the other list to this one

Returns the size of the new combined list
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE int cweeThreadedList<_type_, _tag_>::Append(const cweeThreadedList< _type_, _tag_ >& other) {
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
cweeThreadedList<_type_,_tag_>::AddUnique

Adds the data to the list if it doesn't already exist.  Returns the index of the data in the list
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE int cweeThreadedList<_type_, _tag_>::AddUnique(_type_ const& obj) {
	int index;

	index = FindIndex(obj);
	if (index < 0) {
		index = Append(obj);
	}

	return index;
}

/*
================
cweeThreadedList<_type_,_tag_>::FindIndex

Searches for the specified data in the list and returns it's index.  Returns -1 if the data is not found.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE int cweeThreadedList<_type_, _tag_>::FindIndex(_type_ const& obj) const {
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
cweeThreadedList<_type_,_tag_>::Find

Searches for the specified data in the list and returns it's address. Returns nullptr if the data is not found.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE _type_* cweeThreadedList<_type_, _tag_>::Find(_type_ const& obj) const {
	int i;

	i = FindIndex(obj);
	if (i >= 0) {
		return &list[i];
	}

	return nullptr;
}

/*
================
cweeThreadedList<_type_,_tag_>::FindNull

Searches for a nullptr pointer in the list - Returns -1 if nullptr is not found.

NOTE: This function can only be called on lists containing pointers. Calling it
on non-pointer lists will cause a compiler error.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE int cweeThreadedList<_type_, _tag_>::FindNull() const {
	int i;

	for (i = 0; i < num; i++) {
		if (list[i] == nullptr) {
			return i;
		}
	}

	// Not found
	return -1;
}

/*
================
cweeThreadedList<_type_,_tag_>::IndexOf

Takes a pointer to an element in the list and returns the index of the element.
This is NOT a guarantee that the object is really in the list
Function will assert in debug builds if pointer is outside the bounds of the list,
but remains silent in release builds.
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE int cweeThreadedList<_type_, _tag_>::IndexOf(_type_ const* objptr) const {
	int index;

	index = objptr - list;

	assert(index >= 0);
	assert(index < num);

	return index;
}

/*
================
cweeThreadedList<_type_,_tag_>::RemoveIndex

Removes the element at the specified index and moves all data following the element down to fill in the gap.
The number of elements in the list is reduced by one.  Returns false if the index is outside the bounds of the list
Note that the element is not destroyed, so any memory used by it may not be freed until the destruction of the list
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE bool cweeThreadedList<_type_, _tag_>::RemoveIndex(int index) {
	constexpr bool isPod = std::is_pod<_type_>::value;

	int i;

	//assert(list != nullptr);
	//assert(index >= 0);
	//assert(index < num);

	if ((index < 0) || (index >= num)) {
		return false;
	}

	num--;

#ifdef useMemMoveForThreadedList
	if constexpr (isPod) {
		if (index < num) {
			::memmove((void*)&list[index], (void*)&list[index + 1], sizeof(_type_) * (num - index));
		}
	}
	else {
		for (i = index; i < num; i++) {
			list[i] = list[i + 1];
		}
	}
#else
	for (i = index; i < num; i++) {
		list[i] = list[i + 1];
	}
#endif
	return true;
}

/*
========================
cweeThreadedList<_type_,_tag_>::RemoveIndexFast

Removes the element at the specified index and moves the last element into its spot, rather
than moving the whole array down by one. Of course, this doesn't maintain the order of
elements! The number of elements in the list is reduced by one.

return:	bool	- false if the data is not found in the list

NOTE:	The element is not destroyed, so any memory used by it may not be freed until the
		destruction of the list
========================
*/
template< typename _type_, memTag_t _tag_ >
INLINE bool cweeThreadedList<_type_, _tag_>::RemoveIndexFast(int index) {

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
cweeThreadedList<_type_,_tag_>::Remove

Removes the element if it is found within the list and moves all data following the element down to fill in the gap.
The number of elements in the list is reduced by one.  Returns false if the data is not found in the list - Note that
the element is not destroyed, so any memory used by it may not be freed until the destruction of the list
================
*/
template< typename _type_, memTag_t _tag_ >
INLINE bool cweeThreadedList<_type_, _tag_>::Remove(_type_ const& obj) {
	int index;

	index = FindIndex(obj);
	if (index >= 0) {
		return RemoveIndex(index);
	}

	return false;
}

/*
================
cweeThreadedList<_type_,_tag_>::RemoveFast

Removes the element if it is found within the list and moves the last element into its spot, rather
than moving the whole array down by one. Of course, this doesn't maintain the order of
elements! The number of elements in the list is reduced by one.

================
*/
template< typename _type_, memTag_t _tag_ >
INLINE bool cweeThreadedList<_type_, _tag_>::RemoveFast(_type_ const& obj) {
	int index;

	index = FindIndex(obj);
	if (index >= 0) {
		return RemoveIndexFast(index);
	}

	return false;
}

/*
========================
cweeThreadedList<_type_,_tag_>::SortWithTemplate

Performs a QuickSort on the list using the supplied sort algorithm.

Note:	The data is merely moved around the list, so any pointers to data within the list may
		no longer be valid.
========================

//template< typename _type_, memTag_t _tag_ >
//INLINE void cweeThreadedList<_type_, _tag_>::SortWithTemplate(const idSort<_type_>& sort) {
//	if (list == nullptr) {
//		return;
//	}
//	sort.Sort(Ptr(), Num());
//}


//================
//cweeThreadedList<_type_,_tag_>::SortSubSection
//
//Sorts a subsection of the list
//================
//*/
//template< typename _type_, memTag_t _tag_ >
//INLINE void cweeThreadedList<_type_,_tag_>::SortSubSection( int startIndex, int endIndex, cmp_t *compare ) {
//	if ( !list ) {
//		return;
//	}
//	if ( startIndex < 0 ) {
//		startIndex = 0;
//	}
//	if ( endIndex >= num ) {
//		endIndex = num - 1;
//	}
//	if ( startIndex >= endIndex ) {
//		return;
//	}
//	typedef int cmp_c(const void *, const void *);
//
//	cmp_c *vCompare = (cmp_c *)compare;
//	qsort( ( void * )( &list[startIndex] ), ( size_t )( endIndex - startIndex + 1 ), sizeof( _type_ ), vCompare );
//}

/*
========================
FindFromGeneric

Finds an item in a list based on any another datatype.  Your _type_ must overload operator()== for the _type_.
If your _type_ is a ptr, use the FindFromGenericPtr function instead.
========================
*/
template< typename _type_, memTag_t _tag_, typename _compare_type_ >
_type_* FindFromGeneric(cweeThreadedList<_type_, _tag_>& list, const _compare_type_& other) {
	for (int i = 0; i < list.Num(); i++) {
		if (list[i] == other) {
			return &list[i];
		}
	}
	return nullptr;
}

/*
========================
FindFromGenericPtr
========================
*/
template< typename _type_, memTag_t _tag_, typename _compare_type_ >
_type_* FindFromGenericPtr(cweeThreadedList<_type_, _tag_>& list, const _compare_type_& other) {
	for (int i = 0; i < list.Num(); i++) {
		if (*list[i] == other) {
			return &list[i];
		}
	}
	return nullptr;
}

#endif /* !__LIST_H__ */
