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

#define SETUP_STL_ITERATOR(ParentClass, IterType, StateType) typedef std::ptrdiff_t difference_type;											\
	typedef size_t size_type; typedef IterType value_type; typedef IterType* pointer; typedef const IterType* const_pointer;					\
	typedef IterType& reference;																												\
	typedef const IterType& const_reference;																									\
	class iterator {					\
	public: const ParentClass* ref;	mutable StateType state;			\
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
	class const_iterator {	\
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
	class reverse_iterator {			\
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
	class const_reverse_iterator {	\
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