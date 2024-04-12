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
#endif
#pragma endregion 

#include "Actions.h"
#include "Concurrent_Queue.h"
namespace fibers {
	namespace utilities {
		class Hardware {
		public:
			static int GetNumCpuCores();
			static float GetPercentCpuLoad();
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

		template<typename type = long>
		class atomic_num {
		private:
			static type Sys_InterlockedIncrementV(type& value) { return InterlockedIncrementAcquire(&value); }
			static type Sys_InterlockedDecrementV(type& value) { return InterlockedDecrementRelease(&value); }
			static type Sys_InterlockedAddV(type& value, const type& i) { return InterlockedExchangeAdd(&value, i) + i; }
			static type Sys_InterlockedSubV(type& value, const type& i) { return InterlockedExchangeAdd(&value, -i) - i; }
			static type Sys_InterlockedExchangeV(type& value, const type& exchange) { return InterlockedExchange(&value, exchange); }
			static type Sys_InterlockedCompareExchangeV(type& value, const type& comparand, const type& exchange) { return InterlockedCompareExchange(&value, exchange, comparand); }

		public:
			atomic_num() : value(static_cast<type>(0)) {};
			atomic_num(const type& a) : value(a) {};
			atomic_num(const atomic_num& other) : value(other.GetValue()) {};
			atomic_num& operator=(const atomic_num& other) { SetValue(other.GetValue()); return *this; };
			atomic_num& operator=(const type& newSource) { SetValue(newSource); return *this; };

			operator type() { return GetValue(); };
			operator type() const { return GetValue(); };

			friend atomic_num operator+(const type& i, const atomic_num& b) { atomic_num out(b); out.Add(i); return out; };
			friend atomic_num operator+(const atomic_num& b, const type& i) { atomic_num out(b); out.Add(i); return out; };
			friend atomic_num operator-(const type& i, const atomic_num& b) { atomic_num out(i); out.Add(-b.GetValue()); return out; };
			friend atomic_num operator-(const atomic_num& b, const type& i) { atomic_num out(b); out.Add(-i); return out; };
			friend atomic_num operator/(const type& i, const atomic_num& b) { return i / b.GetValue(); };
			friend atomic_num operator/(const atomic_num& b, const type& i) { return b.GetValue() / i; };

			atomic_num& operator+=(int i) { Add(i); return *this; };
			atomic_num& operator-=(int i) { Add(-i); return *this; };

			atomic_num& operator+=(const atomic_num& i) { Add(i.GetValue()); return *this; };
			atomic_num& operator-=(const atomic_num& i) { Add(-i.GetValue()); return *this; };

			bool operator==(const type& i) { return i == GetValue(); };
			bool operator!=(const type& i) { return i != GetValue(); };
			bool operator==(const atomic_num& i) { return i.GetValue() == GetValue(); };
			bool operator!=(const atomic_num& i) { return i.GetValue() != GetValue(); };

			friend bool operator<=(const type& i, const atomic_num& b) { return i <= b.GetValue(); };
			friend bool operator<=(const atomic_num& b, const type& i) { return i > b.GetValue(); };
			friend bool operator>=(const type& i, const atomic_num& b) { return i >= b.GetValue(); };
			friend bool operator>=(const atomic_num& b, const type& i) { return i < b.GetValue(); };
			friend bool operator>(const type& i, const atomic_num& b) { return i > b.GetValue(); };
			friend bool operator>(const atomic_num& b, const type& i) { return i <= b.GetValue(); };
			friend bool operator<(const type& i, const atomic_num& b) { return i < b.GetValue(); };
			friend bool operator<(const atomic_num& b, const type& i) { return i >= b.GetValue(); };

			friend bool operator<=(const atomic_num& i, const atomic_num& b) { return i.GetValue() <= b.GetValue(); };
			friend bool operator>=(const atomic_num& i, const atomic_num& b) { return i.GetValue() >= b.GetValue(); };
			friend bool operator>(const atomic_num& i, const atomic_num& b) { return i.GetValue() > b.GetValue(); };
			friend bool operator<(const atomic_num& i, const atomic_num& b) { return i.GetValue() < b.GetValue(); };

			type					Increment() { return Sys_InterlockedIncrementV(value); } // atomically increments the integer and returns the new value
			type					Decrement() { return Sys_InterlockedDecrementV(value); } // atomically decrements the integer and returns the new value
			type					Add(const type& v) { return Sys_InterlockedAddV(value, (type)v); } // atomically adds a value to the integer and returns the new value
			type					Sub(const type& v) { return Sys_InterlockedSubV(value, (type)v); } // atomically subtracts a value from the integer and returns the new value
			type					GetValue() const { return value; }
			void				    SetValue(const type& v) { value = v; };

		private:
			type	            value;

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
			const T* Get() const noexcept { return ptr; };

		protected:
			T* ptr;
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
			bool isFirstJobInGroup;	// is the current job the first one in the group?
			bool isLastJobInGroup;	// is the current job the last one in the group?
			void* sharedmemory;		// stack memory shared within the current group (jobs within a group execute serially)
		};

		// Defines a state of execution, can be waited on
		struct context {
			synchronization::atomic_num<uint32_t> counter{ 0 };
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

		struct TaskQueue {
			std::deque<Task> queue;
			synchronization::CriticalMutexLock locker;
			
			__forceinline void push(Task&& item) {
				std::scoped_lock lock(locker);
				queue.push_back(std::forward<Task>(item));
			};
			__forceinline void push(const Task& item) {
				std::scoped_lock lock(locker);
				queue.push_back(item);
			};
			__forceinline bool try_pop(Task& item) {
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
			std::unique_ptr<TaskQueue[]> jobQueuePerThread;
			std::atomic_bool alive{ true };
			std::condition_variable_any wakeCondition; // std::condition_variable wakeCondition;
			synchronization::CriticalMutexLock wakeMutex; // std::mutex wakeMutex;
			std::atomic<uint32_t> nextQueue{ 0 };
			std::vector<std::thread> threads;
			void ShutDown() {
				alive.store(false); // indicate that new jobs cannot be started from this point
				bool wake_loop = true;
				std::thread waker([&] {
					while (wake_loop)
					{
						wakeCondition.notify_all(); // wakes up sleeping worker threads
					}
					});
				for (auto& thread : threads)
				{
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

class MultithreadingInstanceManager {
	public:
		MultithreadingInstanceManager() {};
		virtual ~MultithreadingInstanceManager() {};
	};
/* Instances the fiber system, and destroys it if the DLL / library is unloaded. */
extern std::shared_ptr<MultithreadingInstanceManager> multithreadingInstance;