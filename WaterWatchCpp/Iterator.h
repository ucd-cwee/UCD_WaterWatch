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

namespace iterator_tpl {

	// Use this define to declare both:
	// - `iterator`
	// - `const_iterator`:
	// As members of your class
#define SETUP_ITERATORS(C, T, S)  \
  SETUP_MUTABLE_ITERATOR(C, T, S) \
  SETUP_CONST_ITERATOR(C, T, S)

// Use this define to declare only `iterator`
#define SETUP_MUTABLE_ITERATOR(C, T, S)              \
  typedef iterator_tpl::iterator<C, T, S> iterator;  \
  iterator begin() { return iterator::begin(this); } \
  iterator end()   { return iterator::end(this);   }

// Use this define to declare only `const_iterator`
#define SETUP_CONST_ITERATOR(C, T, S)                                  \
  typedef iterator_tpl::const_iterator<C, T, S> const_iterator;        \
  const_iterator begin() const { return const_iterator::begin(this); } \
  const_iterator end()   const { return const_iterator::end(this);   }

// S should be the state struct used to forward iteration:
#define SETUP_REVERSE_ITERATORS(C, T, S)                            \
  struct S##_reversed : public S {                                  \
    inline void next (const C* ref) { S::prev(ref); }               \
    inline void prev (const C* ref) { S::next(ref); }               \
    inline void begin(const C* ref) { S::end(ref); S::prev(ref);}   \
    inline void end  (const C* ref) { S::begin(ref); S::prev(ref);} \
  };                                                                \
  SETUP_MUTABLE_RITERATOR(C, T, S)                                  \
  SETUP_CONST_RITERATOR(C, T, S)

#define SETUP_MUTABLE_RITERATOR(C, T, S) \
  typedef iterator_tpl::iterator<C, T, S##_reversed > reverse_iterator; \
  reverse_iterator rbegin() { return reverse_iterator::begin(this); }   \
  reverse_iterator rend()   { return reverse_iterator::end(this); }     \

#define SETUP_CONST_RITERATOR(C, T, S)                                              \
  typedef iterator_tpl::const_iterator<C, T, S##_reversed > const_reverse_iterator; \
  const_reverse_iterator rbegin() const {                                           \
    return const_reverse_iterator::begin(this);                                     \
  }                                                                                 \
  const_reverse_iterator rend() const {                                             \
    return const_reverse_iterator::end(this);                                       \
  }

#define STL_TYPEDEFS(T)                    \
  typedef std::ptrdiff_t difference_type;  \
  typedef size_t size_type;                \
  typedef T value_type;                    \
  typedef T* pointer;                      \
  typedef const T* const_pointer;          \
  typedef T& reference;                    \
  typedef const T& const_reference

// Forward declaration of const_iterator:
	template <class C, typename T, class S>
	struct const_iterator;

	/* * * * * MUTABLE ITERATOR TEMPLATE: * * * * */

	// C - The container type
	// T - The content type
	// S - The state keeping structure
	template <class C, typename T, class S>
	// The non-specialized version is used for T=rvalue:
	struct iterator {
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;

		// Keeps a reference to the container:
		C* ref;

		// User defined struct to describe the iterator state:
		// This struct should provide the functions listed below,
		// however, note that some of them are optional
		S state;

		// Set iterator to next() state:
		void next() { state.next(ref); }
		// Initialize iterator to first state:
		void begin() { state.begin(ref); }
		// Initialize iterator to end state:
		void end() { state.end(ref); }
		// Returns current `value`
		value_type get() const { return const_cast<value_type>(state.get(ref)); }
		// Return true if `state != s`:
		bool cmp(const S& s) const { return state.cmp(s); }

		// Optional function for reverse iteration:
		void prev() { state.prev(ref); }

	public:
		static iterator begin(C* ref) {
			iterator it(ref);
			it.begin();
			return it;
		}
		static iterator end(C* ref) {
			iterator it(ref);
			it.end();
			return it;
		}

	protected:
		iterator(C* ref) : ref(ref) {}

	public:
		// Note: Instances build with this constructor should
		// be used only after copy-assigning from other iterator!
		iterator() {}

	public:
		value_type operator*() const { return const_cast<value_type>(get()); }
		value_type operator->() const { return const_cast<value_type>(get()); }
		iterator& operator++() { next(); return *this; }
		iterator operator++(int) { iterator temp(*this); next(); return temp; }
		iterator& operator--() { prev(); return *this; }
		iterator operator--(int) { iterator temp(*this); prev(); return temp; }
		bool operator!=(const iterator& other) const {
			return ref != other.ref || cmp(other.state);
		}
		bool operator==(const iterator& other) const {
			return !operator!=(other);
		}

		friend struct iterator_tpl::const_iterator<C, T, S>;

		// Comparisons between const and normal iterators:
		bool operator!=(const const_iterator<C, T, S>& other) const {
			return ref != other.ref || cmd(other.state);
		}
		bool operator==(const const_iterator<C, T, S>& other) const {
			return !operator!=(other);
		}
	};

	template <class C, typename T, class S>
	// This specialization is used for iterators to reference types:
	struct iterator<C, T&, S> {
		typedef T value_type;
		typedef T* pointer;
		typedef T& reference;

		// Keeps a reference to the container:
		C* ref;

		// User defined struct to describe the iterator state:
		// This struct should provide the functions listed below,
		// however, note that some of them are optional
		S state;

		// Set iterator to next() state:
		void next() { state.next(ref); }
		// Initialize iterator to first state:
		void begin() { state.begin(ref); }
		// Initialize iterator to end state:
		void end() { state.end(ref); }
		// Returns current `value`
		reference get() const { return const_cast<reference>(state.get(ref)); }
		// Return true if `state != s`:
		bool cmp(const S& s) const { return state.cmp(s); }

		// Optional function for reverse iteration:
		void prev() { state.prev(ref); }

	public:
		static iterator begin(C* ref) {
			iterator it(ref);
			it.begin();
			return it;
		}
		static iterator end(C* ref) {
			iterator it(ref);
			it.end();
			return it;
		}

	protected:
		iterator(C* ref) : ref(ref) {}

	public:
		// Note: Instances build with this constructor should
		// be used only after copy-assigning from other iterator!
		iterator() {}

	public:
		reference const operator*() const { return  const_cast<reference>(get()); }
		pointer const operator->() { return const_cast<pointer>(&get()); }
		iterator& operator++() { next(); return *this; }
		iterator operator++(int) { iterator temp(*this); next(); return temp; }
		iterator& operator--() { prev(); return *this; }
		iterator operator--(int) { iterator temp(*this); prev(); return temp; }
		bool operator!=(const iterator& other) const {
			return ref != other.ref || cmp(other.state);
		}
		bool operator==(const iterator& other) const {
			return !operator!=(other);
		}

		friend struct iterator_tpl::const_iterator<C, T&, S>;

		// Comparisons between const and normal iterators:
		bool operator!=(const const_iterator<C, T&, S>& other) const {
			return ref != other.ref || cmd(other.state);
		}
		bool operator==(const const_iterator<C, T&, S>& other) const {
			return !operator!=(other);
		}
	};

	/* * * * * CONST ITERATOR TEMPLATE: * * * * */

	// C - The container type
	// T - The content type
	// S - The state keeping structure
	template <class C, typename T, class S>
	// The non-specialized version is used for T=rvalue:
	struct const_iterator {
		typedef T value_type;
		typedef const T* pointer;
		typedef const T& reference;

		// Keeps a reference to the container:
		const C* ref;

		// User defined struct to describe the iterator state:
		// This struct should provide the functions listed below,
		// however, note that some of them are optional
		S state;

		// Set iterator to next() state:
		void next() { state.next(ref); }
		// Initialize iterator to first state:
		void begin() { state.begin(ref); }
		// Initialize iterator to end state:
		void end() { state.end(ref); }
		// Returns current `value`
		value_type get() const { return const_cast<value_type>(state.get(ref)); }
		// Return true if `state != s`:
		bool cmp(const S& s) const { return state.cmp(s); }

		// Optional function for reverse iteration:
		void prev() { state.prev(ref); }

	public:
		static const_iterator begin(const C* ref) {
			const_iterator it(ref);
			it.begin();
			return it;
		}
		static const_iterator end(const C* ref) {
			const_iterator it(ref);
			it.end();
			return it;
		}

	protected:
		const_iterator(const C* ref) : ref(ref) {}

	public:
		// Note: Instances build with this constructor should
		// be used only after copy-assigning from other iterator!
		const_iterator() {}

		// To make possible copy-construct non-const iterators:
		const_iterator(const iterator<C, T, S>& other) : ref(other.ref) {
			state = other.state;
		}

	public:
		value_type operator*() const { return const_cast<value_type>(get()); }
		value_type operator->() const { return const_cast<value_type>(get()); }
		const_iterator& operator++() { next(); return *this; }
		const_iterator operator++(int) { const_iterator temp(*this); next(); return temp; }
		const_iterator& operator--() { prev(); return *this; }
		const_iterator operator--(int) { const_iterator temp(*this); prev(); return temp; }
		bool operator!=(const const_iterator& other) const {
			return ref != other.ref || cmp(other.state);
		}
		bool operator==(const const_iterator& other) const {
			return !operator!=(other);
		}
		const_iterator& operator=(const iterator<C, T, S>& other) {
			ref = other.ref;
			state = other.state;
			return *this;
		}

		friend struct iterator_tpl::iterator<C, T, S>;

		// Comparisons between const and normal iterators:
		bool operator!=(const iterator<C, T, S>& other) const {
			return ref != other.ref || cmp(other.state);
		}
		bool operator==(const iterator<C, T, S>& other) const {
			return !operator!=(other);
		}
	};

	// This specialization is used for iterators to reference types:
	template <class C, typename T, class S>
	struct const_iterator<C, T&, S> {
		typedef T value_type;
		typedef const T* pointer;
		typedef const T& reference;

		// Keeps a reference to the container:
		const C* ref;

		// User defined struct to describe the iterator state:
		// This struct should provide the functions listed below,
		// however, note that some of them are optional
		S state;

		// Set iterator to next() state:
		void next() { state.next(ref); }
		// Initialize iterator to first state:
		void begin() { state.begin(ref); }
		// Initialize iterator to end state:
		void end() { state.end(ref); }
		// Returns current `value`
		reference get() const { return const_cast<reference>(state.get(ref)); }
		// Return true if `state != s`:
		bool cmp(const S& s) const { return state.cmp(s); }

		// Optional function for reverse iteration:
		void prev() { state.prev(ref); }

	public:
		static const_iterator begin(const C* ref) {
			const_iterator it(ref);
			it.begin();
			return it;
		}
		static const_iterator end(const C* ref) {
			const_iterator it(ref);
			it.end();
			return it;
		}

	protected:
		const_iterator(const C* ref) : ref(ref) {}

	public:
		// Note: Instances build with this constructor should
		// be used only after copy-assigning from other iterator!
		const_iterator() {}

		// To make possible copy-construct non-const iterators:
		const_iterator(const iterator<C, T&, S>& other) : ref(other.ref) {
			state = other.state;
		}

	public:
		reference operator*() const { return  const_cast<reference>(get()); }
		pointer operator->() const { return const_cast<pointer>(&get()); }
		const_iterator& operator++() { next(); return *this; }
		const_iterator operator++(int) { const_iterator temp(*this); next(); return temp; }
		const_iterator& operator--() { prev(); return *this; }
		const_iterator operator--(int) { const_iterator temp(*this); prev(); return temp; }
		bool operator!=(const const_iterator& other) const {
			return ref != other.ref || cmp(other.state);
		}
		bool operator==(const const_iterator& other) const {
			return !operator!=(other);
		}
		const_iterator& operator=(const iterator<C, T&, S>& other) {
			ref = other.ref;
			state = other.state;
			return *this;
		}

		friend struct iterator_tpl::iterator<C, T&, S>;

		// Comparisons between const and normal iterators:
		bool operator!=(const iterator<C, T&, S>& other) const {
			return ref != other.ref || cmp(other.state);
		}
		bool operator==(const iterator<C, T&, S>& other) const {
			return !operator!=(other);
		}
	};


}

#define SETUP_STL_ITERATOR(ParentClass, IterType, StateType) typedef std::ptrdiff_t difference_type;											\
	typedef size_t size_type; typedef IterType value_type; typedef IterType* pointer; typedef const IterType* const_pointer;					\
	typedef IterType& reference;																												\
	typedef const IterType& const_reference;																									\
	class iterator : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference>	{					\
	public: const ParentClass* ref;	mutable StateType state;																					\
		iterator() : ref(nullptr), state() {};																									\
		iterator(const ParentClass* parent) : ref(parent), state() {};																			\
		iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };									\
		iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };									\
		difference_type operator-(iterator const& other) { return state.distance(other.state); };												\
		iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };							\
		iterator& operator--() { state.prev(ref); return *this; };																				\
		iterator operator--(int) { iterator retval = *this; --(*this); return retval; };														\
		iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };							\
		iterator& operator++() { state.next(ref); return *this; };																				\
		iterator operator++(int) { iterator retval = *this; ++(*this); return retval; };														\
		bool operator==(iterator const& other) const { return !(operator!=(other)); };															\
		bool operator!=(iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };									\
		reference operator*() { return const_cast<reference>(state.get(ref)); };																\
		pointer operator->() { return const_cast<pointer>(&state.get(ref)); };																	\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		iterator& begin() { state.begin(ref); return *this; };																					\
		iterator& end() { state.end(ref); return *this; };																						\
	};																													\
	iterator begin() { return iterator(this).begin(); };																						\
	iterator end() { return iterator(this).end(); };																							\
	class const_iterator : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {	\
	public: const ParentClass* ref;	mutable StateType state;																					\
		const_iterator() : ref(nullptr), state() {};																							\
		const_iterator(const ParentClass* parent) : ref(parent), state() {};																	\
		const_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };							\
		const_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };							\
		difference_type operator-(const_iterator const& other) { return state.distance(other.state); };											\
		const_iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };						\
		const_iterator& operator--() { state.prev(ref); return *this; };																		\
		const_iterator operator--(int) { const_iterator retval = *this; --(*this); return retval; };											\
		const_iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };						\
		const_iterator& operator++() { state.next(ref); return *this; };																		\
		const_iterator operator++(int) { const_iterator retval = *this; ++(*this); return retval; };											\
		bool operator==(const_iterator const& other) const { return !(operator!=(other)); };													\
		bool operator!=(const_iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };							\
		const_reference operator*() { return const_cast<reference>(state.get(ref)); };															\
		const_pointer operator->() { return const_cast<pointer>(&state.get(ref)); };															\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		const_iterator& begin() { state.begin(ref); return *this; };																			\
		const_iterator& end() { state.end(ref); return *this; };																				\
	};																												\
	const_iterator cbegin() const { return const_iterator(this).begin(); };																		\
	const_iterator cend() const { return const_iterator(this).end(); };																			\
	const_iterator begin() const { return cbegin(); };																							\
	const_iterator end() const { return cend(); };																								\
	class reverse_iterator : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, pointer, reference>	{			\
	public: const ParentClass* ref;	mutable StateType state;																					\
		reverse_iterator() : ref(nullptr), state() {};																							\
		reverse_iterator(const ParentClass* parent) : ref(parent), state() {};																	\
		reverse_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };							\
		reverse_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };							\
		difference_type operator-(reverse_iterator const& other) { return state.distance(other.state); };										\
		reverse_iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };					\
		reverse_iterator& operator--() { state.next(ref); return *this; };																		\
		reverse_iterator operator--(int) { reverse_iterator retval = *this; --(*this); return retval; };										\
		reverse_iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };					\
		reverse_iterator& operator++() { state.prev(ref); return *this; };																		\
		reverse_iterator operator++(int) { reverse_iterator retval = *this; ++(*this); return retval; };										\
		bool operator==(reverse_iterator const& other) const { return !(operator!=(other)); };													\
		bool operator!=(reverse_iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };							\
		reference operator*() { return const_cast<reference>(state.get(ref)); };																\
		pointer operator->() { return const_cast<pointer>(&state.get(ref)); };																	\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		reverse_iterator& begin() { state.end(ref); state.prev(ref); return *this; };															\
		reverse_iterator& end() { state.begin(ref); state.prev(ref); return *this; };															\
	};																											\
	reverse_iterator rbegin() { return reverse_iterator(this).begin(); };																		\
	reverse_iterator rend() { return reverse_iterator(this).end(); };																			\
	class const_reverse_iterator : public std::iterator<std::random_access_iterator_tag, value_type, difference_type, const_pointer, const_reference> {	\
	public: const ParentClass* ref;	mutable StateType state;																					\
		const_reverse_iterator() : ref(nullptr), state() {};																					\
		const_reverse_iterator(const ParentClass* parent) : ref(parent), state() {};															\
		const_reverse_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };					\
		const_reverse_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };					\
		difference_type operator-(const_reverse_iterator const& other) { return state.distance(other.state); };									\
		const_reverse_iterator& operator-(difference_type dist) { for (int i = 0; i < dist; i++) state.next(ref); return *this; };				\
		const_reverse_iterator& operator--() { state.next(ref); return *this; };																\
		const_reverse_iterator operator--(int) { const_reverse_iterator retval = *this; --(*this); return retval; };							\
		const_reverse_iterator& operator+(difference_type dist) { for (int i = 0; i < dist; i++) state.prev(ref); return *this; };				\
		const_reverse_iterator& operator++() { state.prev(ref); return *this; };																\
		const_reverse_iterator operator++(int) { const_reverse_iterator retval = *this; ++(*this); return retval; };							\
		bool operator==(const_reverse_iterator const& other) const { return !(operator!=(other)); };											\
		bool operator!=(const_reverse_iterator const& other) const { return (ref != other.ref || state.cmp(other.state)); };					\
		const_reference operator*() { return const_cast<reference>(state.get(ref)); };															\
		const_pointer operator->() { return const_cast<pointer>(&state.get(ref)); };															\
		const_reference operator*() const { return state.get(ref); };																			\
		const_pointer operator->() const { return &state.get(ref); };																			\
		const_reverse_iterator& begin() { state.end(ref); state.prev(ref); return *this; };														\
		const_reverse_iterator& end() { state.begin(ref); state.prev(ref); return *this; };														\
	};																										\
	const_reverse_iterator rbegin() const { return const_reverse_iterator(this).begin(); };														\
	const_reverse_iterator rend() const { return const_reverse_iterator(this).end(); };															\
	const_reverse_iterator crbegin() const { return rbegin(); };																				\
	const_reverse_iterator crend() const { return rend(); };