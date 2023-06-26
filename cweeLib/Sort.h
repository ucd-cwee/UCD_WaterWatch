
#ifndef __SORT_H__
#define __SORT_H__

template< typename _type_ >
INLINE void SwapValues(_type_& a, _type_& b) {
	_type_ c = a;
	a = b;
	b = c;
}

template< typename _type_ >
class cweeSort {
public:
	virtual			~cweeSort() {}
	virtual void	Sort(_type_* base, unsigned int num) const = 0;
};

template< typename _type_, typename _derived_ >
class cweeSort_Quick : public cweeSort< _type_ > {
public:
	virtual void Sort(_type_* base, unsigned int num) const {
		if (num <= 0) {
			return;
		}

		const int64 MAX_LEVELS = 128;
		int64 lo[MAX_LEVELS], hi[MAX_LEVELS];

		// 'lo' is the lower index, 'hi' is the upper index of the region of the array that is being sorted.
		lo[0] = 0;
		hi[0] = num - 1;

		for (int64 level = 0; level >= 0; ) {
			int64 i = lo[level];
			int64 j = hi[level];

			// Only use quick-sort when there are 4 or more elements in this region and we are below MAX_LEVELS.
			// Otherwise fall back to an insertion-sort.
			if (((j - i) >= 4) && (level < (MAX_LEVELS - 1))) {

				// Use the center element as the pivot.
				// The median of a multi point sample could be used
				// but simply taking the center works quite well.
				int64 pi = (i + j) / 2;

				// Move the pivot element to the end of the region.
				SwapValues(base[j], base[pi]);

				// Get a reference to the pivot element.
				_type_& pivot = base[j--];

				// Partition the region.
				do {
					while (static_cast<const _derived_*>(this)->Compare(base[i], pivot) < 0) { if (++i >= j) break; }
					while (static_cast<const _derived_*>(this)->Compare(base[j], pivot) > 0) { if (--j <= i) break; }
					if (i >= j) break;
					SwapValues(base[i], base[j]);
				} while (++i < --j);

				// Without these iterations sorting of arrays with many duplicates may
				// become really slow because the partitioning can be very unbalanced.
				// However, these iterations are unnecessary if all elements are unique.
				while (static_cast<const _derived_*>(this)->Compare(base[i], pivot) <= 0 && i < hi[level]) { i++; }
				while (static_cast<const _derived_*>(this)->Compare(base[j], pivot) >= 0 && lo[level] < j) { j--; }

				// Move the pivot element in place.
				SwapValues(pivot, base[i]);

				//assert(level < MAX_LEVELS - 1);
				lo[level + 1] = i;
				hi[level + 1] = hi[level];
				hi[level] = j;
				level++;

			}
			else {
				// Insertion-sort of the remaining elements.
				for (; i < j; j--) {
					int64 m = i;
					for (int64 k = i + 1; k <= j; k++) {
						if (static_cast<const _derived_*>(this)->Compare(base[k], base[m]) > 0) {
							m = k;
						}
					}
					SwapValues(base[m], base[j]);
				}
				level--;
			}
		}
	}
};

template< typename _type_ >
class cweeSort_QuickDefault : public cweeSort_Quick< _type_, cweeSort_QuickDefault< _type_ > > {
public:
	int Compare(const _type_& a, const _type_& b) const { return a - b; }
};

template<>
class cweeSort_QuickDefault< float > : public cweeSort_Quick< float, cweeSort_QuickDefault< float > > {
public:
	int Compare(float a, float b) const {
		if (a < b) {
			return -1;
		}
		if (a > b) {
			return 1;
		}
		return 0;
	}
};

template< typename _type_, typename _derived_ >
class cweeSort_Heap : public cweeSort< _type_ > {
public:
	virtual void Sort(_type_* base, unsigned int num) const {
		for (unsigned int i = num / 2; i > 0; i--) {
			// sift down
			unsigned int parent = i - 1;
			for (unsigned int child = parent * 2 + 1; child < num; child = parent * 2 + 1) {
				if (child + 1 < num && static_cast<const _derived_*>(this)->Compare(base[child + 1], base[child]) > 0) {
					child++;
				}
				if (static_cast<const _derived_*>(this)->Compare(base[child], base[parent]) <= 0) {
					break;
				}
				SwapValues(base[parent], base[child]);
				parent = child;
			}
		}
		// get sorted elements while maintaining heap order
		for (unsigned int i = num - 1; i > 0; i--) {
			SwapValues(base[0], base[i]);
			// sift down
			unsigned int parent = 0;
			for (unsigned int child = parent * 2 + 1; child < i; child = parent * 2 + 1) {
				if (child + 1 < i && static_cast<const _derived_*>(this)->Compare(base[child + 1], base[child]) > 0) {
					child++;
				}
				if (static_cast<const _derived_*>(this)->Compare(base[child], base[parent]) <= 0) {
					break;
				}
				SwapValues(base[parent], base[child]);
				parent = child;
			}
		}
	}
};

template< typename _type_ >
class cweeSort_HeapDefault : public cweeSort_Heap< _type_, cweeSort_HeapDefault< _type_ > > {
public:
	int Compare(const _type_& a, const _type_& b) const { return a - b; }
};

template< typename _type_, typename _derived_ >
class cweeSort_Insertion : public cweeSort< _type_ > {
public:
	virtual void Sort(_type_* base, unsigned int num) const {
		_type_* lo = base;
		_type_* hi = base + (num - 1);
		while (hi > lo) {
			_type_* max = lo;
			for (_type_* p = lo + 1; p <= hi; p++) {
				if (static_cast<const _derived_*>(this)->Compare((*p), (*max)) > 0) {
					max = p;
				}
			}
			SwapValues(*max, *hi);
			hi--;
		}
	}
};

template< typename _type_ >
class cweeSort_InsertionDefault : public cweeSort_Insertion< _type_, cweeSort_InsertionDefault< _type_ > > {
public:
	int Compare(const _type_& a, const _type_& b) const { return a - b; }
};

#endif
