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
#include <new>
#include <functional>

#define useMemMoveForThreadedList

template< typename _type_, memTag_t _tag_ = TAG_LIST >
class cweeThreadedList {
public:
	_type_* cweeThreadedListArrayNew(size_t num, bool zeroBuffer) {
		constexpr bool isPod = std::is_pod<_type_>::value;
		_type_* ptr = nullptr;

			if (zeroBuffer) {
#ifdef useMemMoveForThreadedList
				if constexpr (isPod) {
					ptr = (_type_*)operator new[](sizeof(_type_)* num);
					::memset((void*)ptr, 0, sizeof(_type_) * num);
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

		return ptr;
	};
	_type_* cweeThreadedListArrayDelete(_type_* ptr, size_t num) {
		constexpr bool isPod = std::is_pod<_type_>::value;

		// Call the destructors on all the elements

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

		return nullptr;
	};
	_type_* cweeThreadedListArrayResize(_type_* voldptr, size_t oldNum, size_t newNum, bool zeroBuffer) {
		constexpr bool isPod = std::is_pod<_type_>::value;

		_type_* oldptr = voldptr;
		_type_* newptr = nullptr;
		if (newNum > 0) {
			newptr = cweeThreadedListArrayNew((size_t)newNum, zeroBuffer);
			size_t overlap = ::Min(oldNum, newNum); // Min(

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

		return *this;
	};

	bool								operator==(const _type_& other) {
		int sze = Num();
		bool passed = true;
		for (int i = 0; passed && i < sze; i++) {
			passed = (other == list[i]);
		}
		return passed;
	};
	friend bool							operator==(const  cweeThreadedList<_type_, _tag_>& a, const  cweeThreadedList<_type_, _tag_>& other) {
		constexpr bool isPod = std::is_pod<_type_>::value;
		if (a.Num() != other.Num()) return false;
		if (a.Num() == 0) return true;

#ifdef useMemMoveForThreadedList
		int n;
		n = ::memcmp((void*)&a.Ptr()[0], (void*)&other.Ptr()[0], sizeof(_type_) * a.Num());
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
		return !(a == b);
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
#if 1	
		cweeThreadedList<_type_, _tag_> a = *this;
		Clear();
		for (int i = num - 1; i >= 0; i--) {
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
	int				NumAllocated() const { return _size; };	
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

	size_t			Allocated() const{
	return _size * sizeof(_type_);
};									// returns total size of allocated memory
	size_t			Size() const {
		return sizeof(cweeThreadedList< _type_, _tag_ >) + Allocated();
	};										// returns total size of allocated memory including size of list _type_
	size_t			MemoryUsed() const {
		return num * sizeof(*list);
	};									// returns size of the used elements in the list

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

	operator std::vector<_type_>() {
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
	operator std::vector<_type_>() const {
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

		for (int i = 0; i < num; i++) {
			out.push_back((_othertype_)list[i]);
		}
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
	void			ClearedResize(int newsize) {
		Clear();
		Resize(newsize);
		AssureSize(newsize);
	}; 
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
	void			SetNum(int newnum) {
		assert(newnum >= 0);
		if (!_size || newnum > _size) {
			Resize(newnum);
		}
		num = newnum;
	};								// set number of elements in list and resize to exactly this number if needed
	void			AssureSize(int newSize) {
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
	};							// assure list has given number of elements, but leave them uninitialized
	void			AssureSize(int newSize, const _type_& initValue) {
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
	};	// assure list has given number of elements and initialize any new elements
	void			AssureSizeAlloc(int newSize, new_t* allocator) {
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
	};	// assure the pointer list has the given number of elements and allocate any new elements

	_type_*			Ptr() {
		return list;
	};												// returns a pointer to the list
	const _type_*	Ptr() const {
		return list;
	};										// returns a pointer to the list
	_type_&			Alloc() {
		if (!list) {
			Resize(granularity);
		}

		EnsureCapacity();

		return list[num++]; // error handeling from MVS19 suggests to change this from list[num++] to &list[num++]
	};											// returns reference to a new data element at the end of the list
	_type_& Alloc(_type_&& initValue) {
		if (!list) {
			Resize(granularity);
		}

		EnsureCapacity();

		_type_& out = list[num++];
		out = std::forward<_type_>(initValue);

		return out; // error handeling from MVS19 suggests to change this from list[num++] to &list[num++]
	};											// returns reference to a new data element at the end of the list
	int				Append(const _type_& obj) {
		if (!list) {
			Resize(granularity);
		}

		EnsureCapacity();

		list[num] = obj;
		num++;

		return num - 1;	
	};						// append element
	int				Append(_type_&& obj) {
		if (!list) {
			Resize(granularity);
		}

		EnsureCapacity();

		list[num] = std::move(obj);
		num++;

		return num - 1;	
	};						// append element
	int				Append(const cweeThreadedList& other) {
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
	};						// append list
	int				AddUnique(const _type_& obj) {
		int index;

		index = FindIndex(obj);
		if (index < 0) {
			index = Append(obj);
		}

		return index;
	};					// add unique element
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

	int				FindIndex(const _type_& obj) const {
		int i;

		for (i = 0; i < num; i++) {
			if (list[i] == obj) {
				return i;
			}
		}

		// Not found
		return -1;
	};				// find the index for the given element
	_type_* Find(_type_ const& obj) const {
		int i;

		i = FindIndex(obj);
		if (i >= 0) {
			return &list[i];
		}

		return nullptr;
	};					// find pointer to the given element
	int				FindNull() const {
		int i;

		for (i = 0; i < num; i++) {
			if (list[i] == nullptr) {
				return i;
			}
		}

		// Not found
		return -1;
	};									// find the index for the first nullptr pointer in the list
	int				IndexOf(const _type_* obj) const {
		int index;

		index = obj - list;

		assert(index >= 0);
		assert(index < num);

		return index;
	};					// returns the index for the pointer to an element in the list
	bool			RemoveIndex(int index) {
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
	};							// remove the element at the given index
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
	bool			RemoveIndexFast(int index) {

		if ((index < 0) || (index >= num)) {
			return false;
		}

		num--;
		if (index != num) {
			list[index] = list[num];
		}

		return true;
	};
	// remove the element
	bool			Remove(const _type_& obj) {
		int index;

		index = FindIndex(obj);
		if (index >= 0) {
			return RemoveIndex(index);
		}

		return false;
	};
	bool			RemoveFast(const _type_& obj) {
		int index;

		index = FindIndex(obj);
		if (index >= 0) {
			return RemoveIndexFast(index);
		}

		return false;
	};
	// swap the contents of the lists
	void			Swap(cweeThreadedList& other) {
		cweeThreadedList other2 = *this;
		*this = other;
		other = other2;
	};
	// delete the contents of the list
	void			DeleteContents(bool clear = true) {
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
	};

	cweeThreadedList<_type_>& Sort() {
		if (num >= 2) {
			auto* first = &list[0];
			auto* last = first + num;
			std::sort(first, last);	
		}
		return *this;
	};

	cweeThreadedList<_type_>& Sort(std::function<bool(_type_ const&, _type_ const&)> func) {
		if (num >= 2) {
			auto* first = &list[0];
			auto* last = first + num;
			std::sort(first, last, func);
		}
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
		return at(Num() - 1);
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
		RemoveIndex(Num() - 1);
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
	_type_* list;
	unsigned char	memTag; // byte
};
template <typename type> using cweeList = cweeThreadedList<type>;

