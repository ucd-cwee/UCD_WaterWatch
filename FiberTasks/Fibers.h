#pragma once

#pragma region Precompiled STL Headers
#pragma warning(disable : 4005)				// macro redefinition
#pragma warning(disable : 4010)				// single-line comment contains line-continuation character
#pragma warning(disable : 4018)				// singed / unsigned mismatch
#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4101)				// unreferenced local variable
#pragma warning(disable : 4127)				// conditional expression is constant
#pragma warning(disable : 4172)				// returning address of local variable or temporary
#pragma warning(disable : 4189)				// local variable is initialized but not referenced
#pragma warning(disable : 4200)				// nonstandard extension used: zero-sized array in struct/union
#pragma warning(disable : 4238)				// nonstandard extension used: class rvalue used as lvalue
#pragma warning(disable : 4239)				// conversion from 'T' to 'T&'
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4251)				// needs to have dll-interface
#pragma warning(disable : 4267)				// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4273)				// inconsistent DLL linkage
#pragma warning(disable : 4293)				// '<<': shift count negative or too big, undefined behavior
#pragma warning(disable : 4297)				// function assumed not to throw but does
#pragma warning(disable : 4302)				// truncation from 'void *' to 'int'
#pragma warning(disable : 4305)				// truncating a literal from double to float
#pragma warning(disable : 4311)				// pointer truncation from 'void *' to 'int'
#pragma warning(disable : 4312)				// conversion from 'int' to 'void*' of greater size
#pragma warning(disable : 4351)             // Squelch warnings on initializing arrays; it is new (good) behavior in C++11.
#pragma warning(disable : 4390)				// ';' empty controlled statement
#pragma warning(disable : 4456)				// declaration hides previous local declaration
#pragma warning(disable : 4458)				// hides class member
#pragma warning(disable : 4459)				// hides global declaration
#pragma warning(disable : 4499)				// 'static': an explicit specialization cannot have a storage class
#pragma warning(disable : 4505)				// unreferenced local function has been removed
#pragma warning(disable : 4595)				// non-member operator new or delete functions may not be declared inline
#pragma warning(disable : 4701)				// potentially uninitialized local variable
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4715)				// not all control paths return a value
#pragma warning(disable : 4996)				// unsafe string operations
#pragma warning(disable : 6011)				// Dereferencing NULL ptr
#pragma warning(disable : 6385)				// Reading invalid data from buf
#pragma warning(disable : 26110)			// Caller failing to hold lock
#pragma warning(disable : 26439)			// This kind of function may not throw
#pragma warning(disable : 26450)			// Arithmetic overflow: using '<<'
#pragma warning(disable : 26451)			// Arithmetic overflow: using '*' on a 4 byte variable and casting to 8 bytes
#pragma warning(disable : 26495)			// uninitialized member variable type 6
#pragma warning(disable : 26498)			// Mark function constexpr if compile-time evaluation is desired
#pragma warning(disable : 26812)			// prefer enum class to enum
#pragma warning(disable : 28182)			// Dereferencing NULL pointer
#pragma warning(disable : 28251)			// Inconsistent annotation for 'new'
#define NOMINMAX
#define CONST_MAX( x, y )			( (x) > (y) ? (x) : (y) )
#define _CRT_FUNCTIONS_REQUIRED 1
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include <functional>
#include <atomic>
#include <thread>
#include <emmintrin.h> // _mm_pause()
#include <chrono>
#include <string>
#include <memory>
#include <algorithm>
#include <deque>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <ShlDisp.h>
#include <mutex>
#include <shared_mutex>
#include <synchapi.h>
#include <handleapi.h>
#include <ppl.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_set.h>
#include <boost/any.hpp>
#include <cstdint>
#include <type_traits>
#include <execution>
#include <optional>
#include <future>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

#pragma endregion
#pragma region iterator_definition
#ifdef SETUP_STL_ITERATOR
#else
#define SETUP_STL_ITERATOR(ParentClass, IterType, StateType)  typedef std::ptrdiff_t difference_type;											 \
	typedef size_t size_type; typedef IterType value_type; typedef IterType* pointer; typedef const IterType* const_pointer;					 \
	typedef IterType& reference;																												 \
	typedef const IterType& const_reference;																									 \
	class iterator  : public std::iterator<std::random_access_iterator_tag, value_type> {					 \
	public: ParentClass* ref;	mutable StateType state;			 \
        using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;  \
		iterator() : ref(nullptr), state() {};																									 \
		iterator(ParentClass* parent) : ref(parent), state() {};																			 \
		inline iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };									 \
		inline iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };									 \
        inline value_type& operator*() { return state.get(ref); }  \
        inline value_type* operator->() { return &state.get(ref); }  \
        inline const value_type& operator*() const { return state.get(ref); }  \
        inline const value_type* operator->() const { return &state.get(ref); }  \
        inline value_type& operator[](difference_type rhs) const { return *(*this + rhs); }  \
        inline iterator& operator++() { state.next(ref); return *this; };  \
        inline iterator& operator--() { state.prev(ref); return *this; };  \
		inline iterator operator++(int) { iterator retval = *this; ++(*this); return retval; };  \
		inline iterator operator--(int) { iterator retval = *this; --(*this); return retval; };  \
        inline difference_type operator-(iterator const& other) const { return state.distance(other.state); };  \
		inline iterator operator+(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };	 \
        inline iterator operator-(difference_type dist) const { iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };	 \
		friend inline iterator operator+(difference_type lhs, const iterator& rhs) { return rhs + lhs; }  \
		friend inline iterator operator-(difference_type lhs_pos, const iterator& rhs) { iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++){ ++retval; } return retval; }  \
		inline bool operator==(const iterator& other) const { return !(operator!=(other)); }  \
		inline bool operator!=(const iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }  \
		inline bool operator>(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }  \
		inline bool operator<(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos;  }  \
		inline bool operator>=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos;  }  \
		inline bool operator<=(const iterator& rhs) const { iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos;  }  \
		iterator& begin() { state.begin(ref); return *this; };  \
		iterator& end() { state.end(ref); return *this; };  \
	}; \
	iterator begin() { return iterator(this).begin(); };																						 \
	iterator end() { return iterator(this).end(); }; \
    \
	class const_iterator  : public std::iterator<std::random_access_iterator_tag, value_type> {					 \
	public: const ParentClass* ref;	mutable StateType state;			 \
        using difference_type = typename std::iterator<std::random_access_iterator_tag, value_type>::difference_type;  \
		const_iterator() : ref(nullptr), state() {};																									 \
		const_iterator(const ParentClass* parent) : ref(parent), state() {};																			 \
		inline const_iterator& operator+=(difference_type n) { for (int i = 0; i < n; i++) state.next(ref); return *this; };									 \
		inline const_iterator& operator-=(difference_type n) { for (int i = 0; i < n; i++) state.prev(ref); return *this; };									 \
        inline const value_type& operator*() const { return state.get(ref); }  \
        inline const value_type* operator->() const { return &state.get(ref); }  \
        inline const value_type& operator[](difference_type rhs) const { return *(*this + rhs); }  \
        inline const_iterator& operator++() { state.next(ref); return *this; };  \
        inline const_iterator& operator--() { state.prev(ref); return *this; };  \
		inline const_iterator operator++(int) { const_iterator retval = *this; ++(*this); return retval; };  \
		inline const_iterator operator--(int) { const_iterator retval = *this; --(*this); return retval; };  \
        inline difference_type operator-(const_iterator const& other) const { return state.distance(other.state); };  \
		inline const_iterator operator+(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) ++retval; return retval; };	 \
        inline const_iterator operator-(difference_type dist) const { const_iterator retval = *this; for (int i = 0; i < dist; i++) --retval; return retval; };	 \
		friend inline const_iterator operator+(difference_type lhs, const const_iterator& rhs) { return rhs + lhs; }  \
		friend inline const_iterator operator-(difference_type lhs_pos, const const_iterator& rhs) { const_iterator retval = rhs; auto rhs_pos = rhs - retval.begin(); auto newPos = lhs_pos - rhs_pos; retval = retval.begin(); for (auto x = 0; x < newPos; x++){ ++retval; } return retval; }  \
		inline bool operator==(const const_iterator& other) const { return !(operator!=(other)); }  \
		inline bool operator!=(const const_iterator& other) const { return (ref != other.ref || state.cmp(other.state)); }  \
		inline bool operator>(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos > rhs_pos; }  \
		inline bool operator<(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos < rhs_pos;  }  \
		inline bool operator>=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos >= rhs_pos;  }  \
		inline bool operator<=(const const_iterator& rhs) const { const_iterator beginning = *this; beginning.begin(); auto lhs_pos = *this - beginning; auto rhs_pos = rhs - beginning; return lhs_pos <= rhs_pos;  }  \
		const_iterator& begin() { state.begin(ref); return *this; };  \
		const_iterator& end() { state.end(ref); return *this; };  \
	}; \
	const_iterator cbegin() const { return const_iterator(this).begin(); };																		 \
	const_iterator cend() const { return const_iterator(this).end(); };																			 \
	const_iterator begin() const { return cbegin(); };																							 \
	const_iterator end() const { return cend(); };
#endif
#pragma endregion 

#include "Actions.h"
#include "Concurrent_Queue.h"
#include "../WaterWatchCpp/enum.h"
#include <set>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <assert.h>
#include <thread>
#include <iostream>
#include <memory>
#include <atomic>
#include <cstdarg>
#include <iostream>
#include <array>
#include <cstring>
#include <string>
#include <sstream>
#include <mutex>
#include <map>
#include <type_traits>
#include "MultiWord_CompareAndSwap.h"

namespace fibers {
	namespace containers {
		/* Fiber- and thread-safe vector. Objects are stored and returned as std::shared_ptr. Growth, iterations, and push_back operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire vector. */
		template<typename _Ty>
		class vector {
		protected:
			using underlying = typename concurrency::concurrent_vector<std::shared_ptr<_Ty>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const vector* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const vector* ref) { ++pos; }
				inline void end(const vector* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(vector* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const vector* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const vector* ref) const { return *pos; }
			};

			SETUP_STL_ITERATOR(vector, typename underlying::value_type, it_state);

			vector() : data(new underlying()) {};
			explicit vector(size_type _N) : data(new underlying(_N)) {};
			vector(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			vector(vector const& r) : data(new underlying()) {
				operator=(r);
			}
			vector(vector&& r) = default;
			vector& operator=(vector const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					vector out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			vector& operator=(vector&& r) = default;
			~vector() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(std::make_shared<_Ty>(_Item)); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::make_shared<_Ty>(std::forward<_Ty>(_Item))); };
			decltype(auto) push_back(std::shared_ptr<_Ty> const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(std::shared_ptr<_Ty>&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			std::shared_ptr<_Ty> operator[](size_type _Index) { return data->operator[](_Index); };
			std::shared_ptr<_Ty> operator[](size_type _Index) const { return data->operator[](_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) { return data->at(_Index); };
			std::shared_ptr<_Ty> at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
			decltype(auto) erase(const_iterator _Where) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				vector out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.push_back(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) clear() {
				vector out;
				data.swap(out.data);
			};
			decltype(auto) swap(vector& r) {
				data.swap(r.data);
			};
		};

		/* Fiber- and thread-safe vector that cannot erase elements. Objects are stored as-is, and therefore the array must outlive the underlying objects. All operations are concurrent except for 'operator=(array)'.
		Higher performance is expected with the array than the vector, but the user needs to be slighly more careful with their timing to ensure the array remaings alive until access is complete. */
		template<typename _Ty>
		class array {
		protected:
			using underlying = typename concurrency::concurrent_vector<_Ty>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;
				inline void begin(const array* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const array* ref) { ++pos; }
				inline void end(const array* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(array* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const array* ref) { --pos; }
				// Optional to allow `const_iterator`:
				inline const typename underlying::value_type& get(const array* ref) const { return *pos; }
			};

			SETUP_STL_ITERATOR(array, typename underlying::value_type, it_state);

			array() : data(new underlying()) {};
			explicit array(size_type _N) : data(new underlying(_N)) {};
			array(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			array(array const& r) : data(new underlying()) {
				operator=(r);
			}
			array(array&& r) = default;
			array& operator=(array const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					array out;
					for (auto& x : r)
						out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			array& operator=(array&& r) = default;
			~array() {};

			decltype(auto) grow_by(size_type _Delta) { return data->grow_by(_Delta); };
			decltype(auto) grow_by(size_type _Delta, const_reference _Item) { return data->grow_by(_Delta, _Item); };
			decltype(auto) grow_to_at_least(size_type _N) { return data->grow_to_at_least(_N); };
			decltype(auto) push_back(_Ty const& _Item) { return data->push_back(_Item); };
			decltype(auto) push_back(_Ty&& _Item) { return data->push_back(std::forward<_Ty>(_Item)); };
			decltype(auto) operator[](size_type _Index) { return data->operator[](_Index); };
			decltype(auto) operator[](size_type _Index) const { return data->operator[](_Index); };
			decltype(auto) at(size_type _Index) { return data->at(_Index); };
			decltype(auto) at(size_type _Index) const { return data->at(_Index); };
			decltype(auto) size() const { return data->size(); };
			decltype(auto) empty() const { return data->empty(); };
			decltype(auto) capacity() const { return data->capacity(); };
			decltype(auto) max_size() const { return data->max_size(); };
		};

		/* Fiber- and thread-safe map / dictionary. Objects are stored and returned as std::shared_ptr. Growth, iterations, and insert/emplace operations are concurrent, while erasing and clearing are non-concurrent and will replace the entire map. */
		template<typename _Key_type, typename _Element_type>
		class unordered_map {
		protected:
			using underlying = typename concurrency::concurrent_unordered_map<_Key_type, std::shared_ptr<_Element_type>>;
			std::shared_ptr< underlying > data;

		public:
			struct it_state {
				std::shared_ptr< underlying > lifetime;
				typename underlying::iterator pos;

				inline void begin(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->begin(); }
				inline void next(const unordered_map* ref) { ++pos; }
				inline void end(const unordered_map* ref) { lifetime = ref->data; pos = lifetime->end(); }
				inline typename underlying::value_type& get(unordered_map* ref) { return *pos; }
				inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
				inline long long distance(const it_state& s) const { return pos - s.pos; };
				inline void prev(const unordered_map* ref) { --pos; }
				inline const typename underlying::value_type& get(const unordered_map* ref) const { return *pos; }
			};

			SETUP_STL_ITERATOR(unordered_map, typename underlying::value_type, it_state);

			typedef typename underlying::key_type key_type;
			typedef typename underlying::mapped_type mapped_type;
			typedef typename underlying::key_equal key_equal;
			typedef typename underlying::hasher hasher;
			typedef iterator local_iterator;
			typedef const_iterator const_local_iterator;

			unordered_map() : data(new underlying()) {};
			explicit unordered_map(size_type _N) : data(new underlying(_N)) {};
			unordered_map(size_type _N, const_reference _Item) : data(new underlying(_N, _Item)) {};
			template<class _InputIterator> unordered_map(_InputIterator _Begin, _InputIterator _End) : data(new underlying(_Begin, _End)) {};
			unordered_map(unordered_map const& r) : data(new underlying()) {
				this->operator=(r);
			};
			unordered_map(unordered_map&& r) = default;
			unordered_map& operator=(unordered_map const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					unordered_map out;
					for (auto& x : *r.data) out[x.first] = x.second;
					out.data.swap(data);
				}
				return *this;
			};
			unordered_map& operator=(unordered_map&& r) = default;

			decltype(auto) insert(const value_type& _Value) { return data->insert(_Value); };
			decltype(auto) insert(const_iterator _Where, const value_type& _Value) { return data->insert(_Where, _Value); };
			template<class _Iterator> decltype(auto) insert(_Iterator _First, _Iterator _Last) { return data->insert(_First, _Last); };
			template<class _Valty> decltype(auto) insert(_Valty&& _Value) { return data->insert(_Value); };
			template<class _Valty> decltype(auto) insert(const_iterator _Where, _Valty&& _Value) { return data->insert(_Where, _Value); };
			decltype(auto) hash_function() const { return data->hash_function(); };
			decltype(auto) key_eq() const { return data->key_eq(); };
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> operator[](const key_type& _Keyval) const {
				std::shared_ptr<_Element_type> out = data->operator[](_Keyval);
				if (!out) {
					out = std::make_shared<_Element_type>();
					data->operator[](_Keyval) = out;
				}
				return out;
			};
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) { return data->at(_Keyval); };
			std::shared_ptr<_Element_type> at(const key_type& _Keyval) const { return data->at(_Keyval); };
			decltype(auto) front() { return data->front(); };
			decltype(auto) front() const { return data->front(); };
			decltype(auto) back() { return data->back(); };
			decltype(auto) back() const { return data->back(); };
			decltype(auto) erase(const_iterator _Where) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter == _Where) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const_iterator _First, const_iterator _Last) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter >= _First && iter <= _Last) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) erase(const key_type& _Keyval) {
				unordered_map out;
				auto endIter = end();
				for (auto iter = begin(); iter != endIter; iter++) {
					if (iter->first == _Keyval) continue;
					out.insert(*iter);
				}
				data.swap(out.data);
			};
			decltype(auto) unsafe_erase(const key_type& _Keyval) {
				data->unsafe_erase(_Keyval);
			};
			decltype(auto) count(const key_type& _Keyval) const { return data->count(_Keyval); };
			decltype(auto) emplace(const key_type& _Keyval, const std::shared_ptr<_Element_type>& _Value) { return insert(value_type(_Keyval, _Value)); };
			decltype(auto) emplace(const key_type& _Keyval, std::shared_ptr<_Element_type>&& _Value) { return insert(value_type(_Keyval, std::forward<typename underlying::value_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, const _Element_type& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(_Value))); };
			decltype(auto) emplace(const key_type& _Keyval, _Element_type&& _Value) { return insert(value_type(_Keyval, std::make_shared<_Element_type>(std::forward<typename underlying::value_type>(_Value)))); };
			decltype(auto) clear() {
				unordered_map out;
				data.swap(out.data);
			};
		};

		template<typename _Key_type> using unordered_set = concurrency::concurrent_unordered_set<_Key_type>; /* Wrapper To-Do */
		template<typename _Value_type> using queue = moodycamel::ConcurrentQueue<_Value_type>; //  concurrency::concurrent_queue<_Value_type>; /* Wrapper To-Do */
	};

	namespace synchronization {
		/* *THREAD SAFE* Simple atomic spin-lock that unfairly synchronizes threads or fibers. (No guarrantee of order). Suggest using the ticket spin lock instead. */
		class SpinLock {
		private:
			std::atomic_flag lck = ATOMIC_FLAG_INIT;
		public:
			inline void lock() {
				int spin = 0;
				while (!try_lock())
				{
					if (spin < 10)
					{
						_mm_pause(); // SMT thread swap can occur here
					}
					else
					{
						std::this_thread::yield(); // OS thread swap can occur here. It is important to keep it as fallback, to avoid any chance of lockup by busy wait
					}
					spin++;
				}
			};
			inline bool try_lock() {
				return !lck.test_and_set(std::memory_order_acquire);
			};
			inline void unlock() {
				lck.clear(std::memory_order_release);
			};
		};
		
		/* *THREAD SAFE* Windows-specific high-performance lock that only locks the OS (slow) when contention actually happens. When there is no contention, this is very fast. 
		Generally speaking, out-performs std::mutex under most conditions. */
		class mutex {
		private:
			using mutexHandle_t = CRITICAL_SECTION;
			static void				Sys_MutexCreate(mutexHandle_t& handle) noexcept { InitializeCriticalSection(&handle); };
			static void				Sys_MutexDestroy(mutexHandle_t& handle) noexcept { DeleteCriticalSection(&handle); };
			static void				Sys_MutexLock(mutexHandle_t& handle) noexcept { EnterCriticalSection(&handle); };
			static bool				Sys_MutexTryLock(mutexHandle_t& handle) noexcept { return TryEnterCriticalSection(&handle) != 0; };
			static void				Sys_MutexUnlock(mutexHandle_t& handle) noexcept { LeaveCriticalSection(&handle); };
		public:
			mutex() noexcept { Sys_MutexCreate(Handle); };
			~mutex() noexcept { Sys_MutexDestroy(Handle); };

			inline void lock() {
				Sys_MutexLock(Handle);
			};
			inline bool try_lock() {
				return Sys_MutexTryLock(Handle);
			};
			inline void unlock() {
				Sys_MutexUnlock(Handle);
			};

			mutex(const mutex&) = delete;
			mutex(mutex&&) = delete;
			mutex& operator=(mutex const&) = delete;
			mutex& operator=(mutex&&) = delete;

		protected:
			mutexHandle_t Handle;

		};

		/* mutex which allows multiple readers OR one writer to access a critical section at the same time. */
		template <typename mutex = fibers::synchronization::mutex> class shared_mutex {
		private:
			static auto GetUnderlyingConditionalVariableExample() {
				if constexpr (std::is_same<mutex, std::mutex>::value) {
					return std::condition_variable();
				}
				else {
					return std::condition_variable_any();
				}
			};

		public:
			using cond_var = typename fibers::utilities::function_traits<decltype(std::function(GetUnderlyingConditionalVariableExample))>::result_type;

		private:
			mutex    mut_;
			cond_var gate1_;
			cond_var gate2_;
			unsigned state_;

			static const unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
			static const unsigned n_readers_ = ~write_entered_;

		public:
			shared_mutex() : mut_(), gate1_(), gate2_(), state_(0) {}

			// Exclusive ownership
			void lock() {
				std::unique_lock<mutex> lk(mut_);
				while (state_ & write_entered_) gate1_.wait(lk);
				state_ |= write_entered_;
				while (state_ & n_readers_) gate2_.wait(lk);
			};
			bool try_lock() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock_t);
				if (lk.owns_lock() && state_ == 0) {
					state_ = write_entered_;
					return true;
				}
				return false;
			};
			void unlock() {
				{
					std::scoped_lock<mutex> _(mut_);
					state_ = 0;
				}
				gate1_.notify_all();
			};

			// Shared ownership
			void lock_shared() {
				std::unique_lock<mutex> lk(mut_);
				while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_) gate1_.wait(lk);
				unsigned num_readers = (state_ & n_readers_) + 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
			};
			bool try_lock_shared() {
				std::unique_lock<mutex> lk(mut_, std::try_to_lock_t);
				unsigned num_readers = state_ & n_readers_;
				if (lk.owns_lock() && !(state_ & write_entered_) && num_readers != n_readers_) {
					++num_readers;
					state_ &= ~n_readers_;
					state_ |= num_readers;
					return true;
				}
				return false;
			};
			void unlock_shared() {
				std::scoped_lock<mutex> _(mut_);
				unsigned num_readers = (state_ & n_readers_) - 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
				if (state_ & write_entered_) {
					if (num_readers == 0) gate2_.notify_one();
				}
				else {
					if (num_readers == n_readers_ - 1) gate1_.notify_one();
				}
			};
		
		};

		/* *THREAD SAFE* Thread-safe and fiber-safe wrapper for any type of number, from integers to doubles. 
		   Significant performance boost if the data type is an integer type or one of: long, unsigned int, unsigned long, unsigned __int64		   
		   Slower, but still atomic using multi-word CAS algorithms, if using floating-point numbers like doubles or floats. 
		*/
		template<typename type = double>
		struct atomic_number {
		public:
			static constexpr bool isFloatingPoint = std::is_floating_point<type>::value;
			static constexpr bool isSigned = std::is_signed<type>::value;

		private:
			static auto ValidTypeExample() {
				if constexpr (isFloatingPoint) return fibers::utilities::CAS_Container<type>{};
				else { // Integral. Only 4 integral types are actually supported. long, unsigned int, unsigned long, unsigned __int64
					if constexpr (isSigned) {
						return static_cast<long>(0);
					}
					else {
						// get the largest type that can contain the requested type. 
						if constexpr (sizeof(type) <= sizeof(unsigned int)) {
							return static_cast<unsigned int>(0);
						}
						else if constexpr (sizeof(type) <= sizeof(unsigned long)) {
							return static_cast<unsigned long>(0);
						}
						else {
							return static_cast<unsigned __int64>(0);
						}
					}
				}
			};
			using internalType = typename std::remove_const_t<typename fibers::utilities::function_traits<decltype(std::function(ValidTypeExample))>::result_type>;

		public:
			constexpr atomic_number() : value{ static_cast<internalType>(0) }  {};
			constexpr atomic_number(const type& a) : value{ static_cast<internalType>(a) } {};
			constexpr atomic_number(type&& a) : value{ static_cast<internalType>(std::forward<type>(a)) } {};
			atomic_number(const atomic_number& other) : value{ other.load() } {};
			atomic_number(atomic_number&& other) : value{ other.load() } {};
			atomic_number& operator=(const atomic_number& other) { SetValue(other.load()); return *this; };
			atomic_number& operator=(atomic_number&& other) { SetValue(other.load()); return *this; };
			~atomic_number() = default;

			operator type() { return GetValue(); };
			operator const type() const { return GetValue(); };

			template <typename T> decltype(auto) operator+(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) + static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) + static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) + static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) + static_cast<type>(b.value);
						return out;
					}
				}
			};
			template <typename T> decltype(auto) operator-(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) - static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) - static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) - static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) - static_cast<type>(b.value);
						return out;
					}
				}
			};
			template <typename T> decltype(auto) operator/(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) / static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) / static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) / static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) / static_cast<type>(b.value);
						return out;
					}
				}
			};
			template <typename T> decltype(auto) operator*(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(load()) * static_cast<T>(b.load());
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(load()) * static_cast<type>(b.load());
						return out;
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						atomic_number<T> out;
						out = static_cast<T>(value) * static_cast<T>(b.value);
						return out;
					}
					else {
						atomic_number< type> out;
						out = static_cast<type>(value) * static_cast<type>(b.value);
						return out;
					}
				}
			};
			
			atomic_number& operator--() { 
				if constexpr (isFloatingPoint) {
					value.Add(-1);
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(-1));
					return *this;
				}
			};
			atomic_number& operator++() { 
				if constexpr (isFloatingPoint) {
					value.Add(1);
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(1));
					return *this;
				}
			};
			atomic_number operator--(int) { return Decrement() + 1; };
			atomic_number operator++(int) { return Increment() - 1; };

			atomic_number& operator+=(const atomic_number& i) { 
				if constexpr (isFloatingPoint) {
					value.Add(i.load());
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(i.value));
					return *this;
				}
			};
			atomic_number& operator-=(const atomic_number& i) { 
				if constexpr (isFloatingPoint) {
					value.Add(-1.0 * (i.load()));
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(-i.value));
					return *this;
				}
			};
			atomic_number& operator/=(const atomic_number& i) {
				if constexpr (isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV /= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}
						
					return *this;
				}
				else {
					InterlockedExchange(&value, static_cast<internalType>(value / i.value));
					return *this;
				}
			};			
			atomic_number& operator*=(const atomic_number& i) {
				if constexpr (isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV *= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}

					return *this;
				}
				else {
					InterlockedExchange(&value, static_cast<internalType>(value * i.value));
					return *this;
				}
			};

			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator+=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					value.Add(static_cast<type>(i.load()));
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(i.load()));
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator-=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					value.Add(-(i.load()));
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(-i.load()));
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator/=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV /= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}
				}
				else {
					InterlockedExchange(&value, value / static_cast<internalType>(i.load()));
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator*=(const atomic_number<T>& i) {
				if constexpr (atomic_number<type>::isFloatingPoint) {
					type newV, oldV;
					while (true) {
						newV = oldV = value.load();
						newV *= i.load();

						if (value.CompareSwap(oldV, newV)) break;
					}
				}
				else {
					InterlockedExchange(&value, value * static_cast<internalType>(i.load()));
				}
				return *this;
			};

			template <typename T> bool operator==(const atomic_number<T>& b) {
				if constexpr (sizeof(T) > sizeof(type)) {
					return static_cast<T>(load()) == static_cast<T>(b.load());
				}
				else {
					return static_cast<type>(load()) == static_cast<type>(b.load());
				}				
			};
			template <typename T> bool operator!=(const atomic_number<T>& b) {
				return !operator==(b);
			};
			template <typename T> bool operator<=(const atomic_number<T>& b) {
				if constexpr (sizeof(T) > sizeof(type)) {
					return static_cast<T>(load()) <= static_cast<T>(b.load());
				}
				else {
					return static_cast<type>(load()) <= static_cast<type>(b.load());
				}
			};
			template <typename T> bool operator>=(const atomic_number<T>& b) {
				if constexpr (sizeof(T) > sizeof(type)) {
					return static_cast<T>(load()) >= static_cast<T>(b.load());
				}
				else {
					return static_cast<type>(load()) >= static_cast<type>(b.load());
				}
			};
			template <typename T> bool operator<(const atomic_number<T>& b) {
				return !operator>=(b);
			};
			template <typename T> bool operator>(const atomic_number<T>& b) {
				return !operator<=(b);
			};

			atomic_number pow(atomic_number const& V) const { 
				if constexpr (isFloatingPoint) {
					return std::pow(value.load(), V.load());
				}
				else {
					return std::pow(value, V.value);
				}
			};
			atomic_number sqrt() const { 
				if constexpr (isFloatingPoint) {
					return std::sqrt(value.load());
				}
				else {
					return std::sqrt(value);
				}
			};
			atomic_number floor() const { 
				if constexpr (isFloatingPoint) {
					return std::floor(value.load());
				}
				else {
					return std::floor(value);
				}
			};
			atomic_number ceiling() const { 
				if constexpr (isFloatingPoint) {
					return std::ceil(value.load());
				}
				else {
					return std::ceil(value);
				}
			};

			bool CompareExchange(type expected, type newValue) {
				if constexpr (isFloatingPoint) {
					return value.CompareSwap(expected, newValue);
				}
				else {
					return InterlockedCompareExchange(&value, newValue, expected) == expected;
				}
			}; // atomically sets the value if the old value equals expected. Returns true on success.
			type Max(type newValue) {
				while (true) {
					auto previous = load();
					if (CompareExchange(previous, std::max<type>(previous, newValue))) return previous;
				}
			}; // Sets the atomic_number to the max of its current value and the newValue. Returns the previous value once successful.
			type Min(type newValue) {
				while (true) {
					auto previous = load();
					if (CompareExchange(previous, std::min<type>(previous, newValue))) return previous;
				}
			}; // Sets the atomic_number to the min of its current value and the newValue. Returns the previous value once successful.

			type Increment() { 
				if constexpr (isFloatingPoint) {
					return value.Add(1) + 1;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(1)) + 1;
				}
			} // atomically increments the value and returns the new value
			type Decrement() { 
				if constexpr (isFloatingPoint) {
					return value.Add(-1) - 1;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-1)) - 1;
				}
			} // atomically decrements the value and returns the new value
			type Add(const type& v) { 
				if constexpr (isFloatingPoint) {
					return value.Add(v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(v)) + v;
				}
			} // atomically adds a value and returns the new value
			type Sub(const type& v) {
				if constexpr (isFloatingPoint) {
					return value.Add(-v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-v)) - v;
				}
			}; // atomically subtracts a value and returns the new value
			type GetValue() const { 
				if constexpr (isFloatingPoint) {
					return value.load();
				}
				else {
					return value;
				}				
			}
			type SetValue(const type& v) {
				if constexpr (isFloatingPoint) {
					return value.Swap(v);
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting with the new value
			type SetValue(type&& v) {
				if constexpr (isFloatingPoint) {
					return value.Swap(std::forward<type>(v));
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting with the new value
		
        public: // std::atomic compatability
			type fetch_add(type const& v) {
				if constexpr (isFloatingPoint) {
					return value.fetch_add(v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while incrementing the actual counter
			type fetch_sub(type const& v) {
				if constexpr (isFloatingPoint) {
					return value.fetch_sub(v);
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-v));
				}
			}; // returns the previous value while decrementing the actual counter
			static constexpr bool is_lock_free() {
				if constexpr (isFloatingPoint) {
					return true; // thanks to the MwCAS algorithm
				}
				else {
					return true; // built-in to 99.99% of hardware nowadays
				}
			}; // returns whether a lock is utilized or the number is actually lock-free
			type exchange(type const& v) {
				if constexpr (isFloatingPoint) {
					return value.exchange(v);
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting the value to the input
			type load() const { return GetValue(); };
			void store(type const& v) {
				(void)exchange(v);
			}; // sets the value to the input

		public: // std::scoped_lock compatability
			inline void lock() {
				while (Increment() != static_cast<type>(1)) Decrement();
			};
			inline bool try_lock() {
				return Increment() == static_cast<type>(1);
			};
			inline void unlock() {
				Decrement();
			};

		private:
			internalType value;

		};

		/* *THREAD SAFE* Thread-safe and fiber-safe wrapper for atomic operations on pointers, without having to utilize std::atomic<T*> */
		template< typename T> 
		struct atomic_ptr {
		private:
			static void* Sys_InterlockedExchangePointer(void*& ptr, void* exchange) { return InterlockedExchangePointer(&ptr, exchange); };
			static void* Sys_InterlockedCompareExchangePointer(void*& ptr, void* comparand, void* exchange) { return InterlockedCompareExchangePointer(&ptr, exchange, comparand); };

		public:
			constexpr atomic_ptr() noexcept : ptr(nullptr) {}
			constexpr atomic_ptr(T* newSource) noexcept : ptr(newSource) {}
			constexpr atomic_ptr(const atomic_ptr& other) noexcept : ptr(other.ptr) {};
			atomic_ptr& operator=(const atomic_ptr& other) noexcept { Set(other.Get()); return *this; };
			atomic_ptr& operator=(T* newSource) noexcept { Set(newSource); return *this; };
			~atomic_ptr() { ptr = nullptr; };

			explicit operator bool() { return ptr; };
			explicit operator bool() const { return ptr; };

			operator T* () noexcept { return ptr; };
			operator const T* () const noexcept { return ptr; };

			/* atomically sets the pointer and returns the previous pointer value */
			T* Set(T* newPtr) noexcept {
				return static_cast<T*>(Sys_InterlockedExchangePointer((void* &)ptr, static_cast<void* >(newPtr)));
			};
			bool TrySet(T* newPtr, T** oldPtr = nullptr) noexcept {
				T* PREV_VAL = this->load();
				if (this->CompareExchange(PREV_VAL, newPtr) == PREV_VAL) {
					if (oldPtr) *oldPtr = PREV_VAL;
					return true;
				}
				else {
					if (oldPtr) *oldPtr = nullptr;
					return false;
				}
			};
			bool TrySet(T* newPtr, atomic_ptr<T>* oldPtr) noexcept {
				T* PREV_VAL = this->load();
				if (this->CompareExchange(PREV_VAL, newPtr) == PREV_VAL) {
					if (oldPtr) *oldPtr = PREV_VAL;
					return true;
				}
				else {
					if (oldPtr) *oldPtr = nullptr;
					return false;
				}
			};

			/* atomically sets the pointer to 'newPtr' only if the previous pointer is equal to 'comparePtr' */
			T* CompareExchange(T* comparePtr, T* newPtr) noexcept {
				return static_cast<T*>(Sys_InterlockedCompareExchangePointer((void*&)ptr, static_cast<void*>(comparePtr), static_cast<void*>(newPtr)));
			};

			T* operator->() noexcept { return Get(); };
			const T* operator->() const noexcept { return Get(); };
			T* Get() noexcept { return ptr; };
			T* Get() const noexcept { return ptr; };
			T* load() noexcept { return Get(); };
			T* load() const noexcept { return Get(); };

		protected:
			T* ptr;
		};
	};

	namespace utilities {
		/* *THREAD SAFE* Thread- and Fiber-safe allocator that can create, reserve, and free shared memory. 
		Optimized for POD types, but will correctly manage the destruction of non-POD type data when shutdown. */
		template<class _type_, int _blockSize_, bool ForcePOD = false> class Allocator {
		private:
			struct element_item {
				// actual underlying data
				_type_ data;
				// non-POD, but can be "forgotten" without consequence. 
				synchronization::atomic_ptr<element_item> next; 
				// non-POD, but can be "forgotten" without consequence. 
				synchronization::atomic_number<long> initialized; 
			};
			class memory_block {
			public:
				// static buffer -- does not grow or shrink. Cannot allocate less than this, and if needs more, we allocate another block.
				element_item		elements[_blockSize_]; 
				// ptr to next block.
				synchronization::atomic_ptr<memory_block> next; 
			};

			synchronization::atomic_ptr<memory_block> blocks;
			synchronization::atomic_ptr<element_item> free;
			synchronization::atomic_number<long> total;
			synchronization::atomic_number<long> active;

			// can an element_item be "forgotten" without calling a destructor?
			static constexpr bool isPod() { return std::is_pod<_type_>::value || ForcePOD; };
			// creates a new memory_block and sets the free ptr. 
			void			    AllocNewBlock() {
				memory_block* block{ (memory_block*)Mem_ClearedAlloc((size_t)(sizeof(memory_block))) }; // explicitely initialized to 0 at all bits
				int i;
				while (true) {
					block->next = blocks.load();
					if (blocks.TrySet(block, &block->next)) {
						for (i = 0; i < _blockSize_; i++) {
							while (true) {
								block->elements[i].next = free.load();
								if (free.TrySet(&block->elements[i], &block->elements[i].next)) {
									break;
								}
							}
						}
						total += _blockSize_;
						break;
					}
				}
			};
			// shuts down the allocator and frees all associated memory and (if needed) destroys the non-POD type data.
			void			    Shutdown() {
				memory_block* block;
				int i;

				while (block = blocks.load()) {
					if (blocks.TrySet(block->next, &block)) {
						// Check if the data type if POD...
						if constexpr (!isPod()) {
							// ... because non-POD types must call their destructors to prevent memory leaks per element ...
							for (i = 0; i < _blockSize_; i++) 
								// ... but only when the element was already initialized ...
								if (block->elements[i].initialized.Decrement() == 0) block->elements[i].data.~_type_();
						}
						// ... then we can free the memory block
						Mem_Free(block);
					}
				}

				free = nullptr;
				total = active = 0;
			};

		public:
			Allocator() : blocks(nullptr), free(nullptr), total(0), active(0) {};
			Allocator(int toReserve) : blocks(nullptr), free(nullptr), total(0), active(0) { Reserve(toReserve); };
			~Allocator() { Shutdown(); };

			// returns total size of allocated memory
			size_t				Allocated() const { return total.load() * sizeof(_type_); }

			// returns total size of allocated memory including size of (*this)
			size_t				Size() const { return sizeof(*this) + Allocated(); }

			// Request a new memory pointer. May be recovered from a previously-used location. Will be cleared and correctly initialized, if appropriate.
			template <typename... TArgs> _type_* Alloc(TArgs const&... a) {
				_type_* t{ nullptr };
				element_item* element{ nullptr };

				++active;
				while (!element) {
					while (element = free.load()) { // if we have free elements available...
						if (free.TrySet(element->next, &element)) { // get the free element and swap it with it's next ptr. If this fails, we will simply try again.
							// we now have exclusive access to this element.
							element->next = nullptr;
							break;
						}
					}
					// if we fell through and for some reason the element is still empty, we need to allocate more. May be due to contention and many allocations are happening.
					if (!element) AllocNewBlock(); // adds a new element to the "free" elements
				}

				t = static_cast<_type_*>(static_cast<void*>(element));
				memset(t, 0, sizeof(_type_));
				new (t) _type_(a...);

				return t;
			};

			// Frees the memory pointer, previously provided by this allocator. Calls the destructor for non-POD types, and will store the pointer for later use.
			void				Free(_type_* element) {
				element_item* t{nullptr};
				element_item* prevFree{ nullptr };

				if (!element) return; // no work to be done
				
				t = static_cast<element_item*>(static_cast<void*>(element));

				if constexpr (!isPod()) {
					if (t->initialized.Decrement() == 0)
						element->~_type_();
					
				}				
				while (true) {
					prevFree = (t->next = free.load());
					if (free.CompareExchange(prevFree, t) == prevFree) {
						--active;
						break;
					}
				}				
			};

			// Request a new memory pointer that will self-delete and return to the memory pool automatically. Important: This allocator must out-live the shared_ptr.
			std::shared_ptr< _type_ > AllocShared() {
				return std::shared_ptr<_type_>(Alloc(), [this](_type_* p) { Free(p); });
			};

			// Calls "Alloc" X-num times, and then frees them all for later re-use.
			__forceinline void	Reserve(long long num) {
				// this algorithm can be improved. 
				// TODO: Create (num / _blockSize_) memory_blocks, order them as next->PTR->next->PTR, etc, and the Compare-Swap with the current end of the block chain. Then update the free chain as needed.

				if (total < num) {
					std::vector< _type_* > arr; arr.reserve(2 * (num - total));
					while (total < num) {
						arr.push_back(Alloc());
					}
					for (_type_* p : arr) {
						Free(p);
					}
				}
			};
			
			long long			GetTotalCount() const { return total.load(); }
			long long			GetAllocCount() const { return active.load(); }
			long long			GetFreeCount() const { return total.load() - active.load(); }
		};

		// bw tree
		namespace dbgroup::index::bw_tree
		{
			/**
			 * @brief A class for representing Bw-trees.
			 *
			 * @tparam Key a class of stored keys.
			 * @tparam Payload a class of stored payloads (only fixed-length data for simplicity).
			 * @tparam Comp a class for ordering keys.
			 */
			template <class Key,
				class Payload,
				class Comp = ::std::less<Key>
			>
				class BwTree
			{
			public:
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/
				using KeyWOPtr = std::remove_pointer_t<Key>;
				using PageID = uint64_t;
				using NodePage = component::NodePage;
				using DeltaPage = component::DeltaPage;
				using DeltaRC = component::DeltaRC;
				using DeltaType = component::DeltaType;
				using LogicalPtr = component::LogicalPtr;
				using NodeFixLen_t = component::fixlen::Node<Key, Comp>;
				using Node_t = NodeFixLen_t;
				using DeltaFixLen_t = component::fixlen::DeltaRecord<Key, Comp>;
				using Delta_t = DeltaFixLen_t;
				using Record = typename Delta_t::Record;
				using RecordIterator_t = component::RecordIterator<Key, Payload, Comp>;
				friend RecordIterator_t;  // call sibling scan from iterators
				using MappingTable_t = component::MappingTable<Node_t, Delta_t>;
				using DC = component::DeltaChain<Delta_t>;
				using ConsolidateInfo = std::pair<const void*, const void*>;
				using NodeGC_t = dbgroup::memory::EpochBasedGC<NodePage, DeltaPage>;
				using ScanKey = std::optional<std::tuple<const Key&, size_t, bool>>;

				template <class Entry>
				using BulkIter = typename std::vector<Entry>::const_iterator;
				using NodeEntry = std::tuple<Key, PageID, size_t>;
				using BulkResult = std::pair<size_t, std::vector<NodeEntry>>;
				using BulkPromise = std::promise<BulkResult>;
				using BulkFuture = std::future<BulkResult>;

#if 0
				using value_type = std::pair<Key, Payload>;
				struct it_state {
					mutable RecordIterator_t pos;
					mutable value_type obj;

					inline void begin(const BwTree* ref) { pos = const_cast<BwTree*>(ref)->Scan(); }
					inline void next(const BwTree* ref) { ++pos; }
					inline void end(const BwTree* ref) { pos = RecordIterator_t(); }
					inline value_type& get(BwTree* ref) { if (pos) { obj.first = pos.GetKey(); obj.second = pos.GetPayload(); } return obj; }
					inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
					inline long long distance(const it_state& s) const { return pos - s.pos; };
					inline void prev(const BwTree* ref) { --pos; }
					inline const value_type& get(const BwTree* ref) const { if (pos) { obj.first = pos.GetKey(); obj.second = pos.GetPayload(); } return obj; }
				};
				SETUP_STL_ITERATOR(BwTree, value_type, it_state);
#endif

				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct a new BwTree object.
				  *
				  * @param gc_interval_microsec GC internal [us] (default: 10ms).
				  * @param gc_thread_num the number of GC threads (default: 1).
				  */
				explicit BwTree(  //
					const size_t gc_interval_microsec = kDefaultGCTime,
					const size_t gc_thread_num = kDefaultGCThreadNum)
					: gc_{ gc_interval_microsec, gc_thread_num }
				{
					// create an empty Bw-tree
					auto* root_node = new (GetNodePage()) Node_t{};
					auto root_id = mapping_table_.GetNewPageID();
					auto* root_ptr = mapping_table_.GetLogicalPtr(root_id);
					root_ptr->Store(root_node);
					root_.store(root_id, std::memory_order_relaxed);

					gc_.StartGC();
				}

				BwTree(const BwTree&) = delete;
				BwTree(BwTree&&) = delete;

				auto operator=(const BwTree&)->BwTree & = delete;
				auto operator=(BwTree&&)->BwTree & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the BwTree object.
				  *
				  */
				~BwTree() = default;

				/*####################################################################################
				 * Public read APIs
				 *##################################################################################*/

				 /**
				  * @brief Read the payload corresponding to a given key if it exists.
				  *
				  * @param key a target key.
				  * @param key_len the length of the target key.
				  * @retval the payload of a given key wrapped with std::optional if it is in this tree.
				  * @retval std::nullopt otherwise.
				  */
				auto
					Read(  //
						const Key& key,
						[[maybe_unused]] const size_t key_len = sizeof(Key)) //
					-> std::optional<Payload>
				{
					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();

					// check whether the leaf node has a target key
					auto&& stack = SearchLeafNode(key, kClosed);

					for (Payload payload{}; true;) {
						// check whether the node is active and has a target key
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();

						uintptr_t out_ptr{};
						auto rc = DC::SearchRecord(head, key, out_ptr);
						switch (rc) {
						case DeltaRC::kRecordFound:
							payload = reinterpret_cast<Delta_t*>(out_ptr)->template GetPayload<Payload>();
							break;

						case DeltaRC::kRecordNotFound:
							break;

						case DeltaRC::kKeyIsInSibling:
							// swap a current node in a stack and retry
							stack.back() = out_ptr;
							continue;

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							SearchChildNode(key, kClosed, stack);
							continue;

						case DeltaRC::kReachBaseNode:
						default: {
							// search a target key in the base node
							const auto* node = reinterpret_cast<Node_t*>(out_ptr);
							std::tie(rc, out_ptr) = node->SearchRecord(key);
							if (rc == DeltaRC::kRecordFound) {
								payload = node->template GetPayload<Payload>(out_ptr);
							}
							break;
						}
						}

						if (rc == DeltaRC::kRecordNotFound) return std::nullopt;
						return payload;
					}
				}

				/**
				 * @brief Perform a range scan with given keys.
				 *
				 * @param begin_key a pair of a begin key and its openness (true=closed).
				 * @param end_key a pair of an end key and its openness (true=closed).
				 * @return an iterator to access scanned records.
				 */
				auto
					Scan(  //
						const ScanKey& begin_key = std::nullopt,
						const ScanKey& end_key = std::nullopt)   //
					-> RecordIterator_t
				{
					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();
					thread_local std::unique_ptr<void, std::function<void(void*)>>  //
						page{ dbgroup::memory::Allocate<NodePage>(2 * kPageSize),
							 dbgroup::memory::Release<NodePage> };

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};
					if (begin_key) {
						// traverse to a leaf node and sort records for scanning
						const auto& [b_key, b_key_len, b_closed] = *begin_key;
						auto&& stack = SearchLeafNode(b_key, b_closed);
						begin_pos = ConsolidateForScan(node, b_key, b_closed, stack);
					}
					else {
						Node_t* dummy_node = nullptr;
						// traverse to the leftmost leaf node directly
						auto&& stack = SearchLeftmostLeaf();
						while (true) {
							const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
							const auto* head = lptr->template Load<Delta_t*>();
							if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
							TryConsolidate(head, node, dummy_node, kIsScan);
							break;
						}
						begin_pos = 0;
					}

					// check the end position of scanning
					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);

					return RecordIterator_t{ this, node, begin_pos, end_pos, end_key, is_end };
				}

				/**
				 * @brief Try to find the node the smallest key that is equal to or greater than the requested key. (e.g. search for 55.5, returns 56)
				 *
				 * @param key a key to search for.
				 * @return an iterator to access the scanned record.
				 */
				auto
					FindSmallestLargerEqual(const Key& key, std::optional<Key> const& maxKey = std::nullopt)
					-> RecordIterator_t
				{
					ScanKey begin_key(std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true })); // true = may include the value if found
					ScanKey end_key;
					if (maxKey.has_value()) {
						end_key.emplace(std::tuple<const Key&, size_t, bool>({ maxKey.value(), (size_t)(sizeof(Key)), true })); // true = may include the value if found
					}

					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();
					thread_local std::unique_ptr<void, std::function<void(void*)>> page{ dbgroup::memory::Allocate<NodePage>(2 * kPageSize), dbgroup::memory::Release<NodePage> };

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					{
						// traverse to a leaf node and sort records for scanning
						const auto& [b_key, b_key_len, b_closed] = *begin_key;
						auto&& stack = SearchLeafNode(b_key, b_closed);
						begin_pos = ConsolidateForScan(node, b_key, b_closed, stack);

						RecordIterator_t record{ this, node, begin_pos, begin_pos + 1, std::nullopt, true };

						if (record && record.GetKey() >= key) { // success. Accounts for ~ 95% - 99% of cases. 
							// check the end position of scanning
							const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
							if (end_key.has_value()) {
								record.end_key_.emplace(end_key.value());
							}
							record.is_end_ = is_end;
							record.rec_count_ = end_pos;
							return record;
						}
						else {
							// We are at the end of a node, and the solution is either on the next node, or the solution does not exist. 
							if (record.node_->has_high_key_) {
								// go to the next sibling/node and continue scanning
								const auto& next_key = record.node_->GetHighKey();
								const auto sib_pid = record.node_->template GetNext<PageID>();
								record = this->SiblingScan(sib_pid, record.node_, next_key, std::nullopt);

								//record.bw_tree_ = this;
								RecordIterator_t record2{ record };
								while (record && record.GetKey() < key) {
									record++;
									if (record) {
										record2++;
									}
								}

								if (record) {
									const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
									if (end_key.has_value()) {
										record2.end_key_.emplace(end_key.value());
									}
									record2.is_end_ = is_end;
									record2.rec_count_ = end_pos;
								} // otherwise we are already at the end and nothing can be done -- no need to search.
								return record2;
							}
							else {
								// this happens when we are out-of-bounds for the search region and the desired key is larger than our highest available key.
								// Because we attempted to find a key that was out-of-bounds, the "node" we got is slightly broken, and must be repaired by re-searching:
								auto lowKey = node->GetLowKey();

								node = new (page.get()) Node_t{};
								// traverse to a leaf node and sort records for scanning
								auto&& stack2 = SearchLeafNode(lowKey, b_closed);
								begin_pos = ConsolidateForScan(node, lowKey, b_closed, stack2);

								const auto [is_end, end_pos] = node->SearchEndPositionFor(std::nullopt);

								record = RecordIterator_t{ this, node, begin_pos, end_pos, std::nullopt, is_end };
								RecordIterator_t record2{ record };
								while (record && record.GetKey() < key) {
									record++;
									if (record) {
										record2++;
									}
								}
								return record2;
							}
						}
					}
				};

				/**
				 * @brief Try to find the first, smallest node in the tree.
				 *
				 * @return an iterator to access the first, smallest keyed record.
				 */
				auto
					First()
					-> RecordIterator_t
				{
					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();
					thread_local std::unique_ptr<void, std::function<void(void*)>> page{ dbgroup::memory::Allocate<NodePage>(2 * kPageSize), dbgroup::memory::Release<NodePage> };

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					Node_t* dummy_node = nullptr;
					// traverse to the leftmost leaf node directly
					auto&& stack = SearchLeftmostLeaf();
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}
					begin_pos = 0;

					const auto [is_end, end_pos] = node->SearchEndPositionFor(std::nullopt);

					return RecordIterator_t{ this, node, begin_pos, end_pos, std::nullopt, is_end };
				};

				/**
				 * @brief Try to find the node the largest key that is equal to or less than the requested key. (e.g. search for 55.5, returns 55)
				 *
				 * @param key a key to search for.
				 * @return an iterator to access the scanned record.
				 */
				auto
					FindLargestSmallerEqual(const Key& key, std::optional<Key> const& maxKey = std::nullopt)
					-> RecordIterator_t
				{
					ScanKey keyFind{ std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true }) };
					ScanKey end_key;
					if (maxKey.has_value()) {
						end_key.emplace(std::tuple<const Key&, size_t, bool>({ maxKey.value(), (size_t)(sizeof(Key)), true })); // true = may include the value if found
					}

					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();
					thread_local std::unique_ptr<void, std::function<void(void*)>> page{ dbgroup::memory::Allocate<NodePage>(2 * kPageSize), dbgroup::memory::Release<NodePage> };

					auto* node = new (page.get()) Node_t{};
					size_t begin_pos{};

					Node_t* dummy_node = nullptr;
					// traverse to the leftmost leaf node directly
					auto&& stack = SearchLeftmostLeaf();
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}
					begin_pos = 0;

					RecordIterator_t record{ this, node, begin_pos, begin_pos + 1, std::nullopt, true }; // first "node" in the entire tree. If this is not valid, we are hopeless.
					RecordIterator_t FinalRecord{ record };
					if (!record) { return record; }

					while (record) {
						const auto [is_end, end_pos] = record.node_->SearchEndPositionFor(keyFind);
						if (is_end) { // indicates that a node larger than ours exists on this node.
							if (end_pos > 0) {
								FinalRecord = RecordIterator_t{ this, record.node_, end_pos - 1, end_pos, std::nullopt, true };
								break;
							}
							else {
								// end_pos was 0, meaning the best value was the last one!
								break;
							}
						}
						else {
							// indicates that there will be another node after this, likely with a node larger than ours. BUT it could be the first item in that new node, so we capture the last item in this one just in case.
							FinalRecord = RecordIterator_t{ this, record.node_, end_pos, end_pos + 1, std::nullopt, true };

							// go to the next sibling/node and continue scanning
							const auto& next_key = record.node_->GetHighKey();
							const auto sib_pid = record.node_->template GetNext<PageID>();
							record = this->SiblingScan(sib_pid, record.node_, next_key, keyFind);
						}
					}

					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);
					if (end_key.has_value()) {
						FinalRecord.end_key_.emplace(end_key.value());
					}
					FinalRecord.is_end_ = is_end;
					FinalRecord.rec_count_ = end_pos;

					return FinalRecord;
				};

				/*####################################################################################
				 * Public write APIs
				 *##################################################################################*/

				 /**
				  * @brief Write (i.e., put) a given key/payload pair. E.g. if not exist, insert, and if exists, overwrites.
				  *
				  * This function always overwrites a payload and can be optimized for that purpose;
				  * the procedure may omit the key uniqueness check.
				  *
				  * @param key a target key.
				  * @param payload a target payload.
				  * @param key_len the length of the target key.
				  * @param pay_len the length of the target payload.
				  * @return kSuccess.
				  */
				auto
					Write(  //
						const Key& key,
						const Payload& payload,
						const size_t key_len = sizeof(Key),
						[[maybe_unused]] const size_t pay_len = sizeof(Payload))  //
					-> ReturnCode
				{
					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					const auto rec_len = key_len + kPayLen + kMetaLen;
					auto* write_d = new (GetRecPage()) Delta_t{ DeltaType::kInsert, key, key_len, payload };
					while (true) {
						// check whether the target node includes incomplete SMOs
						const auto [head, rc] = GetHeadWithKeyCheck(key, stack);
						if (rc == DeltaRC::kRecordFound) {
							write_d->SetDeltaType(DeltaType::kModify);
							write_d->SetNext(head, 0);
						}
						else {
							write_d->SetDeltaType(DeltaType::kInsert);
							write_d->SetNext(head, rec_len);
						}

						// try to insert the delta record
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, write_d)) break;
					}

					if (write_d->NeedConsolidation()) {
						TrySMOs(write_d, stack);
					}

					return kSuccess;
				}

				/**
				 * @brief Insert a given key/payload pair.
				 *
				 * This function performs a uniqueness check on its processing. If the given key does
				 * not exist in this tree, this function inserts a target payload into this tree. If
				 * the given key exists in this tree, this function does nothing and returns kKeyExist.
				 *
				 * @param key a target key.
				 * @param payload a target payload.
				 * @param key_len the length of the target key.
				 * @param pay_len the length of the target payload.
				 * @retval kSuccess if inserted.
				 * @retval kKeyExist otherwise.
				 */
				auto
					Insert(  //
						const Key& key,
						const Payload& payload,
						const size_t key_len = sizeof(Key),
						[[maybe_unused]] const size_t pay_len = sizeof(Payload))  //
				{
					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					const auto rec_len = key_len + kPayLen + kMetaLen;
					auto* insert_d = new (GetRecPage()) Delta_t{ DeltaType::kInsert, key, key_len, payload };
					auto rc = kSuccess;
					while (true) {
						// check target record's existence and get a head pointer
						const auto [head, existence] = GetHeadWithKeyCheck(key, stack);
						if (existence == DeltaRC::kRecordFound) {
							rc = kKeyExist;
							tls_delta_page_.reset(insert_d);
							break;
						}

						// try to insert the delta record
						insert_d->SetNext(head, rec_len);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, insert_d)) {
							if (insert_d->NeedConsolidation()) {
								TrySMOs(insert_d, stack);
							}
							break;
						}
					}

					return rc;
				}

				/**
				 * @brief Update the record corresponding to a given key with a given payload.
				 *
				 * This function performs a uniqueness check on its processing. If the given key
				 * exists in this tree, this function updates the corresponding payload. If the given
				 * key does not exist in this tree, this function does nothing and returns
				 * kKeyNotExist.
				 *
				 * @param key a target key.
				 * @param payload a target payload.
				 * @param key_len the length of the target key.
				 * @param pay_len the length of the target payload.
				 * @retval kSuccess if updated.
				 * @retval kKeyNotExist otherwise.
				 */
				auto
					Update(  //
						const Key& key,
						const Payload& payload,
						const size_t key_len = sizeof(Key),
						[[maybe_unused]] const size_t pay_len = sizeof(Payload))  //
				{
					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					auto* modify_d = new (GetRecPage()) Delta_t{ DeltaType::kModify, key, key_len, payload };
					auto rc = kSuccess;
					while (true) {
						// check target record's existence and get a head pointer
						const auto [head, existence] = GetHeadWithKeyCheck(key, stack);
						if (existence == DeltaRC::kRecordNotFound) {
							rc = kKeyNotExist;
							tls_delta_page_.reset(modify_d);
							break;
						}

						// try to insert the delta record
						modify_d->SetNext(head, 0);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, modify_d)) {
							if (modify_d->NeedConsolidation()) {
								TrySMOs(modify_d, stack);
							}
							break;
						}
					}

					return rc;
				}

				/**
				 * @brief Delete the record corresponding to a given key from this tree.
				 *
				 * This function performs a uniqueness check on its processing. If the given key
				 * exists in this tree, this function deletes it. If the given key does not exist in
				 * this tree, this function does nothing and returns kKeyNotExist.
				 *
				 * @param key a target key.
				 * @param key_len the length of the target key.
				 * @retval kSuccess if deleted.
				 * @retval kKeyNotExist otherwise.
				 */
				auto
					Delete(const Key& key,
						const size_t key_len = sizeof(Key))  //
					-> ReturnCode
				{
					[[maybe_unused]] const auto& guard = gc_.CreateEpochGuard();

					// traverse to a target leaf node
					auto&& stack = SearchLeafNode(key, kClosed);

					// insert a delta record
					const auto rec_len = key_len + kPayLen + kMetaLen;
					auto* delete_d = new (GetRecPage()) Delta_t{ key, key_len };
					auto rc = kSuccess;
					while (true) {
						// check target record's existence and get a head pointer
						auto [head, existence] = GetHeadWithKeyCheck(key, stack);
						if (existence == DeltaRC::kRecordNotFound) {
							rc = kKeyNotExist;
							tls_delta_page_.reset(delete_d);
							break;
						}

						// try to insert the delta record
						delete_d->SetNext(head, rec_len); // delete_d->SetNext(head, -rec_len);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, delete_d)) {
							if (delete_d->NeedConsolidation()) {
								TrySMOs(delete_d, stack);
							}
							break;
						}
					}

					return rc;
				}

				/*####################################################################################
				 * Public bulkload API
				 *##################################################################################*/

				 /**
				  * @brief Bulkload specified kay/payload pairs.
				  *
				  * This function loads the given entries into this index, assuming that the entries
				  * are given as a vector of key/payload pairs (or the tuples key/payload/key-length
				  * for variable-length keys). Note that keys in records are assumed to be unique and
				  * sorted.
				  *
				  * @tparam Entry a container of a key/payload pair.
				  * @param entries the vector of entries to be bulkloaded.
				  * @param thread_num the number of threads used for bulk loading.
				  * @return kSuccess.
				  */
				template <class Entry>
				auto
					Bulkload(  //
						const std::vector<Entry>& entries,
						const size_t thread_num = 1)  //
					-> ReturnCode
				{
					if (entries.empty()) return ReturnCode::kSuccess;

					std::vector<NodeEntry> nodes{};
					auto&& iter = entries.cbegin();
					const auto rec_num = entries.size();
					if (thread_num <= 1 || rec_num < thread_num) {
						// bulkloading with a single thread
						nodes = BulkloadWithSingleThread<Entry>(iter, rec_num).second;
					}
					else {
						// bulkloading with multi-threads
						std::vector<BulkFuture> futures{};
						futures.reserve(thread_num);

						// a lambda function for bulkloading with multi-threads
						auto loader = [&](BulkPromise p, BulkIter<Entry> iter, size_t n) {
							p.set_value(BulkloadWithSingleThread<Entry>(iter, n));
						};

						// create threads to construct partial BzTrees
						for (size_t i = 0; i < thread_num; ++i) {
							// create a partial BzTree
							BulkPromise p{};
							futures.emplace_back(p.get_future());
							const size_t n = (rec_num + i) / thread_num;
							std::thread{ loader, std::move(p), iter, n }.detach();

							// forward the iterator to the next begin position
							iter += n;
						}

						// wait for the worker threads to create partial trees
						std::vector<BulkResult> partial_trees{};
						partial_trees.reserve(thread_num);
						size_t height = 1;
						for (auto&& future : futures) {
							partial_trees.emplace_back(future.get());
							const auto partial_height = partial_trees.back().first;
							height = (partial_height > height) ? partial_height : height;
						}

						// align the height of partial trees
						nodes.reserve(kInnerNodeCap * thread_num);
						PageID prev_pid = kNullPtr;
						for (auto&& [p_height, p_nodes] : partial_trees) {
							while (p_height < height) {  // NOLINT
								p_nodes = ConstructSingleLayer<NodeEntry>(p_nodes.cbegin(), p_nodes.size(), kIsInner);
								++p_height;
							}
							nodes.insert(nodes.end(), p_nodes.begin(), p_nodes.end());

							// link partial trees
							Node_t::LinkVerticalBorderNodes(prev_pid, std::get<1>(p_nodes.front()), mapping_table_);
							prev_pid = std::get<1>(p_nodes.back());
						}
					}

					// create upper layers until a root node is created
					while (nodes.size() > 1) {
						nodes = ConstructSingleLayer<NodeEntry>(nodes.cbegin(), nodes.size(), kIsInner);
					}
					const auto new_pid = std::get<1>(nodes.front());
					Node_t::RemoveLeftmostKeys(new_pid, mapping_table_);

					// set a new root
					const auto old_pid = root_.exchange(new_pid, std::memory_order_release);
					auto* old_lptr = mapping_table_.GetLogicalPtr(old_pid);
					gc_.AddGarbage<NodePage>(old_lptr->template Load<Delta_t*>());
					old_lptr->Clear();

					return ReturnCode::kSuccess;
				}

				/*####################################################################################
				 * Public utilities
				 *##################################################################################*/

				 /**
				  * @brief Collect statistical data of this tree.
				  *
				  * @retval 1st: the number of nodes.
				  * @retval 2nd: the actual usage in bytes.
				  * @retval 3rd: the virtual usage (i.e., reserved memory) in bytes.
				  */
				auto
					CollectStatisticalData()  //
					-> std::vector<std::tuple<size_t, size_t, size_t>>
				{
					std::vector<std::tuple<size_t, size_t, size_t>> stat_data{};
					const auto pid = root_.load(std::memory_order_acquire);

					CollectStatisticalData(pid, 0, stat_data);
					stat_data.emplace_back(mapping_table_.CollectStatisticalData());

					return stat_data;
				}

			private:
				/*####################################################################################
				 * Internal constants
				 *##################################################################################*/

				 /// an expected maximum height of a tree.
				static constexpr size_t kExpectedTreeHeight = 8;

				/// the NULL value for uintptr_t
				static constexpr uintptr_t kNullPtr = 0;

				/// the maximum length of keys.
				static constexpr size_t kMaxKeyLen = sizeof(Key);

				/// the length of payloads.
				static constexpr size_t kPayLen = sizeof(Payload);

				/// the length of child pointers.
				static constexpr size_t kPtrLen = sizeof(PageID);

				/// the length of record metadata.
				static constexpr size_t kMetaLen = 0;

				/// Header length in bytes.
				static constexpr size_t kHeaderLen = sizeof(Node_t);

				/// the maximum size of delta records.
				static constexpr size_t kDeltaRecSize = Delta_t::template GetMaxDeltaSize<Payload>();

				/// the expected length of keys for bulkloading.
				static constexpr size_t kBulkKeyLen = sizeof(Key);

				/// the expected length of records in leaf nodes for bulkloading.
				static constexpr size_t kLeafRecLen = kBulkKeyLen + kPayLen;

				/// the expected capacity of leaf nodes for bulkloading.
				static constexpr size_t kLeafNodeCap =
					(kPageSize - kHeaderLen - kBulkKeyLen) / (kLeafRecLen + kMetaLen);

				/// the expected length of records in internal nodes for bulkloading.
				static constexpr size_t kInnerRecLen = kBulkKeyLen + kPtrLen;

				/// the expected capacity of internal nodes for bulkloading.
				static constexpr size_t kInnerNodeCap =
					(kPageSize - kHeaderLen - kBulkKeyLen) / (kInnerRecLen + kMetaLen);

				/// a flag for preventing a consolidate-operation from splitting a node.
				static constexpr bool kIsScan = true;

				/// a flag for indicating leaf nodes.
				static constexpr bool kIsLeaf = false;

				/// a flag for indicating inner nodes.
				static constexpr auto kIsInner = true;

				/**
				 * @brief An internal enum for distinguishing a partial SMO status.
				 *
				 */
				enum SMOsRC {
					kConsolidate,
					kTrySplit,
					kTryMerge,
					kAlreadyConsolidated,
				};

				/*####################################################################################
				 * Internal utility functions
				 *##################################################################################*/

				 /**
				  * @brief Allocate or reuse a memory region for a base node.
				  *
				  * @returns the reserved memory page.
				  */
				[[nodiscard]] auto
					GetNodePage()  //
					-> Node_t*
				{
					auto* page = gc_.template GetPageIfPossible<NodePage>();
					if (page == nullptr) {
						page = dbgroup::memory::Allocate<NodePage>();
					}
					return static_cast<Node_t*>(page);
				}

				/**
				 * @brief Allocate or reuse a memory region for a delta record.
				 *
				 * @returns the reserved memory page.
				 */
				[[nodiscard]] auto
					GetRecPage()  //
					-> void*
				{
					if (tls_delta_page_) return tls_delta_page_.release();

					auto* page = gc_.template GetPageIfPossible<DeltaPage>();
					return (page == nullptr) ? (dbgroup::memory::Allocate<DeltaPage>(kDeltaRecSize)) : page;
				}

				/**
				 * @brief Add a given delta-chain to GC targets.
				 *
				 * If a given delta-chain has multiple delta records and base nodes, this function
				 * adds all of them to GC.
				 *
				 * @tparam T a templated class for simplicity.
				 * @param head the head pointer of a target delta-chain.
				 */
				template <class T>
				void
					AddToGC(const T* head)
				{
					static_assert(std::is_same_v<T, Node_t> || std::is_same_v<T, Delta_t>);

					// delete delta records
					const auto* garbage = reinterpret_cast<const Delta_t*>(head);
					while (garbage->GetDeltaType() != DeltaType::kNotDelta) {
						// register this delta record with GC
						gc_.AddGarbage<DeltaPage>(garbage);

						// if the delta record is merge-delta, delete the merged sibling node
						if (garbage->GetDeltaType() == DeltaType::kMerge) {
							auto* removed_node = garbage->template GetPayload<Node_t*>();
							gc_.AddGarbage<NodePage>(removed_node);
						}

						// check the next delta record or base node
						garbage = garbage->GetNext();
						if (garbage == nullptr) return;
					}

					// register a base node with GC
					gc_.AddGarbage<NodePage>(reinterpret_cast<const Node_t*>(garbage));
				}

				/**
				 * @brief Collect statistical data recursively.
				 *
				 * @param pid the page ID of a target node.
				 * @param level the current level in the tree.
				 * @param stat_data an output statistical data.
				 */
				void
					CollectStatisticalData(  //
						const PageID pid,
						const size_t level,
						std::vector<std::tuple<size_t, size_t, size_t>>& stat_data)
				{
					// add an element for a new level
					if (stat_data.size() <= level) {
						stat_data.emplace_back(0, 0, 0);
					}

					// get the head of the current logical ID
					const auto* head = LoadValidHead(pid);
					while (head->GetDeltaType() == DeltaType::kRemoveNode) {
						head = LoadValidHead(pid);
					}

					// add statistical data of this node
					auto& [node_num, actual_usage, virtual_usage] = stat_data.at(level);
					const auto [node_size, delta_num] = head->GetNodeUsage();
					const auto delta_size = delta_num * kDeltaRecSize;
					++node_num;
					actual_usage += node_size + delta_size;
					virtual_usage += kPageSize + delta_size;

					// collect data recursively
					if (!head->IsLeaf()) {
						// consolidate the node to traverse child nodes
						auto* page = dbgroup::memory::Allocate<NodePage>(2 * kPageSize);
						auto* consolidated = new (page) Node_t{ !kIsLeaf };
						Node_t* dummy_node = nullptr;
						TryConsolidate(head, consolidated, dummy_node, kIsScan);

						for (size_t i = 0; i < consolidated->GetRecordCount(); ++i) {
							const auto child_pid = consolidated->template GetPayload<PageID>(i);
							CollectStatisticalData(child_pid, level + 1, stat_data);
						}

						dbgroup::memory::Release<NodePage>(consolidated);
					}
				}

				/**
				 * @brief Search a child node of the top node in a given stack.
				 *
				 * @param key a search key.
				 * @param closed a flag for indicating closed/open-interval.
				 * @param stack a stack of traversed nodes.
				 * @param target_pid an optional node to prevent this function from searching a child.
				 * @retval true if the search for the target node is successful.
				 * @retval false otherwise.
				 */
				auto
					SearchChildNode(  //
						const Key& key,
						const bool closed,
						std::vector<PageID>& stack,
						const PageID target_pid = kNullPtr) const  //
					-> bool
				{
					for (uintptr_t out_ptr{}; true;) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* head = lptr->template Load<Delta_t*>();
						switch (DC::SearchChildNode(head, key, closed, out_ptr)) {
						case DeltaRC::kRecordFound:
							stack.emplace_back(out_ptr);
							break;

						case DeltaRC::kKeyIsInSibling: {
							// swap a current node in a stack and retry
							if (out_ptr == target_pid) return true;
							stack.back() = out_ptr;
							continue;
						}

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							if (stack.empty()) {
								if (target_pid != kNullPtr) return false;
								stack.emplace_back(root_.load(std::memory_order_relaxed));
							}
							else {
								if (SearchChildNode(key, closed, stack, target_pid)) return true;
								if (stack.empty()) return false;  // the tree structure has modified
							}
							continue;

						case DeltaRC::kReachBaseNode:
						default: {
							// search a child node in a base node
							const auto* node = reinterpret_cast<Node_t*>(out_ptr);
							stack.emplace_back(node->SearchChild(key, closed));
							break;
						}
						}

						return false;
					}
				}

				/**
				 * @brief Search a leaf node that may have a target key.
				 *
				 * @param key a search key.
				 * @param closed a flag for indicating closed/open-interval.
				 * @return a stack of traversed nodes.
				 */
				[[nodiscard]] auto
					SearchLeafNode(  //
						const Key& key,
						const bool closed) const  //
					-> std::vector<PageID>
				{
					std::vector<PageID> stack{};
					stack.reserve(kExpectedTreeHeight);
					stack.emplace_back(root_.load(std::memory_order_relaxed));

					// traverse a Bw-tree
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const auto* node = lptr->template Load<Node_t*>();
						if (node->IsLeaf()) return stack;
						SearchChildNode(key, closed, stack);
					}
				}

				///**
				// * @brief Search a rightmost leaf node in this tree.
				// *
				// * @return a stack of traversed nodes.
				// */
				//[[nodiscard]] auto
				//	SearchRightmostLeaf() const  //
				//	-> std::vector<PageID>
				//{
				//	std::vector<PageID> stack{};
				//	stack.reserve(kExpectedTreeHeight);
				//	stack.emplace_back(root_.load(std::memory_order_relaxed));

				//	// traverse a Bw-tree
				//	while (true) {
				//		const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
				//		const Node_t* node = lptr->template Load<Node_t*>();
				//		if (node->IsLeaf()) break;
				//		stack.emplace_back(node->GetRightmostChild());
				//	}

				//	return stack;
				//}

				/**
				 * @brief Search a leftmost leaf node in this tree.
				 *
				 * @return a stack of traversed nodes.
				 */
				[[nodiscard]] auto
					SearchLeftmostLeaf() const  //
					-> std::vector<PageID>
				{
					std::vector<PageID> stack{};
					stack.reserve(kExpectedTreeHeight);
					stack.emplace_back(root_.load(std::memory_order_relaxed));

					// traverse a Bw-tree
					while (true) {
						const auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						const Node_t* node = lptr->template Load<Node_t*>();
						if (node->IsLeaf()) break;
						stack.emplace_back(node->GetLeftmostChild());
					}

					return stack;
				}

				/**
				 * @brief Search a target node to trace a current node path.
				 *
				 * @param stack a stack of traversed nodes.
				 * @param key a search key.
				 * @param target_pid the page ID of a target node.
				 */
				void
					SearchTargetNode(  //
						std::vector<PageID>& stack,
						const Key& key,
						const PageID target_pid)
				{
					do {
						auto pid = root_.load(std::memory_order_relaxed);
						const auto* lptr = mapping_table_.GetLogicalPtr(pid);
						const auto* node = lptr->template Load<Node_t*>();
						stack.emplace_back(pid);

						while (!node->IsLeaf()) {
							if (SearchChildNode(key, kClosed, stack, target_pid)) return;
							if (stack.empty()) break;
							lptr = mapping_table_.GetLogicalPtr(stack.back());
							node = lptr->template Load<Node_t*>();
						}
					} while (stack.empty());
				}

				/**
				 * @brief Load a head of a delta chain in a given logical node.
				 *
				 * This function waits for other threads if the given logical node in SMOs.
				 *
				 * @param pid the page ID of a target node.
				 * @return a head of a delta chain.
				 */
				auto
					LoadValidHead(const PageID pid)  //
					-> const Delta_t*
				{
					const auto* lptr = mapping_table_.GetLogicalPtr(pid);
					while (true) {
						for (size_t i = 1; true; ++i) {
							const auto* head = lptr->template Load<Delta_t*>();
							if (!head->NeedWaitSMOs()) return head;
							if (i >= kRetryNum) break;
						}
						std::this_thread::sleep_for(kShortSleep);
					}
				}

				/**
				 * @brief Get the head pointer of a logical node.
				 *
				 * @param key a search key.
				 * @param closed a flag for indicating closed/open-interval.
				 * @param stack a stack of traversed nodes.
				 * @param target_pid an optional node to prevent this function from searching a head.
				 * @return the head of this logical node.
				 */
				auto
					GetHead(  //
						const Key& key,
						const bool closed,
						std::vector<PageID>& stack,
						const PageID target_pid = kNullPtr)  //
					-> const Delta_t*
				{
					for (uintptr_t out_ptr{}; true;) {
						// check whether the node is active and can include a target key
						const auto* head = LoadValidHead(stack.back());
						switch (DC::Validate(head, key, closed, out_ptr)) {
						case DeltaRC::kKeyIsInSibling: {
							// swap a current node in a stack and retry
							if (out_ptr == target_pid) return head;
							stack.back() = out_ptr;
							continue;
						}

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							if (stack.empty()) {
								if (target_pid != kNullPtr) return nullptr;
								stack = SearchLeafNode(key, closed);
							}
							else {
								SearchChildNode(key, closed, stack, target_pid);
								if (stack.empty()) return nullptr;
							}
							continue;

						case DeltaRC::kReachBaseNode:
						default:
							break;  // do nothing
						}

						return head;
					}
				}

				/**
				 * @brief Get the head pointer of a logical node and check key existence.
				 *
				 * @param key a search key.
				 * @param stack a stack of traversed nodes.
				 * @retval 1st: the head of this logical node.
				 * @retval 2nd: key existence.
				 */
				auto
					GetHeadWithKeyCheck(  //
						const Key& key,
						std::vector<PageID>& stack)  //
					-> std::pair<const Delta_t*, DeltaRC>
				{
					for (uintptr_t out_ptr{}; true;) {
						// check whether the node is active and has a target key
						const auto* head = LoadValidHead(stack.back());
						auto rc = DC::SearchRecord(head, key, out_ptr);
						switch (rc) {
						case DeltaRC::kRecordFound:
						case DeltaRC::kRecordNotFound:
							break;

						case DeltaRC::kKeyIsInSibling:
							// swap a current node in a stack and retry
							stack.back() = out_ptr;
							continue;

						case DeltaRC::kNodeRemoved:
							// retry from the parent node
							stack.pop_back();
							if (stack.empty()) {  // the tree structure has modified
								stack = SearchLeafNode(key, kClosed);
							}
							else {
								SearchChildNode(key, kClosed, stack);
							}
							continue;

						case DeltaRC::kReachBaseNode:
						default: {
							// search a target key in the base node
							rc = reinterpret_cast<Node_t*>(out_ptr)->SearchRecord(key).first;
							break;
						}
						}

						return { head, rc };
					}
				}

				/**
				 * @brief Get the head pointer of a logical node and check keys existence.
				 *
				 * @param key a search key.
				 * @param sib_key a separator key of a right-sibling node.
				 * @param stack a stack of traversed nodes.
				 * @retval 1st: the head of this logical node.
				 * @retval 2nd: key existence.
				 */
				auto
					GetHeadForMerge(  //
						const Key& key,
						const std::optional<Key>& sib_key,
						std::vector<PageID>& stack)  //
					-> std::pair<const Delta_t*, DeltaRC>
				{
					for (uintptr_t out_ptr{}; true;) {
						// check whether the node is active and has a target key
						const auto* head = LoadValidHead(stack.back());
						auto key_found = false;
						auto sib_key_found = !sib_key;
						auto rc = DC::SearchForMerge(head, key, sib_key, out_ptr, key_found, sib_key_found);
						switch (rc) {
						case DeltaRC::kRecordFound:
						case DeltaRC::kAbortMerge:
							break;

						case DeltaRC::kKeyIsInSibling:
							// swap a current node in a stack and retry
							stack.back() = out_ptr;
							continue;

						case DeltaRC::kNodeRemoved:
							rc = DeltaRC::kAbortMerge;
							break;

						case DeltaRC::kReachBaseNode:
						default: {
							const auto* node = reinterpret_cast<Node_t*>(out_ptr);
							if (!key_found) {
								if (node->SearchRecord(key).first == DeltaRC::kRecordFound) {
									key_found = true;
								}
							}
							if (!sib_key_found) {
								if (node->SearchRecord(*sib_key).first != DeltaRC::kRecordFound) {
									rc = DeltaRC::kAbortMerge;
									break;
								}
								sib_key_found = true;
							}
							rc = (key_found && sib_key_found) ? DeltaRC::kRecordFound : DeltaRC::kAbortMerge;
							break;
						}
						}

						return { head, rc };
					}
				}

				/*####################################################################################
				 * Internal scan utilities
				 *##################################################################################*/

				 /**
				  * @brief Perform consolidation for scanning.
				  *
				  * @param node a node page to store records.
				  * @param begin_key a search key.
				  * @param closed a flag for indicating closed/open-interval.
				  * @param stack a stack of traversed nodes.
				  * @return the begin position for scanning.
				  */
				auto
					ConsolidateForScan(  //
						Node_t*& node,
						const Key& begin_key,
						const bool closed,
						std::vector<PageID>& stack)  //
					-> size_t
				{
					Node_t* dummy_node = nullptr;

					while (true) {
						const auto* head = GetHead(begin_key, closed, stack);
						if (head->GetDeltaType() == DeltaType::kRemoveNode) continue;
						TryConsolidate(head, node, dummy_node, kIsScan);
						break;
					}

					// check the begin position for scanning
					const auto [rc, pos] = node->SearchRecord(begin_key);

					return (rc == DeltaRC::kRecordNotFound || closed) ? pos : pos + 1;
				}

				/**
				 * @brief Perform scanning with a given sibling node.
				 *
				 * @param sib_pid the page ID of a sibling node.
				 * @param node a node page to store records.
				 * @param begin_key a begin key (i.e., the highest key of the previous node).
				 * @param end_key an optional end key for scanning.
				 * @return the next iterator for scanning.
				 */
				auto
					SiblingScan(  //
						const PageID sib_pid,
						Node_t* node,
						const Key& begin_key,
						const ScanKey& end_key)  //
					-> RecordIterator_t
				{
					// consolidate a sibling node
					std::vector<PageID> stack{ sib_pid };
					stack.reserve(kExpectedTreeHeight);
					const auto begin_pos = ConsolidateForScan(node, begin_key, kClosed, stack);

					// check the end position of scanning
					const auto [is_end, end_pos] = node->SearchEndPositionFor(end_key);

					return RecordIterator_t{ this, node, begin_pos, end_pos, is_end };
				}

				/*####################################################################################
				 * Internal structure modifications
				 *##################################################################################*/

				 /**
				  * @brief Create a temporary array for sorting delta records.
				  *
				  */
				static auto
					CreateTempRecords()
				{
					thread_local std::array<Record, kMaxDeltaRecordNum> arr{};

					return arr;
				}

				/**
				 * @brief Try consolidation of a given node.
				 *
				 * This function will perform splitting/merging if needed.
				 *
				 * @param head a head delta record of a target delta chain.
				 * @param stack a stack of traversed nodes.
				 */
				void
					TrySMOs(  //
						Delta_t* head,
						std::vector<PageID>& stack)
				{
					thread_local std::unique_ptr<Node_t, std::function<void(void*)>>  //
						tls_node{ nullptr, dbgroup::memory::Release<NodePage> };
					Node_t* r_node = nullptr;

					// recheck other threads have modifed this delta chain
					auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
					if (head != lptr->template Load<Delta_t*>()) return;

					// prepare a consolidated node
					auto* new_node = (tls_node) ? tls_node.release() : GetNodePage();
					switch (TryConsolidate(head, new_node, r_node)) {
					case kTrySplit:
						// we use fixed-length pages, and so splitting a node must succeed
						Split(new_node, r_node, stack);
						break;

					case kTryMerge:
						if (!TryMerge(head, new_node, stack)) {
							tls_node.reset(new_node);
							return;
						}
						break;

					case kConsolidate:
					default:
						// install a consolidated node
						if (!lptr->CASStrong(head, new_node)) {
							tls_node.reset(new_node);
							return;
						}
						break;
					}
					AddToGC(head);
				}

				/**
				 * @brief Consolidate a given node.
				 *
				 * @param head the head pointer of a terget node.
				 * @param new_node a node page to store consolidated records.
				 * @param r_node a node page to store split-right records.
				 * @param is_scan a flag to prevent a split-operation.
				 * @return the status of a consolidation result.
				 */
				auto
					TryConsolidate(  //
						const Delta_t* head,
						Node_t* new_node,
						Node_t*& r_node,
						const bool is_scan = false)  //
					-> SMOsRC
				{
					thread_local std::vector<const void*> nodes{};
					nodes.reserve(kDeltaRecordThreshold);
					nodes.clear();
					auto&& records = CreateTempRecords();

					// sort delta records
					const auto rec_num = DC::Sort(head, records, nodes);

					// check whether splitting is needed
					const auto node_size = head->GetNodeSize();
					const auto do_split = !is_scan && node_size > kPageSize;

					// consolidate a target node
					const auto is_inner = !(head->IsLeaf());
					new (new_node) Node_t{ is_inner };
					if (do_split) {
						r_node = new (GetNodePage()) Node_t{ is_inner };
					}
					if (is_inner) {
						Consolidate<PageID>(new_node, r_node, nodes, records, rec_num, is_scan);
					}
					else {
						Consolidate<Payload>(new_node, r_node, nodes, records, rec_num, is_scan);
					}

					if (do_split) return kTrySplit;
					if (node_size <= kMinNodeSize) return kTryMerge;
					return kConsolidate;
				}

				/**
				 * @brief Consolidate given leaf nodes and delta records.
				 *
				 * @tparam T a class of expected payloads.
				 * @param new_node a node page to store consolidated records.
				 * @param r_node a node page to store split-right records.
				 * @param nodes the set of leaf nodes to be consolidated.
				 * @param arr insert/modify/delete-delta records.
				 * @param rec_num The number of delta records.
				 * @param is_scan a flag to prevent a split-operation.
				 */
				template <class T>
				void
					Consolidate(  //
						Node_t* new_node,
						Node_t* r_node,
						const std::vector<const void*>& nodes,
						const std::array<Record, kMaxDeltaRecordNum>& arr,
						const size_t new_rec_num,
						const bool is_scan)
				{
					constexpr auto kIsSplitLeft = true;
					auto* l_node = (r_node != nullptr) ? new_node : nullptr;

					// perform merge-sort to consolidate a node
					size_t offset = kPageSize * (is_scan ? 2 : 1);
					size_t j = 0;
					for (int64_t k = nodes.size() - 1; k >= 0; --k) {
						const auto* node = reinterpret_cast<const Node_t*>(nodes[k]);
						const auto node_rec_num = node->GetRecordCount();

						// check a null key for inner nodes
						size_t i = 0;
						if (!node->IsLeaf() && node->IsLeftmost()) {
							offset = Node_t::template CopyRecordFrom<T>(new_node, node, i++, offset, r_node);
						}
						for (; i < node_rec_num; ++i) {
							// copy new records
							const auto& node_key = node->GetKey(i);
							for (; j < new_rec_num && Comp{}(arr[j].key, node_key); ++j) {
								offset = Node_t::template CopyRecordFrom<T>(new_node, arr[j].ptr, offset, r_node);
							}

							// check a new record is updated one
							if (j < new_rec_num && !Comp{}(node_key, arr[j].key)) {
								offset = Node_t::template CopyRecordFrom<T>(new_node, arr[j++].ptr, offset, r_node);
							}
							else {
								offset = Node_t::template CopyRecordFrom<T>(new_node, node, i, offset, r_node);
							}
						}
					}

					// copy remaining new records
					for (; j < new_rec_num; ++j) {
						offset = Node_t::template CopyRecordFrom<T>(new_node, arr[j].ptr, offset, r_node);
					}

					// copy the lowest/highest keys
					if (l_node == nullptr) {
						// consolidated node
						offset = new_node->CopyLowKeyFrom(nodes.back());
						new_node->CopyHighKeyFrom(nodes.front(), offset);
					}
					else {
						// split nodes
						offset = l_node->CopyLowKeyFrom(nodes.back());
						l_node->CopyHighKeyFrom(new_node, offset, kIsSplitLeft);
						offset = new_node->CopyLowKeyFrom(new_node);
						new_node->CopyHighKeyFrom(nodes.front(), offset);
					}

					if (is_scan) {
						new_node->SetNodeSizeForScan();
					}
				}

				/**
				 * @brief Try splitting a target node.
				 *
				 * @param l_node a split-left node to be updated.
				 * @param r_node a split-right node to be inserted to this tree.
				 * @param stack a stack of traversed nodes.
				 */
				void
					Split(  //
						Node_t* l_node,
						const Node_t* r_node,
						std::vector<PageID>& stack)
				{
					// install the split nodes
					const auto r_pid = mapping_table_.GetNewPageID();
					auto* r_lptr = mapping_table_.GetLogicalPtr(r_pid);
					r_lptr->Store(r_node);
					l_node->SetNext(r_pid);
					const auto l_pid = stack.back();
					auto* l_lptr = mapping_table_.GetLogicalPtr(l_pid);
					l_lptr->Store(l_node);
					stack.pop_back();  // remove the split child node to modify its parent node

					// create an index-entry delta record to complete split
					const auto* r_node_d = reinterpret_cast<const Delta_t*>(r_node);
					auto* entry_d = new (GetRecPage()) Delta_t{ DeltaType::kInsert, r_node_d, r_pid };
					const auto& key = r_node->GetLowKey();
					const auto rec_len = r_node->GetLowKeyLen() + kPtrLen + kMetaLen;

					while (true) {
						// check the current node is a root node
						if (stack.empty()) {
							if (TryRootSplit(entry_d, l_pid)) {
								tls_delta_page_.reset(entry_d);
								return;
							}
							SearchTargetNode(stack, key, r_pid);
							stack.pop_back();  // remove the split node
							continue;
						}

						// insert the delta record into a parent node
						while (true) {
							const auto* head = GetHead(key, kClosed, stack, r_pid);
							if (head == nullptr) break;  // the tree structure has modified, so retry

							// try to insert the index-entry delta record
							entry_d->SetNext(head, rec_len);
							auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
							if (lptr->CASWeak(head, entry_d)) {
								if (entry_d->NeedConsolidation()) {
									TrySMOs(entry_d, stack);
								}
								return;
							}
						}
					}
				}

				/**
				 * @brief Perform splitting a root node.
				 *
				 * @param entry_d a insert-entry delta record.
				 * @param old_pid a logical node ID of an old root node.
				 * @retval true if splitting succeeds.
				 * @retval false otherwise.
				 */
				auto
					TryRootSplit(  //
						const Delta_t* entry_d,
						const PageID old_pid)  //
					-> bool
				{
					if (root_.load(std::memory_order_relaxed) != old_pid) return false;

					// create a new root node
					const auto* entry_delta_n = reinterpret_cast<const Node_t*>(entry_d);
					auto* new_root = new (GetNodePage()) Node_t{ entry_delta_n, old_pid };
					const auto new_pid = mapping_table_.GetNewPageID();
					mapping_table_.GetLogicalPtr(new_pid)->Store(new_root);

					// install a new root page
					root_.store(new_pid, std::memory_order_relaxed);
					return true;
				}

				/**
				 * @brief
				 *
				 * @param head the head pointer of a terget node.
				 * @param removed_node a removed (i.e., merged) node.
				 * @param stack a stack of traversed nodes.
				 * @retval true if partial merging succeeds.
				 * @retval false otherwise.
				 */
				auto
					TryMerge(  //
						const Delta_t* head,
						Node_t* removed_node,
						std::vector<PageID>& stack)  //
					-> bool
				{
					auto* removed_node_d = reinterpret_cast<Delta_t*>(removed_node);

					// insert a remove-node delta to prevent other threads from modifying this node
					auto* remove_d = new (GetRecPage()) Delta_t{ removed_node->IsLeaf() };
					const auto rem_pid = stack.back();
					auto* rem_lptr = mapping_table_.GetLogicalPtr(rem_pid);
					if (!rem_lptr->CASStrong(head, remove_d)) {
						tls_delta_page_.reset(remove_d);
						return false;
					}
					stack.pop_back();  // remove the child node

					// remove the index entry before merging
					const auto del_key_len = removed_node->GetLowKeyLen();
					const auto& del_key = component::DeepCopy<Key>(removed_node->GetLowKey(), del_key_len);
					auto* delete_d = TryDeleteIndexEntry(removed_node_d, del_key, del_key_len, stack);
					if (delete_d == nullptr) {
						// check this tree should be shrinked
						if (!TryRemoveRoot(removed_node, rem_pid, stack)) {
							// merging has failed, but consolidation succeeds
							rem_lptr->Store(removed_node);
							AddToGC(remove_d);
						}
						return true;
					}

					// insert a merge delta into the left sibling node
					const auto rem_uintptr = reinterpret_cast<uintptr_t>(removed_node);
					auto* merge_d = new (GetRecPage()) Delta_t{ DeltaType::kMerge, removed_node_d, rem_uintptr };
					const auto diff = removed_node->GetNodeDiff();
					while (true) {
						if (stack.empty()) {
							// concurrent SMOs have modified the tree structure, so reconstruct a stack
							SearchTargetNode(stack, del_key, rem_pid);
						}
						else {
							SearchChildNode(del_key, kOpen, stack, rem_pid);
							if (stack.empty()) continue;
						}

						while (true) {  // continue until insertion succeeds
							const auto* sib_head = GetHead(del_key, kOpen, stack, rem_pid);
							if (sib_head == nullptr) break;  // retry from searching the left sibling node

							// try to insert the merge-delta record
							merge_d->SetNext(sib_head, diff);
							auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
							if (lptr->CASWeak(sib_head, merge_d)) {
								delete_d->SetSiblingPID(stack.back());  // set a shortcut
								if (merge_d->NeedConsolidation()) {
									TrySMOs(merge_d, stack);
								}
								return true;
							}
						}
					}
				}

				/**
				 * @brief Complete partial merging by deleting index-entry from this tree.
				 *
				 * @param removed_node a consolidated node to be removed.
				 * @param del_key a lowest key of a removed node.
				 * @param del_key_len the length of the lowest key.
				 * @param stack a copied stack of traversed nodes.
				 * @retval the delete-delta record if successful.
				 * @retval nullptr otherwise.
				 */
				auto
					TryDeleteIndexEntry(  //
						const Delta_t* removed_node,
						const Key& del_key,
						const size_t del_key_len,
						std::vector<PageID> stack)  //
					-> Delta_t*
				{
					// check a current node can be merged
					if (stack.empty()) return nullptr;     // a root node cannot be merged
					if (del_key_len == 0) return nullptr;  // the leftmost nodes cannot be merged

					// insert the delta record into a parent node
					auto* delete_d = new (GetRecPage()) Delta_t{ removed_node };
					const auto rec_len = delete_d->GetKeyLength() + kPtrLen + kMetaLen;
					const auto& sib_key = removed_node->GetHighKey();
					while (true) {
						// check the removed node is not leftmost in its parent node
						auto [head, rc] = GetHeadForMerge(del_key, sib_key, stack);
						if (rc == DeltaRC::kAbortMerge) {
							// the leftmost nodes cannot be merged
							tls_delta_page_.reset(delete_d);
							return nullptr;
						}

						// try to insert the index-delete delta record
						delete_d->SetNext(head, rec_len); // delete_d->SetNext(head, -rec_len);
						auto* lptr = mapping_table_.GetLogicalPtr(stack.back());
						if (lptr->CASWeak(head, delete_d)) break;
					}

					if (delete_d->NeedConsolidation()) {
						TrySMOs(delete_d, stack);
					}
					return delete_d;
				}

				/**
				 * @brief Remove a root node and shrink a tree.
				 *
				 * @param root an old root node to be removed.
				 * @param old_pid the page ID of an old root node.
				 * @param stack a stack of ancestor nodes.
				 * @retval true if a root node is removed.
				 * @return false otherwise.
				 */
				auto
					TryRemoveRoot(  //
						const Node_t* root,
						PageID old_pid,
						std::vector<PageID>& stack)  //
					-> bool
				{
					// check a given node can be shrinked
					if (!stack.empty() || root->GetRecordCount() > 1 || root->IsLeaf()) return false;

					// shrink the tree by removing a useless root node
					const auto new_pid = root->GetLeftmostChild();
					auto* new_lptr = mapping_table_.GetLogicalPtr(new_pid);
					if (new_lptr->template Load<Node_t*>()->IsLeaf()
						|| !root_.compare_exchange_strong(old_pid, new_pid, std::memory_order_relaxed)) {
						return false;
					}
					AddToGC(root);
					return true;
				}

				/*####################################################################################
				 * Internal bulkload utilities
				 *##################################################################################*/

				 /**
				  * @brief Bulkload specified kay/payload pairs with a single thread.
				  *
				  * Note that this function does not create a root node. The main process must create a
				  * root node by using the nodes constructed by this function.
				  *
				  * @tparam Entry a container of a key/payload pair.
				  * @param iter the begin position of target records.
				  * @param n the number of entries to be bulkloaded.
				  * @retval 1st: the height of a constructed tree.
				  * @retval 2nd: constructed nodes in the top layer.
				  */
				template <class Entry>
				auto
					BulkloadWithSingleThread(  //
						BulkIter<Entry> iter,
						const size_t n)  //
					-> BulkResult
				{
					// construct a data layer (leaf nodes)
					auto&& nodes = ConstructSingleLayer<Entry>(iter, n, kIsLeaf);

					// construct index layers (inner nodes)
					size_t height = 1;
					for (auto n = nodes.size(); n > kInnerNodeCap; n = nodes.size(), ++height) {
						// continue until the number of inner nodes is sufficiently small
						nodes = ConstructSingleLayer<NodeEntry>(nodes.cbegin(), n, kIsInner);
					}

					return { height, std::move(nodes) };
				}

				/**
				 * @brief Construct nodes based on given entries.
				 *
				 * @tparam Entry a container of a key/payload pair.
				 * @param iter the begin position of target records.
				 * @param n the number of entries to be bulkloaded.
				 * @param is_inner a flag for indicating inner nodes.
				 * @return constructed nodes.
				 */
				template <class Entry>
				auto
					ConstructSingleLayer(  //
						BulkIter<Entry> iter,
						const size_t n,
						const bool is_inner)  //
					-> std::vector<NodeEntry>
				{
					// reserve space for nodes in the upper layer
					std::vector<NodeEntry> nodes{};
					nodes.reserve((n / (is_inner ? kInnerNodeCap : kLeafNodeCap)) + 1);

					// load child nodes into parent nodes
					const auto& iter_end = iter + n;
					for (Node_t* prev_node = nullptr; iter < iter_end;) {
						auto* node = new (GetNodePage()) Node_t{ is_inner };
						const auto pid = mapping_table_.GetNewPageID();
						auto* lptr = mapping_table_.GetLogicalPtr(pid);
						lptr->Store(node);
						node->template Bulkload<Entry>(iter, iter_end, prev_node, pid, nodes, is_inner);
						prev_node = node;
					}

					return nodes;
				}

				/*####################################################################################
				 * Static assertions
				 *##################################################################################*/

				 /**
				  * @retval true if a target key class is trivially copyable.
				  * @retval false otherwise.
				  */
				[[nodiscard]] static constexpr auto
					KeyIsTriviallyCopyable()  //
					-> bool
				{
					// check a given key type is trivially copyable
					return std::is_trivially_copyable_v<Key>;
				}

				// target keys must be trivially copyable.
				static_assert(KeyIsTriviallyCopyable());

				// target payloads must be trivially copyable.
				static_assert(std::is_trivially_copyable_v<Payload>);

				// node pages have sufficient capacity for records.
				static_assert(kMaxKeyLen + kPayLen <= kPageSize / 4);

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// a root node of this Bw-tree.
				std::atomic_uint64_t root_{};

				/// a table to map logical IDs with physical pointers.
				MappingTable_t mapping_table_{};

				/// a garbage collector of base nodes and delta records.
				NodeGC_t gc_{};

				/// a thread-local delta-record page to reuse
				inline static thread_local std::unique_ptr<void, std::function<void(void*)>>  //
					tls_delta_page_{ nullptr, dbgroup::memory::Release<DeltaPage> };           // NOLINT
			};
		};  // namespace dbgroup::index::bw_tree

		template<class _type_> class GarbageCollectedAllocator {
		private:
			synchronization::atomic_number<long> active;
			fibers::utilities::dbgroup::index::bw_tree::BwTree< _type_*, bool> ptrs;
			utilities::dbgroup::memory::EpochBasedGC < fibers::utilities::dbgroup::memory::Target< _type_ > > gc{ 1E3, 1 };
			
		public:
			// Prevents deletion of elements until at least 3 Epochs after the current Epoch. Allows use of ptr's in one thread while another thread can still queue for deletion of ptr's. 
			auto CreateEpochGuard() {
				return gc.CreateEpochGuard();
			};
			// Create a new ptr that will be deleted by the allocator on destruction or 
			template <typename... TArgs> _type_* Alloc(TArgs const&... a) {
				_type_* ptr = new _type_(a...);
				ptrs.Write(ptr, false);
				active.Increment();
				return ptr;
			};
			// Requests to free / delete the element. Note that this does not delete the element immediately -- and it is delayed for as long as an older EpochGuard exists. 
			void Free(_type_* element) {
				ptrs.Delete(element); 
				gc.Push(element);
				active.Decrement();
				// do NOT clear the ptr or call the destructor yet -- leave that to the garbage collector. 
			};
			// returns the number of "live" ptrs, approximately
			long long			GetAllocCount() { return active.load(); }

			// Initializes the garbage collector automatically. 
			GarbageCollectedAllocator() : active{ 0 }, ptrs(), gc() {
				gc.StartGC(); 
			};
			GarbageCollectedAllocator(GarbageCollectedAllocator&) = delete;
			GarbageCollectedAllocator(GarbageCollectedAllocator&&) = delete;
			GarbageCollectedAllocator& operator=(GarbageCollectedAllocator const&) = delete; 
			GarbageCollectedAllocator& operator=(GarbageCollectedAllocator&&) = delete;
			// Clears the garbage collector. 
			~GarbageCollectedAllocator() {				
				for (auto iter = ptrs.Scan(); iter; iter++) {
					gc.Push(iter.GetKey());
				}
			};
		};
	};



	namespace containers {
		/* Fiber- and thread-safe sorted container for time-series patterns, where the x- and y-values may be integers, floating numbers, long doubles, etc. */
		template <typename xType, typename yType> class Pattern {
		protected:
			using underlying = fibers::utilities::dbgroup::index::bw_tree::BwTree< typename utilities::impl::CAS_Safe_Type < xType >::type, typename utilities::impl::CAS_Safe_Type < yType >::type >;
			std::shared_ptr< underlying > data;

		public:
			enum class interp_t { SPLINE, LEFT, RIGHT, LINEAR };

			Pattern() : data(std::make_shared<underlying>()) {};
			Pattern(Pattern const& r) : data(std::make_shared<underlying>()) {
				operator=(r);
			}
			Pattern(Pattern&& r) = default;
			Pattern& operator=(Pattern const& r) {
				if (static_cast<void*>(this) != static_cast<const void*>(&r)) {
					Pattern out;
					//for (auto& x : r)
					//	out->push_back(x);
					out.data.swap(data);
				}
				return *this;
			};
			Pattern& operator=(Pattern&& r) = default;
			~Pattern() = default;

			typename underlying::RecordIterator_t FindSmallestLargerEqual(xType position, std::optional<xType> const& end = std::nullopt) {
				if (end.has_value()) {
					return data->FindSmallestLargerEqual(position, end.value());
				}
				else {
					return data->FindSmallestLargerEqual(position);
				}
			};
			typename underlying::RecordIterator_t FindLargestSmallerEqual(xType position, std::optional<xType> const& end = std::nullopt) {
				if (end.has_value()) {
					return data->FindLargestSmallerEqual(position, end.value());
				}
				else {
					return data->FindLargestSmallerEqual(position);
				}
			};
			std::optional<yType> Read(xType position) {
				auto payload = data->Read(position);
				if (payload.has_value()) {
					return (yType)payload.value();
				}
				else {
					return std::nullopt;
				}
			};
			std::optional<xType> GetMinTime() {
				auto iter = FindSmallestLargerEqual(-std::numeric_limits<xType>::max());
				if (iter) {
					return (xType)iter.GetKey(); // iter.GetPayload()
				}
				else {
					return std::nullopt;
				}
			};
			std::optional<xType> GetMaxTime() {
				auto iter = FindLargestSmallerEqual(std::numeric_limits<xType>::max());
				if (iter) {
					return (xType)iter.GetKey();
				}
				else {
					return std::nullopt;
				}
			};
			//size_t GetNumValues();
			//std::optional<yType> GetMinValue();
			//std::optional<yType> GetMaxValue();
			//std::optional<yType> GetCurrentValue(xType position, interp_t interpolationType = interp_t::LINEAR);
			bool Delete(xType position) {
				return data->Delete(position) == fibers::utilities::dbgroup::index::bw_tree::ReturnCode::kSuccess;
			};
			bool Insert(xType position, yType value, bool overwiteIfExists = true) {
				if (overwiteIfExists) {
					return data->Write(position, value) == fibers::utilities::dbgroup::index::bw_tree::ReturnCode::kSuccess;
				}
				else {
					return data->Insert(position, value) == fibers::utilities::dbgroup::index::bw_tree::ReturnCode::kSuccess;
				}
			};
			typename underlying::RecordIterator_t Scan(std::optional<xType> start = std::nullopt, std::optional<xType> end = std::nullopt) {
				using scanKeyType = typename underlying::ScanKey;
				if (start.has_value()) {
					if (end.has_value()) {
						return data->FindSmallestLargerEqual(start.value(), end.value());
					}
					else {
						return data->FindSmallestLargerEqual(start.value());
					}
				}
				else {
					if (end.has_value()) {
						return data->Scan(
							std::nullopt,
							scanKeyType(
								std::tuple<const typename utilities::impl::CAS_Safe_Type < xType >::type&, size_t, bool> {
							end.value(), sizeof(typename utilities::impl::CAS_Safe_Type < xType >::type), true
						}
						)
						);
					}
					else {
						return data->Scan();
					}
				}
			};

			struct Iterator : public std::iterator<std::forward_iterator_tag, std::pair<xType, yType>> {
			public:
				using difference_type = typename std::iterator<std::forward_iterator_tag, std::pair<xType, yType>>::difference_type;

				Iterator() = default;
				Iterator(Pattern*&& _parent, int pos, std::optional<xType> const& _start = std::nullopt, std::optional<xType> const& _end = std::nullopt) :
					start{ std::nullopt }
					, parent{ std::forward<Pattern*>(_parent) }
					, _ptr{ begin(_parent, _start, _end) }
					, result{}
					, position{ pos }
				{
					if (_start.has_value()) start.emplace(_start.value());
					while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator(const Iterator& rhs) :
					start{ std::nullopt }
					, parent{ rhs.parent }
					, _ptr{ begin(rhs.parent, rhs.start, rhs._ptr.GetEndKey<xType>()) }
					, result{}
					, position{ rhs.position }
				{
					if (rhs.start.has_value()) start.emplace(rhs.start.value());
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator(Iterator&& rhs) = default; /* : start{ std::nullopt }
					, parent{ rhs.parent }
					, _ptr{ begin(rhs.parent, rhs.start, rhs._ptr.GetEndKey<xType>()) }
					, result{}
					, position{ rhs.position }
				{
					if (rhs.start.has_value()) start.emplace(rhs.start.value());
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				}; */
				Iterator& operator=(const Iterator& rhs) {
					start.reset();;
					parent = rhs.parent;
					_ptr = begin(rhs.parent, rhs.start, rhs._ptr.GetEndKey<xType>());
					position = rhs.position;

					if (rhs.start.has_value()) start.emplace(rhs.start.value());
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				Iterator& operator=(Iterator&& rhs) {
					start.reset();;
					parent = rhs.parent;
					_ptr = begin(rhs.parent, rhs.start, rhs._ptr.GetEndKey<xType>());
					position = rhs.position;

					if (rhs.start.has_value()) start.emplace(rhs.start.value());
					int pos = position; while (pos > 0 && _ptr) { ++_ptr;  --pos; }
				};
				~Iterator() = default;

				std::pair<xType, yType>& operator*() const { LoadResult(); return result; };
				std::pair<xType, yType>* operator->() const { LoadResult(); return &result; };

				Iterator& operator++() { Increment(); return *this; }
				// Iterator operator++(int) { Iterator out{ *this }; Increment(); return out; }

				explicit operator bool() const { return Valid(); };
				bool operator==(const Iterator& rhs) const {
					if (!rhs.Valid() && !Valid()) return true;
					if (!rhs.Valid()) {
						return !Valid();
					}
					if (!Valid()) {
						return !rhs.Valid();
					}
					return _ptr == rhs._ptr;
				}
				bool operator!=(const Iterator& rhs) const { return !operator==(rhs); }

			private:
				bool Valid() const { return (bool)_ptr; };
				void Increment() { if (Valid()) ++_ptr; ++position; };
				void LoadResult() const { if (Valid()) result = { (xType)_ptr.GetKey(), (yType)_ptr.GetPayload() }; };

				static typename underlying::RecordIterator_t begin(Pattern* parent, std::optional<xType> const& start = std::nullopt, std::optional<xType> const& end = std::nullopt) {
					if (parent) {
						if (start.has_value()) {
							if (end.has_value()) {
								return parent->Scan(start.value(), end.value());
							}
							else {
								return parent->Scan(start.value());
							}
						}
						else {
							if (end.has_value()) {
								return parent->Scan(std::nullopt, end.value());
							}
							else {
								return parent->Scan();
							}
						}
					}
					return typename underlying::RecordIterator_t();
				};

			public:
				std::optional<xType> start{ std::nullopt };
				Pattern* parent{ nullptr };
				mutable typename underlying::RecordIterator_t _ptr{};
				mutable std::pair<xType, yType> result{};
				int position{ 0 };

			};
			using iterator = Iterator;
			using const_iterator = Iterator;

			Iterator begin(std::optional<xType> const& start = std::nullopt, std::optional<xType> const& end = std::nullopt) const { return Iterator(const_cast<Pattern*>(this), 0, start, end); };
			Iterator end() const { return Iterator(nullptr, 0, std::nullopt, std::nullopt); };
			Iterator cbegin(std::optional<xType> start = std::nullopt, std::optional<xType> end = std::nullopt) const { return begin(start, end); };
			Iterator cend() const { return end(); };
		};

		/* *THREAD SAFE* Thread-safe and fiber-safe wrapper for any type of number, from integers to doubles.
		   Significant performance boost if the data type is an integer type or one of: long, unsigned int, unsigned long, unsigned __int64
		   Slower, but still atomic using multi-word CAS algorithms, if using floating-point numbers like doubles or floats.
		*/
		template<typename _Value_type> using number = fibers::synchronization::atomic_number<_Value_type>;
	
		/* *THREAD SAFE* Thread- and Fiber-safe queue with Last-In-First-Out functionality.
		* POD types are stored by-value, non-POD types are stored as shared_ptr's. 
		* POD types have a speed-up by avoiding destructor calls.
		* Example: [1,2]. Push(5) -> [1,2,5]. Pop(&Out) -> [1,2] & sets Out to 5. 
		*/
		template <typename value> class Stack {
		private:
			static auto GetValueStorageType() {
				if constexpr (std::is_pod<value>::value) {
					value out{};
					return out;
				}
				else {
					auto out{ std::make_shared<value>() };
					return out;
				}
			};
			static auto MakeValueStorageType(value const& v) {
				if constexpr (std::is_pod<value>::value) {
					return static_cast<value const&>(v);
				}
				else {
					return std::make_shared<value>(v);
				}
			};
			static auto MakeValueStorageType(value && v) {
				if constexpr (std::is_pod<value>::value) {
					return static_cast<value const&>(v);
				}
				else {
					return std::make_shared<value>(std::forward<value>(v));
				}
			};
			using ValueStorageType = typename fibers::utilities::function_traits<decltype(std::function(GetValueStorageType))>::result_type;
			static value& GetValueFromStorageType(ValueStorageType& v) {
				if constexpr (std::is_pod<value>::value) {
					return v;
				}
				else {
					return *v;
				}
			};

			/* storage class for each node in the stack. May or may not be POD, depending on the value type being stored. */
			class linkedListItem {
			public:
				ValueStorageType value; // may be copied-as-value or may be a shared_ptr. May or may not be POD. 
				synchronization::atomic_ptr<linkedListItem> prev; // ptr to the "prev" ptr. Non-POD but can be "forgotten" without penalty
				
				linkedListItem() : value(), prev(nullptr) {}
				linkedListItem(ValueStorageType const& mvalue) : value(mvalue), prev(nullptr) {}
				linkedListItem(linkedListItem const&) = default;
				linkedListItem(linkedListItem &&) = default;
				linkedListItem& operator=(linkedListItem const&) = default;
				linkedListItem& operator=(linkedListItem&&) = default;
				~linkedListItem() = default;
			};

			/* allocator is forced to use POD optimization if the value type is POD. */
			utilities::GarbageCollectedAllocator< linkedListItem > nodeAlloc;
			/* the current "tip" of the stack, which will be pop'd on request. */
			synchronization::atomic_ptr<linkedListItem> head;

		public:
			Stack() = default;
			Stack(Stack const&) = delete;
			Stack(Stack &&) = delete;
			Stack& operator=(Stack const&) = delete;
			Stack& operator=(Stack&&) = delete;
			~Stack() { clear(); };

			/**
			 * @brief current size of the list.
			 * @return number of items in list
			 */
			unsigned long size() const {
				return nodeAlloc.GetAllocCount();
			};

			/**
			 * @brief clears the list.
			 * @return void
			 */
			void clear() {
				linkedListItem* p = head.Set(nullptr);
				linkedListItem* n = nullptr;

				while (p) {
					n = p->prev.Set(nullptr);
					nodeAlloc.Free(p);
					p = n;
					if (!p) // At end, lets check if new stuff came in
						p = head.Set(nullptr);
				}

				//nodeAlloc.Clean();
			};

			/**
			 * @brief pushes a copy of Value at the end of the list.
			 * @return void
			 */
			void push(value const& Value) {
				auto* node = nodeAlloc.Alloc(); {
					node->value = MakeValueStorageType(Value);
				}

				while (true) {
					node->prev = head.load();
					if (head.CompareExchange(node->prev, node) == node->prev) {
						break;
					}
				}
			};

			/**
			 * @brief pushes a copy of Value at the end of the list.
			 * @return void
			 */
			void push(value && Value) {
				auto* node = nodeAlloc.Alloc(); {
					node->value = MakeValueStorageType(std::forward<value>(Value));
				}

				while (true) {
					node->prev = head.load();
					if (head.CompareExchange(node->prev, node) == node->prev) {
						break;
					}
				}
			};

			/**
			 * @brief pushes a shared_ptr of the Value at the end of the list. Only available if value type is non-POD. (POD data are stored-by-value and a shared_ptr would not be respected)
			 * @return void
			 */
			template<typename = std::enable_if_t<!std::is_pod<value>::value>> void push(std::shared_ptr<value> const& Value) {
				auto* node = nodeAlloc.Alloc(); {
					node->value = Value;
				}

				while (true) {
					node->prev = head.load();
					if (head.CompareExchange(node->prev, node) == node->prev) {
						break;
					}
				}
			};

			/**
			 * @brief Searches for the first match with Value and removes it from the list.
			 * @return successful or not
			 */
			bool try_remove(value const& Value) {
				decltype(auto) guard { nodeAlloc.CreateEpochGuard() };

				bool found = false;
				linkedListItem* next = nullptr;
				linkedListItem* current = head.load(); 
				if (current) {
					linkedListItem* prev = current->prev.load();
					while (!found && current) {
						if (GetValueFromStorageType(current->value) == Value) {
							// found it
							if (next) {
								// set it's next->prev to prev, not current
								if (next->prev.CompareExchange(current, prev) == current) {
									found = true;
									break;
								}
								else {
									// something interupted -- try again
									continue;
								}
							}
							else {
								// no "next" implies current was the head.
								if (head.CompareExchange(current, prev) == current) {
									found = true;
									break;
								}
								else {
									continue;
								}
							}
						}

						next = current;
						current = prev;
						if (current) {
							prev = current->prev.load();
						}
						else {
							prev = nullptr;
						}
					}
					if (found && current) {
						nodeAlloc.Free(current);
					}
				}
				return found;
			};

			/**
			 * @brief Trys to retrieve the item at the end of the list. If found, sets Out to that value.
			 * @return successful or not, and will set Out if successful
			 */
			bool try_pop(ValueStorageType& out) {
				decltype(auto) guard{ nodeAlloc.CreateEpochGuard() };

				linkedListItem* current{ head.load() };
				linkedListItem* prev{ nullptr };
				bool found = false;

				while (!found && current) {
					current = head.load();
					prev = current->prev.load();
					// no "next" implies current was the head.
					if (head.CompareExchange(current, prev) == current) {
						found = true;
						break;
					}
					else { // something interupted -- try again
						continue;
					}
				}
				if (found) {
					out = std::move(current->value);
					nodeAlloc.Free(current);
				}
				return found;
			};

			/**
			 * @brief Trys to remove the last (most recent) element from the list. 
			 * @return successful or not
			 */
			bool try_pop() {
				decltype(auto) guard{ nodeAlloc.CreateEpochGuard() };

				linkedListItem* current{ head.load() };
				linkedListItem* prev{ nullptr };
				bool found = false;

				while (!found && current) {
					current = head.load();
					prev = current->prev.load();
					// no "next" implies current was the head.
					if (head.CompareExchange(current, prev) == current) {
						found = true;
						break;
					}
					else { // something interupted -- try again
						continue;
					}
				}
				if (found) {
					nodeAlloc.Free(current);
				}
				return found;
			};

			/**
			 * @brief Trys to retrieve the item at the end of the list. If found, sets Out to that value.
			 * @return successful or not, and will set Out if successful
			 */
			template<typename = std::enable_if_t<!std::is_pod<value>::value>> bool try_pop(value& out) {
				decltype(auto) guard{ nodeAlloc.CreateEpochGuard() };

				linkedListItem* current{ head.load() };
				linkedListItem* prev{ nullptr };
				bool found = false;

				while (!found && current) {
					current = head.load();
					prev = current->prev.load();
					// no "next" implies current was the head.
					if (head.CompareExchange(current, prev) == current) {
						found = true;
						break;
					}
					else { // something interupted -- try again
						continue;
					}
				}
				if (found) {
					out = std::move(*current->value);
					nodeAlloc.Free(current);
				}
				return found;
			};

			/**
			 * @brief counts how many times the Value is found in the list.
			 * @return number of matched items in list
			 */
			unsigned long count(value const& Value, unsigned long maxCount = std::numeric_limits<unsigned long>::max())  {
				decltype(auto) guard{ nodeAlloc.CreateEpochGuard() };

				linkedListItem* current = head.load();
				unsigned long counted{ 0 };
				while (current && (counted < maxCount)) {
					if (GetValueFromStorageType(current->value) == Value) {
						counted++;
					}
					current = current->prev.load();
				}
				return counted;
			};

			/**
			 * @brief Determines if the Value is in the list.
			 * @return whether or not the Value was found
			 */
			bool contains(value const& Value)  {
				return count(Value, 1) > 0;
			};

		};

		template <typename value> class AtomicQueue {
		public:
			/* storage class for each node in the stack. May or may not be POD, depending on the value type being stored. */
			class linkedListItem {
			public:
				value Value; // may be copied-as-value or may be a shared_ptr. May or may not be POD. 
				linkedListItem* prev;
				linkedListItem* next;

				linkedListItem(value const& mvalue, linkedListItem* mprev = nullptr, linkedListItem* mnext = nullptr) : Value(mvalue), prev(mprev), next(mnext) {}
				linkedListItem() = default; 
				linkedListItem(linkedListItem const&) = default;
				linkedListItem(linkedListItem&&) = default;
				linkedListItem& operator=(linkedListItem const&) = default;
				linkedListItem& operator=(linkedListItem&&) = default;
				~linkedListItem() = default;
			};
			// GarbageCollected
			fibers::synchronization::shared_mutex<fibers::synchronization::mutex> deleteGuard;
			utilities::Allocator< linkedListItem, 128, std::is_pod<value>::value > nodeAlloc; /* the allocator, which supports deferred deletion when we want to access the data of a node that may have been deleted concurrently. */
			linkedListItem* head; /* the current "beginning" of the line, which will be pushed forward on request. */
			linkedListItem* tail; /* the current "end" of the line, which will be pop'd (e.g. somebody leaves the line) on request. */

			// try to add new element to the start of the line. May fail under competition with other threads.
			bool try_push(value const& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				auto* newNode = nodeAlloc.Alloc(element);
				if (true) {
					newNode->prev = newNode;
					newNode->next = newNode;

					linkedListItem* currentHead; 
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}
					if (currentHead && currentTail) {
						// there is already a head and tail. 
						linkedListItem* currentHeadNext; {
							auto container{ fibers::utilities::MultiItemCAS(
								&currentHead->next
							) };
							currentHeadNext = container.Read<0>();
						}

						if (currentHeadNext == currentHead) {
							// the head points to itself -- e.g. the list only has one element
							newNode->next = currentHead;
							newNode->prev = currentHead;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->next, // becomes newNode
								&currentHead->prev, // becomes newNode
								& tail // becomes currentHead
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentHead, currentHead, currentTail);
							if (container.TrySwap(OldValues, newNode, newNode, newNode, currentHead)) return true;
						}
						else {
							// the head points to something -- e.g. the list has two or more elements
							newNode->next = currentHead;
							newNode->prev = currentTail;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->prev, // becomes newNode
								& currentTail->next // becomes newNode
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentHead);
							if (container.TrySwap(OldValues, newNode, newNode, newNode)) return true;
						}
					}
					else {
						// if there is no head, try to swap the head and tail for the new node.
						fibers::utilities::Union< linkedListItem*, linkedListItem*> OldValues(nullptr, nullptr);
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						if (container.TrySwap(OldValues, newNode, newNode)) return true; 						
					}
				}
				nodeAlloc.Free(newNode); // easy replacement
				return false;
			}
			// add new element to the start of the line. Will always succeed, eventually. 
			bool push(value const& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				auto* newNode = nodeAlloc.Alloc(element);
				while (true) {
					newNode->prev = newNode;
					newNode->next = newNode;

					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}
					if (currentHead && currentTail) {
						// there is already a head and tail. 
						linkedListItem* currentHeadNext; {
							auto container{ fibers::utilities::MultiItemCAS(
								&currentHead->next
							) };
							currentHeadNext = container.Read<0>();
						}

						if (currentHeadNext == currentHead) {
							// the head points to itself -- e.g. the list only has one element
							newNode->next = currentHead;
							newNode->prev = currentHead;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->next, // becomes newNode
								&currentHead->prev, // becomes newNode
								&tail // becomes currentHead
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentHead, currentHead, currentTail);
							if (container.TrySwap(OldValues, newNode, newNode, newNode, currentHead)) return true;
						}
						else {
							// the head points to something -- e.g. the list has two or more elements
							newNode->next = currentHead;
							newNode->prev = currentTail;

							auto container{ fibers::utilities::MultiItemCAS(
								&head, // becomes newNode
								&currentHead->prev, // becomes newNode
								&currentTail->next // becomes newNode
							) };
							fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentHead);
							if (container.TrySwap(OldValues, newNode, newNode, newNode)) return true;
						}
					}
					else {
						// if there is no head, try to swap the head and tail for the new node.
						fibers::utilities::Union< linkedListItem*, linkedListItem*> OldValues(nullptr, nullptr);
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						if (container.TrySwap(OldValues, newNode, newNode)) return true;
					}
				}
				nodeAlloc.Free(newNode); // easy replacement
				return false;
			}
			// Remove the front element from line or return false if the list is empty. Will always succeed, eventually. 
			bool pop(value& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				while (true) {
					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}

					if (!currentHead || !currentTail) return false; // the list is empty
					
					linkedListItem* currentHeadPrev;
					linkedListItem* currentTailPrev; {
						auto container{ fibers::utilities::MultiItemCAS(
							&currentTail->prev,
							& currentHead->prev
						) };
						currentTailPrev = container.Read<0>();
						currentHeadPrev = container.Read<1>();
					}

					if (currentHead == currentTail) {
						// they are the same node, meaning the list has only one item. Clear it out and allow us to start fresh. 
						auto container{ fibers::utilities::MultiItemCAS(
							&head, // becomes nullptr
	                        &currentHead->prev, // becomes nullptr
							&currentTailPrev->next, // becomes nullptr
							&tail // becomes nullptr
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, nullptr, nullptr, nullptr, nullptr)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
					else {
						auto container{ fibers::utilities::MultiItemCAS(
							&tail, // becomes currentTailPrev
							&currentHead->prev, // becomes currentTailPrev
							&currentTailPrev->next // becomes currentHead
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, currentTailPrev, currentTailPrev, currentHead)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
				}

				return false;
			}
			// try to remove the front element from line. May fail under competition with other threads or if the list is empty.
			bool try_pop(value& element) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };

				if (true) {
					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}

					if (!currentHead || !currentTail) return false; // the list is empty

					linkedListItem* currentHeadPrev;
					linkedListItem* currentTailPrev; {
						auto container{ fibers::utilities::MultiItemCAS(
							&currentTail->prev,
							& currentHead->prev
						) };
						currentTailPrev = container.Read<0>();
						currentHeadPrev = container.Read<1>();
					}

					if (currentHead == currentTail) {
						// they are the same node, meaning the list has only one item. Clear it out and allow us to start fresh. 
						auto container{ fibers::utilities::MultiItemCAS(
							&head, // becomes nullptr
							&currentHead->prev, // becomes nullptr
							&currentTailPrev->next, // becomes nullptr
							&tail // becomes nullptr
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, nullptr, nullptr, nullptr, nullptr)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
					else {
						auto container{ fibers::utilities::MultiItemCAS(
							&tail, // becomes currentTailPrev
							&currentHead->prev, // becomes currentTailPrev
							&currentTailPrev->next // becomes currentHead
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentTail, currentTail, currentTail);
						if (container.TrySwap(OldValues, currentTailPrev, currentTailPrev, currentHead)) {
							element = currentTail->Value;
							auto guard{ std::shared_lock(deleteGuard) };
							nodeAlloc.Free(currentTail);
							return true;
						}
					}
				}

				return false;
			}
			// try to remove the front element from line if its value matches the element.
			bool try_remove_front_if(std::function<bool(value const&)> test) noexcept {
				//auto guard{ nodeAlloc.CreateEpochGuard() };
				
				while (true) {
					linkedListItem* currentHead;
					linkedListItem* currentTail; {
						auto container{ fibers::utilities::MultiItemCAS(
							&head,
							&tail
						) };
						currentHead = container.Read<0>();
						currentTail = container.Read<1>();
					}

					if (!currentHead || !currentTail) return false; // the list is empty

					linkedListItem* currentHeadPrev;
					linkedListItem* currentTailPrev; {
						auto container{ fibers::utilities::MultiItemCAS(
							&currentTail->prev,
							& currentHead->prev
						) };
						currentTailPrev = container.Read<0>();
						currentHeadPrev = container.Read<1>();
					}

					if (currentHead == currentTail) {
						// they are the same node, meaning the list has only one item. Clear it out and allow us to start fresh. 
						auto container{ fibers::utilities::MultiItemCAS(
							&head, // becomes nullptr
							&currentHead->prev, // becomes nullptr
							&currentTailPrev->next, // becomes nullptr
							&tail // becomes nullptr
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentHead, currentTail, currentTail, currentTail);

						if (test(currentTail->Value)) {
							if (container.TrySwap(OldValues, nullptr, nullptr, nullptr, nullptr)) {
								auto guard{ std::shared_lock(deleteGuard) };
								nodeAlloc.Free(currentTail);
								return true;
							}
							else {
								// try again
							}
						}
						else {
							return false;
						}
					}
					else {
						auto container{ fibers::utilities::MultiItemCAS(
							&tail, // becomes currentTailPrev
							&currentHead->prev, // becomes currentTailPrev
							&currentTailPrev->next // becomes currentHead
						) };
						fibers::utilities::Union< linkedListItem*, linkedListItem*, linkedListItem*> OldValues(currentTail, currentTail, currentTail);
						
						if (test(currentTail->Value)) {
							if (container.TrySwap(OldValues, currentTailPrev, currentTailPrev, currentHead)) {
								auto guard{ std::shared_lock(deleteGuard) };
								nodeAlloc.Free(currentTail);
								return true;
							}
							else {
								// try again
							}
						}
						else {
							return false;
						}
					}
				}

				return false;
			}

			bool front(value& Value) {
				//auto guard{ nodeAlloc.CreateEpochGuard() };
				auto guard{ std::scoped_lock(deleteGuard) };

				linkedListItem* currentTail; {
					auto container{ fibers::utilities::MultiItemCAS(
						&tail
					) };
					currentTail = container.Read<0>();
				}
				if (currentTail) {
					Value = currentTail->Value;
					return true;
				}
				else {
					return false;
				}
			};
		};







	};

	namespace impl {
		bool Initialize(uint32_t maxThreadCount = std::numeric_limits< uint32_t>::max());
		void ShutDown();

		/* job arguments used to perform work as part of a loop over a Task */
		struct JobArgs {
			uint32_t jobIndex;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
			uint32_t groupID;		// group index relative to dispatch (like SV_GroupID in HLSL)
			uint32_t groupIndex;	// job index relative to group (like SV_GroupIndex in HLSL)
			void* sharedmemory;		// stack memory shared within the current group (jobs within a group execute serially)
		};

		// Defines a state of execution, can be waited on
		struct context {
			synchronization::atomic_number<long> counter{ 0 }; // how many Tasks* are awaited
			synchronization::atomic_ptr<std::exception_ptr> e{ nullptr }; // shared error PTR for re-throwing at the end of the Tasks.
		};

		/* job arguments used to perform work as part of a loop over a Task */
		struct Task {
			std::function<void(JobArgs)> task;
			context* ctx;
			uint32_t groupID;
			uint32_t groupJobOffset;
			uint32_t groupJobEnd;
			uint32_t sharedmemory_size;
			std::function<void(void*)> GroupStartJob; // callback func with memory for type T
			std::function<void(void*)> GroupEndJob; // callback func with memory for type T
		};

		template <typename T> class Queue {
		public:
#if 0 // breaks so far, for reasons unknown. May be a bug with AtomicQueue.
			moodycamel::ConcurrentQueue<T> queue;
			// fibers::containers::AtomicQueue<T> queue;

			__forceinline void push(T&& item) {
				queue.push(std::forward<T>(item));
			};
			__forceinline void push(const T& item) {
				queue.push(item);
			};
			__forceinline bool try_pop(T& item) {
				return queue.try_pop(item);
			};
			__forceinline bool front(T& item) {
				return queue.front(item);
			};
#else
#if 0 // utilizes a lock-free design that guarrantees progress under heavy contention
			concurrency::concurrent_queue<T> queue;

			__forceinline void push(T&& item) {
				queue.push(std::forward<T>(item));
			};
			__forceinline void push(const T& item) {
				queue.push(item);
			};
			__forceinline bool try_pop(T& item) {
				return queue.try_pop(item);
			};
			__forceinline bool front(T& item) = delete; // this function is not supported under this mode.
#else
			std::deque<T> queue;
			fibers::synchronization::mutex locker;

			__forceinline void push(T&& item) {
				std::scoped_lock lock(locker);
				queue.push_back(std::forward<T>(item));
			};
			__forceinline void push(const T& item) {
				std::scoped_lock lock(locker);
				queue.push_back(item);
			};
			__forceinline bool try_pop(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.front());
				queue.pop_front();
				return true;
			};
			__forceinline bool front(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.front());				
				return true;
			};
			__forceinline bool try_pop_back(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.back());
				queue.pop_back();
				return true;
			};
			__forceinline bool back(T& item) {
				std::scoped_lock lock(locker);
				if (queue.empty()) return false;
				item = std::move(queue.back());
				return true;
			};
#endif
#endif
			Queue() = default;
			Queue(Queue const&) = default;
			Queue(Queue &&) = default;
			Queue& operator=(Queue const&) = default;
			Queue& operator=(Queue&&) = default;
			~Queue() = default;
		};
		struct InternalState {
			uint32_t numCores = 0;
			uint32_t numThreads = 0;

			std::unique_ptr<Queue<Task>[]> jobQueuePerThread;
			// std::unique_ptr<Task[]> currentTaskPerThread;

			fibers::containers::number<bool> alive{ true };
			std::condition_variable_any wakeCondition; // std::condition_variable wakeCondition; //  
			synchronization::mutex wakeMutex; // std::mutex wakeMutex; //  
			fibers::containers::number<long long> nextQueue{ 0 };
			std::vector<std::thread> threads;
			void ShutDown() {
				alive = false; // indicate that new jobs cannot be started from this point
				bool wake_loop = true;
				std::thread waker([&] {
					while (wake_loop) wakeCondition.notify_all(); // wakes up sleeping worker threads
				});
				for (auto& thread : threads) {
					thread.join();
				}
				wake_loop = false;
				waker.join();
				jobQueuePerThread.reset();
				// currentTaskPerThread.reset();
				threads.clear();
				numCores = 0;
				numThreads = 0;
			};
			~InternalState() {
				ShutDown();
			};
		} static internal_state;

		__forceinline uint32_t GetThreadCount() { return internal_state.numThreads; };

		// Add a task to execute asynchronously. Any idle thread will execute this.
		void Execute(context& ctx, std::function<void(JobArgs const&)> const& task) noexcept;

		// Divide a task onto multiple jobs and execute in parallel.
		//	jobCount	: how many jobs to generate for this task.
		//	groupSize	: how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
		//	task		: receives a JobArgs as parameter
		void Dispatch(
			context& ctx, 
			uint32_t jobCount, 
			std::function<void(JobArgs const&)> const& task
		) noexcept;

		void Dispatch(
			context& ctx,
			uint32_t jobCount,
			std::function<void(JobArgs const&)> const& task,
			size_t sharedmemory_size,
			std::function<void(void*)> const& GroupStartJob, // callback func with memory for type T
			std::function<void(void*)> const& GroupEndJob // callback func with memory for type T
		) noexcept;

		// Returns the amount of job groups that will be created for a set number of jobs and group size
		__forceinline constexpr uint32_t DispatchGroupCount(uint32_t jobCount, uint32_t groupSize) { return (jobCount + groupSize - 1) / groupSize; /* Calculate the amount of job groups to dispatch (overestimate, or "ceil"): */ };

		// Check if any threads are working currently or not
		bool IsBusy(const context& ctx);

		void HandleExceptions(context& ctx);

		// Wait until all threads become idle. Current thread will become a worker thread, executing jobs.
		void Wait(context& ctx);

		struct TaskGroup {
		private:
			context ctx{0, nullptr};

		public:
			auto Wait() { return impl::Wait(ctx); };
			auto IsBusy() const { return impl::IsBusy(ctx); };
			auto Queue(std::function<void(JobArgs const&)> const& task) { return impl::Execute(ctx, task); };
			
			/* Dispatch a function that does not need to share memory within a group / cluster of the Task jobs. */
			auto Dispatch(
				uint32_t jobCount,
				std::function<void(JobArgs const&)> const& task
			) {
				return impl::Dispatch(ctx, jobCount, task);
			};	
			
			/* Dispatch a function that intends to share memory serially within a group / cluster of the Task jobs. */
			template <typename T> auto Dispatch(
				uint32_t jobCount,
				std::function<void(JobArgs const&)> const& task,
				std::function<void(void*)> const& GroupStartJob, // callback func with memory for type T
				std::function<void(void*)> const& GroupEndJob // callback func with memory for type T
			){
				return impl::Dispatch(ctx, jobCount, task, sizeof(T), GroupStartJob, GroupEndJob);
			};
		};
	};

	class JobGroup;

	/*! Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wrapper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	auto __awaiter__ = Job([](){ return std::string("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, and guarrantees its completion before the scope ends. */
	class Job {
		friend JobGroup;
	protected:
		mutable std::shared_ptr<fibers::Action_Base> impl;

	private:
		template <typename T> static constexpr const bool IsStaticFunction() {
			if constexpr (std::is_pointer<T>::value) {
				return std::is_function<typename std::remove_pointer_t<T>>::value;
			}
			else {
				return std::is_function<T>::value;
			}
		};
		template <typename T> static constexpr const bool IsLambda(){
			if constexpr (IsStaticFunction<T>() || std::is_member_function_pointer<T>::value) {
				return false;
			}
			if constexpr (std::is_invocable<T>::value) {
				return true;
			}
			return false;
		};
		template<typename T, typename... Args> static constexpr const bool IsStatelessTest() {
			using return_type = typename std::invoke_result<T, Args...>::type;
			using ftype = return_type(*)(Args...);
			return std::is_convertible<T, ftype>::value;
		};
		template <typename T, typename... Args> static constexpr const bool IsLambdaStateless() {
			if constexpr (IsLambda<T>()) {
				return IsStatelessTest<T, Args...>();
			}
			else {
				return false;
			}
		};
		template<typename T, typename... Args> static constexpr const bool IsStateless() {
			if constexpr (IsLambdaStateless<T, Args...>() || IsStaticFunction<T>()) {
				return true;
			}
			else {
				return false;
			}
		};

	public:
		Job() : impl(nullptr) {};
		Job(const Job& other) : impl(other.impl) {};
		Job(Job&& other) : impl(std::move(other.impl)) {};
		Job& operator=(const Job& other) { impl = other.impl; return *this; };
		Job& operator=(Job&& other) { impl = std::move(other.impl); return *this; };

		/* Creates a job from a function and (optionally) input parameters. Can handle basic type-casting from inputs to parameters, and supports shared_ptr casting (to and from). */
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<fibers::Job, std::decay_t<T>> && !std::is_same_v<fibers::Any, std::decay_t<T>> >> explicit Job(T&& function, Args && ... Fargs) : impl(nullptr) {
			auto func{ fibers::Function(std::function(std::forward<T>(function)), std::forward<Args>(Fargs)...) }; 
			
			static constexpr const bool is_stateless{ IsStateless<T, Args...>() };

			if constexpr (decltype(func)::returnsNothing) {
				auto* action{ new fibers::Action_NoReturn(std::move(func), is_stateless) }; // create ptr
				impl = std::static_pointer_cast<fibers::Action_Base>(std::shared_ptr<typename std::remove_pointer_t<decltype(action)>>(std::move(action))); // move to smart ptr and then cast-down to base. Base handle counter will do destruction
			}
			else {
				auto* action{ new fibers::Action_Returns(std::move(func), is_stateless) }; // create ptr
				impl = std::static_pointer_cast<fibers::Action_Base>(std::shared_ptr<typename std::remove_pointer_t<decltype(action)>>(std::move(action))); // move to smart ptr and then cast-down to base. Base handle counter will do destruction
			}
		};
		~Job() = default;

	public:
		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		fibers::Any& Invoke() const noexcept {
			static fibers::Any staticVal{};
			if (impl) {
				return impl->Invoke();
			}
			else {
				return staticVal;
			}
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. The awaiter group guarrantees job completion before the awaiter or job goes out-of-scope. Useful for most basic task scheduling. */
		[[nodiscard]] JobGroup AsyncInvoke();

		/* Add the task to a thread / fiber, and then "forgets" the job. 
		CAUTION: user is responsible for guarranteeing that all data used by the job outlives the job itself, if using this mode of tasking. 
		This will throw an error if the underlying job is a capturing lambda, to reinforce the above requirement. 
		*/
		void AsyncFireAndForget();

		bool IsStatic() const noexcept {
			static bool staticVal{ true };
			if (impl) {
				return impl->IsStatic();
			}
			return staticVal;
		};

		/* Returns the potential name of the static function, if one is known. */
		std::string FunctionName() const {
			static std::string staticVal{ "" };
			if (impl) {
				return impl->FunctionName();
			}
			return staticVal;
		};

		/* Returns the result of the job, if any, if already performed. If not performed, the result will be empty. */
		const fibers::Any& GetResult() const {
			static fibers::Any staticVal{};
			if (impl) {
				return impl->GetResult();
			}
			return staticVal;
		};

		/* Do the task immediately, without using any thread/fiber tools, and returns the result (if any). */
		fibers::Any& operator()() {
			return Invoke();
		};
	};

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup {
		friend Job;
	private:
		class JobGroupImpl {
		public:
			std::shared_ptr<void> waitGroup;
			Job last_job;

			JobGroupImpl() : waitGroup(nullptr), last_job() {};
			JobGroupImpl(std::shared_ptr<void> wg) : waitGroup(wg), last_job() {};
			JobGroupImpl(JobGroupImpl const&) = delete;
			JobGroupImpl(JobGroupImpl&&) = delete;
			JobGroupImpl& operator=(JobGroupImpl const&) = delete;
			JobGroupImpl& operator=(JobGroupImpl&&) = delete;

			void Queue(Job const& job);
			void Queue(std::vector<Job> const& listOfJobs);
			void Wait();
			~JobGroupImpl() { Wait(); };
		};

	public:
		JobGroup();
		JobGroup(Job const& job);

		// The waiter should not be passed around. Ideally we want to follow Fiber job logic, e.g. splitting jobs quickly and 
		// then finishing them in the same job that started them, continuing like the split never happened.
		JobGroup(JobGroup const&) = delete;
		JobGroup(JobGroup&& a) : impl(std::move(a.impl)) {};
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() = default;

		/* Queue job, and return tool to await the result */
		JobGroup& Queue(Job const& job) {
			impl->Queue(job);
			return *this;
		};

		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			impl->Queue(listOfJobs);
			return *this;
		};

		/* Await all jobs in this group, and gets the return value of the last job submitted */
		template <typename T = void>
		decltype(auto) Wait_Get() {
			impl->Wait();

			if constexpr (std::is_same<T, void>::value) {
				return impl->last_job.GetResult();
			}
			else {
				return impl->last_job.GetResult().cast<T>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
		};

		impl::TaskGroup& GetTaskGroup() const {
			return *static_cast<impl::TaskGroup*>(impl->waitGroup.get());
		};

	protected:
		std::unique_ptr<JobGroupImpl> impl{};

	};

	namespace parallel {
#if 0
		namespace impl {
			template <class _FwdIt, class _Diff = std::_Iter_diff_t<_FwdIt>, bool = std::_Is_random_iter_v<_FwdIt>>
			struct _Static_partition_range;

			template <class _RanIt, class _Diff> /* random iteration type */
			struct _Static_partition_range<_RanIt, _Diff, true> {
				using _Target_diff = std::_Iter_diff_t<_RanIt>;
				using _URanIt = std::_Unwrapped_t<const _RanIt&>;
				_URanIt _Start_at;
				using _Chunk_type = std::_Iterator_range<_URanIt>;

				_RanIt _Populate(const std::_Static_partition_team<_Diff>& _Team, _RanIt _First) {
					// statically partition a random-access iterator range and return next(_First, _Team._Count)
					// pre: _Populate hasn't yet been called on this instance
					auto _Result = _First + static_cast<_Target_diff>(_Team._Count); // does verification
					_Start_at = std::_Get_unwrapped(_First);
					return _Result;
				}

				bool _Populate(const std::_Static_partition_team<_Diff>& _Team, _RanIt _First, _RanIt _Last) {
					// statically partition a random-access iterator range and check if the range ends at _Last
					// pre: _Populate hasn't yet been called on this instance
					std::_Adl_verify_range(_First, _Last);
					_Start_at = std::_Get_unwrapped(_First);
					return _Team._Count == _Last - _First;
				}

				_URanIt _Get_first(size_t /* _Chunk_number */, const _Diff _Offset) {
					// get the first iterator for _Chunk _Chunk_number (which is at offset _Offset)
					return _Start_at + static_cast<_Target_diff>(_Offset);
				}

				_Chunk_type _Get_chunk(const std::_Static_partition_key<_Diff> _Key) const {
					// get a static partition chunk from a random-access range
					// pre: _Key was generated by the _Static_partition_team instance passed to a previous call to _Populate
					const auto _First = _Start_at + static_cast<_Target_diff>(_Key._Start_at);
					return { _First, _First + static_cast<_Target_diff>(_Key._Size) };
				}
			};

			template <class _FwdIt, class _Diff> /* non-random (e.g. forward) iter type */
			struct _Static_partition_range<_FwdIt, _Diff, false> {
				using _Target_diff = std::_Iter_diff_t<_FwdIt>;
				using _UFwdIt = std::_Unwrapped_t<const _FwdIt&>;
				std::_Parallel_vector<_UFwdIt> _Division_points;
				using _Chunk_type = std::_Iterator_range<_UFwdIt>;

				_FwdIt _Populate(const std::_Static_partition_team<_Diff>& _Team, _FwdIt _First) {
					// statically partition a forward iterator range and return next(_First, _Team._Count)
					// pre: _Populate hasn't yet been called on this instance
					const auto _Chunks = _Team._Chunks;
					_Division_points.resize(_Chunks + 1);
					// The following potentially narrowing cast is OK because caller has ensured
					// next(_First, _Team._Count) is valid (and _Count <= _Chunk_size)
					const auto _Chunk_size = static_cast<_Target_diff>(_Team._Chunk_size);
					const auto _Unchunked_items = _Team._Unchunked_items;
					auto _Result = _Division_points.begin();
					*_Result = std::_Get_unwrapped(_First);
					for (_Diff _Idx{}; _Idx < _Unchunked_items; ++_Idx) { // record bounds of chunks with an extra item
						_STD advance(_First, static_cast<_Target_diff>(_Chunk_size + 1));
						*++_Result = std::_Get_unwrapped(_First);
					}

					const auto _Diff_chunks = static_cast<_Diff>(_Chunks);
					for (_Diff _Idx = _Unchunked_items; _Idx < _Diff_chunks; ++_Idx) { // record bounds of chunks with no extra item
						_STD advance(_First, _Chunk_size);
						*++_Result = std::_Get_unwrapped(_First);
					}

					return _First;
				}

				bool _Populate(const std::_Static_partition_team<_Diff>& _Team, _FwdIt _First, _FwdIt _Last) {
					// statically partition a forward iterator range and check if the range ends at _Last
					// pre: _Populate hasn't yet been called on this instance
					const auto _Chunks = _Team._Chunks;
					_Division_points.resize(_Chunks + 1);
					const auto _Chunk_size = _Team._Chunk_size;
					const auto _Unchunked_items = _Team._Unchunked_items;
					auto _Result = _Division_points.begin();
					*_Result = std::_Get_unwrapped(_First);
					for (_Diff _Idx{}; _Idx < _Unchunked_items; ++_Idx) { // record bounds of chunks with an extra item
						for (_Diff _This_chunk_size = _Chunk_size + 1; 0 < _This_chunk_size--;) {
							if (_First == _Last) {
								return false;
							}

							++_First;
						}

						*++_Result = std::_Get_unwrapped(_First);
					}

					const auto _Diff_chunks = static_cast<_Diff>(_Chunks);
					for (_Diff _Idx = _Unchunked_items; _Idx < _Diff_chunks; ++_Idx) { // record bounds of chunks with no extra item
						for (_Diff _This_chunk_size = _Chunk_size; 0 < _This_chunk_size--;) {
							if (_First == _Last) {
								return false;
							}

							++_First;
						}

						*++_Result = std::_Get_unwrapped(_First);
					}

					return _First == _Last;
				}

				_UFwdIt _Get_first(const size_t _Chunk_number, _Diff /* _Offset */) {
					// get the first iterator for _Chunk _Chunk_number (which is at offset _Offset)
					return _Division_points[_Chunk_number];
				}

				_Chunk_type _Get_chunk(const std::_Static_partition_key<_Diff> _Key) const {
					// get a static partition chunk from a forward range
					// pre: _Key was generated by the _Static_partition_team instance passed to a previous call to _Populate
					return { _Division_points[_Key._Chunk_number], _Division_points[_Key._Chunk_number + 1] };
				}
			};

			template <class _FwdIt, class _Diff, class _Fn>
			struct _Static_partitioned_for_each2 { // for_each task scheduled on the system thread pool
				std::_Static_partition_team<_Diff> _Team;
				impl::_Static_partition_range<_FwdIt, _Diff> _Basis;
				_Fn _Func;

				_Static_partitioned_for_each2(const size_t _Hw_threads, const _Diff _Count, _Fn _Fx)
					: _Team{ _Count, std::_Get_chunked_work_chunk_count(_Hw_threads, _Count) }, _Basis{}, _Func(_Fx) {}

				std::_Cancellation_status _Process_chunk() {
					const auto _Key = _Team._Get_next_key();
					if (_Key) {
						const auto _Chunk = _Basis._Get_chunk(_Key);
						std::_For_each_ivdep(_Chunk._First, _Chunk._Last, _Func);
						return std::_Cancellation_status::_Running;
					}

					return std::_Cancellation_status::_Canceled;
				}

				static void __stdcall _Threadpool_callback(
					__std_PTP_CALLBACK_INSTANCE, void* const _Context, __std_PTP_WORK) noexcept /* terminates */ {
					std::_Run_available_chunked_work(*static_cast<_Static_partitioned_for_each2*>(_Context));
				}
			};
		}
#endif

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, F ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
			if constexpr (retNo) {
				synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

				auto _Passed_fn = /*std::_Pass_fn*/([&](iteratorType i) {
					if (!e) {
						try {
							i += start;
							ToDo(i);
						}
						catch (...) {
							if (!e) {
								auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
								if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
									delete eptr;
								}
							}
						}
					}
				});
				fibers::utilities::Sequence<iteratorType> seq(end - start);

				auto _Count = end - start;
				const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
				if (_Hw_threads > 1 && _Count > 2) {
					auto _First = seq.begin();

					auto _UFirst = std::_Get_unwrapped_n(_First, _Count);
					auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
					std::_Seek_wrapped(_First, _Operation._Basis._Populate(_Operation._Team, _UFirst));

					// process chunks of _Operation on the thread pool
					const std::_Work_ptr _Work_op{ _Operation };

					// setup complete, hereafter nothrow or terminate
					_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
					std::_Run_available_chunked_work(_Operation);
				}
				else {
					// not enough threads or the num jobs is not large enough to warrent multithreading
					for (; start < end; start++) {
						_Passed_fn(start);
					}
				}

				if (e) {
					auto eptr = e.Set(nullptr);
					if (eptr) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				}
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;
				auto _Count = end - start;
				if (_Count > 0) {
					std::vector< returnT > out(_Count, returnT());

					synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

					auto _Passed_fn = /*std::_Pass_fn*/([&](iteratorType i) {
						if (!e) {
							try {
								i += start;
								out[i] = ToDo(i);
							}
							catch (...) {
								if (!e) {
									auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
									if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
										delete eptr;
									}
								}
							}
						}
					});
					fibers::utilities::Sequence<iteratorType> seq(end - start);

					auto _Count = end - start;
					const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
					if (_Hw_threads > 1 && _Count > 2) {
						auto _First = seq.begin();

						auto _UFirst = std::_Get_unwrapped_n(_First, _Count);
						auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
						std::_Seek_wrapped(_First, _Operation._Basis._Populate(_Operation._Team, _UFirst));

						// process chunks of _Operation on the thread pool
						const std::_Work_ptr _Work_op{ _Operation };

						// setup complete, hereafter nothrow or terminate
						_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
						std::_Run_available_chunked_work(_Operation);
					}
					else {
						// not enough threads or the num jobs is not large enough to warrent multithreading
						for (; start < end; start++) {
							_Passed_fn(start);
						}
					}

					if (e) {
						auto eptr = e.Set(nullptr);
						if (eptr) {
							std::exception_ptr copy{ *eptr };
							delete eptr;
							std::rethrow_exception(std::move(copy));
						}
					}

					return out;
				}
				else {
					return std::vector< returnT >();
				}
			}
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, class F> decltype(auto) For(iteratorType start, iteratorType end, iteratorType step, F ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
			if constexpr (retNo) {
				synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

				auto _Passed_fn = /*std::_Pass_fn*/([&](iteratorType i) {
					if (!e) {
						try {
							i *= step;
							i += start;
							ToDo(i);
						}
						catch (...) {
							if (!e) {
								auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
								if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
									delete eptr;
								}
							}
						}
					}
					});
				fibers::utilities::Sequence<iteratorType> seq(end - start);

				auto _Count = (end - start) / step;
				const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
				if (_Hw_threads > 1 && _Count > 2) {
					auto _First = seq.begin();

					auto _UFirst = std::_Get_unwrapped_n(_First, _Count);
					auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
					std::_Seek_wrapped(_First, _Operation._Basis._Populate(_Operation._Team, _UFirst));

					// process chunks of _Operation on the thread pool
					const std::_Work_ptr _Work_op{ _Operation };

					// setup complete, hereafter nothrow or terminate
					_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
					std::_Run_available_chunked_work(_Operation);
				}
				else {
					// not enough threads or the num jobs is not large enough to warrent multithreading
					for (; start < end; start += step) {
						_Passed_fn(start);
					}
				}

				if (e) {
					auto eptr = e.Set(nullptr);
					if (eptr) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				}
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;
				auto _Count = end - start;
				if (_Count > 0) {
					std::vector< returnT > out(_Count, returnT());

					synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

					auto _Passed_fn = /*std::_Pass_fn*/([&](iteratorType i) {
						if (!e) {
							try {
								i *= step;
								i += start;
								out[i] = ToDo(i);
							}
							catch (...) {
								if (!e) {
									auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
									if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
										delete eptr;
									}
								}
							}
						}
					});
					fibers::utilities::Sequence<iteratorType> seq(end - start);

					auto _Count = (end - start) / step;
					const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
					if (_Hw_threads > 1 && _Count > 2) {
						auto _First = seq.begin();

						auto _UFirst = std::_Get_unwrapped_n(_First, _Count);
						auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
						std::_Seek_wrapped(_First, _Operation._Basis._Populate(_Operation._Team, _UFirst));

						// process chunks of _Operation on the thread pool
						const std::_Work_ptr _Work_op{ _Operation };

						// setup complete, hereafter nothrow or terminate
						_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
						std::_Run_available_chunked_work(_Operation);
					}
					else {
						// not enough threads or the num jobs is not large enough to warrent multithreading
						for (; start < end; start += step) {
							_Passed_fn(start);
						}
					}

					if (e) {
						auto eptr = e.Set(nullptr);
						if (eptr) {
							std::exception_ptr copy{ *eptr };
							delete eptr;
							std::rethrow_exception(std::move(copy));
						}
					}

					return out;
				}
				else {
					return std::vector< returnT >();
				}
			}
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType& container, F ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
			using iterType = typename containerType::iterator;
			using value_type = std::remove_reference_t<typename iterType::reference>;

			if constexpr (retNo) {
				synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

				auto _Passed_fn = /*std::_Pass_fn*/([&](value_type& i) {
					if (!e) {
						try {
							ToDo(i);
						} catch (...) {
							if (!e) {
								auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
								if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
									delete eptr;
								}
							}
						}
					}
				});

				auto _UFirst = std::_Get_unwrapped(container.begin());
				auto _ULast = std::_Get_unwrapped(container.end());
				auto _Count = std::distance(_UFirst, _ULast);
				const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
				if (_Hw_threads > 1 && _Count > 2) {
					auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
					(void)_Operation._Basis._Populate(_Operation._Team, _UFirst);
					const std::_Work_ptr _Work_op{ _Operation }; // process chunks of _Operation on the thread pool

					// setup complete, hereafter nothrow or terminate
					_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
					std::_Run_available_chunked_work(_Operation);
				}
				else {
					// not enough threads or the num jobs is not large enough to warrent multithreading
					for (; _UFirst != _ULast; ++_UFirst) {
						_Passed_fn(*_UFirst);
					}
				}

				if (e) {
					auto eptr = e.Set(nullptr);
					if (eptr) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				}
			}
			else{
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;
				auto _First = container.begin();
				auto _Last = container.end();
				auto _Count = std::distance(_First, _Last);

				if (_Count > 0) {
					std::vector< returnT > out(_Count, returnT());

					synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

					fibers::utilities::IteratorSequence<decltype(_First)> seq(_First, _Last);
					auto _UFirst = std::_Get_unwrapped(seq.begin());
					auto _ULast = std::_Get_unwrapped(seq.end());
					
					auto _Passed_fn = /*std::_Pass_fn*/([&](typename decltype(seq)::iterator::value_type& i) {
						if (!e) {
							try {
								out[i.first] = ToDo(*i.second);
							}
							catch (...) {
								if (!e) {
									auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
									if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
										delete eptr;
									}
								}
							}
						}
					});

					const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
					if (_Hw_threads > 1 && _Count > 2) {
						auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
						(void)_Operation._Basis._Populate(_Operation._Team, _UFirst);
						const std::_Work_ptr _Work_op{ _Operation }; // process chunks of _Operation on the thread pool

						// setup complete, hereafter nothrow or terminate
						_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
						std::_Run_available_chunked_work(_Operation);
					}
					else {
						// not enough threads or the num jobs is not large enough to warrent multithreading
						for (; _UFirst != _ULast; ++_UFirst) {
							_Passed_fn(*_UFirst);
						}
					}

					if (e) {
						auto eptr = e.Set(nullptr);
						if (eptr) {
							std::exception_ptr copy{ *eptr };
							delete eptr;
							std::rethrow_exception(std::move(copy));
						}
					}

					return out;
				}
				else {
					return std::vector< returnT >();
				}
			}
			
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType const& container, F ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;
			using iterType = typename containerType::const_iterator;
			using value_type = std::remove_reference_t<typename iterType::value_type>;

			if constexpr (retNo) {
				synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

				auto _Passed_fn = /*std::_Pass_fn*/([&](value_type& i) {
					if (!e) {
						try {
							ToDo(i);
						}
						catch (...) {
							if (!e) {
								auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
								if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
									delete eptr;
								}
							}
						}
					}
					});

				auto _UFirst = std::_Get_unwrapped(container.begin());
				auto _ULast = std::_Get_unwrapped(container.end());
				auto _Count = std::distance(_UFirst, _ULast);
				const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
				if (_Hw_threads > 1 && _Count > 2) {
					auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
					(void)_Operation._Basis._Populate(_Operation._Team, _UFirst);
					const std::_Work_ptr _Work_op{ _Operation }; // process chunks of _Operation on the thread pool

					// setup complete, hereafter nothrow or terminate
					_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
					std::_Run_available_chunked_work(_Operation);
				}
				else {
					// not enough threads or the num jobs is not large enough to warrent multithreading
					for (; _UFirst != _ULast; ++_UFirst) {
						_Passed_fn(*_UFirst);
					}
				}

				if (e) {
					auto eptr = e.Set(nullptr);
					if (eptr) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				}
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;
				auto _First = container.begin();
				auto _Last = container.end();
				auto _Count = std::distance(_First, _Last);

				if (_Count > 0) {
					std::vector< returnT > out(_Count, returnT());

					synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };

					fibers::utilities::IteratorSequence<decltype(_First)> seq(_First, _Last);
					auto _UFirst = std::_Get_unwrapped(seq.begin());
					auto _ULast = std::_Get_unwrapped(seq.end());

					auto _Passed_fn = /*std::_Pass_fn*/([&](typename decltype(seq)::iterator::value_type& i) {
						if (!e) {
							try {
								out[i.first] = ToDo(*i.second);
							}
							catch (...) {
								if (!e) {
									auto eptr = e.Set(new std::exception_ptr(std::current_exception())); // Sets the error to the new PTR
									if (eptr) { // If we accidentilly errored at the same time as another group, prevent leak
										delete eptr;
									}
								}
							}
						}
					});

					const size_t _Hw_threads = __std_parallel_algorithms_hw_threads();
					if (_Hw_threads > 1 && _Count > 2) {
						auto _Operation = std::_Static_partitioned_for_each2<decltype(_UFirst), decltype(_Count), decltype(_Passed_fn)>{ _Hw_threads, _Count, _Passed_fn };
						(void)_Operation._Basis._Populate(_Operation._Team, _UFirst);
						const std::_Work_ptr _Work_op{ _Operation }; // process chunks of _Operation on the thread pool

						// setup complete, hereafter nothrow or terminate
						_Work_op._Submit_for_chunks(_Hw_threads, _Operation._Team._Chunks);
						std::_Run_available_chunked_work(_Operation);
					}
					else {
						// not enough threads or the num jobs is not large enough to warrent multithreading
						for (; _UFirst != _ULast; ++_UFirst) {
							_Passed_fn(*_UFirst);
						}
					}

					if (e) {
						auto eptr = e.Set(nullptr);
						if (eptr) {
							std::exception_ptr copy{ *eptr };
							delete eptr;
							std::rethrow_exception(std::move(copy));
						}
					}

					return out;
				}
				else {
					return std::vector< returnT >();
				}
			}
		};

		/* wrapper for std::find_if. This is not parallelized, it is linear, which appears to be the fastest search for some reason under most cases. */
		template<typename containerType, typename F> decltype(auto) Find(containerType& container, F const& ToDo) {
			return std::find_if(container.begin(), container.end(), [&](auto& x) ->bool { return ToDo(x); }); 
		};

		/* wrapper for std::find_if. This is not parallelized, it is linear, which appears to be the fastest search for some reason under most cases. */
		template<typename containerType, typename F> decltype(auto) Find(containerType const& container, F const& ToDo) {
			return std::find_if(container.cbegin(), container.cend(), [&](auto const& x) ->bool { return ToDo(x); }); 
		};

		/* outputType x;
		for (auto& v : resultList){ x += v; }
		return x; */
		template<typename outputType>
		decltype(auto) Accumulate(std::vector<outputType> const& resultList) {
#if 1 
			return std::reduce(std::execution::par, resultList.begin(), resultList.end(), 0, [](outputType a, outputType b) { return a + b; });
#else
			outputType out{ 0 };
			for (auto& v : resultList) {
				if (v) {
					out += *v;
				}
			}
			return out;
#endif
		};


		












		/* Generic form of a future<T>, which can be used to wait on and get the results of any job. Can be safely shared if multiple places will need access to the result once available. */
		class promise {
		protected:
			std::shared_ptr< synchronization::atomic_ptr<JobGroup> > shared_state;
			std::shared_ptr< synchronization::atomic_ptr<Any> > result;
			std::shared_ptr< synchronization::mutex > waiting;

		public:
			promise() : shared_state(nullptr), result(nullptr), waiting(nullptr) {};
			promise(Job const& job) :
				shared_state(std::shared_ptr<synchronization::atomic_ptr<JobGroup>>(new synchronization::atomic_ptr<JobGroup>(new JobGroup(job)), [](synchronization::atomic_ptr<JobGroup>* anyP) { if (anyP) { auto* p = anyP->Set(nullptr); if (p) { delete p; } delete anyP; } })),
				result(std::shared_ptr<synchronization::atomic_ptr<Any>>(new synchronization::atomic_ptr<Any>(), [](synchronization::atomic_ptr<Any>* anyP) { if (anyP) { auto* p = anyP->Set(nullptr); if (p) { delete p; } delete anyP; } })),
				waiting(std::shared_ptr<synchronization::mutex>(new synchronization::mutex()))
			{};
			promise(promise const&) = default;
			promise(promise&&) = default;
			promise& operator=(promise const&) = default;
			promise& operator=(promise&&) = default;
			virtual ~promise() {};

			/* Returns true if this promise has been initialized correctly. Otherwise, false. */
			bool valid() const noexcept { return (bool)shared_state; };
			/* Wait until the requested job is completed. Repeated waiting is OK, however only the first "waiting" thread actually helps to complete the job - the remaining waiters will spin-wait. */
			void wait() {
				JobGroup* p{ nullptr };
				Any* p2{ nullptr };
				
				defer(if (p) { delete p; });
				defer(if (p2) { delete p2; });

				if (valid() && !result->load()) {
					auto guard{ std::lock_guard(*waiting) };

					p = shared_state->Set(nullptr);
					if (p) {
						p2 = result->Set(new Any(p->Wait_Get()));					
					}					
				}
			};
			/* Try to get the result, if available. Does not wait. */
			Any get_any() const noexcept {
				if (result) {
					Any* p = result->Get();
					if (p) {
						return Any(*p);
					}
				}
				return Any();
			};
			/* Get the result, once available. Waits for the result, if necessary. */
			Any wait_get_any() {
				wait();
				return get_any();
			};
		};

		/* A secondary type tag used to identify if a template type is a future<T> type. */
		class future_type { public: virtual ~future_type() {}; };

		/* Specialized form of a promise, which can be used to handle type-casting for lambdas automatically, while still being useful for waiting on and getting the results of any job.
		Note: Only the first thread that "waits" on a future<T> assists the thread pool. More waiters != more jobs, and therefore additional waiters are spin-locking.
		Recommended that only the thread (or consuming thread) that scheduled the future<T> object should wait for it. */
		template <typename T> class future final : public promise/*, public future_type*/ {
		public:
			future() : promise()/*, future_type()*/ {};
			future(Job const& job) : promise(job)/*, future_type()*/ {};
			future(promise const& p_promise) : promise(p_promise)/*, future_type()*/ {};
			future(future const&) = default;
			future(future&&) = default;
			future& operator=(future const&) = default;
			future& operator=(future&&) = default;
			virtual ~future() {};

			/* Cast-down to a generic promise that erases the information on the return type. Useful for sharing tasks between libraries where type info itself cannot be shared. */
			promise as_promise() const { return promise(reinterpret_cast<const promise&>(*this)); };

			/* get a copy of the result of the task. must have already waited. */
			decltype(auto) get() {
				if (result) {
					Any* p = result->Get();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get();
							//}
							//else {
							return static_cast<T>(p->cast<T>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};
			/* get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. must have already waited. */
			decltype(auto) get_ref() {
				if (result) {
					Any* p = result->Get();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get_ref();
							//}
							//else {
							return static_cast<T&>(p->cast<T&>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};
			/* get a shared_pointer of the result of the task. must have already waited. */
			decltype(auto) get_shared() {
				if (result) {
					Any* p = result->Get();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						}
						else {
							// if the return type is itself a future_type, then we should "wait_get" it as well.
							//if constexpr (std::is_base_of_v<future_type, T>) {
							//	return static_cast<T>(p->cast<T>()).wait_get_shared();
							//}
							//else {
							return static_cast<std::shared_ptr<T>>(p->cast<std::shared_ptr<T>>());
							//}
						}
					}
				}
				throw(std::runtime_error("future was empty"));
			};

			/* wait to get a copy of the result of the task. Repeated waiting is OK. */
			decltype(auto) wait_get() {
				wait();
				return get();
			};
			/* wait to get a reference to the result of the task. Note: lifetime of return reference must not outlive the future<T> object. Repeated waiting is OK. */
			decltype(auto) wait_get_ref() {
				wait();
				return get_ref();
			};
			/* wait to get a shared_pointer of the result of the task. Repeated waiting is OK. */
			decltype(auto) wait_get_shared() {
				wait();
				return get_shared();
			};
		};

		/* returns a future<T> object for awaiting the results of the job. */
		template < typename F, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<F>> && !std::is_same_v<Any, std::decay_t<F>> >>
		__forceinline static decltype(auto) async(F function, Args... Fargs) {
			return future<typename utilities::function_traits<decltype(std::function(function))>::result_type>(Job(function, Fargs...));
		};
	};
};

#include "Units.h"

class MultithreadingInstanceManager {
	public:
		MultithreadingInstanceManager() {};
		virtual ~MultithreadingInstanceManager() {};
	};
/* Instances the fiber system, and destroys it if the DLL / library is unloaded. */
extern std::shared_ptr<MultithreadingInstanceManager> multithreadingInstance;