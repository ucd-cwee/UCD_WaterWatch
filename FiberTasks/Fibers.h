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
#pragma warning(disable : 4238)				// nonstandard extension used: class rvalue used as lvalue
#pragma warning(disable : 4239)				// conversion from 'T' to 'T&'
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4251)				// needs to have dll-interface
#pragma warning(disable : 4267)				// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4273)				// inconsistent DLL linkage
#pragma warning(disable : 4297)				// function assumed not to throw but does
#pragma warning(disable : 4302)				// truncation from 'void *' to 'int'
#pragma warning(disable : 4305)				// truncating a literal from double to float
#pragma warning(disable : 4311)				// pointer truncation from 'void *' to 'int'
#pragma warning(disable : 4312)				// conversion from 'int' to 'void*' of greater size
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

#define CONST_MAX( x, y )			( (x) > (y) ? (x) : (y) )
namespace fibers {

	namespace utilities {
		class Hardware {
		public:
			static int GetNumCpuCores();
			static float GetPercentCpuLoad();
		};
		
		template<typename Type = size_t>
		class Sequence {
		private:
			Type min;
			Type max;

		public:
			Sequence() : min(0), max(0) {};
			Sequence(Type N) : min(0), max(N) {};
			Sequence(Type N0, Type N1) : min(N0), max(N1) {};

			class Iterator : public std::iterator<std::random_access_iterator_tag, Type> {
			public:
				using difference_type = typename std::iterator<std::random_access_iterator_tag, Type>::difference_type;

				Iterator() : _ptr(0) {}
				Iterator(Type rhs) : _ptr(rhs) {}
				Iterator(const Iterator& rhs) : _ptr(rhs._ptr) {}

				inline Iterator& operator+=(difference_type rhs) { _ptr += rhs; return *this; }
				inline Iterator& operator-=(difference_type rhs) { _ptr -= rhs; return *this; }
				inline Type& operator*() { return _ptr; }
				inline Type* operator->() { return &_ptr; }
				inline Type operator[](difference_type rhs) { return static_cast<Type>(rhs); }
				inline const Type& operator*() const { return _ptr; }
				inline const Type* operator->() const { return &_ptr; }
				inline const Type operator[](difference_type rhs) const { return static_cast<Type>(rhs); }

				inline Iterator& operator++() { ++_ptr; return *this; }
				inline Iterator& operator--() { --_ptr; return *this; }
				inline Iterator operator++(int) { Iterator tmp(*this); ++_ptr; return tmp; }
				inline Iterator operator--(int) { Iterator tmp(*this); --_ptr; return tmp; }
				inline difference_type operator-(const Iterator& rhs) const { return _ptr - rhs._ptr; }
				inline Iterator operator+(difference_type rhs) const { return Iterator(_ptr + rhs); }
				inline Iterator operator-(difference_type rhs) const { return Iterator(_ptr - rhs); }
				friend inline Iterator operator+(difference_type lhs, const Iterator& rhs) { return Iterator(lhs + rhs._ptr); }
				friend inline Iterator operator-(difference_type lhs, const Iterator& rhs) { return Iterator(lhs - rhs._ptr); }

				inline bool operator==(const Iterator& rhs) const { return _ptr == rhs._ptr; }
				inline bool operator!=(const Iterator& rhs) const { return _ptr != rhs._ptr; }
				inline bool operator>(const Iterator& rhs) const { return _ptr > rhs._ptr; }
				inline bool operator<(const Iterator& rhs) const { return _ptr < rhs._ptr; }
				inline bool operator>=(const Iterator& rhs) const { return _ptr >= rhs._ptr; }
				inline bool operator<=(const Iterator& rhs) const { return _ptr <= rhs._ptr; }

			private:
				Type _ptr;

			};
			class ConstIterator : public std::iterator<std::random_access_iterator_tag, Type> {
			public:
				using difference_type = typename std::iterator<std::random_access_iterator_tag, Type>::difference_type;

				ConstIterator() : _ptr(0) {}
				ConstIterator(Type rhs) : _ptr(rhs) {}
				ConstIterator(const ConstIterator& rhs) : _ptr(rhs._ptr) {}

				inline ConstIterator& operator+=(difference_type rhs) { _ptr += rhs; return *this; }
				inline ConstIterator& operator-=(difference_type rhs) { _ptr -= rhs; return *this; }
				inline const Type& operator*() const { return _ptr; }
				inline const Type* operator->() const { return &_ptr; }
				inline const Type operator[](difference_type rhs) const { return static_cast<Type>(rhs); }

				inline ConstIterator& operator++() { ++_ptr; return *this; }
				inline ConstIterator& operator--() { --_ptr; return *this; }
				inline ConstIterator operator++(int) { ConstIterator tmp(*this); ++_ptr; return tmp; }
				inline ConstIterator operator--(int) { ConstIterator tmp(*this); --_ptr; return tmp; }
				inline difference_type operator-(const ConstIterator& rhs) const { return _ptr - rhs._ptr; }
				inline ConstIterator operator+(difference_type rhs) const { return ConstIterator(_ptr + rhs); }
				inline ConstIterator operator-(difference_type rhs) const { return ConstIterator(_ptr - rhs); }
				friend inline ConstIterator operator+(difference_type lhs, const ConstIterator& rhs) { return ConstIterator(lhs + rhs._ptr); }
				friend inline ConstIterator operator-(difference_type lhs, const ConstIterator& rhs) { return ConstIterator(lhs - rhs._ptr); }

				inline bool operator==(const ConstIterator& rhs) const { return _ptr == rhs._ptr; }
				inline bool operator!=(const ConstIterator& rhs) const { return _ptr != rhs._ptr; }
				inline bool operator>(const ConstIterator& rhs) const { return _ptr > rhs._ptr; }
				inline bool operator<(const ConstIterator& rhs) const { return _ptr < rhs._ptr; }
				inline bool operator>=(const ConstIterator& rhs) const { return _ptr >= rhs._ptr; }
				inline bool operator<=(const ConstIterator& rhs) const { return _ptr <= rhs._ptr; }

			private:
				Type _ptr;

			};

			auto begin() { return Iterator(min); };
			auto end() { return Iterator(max); };
			auto cbegin() const { return ConstIterator(min); };
			auto cend() const { return ConstIterator(max); };
			auto begin() const { return ConstIterator(min); };
			auto end() const { return ConstIterator(max); };
		};
	};
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
#if 0
			using ParentClass = vector;
			using IterType = typename underlying::value_type;
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
			SETUP_STL_ITERATOR(vector, typename underlying::value_type, it_state);
#endif
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
#if 0
			using ParentClass = array;
			using IterType = typename underlying::value_type;
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
			SETUP_STL_ITERATOR(array, typename underlying::value_type, it_state);
#endif
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

#if 0
			using ParentClass = unordered_map;
			using IterType = typename underlying::value_type;
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
			SETUP_STL_ITERATOR(unordered_map, typename underlying::value_type, it_state);
#endif

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

		class CriticalMutexLock {
		private:
			using mutexHandle_t = CRITICAL_SECTION;
			static void				Sys_MutexCreate(mutexHandle_t& handle) noexcept { InitializeCriticalSection(&handle); };
			static void				Sys_MutexDestroy(mutexHandle_t& handle) noexcept { DeleteCriticalSection(&handle); };
			static void				Sys_MutexLock(mutexHandle_t& handle) noexcept { EnterCriticalSection(&handle); };
			static bool				Sys_MutexTryLock(mutexHandle_t& handle) noexcept { return TryEnterCriticalSection(&handle) != 0; };
			static void				Sys_MutexUnlock(mutexHandle_t& handle) noexcept { LeaveCriticalSection(&handle); };
		public:
			CriticalMutexLock() noexcept { Sys_MutexCreate(Handle); };
			~CriticalMutexLock() noexcept { Sys_MutexDestroy(Handle); };

			inline void lock() {
				Sys_MutexLock(Handle);
			};
			inline bool try_lock() {
				return Sys_MutexTryLock(Handle);
			};
			inline void unlock() {
				Sys_MutexUnlock(Handle);
			};

			CriticalMutexLock(const CriticalMutexLock&) = delete;
			CriticalMutexLock(CriticalMutexLock&&) = delete;
			CriticalMutexLock& operator=(CriticalMutexLock const&) = delete;
			CriticalMutexLock& operator=(CriticalMutexLock&&) = delete;

		protected:
			mutexHandle_t Handle;

		};

		namespace impl {
			extern CriticalMutexLock* atomic_number_lock;
		};

		template<typename type = double>
		class atomic_number {
		public:
			static constexpr bool isFloatingPoint = std::is_floating_point<type>::value;
			static constexpr bool isSigned = std::is_signed<type>::value;

		private:
			static constexpr auto ValidTypeExample() {
				if constexpr (isFloatingPoint) return static_cast<type>(0);
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
			static constexpr auto ValidTypeExampleImpl = ValidTypeExample();
			using internalType = typename std::remove_const_t<decltype(ValidTypeExampleImpl)>;

		public:
			constexpr atomic_number() : value(static_cast<type>(0)) {};
			constexpr atomic_number(const type& a) : value(a) {};
			constexpr atomic_number(type&& a) : value(std::forward<type>(a)) {};
			atomic_number(const atomic_number& other) : value(other.GetValue()) {};
			atomic_number(atomic_number&& other) : value(std::move(other.value)) {};
			atomic_number& operator=(const atomic_number& other) { 
				SetValue(other.value);
				return *this; 
			};
			atomic_number& operator=(atomic_number&& other) {
				SetValue(std::move(other.value));
				return *this;
			};
			~atomic_number() = default;

			operator type() { return GetValue(); };
			operator const type() const { return GetValue(); };

			template <typename T> decltype(auto) operator+(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
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
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
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
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
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
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
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
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) }; 
					value--;  
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(-1));
					return *this;
				}
			};
			atomic_number& operator++() { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) }; 
					value++; 
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
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					value += i.value;
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(i.value));
					return *this;
				}
			};
			atomic_number& operator-=(const atomic_number& i) { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					value -= i.value;
					return *this;
				}
				else {
					InterlockedExchangeAdd(&value, static_cast<internalType>(-i.value));
					return *this;
				}
			};
			atomic_number& operator/=(const atomic_number& i) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					value /= i.value;
					return *this;
				}
				else {
					InterlockedExchange(&value, static_cast<internalType>(value / i.value));
					return *this;
				}
			};			
			atomic_number& operator*=(const atomic_number& i) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					value *= i.value;
					return *this;
				}
				else {
					InterlockedExchange(&value, static_cast<internalType>(value * i.value));
					return *this;
				}
			};

			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator+=(const atomic_number<T>& i) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value += static_cast<type>(i.value);
					}
					else {
						InterlockedExchangeAdd(&value, static_cast<internalType>(i.value));
					}
				}
				else {
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value += static_cast<type>(i.value);
					}
					else {
						InterlockedExchangeAdd(&value, static_cast<internalType>(i.value));
					}
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator-=(const atomic_number<T>& i) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value -= static_cast<type>(i.value);
					}
					else {
						InterlockedExchangeAdd(&value, static_cast<internalType>(-i.value));
					}
				}
				else {
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value -= static_cast<type>(i.value);
					}
					else {
						InterlockedExchangeAdd(&value, static_cast<internalType>(-i.value));
					}
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator/=(const atomic_number<T>& i) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value /= static_cast<type>(i.value);
					}
					else {
						InterlockedExchange(&value, value / static_cast<internalType>(i.value));
					}
				}
				else {
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value /= static_cast<type>(i.value);
					}
					else {
						InterlockedExchange(&value, value / static_cast<internalType>(i.value));
					}
				}
				return *this;
			};
			template<typename T, typename = std::enable_if_t<!std::is_same_v<type, T>>>
			atomic_number& operator*=(const atomic_number<T>& i) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value *= static_cast<type>(i.value);
					}
					else {
						InterlockedExchange(&value, value * static_cast<internalType>(i.value));
					}
				}
				else {
					if constexpr (atomic_number<type>::isFloatingPoint) {
						value *= static_cast<type>(i.value);
					}
					else {
						InterlockedExchange(&value, value * static_cast<internalType>(i.value));
					}
				}
				return *this;
			};

			template <typename T> bool operator==(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) == static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) == static_cast<type>(b.value);
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) == static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) == static_cast<type>(b.value);
					}
				}
			};
			template <typename T> bool operator!=(const atomic_number<T>& b) {
				return !operator==(b);
			};
			template <typename T> bool operator<=(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) <= static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) <= static_cast<type>(b.value);
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) <= static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) <= static_cast<type>(b.value);
					}
				}
			};
			template <typename T> bool operator<(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) < static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) < static_cast<type>(b.value);
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) < static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) < static_cast<type>(b.value);
					}
				}
			};
			template <typename T> bool operator>=(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) >= static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) >= static_cast<type>(b.value);
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) >= static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) >= static_cast<type>(b.value);
					}
				}
			};
			template <typename T> bool operator>(const atomic_number<T>& b) {
				if constexpr (atomic_number<T>::isFloatingPoint || atomic_number<type>::isFloatingPoint) {
					// one of these is a floating-point type, therefore the lock is needed.
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) > static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) > static_cast<type>(b.value);
					}
				}
				else {
					if constexpr (sizeof(T) > sizeof(type)) {
						return static_cast<T>(value) > static_cast<T>(b.value);
					}
					else {
						return static_cast<type>(value) > static_cast<type>(b.value);
					}
				}
			};

			atomic_number pow(atomic_number const& V) const { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return std::pow(value, V.value);
				}
				else {
					return std::pow(value, V.value);
				}
			};
			atomic_number sqrt() const { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return std::sqrt(value);
				}
				else {
					return std::sqrt(value);
				}
			};
			atomic_number floor() const { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return std::floor(value);
				}
				else {
					return std::floor(value);
				}
			};
			atomic_number ceiling() const { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return std::ceil(value);
				}
				else {
					return std::ceil(value);
				}
			};

			type Increment() { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return ++value;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(1)) + 1;
				}
			} // atomically increments the value and returns the new value
			type Decrement() { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return --value;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-1)) - 1;
				}
			} // atomically decrements the value and returns the new value
			type Add(const type& v) { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return value += v;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(v)) + v;
				}
			} // atomically adds a value and returns the new value
			type Sub(const type& v) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return value -= v;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-v)) - v;
				}
			}; // atomically subtracts a value and returns the new value
			type GetValue() const { 
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					return value;
				}
				else {
					return value;
				}				
			}
			type SetValue(const type& v) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					type out{value};
					value = v;
					return out;
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting with the new value
			type SetValue(type&& v) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					type out{ std::forward< type>(value) };
					value = v;
					return out;
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting with the new value
		
        public: // std::atomic compatability
			type fetch_add(type const& v) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					type out{value};
					value += v;
					return out;					
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while incrementing the actual counter
			type fetch_sub(type const& v) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					type out{ value };
					value -= v;
					return out;
				}
				else {
					return InterlockedExchangeAdd(&value, static_cast<internalType>(-v));
				}
			}; // returns the previous value while decrementing the actual counter
			static constexpr bool is_lock_free() {
				if constexpr (isFloatingPoint) {
					return false;
				}
				else {
					return true;
				}
			}; // returns whether a lock is utilized or the number is actually lock-free
			type exchange(type const& v) {
				if constexpr (isFloatingPoint) {
					auto Locked{ std::scoped_lock(*impl::atomic_number_lock) };
					type out{ value };
					value = v;
					return out;
				}
				else {
					return InterlockedExchange(&value, static_cast<internalType>(v));
				}
			}; // returns the previous value while setting the value to the input
			type load() const {
				return GetValue();
			};
			void store(type const& v) {
				(void)exchange(v);
			}; // sets the value to the input

		public:
			internalType value;

		};

		template< typename T> 
		class atomic_ptr {
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
			/* atomically sets the pointer to 'newPtr' only if the previous pointer is equal to 'comparePtr' */
			T* CompareExchange(T* comparePtr, T* newPtr) noexcept {
				return static_cast<T*>(Sys_InterlockedCompareExchangePointer((void*&)ptr, static_cast<void*>(comparePtr), static_cast<void*>(newPtr)));
			};

			T* operator->() noexcept { return Get(); };
			const T* operator->() const noexcept { return Get(); };
			T* Get() noexcept { return ptr; };
			T* Get() const noexcept { return ptr; };

		protected:
			T* ptr;
		};
	};

	namespace utilities {
		__forceinline static void* Mem_Alloc16(const size_t& size) {
			if (!size) return NULL; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16);
		};
		__forceinline static void  Mem_Free16(void* ptr) {
			if (ptr)
				::_aligned_free(ptr);
		};
		__forceinline static void* Mem_ClearedAlloc(const size_t& size) {
			void* mem = Mem_Alloc16(size);
			::memset(mem, 0, size);
			return mem;
		};
		__forceinline static void  Mem_Free(void* ptr) { Mem_Free16(ptr); }
		__forceinline static void* Mem_Alloc(const size_t size) { return Mem_ClearedAlloc(size); }
		__forceinline static char* Mem_CopyString(const char* in) {
			size_t L{ strlen(in) + 1 };
			char* out = (char*)Mem_Alloc(L);
			::strncpy(out, in, L - 1); // ::strcpy_s(out, L, in);
			return out;
		};

		template<class _type_, int _blockSize_> class BlockAllocator {
		public:
			BlockAllocator(bool clear = true) : // = false
				blocks(nullptr),
				free(nullptr),
				total(0),
				active(0),
				allowAllocs(true),
				clearAllocs(clear)
			{};
			BlockAllocator(int toReserve) :
				blocks(nullptr),
				free(nullptr),
				total(0),
				active(0),
				allowAllocs(true),
				clearAllocs(true)
			{
				Reserve(toReserve);
			};
			~BlockAllocator() {
				Shutdown();
			};

			// returns total size of allocated memory
			size_t				Allocated() const { return total * sizeof(_type_); }

			// returns total size of allocated memory including size of (*this)
			size_t				Size() const { return sizeof(*this) + Allocated(); }

			void				Shutdown() {
				while (blocks != nullptr) {
					cweeBlock* block = blocks;
					blocks = blocks->next;
					Mem_Free(block);
				}
				blocks = nullptr;
				free = nullptr;
				total = active = 0;
			};
			__forceinline void	SetFixedBlocks(long long numBlocks) {
				long long currentNumBlocks = 0;
				for (cweeBlock* block = blocks; block != nullptr; block = block->next) {
					currentNumBlocks++;
				}
				for (long long i = currentNumBlocks; i < numBlocks; i++) {
					AllocNewBlock();
				}
				allowAllocs = false;
			};
			__forceinline void	FreeEmptyBlocks() {
				// first count how many free elements are in each block and build up a free chain per block
				for (cweeBlock* block = blocks; block != nullptr; block = block->next) {
					block->free = nullptr;
					block->freeCount = 0;
				}
				for (element_t* element = free; element != nullptr; ) {
					element_t* next = element->next;
					for (cweeBlock* block = blocks; block != nullptr; block = block->next) {
						if (element >= block->elements && element < block->elements + _blockSize_) {
							element->next = block->free;
							block->free = element;
							block->freeCount++;
							break;
						}
					}
					// if this assert fires, we couldn't find the element in any block
					assert(element->next != next);
					element = next;
				}
				// now free all blocks whose free count == _blockSize_
				cweeBlock* prevBlock = nullptr;
				for (cweeBlock* block = blocks; block != nullptr; ) {
					cweeBlock* next = block->next;
					if (block->freeCount == _blockSize_) {
						if (prevBlock == nullptr) {
							assert(blocks == block);
							blocks = block->next;
						}
						else {
							assert(prevBlock->next == block);
							prevBlock->next = block->next;
						}
						Mem_Free(block);
						total -= _blockSize_;
					}
					else {
						prevBlock = block;
					}
					block = next;
				}
				// now rebuild the free chain
				free = nullptr;
				for (cweeBlock* block = blocks; block != nullptr; block = block->next) {
					for (element_t* element = block->free; element != nullptr; ) {
						element_t* next = element->next;
						element->next = free;
						free = element;
						element = next;
					}
				}
			};

			static constexpr bool isPod() { return std::is_pod<_type_>::value; };
			_type_* Alloc() {
				if (free == nullptr) {
					if (!allowAllocs) {
						return nullptr;
					}
					AllocNewBlock();
				}

				active++;
				element_t* element = free;
				free = free->next;
				element->next = nullptr;

				_type_* t = (_type_*)element->buffer;
				if constexpr (isPod()) {
					memset(t, 0, sizeof(_type_));
				}
				else {
					if (clearAllocs) {
						memset(t, 0, sizeof(_type_));
					}
					new (t) _type_;
				}

				return t;
			};
			void				Free(_type_* element) {
				if (element == nullptr) {
					return;
				}

				if constexpr (!isPod()) {
					element->~_type_();
				}

				element_t* t = (element_t*)(element);
				t->next = free;
				free = t;
				active--;
			};
			__forceinline void	Reserve(long long num) {
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
			long long			GetTotalCount() const { return total; }
			long long			GetAllocCount() const { return active; }
			long long			GetFreeCount() const { return total - active; }

		private:
			union element_t {
				_type_* data;
				element_t* next;
				::byte			buffer[(CONST_MAX(sizeof(_type_), sizeof(element_t*)) + (16 - 1)) & ~(16 - 1)];
			};

			class cweeBlock {
			public:
				element_t		elements[_blockSize_];
				cweeBlock* next;
				element_t* free;		// list with free elements in this block (temp used only by FreeEmptyBlocks)
				long long		freeCount;	// number of free elements in this block (temp used only by FreeEmptyBlocks)
			};

			cweeBlock* blocks;
			element_t* free;
			long long			total;
			long long			active;
			bool				allowAllocs;
			bool				clearAllocs;

			void			AllocNewBlock() {
				cweeBlock* block = (cweeBlock*)Mem_Alloc((size_t)(sizeof(cweeBlock)));
				block->next = blocks;
				blocks = block;
				for (int i = 0; i < _blockSize_; i++) {
					block->elements[i].next = free;
					free = &block->elements[i];
					assert((((UINT_PTR)free) & (16 - 1)) == 0);
				}
				total += _blockSize_;
			};
		};

		template<class _type_, size_t BlockSize = 128> class Allocator {
		private:
			static constexpr bool isPod() { return std::is_pod<_type_>::value; };

		public:
			Allocator() : lock(), ptrs(), alloc() {};
			Allocator(int toReserve) : lock(), ptrs(), alloc(toReserve) {};
			~Allocator() { Clear(); };

			_type_* Alloc() {
				auto locked{ std::scoped_lock(lock) };
				decltype(auto) p = alloc.Alloc();
				if constexpr (!isPod()) {
					ptrs.insert(p);
				}
				return p;
			};
			void	Free(_type_* element) {
				auto locked{ std::scoped_lock(lock) };
				if constexpr (!isPod()) {
					ptrs.erase(element);
				}
				alloc.Free(element);
			};
			void	Clean() {
				auto locked{ std::scoped_lock(lock) };
				alloc.FreeEmptyBlocks();
			};
			long long	GetTotalCount() const {
				long long out;
				auto locked{ std::scoped_lock(lock) };
				out = alloc.GetTotalCount();
				return out;
			};
			long long	GetAllocCount() const {
				long long out;
				auto locked{ std::scoped_lock(lock) };
				if constexpr (!isPod()) {
					out = ptrs.size();
				}
				else {
					out = alloc.GetAllocCount();
				}
				return out;
			};
			void	Clear() {
				auto locked{ std::scoped_lock(lock) };
				if constexpr (!isPod()) {
					for (auto& x : ptrs) {
						if (x != nullptr) {
							alloc.Free(x);
						}
					}
					ptrs.clear();
				}
				else {
					alloc.Shutdown();
					alloc.Free(alloc.Alloc());
				}
			};
			void	Reserve(long long n) {
				auto locked{ std::scoped_lock(lock) };
				alloc.Reserve(n);
			};

		private:
			mutable synchronization::CriticalMutexLock lock;
			std::set<_type_*> ptrs;
			BlockAllocator<_type_, BlockSize> alloc;
		};

	};

	namespace containers {
		template<typename _Value_type> using number = fibers::synchronization::atomic_number<_Value_type>;

		template< class objType, class keyType, int maxChildrenPerNode = 10 >
		class Tree {
		public:
			struct TreeNode {

				keyType				  key;							// key used for sorting
				objType* object;						// if != NULL pointer to object stored in leaf node
				TreeNode* parent;						// parent node
				TreeNode* next;							// next sibling
				TreeNode* prev;							// prev sibling
				long long			  numChildren;					// number of children
				TreeNode* firstChild;					// first child
				TreeNode* lastChild;					// last child
			};

			__forceinline TreeNode* InitNode(TreeNode* p) {
				p->key = 0;
				p->object = nullptr;
				p->parent = nullptr;
				p->next = nullptr;
				p->prev = nullptr;
				p->numChildren = 0;
				p->firstChild = nullptr;
				p->lastChild = nullptr;
				return p;
			};

		public:
			typedef TreeNode _iterType;
			struct it_state {
				_iterType _node;
				_iterType* node = &_node;
				inline void begin(const Tree* ref) {
					node = ref->GetFirst();
					if (!node) node = &_node;
				};
				inline void begin_at(const Tree* ref, keyType key) {
					node = ref->NodeFindLargestSmallerEqual(key);
					if (!node) this->begin(ref);
				};
				inline void next(const Tree* ref) {
					node = ref->GetNextLeaf(node);
					if (!node) node = &_node;
				};
				inline void end(const Tree* ref) {
					node = &_node;
				};
				inline _iterType& get(Tree* ref) {
					return *node;
				};
				inline bool cmp(const it_state& s) const {
					return !(!node->object || (node->object == s.node->object));
				};

				inline long long distance(const it_state& s) const { throw(std::runtime_error("Cannot evaluate distance")); }
				// Optional to allow operator--() and reverse iterators:
				inline void prev(const Tree* ref) {
					node = ref->GetPrevLeaf(node);
					if (!node) node = &_node;
				};
				// Optional to allow `const_iterator`:
				inline const _iterType& get(const Tree* ref) const { return *node; }
			};
#if 0
			using ParentClass = Tree;
			using IterType = _iterType;
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
			SETUP_STL_ITERATOR(Tree, _iterType, it_state);
#endif
			iterator begin_at(keyType key) {
				decltype(auto) iter = this->begin();
				iter.state.begin_at(this, key);
				return iter;
			};
			const_iterator begin_at(keyType key) const {
				decltype(auto) iter = this->begin();
				iter.state.begin_at(this, key);
				return iter;
			};
			const_iterator cbegin_at(keyType key) const {
				decltype(auto) iter = this->cbegin();
				iter.state.begin_at(this, key);
				return iter;
			};

		private:
			synchronization::atomic_number<long long> Num; // thread-safe
			synchronization::atomic_ptr< TreeNode > root; // thread-safe
			synchronization::atomic_ptr< TreeNode > first; // thread-safe
			synchronization::atomic_ptr< TreeNode > last; // thread-safe
			utilities::Allocator<objType, maxChildrenPerNode>	objAllocator; // thread-safe
			utilities::Allocator<TreeNode, maxChildrenPerNode>			nodeAllocator; // thread-safe

		public:
			Tree& operator=(const Tree& obj) {
				Clear(); // empty out whatever this container had 

				for (auto& x : obj) {
					Add(*x.object, x.key, false);
				}

				return *this;
			};
			bool operator==(const Tree& obj) {
				return GetFirst() == obj.GetFirst() && GetLast() == obj.GetLast();
			};
			bool operator!=(const Tree& obj) { return !operator==(obj); };

			Tree() : Num(0), root(nullptr), first(nullptr), last(nullptr), objAllocator(), nodeAllocator() {
				static_assert(maxChildrenPerNode >= 4);
				Init();
			};
			Tree(int toReserve) :
				Num(0),
				root(nullptr),
				first(nullptr),
				last(nullptr),
				objAllocator(toReserve),
				nodeAllocator(toReserve * 1.25)
			{
				static_assert(maxChildrenPerNode >= 4);
				Init();
			};
			~Tree() {
				Clear();
			};

			void									Reserve(long long num) {
				objAllocator.Reserve(num);
				nodeAllocator.Reserve(num * 1.25); // approximately 25% more for 'overage'
			};

			TreeNode* Add(objType const& object, keyType const& key, bool addUnique = true) {
				TreeNode* node, * child, * newNode; objType* OBJ;

				if (root == nullptr) {
					root = AllocNode();
				}

				// check that the key does not already exist		
				if (addUnique) {
					node = NodeFind(key);
					if (node && node->object) {
						*node->object = const_cast<objType&>(object);
						return CheckLastNode(CheckFirstNode(node));
					}
				}

				if (root->numChildren >= maxChildrenPerNode) {
					newNode = AllocNode();
					newNode->key = root->key;
					newNode->firstChild = root;
					newNode->lastChild = root;
					newNode->numChildren = 1;
					root->parent = newNode;
					SplitNode(root);
					root = newNode;
				}

				newNode = AllocNode();
				newNode->key = key;

				OBJ = nullptr;
				{
					OBJ = objAllocator.Alloc();
					*OBJ = const_cast<objType&>(object);
					Num++;
				}

				newNode->object = OBJ;

				for (node = root; node->firstChild != nullptr; node = child) {

					if (key > node->key) {
						node->key = key;
					}

					// find the first child with a key larger equal to the key of the new node
					for (child = node->firstChild; child->next; child = child->next) {
						if (key <= child->key) {
							break;
						}
					}

					if (child->object) {

						if (key <= child->key) {
							// insert new node before child
							if (child->prev) {
								child->prev->next = newNode;
							}
							else {
								node->firstChild = newNode;
							}
							newNode->prev = child->prev;
							newNode->next = child;
							child->prev = newNode;
						}
						else {
							// insert new node after child
							if (child->next) {
								child->next->prev = newNode;
							}
							else {
								node->lastChild = newNode;
							}
							newNode->prev = child;
							newNode->next = child->next;
							child->next = newNode;
						}

						newNode->parent = node;
						node->numChildren++;

						return CheckLastNode(CheckFirstNode(newNode));
					}

					// make sure the child has room to store another node
					if (child->numChildren >= maxChildrenPerNode) {
						SplitNode(child);
						if (key <= child->prev->key) {
							child = child->prev;
						}
					}
				}

				// we only end up here if the root node is empty
				newNode->parent = root;
				root->key = key;
				root->firstChild = newNode;
				root->lastChild = newNode;
				root->numChildren++;

				return CheckLastNode(CheckFirstNode(newNode));
			};
			void									Add(std::vector<std::pair<keyType, objType>> const& data, bool addUnique = true) {
				for (auto& source : data) {
					Add(source.second, source.first, addUnique);
				}
			};
			
			void									Remove(TreeNode* node) {
				if (!node) return;

				if (first == node) {
					first = this->GetNextLeaf(node);
				}

				if (last == node) {
					last = this->GetPrevLeaf(node);
				}

				TreeNode* parent, * oldRoot;

				// unlink the node from it's parent
				if (node->prev) {
					node->prev->next = node->next;
				}
				else {
					node->parent->firstChild = node->next;
				}
				if (node->next) {
					node->next->prev = node->prev;
				}
				else {
					node->parent->lastChild = node->prev;
				}
				node->parent->numChildren--;

				// make sure there are no parent nodes with a single child
				for (parent = node->parent; parent != root && parent->numChildren <= 1; parent = parent->parent) {

					if (parent->next) {
						parent = MergeNodes(parent, parent->next);
					}
					else if (parent->prev) {
						parent = MergeNodes(parent->prev, parent);
					}

					// a parent may not use a key higher than the key of it's last child
					if (parent->key > parent->lastChild->key) {
						parent->key = parent->lastChild->key;
					}

					if (parent->numChildren > maxChildrenPerNode) {
						SplitNode(parent);
						break;
					}
				}
				for (; parent != nullptr && parent->lastChild != nullptr; parent = parent->parent) {
					// a parent may not use a key higher than the key of it's last child
					if (parent->key > parent->lastChild->key) {
						parent->key = parent->lastChild->key;
					}
				}

				// free the node
				FreeNode(node);

				// remove the root node if it has a single internal node as child
				if (root->numChildren == 1 && root->firstChild->object == nullptr) {
					oldRoot = root;
					root->firstChild->parent = nullptr;
					root = root->firstChild;
					FreeNode(oldRoot);
				}
			};				// remove an object node from the tree
			void									Clear() {
				// while (first) { Remove(first); }

				// remove all
				nodeAllocator.Clear();
				objAllocator.Clear();
				root = nullptr;
				first = nullptr;
				last = nullptr;
				Num = 0;

				Init();
			};
			TreeNode* NodeFindByIndex(int index) const {
				if (index <= 0) return first;
				else if (index >= (Num - 1)) return last;
				else return NodeFindByIndex(index, const_cast<TreeNode*>(root.Get()));
			};
			TreeNode* NodeFind(keyType  const& key) const {
				return NodeFind(key, const_cast<TreeNode*>(root.Get()));
			};								// find an object using the given key;
			TreeNode* NodeFindSmallestLargerEqual(keyType const& key) const {
				return NodeFindSmallestLargerEqual(key, const_cast<TreeNode*>(root.Get()));
			};			// find an object with the smallest key larger equal the given key;
			TreeNode* NodeFindLargestSmallerEqual(keyType const& key) const {
				return NodeFindLargestSmallerEqual(key, const_cast<TreeNode*>(root.Get()));
			};			// find an object with the largest key smaller equal the given key;

			static TreeNode* NodeFind(keyType  const& key, TreeNode* root) {
				TreeNode* node = NodeFindLargestSmallerEqual(key, root);
				if (node && node->object && node->key == key) return node;
				return nullptr;
			};								// find an object using the given key;


			static TreeNode* NodeFindByIndex(int index, TreeNode* Root) {
				int startIndex{ 0 };

				if (Root == nullptr) {
					return nullptr;
				}

				while (Root) {
					if (index == startIndex && Root->object) { return Root; }

					if (startIndex <= index && (startIndex + Root->numChildren) > index) {
						// one of my children has this index				
						Root = Root->firstChild;
					}
					else {
						// one of my neighbors has this index				
						if (Root->object) ++startIndex;
						else startIndex += Root->numChildren;

						Root = Root->next;
					}
				}

				return Root;
			};			// find an object with the largest key smaller equal the given key;

			static TreeNode* NodeFindSmallestLargerEqual(keyType const& key, TreeNode* Root) {
				TreeNode* node, * smaller;

				if (Root == nullptr) {
					return nullptr;
				}

				smaller = nullptr;
				for (node = Root->lastChild; node != nullptr; node = node->lastChild) {
					//if (node->lastChild && node->firstChild) {
					//	if (node->firstChild->key > node->lastChild->key) {
					//		node = GetPrevLeaf(Root);
					//		break;
					//	}
					//}
					while (node->prev) {
						if (node->key <= key) {
							if (!smaller) {
								smaller = GetPrevLeaf(Root);
							}
							break;
						}
						smaller = node;
						node = node->prev;
					}
					if (node->object) {
						if (node->key >= key) {
							break;
						}
						else if (smaller == nullptr) {
							return nullptr;
						}
						else {
							node = smaller;
							if (node->object) {
								break;
							}
						}
					}
				}

				return node;
			};			// find an object with the smallest key larger equal the given key;
			static TreeNode* NodeFindLargestSmallerEqual(keyType const& key, TreeNode* Root) {
				TreeNode* node, * smaller;

				if (Root == nullptr) {
					return nullptr;
				}

				smaller = nullptr;
				for (node = Root->firstChild; node != nullptr; node = node->firstChild) {
					while (node->next) {
						if (node->key >= key) {
							if (!smaller) {
								smaller = GetNextLeaf(Root);
							}
							break;
						}
						smaller = node;
						node = node->next;
					}
					if (node->object) {
						if (node->key <= key) {
							break;
						}
						else if (smaller == nullptr) {
							return nullptr;
						}
						else {
							node = smaller;
							if (node->object) {
								break;
							}
						}
					}
				}
				return node;
			};			// find an object with the largest key smaller equal the given key;

			objType* Find(keyType  const& key) const {
				TreeNode* node = NodeFind(key, root);
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};									// find an object using the given key;
			objType* FindSmallestLargerEqual(keyType const& key) const {
				TreeNode* node = NodeFindSmallestLargerEqual(key, root);
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};				// find an object with the smallest key larger equal the given key;
			objType* FindLargestSmallerEqual(keyType const& key) const {
				TreeNode* node = NodeFindLargestSmallerEqual(key, root);
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};				// find an object with the largest key smaller equal the given key;

			TreeNode* GetFirst() const { return first; };
			TreeNode* GetLast() const { return last; };
			TreeNode* GetRoot() const { return root; };
			long long								GetNodeCount() const {
				return Num;
			};										// returns the total number of nodes in the tree;
			long long								GetReservedCount() const {
				return objAllocator.GetTotalCount();  // .Num(); //  
			};
			static TreeNode* GetNext(TreeNode* node) {
				if (node) {
					if (node->firstChild) {
						node = node->firstChild;
					}
					else {
						while (node && node->next == nullptr) {
							node = node->parent;
						}
					}
				}
				return node;

				//if (!node) return nullptr; 
				//if (node->firstChild) {
				//	return node->firstChild;
				//}
				//else {
				//	while (node && node->next == nullptr) {
				//		node = node->parent;
				//	}
				//	return node;
				//}
			};		// goes through all nodes of the tree;

		public:
			static TreeNode* GetNextLeaf(TreeNode* node) {
				if (node) {
					if (node->firstChild) {
						while (node->firstChild) {
							node = node->firstChild;
						}
					}
					else {
						while (node && !node->next) {
							node = node->parent;
						}
						if (node) {
							node = node->next;
							while (node->firstChild) {
								node = node->firstChild;
							}
						}
						else {
							node = nullptr;
						}
					}
				}
				return node;

				//if (!node) return nullptr;
				//if (node->firstChild) {
				//	while (node->firstChild) {
				//		node = node->firstChild;
				//	}
				//	return node;
				//}
				//else {
				//	while (node && node->next == nullptr) {
				//		node = node->parent;
				//	}
				//	if (node) {
				//		node = node->next;
				//		while (node->firstChild) {
				//			node = node->firstChild;
				//		}
				//		return node;
				//	}
				//	else {
				//		return nullptr;
				//	}
				//}
			};	// goes through all leaf nodes of the tree;
			static TreeNode* GetPrevLeaf(TreeNode* node) {
				if (!node) return nullptr;
				if (node->lastChild) {
					while (node->lastChild) {
						node = node->lastChild;
					}
					return node;
				}
				else {
					while (node && node->prev == nullptr) {
						node = node->parent;
					}
					if (node) {
						node = node->prev;
						while (node->lastChild) {
							node = node->lastChild;
						}
						return node;
					}
					else {
						return nullptr;
					}
				}
			};	// goes through all leaf nodes of the tree;

		private:
			TreeNode* CheckFirstNode(TreeNode* newNode) {
				if (newNode && first) {
					if (newNode->key < first->key) {
						first = newNode;
					}
				}
				else {
					first = newNode;
				}
				return newNode;
			};
			TreeNode* CheckLastNode(TreeNode* newNode) {
				if (newNode && last) {
					if (newNode->key > last->key) {
						last = newNode;
					}
				}
				else {
					last = newNode;
				}
				return newNode;
			};
			void									Init() {
				root = AllocNode();
				{ // helps init the objAllocator
					auto x = objAllocator.Alloc();
					objAllocator.Free(x);
				}
			};
			void									Shutdown() {
				nodeAllocator.Clear();

				objAllocator.Clear();
				root = nullptr;
				first = nullptr;
				last = nullptr;
				Num = 0;
			};
			TreeNode* AllocNode() {
				TreeNode* node;

				node = nodeAllocator.Alloc();
				return InitNode(node);

				//node->key = 0;
				//node->parent = nullptr;
				//node->next = nullptr;
				//node->prev = nullptr;
				//node->numChildren = 0;
				//node->firstChild = nullptr;
				//node->lastChild = nullptr;
				//node->object = nullptr;

				//return node;
			};
			void									FreeNode(TreeNode* node) {
				if (node && node->object) {
					objAllocator.Free(node->object);  // RemoveFast(node->object); // 
					Num--;
				}
				nodeAllocator.Free(node); // RemoveFast(node); //  
			};
			void									SplitNode(TreeNode* node) {
				long long i;
				TreeNode* child, * newNode;

				// allocate a new node
				newNode = AllocNode();
				newNode->parent = node->parent;

				// divide the children over the two nodes
				child = node->firstChild;
				child->parent = newNode;
				for (i = 3; i < node->numChildren; i += 2) {
					child = child->next;
					child->parent = newNode;
				}

				newNode->key = child->key;
				newNode->numChildren = node->numChildren / 2;
				newNode->firstChild = node->firstChild;
				newNode->lastChild = child;

				node->numChildren -= newNode->numChildren;
				node->firstChild = child->next;

				child->next->prev = nullptr;
				child->next = nullptr;

				// add the new child to the parent before the split node
				assert(node->parent->numChildren < maxChildrenPerNode);

				if (node->prev) {
					node->prev->next = newNode;
				}
				else {
					node->parent->firstChild = newNode;
				}
				newNode->prev = node->prev;
				newNode->next = node;
				node->prev = newNode;

				node->parent->numChildren++;
			};
			TreeNode* MergeNodes(TreeNode* node1, TreeNode* node2) {
				TreeNode* child;

				assert(node1->parent == node2->parent);
				assert(node1->next == node2 && node2->prev == node1);
				assert(node1->object == nullptr && node2->object == nullptr);
				assert(node1->numChildren >= 1 && node2->numChildren >= 1);

				for (child = node1->firstChild; child->next; child = child->next) {
					child->parent = node2;
				}
				child->parent = node2;
				child->next = node2->firstChild;
				node2->firstChild->prev = child;
				node2->firstChild = node1->firstChild;
				node2->numChildren += node1->numChildren;

				// unlink the first node from the parent
				if (node1->prev) {
					node1->prev->next = node2;
				}
				else {
					node1->parent->firstChild = node2;
				}
				node2->prev = node1->prev;
				node2->parent->numChildren--;

				FreeNode(node1);

				return node2;
			};

		};
	};

	namespace utilities {
		template <class _Diff>
		struct Static_partition_key { // "pointer" identifying a static partition
			size_t _Chunk_number; // In range [0, numeric_limits<_Diff>::max()]
			_Diff _Start_at;
			_Diff _Size;
			explicit operator bool() const { // test if this is a valid key
				return _Chunk_number != static_cast<size_t>(-1);
			}
		};

		template <class _Diff>
		struct Static_partition_team { // common data for all static partitioned ops
			fibers::containers::number<size_t> _Consumed_chunks;
			size_t _Chunks;
			_Diff _Count;
			_Diff _Chunk_size;
			_Diff _Unchunked_items;

			constexpr Static_partition_team(const _Diff _Count_, const size_t _Chunks_) : _Consumed_chunks{ 0 }, _Chunks{ _Chunks_ }, _Count{ _Count_ }, _Chunk_size{ static_cast<_Diff>(
																			   _Count_ / static_cast<_Diff>(_Chunks_)) },
				_Unchunked_items{ static_cast<_Diff>(_Count_ % static_cast<_Diff>(_Chunks_)) } {
				// Calculate common data for statically partitioning iterator ranges.
				// pre: _Count_ >= _Chunks_ && _Chunks_ >= 1
			}

			Static_partition_key<_Diff> Get_chunk_key(const size_t _This_chunk) const {
				const auto _This_chunk_diff = static_cast<_Diff>(_This_chunk);
				auto _This_chunk_size = _Chunk_size;
				auto _This_chunk_start_at = static_cast<_Diff>(_This_chunk_diff * _This_chunk_size);
				if (_This_chunk_diff < _Unchunked_items) {
					// chunks at index lower than _Unchunked_items get an extra item,
					// and need to shift forward by all their predecessors' extra items
					_This_chunk_start_at += _This_chunk_diff;
					++_This_chunk_size;
				}
				else { // chunks without an extra item need to account for all the extra items
					_This_chunk_start_at += _Unchunked_items;
				}

				return { _This_chunk, _This_chunk_start_at, _This_chunk_size };
			}

			_Diff Get_chunk_offset(const size_t _This_chunk) const {
				const auto _This_chunk_diff = static_cast<_Diff>(_This_chunk);
				return _This_chunk_diff * _Chunk_size + (_STD min)(_This_chunk_diff, _Unchunked_items);
			}

			Static_partition_key<_Diff> Get_next_key() {
				// retrieves the next static partition key to process, if it exists;
				// otherwise, retrieves an invalid partition key
				const auto _This_chunk = _Consumed_chunks.fetch_add(1);
				if (_This_chunk < _Chunks) {
					return Get_chunk_key(_This_chunk);
				}

				return { static_cast<size_t>(-1), 0, 0 };
			}
		};
	};

	namespace impl {
		bool Initialize(uint32_t maxThreadCount = ~0u);
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
		};
		
		template <typename T> struct Queue {
			std::deque<T> queue;
			synchronization::CriticalMutexLock locker;

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
			synchronization::CriticalMutexLock wakeMutex; // std::mutex wakeMutex; // 
			fibers::containers::number<long long> nextQueue{ 0 };
			std::vector<std::thread> threads;
			void ShutDown() {
				alive = false; // indicate that new jobs cannot be started from this point
				bool wake_loop = true;
				std::thread waker([&] {
					while (wake_loop)
					{
						wakeCondition.notify_all(); // wakes up sleeping worker threads
					}
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
		void Dispatch(context& ctx, uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>& task, size_t sharedmemory_size = 0) noexcept;

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
			auto Dispatch(uint32_t jobCount, const std::function<void(JobArgs)>& task) { return impl::Dispatch(ctx, jobCount, (10000.0 / 400.0) * jobCount, task); };
		};
	};

	class JobGroup;

	/*! Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wrapper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	auto __awaiter__ = Job([](){ return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, and guarrantees its completion before the scope ends. */
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

			group.Dispatch(end - start, [=](impl::JobArgs args) {
				ToDo(start + args.jobIndex);
			});			
		};

		/* parallel_for (auto i = start; i < end; i += step){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, typename F> decltype(auto) For(JobGroup& jobgroup, iteratorType start, iteratorType end, iteratorType step, F const& ToDo) {
			if (end <= start) return;
			auto& group = jobgroup.GetTaskGroup();

			group.Dispatch((end - start) / step, [=](impl::JobArgs args) {
				ToDo(start + args.jobIndex * step);
			});			
		};

		/* parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(JobGroup& jobgroup, containerType& container, F const& ToDo) {
			int n = container.size();
			if (n <= 0) return;
			auto& group = jobgroup.GetTaskGroup();

			group.Dispatch(n, [&container, to_do = ToDo](impl::JobArgs args) {
				auto iter = container.begin();
				std::advance(iter, args.jobIndex);
				to_do(*iter);
			});			
		};

		/* parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(JobGroup& jobgroup, containerType const& container, F const& ToDo) {
			int n = container.size();
			if (n <= 0) return;
			auto& group = jobgroup.GetTaskGroup();

			group.Dispatch(n, [&container, to_do = ToDo](impl::JobArgs args) {
				auto iter = container.cbegin();
				std::advance(iter, args.jobIndex);
				to_do(*iter);
			});
		};

		/* parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end. */
		template<typename iteratorType, typename F> decltype(auto) For(iteratorType start, iteratorType end, F const& ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;

			if constexpr (retNo) {
				if (end <= start) return;

				impl::TaskGroup group;

				group.Dispatch(end - start, [&](impl::JobArgs args) {
					ToDo(start + args.jobIndex);
				});

				group.Wait();
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

				if (end <= start) return std::vector< returnT >();

				std::vector< returnT > out(end - start, returnT());

				impl::TaskGroup group;

				group.Dispatch(end - start, [&](impl::JobArgs args) {
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

				group.Dispatch((end - start) / step, [&](impl::JobArgs args) {
					ToDo(start + args.jobIndex * step);
				});

				group.Wait();
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

				if (end <= start) return std::vector< returnT >();

				std::vector< returnT > out((end - start) / step, returnT());

				impl::TaskGroup group;

				group.Dispatch((end - start) / step, [&](impl::JobArgs args) {
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

			int n = container.size();
			if constexpr (retNo) {
				if (n <= 0) return;

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
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

				if (n <= 0) return std::vector< returnT >();

				std::vector<containerType::iterator> iterators(n, container.begin());
				std::vector< returnT > out(n, returnT());

				impl::TaskGroup group;

				group.Dispatch(n, [&](impl::JobArgs args) {
					std::advance(iterators[static_cast<int>(args.jobIndex)], args.jobIndex);
					out[static_cast<int>(args.jobIndex)] = ToDo(*iterators[static_cast<int>(args.jobIndex)]);
				});

				group.Wait();

				return out;
			}
		};

		/* parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. */
		template<typename containerType, typename F> decltype(auto) ForEach(containerType const& container, F const& ToDo) {
			constexpr bool retNo = std::is_same<typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type, void>::value;

			int n = container.size();
			if constexpr (retNo) {
				if (n <= 0) return;

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
			}
			else {
				using returnT = typename fibers::utilities::function_traits<decltype(std::function(ToDo))>::result_type;

				if (n <= 0) return std::vector< returnT >();

				std::vector<containerType::const_iterator> iterators(n, container.begin());

				std::vector< returnT > out(n, returnT());

				impl::TaskGroup group;

				group.Dispatch(n, [&](impl::JobArgs args) {
					std::advance(iterators[args.jobIndex], args.jobIndex);
					out[args.jobIndex] = ToDo(*iterators[args.jobIndex]);
				});

				group.Wait();

				return out;
			}
		};

		/* wrapper for std::find_if */
		template<typename containerType, typename F> decltype(auto) Find(containerType& container, F const& ToDo) {
#if 1
			return std::find_if(container.begin(), container.end(), [&](auto& x) ->bool { return ToDo(x); }); // std::execution::par, 
#else
			uint32_t n{ static_cast<uint32_t>(container.size()) };
			auto out{ container.end() };
			impl::TaskGroup group;

			if (n > 0) {
				std::vector<containerType::iterator> iterators(n, container.begin());
				group.Dispatch(n, [&](impl::JobArgs args) {
					std::advance(iterators[static_cast<int>(args.jobIndex)], args.jobIndex);
					if (ToDo(*iterators[static_cast<int>(args.jobIndex)])) {
						if (group.TryEarlyExit()) {
							out = iterators[static_cast<int>(args.jobIndex)];
						}
					}
				});
				group.Wait();
			}
			return out;
#endif
		};

		/* wrapper for std::find_if */
		template<typename containerType, typename F> decltype(auto) Find(containerType const& container, F const& ToDo) {
#if 1
			return std::find_if(container.cbegin(), container.cend(), [&](auto const& x) ->bool { return ToDo(x); }); // std::execution::par, 
#else
			uint32_t n{ static_cast<uint32_t>(container.size()) };
			auto out{ container.cend() };
			impl::TaskGroup group;

			if (n > 0) {
				std::vector<containerType::const_iterator> iterators(n, container.cbegin());
				group.Dispatch(n, [&](impl::JobArgs args) {
					std::advance(iterators[static_cast<int>(args.jobIndex)], args.jobIndex);
					if (ToDo(*iterators[static_cast<int>(args.jobIndex)])) {
						if (group.TryEarlyExit()) {
							out = iterators[static_cast<int>(args.jobIndex)];
						}
					}					
				});
				group.Wait();
			}
			return out;
#endif
		};

		/*
		outputType x;
		for (auto& v : resultList){ x += v; }
		return x;
		*/
		template<typename outputType>
		decltype(auto) Accumulate(std::vector<outputType> const& resultList) {
			outputType out{ 0 };
			for (auto& v : resultList) {
				if (v) {
					out += *v;
				}
			}
			return out;
		};

		/* Generic form of a future<T>, which can be used to wait on and get the results of any job. */
		class promise {
		protected:
			std::shared_ptr< synchronization::atomic_ptr<JobGroup> > shared_state;
			std::shared_ptr< synchronization::atomic_ptr<Any> > result;

		public:
			promise() : shared_state(nullptr), result(nullptr) {};
			promise(Job const& job) :
				shared_state(std::shared_ptr<synchronization::atomic_ptr<JobGroup>>(new synchronization::atomic_ptr<JobGroup>(new JobGroup(job)), [](synchronization::atomic_ptr<JobGroup>* anyP) { if (anyP) { auto* p = anyP->Set(nullptr); if (p) { delete p; } delete anyP; } })),
				result(std::shared_ptr<synchronization::atomic_ptr<Any>>(new synchronization::atomic_ptr<Any>(), [](synchronization::atomic_ptr<Any>* anyP) { if (anyP) { auto* p = anyP->Set(nullptr); if (p) { delete p; } delete anyP; } })) {};
			promise(promise const&) = default;
			promise(promise&&) = default;
			promise& operator=(promise const&) = default;
			promise& operator=(promise&&) = default;
			virtual ~promise() {};

			bool valid() const noexcept {
				return (bool)shared_state;
			};
			void wait() {
				if (shared_state) {
					auto p = shared_state->Set(nullptr);
					if (p) {
						auto list = p->Wait_Get();
						if (list.size() > 0) {
							Any* p2 = result->Set(new Any(list[0]));
							if (p2) {
								delete p2;
							}
						}						
						delete p;
					}
				}
				while (result && !result) {
					std::this_thread::yield();
				}
			};
			Any get_any() const noexcept {
				if (result) {
					Any* p = result->Get();
					if (p) {
						return Any(*p);
					}
				}
				return Any();
			};
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

#include <cstdarg>
#include <iostream>
#include <array>
#include <cstring>
#include <string>
#include <sstream>
#include <mutex>
#include <map>
#include <type_traits>

#define DefineCategoryType(type, a, b, c, d, e) class type : public value { public: \
	type() noexcept : value(0.0, Unit_ID(a, b, c, d, e, false, "", 1.0)) {}; \
	type(double V) noexcept : value(V, Unit_ID(a, b, c, d, e, false, "", 1.0)) {}; \
	type(double V, const char* abbreviation) noexcept : value(V, Unit_ID(a, b, c, d, e, false, abbreviation, 1.0)) {}; \
	type(double V, const char* abbreviation, double ratio) noexcept : value(V, Unit_ID(a, b, c, d, e, false, abbreviation, ratio)) {}; \
    type(value const& V) noexcept = delete; \
    virtual ~type() {}; \
	friend inline std::ostream& operator<<(std::ostream& os, type const& obj) { os << obj.ToString(); return os; }; \
	friend inline std::stringstream& operator>>(std::stringstream& os, type& obj) { double v = 0; os >> v; obj = v; return os; }; \
	constexpr static double A() noexcept { return a; } \
	constexpr static double B() noexcept { return b; } \
	constexpr static double C() noexcept { return c; } \
	constexpr static double D() noexcept { return d; } \
	constexpr static double E() noexcept { return e; } \
};
#define DefineCategoryStd(type, a, b, c, d, e) namespace std { template<> class numeric_limits<Units::type> { public: \
	static constexpr double min() { return std::numeric_limits<double>::min(); } \
	static constexpr double max() { return std::numeric_limits<double>::max(); } \
	static constexpr double lowest() { return std::numeric_limits<double>::lowest(); } \
	static constexpr bool is_integer = std::numeric_limits<double>::is_integer; \
	static constexpr bool is_signed = std::numeric_limits<double>::is_signed; }; \
};
#define DerivedUnitType(type, category, abbreviation, ratio) class type final : public category  { public: \
	constexpr static double conversion() noexcept { return ratio; }; \
	constexpr static const char* specialized_abbreviation() noexcept { return #abbreviation; }; \
	constexpr static const char* specialized_name() noexcept { return #type; }; \
	type() noexcept : category(0.0, specialized_abbreviation(), ratio) {}; \
	type(double V) noexcept : category(V, specialized_abbreviation(), ratio) {}; \
	type(value const& V) : category(0.0, specialized_abbreviation(), ratio) { \
		if (this->unit_m.IsSameCategory(V.unit_m)) { this->value_m = V.value_m; } \
		else if (value::is_scalar(V)) { this->value_m = V() * ratio; } \
		else if (value::is_scalar(*this)) { this->unit_m = V.unit_m; this->value_m = V.value_m; } \
		else { throw(std::runtime_error(Units::Unit_ID::printf("Assignment(const&) failed due to incompatible non-scalar units: '%s' and '%s'.", specialized_abbreviation(), V.Abbreviation().c_str()))); } \
    }; \
    virtual bool IsStaticType() const { return true; }; \
    virtual ~type() {}; \
	friend inline std::ostream& operator<<(std::ostream& os, type const& obj) { os << obj.ToString(); return os; }; \
	friend inline std::stringstream& operator>>(std::stringstream& os, type& obj) { double v = 0; os >> v; obj = v; return os; }; \
};
#define CalculateMetricPrefixV(metric) ((double)std::metric::num / (double)std::metric::den)
#define DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) DerivedUnitType(prefix ## type, category, prefix_abbrev ## abbreviation, ratio * CalculateMetricPrefixV(prefix))
#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio)\
	DerivedUnitType(type, category, abbreviation, ratio);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, pico, p);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, nano, n);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, micro, u);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, milli, m);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, centi, c);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deci, d);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, deca, da);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, hecto, h);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, kilo, k);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, mega, M);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, giga, G);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, tera, T);\
	DerivedUnitTypeWithMetricPrefix(type, category, abbreviation, ratio, peta, P)
#define DerivedUnitStd(type, category, abbreviation, ratio) namespace std { template<> class numeric_limits<Units::type> { public: \
	static constexpr double min() { return std::numeric_limits<double>::min(); } \
	static constexpr double max() { return std::numeric_limits<double>::max(); } \
	static constexpr double lowest() { return std::numeric_limits<double>::lowest(); } \
	static constexpr bool is_integer = std::numeric_limits<double>::is_integer; \
	static constexpr bool is_signed = std::numeric_limits<double>::is_signed; }; \
}; /* Unit Litersl (e.g. 1_ft, 1_gpm, etc.) */ namespace literals { \
	__forceinline auto operator""_ ## abbreviation (long double d) { return Units::type(static_cast<double>(d)); } \
	__forceinline auto operator""_ ## abbreviation (unsigned long long d) { return Units::type(static_cast<double>(d)); }\
};

#define DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, prefix, prefix_abbrev) DerivedUnitStd(prefix ## type, category, prefix_abbrev ## abbreviation, ratio * CalculateMetricPrefixV(prefix))
#define DerivedUnitStdWithMetricPrefixes(type, category, abbreviation, ratio)\
	DerivedUnitStd(type, category, abbreviation, ratio);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, femto, f); \
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, pico, p);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, nano, n);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, micro, u);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, milli, m);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, centi, c);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, deci, d);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, deca, da);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, hecto, h);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, kilo, k);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, mega, M);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, giga, G);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, tera, T);\
	DerivedUnitStdWithMetricPrefix(type, category, abbreviation, ratio, peta, P)

#define FunctionNameTest(Name) \
        template<typename T, typename = void> struct has_ ## Name : std::false_type {}; \
		template<typename T> struct has_ ## Name<T, void_t<decltype(std::declval<T>(). ## Name() == true)>> : std::true_type {}

namespace unitTypes {
	BETTER_ENUM(units_type, uint8_t, METERS, KILOGRAMS, SECONDS, AMPERES, DOLLAR);
};
class Units {
public:
	template <typename Derived> static constexpr __forceinline double Conversion(double X) { return Derived::conversion() * X; };
	static constexpr __forceinline double SQUARED(double X) { return X * X; };
	static constexpr __forceinline double CUBED(double X) { return X * X * X; };

	static constexpr size_t HashUnits(double a, double b, double c, double d, double e) noexcept {
		constexpr double OFFSET = 1000;
		constexpr size_t A = 54059; /* a prime */
		constexpr double B = 76963; /* another prime */
		constexpr size_t C = 86969; /* yet another prime */
		constexpr size_t FIRSTH = 37; /* also prime */

		size_t h = FIRSTH;
		h = (h * A) ^ (size_t)((a + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((b + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((c + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((d + OFFSET) * B * 100.0);
		h = (h * A) ^ (size_t)((e + OFFSET) * B * 100.0);

		decltype(auto) result = h % C;
		return result;
	};
	static constexpr size_t HashUnitAndRatio(double unitHash, double ratio) noexcept {
		constexpr double OFFSETA = 10000;
		constexpr double OFFSETB = 1000;
		constexpr size_t A = 54059; /* a prime */
		constexpr double B = 76963; /* another prime */
		constexpr size_t C = 86969; /* yet another prime */
		constexpr size_t FIRSTH = 37; /* also prime */

		size_t h = FIRSTH;
		h = (h * A) ^ (size_t)((unitHash + OFFSETA) * B);
		h = (h * A) ^ (size_t)((ratio + OFFSETB) * B * 100000.0); // 10000000.0

		decltype(auto) result = h % C;
		return result;
	};

	static int INT32_SIGNBITNOTSET(int i) {
		int	r = ((~static_cast<const unsigned int>(i)) >> 31);
		assert(r == 0 || r == 1);
		return r;
	};
	static long long	StrCmp(const char* s1, const char* s2) {
		long long c1, c2, d;
		if (!s1 || !s2) return 0;
		do {
			c1 = *s1++;
			c2 = *s2++;

			d = c1 - c2;
			if (d) {
				return (INT32_SIGNBITNOTSET(d) << 1) - 1;
			}
		} while (c1);

		return 0;		// strings are equal
	};

	class Unit_ID {
		static constexpr size_t NumUnits = unitTypes::units_type::_size_constant;
	private:
		constexpr double abs(double x) { return x > 0 ? x : -x; };
	public:
		constexpr Unit_ID() noexcept :
			unitType_m{ 0.0, 0.0, 0.0, 0.0, 0.0 }, isScalar_m(true), isSI_m(false), abbreviation_m(const_cast<char*>("")), ratio_m(1.)
		{};
		constexpr Unit_ID(double a, double b, double c, double d, double e, double isScalar_p, const char* abbreviation_p, double ratio_p) noexcept :
			unitType_m{ a, b, c, d, e }, isScalar_m(isScalar_p), isSI_m((abs(a) + abs(b) + abs(c) + abs(d) + abs(e)) == 1.0 && abs(ratio_p) == 1.0), abbreviation_m(const_cast<char*>(abbreviation_p)), ratio_m(ratio_p)
		{};
		Unit_ID(Unit_ID const& other) noexcept :
			unitType_m{ other.unitType_m }, isScalar_m{ other.isScalar_m }, isSI_m{ other.isSI_m }, abbreviation_m{other.abbreviation_m}, ratio_m{ other.ratio_m }
		{};
		Unit_ID(Unit_ID && other) noexcept :
			unitType_m{ std::move(other.unitType_m) }, isScalar_m{ std::move(other.isScalar_m) }, isSI_m{ std::move(other.isSI_m) }, abbreviation_m{ std::move(other.abbreviation_m) }, ratio_m{ std::move(other.ratio_m) }
		{};
		Unit_ID& operator=(Unit_ID const& other) noexcept {
			unitType_m = other.unitType_m;
			isScalar_m = other.isScalar_m;
			isSI_m = other.isSI_m;
			abbreviation_m = other.abbreviation_m;
			ratio_m = other.ratio_m;

			return *this;
		};
		~Unit_ID() {};

	public:

		bool IsSameCategory(Unit_ID const& other) const noexcept {
			if (isScalar_m && other.isScalar_m) return true;
			return std::memcmp(&unitType_m, &other.unitType_m, sizeof(unitType_m)) == 0;
		};
		bool IsSameUnit(Unit_ID const& other) const noexcept {
			return IsSameCategory(other) && (ratio_m == other.ratio_m);
		};
		decltype(auto) HashCategory() const noexcept {
			return Units::HashUnits(unitType_m[0], unitType_m[1], unitType_m[2], unitType_m[3], unitType_m[4]);
		};
		const char* LookupAbbreviation(bool isStatic) const noexcept {
			if (!isStatic && !isScalar_m) {
				abbreviation_m.Set(const_cast<char*>(Units::UnitsDetail::lookup_abbreviation(HashCategory(), ratio_m)));
				if (StrCmp(abbreviation_m, "") == 0) {
					ratio_m = 1;
				}
			}
			return abbreviation_m;
		};
		const char* LookupTypeName() const noexcept {
			return Units::UnitsDetail::lookup_typename(HashCategory(), ratio_m);
		};

	private:
		static void AddToDelimiter(std::string& obj, std::string const& toAdd, std::string const& delim) {
			if (obj.length() == 0) {
				obj += toAdd;
			}
			else {
				obj += delim;
				obj += toAdd;
			}
		};
		static size_t		vsnPrintf(char* dest, size_t size, const char* fmt, va_list argptr) {
			size_t ret;
#undef _vsnprintf
			ret = ::_vsnprintf(dest, size - 1, fmt, argptr);
			dest[size - 1] = '\0';
			if (ret < 0 || ret >= size)  ret = -1;
			return ret;
		};

	public:
		static std::string	printf(const char* fmt, ...) {
			va_list argptr;

			decltype(auto) buffer = new char[128000];
			buffer[128000 - 1] = '\0';

			va_start(argptr, fmt);
			vsnPrintf(buffer, 128000 - 1 /*sizeof(buffer) - 1*/, fmt, argptr);
			va_end(argptr);
			buffer[128000 /*sizeof(buffer)*/ - 1] = '\0';

			std::string out(buffer);

			delete[] buffer;
			return out;
		};
		static bool IsInteger(double value) {
			double intpart;
			return modf(value, &intpart) == 0.0;
		};

	public:
		std::string CreateAbbreviation(bool isStatic) const noexcept {
			std::string out = LookupAbbreviation(isStatic);
			if (!isScalar_m && out.empty()) {
				std::array< const char*, NumUnits> unitBases{ "m", "kg", "s", "A", "$" };

				bool anyNegatives = false;
				for (int i = NumUnits - 1; i >= 0; i--) {
					decltype(auto) unitBase = unitBases[i];
					decltype(auto) v = unitType_m[i];

					if (v > 0) {
						if (v == 1)
							AddToDelimiter(out, unitBase, " ");
						else {
							std::string Num;
							if (IsInteger(v)) {
								Num = std::to_string((int)v);
							}
							else {
								Num = std::to_string((float)v);
							}
							AddToDelimiter(out, printf("%s^%s", unitBase, Num.c_str()), " ");
						}
					}
					else if (v < 0) {
						anyNegatives = true;
					}
				}
				if (anyNegatives) {
					out += " /";
					for (int i = NumUnits - 1; i >= 0; i--) {
						decltype(auto) unitBase = unitBases[i];
						decltype(auto) v = unitType_m[i];

						if (v < 0) {
							if (v == -1)
								AddToDelimiter(out, unitBase, " ");
							else {
								std::string Num;
								if (IsInteger(v)) {
									Num = std::to_string((int)(-1.0 * v));
								}
								else {
									Num = std::to_string((float)(-1.0 * v));
								}
								AddToDelimiter(out, printf("%s^%s", unitBase, Num.c_str()), " ");
							}
						}
					}
				}
			}
			return out;
		};

	public:
		std::array< fibers::synchronization::atomic_number<double>, NumUnits> unitType_m;
		fibers::synchronization::atomic_number<bool> isScalar_m;
		fibers::synchronization::atomic_number<bool> isSI_m;
		mutable fibers::synchronization::atomic_ptr<char> abbreviation_m;
		mutable fibers::synchronization::atomic_number<double> ratio_m;
	};

	class value {
	public:
		Units::Unit_ID unit_m;
		fibers::synchronization::atomic_number<double> value_m;

	protected:
		double conversion() const noexcept { return unit_m.ratio_m; };

	public: // constructors
		value() noexcept : unit_m(), value_m(0.0) {};
		explicit value(Units::Unit_ID const& unit_p) noexcept : unit_m(unit_p), value_m(0.0) {};
		explicit value(double V, Units::Unit_ID const& unit_p) noexcept : unit_m(unit_p), value_m(V* conversion()) {};
		value(value&& V) noexcept : unit_m(std::move(V.unit_m)), value_m(std::move(V.value_m)) {};
		value(value const& V) noexcept : unit_m(V.unit_m), value_m(V.value_m) {};
		value(double V) noexcept : unit_m(), value_m(V* conversion()) {};

		virtual bool IsStaticType() const { return false; };
		virtual ~value() = default;

	private:
		double GetVisibleValue() const noexcept {
			if (unit_m.isSI_m && unit_m.ratio_m == 1.0) {
				return value_m.GetValue();
			}
			else {
				unit_m.LookupAbbreviation(IsStaticType());
				return value_m.GetValue() / conversion();
			}
		};

	public: // value operator
		explicit operator double() const noexcept { return GetVisibleValue(); };
		double operator()() const noexcept { return GetVisibleValue(); };

	public: // Functions
		const char* UnitName() const noexcept {
			unit_m.LookupAbbreviation(IsStaticType());
			return unit_m.LookupTypeName();
		};
		bool AreConvertableTypes(value const& V) const {
			return value::NormalArithmeticOkay(*this, V);
		};
		void Clear() { unit_m = Units::Unit_ID(); value_m = 0.0; };

	public:
		std::string Abbreviation() const noexcept {
			return unit_m.CreateAbbreviation(IsStaticType());
		};
		std::string ToString() const {
			std::string abbreviation{ Abbreviation() };
			if (abbreviation.length() > 0) return GetValueStr(*this) + " " + abbreviation;
			else return GetValueStr(*this);
		};

	public: // Streaming functions (should be specialized per type)
		friend inline std::ostream& operator<<(std::ostream& os, value const& obj) { os << obj.ToString(); return os; };
		friend inline std::stringstream& operator>>(std::stringstream& os, value& obj) { double v = 0; os >> v; obj = v; return os; };
		static bool IdenticalUnits(value const& LHS, value const& RHS) noexcept { return LHS.unit_m.IsSameCategory(RHS.unit_m); };
		static bool is_scalar(value const& V) noexcept { return V.unit_m.isScalar_m; };

	private:
		static std::string GetValueStr(value const& V) noexcept {
			float v = V();
			if (std::fmod(v, 1.0) == 0.0) { // integer?
				return Units::Unit_ID::printf("%i", (int)v);
			}
			else { // floating-point
				return Units::Unit_ID::printf("%.4f", (float)v);
			}
		};
		static bool NormalArithmeticOkay(value const& LHS, value const& RHS) noexcept {
			if (is_scalar(LHS) || is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		static bool UnaryArithmeticOkay(value const& LHS, value const& RHS) noexcept {
			if (is_scalar(RHS)) return true;
			if (IdenticalUnits(LHS, RHS)) return true;
			return false;
		};
		static void HandleNormalArithmetic(value const& LHS, value const& RHS) {
			if (NormalArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(Units::Unit_ID::printf("Normal, dynamic arithmetic failed due to incompatible non-scalar value: '%s' and '%s'", LHS.Abbreviation().c_str(), RHS.Abbreviation().c_str())));
		};
		static void HandleUnaryArithmetic(value const& LHS, value const& RHS) {
			if (UnaryArithmeticOkay(LHS, RHS)) return;
			throw(std::runtime_error(Units::Unit_ID::printf("Unary (in-place or self-modifying) arithmetic failed due to incompatible value: '%s' and '%s'", LHS.Abbreviation().c_str(), RHS.Abbreviation().c_str())));
		};
		static void HandleNotScalar(value const& V) {
			if (is_scalar(V)) return;
			throw(std::runtime_error(Units::Unit_ID::printf("Type must be scalar (was '%s').", V.Abbreviation().c_str())));
		};
		/* Used for multiplication or division operations */
		template <bool multiplication = true> value& CompoundUnits(value const& V) noexcept {
			if (is_scalar(*this) && is_scalar(V)) return *this;
			// if V is a scaler, then there is no point changing this unit type
			if (is_scalar(V)) return *this;
			// V is not a scaler, but I could be.
			bool allZero = true;
			for (int i = unit_m.unitType_m.size() - 1; i >= 0; i--) {
				if constexpr (multiplication) {
					unit_m.unitType_m[i] += V.unit_m.unitType_m[i];
				}
				else {
					unit_m.unitType_m[i] -= V.unit_m.unitType_m[i];
				}
				allZero = allZero && unit_m.unitType_m[i] == 0;
			}
			if (allZero) { unit_m.isScalar_m = true; }
			else { unit_m.isScalar_m = false; }

			// now that we have modified the value, the conversion ratio makes no sense anymore and must be reset.
			if constexpr (multiplication) {
				unit_m.ratio_m *= V.unit_m.ratio_m;
			}
			else {
				unit_m.ratio_m /= V.unit_m.ratio_m;
			}
			return *this;
		};
		/* Used for exponential operations */
		value& MultiplyUnits(double const& V) noexcept {
			if (is_scalar(*this) || V == 1.0) return *this;
			for (int i = unit_m.unitType_m.size() - 1; i >= 0; i--) unit_m.unitType_m[i] *= V;
			if (V == 0) unit_m.isScalar_m = true;

			// now that we have modified the value, the conversion ratio makes no sense anymore and must be reset. 
			unit_m.ratio_m = std::pow(unit_m.ratio_m, V);

			return *this;
		};

	public: // = Operators
		value& operator=(value const& V) {
			if (this->unit_m.IsSameCategory(V.unit_m)) { // same category, but perhaps different conversion factor. That's OK. 
				value_m = V.value_m;
			}
			else if (is_scalar(V)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				value_m = V.GetVisibleValue() * conversion();
			}
			else if (is_scalar(*this)) { // I am a scaler but the incoming unit is not. Simply copy the incoming unit entirely.
				unit_m = V.unit_m;
				value_m = V.value_m;
			}
			else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
				throw(std::runtime_error(Units::Unit_ID::printf("Assignment(const&) failed due to incompatible non-scalar value: '%s' and '%s'.", this->Abbreviation().c_str(), V.Abbreviation().c_str())));
			}
			return *this;
		};

	public: // Comparison operators
		friend bool operator==(value const& A, value const& V) noexcept { if (!NormalArithmeticOkay(A, V)) return false; if (is_scalar(V) == is_scalar(A)) { return A.value_m == V.value_m; } else if (is_scalar(V)) { value W = A; W = V; return A.value_m == W.value_m; } else { value W = V; W = A; return W.value_m == V.value_m; } };
		friend bool operator!=(value const& A, value const& V) noexcept { return !(operator==(A, V)); };
		friend bool operator<(value const& A, value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m < V.value_m; } else if (is_scalar(V)) { value W = A; W = V; return A.value_m < W.value_m; } else { value W = V; W = A; return W.value_m < V.value_m; } };
		friend bool operator<=(value const& A, value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m <= V.value_m; } else if (is_scalar(V)) { value W = A; W = V; return A.value_m <= W.value_m; } else { value W = V; W = A; return W.value_m <= V.value_m; } };
		friend bool operator>(value const& A, value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m > V.value_m; } else if (is_scalar(V)) { value W = A; W = V; return A.value_m > W.value_m; } else { value W = V; W = A; return W.value_m > V.value_m; } };
		friend bool operator>=(value const& A, value const& V) { HandleNormalArithmetic(A, V); if (is_scalar(V) == is_scalar(A)) { return A.value_m >= V.value_m; } else if (is_scalar(V)) { value W = A; W = V; return A.value_m >= W.value_m; } else { value W = V; W = A; return W.value_m >= V.value_m; } };

	public: // Unary operators
		value& operator++() { value_m = (GetVisibleValue() + 1) * conversion(); return *this; };
		value& operator--() { value_m = (GetVisibleValue() - 1) * conversion(); return *this; };
		value operator++(int) { value out = *this; value_m = (GetVisibleValue() + 1) * conversion(); return out; };
		value operator--(int) { value out = *this; value_m = (GetVisibleValue() - 1) * conversion(); return out; };

	public: // + and - Operators
		static value Add(value const& a, value const& b) {
			HandleNormalArithmetic(a, b);
			value out1 = a;
			value out2 = a; out2 = b;
			out1.value_m += out2.value_m;
			return out1;
		};
		static value Sub(value const& a, value const& b) {
			HandleNormalArithmetic(a, b);
			value out1 = a;
			value out2 = a; out2 = b;
			out1.value_m -= out2.value_m;
			return out1;
		};

		friend value operator+(value const& A, value const& V) { return Add(A, V); };
		friend value operator-(value const& A, value const& V) { return Sub(A, V); };
		value& operator+=(value const& V) {
			if (this->unit_m.IsSameCategory(V.unit_m)) { // same category, but perhaps different conversion factor. That's OK. 
				this->value_m += V.value_m;
			}
			else if (is_scalar(V)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				this->value_m += V.GetVisibleValue() * conversion();
			}
			else if (is_scalar(*this)) { // I am a scaler but the incoming unit is not. Copy the incoming value's visible value.
				this->value_m += V.GetVisibleValue();
			}
			else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
				HandleUnaryArithmetic(*this, V);
				value temp = *this;
				temp = V;
				this->value_m += temp.value_m;
			}

			return *this;
		};
		value& operator-=(value const& V) {
			if (this->unit_m.IsSameCategory(V.unit_m)) { // same category, but perhaps different conversion factor. That's OK. 
				this->value_m -= V.value_m;
			}
			else if (is_scalar(V)) { // incoming is a scaler and this unit is not. Use this unit's conversion factor.
				this->value_m -= V.GetVisibleValue() * conversion();
			}
			else if (is_scalar(*this)) { // I am a scaler but the incoming unit is not. Copy the incoming value's visible value.
				this->value_m -= V.GetVisibleValue();
			}
			else { // incoming unit AND this unit are different non-scalers of different categories. No exchange is reasonable. 
				HandleUnaryArithmetic(*this, V);
				value temp = *this;
				temp = V;
				this->value_m -= temp.value_m;
			}

			return *this;
		};

	public: // * and / Operators
		static value Multiply(value const& A, value const& V) {
			value out = A;
			out.CompoundUnits<true>(V);
			out.value_m *= V.value_m;
			return out;
		};
		static value Divide(value const& A, value const& V) {
			value out = A;
			out.CompoundUnits<false>(V);
			out.value_m /= V.value_m;
			return out;
		};

		friend value operator*(value const& A, value const& V) { return Multiply(A,V); };
		friend value operator/(value const& A, value const& V) { return Divide(A, V); };
		value& operator*=(value const& V) {
			HandleNotScalar(V);
			value_m *= V.value_m;
			return *this;
		};
		value& operator/=(value const& V) {
			HandleNotScalar(V);
			value_m /= V.value_m;
			return *this;
		};

	public: // pow and sqrt Operators
		value pow(value const& V) const {
			HandleNotScalar(V);

			value out = *this;
			out.MultiplyUnits(V.value_m);

			// i.e. (10 (ft)) ^ (3) -> (1000 (cu_ft)) * (1 / 35.3147 (cu_m/cu_ft)) -> 28.3168 cu_m in SI value
			out.value_m = std::pow(this->GetVisibleValue(), V.value_m) * out.conversion(); // save in SI value

			return out;
		};
		value& pow_value(value const& V) { HandleNotScalar(V); value_m = std::pow(GetVisibleValue(), V.GetVisibleValue()) * conversion(); return *this; };
		value sqrt() const { value out = *this; out.MultiplyUnits(0.5); out.value_m = std::sqrt(out.value_m); return out; };
		value floor() const { value out = *this; out.value_m = std::floor(GetVisibleValue()) * conversion(); return out; };
		value ceiling() const { value out = *this; out.value_m = std::ceil(GetVisibleValue()) * conversion(); return out; };
	};
	using scalar = value;

	class traits {
		/* test if two unit types are convertable */
		template<class U1, class U2> struct is_convertible_unit_t {
			static constexpr const std::intmax_t value = HashUnits(U1::A(), U1::B(), U1::C(), U1::D(), U1::E()) == HashUnits(U2::A(), U2::B(), U2::C(), U2::D(), U2::E());
		};

		template<class U1> struct is_unit_t {
			static constexpr const std::intmax_t value = std::is_base_of<Units::value, U1>::value;
		};
	};

private:
	// Base classes
	DefineCategoryType(length, 1, 0, 0, 0, 0);
	DefineCategoryType(mass, 0, 1, 0, 0, 0);
	DefineCategoryType(time, 0, 0, 1, 0, 0);
	DefineCategoryType(current, 0, 0, 0, 1, 0);
	DefineCategoryType(dollar, 0, 0, 0, 0, 1);
	// Derived classes
	DefineCategoryType(frequency, 0, 0, -1, 0, 0);
	DefineCategoryType(velocity, 1, 0, -1, 0, 0);
	DefineCategoryType(acceleration, 1, 0, -2, 0, 0);
	DefineCategoryType(force, 1, 1, -2, 0, 0);
	DefineCategoryType(pressure, -1, 1, -2, 0, 0);
	DefineCategoryType(charge, 0, 0, 1, 1, 0);
	DefineCategoryType(power, 2, 1, -3, 0, 0);
	DefineCategoryType(energy, 2, 1, -2, 0, 0);
	DefineCategoryType(voltage, 2, 1, -3, -1, 0);
	DefineCategoryType(impedance, 2, 1, -3, -2, 0);
	DefineCategoryType(conductance, -2, -1, 3, 2, 0);
	DefineCategoryType(area, 2, 0, 0, 0, 0);
	DefineCategoryType(volume, 3, 0, 0, 0, 0);
	DefineCategoryType(fillrate, 0, 1, -1, 0, 0);
	DefineCategoryType(flowrate, 3, 0, -1, 0, 0);
	DefineCategoryType(density, -3, 1, 0, 0, 0);
	DefineCategoryType(energy_cost_rate, -2, -1, 2, 0, 1);
	DefineCategoryType(power_cost_rate, -2, -1, 3, 0, 1);
	DefineCategoryType(volume_cost_rate, -3, 0, 0, 0, 1);
	DefineCategoryType(energy_intensity, -1, 1, -2, 0, 1);
	DefineCategoryType(length_cost_rate, -1, 0, 0, 0, 1);
	DefineCategoryType(mass_cost_rate, 0, -1, 0, 0, 1);
	DefineCategoryType(emission_rate, -2, 0, 2, 0, 1);
	DefineCategoryType(time_rate, 0, 0, -1, 0, 1);

public:
	/* LENGTH DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0);
	DerivedUnitType(foot, length, ft, 381.0 / 1250.0);
	DerivedUnitType(inch, length, in, Conversion<foot>(1.0 / 12.0));
	DerivedUnitType(mile, length, mi, Conversion<foot>(5280.0 / 1.0));
	DerivedUnitType(nauticalMile, length, nmi, Conversion<meter>(1852.0));
	DerivedUnitType(astronicalUnit, length, au, Conversion<meter>(149597870700.0));
	DerivedUnitType(yard, length, yd, Conversion<foot>(3.0));

	/* MASS DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0);
	DerivedUnitType(metric_ton, mass, t, Conversion<kilogram>(1000.0));
	DerivedUnitType(pound, mass, lb, Conversion<kilogram>(45359237.0 / 100000000.0));
	DerivedUnitType(long_ton, mass, ln_t, Conversion < pound>(2240.0));
	DerivedUnitType(short_ton, mass, sh_t, Conversion < pound>(2000.0));
	DerivedUnitType(stone, mass, st, Conversion < pound>(14.0));
	DerivedUnitType(ounce, mass, oz, Conversion < pound>(1.0 / 16.0));
	DerivedUnitType(carat, mass, ct, Conversion < milligram>(200.0));
	DerivedUnitType(slug, mass, slug, Conversion<kilogram>(145939029.0 / 10000000.0));

	/* TIME DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(second, time, s, 1.0);
	DerivedUnitType(minute, time, min, Conversion<second>(60.0));
	DerivedUnitType(hour, time, hr, Conversion<minute>(60.0));
	DerivedUnitType(day, time, d, Conversion<hour>(24.0));
	DerivedUnitType(week, time, wk, Conversion<day>(7.0));
	DerivedUnitType(year, time, yr, Conversion<day>(365));
	DerivedUnitType(month, time, mnth, Conversion<year>(1.0 / 12.0));
	DerivedUnitType(julian_year, time, a_j, Conversion<second>(31557600.0));
	DerivedUnitType(gregorian_year, time, a_g, Conversion<second>(31556952.0));

	/* CURRENT DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(ampere, current, A, 1.0);

	/* DOLLAR DERIVATIONS */
	DerivedUnitType(Dollar, dollar, USD, 1.0);
	DerivedUnitType(MillionDollar, dollar, MUSD, Conversion<Dollar>(1000000.0));

	/* FREQUENCY DERIVATIONS */
	DerivedUnitTypeWithMetricPrefixes(hertz, frequency, Hz, 1.0);

	/* VELOCITY DERIVATIONS */
	DerivedUnitType(meters_per_second, velocity, mps, Conversion<meter>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(feet_per_second, velocity, fps, Conversion<foot>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(feet_per_minute, velocity, fpm, Conversion<foot>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(feet_per_hour, velocity, fph, Conversion<foot>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(miles_per_hour, velocity, mph, Conversion<mile>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(kilometers_per_hour, velocity, kph, Conversion<kilometer>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(knot, velocity, kts, Conversion<nauticalMile>(1.0) / Conversion<hour>(1.0));

	/* ACCELERATION DERIVATIONS */
	DerivedUnitType(meters_per_second_squared, acceleration, mps_sq, Conversion<meter>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
	DerivedUnitType(feet_per_second_squared, acceleration, fps_sq, Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
	DerivedUnitType(standard_gravity, acceleration, SG, Conversion<meters_per_second_squared>(980665.0 / 100000.0));

	// FORCE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(newton, force, N, Conversion<kilogram>(1.0)* Conversion<meters_per_second_squared>(1.0));
	DerivedUnitTypeWithMetricPrefixes(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0));
	DerivedUnitType(dyne, force, dyn, Conversion <newton>(1.0 / 100000.0));
	DerivedUnitType(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0));
	DerivedUnitType(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));

	// PRESSURE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(pascals, pressure, Pa, 1.0);
	DerivedUnitTypeWithMetricPrefixes(bar, pressure, bar, Conversion<kilopascals>(100.0));
	DerivedUnitType(atmosphere, pressure, atm, Conversion<pascals>(101325.0));
	DerivedUnitType(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0)));
	DerivedUnitType(head, pressure, ft_water, Conversion<pound_f>(62.43) / (Conversion<foot>(1.0) * Conversion<foot>(1.0)));
	DerivedUnitType(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0));

	// CHARGE DERIVATIONS
	DerivedUnitType(coulomb, charge, C, 1.0); /* WithMetricPrefixes */
	DerivedUnitTypeWithMetricPrefixes(ampere_hour, charge, Ah, Conversion< ampere>(1.0)* Conversion<hour>(1.0));

	// POWER DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(watt, power, W, 1.0);
	DerivedUnitType(horsepower, power, hp, Conversion<watt>(7457.0 / 10.0));

	// ENERGY DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(joule, energy, J, 1.0);
	DerivedUnitTypeWithMetricPrefixes(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0));
	DerivedUnitTypeWithMetricPrefixes(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0));
	DerivedUnitTypeWithMetricPrefixes(watt_hour, energy, Wh, Conversion<watt>(1.0)* Conversion<hour>(1.0));
	DerivedUnitType(watt_day, energy, Wd, Conversion<watt>(1.0)* Conversion<day>(1.0));
	DerivedUnitType(british_thermal_unit, energy, BTU, Conversion<joule>(105505585262.0 / 100000000.0));
	DerivedUnitType(british_thermal_unit_iso, energy, BTU_iso, Conversion<joule>(1055056.0 / 1000.0));
	DerivedUnitType(british_thermal_unit_59, energy, BTU59, Conversion<joule>(1054804.0 / 1000.0));
	DerivedUnitType(therm, energy, thm, Conversion<british_thermal_unit_59>(100000.0));
	DerivedUnitType(foot_pound, energy, ftlbf, Conversion<joule>(13558179483314004.0 / 10000000000000000.0));

	// VOLTAGE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(volt, voltage, V, 1.0);

	// IMPEDANCE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(ohm, impedance, Ohm, 1.0);

	// CONDUCTANCE DERIVATIONS
	DerivedUnitType(siemens, conductance, S, 1.0); // WithMetricPrefixes

	// AREA DERIVATIONS
	DerivedUnitType(square_meter, area, sq_m, 1.0);
	DerivedUnitType(square_foot, area, sq_ft, Conversion<foot>(1.0)* Conversion<foot>(1.0));
	DerivedUnitType(square_inch, area, sq_in, Conversion<inch>(1.0)* Conversion<inch>(1.0));
	DerivedUnitType(square_mile, area, sq_mi, Conversion<mile>(1.0)* Conversion<mile>(1.0));
	DerivedUnitType(square_kilometer, area, sq_km, Conversion<kilometer>(1.0)* Conversion<kilometer>(1.0));
	DerivedUnitType(hectare, area, ha, Conversion<square_meter>(1000.0));
	DerivedUnitType(acre, area, acre, Conversion<square_foot>(43560.0));

	// VOLUME DERIVATIONS
	DerivedUnitType(cubic_meter, volume, cu_m, 1.0);
	DerivedUnitType(cubic_millimeter, volume, cu_mm, CUBED(Conversion<millimeter>(1.0)));
	DerivedUnitType(cubic_kilometer, volume, cu_km, CUBED(Conversion<kilometer>(1.0)));
	DerivedUnitTypeWithMetricPrefixes(liter, volume, L, CUBED(Conversion<decimeter>(1.0)));
	DerivedUnitType(cubic_inch, volume, cu_in, CUBED(Conversion<inch>(1.0)));
	DerivedUnitType(cubic_foot, volume, cu_ft, CUBED(Conversion<foot>(1.0)));
	DerivedUnitType(cubic_yard, volume, cu_yd, CUBED(Conversion<yard>(1.0)));
	DerivedUnitType(cubic_mile, volume, cu_mi, CUBED(Conversion<mile>(1.0)));
	DerivedUnitTypeWithMetricPrefixes(gallon, volume, gal, Conversion<cubic_inch>(231.0));
	DerivedUnitType(imperial_gallon, volume, igal, Conversion<gallon>(10.0 / 12.0));
	DerivedUnitType(million_gallon, volume, MG, Conversion<gallon>(1.0)* CalculateMetricPrefixV(mega));
	DerivedUnitType(imperial_million_gallon, volume, IMG, Conversion<imperial_gallon>(1.0)* CalculateMetricPrefixV(mega));
	DerivedUnitType(acre_foot, volume, ac_ft, Conversion<acre>(1.0)* Conversion<foot>(1.0));
	DerivedUnitType(quart, volume, qt, Conversion<gallon>(0.25));
	DerivedUnitType(pint, volume, pt, Conversion<quart>(0.5));
	DerivedUnitType(cup, volume, c, Conversion<pint>(0.5));
	DerivedUnitType(fluid_ounce, volume, fl_oz, Conversion<cup>(0.125));
	DerivedUnitType(barrel, volume, bl, Conversion<gallon>(42.0));
	DerivedUnitType(bushel, volume, bu, Conversion<cubic_inch>(215042.0 / 100.0));
	DerivedUnitType(cord, volume, cord, Conversion<cubic_foot>(128.0));
	DerivedUnitType(tablespoon, volume, tbsp, Conversion<fluid_ounce>(0.5));
	DerivedUnitType(teaspoon, volume, tsp, Conversion<fluid_ounce>(1.0 / 6.0));
	DerivedUnitType(pinch, volume, pinch, Conversion<teaspoon>(1.0 / 8.0));
	DerivedUnitType(dash, volume, dash, Conversion<pinch>(1.0 / 2.0));
	DerivedUnitType(drop, volume, drop, Conversion<fluid_ounce>(1.0 / 360.0));
	DerivedUnitType(fifth, volume, fifth, Conversion<gallon>(0.2));
	DerivedUnitType(dram, volume, dr, Conversion<fluid_ounce>(0.125));
	DerivedUnitType(gill, volume, gi, Conversion<fluid_ounce>(4.0));
	DerivedUnitType(peck, volume, pk, Conversion<bushel>(0.25));
	DerivedUnitType(sack, volume, sacks, Conversion<bushel>(3.0));
	DerivedUnitType(shot, volume, shots, Conversion<fluid_ounce>(3.0 / 2.0));
	DerivedUnitType(strike, volume, strikes, Conversion<bushel>(2.0));

	// FILLRATE DERIVATIONS
	DerivedUnitTypeWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0);
	DerivedUnitType(metric_ton_per_second, fillrate, mTs, Conversion<metric_ton>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(metric_ton_per_minute, fillrate, mTm, Conversion<metric_ton>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(metric_ton_per_hour, fillrate, mTh, Conversion<metric_ton>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(metric_ton_per_day, fillrate, mTd, Conversion<metric_ton>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(metric_ton_per_year, fillrate, mTy, Conversion<metric_ton>(1.0) / Conversion<year>(1.0));

	// FLOWRATE DERIVATIONS
	DerivedUnitType(cubic_meter_per_second, flowrate, cms, 1.0);
	DerivedUnitType(cubic_meter_per_hour, flowrate, cmh, Conversion<cubic_meter>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(cubic_meter_per_day, flowrate, cmd, Conversion<cubic_meter>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(cubic_millimeter_per_second, flowrate, cmms, Conversion<cubic_millimeter>(1.0) / Conversion<second>(1.0));
	DerivedUnitTypeWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion<liter>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(liter_per_minute, flowrate, lpm, Conversion<liter>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(liter_per_day, flowrate, lpd, Conversion<liter>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(megaliter_per_day, flowrate, Mlpd, Conversion<megaliter>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(cubic_inch_per_second, flowrate, cis, Conversion<cubic_inch>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(cubic_inch_per_hour, flowrate, cih, Conversion<cubic_inch>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(cubic_foot_per_second, flowrate, cfs, Conversion<cubic_foot>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(cubic_foot_per_hour, flowrate, cfh, Conversion<cubic_foot>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(gallon_per_second, flowrate, gps, Conversion<gallon>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(gallon_per_minute, flowrate, gpm, Conversion<gallon>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(gallon_per_hour, flowrate, gph, Conversion<gallon>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(gallon_per_day, flowrate, gpd, Conversion<gallon>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(gallon_per_year, flowrate, gpy, Conversion<gallon>(1.0) / Conversion<year>(1.0));
	DerivedUnitType(million_gallon_per_second, flowrate, MGS, Conversion<megagallon>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(million_gallon_per_minute, flowrate, MGM, Conversion<megagallon>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(million_gallon_per_hour, flowrate, MGH, Conversion<megagallon>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(million_gallon_per_day, flowrate, MGD, Conversion<megagallon>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(million_gallon_per_year, flowrate, MGY, Conversion<megagallon>(1.0) / Conversion<year>(1.0));
	DerivedUnitType(imperial_million_gallon_per_second, flowrate, IMGS, Conversion<imperial_million_gallon>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion<imperial_million_gallon>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion<imperial_million_gallon>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(imperial_million_gallon_per_day, flowrate, IMGD, Conversion<imperial_million_gallon>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(imperial_million_gallon_per_year, flowrate, IMGY, Conversion<imperial_million_gallon>(1.0) / Conversion<year>(1.0));
	DerivedUnitType(acre_foot_per_second, flowrate, ac_ft_s, Conversion<acre_foot>(1.0) / Conversion<second>(1.0));
	DerivedUnitType(acre_foot_per_minute, flowrate, ac_ft_m, Conversion<acre_foot>(1.0) / Conversion<minute>(1.0));
	DerivedUnitType(acre_foot_per_hour, flowrate, ac_ft_h, Conversion<acre_foot>(1.0) / Conversion<hour>(1.0));
	DerivedUnitType(acre_foot_per_day, flowrate, ac_ft_d, Conversion<acre_foot>(1.0) / Conversion<day>(1.0));
	DerivedUnitType(acre_foot_per_year, flowrate, ac_ft_y, Conversion<acre_foot>(1.0) / Conversion<year>(1.0));

	// DENSITY DERIVATIONS
	DerivedUnitType(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0);
	DerivedUnitType(grams_per_milliliter, density, g_per_mL, Conversion<gram>(1.0) / Conversion<milliliter>(1.0));
	DerivedUnitType(kilograms_per_liter, density, kg_per_L, Conversion<kilogram>(1.0) / Conversion<liter>(1.0));
	DerivedUnitType(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion<ounce>(1.0) / Conversion<cubic_foot>(1.0));
	DerivedUnitType(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion<ounce>(1.0) / Conversion<cubic_inch>(1.0));
	DerivedUnitType(ounces_per_gallon, density, oz_per_gal, Conversion<ounce>(1.0) / Conversion<gallon>(1.0));
	DerivedUnitType(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion<pound>(1.0) / Conversion<cubic_foot>(1.0));
	DerivedUnitType(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion<pound>(1.0) / Conversion<cubic_inch>(1.0));
	DerivedUnitType(pounds_per_gallon, density, lb_per_gal, Conversion<pound>(1.0) / Conversion<gallon>(1.0));
	DerivedUnitType(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion<slug>(1.0) / Conversion<cubic_foot>(1.0));

	// DOLLAR RATES DERIVATIONS
	DerivedUnitType(Dollar_per_joule, energy_cost_rate, USD_per_j, Conversion<Dollar>(1.0) / Conversion<joule>(1.0));
	DerivedUnitType(Dollar_per_kilowatt_hour, energy_cost_rate, USD_per_kWh, Conversion<Dollar>(1.0) / Conversion<kilowatt_hour>(1.0));
	DerivedUnitType(Dollar_per_watt, power_cost_rate, USD_per_w, Conversion<Dollar>(1.0) / Conversion<watt>(1.0));
	DerivedUnitType(Dollar_per_kilowatt, power_cost_rate, USD_per_kW, Conversion<Dollar>(1.0) / Conversion<kilowatt>(1.0));
	DerivedUnitType(Dollar_per_cubic_meter, volume_cost_rate, USD_per_cm, Conversion<Dollar>(1.0) / Conversion<cubic_meter>(1.0));
	DerivedUnitType(Dollar_per_gallon, volume_cost_rate, USD_per_gal, Conversion<Dollar>(1.0) / Conversion<gallon>(1.0));

	// Rates
	DerivedUnitType(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion<kilowatt_hour>(1.0) / Conversion<acre_foot>(1.0));
	DerivedUnitType(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion<Dollar>(1.0) / Conversion<mile>(1.0));
	DerivedUnitType(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion<Dollar>(1.0) / Conversion<metric_ton>(1.0));
	DerivedUnitType(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion<metric_ton>(1.0) / Conversion<kilowatt_hour>(1.0));
	// DerivedUnitType(per_year, time_rate, p_yr, 1.0 / Conversion<year>(1.0));

	class UnitsDetail {
	public:
#define CreateRow(model, Type) model->insert({ HashUnitAndRatio(HashUnits(Type::A(), Type::B(), Type::C(), Type::D(), Type::E()), Type::conversion()), { Type::specialized_abbreviation(), #Type } })
#define CreateRowWithMetricPrefixes(model, Type)\
			CreateRow(model, Type); \
			CreateRow(model, femto ## Type); \
			CreateRow(model, pico ## Type); \
			CreateRow(model, nano ## Type); \
			CreateRow(model, micro ## Type); \
			CreateRow(model, milli ## Type); \
			CreateRow(model, centi ## Type); \
			CreateRow(model, deci ## Type); \
			CreateRow(model, deca ## Type); \
			CreateRow(model, hecto ## Type); \
			CreateRow(model, kilo ## Type); \
			CreateRow(model, mega ## Type); \
			CreateRow(model, giga ## Type); \
			CreateRow(model, tera ## Type); \
			CreateRow(model, peta ## Type);

		/*! Lookup the abbreviation for the type based on its unique characteristic combination (time/length/mass/etc.) */
		static std::pair<const char*, const char*> lookup_impl(size_t ull) noexcept {
			static std::pair<const char*, const char*> out{ "","" };
			static std::mutex mut;
			static std::shared_ptr<void> Tag{ nullptr };

			std::shared_ptr<std::map<size_t, std::pair< const char*, const char*>>> model;

			if (!Tag) {
				mut.lock();
			}
			if (!Tag) {
				model = std::make_shared<std::map<size_t, std::pair< const char*, const char*>>>();
				{
					CreateRowWithMetricPrefixes(model, meter);
					CreateRow(model, foot);
					CreateRow(model, inch);
					CreateRow(model, mile);
					CreateRow(model, nauticalMile);
					CreateRow(model, astronicalUnit);
					CreateRow(model, yard);
					CreateRowWithMetricPrefixes(model, gram);
					CreateRow(model, metric_ton);
					CreateRow(model, pound);
					CreateRow(model, long_ton);
					CreateRow(model, short_ton);
					CreateRow(model, stone);
					CreateRow(model, ounce);
					CreateRow(model, carat);
					CreateRow(model, slug);
					CreateRowWithMetricPrefixes(model, second);
					CreateRow(model, minute);
					CreateRow(model, hour);
					CreateRow(model, day);
					CreateRow(model, week);
					CreateRow(model, year);
					CreateRow(model, month);
					CreateRow(model, julian_year);
					CreateRow(model, gregorian_year);
					CreateRowWithMetricPrefixes(model, ampere);
					CreateRow(model, Dollar);
					CreateRow(model, MillionDollar);
					CreateRowWithMetricPrefixes(model, hertz);
					CreateRow(model, meters_per_second);
					CreateRow(model, feet_per_second);
					CreateRow(model, feet_per_minute);
					CreateRow(model, feet_per_hour);
					CreateRow(model, miles_per_hour);
					CreateRow(model, kilometers_per_hour);
					CreateRow(model, knot);
					CreateRow(model, meters_per_second_squared);
					CreateRow(model, feet_per_second_squared);
					CreateRow(model, standard_gravity);
					CreateRowWithMetricPrefixes(model, newton);
					CreateRowWithMetricPrefixes(model, pound_f);
					CreateRow(model, dyne);
					CreateRow(model, kilopond);
					CreateRow(model, poundal);
					CreateRowWithMetricPrefixes(model, pascals);
					CreateRowWithMetricPrefixes(model, bar);
					CreateRow(model, atmosphere);
					CreateRow(model, pounds_per_square_inch);
					CreateRow(model, head);
					CreateRow(model, torr);
					CreateRow(model, coulomb); // WithMetricPrefixes
					CreateRowWithMetricPrefixes(model, ampere_hour);
					CreateRowWithMetricPrefixes(model, watt);
					CreateRow(model, horsepower);
					CreateRowWithMetricPrefixes(model, joule);
					CreateRowWithMetricPrefixes(model, calorie);
					CreateRowWithMetricPrefixes(model, watt_minute);
					CreateRowWithMetricPrefixes(model, watt_hour);
					CreateRow(model, watt_day);
					CreateRow(model, british_thermal_unit);
					CreateRow(model, british_thermal_unit_iso);
					CreateRow(model, british_thermal_unit_59);
					CreateRow(model, therm);
					CreateRow(model, foot_pound);
					CreateRowWithMetricPrefixes(model, volt);
					CreateRowWithMetricPrefixes(model, ohm);
					CreateRow(model, siemens); // WithMetricPrefixes
					CreateRow(model, square_meter);
					CreateRow(model, square_foot);
					CreateRow(model, square_inch);
					CreateRow(model, square_mile);
					CreateRow(model, square_kilometer);
					CreateRow(model, hectare);
					CreateRow(model, acre);
					CreateRow(model, cubic_meter);
					CreateRow(model, cubic_millimeter);
					CreateRow(model, cubic_kilometer);
					CreateRowWithMetricPrefixes(model, liter);
					CreateRow(model, cubic_inch);
					CreateRow(model, cubic_foot);
					CreateRow(model, cubic_yard);
					CreateRow(model, cubic_mile);
					CreateRowWithMetricPrefixes(model, gallon);
					CreateRow(model, imperial_gallon);
					CreateRow(model, million_gallon);
					CreateRow(model, imperial_million_gallon);
					CreateRow(model, acre_foot);
					CreateRow(model, quart);
					CreateRow(model, pint);
					CreateRow(model, cup);
					CreateRow(model, fluid_ounce);
					CreateRow(model, barrel);
					CreateRow(model, bushel);
					CreateRow(model, cord);
					CreateRow(model, tablespoon);
					CreateRow(model, teaspoon);
					CreateRow(model, pinch);
					CreateRow(model, dash);
					CreateRow(model, drop);
					CreateRow(model, fifth);
					CreateRow(model, dram);
					CreateRow(model, gill);
					CreateRow(model, peck);
					CreateRow(model, sack);
					CreateRow(model, shot);
					CreateRow(model, strike);
					CreateRowWithMetricPrefixes(model, gram_per_second);
					CreateRow(model, metric_ton_per_second);
					CreateRow(model, metric_ton_per_minute);
					CreateRow(model, metric_ton_per_hour);
					CreateRow(model, metric_ton_per_day);
					CreateRow(model, metric_ton_per_year);
					CreateRow(model, cubic_meter_per_second);
					CreateRow(model, cubic_meter_per_hour);
					CreateRow(model, cubic_meter_per_day);
					CreateRow(model, cubic_millimeter_per_second);
					CreateRowWithMetricPrefixes(model, liter_per_second);
					CreateRow(model, liter_per_minute);
					CreateRow(model, liter_per_day);
					CreateRow(model, megaliter_per_day);
					CreateRow(model, cubic_inch_per_second);
					CreateRow(model, cubic_inch_per_hour);
					CreateRow(model, cubic_foot_per_second);
					CreateRow(model, cubic_foot_per_hour);
					CreateRow(model, gallon_per_second);
					CreateRow(model, gallon_per_minute);
					CreateRow(model, gallon_per_hour);
					CreateRow(model, gallon_per_day);
					CreateRow(model, gallon_per_year);
					CreateRow(model, million_gallon_per_second);
					CreateRow(model, million_gallon_per_minute);
					CreateRow(model, million_gallon_per_hour);
					CreateRow(model, million_gallon_per_day);
					CreateRow(model, million_gallon_per_year);
					CreateRow(model, imperial_million_gallon_per_second);
					CreateRow(model, imperial_million_gallon_per_minute);
					CreateRow(model, imperial_million_gallon_per_hour);
					CreateRow(model, imperial_million_gallon_per_day);
					CreateRow(model, imperial_million_gallon_per_year);
					CreateRow(model, acre_foot_per_second);
					CreateRow(model, acre_foot_per_minute);
					CreateRow(model, acre_foot_per_hour);
					CreateRow(model, acre_foot_per_day);
					CreateRow(model, acre_foot_per_year);
					CreateRow(model, kilograms_per_cubic_meter);
					CreateRow(model, grams_per_milliliter);
					CreateRow(model, kilograms_per_liter);
					CreateRow(model, ounces_per_cubic_foot);
					CreateRow(model, ounces_per_cubic_inch);
					CreateRow(model, ounces_per_gallon);
					CreateRow(model, pounds_per_cubic_foot);
					CreateRow(model, pounds_per_cubic_inch);
					CreateRow(model, pounds_per_gallon);
					CreateRow(model, slugs_per_cubic_foot);
					CreateRow(model, Dollar_per_joule);
					CreateRow(model, Dollar_per_kilowatt_hour);
					CreateRow(model, Dollar_per_watt);
					CreateRow(model, Dollar_per_kilowatt);
					CreateRow(model, Dollar_per_cubic_meter);
					CreateRow(model, Dollar_per_gallon);
					CreateRow(model, kilowatt_hour_per_acre_foot);
					CreateRow(model, Dollar_per_mile);
					CreateRow(model, Dollar_per_ton);
					CreateRow(model, ton_per_kilowatt_hour);
					// CreateRow(model, per_year);
				}

				Tag = std::static_pointer_cast<void>(model);
				mut.unlock();
			}
			else {
				model = std::static_pointer_cast<std::map<size_t, std::pair< const char*, const char*>>>(Tag);
			}

			if (model && model->count(ull) > 0) return model->at(ull);
			else return out;
		};
#undef CreateRowWithMetricPrefixes
#undef CreateRow

#define CreateRow(model, Type) model->operator[](HashUnits(Type::A(), Type::B(), Type::C(), Type::D(), Type::E())).Add({ Type::specialized_abbreviation(), #Type }, Type::conversion(), true)
#define CreateRowWithMetricPrefixes(model, Type)\
			CreateRow(model, Type); \
			CreateRow(model, femto ## Type); \
			CreateRow(model, pico ## Type); \
			CreateRow(model, nano ## Type); \
			CreateRow(model, micro ## Type); \
			CreateRow(model, milli ## Type); \
			CreateRow(model, centi ## Type); \
			CreateRow(model, deci ## Type); \
			CreateRow(model, deca ## Type); \
			CreateRow(model, hecto ## Type); \
			CreateRow(model, kilo ## Type); \
			CreateRow(model, mega ## Type); \
			CreateRow(model, giga ## Type); \
			CreateRow(model, tera ## Type); \
			CreateRow(model, peta ## Type);

		/*
		UnitHash determines the class of unit (length, time, length/time, length/time^2, length^1.25, etc.
		UnitRatio determines the specific ratio within that class (meter, foot, inch, etc.)
		*/
		static std::pair< const char*, const char*>& lookup_impl_2(size_t UnitHash, fibers::synchronization::atomic_number<double>& UnitRatio) noexcept {
			static std::pair<const char*, const char*> out{ "","" };
			static std::mutex mut;
			static std::shared_ptr<void> Tag{ nullptr };

			std::shared_ptr < std::map<size_t, fibers::containers::Tree<std::pair< const char*, const char*>, double>> > model;

			if (!Tag) {
				mut.lock();
			}
			if (!Tag) {
				model = std::make_shared<std::map<size_t, fibers::containers::Tree<std::pair< const char*, const char*>, double>>>();
				{
					CreateRowWithMetricPrefixes(model, meter);
					CreateRow(model, foot);
					CreateRow(model, inch);
					CreateRow(model, mile);
					CreateRow(model, nauticalMile);
					CreateRow(model, astronicalUnit);
					CreateRow(model, yard);
					CreateRowWithMetricPrefixes(model, gram);
					CreateRow(model, metric_ton);
					CreateRow(model, pound);
					CreateRow(model, long_ton);
					CreateRow(model, short_ton);
					CreateRow(model, stone);
					CreateRow(model, ounce);
					CreateRow(model, carat);
					CreateRow(model, slug);
					CreateRowWithMetricPrefixes(model, second);
					CreateRow(model, minute);
					CreateRow(model, hour);
					CreateRow(model, day);
					CreateRow(model, week);
					CreateRow(model, year);
					CreateRow(model, month);
					CreateRow(model, julian_year);
					CreateRow(model, gregorian_year);
					CreateRowWithMetricPrefixes(model, ampere);
					CreateRow(model, Dollar);
					CreateRow(model, MillionDollar);
					CreateRowWithMetricPrefixes(model, hertz);
					CreateRow(model, meters_per_second);
					CreateRow(model, feet_per_second);
					CreateRow(model, feet_per_minute);
					CreateRow(model, feet_per_hour);
					CreateRow(model, miles_per_hour);
					CreateRow(model, kilometers_per_hour);
					CreateRow(model, knot);
					CreateRow(model, meters_per_second_squared);
					CreateRow(model, feet_per_second_squared);
					CreateRow(model, standard_gravity);
					CreateRowWithMetricPrefixes(model, newton);
					CreateRowWithMetricPrefixes(model, pound_f);
					CreateRow(model, dyne);
					CreateRow(model, kilopond);
					CreateRow(model, poundal);
					CreateRowWithMetricPrefixes(model, pascals);
					CreateRowWithMetricPrefixes(model, bar);
					CreateRow(model, atmosphere);
					CreateRow(model, pounds_per_square_inch);
					CreateRow(model, head);
					CreateRow(model, torr);
					CreateRow(model, coulomb); // WithMetricPrefixes
					CreateRowWithMetricPrefixes(model, ampere_hour);
					CreateRowWithMetricPrefixes(model, watt);
					CreateRow(model, horsepower);
					CreateRowWithMetricPrefixes(model, joule);
					CreateRowWithMetricPrefixes(model, calorie);
					CreateRowWithMetricPrefixes(model, watt_minute);
					CreateRowWithMetricPrefixes(model, watt_hour);
					CreateRow(model, watt_day);
					CreateRow(model, british_thermal_unit);
					CreateRow(model, british_thermal_unit_iso);
					CreateRow(model, british_thermal_unit_59);
					CreateRow(model, therm);
					CreateRow(model, foot_pound);
					CreateRowWithMetricPrefixes(model, volt);
					CreateRowWithMetricPrefixes(model, ohm);
					CreateRow(model, siemens); // WithMetricPrefixes
					CreateRow(model, square_meter);
					CreateRow(model, square_foot);
					CreateRow(model, square_inch);
					CreateRow(model, square_mile);
					CreateRow(model, square_kilometer);
					CreateRow(model, hectare);
					CreateRow(model, acre);
					CreateRow(model, cubic_meter);
					CreateRow(model, cubic_millimeter);
					CreateRow(model, cubic_kilometer);
					CreateRowWithMetricPrefixes(model, liter);
					CreateRow(model, cubic_inch);
					CreateRow(model, cubic_foot);
					CreateRow(model, cubic_yard);
					CreateRow(model, cubic_mile);
					CreateRowWithMetricPrefixes(model, gallon);
					CreateRow(model, imperial_gallon);
					CreateRow(model, million_gallon);
					CreateRow(model, imperial_million_gallon);
					CreateRow(model, acre_foot);
					CreateRow(model, quart);
					CreateRow(model, pint);
					CreateRow(model, cup);
					CreateRow(model, fluid_ounce);
					CreateRow(model, barrel);
					CreateRow(model, bushel);
					CreateRow(model, cord);
					CreateRow(model, tablespoon);
					CreateRow(model, teaspoon);
					CreateRow(model, pinch);
					CreateRow(model, dash);
					CreateRow(model, drop);
					CreateRow(model, fifth);
					CreateRow(model, dram);
					CreateRow(model, gill);
					CreateRow(model, peck);
					CreateRow(model, sack);
					CreateRow(model, shot);
					CreateRow(model, strike);
					CreateRowWithMetricPrefixes(model, gram_per_second);
					CreateRow(model, metric_ton_per_second);
					CreateRow(model, metric_ton_per_minute);
					CreateRow(model, metric_ton_per_hour);
					CreateRow(model, metric_ton_per_day);
					CreateRow(model, metric_ton_per_year);
					CreateRow(model, cubic_meter_per_second);
					CreateRow(model, cubic_meter_per_hour);
					CreateRow(model, cubic_meter_per_day);
					CreateRow(model, cubic_millimeter_per_second);
					CreateRowWithMetricPrefixes(model, liter_per_second);
					CreateRow(model, liter_per_minute);
					CreateRow(model, liter_per_day);
					CreateRow(model, megaliter_per_day);
					CreateRow(model, cubic_inch_per_second);
					CreateRow(model, cubic_inch_per_hour);
					CreateRow(model, cubic_foot_per_second);
					CreateRow(model, cubic_foot_per_hour);
					CreateRow(model, gallon_per_second);
					CreateRow(model, gallon_per_minute);
					CreateRow(model, gallon_per_hour);
					CreateRow(model, gallon_per_day);
					CreateRow(model, gallon_per_year);
					CreateRow(model, million_gallon_per_second);
					CreateRow(model, million_gallon_per_minute);
					CreateRow(model, million_gallon_per_hour);
					CreateRow(model, million_gallon_per_day);
					CreateRow(model, million_gallon_per_year);
					CreateRow(model, imperial_million_gallon_per_second);
					CreateRow(model, imperial_million_gallon_per_minute);
					CreateRow(model, imperial_million_gallon_per_hour);
					CreateRow(model, imperial_million_gallon_per_day);
					CreateRow(model, imperial_million_gallon_per_year);
					CreateRow(model, acre_foot_per_second);
					CreateRow(model, acre_foot_per_minute);
					CreateRow(model, acre_foot_per_hour);
					CreateRow(model, acre_foot_per_day);
					CreateRow(model, acre_foot_per_year);
					CreateRow(model, kilograms_per_cubic_meter);
					CreateRow(model, grams_per_milliliter);
					CreateRow(model, kilograms_per_liter);
					CreateRow(model, ounces_per_cubic_foot);
					CreateRow(model, ounces_per_cubic_inch);
					CreateRow(model, ounces_per_gallon);
					CreateRow(model, pounds_per_cubic_foot);
					CreateRow(model, pounds_per_cubic_inch);
					CreateRow(model, pounds_per_gallon);
					CreateRow(model, slugs_per_cubic_foot);
					CreateRow(model, Dollar_per_joule);
					CreateRow(model, Dollar_per_kilowatt_hour);
					CreateRow(model, Dollar_per_watt);
					CreateRow(model, Dollar_per_kilowatt);
					CreateRow(model, Dollar_per_cubic_meter);
					CreateRow(model, Dollar_per_gallon);
					CreateRow(model, kilowatt_hour_per_acre_foot);
					CreateRow(model, Dollar_per_mile);
					CreateRow(model, Dollar_per_ton);
					CreateRow(model, ton_per_kilowatt_hour);
					// CreateRow(model, per_year);
				}

				Tag = std::static_pointer_cast<void>(model);
				mut.unlock();
			}
			else {
				model = std::static_pointer_cast<std::map<size_t, fibers::containers::Tree<std::pair< const char*, const char*>, double>>>(Tag);
			}

			if (model && model->count(UnitHash) > 0) {
				auto& curve = model->at(UnitHash);
				if (curve.GetNodeCount() > 0) {
					auto knot = curve.NodeFindSmallestLargerEqual(UnitRatio.GetValue());
					if (knot && knot->object) {
						UnitRatio = knot->key;
						return *knot->object;
					}
				}
			}
			return out;
		};

#undef CreateRowWithMetricPrefixes
#undef CreateRow

		static const char* lookup_abbreviation(size_t ull) noexcept {
			return lookup_impl(ull).first;
		};
		static const char* lookup_typename(size_t ull) noexcept {
			return lookup_impl(ull).second;
		};
		static const char* lookup_abbreviation(size_t UnitHash, fibers::synchronization::atomic_number<double>& UnitRatio) noexcept {
			return lookup_impl_2(UnitHash, UnitRatio).first;
		};
		static const char* lookup_typename(size_t UnitHash, fibers::synchronization::atomic_number<double>& UnitRatio) noexcept {
			return lookup_impl_2(UnitHash, UnitRatio).second;
		};
	};

	class math {
	public:
		static Units::value fabs(const Units::value& V) {
			if (V < 0) return V * -1.0; else return V;
		};
		static Units::value abs(const Units::value& V) {
			return fabs(V);
		};
		static Units::value clamp(const Units::value& V, const Units::value& min, const Units::value& max) {
			if (V < min) return min;
			if (V > max) return max;
			return V;
		};
		static Units::value floor(const Units::value& f) {
			return f.floor();
		};
		static Units::value ceiling(const Units::value& f) {
			return f.ceiling();
		};
		static Units::value round(const Units::value& a, float magnitude) {
			return floor((a / magnitude) + 0.5) * magnitude;
		};
		static Units::value max(const Units::value& a, const Units::value& b) {
			return a > b ? a : b;
		};
		static Units::value min(const Units::value& a, const Units::value& b) {
			return a < b ? a : b;
		};
		static void max_ref(Units::value* a, const Units::value& b) {
			if (b > *a) *a = b;

		};
		static void min_ref(Units::value* a, const Units::value& b) {
			if (b < *a) *a = b;
		};
	};

	//class traits {
	//public:
	//	/* test if two unit types are convertable */
	//	template<class U1, class U2> struct is_convertible_unit_t {
	//		static constexpr const std::intmax_t value = HashUnits(U1::A(), U1::B(), U1::C(), U1::D(), U1::E()) == HashUnits(U2::A(), U2::B(), U2::C(), U2::D(), U2::E());
	//	};

	//	template<class U1> struct is_unit_t {
	//		static constexpr const std::intmax_t value = std::is_base_of<Units::value, U1>::value;
	//	};
	//};

	class constants {
	public:
		/* PI (unitless) */
		static Units::scalar					pi() { 
			return 3.141592653589793238462643383279502884197169399375105820974944; 
		}; 
		
		/* speed of light in a vacuum (m/s) */
		static Units::meters_per_second		    c() { 
			return 299792458.0; 
		}; 

		/* ( m^3 / (kg * s^2) ) */
		static Units::value				        G() { 
			return Units::meter(6.67408e-11) * Units::meter(1) * Units::meter(1) / (Units::kilogram(1) * Units::second(1) * Units::second(1)); 
		}; 
		
        /* acceleration due to gravity ( m/s^2 ) */
		static Units::value				        g() { 
			return Units::meter(9.8067) / (Units::second(1) * Units::second(1)); 
		};
		
		/* density of water ( kg/m^3 ) */
		static Units::value d() { 
			return Units::kilogram(998.57) / (Units::meter(1) * Units::meter(1) * Units::meter(1)); 
		};
	};
};

// Base classes
DefineCategoryStd(length, 1, 0, 0, 0, 0);
DefineCategoryStd(mass, 0, 1, 0, 0, 0);
DefineCategoryStd(time, 0, 0, 1, 0, 0);
DefineCategoryStd(current, 0, 0, 0, 1, 0);
DefineCategoryStd(dollar, 0, 0, 0, 0, 1);
// Derived classes
DefineCategoryStd(frequency, 0, 0, -1, 0, 0);
DefineCategoryStd(velocity, 1, 0, -1, 0, 0);
DefineCategoryStd(acceleration, 1, 0, -2, 0, 0);
DefineCategoryStd(force, 1, 1, -2, 0, 0);
DefineCategoryStd(pressure, -1, 1, -2, 0, 0);
DefineCategoryStd(charge, 0, 0, 1, 1, 0);
DefineCategoryStd(power, 2, 1, -3, 0, 0);
DefineCategoryStd(energy, 2, 1, -2, 0, 0);
DefineCategoryStd(voltage, 2, 1, -3, -1, 0);
DefineCategoryStd(impedance, 2, 1, -3, -2, 0);
DefineCategoryStd(conductance, -2, -1, 3, 2, 0);
DefineCategoryStd(area, 2, 0, 0, 0, 0);
DefineCategoryStd(volume, 3, 0, 0, 0, 0);
DefineCategoryStd(fillrate, 0, 1, -1, 0, 0);
DefineCategoryStd(flowrate, 3, 0, -1, 0, 0);
DefineCategoryStd(density, -3, 1, 0, 0, 0);
DefineCategoryStd(energy_cost_rate, -2, -1, 2, 0, 1);
DefineCategoryStd(power_cost_rate, -2, -1, 3, 0, 1);
DefineCategoryStd(volume_cost_rate, -3, 0, 0, 0, 1);
DefineCategoryStd(energy_intensity, -1, 1, -2, 0, 1);
DefineCategoryStd(length_cost_rate, -1, 0, 0, 0, 1);
DefineCategoryStd(mass_cost_rate, 0, -1, 0, 0, 1);
DefineCategoryStd(emission_rate, -2, 0, 2, 0, 1);
DefineCategoryStd(time_rate, 0, 0, -1, 0, 1);

/* Unit Literals (e.g. 1_ft, 10.0_gpm, 0.01_cfs, etc.) */
DerivedUnitStdWithMetricPrefixes(meter, length, m, 1.0); 
DerivedUnitStd(foot, length, ft, 381.0 / 1250.0);
DerivedUnitStd(inch, length, in, Conversion<foot>(1.0 / 12.0));
DerivedUnitStd(mile, length, mi, Conversion<foot>(5280.0 / 1.0));
DerivedUnitStd(nauticalMile, length, nmi, Conversion<meter>(1852.0));
DerivedUnitStd(astronicalUnit, length, au, Conversion<meter>(149597870700.0));
DerivedUnitStd(yard, length, yd, Conversion<foot>(3.0));

/* MASS DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(gram, mass, g, 1.0 / 1000.0);
DerivedUnitStd(metric_ton, mass, t, Conversion<kilogram>(1000.0));
DerivedUnitStd(pound, mass, lb, Conversion<kilogram>(45359237.0 / 100000000.0));
DerivedUnitStd(long_ton, mass, ln_t, Conversion < pound>(2240.0));
DerivedUnitStd(short_ton, mass, sh_t, Conversion < pound>(2000.0));
DerivedUnitStd(stone, mass, st, Conversion < pound>(14.0));
DerivedUnitStd(ounce, mass, oz, Conversion < pound>(1.0 / 16.0));
DerivedUnitStd(carat, mass, ct, Conversion < milligram>(200.0));
DerivedUnitStd(slug, mass, slug, Conversion<kilogram>(145939029.0 / 10000000.0));

/* TIME DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(second, time, s, 1.0);
DerivedUnitStd(minute, time, min, Conversion<second>(60.0));
DerivedUnitStd(hour, time, hr, Conversion<minute>(60.0));
DerivedUnitStd(day, time, d, Conversion<hour>(24.0));
DerivedUnitStd(week, time, wk, Conversion<day>(7.0));
DerivedUnitStd(year, time, yr, Conversion<day>(365));
DerivedUnitStd(month, time, mnth, Conversion<year>(1.0 / 12.0));
DerivedUnitStd(julian_year, time, a_j, Conversion<second>(31557600.0));
DerivedUnitStd(gregorian_year, time, a_g, Conversion<second>(31556952.0));

/* CURRENT DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(ampere, current, A, 1.0);

/* DOLLAR DERIVATIONS */
DerivedUnitStd(Dollar, dollar, USD, 1.0);
DerivedUnitStd(MillionDollar, dollar, MUSD, Conversion<Dollar>(1000000.0));

/* FREQUENCY DERIVATIONS */
DerivedUnitStdWithMetricPrefixes(hertz, frequency, Hz, 1.0);

/* VELOCITY DERIVATIONS */
DerivedUnitStd(meters_per_second, velocity, mps, Conversion<meter>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(feet_per_second, velocity, fps, Conversion<foot>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(feet_per_minute, velocity, fpm, Conversion<foot>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(feet_per_hour, velocity, fph, Conversion<foot>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(miles_per_hour, velocity, mph, Conversion<mile>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(kilometers_per_hour, velocity, kph, Conversion<kilometer>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(knot, velocity, kts, Conversion<nauticalMile>(1.0) / Conversion<hour>(1.0));

/* ACCELERATION DERIVATIONS */
DerivedUnitStd(meters_per_second_squared, acceleration, mps_sq, Conversion<meter>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
DerivedUnitStd(feet_per_second_squared, acceleration, fps_sq, Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));
DerivedUnitStd(standard_gravity, acceleration, SG, Conversion<meters_per_second_squared>(980665.0 / 100000.0));

// FORCE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(newton, force, N, Conversion<kilogram>(1.0)* Conversion<meters_per_second_squared>(1.0));
DerivedUnitStdWithMetricPrefixes(pound_f, force, lbf, Conversion<slug>(1.0)* Conversion<feet_per_second_squared>(1.0));
DerivedUnitStd(dyne, force, dyn, Conversion <newton>(1.0 / 100000.0));
DerivedUnitStd(kilopond, force, kp, Conversion<standard_gravity>(1.0)* Conversion<kilogram>(1.0));
DerivedUnitStd(poundal, force, pdl, Conversion<pound>(1.0)* Conversion<foot>(1.0) / (Conversion<second>(1.0) * Conversion<second>(1.0)));

// PRESSURE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(pascals, pressure, Pa, 1.0);
DerivedUnitStdWithMetricPrefixes(bar, pressure, bar, Conversion<kilopascals>(100.0));
DerivedUnitStd(atmosphere, pressure, atm, Conversion<pascals>(101325.0));
DerivedUnitStd(pounds_per_square_inch, pressure, psi, Conversion<pound_f>(1.0) / (Conversion<inch>(1.0) * Conversion<inch>(1.0)));
DerivedUnitStd(head, pressure, ft_water, Conversion<pound_f>(62.43) / (Conversion<foot>(1.0) * Conversion<foot>(1.0)));
DerivedUnitStd(torr, pressure, torr, Conversion<atmosphere>(1.0 / 760.0));

// CHARGE DERIVATIONS
DerivedUnitStd(coulomb, charge, C, 1.0); /* WithMetricPrefixes */
DerivedUnitStdWithMetricPrefixes(ampere_hour, charge, Ah, Conversion< ampere>(1.0)* Conversion<hour>(1.0));

// POWER DERIVATIONS
DerivedUnitStdWithMetricPrefixes(watt, power, W, 1.0);
DerivedUnitStd(horsepower, power, hp, Conversion<watt>(7457.0 / 10.0));

// ENERGY DERIVATIONS
DerivedUnitStdWithMetricPrefixes(joule, energy, J, 1.0);
DerivedUnitStdWithMetricPrefixes(calorie, energy, cal, Conversion<joule>(4184.0 / 1000.0));
DerivedUnitStdWithMetricPrefixes(watt_minute, energy, Wm, Conversion<watt>(1.0)* Conversion<minute>(1.0));
DerivedUnitStdWithMetricPrefixes(watt_hour, energy, Wh, Conversion<watt>(1.0)* Conversion<hour>(1.0));
DerivedUnitStd(watt_day, energy, Wd, Conversion<watt>(1.0)* Conversion<day>(1.0));
DerivedUnitStd(british_thermal_unit, energy, BTU, Conversion<joule>(105505585262.0 / 100000000.0));
DerivedUnitStd(british_thermal_unit_iso, energy, BTU_iso, Conversion<joule>(1055056.0 / 1000.0));
DerivedUnitStd(british_thermal_unit_59, energy, BTU59, Conversion<joule>(1054804.0 / 1000.0));
DerivedUnitStd(therm, energy, thm, Conversion<british_thermal_unit_59>(100000.0));
DerivedUnitStd(foot_pound, energy, ftlbf, Conversion<joule>(13558179483314004.0 / 10000000000000000.0));

// VOLTAGE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(volt, voltage, V, 1.0);

// IMPEDANCE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(ohm, impedance, Ohm, 1.0);

// CONDUCTANCE DERIVATIONS
DerivedUnitStd(siemens, conductance, S, 1.0); // WithMetricPrefixes

// AREA DERIVATIONS
DerivedUnitStd(square_meter, area, sq_m, 1.0);
DerivedUnitStd(square_foot, area, sq_ft, Conversion<foot>(1.0)* Conversion<foot>(1.0));
DerivedUnitStd(square_inch, area, sq_in, Conversion<inch>(1.0)* Conversion<inch>(1.0));
DerivedUnitStd(square_mile, area, sq_mi, Conversion<mile>(1.0)* Conversion<mile>(1.0));
DerivedUnitStd(square_kilometer, area, sq_km, Conversion<kilometer>(1.0)* Conversion<kilometer>(1.0));
DerivedUnitStd(hectare, area, ha, Conversion<square_meter>(1000.0));
DerivedUnitStd(acre, area, acre, Conversion<square_foot>(43560.0));

// VOLUME DERIVATIONS
DerivedUnitStd(cubic_meter, volume, cu_m, 1.0);
DerivedUnitStd(cubic_millimeter, volume, cu_mm, CUBED(Conversion<millimeter>(1.0)));
DerivedUnitStd(cubic_kilometer, volume, cu_km, CUBED(Conversion<kilometer>(1.0)));
DerivedUnitStdWithMetricPrefixes(liter, volume, L, CUBED(Conversion<decimeter>(1.0)));
DerivedUnitStd(cubic_inch, volume, cu_in, CUBED(Conversion<inch>(1.0)));
DerivedUnitStd(cubic_foot, volume, cu_ft, CUBED(Conversion<foot>(1.0)));
DerivedUnitStd(cubic_yard, volume, cu_yd, CUBED(Conversion<yard>(1.0)));
DerivedUnitStd(cubic_mile, volume, cu_mi, CUBED(Conversion<mile>(1.0)));
DerivedUnitStdWithMetricPrefixes(gallon, volume, gal, Conversion<cubic_inch>(231.0));
DerivedUnitStd(imperial_gallon, volume, igal, Conversion<gallon>(10.0 / 12.0));
DerivedUnitStd(million_gallon, volume, MG, Conversion<gallon>(1.0)* CalculateMetricPrefixV(mega));
DerivedUnitStd(imperial_million_gallon, volume, IMG, Conversion<imperial_gallon>(1.0)* CalculateMetricPrefixV(mega));
DerivedUnitStd(acre_foot, volume, ac_ft, Conversion<acre>(1.0)* Conversion<foot>(1.0));
DerivedUnitStd(quart, volume, qt, Conversion<gallon>(0.25));
DerivedUnitStd(pint, volume, pt, Conversion<quart>(0.5));
DerivedUnitStd(cup, volume, c, Conversion<pint>(0.5));
DerivedUnitStd(fluid_ounce, volume, fl_oz, Conversion<cup>(0.125));
DerivedUnitStd(barrel, volume, bl, Conversion<gallon>(42.0));
DerivedUnitStd(bushel, volume, bu, Conversion<cubic_inch>(215042.0 / 100.0));
DerivedUnitStd(cord, volume, cord, Conversion<cubic_foot>(128.0));
DerivedUnitStd(tablespoon, volume, tbsp, Conversion<fluid_ounce>(0.5));
DerivedUnitStd(teaspoon, volume, tsp, Conversion<fluid_ounce>(1.0 / 6.0));
DerivedUnitStd(pinch, volume, pinch, Conversion<teaspoon>(1.0 / 8.0));
DerivedUnitStd(dash, volume, dash, Conversion<pinch>(1.0 / 2.0));
DerivedUnitStd(drop, volume, drop, Conversion<fluid_ounce>(1.0 / 360.0));
DerivedUnitStd(fifth, volume, fifth, Conversion<gallon>(0.2));
DerivedUnitStd(dram, volume, dr, Conversion<fluid_ounce>(0.125));
DerivedUnitStd(gill, volume, gi, Conversion<fluid_ounce>(4.0));
DerivedUnitStd(peck, volume, pk, Conversion<bushel>(0.25));
DerivedUnitStd(sack, volume, sacks, Conversion<bushel>(3.0));
DerivedUnitStd(shot, volume, shots, Conversion<fluid_ounce>(3.0 / 2.0));
DerivedUnitStd(strike, volume, strikes, Conversion<bushel>(2.0));

// FILLRATE DERIVATIONS
DerivedUnitStdWithMetricPrefixes(gram_per_second, fillrate, gs, 1.0 / 1000.0);
DerivedUnitStd(metric_ton_per_second, fillrate, mTs, Conversion<metric_ton>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(metric_ton_per_minute, fillrate, mTm, Conversion<metric_ton>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(metric_ton_per_hour, fillrate, mTh, Conversion<metric_ton>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(metric_ton_per_day, fillrate, mTd, Conversion<metric_ton>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(metric_ton_per_year, fillrate, mTy, Conversion<metric_ton>(1.0) / Conversion<year>(1.0));

// FLOWRATE DERIVATIONS
DerivedUnitStd(cubic_meter_per_second, flowrate, cms, 1.0);
DerivedUnitStd(cubic_meter_per_hour, flowrate, cmh, Conversion<cubic_meter>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(cubic_meter_per_day, flowrate, cmd, Conversion<cubic_meter>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(cubic_millimeter_per_second, flowrate, cmms, Conversion<cubic_millimeter>(1.0) / Conversion<second>(1.0));
DerivedUnitStdWithMetricPrefixes(liter_per_second, flowrate, lps, Conversion<liter>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(liter_per_minute, flowrate, lpm, Conversion<liter>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(liter_per_day, flowrate, lpd, Conversion<liter>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(megaliter_per_day, flowrate, Mlpd, Conversion<megaliter>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(cubic_inch_per_second, flowrate, cis, Conversion<cubic_inch>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(cubic_inch_per_hour, flowrate, cih, Conversion<cubic_inch>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(cubic_foot_per_second, flowrate, cfs, Conversion<cubic_foot>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(cubic_foot_per_hour, flowrate, cfh, Conversion<cubic_foot>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(gallon_per_second, flowrate, gps, Conversion<gallon>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(gallon_per_minute, flowrate, gpm, Conversion<gallon>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(gallon_per_hour, flowrate, gph, Conversion<gallon>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(gallon_per_day, flowrate, gpd, Conversion<gallon>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(gallon_per_year, flowrate, gpy, Conversion<gallon>(1.0) / Conversion<year>(1.0));
DerivedUnitStd(million_gallon_per_second, flowrate, MGS, Conversion<megagallon>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(million_gallon_per_minute, flowrate, MGM, Conversion<megagallon>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(million_gallon_per_hour, flowrate, MGH, Conversion<megagallon>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(million_gallon_per_day, flowrate, MGD, Conversion<megagallon>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(million_gallon_per_year, flowrate, MGY, Conversion<megagallon>(1.0) / Conversion<year>(1.0));
DerivedUnitStd(imperial_million_gallon_per_second, flowrate, IMGS, Conversion<imperial_million_gallon>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(imperial_million_gallon_per_minute, flowrate, IMGM, Conversion<imperial_million_gallon>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(imperial_million_gallon_per_hour, flowrate, IMGH, Conversion<imperial_million_gallon>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(imperial_million_gallon_per_day, flowrate, IMGD, Conversion<imperial_million_gallon>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(imperial_million_gallon_per_year, flowrate, IMGY, Conversion<imperial_million_gallon>(1.0) / Conversion<year>(1.0));
DerivedUnitStd(acre_foot_per_second, flowrate, ac_ft_s, Conversion<acre_foot>(1.0) / Conversion<second>(1.0));
DerivedUnitStd(acre_foot_per_minute, flowrate, ac_ft_m, Conversion<acre_foot>(1.0) / Conversion<minute>(1.0));
DerivedUnitStd(acre_foot_per_hour, flowrate, ac_ft_h, Conversion<acre_foot>(1.0) / Conversion<hour>(1.0));
DerivedUnitStd(acre_foot_per_day, flowrate, ac_ft_d, Conversion<acre_foot>(1.0) / Conversion<day>(1.0));
DerivedUnitStd(acre_foot_per_year, flowrate, ac_ft_y, Conversion<acre_foot>(1.0) / Conversion<year>(1.0));

// DENSITY DERIVATIONS
DerivedUnitStd(kilograms_per_cubic_meter, density, kg_per_cu_m, 1.0);
DerivedUnitStd(grams_per_milliliter, density, g_per_mL, Conversion<gram>(1.0) / Conversion<milliliter>(1.0));
DerivedUnitStd(kilograms_per_liter, density, kg_per_L, Conversion<kilogram>(1.0) / Conversion<liter>(1.0));
DerivedUnitStd(ounces_per_cubic_foot, density, oz_per_cu_ft, Conversion<ounce>(1.0) / Conversion<cubic_foot>(1.0));
DerivedUnitStd(ounces_per_cubic_inch, density, oz_per_cu_in, Conversion<ounce>(1.0) / Conversion<cubic_inch>(1.0));
DerivedUnitStd(ounces_per_gallon, density, oz_per_gal, Conversion<ounce>(1.0) / Conversion<gallon>(1.0));
DerivedUnitStd(pounds_per_cubic_foot, density, lb_per_cu_ft, Conversion<pound>(1.0) / Conversion<cubic_foot>(1.0));
DerivedUnitStd(pounds_per_cubic_inch, density, lb_per_cu_in, Conversion<pound>(1.0) / Conversion<cubic_inch>(1.0));
DerivedUnitStd(pounds_per_gallon, density, lb_per_gal, Conversion<pound>(1.0) / Conversion<gallon>(1.0));
DerivedUnitStd(slugs_per_cubic_foot, density, slug_per_cu_ft, Conversion<slug>(1.0) / Conversion<cubic_foot>(1.0));

// DOLLAR RATES DERIVATIONS
DerivedUnitStd(Dollar_per_joule, energy_cost_rate, USD_per_j, Conversion<Dollar>(1.0) / Conversion<joule>(1.0));
DerivedUnitStd(Dollar_per_kilowatt_hour, energy_cost_rate, USD_per_kWh, Conversion<Dollar>(1.0) / Conversion<kilowatt_hour>(1.0));
DerivedUnitStd(Dollar_per_watt, power_cost_rate, USD_per_w, Conversion<Dollar>(1.0) / Conversion<watt>(1.0));
DerivedUnitStd(Dollar_per_kilowatt, power_cost_rate, USD_per_kW, Conversion<Dollar>(1.0) / Conversion<kilowatt>(1.0));
DerivedUnitStd(Dollar_per_cubic_meter, volume_cost_rate, USD_per_cm, Conversion<Dollar>(1.0) / Conversion<cubic_meter>(1.0));
DerivedUnitStd(Dollar_per_gallon, volume_cost_rate, USD_per_gal, Conversion<Dollar>(1.0) / Conversion<gallon>(1.0));

// Rates
DerivedUnitStd(kilowatt_hour_per_acre_foot, energy_intensity, kWh_p_ac_ft, Conversion<kilowatt_hour>(1.0) / Conversion<acre_foot>(1.0));
DerivedUnitStd(Dollar_per_mile, length_cost_rate, USD_p_mi, Conversion<Dollar>(1.0) / Conversion<mile>(1.0));
DerivedUnitStd(Dollar_per_ton, mass_cost_rate, USD_p_t, Conversion<Dollar>(1.0) / Conversion<metric_ton>(1.0));
DerivedUnitStd(ton_per_kilowatt_hour, emission_rate, t_p_kWh, Conversion<metric_ton>(1.0) / Conversion<kilowatt_hour>(1.0));
// DerivedUnitStd(per_year, time_rate, p_yr, 1.0 / Conversion<year>(1.0));

namespace std {
	template<> class numeric_limits<Units::value> {
	public:
		static constexpr double min() { return std::numeric_limits<double>::min(); }
		static constexpr double max() { return std::numeric_limits<double>::max(); }
		static constexpr double lowest() { return std::numeric_limits<double>::lowest(); }
		static constexpr bool is_integer = std::numeric_limits<double>::is_integer;
		static constexpr bool is_signed = std::numeric_limits<double>::is_signed;
	};
};

#undef DefineCategoryType
#undef DefineCategoryStd

#undef DerivedUnitTypeWithMetricPrefixes
#undef DerivedUnitTypeWithMetricPrefix
#undef DerivedUnitStdWithMetricPrefixes
#undef DerivedUnitStdWithMetricPrefix
#undef CalculateMetricPrefixV
#undef DerivedUnitType



class MultithreadingInstanceManager {
	public:
		MultithreadingInstanceManager() {};
		virtual ~MultithreadingInstanceManager() {};
	};
/* Instances the fiber system, and destroys it if the DLL / library is unloaded. */
extern std::shared_ptr<MultithreadingInstanceManager> multithreadingInstance;