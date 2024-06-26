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
			_type_*             Alloc() {
				_type_* t{nullptr};
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
				if constexpr (!isPod()) {
					if (element->initialized.Increment() == 1) {
						new (t) _type_;
					}
					
				}				

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
	};

	namespace containers {
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
			utilities::Allocator< linkedListItem, 128, std::is_pod<value>::value> nodeAlloc;
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
				linkedListItem* current{ nullptr };
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
				linkedListItem* current{ nullptr };
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
				linkedListItem* current{ nullptr };
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
			unsigned long count(value const& Value, unsigned long maxCount = std::numeric_limits<unsigned long>::max()) const {
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
			bool contains(value const& Value) const {
				return count(Value, 1) > 0;
			};

		};
	






	};

	namespace impl {
		bool Initialize(uint32_t maxThreadCount = std::numeric_limits< uint32_t>::max());
		void ShutDown();

		struct JobArgs
		{
			uint32_t jobIndex;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
			uint32_t groupID;		// group index relative to dispatch (like SV_GroupID in HLSL)
			uint32_t groupIndex;	// job index relative to group (like SV_GroupIndex in HLSL)
			void* sharedmemory;		// stack memory shared within the current group (jobs within a group execute serially)
		};

		// Defines a state of execution, can be waited on
		struct context {
			synchronization::atomic_number<long> counter{ 0 };
			synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };
		};
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
		
		template <typename T> struct Queue {
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
		};
		struct InternalState {
			uint32_t numCores = 0;
			uint32_t numThreads = 0;
			std::unique_ptr<Queue<Task>[]> jobQueuePerThread;

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
		void Execute(context& ctx, const std::function<void(JobArgs)>& task) noexcept;

		// Divide a task onto multiple jobs and execute in parallel.
		//	jobCount	: how many jobs to generate for this task.
		//	groupSize	: how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
		//	task		: receives a JobArgs as parameter
		void Dispatch(
			context& ctx, 
			uint32_t jobCount, 
			const std::function<void(JobArgs)>& task, 
			size_t sharedmemory_size = 0, 
			const std::function<void(void*)>& GroupStartJob = nullptr, // callback func with memory for type T
			const std::function<void(void*)>& GroupEndJob = nullptr // callback func with memory for type T
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
			auto Queue(const std::function<void(JobArgs)>& task) { return impl::Execute(ctx, task); };
			template <typename T> auto Dispatch(
				uint32_t jobCount, 
				const std::function<void(JobArgs)>& task, 
				const std::function<void(void*)>& GroupStartJob = nullptr, // callback func with memory for type T
				const std::function<void(void*)>& GroupEndJob = nullptr // callback func with memory for type T
			){ 
				return impl::Dispatch(ctx, jobCount, task, sizeof(T), GroupStartJob, GroupEndJob);
			};
			template <> auto Dispatch<void>(
				uint32_t jobCount, 
				const std::function<void(JobArgs)>& task, 
				const std::function<void(void*)>& GroupStartJob, // callback func with memory for type T
				const std::function<void(void*)>& GroupEndJob // callback func with memory for type T
			) { 
				return impl::Dispatch(ctx, jobCount, task, 0, GroupStartJob, GroupEndJob);
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
			fibers::containers::vector<Job> jobs;

			JobGroupImpl() : waitGroup(nullptr), jobs() {};
			JobGroupImpl(std::shared_ptr<void> wg) : waitGroup(wg), jobs() {};
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
		JobGroup(JobGroup&&) = delete;
		JobGroup& operator=(JobGroup const&) = delete;
		JobGroup& operator=(JobGroup&&) = delete;
		~JobGroup() {};

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

		/* Await all jobs in this group, and get the return values (which may be empty) for each job */
		std::vector<fibers::Any> Wait_Get() {
			impl->Wait();

			typename decltype(JobGroupImpl::jobs) out;
			out.swap(impl->jobs);

			if (out.size() > 0) {
				std::vector<fibers::Any> any;
				for (auto& job : out) {
					if (job) {
						any.push_back(job->GetResult());
					}
				}
				return any;
			}
			else {
				return std::vector<fibers::Any>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
			impl->jobs.clear();
		};

		impl::TaskGroup& GetTaskGroup() const {
			return *std::static_pointer_cast<impl::TaskGroup>(impl->waitGroup);
		};
	private:
		std::unique_ptr<JobGroupImpl> impl;

	};

	namespace parallel {
		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, typename F> decltype(auto) For(JobGroup& jobgroup, iteratorType start, iteratorType end, F const& ToDo) {
			if (end <= start) return;
			auto& group = jobgroup.GetTaskGroup();

			group.Dispatch<void>(end - start, [=](impl::JobArgs args) {
				ToDo(start + args.jobIndex);
			});			
		};

		/* parallel_for (auto i = start; i < end; i += step){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, typename F> decltype(auto) For(JobGroup& jobgroup, iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
			if (end <= start) return;
			auto& group = jobgroup.GetTaskGroup();

			group.Dispatch<void>((end - start) / step, [=](impl::JobArgs args) {
				ToDo(start + args.jobIndex * step);
			});			
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(JobGroup& jobgroup, containerType& container, F const& ToDo) {
			using iterType = typename containerType::iterator;

			auto iter = container.begin();
			auto iter_end = container.end();

			if ((iter_end - iter) > 0) {
				auto& group = jobgroup.GetTaskGroup();

				group.Dispatch<iterType>(iter_end - iter, [&](impl::JobArgs args) { // actual work
					iterType* iterInternal{ nullptr };
					iterInternal = static_cast<iterType*>(args.sharedmemory); // cast PTR
					if ((iterInternal) && (args.groupIndex == 0) && (args.jobIndex > 0)) std::advance(*iterInternal, args.jobIndex); // advance as needed if this is a new non-starting group

					if (*iterInternal != iter_end) {
						ToDo(**iterInternal);
						iterInternal->operator++();
					}
				}, [&iter](void* p)->void { // start-up
					new (p) iterType(iter);
				}, [&](void* p)->void { // close-out
					if constexpr (!std::is_pod<iterType>::value) {
						static_cast<iterType*>(p)->~iterType();
					}
				});

				group.Wait();
			}
		};

		/* parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(JobGroup& jobgroup, containerType const& container, F const& ToDo) {
			using iterType = typename containerType::const_iterator;

			auto iter = container.cbegin();
			auto iter_end = container.cend();

			if ((iter_end - iter) > 0) {
				auto& group = jobgroup.GetTaskGroup();

				group.Dispatch<iterType>(iter_end - iter, [&](impl::JobArgs args) { // actual work
					iterType* iterInternal{ nullptr };
					iterInternal = static_cast<iterType*>(args.sharedmemory); // cast PTR
					if ((iterInternal) && (args.groupIndex == 0) && (args.jobIndex > 0)) std::advance(*iterInternal, args.jobIndex); // advance as needed if this is a new non-starting group

					if (*iterInternal != iter_end) {
						ToDo(**iterInternal);
						iterInternal->operator++();
					}
				}, [&iter](void* p)->void { // start-up
					new (p) iterType(iter);
				}, [&](void* p)->void { // close-out
					if constexpr (!std::is_pod<iterType>::value) {
						static_cast<iterType*>(p)->~iterType();
					}
				});

				group.Wait();
			}
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, typename F> decltype(auto) For(iteratorType start, iteratorType end, F const& ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;

			if constexpr (retNo) {
				if (end <= start) return;

				impl::TaskGroup group;

				group.Dispatch<void>(end - start, [&](impl::JobArgs args) {
					ToDo(start + args.jobIndex);
				});

				group.Wait();
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

				if (end <= start) return std::vector< returnT >();

				std::vector< returnT > out(end - start, returnT());

				impl::TaskGroup group;

				group.Dispatch<void>(end - start, [&](impl::JobArgs args) {
					out[args.jobIndex] = ToDo(start + args.jobIndex);
				});

				group.Wait();

				return out;
			}
		};

		/* parallel_for (auto i = start; i < end; i += step){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, typename F> decltype(auto) For(iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;

			if constexpr (retNo) {
				if (end <= start) return;

				impl::TaskGroup group;

				group.Dispatch<void>((end - start) / step, [&](impl::JobArgs args) {
					ToDo(start + args.jobIndex * step);
				});

				group.Wait();
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

				if (end <= start) return std::vector< returnT >();

				std::vector< returnT > out((end - start) / step, returnT());

				impl::TaskGroup group;

				group.Dispatch<void>((end - start) / step, [&](impl::JobArgs args) {
					out[args.jobIndex] = ToDo(start + args.jobIndex * step);
				});

				group.Wait();

				return out;
			}
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType& container, F const& ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;

			if constexpr (retNo) {
				// int n = container.size();
				// if (n <= 0) return;

#if 0
				synchronization::atomic_ptr<std::exception_ptr> e{nullptr};
				std::for_each_n(std::execution::par_unseq, container.begin(), container.end() - container.begin(), [&](auto& v) {
					try {
						ToDo(v);
					}
					catch (...) {
						if (!e) {
							auto eptr = e.Set(new std::exception_ptr(std::current_exception()));
							if (eptr) {
								delete eptr;
							}
						}
					}
				});
				if (e) {
					auto eptr = e.Set(nullptr);
					if (eptr) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				}

#else
				using iterType = typename containerType::iterator;
				
				auto iter = container.begin();
				auto iter_end = container.end();

				if ((iter_end - iter) > 0) {
					impl::TaskGroup group;

					group.Dispatch<iterType>(iter_end - iter, [&](impl::JobArgs args) { // actual work
						iterType* iterInternal{ nullptr };
						iterInternal = static_cast<iterType*>(args.sharedmemory); // cast PTR
						if ((iterInternal) && (args.groupIndex == 0) && (args.jobIndex > 0)) std::advance(*iterInternal, args.jobIndex); // advance as needed if this is a new non-starting group
						
						if (*iterInternal != iter_end) {
							ToDo(**iterInternal);
							iterInternal->operator++();
						}
					}, [&iter](void* p)->void { // start-up
						new (p) iterType(iter);
					}, [&](void* p)->void { // close-out
						if constexpr (!std::is_pod<iterType>::value) {
							static_cast<iterType*>(p)->~iterType();
						}
					});

					group.Wait();
				}

#endif
			}
			else {


				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;
				

				using iterType = typename containerType::iterator;

				auto iter = container.begin();
				auto iter_end = container.end();

				if ((iter_end - iter) > 0) {
					std::vector< returnT > out(iter_end - iter, returnT());

					impl::TaskGroup group;

					group.Dispatch<iterType>(iter_end - iter, [&](impl::JobArgs args) { // actual work
						iterType* iterInternal{ nullptr };
						iterInternal = static_cast<iterType*>(args.sharedmemory); // cast PTR
						if ((iterInternal) && (args.groupIndex == 0) && (args.jobIndex > 0)) std::advance(*iterInternal, args.jobIndex); // advance as needed if this is a new non-starting group

						if (*iterInternal != iter_end) {
							out[static_cast<int>(args.jobIndex)] = ToDo(**iterInternal);
							iterInternal->operator++();
						}
						}, [&iter](void* p)->void { // start-up
							new (p) iterType(iter);
						}, [&](void* p)->void { // close-out
							if constexpr (!std::is_pod<iterType>::value) {
								static_cast<iterType*>(p)->~iterType();
							}
						});

					group.Wait();

					return out;
				}
				else {
					return std::vector< returnT >();
				}

			}
		};

		/* parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType const& container, F const& ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;

			if constexpr (retNo) {
				// int n = container.size();
				// if (n <= 0) return;

#if 1
				using iterType = typename containerType::const_iterator;

				auto iter = container.cbegin();
				auto iter_end = container.cend();
				
				if ((iter_end - iter) > 0) {
					impl::TaskGroup group;

					group.Dispatch<iterType>(iter_end - iter, [&](impl::JobArgs args) { // actual work
						iterType* iterInternal{ nullptr };
						iterInternal = static_cast<iterType*>(args.sharedmemory); // cast PTR
						if ((iterInternal) && (args.groupIndex == 0) && (args.jobIndex > 0)) std::advance(*iterInternal, args.jobIndex); // advance as needed if this is a new non-starting group

						if (*iterInternal != iter_end) {
							ToDo(**iterInternal);
							iterInternal->operator++();
						}
					}, [&iter](void* p)->void { // start-up
						new (p) iterType(iter);
					}, [&](void* p)->void { // close-out
						if constexpr (!std::is_pod<iterType>::value) {
							static_cast<iterType*>(p)->~iterType();
						}
					});

					group.Wait();
				}
				
#else
				synchronization::atomic_ptr<std::exception_ptr> e{ nullptr };
				std::for_each_n(std::execution::par_unseq, container.cbegin(), container.cend() - container.cbegin(), [&](auto const& v) {
					try {
						ToDo(v);
					}
					catch (...) {
						if (!e) {
							auto eptr = e.Set(new std::exception_ptr(std::current_exception()));
							if (eptr) {
								delete eptr;
							}
						}
					}
				});
				if (e) {
					auto eptr = e.Set(nullptr);
					if (eptr) {
						std::exception_ptr copy{ *eptr };
						delete eptr;
						std::rethrow_exception(std::move(copy));
					}
				}

#endif
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

				using iterType = typename containerType::const_iterator;

				auto iter = container.cbegin();
				auto iter_end = container.cend();

				if ((iter_end - iter) > 0) {
					std::vector< returnT > out(iter_end - iter, returnT());

					impl::TaskGroup group;

					group.Dispatch<iterType>(iter_end - iter, [&](impl::JobArgs args) { // actual work
						iterType* iterInternal{ nullptr };
						iterInternal = static_cast<iterType*>(args.sharedmemory); // cast PTR
						if ((iterInternal) && (args.groupIndex == 0) && (args.jobIndex > 0)) std::advance(*iterInternal, args.jobIndex); // advance as needed if this is a new non-starting group

						if (*iterInternal != iter_end) {
							out[static_cast<int>(args.jobIndex)] = ToDo(**iterInternal);
							iterInternal->operator++();
						}
						}, [&iter](void* p)->void { // start-up
							new (p) iterType(iter);
						}, [&](void* p)->void { // close-out
							if constexpr (!std::is_pod<iterType>::value) {
								static_cast<iterType*>(p)->~iterType();
							}
						});

					group.Wait();

					return out;
				}
				else {
					return std::vector< returnT >();
				}

			}
		};

		/* wrapper for std::find_if */
		template<typename containerType, typename F> decltype(auto) Find(containerType& container, F const& ToDo) {
			return std::find_if(container.begin(), container.end(), [&](auto& x) ->bool { return ToDo(x); }); 
		};

		/* wrapper for std::find_if */
		template<typename containerType, typename F> decltype(auto) Find(containerType const& container, F const& ToDo) {
			return std::find_if(container.cbegin(), container.cend(), [&](auto const& x) ->bool { return ToDo(x); }); 
		};

		/* outputType x;
		for (auto& v : resultList){ x += v; }
		return x; */
		template<typename outputType>
		decltype(auto) Accumulate(std::vector<outputType> const& resultList) {
#if 0 // Works, and is very fast, but not as fast as the C++ for-loop and accumulation. It is just that hard to beat. 
			outputType finalSum{ 0 };
			fibers::synchronization::mutex lock;

			using iterType = typename std::vector<outputType>::const_iterator;

			auto iter = resultList.cbegin();
			auto iter_end = resultList.cend();

			if ((iter_end - iter) > 0) {
				impl::TaskGroup group;

				struct data_container{
					iterType iter;
					outputType partialSum;

					data_container() = default;
					data_container(iterType i) : iter(i), partialSum{ 0 } {};
					data_container(data_container const&) = default;
					data_container(data_container &&) = default;
					data_container& operator=(data_container const&) = default;
					data_container& operator=(data_container&&) = default;
					~data_container() = default;
				};

				group.Dispatch<data_container>(iter_end - iter, [&](impl::JobArgs args) { // actual work
					data_container* dataInternal{ nullptr };
					iterType* iterInternal{ nullptr };

					dataInternal = static_cast<data_container*>(args.sharedmemory); // cast PTR
					iterInternal = &dataInternal->iter; // cast PTR
					if ((iterInternal) && (args.groupIndex == 0) && (args.jobIndex > 0)) std::advance(*iterInternal, args.jobIndex); // advance as needed if this is a new non-starting group

					if (*iterInternal != iter_end) {
						dataInternal->partialSum += **iterInternal;
						iterInternal->operator++();
					}
				}, [&iter](void* p)->void { // start-up
					new (p) data_container(iter);
				}, [&](void* p)->void { // close-out
					{
						auto locked{ std::scoped_lock(lock) };
						finalSum += static_cast<data_container*>(p)->partialSum;
					}
					if constexpr (!std::is_pod<iterType>::value || !std::is_pod<outputType>::value) {
						static_cast<data_container*>(p)->~data_container();
					}
				});

				group.Wait();

				return outputType(finalSum);
			}

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
						auto list{ p->Wait_Get() };
						if (list.size() > 0) p2 = result->Set(new Any(list[0]));
						else p2 = result->Set(new Any());							
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