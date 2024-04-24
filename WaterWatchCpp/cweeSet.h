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
#include "SharedPtr.h"
#include "List.h"
#include "BalancedTree.h"
#include "InterlockedValues.h"
#include "Mutex.h"

/*! Thread-safe list that allows the user to provide objects and a hashing function. The hashing function is called on object insert or on failure to find the requested object. */
template<typename Value, typename hashT> class cweeThreadedSet {
public:
	class cweeThreadedSet_Impl {
	public:
		/*! prevent multi-thread access to the list while alive. Will automatically unlock the list once out of scope. */
		NODISCARD AUTO			Guard(void) const { return lock.Guard(); };
		/*! Prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock. A "Unlock" must be called to re-enable access to the list. */
		void					Lock(void) const { lock.Lock(); };
		/*! Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior. */
		void					Unlock(void) const { lock.Unlock(); };

		/*! Searches for the hash immediately. If not found, will search through each item in the list and re-hash. */
		cweeSharedPtr<Value>	FindOrMake(hashT const& hash)  {
			AUTO f = Find(hash);
			if (!f) {
				f = Alloc();
			}
			return f;
		};
		cweeSharedPtr<Value>	Find(hashT const& hash) const {
			Lock();
			cweeSharedPtr<Value> toReturn = nullptr;
			{
				AUTO ptr = list.NodeFind(hash);
				if (ptr && ptr->object) {
					if (value_hasher(**ptr->object) == hash) {
						// found it and the hash was unchanged
						toReturn = *ptr->object;
					}
				}
			}
			if (!toReturn) {
				cweeBalancedTree< cweeSharedPtr<Value>, hashT, 10> out;
				{
					for (auto& x : list) {
						if (x.object && *x.object) {
							AUTO h = value_hasher(**x.object);
							if (h == hash) toReturn = *x.object;
							out.Add(*x.object, h, true);
						}
					}
				}
				list = out;
			}
			Unlock();
			return toReturn;
		};
		cweeSharedPtr<Value>	FindFast(hashT const& hash) const {
			Lock();
			cweeSharedPtr<Value> toReturn = nullptr;
			{
				AUTO ptr = list.NodeFind(hash);
				if (ptr && ptr->object) {
					//if (value_hasher(**ptr->object) == hash) {
						toReturn = *ptr->object; // found it
					//}
				}
			}
			Unlock();
			return toReturn;
		};

		cweeSharedPtr<Value>	Add(Value&& v) {
			cweeSharedPtr<Value> V_container = make_cwee_shared<Value>(std::forward<Value>(v));
			AUTO h = value_hasher(*V_container);
			AUTO g = Guard();
			list.Add(V_container, h, false);
			return V_container;
		};
		cweeSharedPtr<Value>	Add(Value const& v) {
			cweeSharedPtr<Value> V_container = make_cwee_shared<Value>(v);
			AUTO h = value_hasher(*V_container);
			AUTO g = Guard();
			list.Add(V_container, h, false);
			return V_container;
		};
		cweeSharedPtr<Value>	AddOrReplace(Value&& v) {
			cweeSharedPtr<Value> V_container = make_cwee_shared<Value>(std::forward<Value>(v));
			AUTO h = value_hasher(*V_container);
			AUTO g = Guard();
			list.Add(V_container, h, true);
			return V_container;
		};
		cweeSharedPtr<Value>	AddOrReplace(Value const& v) {
			cweeSharedPtr<Value> V_container = make_cwee_shared<Value>(v);
			AUTO h = value_hasher(*V_container);
			AUTO g = Guard();
			list.Add(V_container, h, true);
			return V_container;
		};

		cweeSharedPtr<Value>	Add(cweeSharedPtr<Value> v) {
			cweeSharedPtr<Value> V_container = v;
			AUTO h = value_hasher(*V_container);
			AUTO g = Guard();
			list.Add(V_container, h, false);
			return V_container;
		};
		cweeSharedPtr<Value>	AddOrReplace(cweeSharedPtr<Value> v) {
			cweeSharedPtr<Value> V_container = v;
			AUTO h = value_hasher(*V_container);
			AUTO g = Guard();
			list.Add(V_container, h, true);
			return V_container;
		};

		cweeSharedPtr<Value>	Alloc() {
			cweeSharedPtr<Value> V_container = make_cwee_shared<Value>();
			AUTO h = value_hasher(*V_container);
			AUTO g = Guard();
			list.Add(V_container, h, false);
			return V_container;
		};
		void					Remove(cweeSharedPtr<Value> v) {
			if (v) {
				AUTO h = value_hasher(*v);
				AUTO g = Guard();
				AUTO n = list.NodeFind(h);
				if (n) {
					list.Remove(n);
				}
			}
		};
		void					Clear() {
			AUTO g = Guard();
			list = cweeBalancedTree< cweeSharedPtr<Value>, hashT, 10>();
		};
		size_t					Num() const {
			AUTO g = Guard(); 
			return list.GetNodeCount();
		};
		cweeThreadedList<hashT>	HashList() const {
			cweeThreadedList<hashT> out(Num() + 1);
			AUTO g = Guard();
			for (auto& x : list) {
				if (x.object && *x.object) {
					out.Append(value_hasher(**x.object));
				}
			}
			return out;
		};
		cweeSharedPtr<Value>	At(size_t idx) const {
			size_t i = 0; 
			AUTO g = Guard();		
			
			for (auto& x : list) {
				if (x.object) {
					if (idx <= i) {
						return *x.object;
					}
					else {
						i++;
					}
				}
			}
			return nullptr;
		};
		cweeSharedPtr<Value>	UnsafeAt(size_t idx) const {
			size_t i = 0;
			for (auto& x : list) {
				if (x.object) {
					if (idx <= i) {
						return *x.object;
					}
					else {
						i++;
					}
				}
			}
			return nullptr;
		};
		long long				IndexOf(hashT v) const {
			long long i = 0; 
			AUTO g = Guard();
			for (auto& x : list) {
				if (x.object) {
					if (value_hasher(**x.object) == v) return i;
					else i++;
				}
			}
			return -1;
		};

		cweeBalancedTree< cweeSharedPtr<Value>, hashT, 10>& UnsafeContainer() const { return list; };
		hashT					Hash(Value const& v) const { return value_hasher(v); };
		cweeThreadedSet_Impl() : lock(), list(), value_hasher() {};
		cweeThreadedSet_Impl(std::function<hashT(Value const&)> f) : lock(), list(), value_hasher(f) {};
		~cweeThreadedSet_Impl() {};

	private:		
		// mutable cweeConstexprLock													lock; /*! Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection */
		mutable cweeSysMutex														lock; /*! Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection */
		mutable cweeBalancedTree< cweeSharedPtr<Value>, hashT, 10>					list; /*! Map between key and heap ptr. */
		mutable std::function<hashT(Value const&)>									value_hasher; /*! Function that can hash a value into a hashT representation */

	};

protected:
	cweeSharedPtr<cweeThreadedSet_Impl> impl;

public:
	struct it_state {
		typename cweeBalancedTree< cweeSharedPtr<Value>, hashT, 10 >::iterator iterNode;
		size_t pos;
		mutable cweeSharedPtr<Value> v;

		inline void begin(const cweeThreadedSet* ref) { 
			pos = 0; 
			AUTO g = ref->Guard();
			iterNode = ref->UnsafeContainer().begin();
		};
		inline void next(const cweeThreadedSet* ref) { pos++; iterNode++; };
		inline void end(const cweeThreadedSet* ref) { pos = ref->Num(); AUTO g = ref->Guard(); iterNode = ref->UnsafeContainer().end(); };
		inline Value& get(cweeThreadedSet* ref) {
			AUTO V = iterNode.state.node;
			if (V) {
				v = *V->object;
			}
			if (!v) {
				v = make_cwee_shared<Value>();
			}
			return *v;
		};
		inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; };
		inline long long distance(const it_state& s) const { return pos - s.pos; };
		inline void prev(const cweeThreadedSet* ref) { pos--; iterNode--; };
		inline const Value& get(const cweeThreadedSet* ref) const {
			AUTO V = iterNode.state.node;
			if (V) {
				v = *V->object;
			}
			if (!v) {
				v = make_cwee_shared<Value>();
			}
			return *v;
		};
	};

#if 0
	using ParentClass = cweeThreadedSet;
	using IterType = Value;
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
	SETUP_STL_ITERATOR(cweeThreadedSet, Value, it_state);
#endif

	cweeThreadedSet() : impl(new cweeThreadedSet_Impl()) {};
	cweeThreadedSet(std::function<hashT(Value const&)> f) : impl(new cweeThreadedSet_Impl(f)) {};
	cweeThreadedSet(cweeThreadedSet const& a) : impl(a.impl) {};
	cweeThreadedSet(cweeThreadedSet&& a) : impl(std::move(a.impl)) { a.impl = nullptr; };
	cweeThreadedSet& operator=(cweeThreadedSet const& a) { impl = a.impl; return *this; };
	cweeThreadedSet& operator=(cweeThreadedSet&& a) { impl = a.impl; a.impl = nullptr; return *this; };
	~cweeThreadedSet() {};

	NODISCARD AUTO			Guard() const { return impl->Guard(); }
	cweeBalancedTree< cweeSharedPtr<Value>, hashT, 10>& UnsafeContainer() const { return impl->UnsafeContainer(); };
	cweeSharedPtr<Value>	FindOrMake(hashT const& hash) { return impl->FindOrMake(hash); };
	cweeSharedPtr<Value>	Find(hashT const& hash) const { return impl->Find(hash); };
	cweeSharedPtr<Value>	FindFast(hashT const& hash) const { return impl->FindFast(hash); };
	cweeSharedPtr<Value>	Add(Value&& v) { return impl->Add(std::forward<Value>(v)); };
	cweeSharedPtr<Value>	Add(Value const& v) { return impl->Add(v); };
	cweeSharedPtr<Value>	AddOrReplace(Value&& v) { return impl->AddOrReplace(std::forward<Value>(v)); };
	cweeSharedPtr<Value>	AddOrReplace(Value const& v) { return impl->AddOrReplace(v); };
	cweeSharedPtr<Value>	Add(cweeSharedPtr<Value> v) { return impl->Add(v); };
	cweeSharedPtr<Value>	AddOrReplace(cweeSharedPtr<Value> v) { return impl->AddOrReplace(v); };
	cweeSharedPtr<Value>	Alloc() { return impl->Alloc(); };
	void					Remove(cweeSharedPtr<Value> v) { impl->Remove(v); };
	void					Clear() { impl->Clear(); };
	size_t					Num() const { return impl->Num(); };
	cweeThreadedList<hashT>	HashList() const { return impl->HashList(); };
	cweeSharedPtr<Value>	At(size_t idx) const { return impl->At(idx); };
	long long				IndexOf(hashT v) const { return impl->IndexOf(v); };
};