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
namespace fibers{
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
		template<typename _Value_type> using queue = concurrency::concurrent_queue<_Value_type>; /* Wrapper To-Do */
    };

	/*! Class used to queue and await one or multiple jobs submitted to a concurrent fiber manager. */
	class JobGroup;

	/*!
	Class used to define and easily shared work that can be performed concurrently on in-line. e.g:
	int result1 = Job(&cweeMath::Ceil, 10.0f).Invoke().cast(); // Job takes function and up to 16 inputs. Invoke returns "Any" wraper. Any.cast() does the cast to the target destination, if the conversion makes sense.
	float result2 = Job([](float& x)->float{ return x - 10.0f; }, 55.0f).Invoke().cast(); // Can also use lambdas instead of static function pointers.
	Job([](){ return cweeStr("HELLO"); }).AsyncInvoke(); // Queues the job to take place on a fiber/thread, allowing you to continue work on this thread.
	*/
	class Job {
		friend JobGroup;
	protected:
		mutable std::shared_ptr<Action> impl;

	public:
		static Job Finished() {
			decltype(auto) toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished());
			return toReturn;
		};
		template <typename T> static Job Finished(const T& returnMe) {
			decltype(auto) toReturn = Job();
			toReturn.impl = std::make_shared<Action>(Action::Finished(returnMe));
			return toReturn;
		};

		Job() : impl(std::make_shared<Action>()) {};
		Job(const Job& other) : impl(other.impl) {};
		Job(Job&& other) : impl(other.impl) {};
		Job& operator=(const Job& other) { impl = other.impl; return *this; };
		Job& operator=(Job&& other) { impl = std::move(other.impl); return *this; };
		template < typename T, typename... Args, typename = std::enable_if_t< !std::is_same_v<Job, std::decay_t<T>> && !std::is_same_v<Any, std::decay_t<T>> >>
		explicit Job(T function, Args && ... Fargs) : impl(new Action(function, std::forward<Args>(Fargs)...)) {};
		~Job() = default;

	public:
		/* Do the task immediately, without using any thread/fiber tools. Does not do the task if it has been previously performed. */
		Any Invoke() noexcept {
			Any out;
			if (impl) {
				auto* p = impl->Invoke();
				if (p) out = *p;
			}
			return out;
		};

		/* Do the task immediately, without using any thread/fiber tools, whether or not it has been performed before. */
		Any ForceInvoke() noexcept {
			Any out;
			if (impl) {
				auto* p = impl->ForceInvoke();
				if (p) out = *p;
			}
			return out;
		};

		/* Add the task to a thread / fiber, and retrieve an awaiter group. Does not do the task if it has been previously performed. */
		JobGroup AsyncInvoke();

		/* Add the task to a thread / fiber, and retrieve an awaiter group, whether or not it has been performed before. */
		JobGroup AsyncForceInvoke();

		/* Queue job, and return tool to await the result */
		void DelayedInvoke(double milliseconds_delay);

		/* Queue job, and return tool to await the result */
		void DelayedForceInvoke(double milliseconds_delay);

		/* Queues the job onto a new thread to take place in the future */
		void AsyncDelayedInvoke(double milliseconds_delay);

		/* Queues the job onto a new thread to take place in the future */
		void AsyncDelayedForceInvoke(double milliseconds_delay);

		/* Returns the potential name of the static function, if one was provided. */
		const char* FunctionName() const {
			if (impl) {
				return impl->FunctionName();
			}
			return "";
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any GetResult() {
			return Invoke();
		};

		/* Returns the result of the job, if any. If the job has not been previously completed, it will perform the job. */
		Any operator()() {
			return Invoke();
		};

		/* Checks if the job has been completed before */
		bool IsFinished() const {
			if (impl) {
				return impl->IsFinished();
			}
			return true;
		};

		bool ReturnsNothing() const {
			if (impl) {
				return impl->ReturnsNothing();
			}
			return true;
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
			void ForceQueue(Job const& job); 
			void Queue(std::vector<Job> const& listOfJobs);
			void ForceQueue(std::vector<Job> const& listOfJobs);
			void Wait();
			~JobGroupImpl() {
				// Wait(); // any jobs queued with this waiter will want to talk w/it when finished -- we must wait to ensure they go out of scope before we do.
			};
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
		/* Queue job, and return tool to await the result */
		JobGroup& ForceQueue(Job const& job) {
			impl->ForceQueue(job);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& Queue(std::vector<Job> const& listOfJobs) {
			impl->Queue(listOfJobs);
			return *this;
		};
		/* Queue jobs, and return tool to await the results */
		JobGroup& ForceQueue(std::vector<Job> const& listOfJobs) {
			impl->ForceQueue(listOfJobs);
			return *this;
		};

		/* Await all jobs in this group, and get the return values (which may be empty) for each job */
		std::vector<Any> Wait_Get() {
			impl->Wait();

			typename decltype(JobGroupImpl::jobs) out;
			out.swap(impl->jobs);

			if (out.size() > 0) {
				std::vector<Any> any;
				for (auto& job : out) {
					if (job) {
						any.push_back(job->GetResult());
					}
				}
				return any;
			}
			else {
				return std::vector<Any>();
			}
		};

		/* Await all jobs in this group */
		void Wait() {
			impl->Wait();
			impl->jobs.clear();
		};

	private:
		std::unique_ptr<JobGroupImpl> impl;

	};

	namespace synchronization {
		/* Fiber mutex */
		class mutex {
		public:
			mutex();
			mutex(const mutex& other);
			mutex(mutex&& other);
			mutex& operator=(const mutex& s) { return *this; };
			mutex& operator=(mutex&& s) { return *this; };
			~mutex() = default;

			[[nodiscard]] std::lock_guard<mutex>	guard() noexcept;
			void			lock() noexcept;
			void			unlock() noexcept;
			bool            try_lock() noexcept;

		protected:
			std::shared_ptr<void> Handle;

		};

		/* Read-Write mutex that allows multiple readers and one writer to cooperatively access an underlying object. Very fast for 100% reading operations, as (effectively) no locking actually happens. */
		template <class MutexType = fibers::synchronization::mutex>
		class shared_mutex {
		private:
			MutexType    mut_;
			std::condition_variable_any gate1_;
			std::condition_variable_any gate2_;
			unsigned state_;

			static const unsigned write_entered_ = 1U << (sizeof(unsigned) * CHAR_BIT - 1);
			static const unsigned n_readers_ = ~write_entered_;

		public:
			shared_mutex() : state_(0) {}
			shared_mutex(shared_mutex const&) = default;
			shared_mutex(shared_mutex&&) = default;
			shared_mutex& operator=(shared_mutex const&) = default;
			shared_mutex& operator=(shared_mutex&&) = default;
			~shared_mutex() = default;

			// Exclusive/Writer ownership
			void lock() {
				std::unique_lock<MutexType> lk(mut_);
				while (state_ & write_entered_) gate1_.wait(lk);
				state_ |= write_entered_;
				while (state_ & n_readers_) gate2_.wait(lk);
			};
			// Exclusive/Writer ownership
			bool try_lock() {
				std::unique_lock<MutexType> lk(mut_, std::try_to_lock);
				if (lk.owns_lock() && state_ == 0)
				{
					state_ = write_entered_;
					return true;
				}
				return false;
			};
			// Exclusive/Writer ownership
			void unlock() {
				{
					std::scoped_lock<MutexType> _(mut_);
					state_ = 0;
				}
				gate1_.notify_all();
			};

			// Shared/Reader ownership
			void lock_shared() {
				std::unique_lock<MutexType> lk(mut_);
				while ((state_ & write_entered_) || (state_ & n_readers_) == n_readers_)
					gate1_.wait(lk);
				unsigned num_readers = (state_ & n_readers_) + 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
			};
			// Shared/Reader ownership
			bool try_lock_shared() {
				std::unique_lock<MutexType> lk(mut_, std::try_to_lock);
				unsigned num_readers = state_ & n_readers_;
				if (lk.owns_lock() && !(state_ & write_entered_) && num_readers != n_readers_)
				{
					++num_readers;
					state_ &= ~n_readers_;
					state_ |= num_readers;
					return true;
				}
				return false;
			};
			// Shared/Reader ownership
			void unlock_shared() {
				std::scoped_lock<MutexType> _(mut_);
				unsigned num_readers = (state_ & n_readers_) - 1;
				state_ &= ~n_readers_;
				state_ |= num_readers;
				if (state_ & write_entered_)
				{
					if (num_readers == 0)
						gate2_.notify_one();
				}
				else
				{
					if (num_readers == n_readers_ - 1)
						gate1_.notify_one();
				}
			};

			[[nodiscard]] decltype(auto) Write_Guard() noexcept { return std::lock_guard(*this); };
			[[nodiscard]] decltype(auto) Read_Guard() noexcept { return std::shared_lock(*this); };
		};

		/* Wrapper for non-atomic objects to allow for threads to lock them for exclusive access */
		template< typename type, typename MutexType = fibers::synchronization::mutex>
		class interlocked {
		public:
			class ExclusiveObject {
			public:
				constexpr ExclusiveObject(const interlocked<type>& mut) : owner(const_cast<interlocked<type>&>(mut)) { this->owner.Lock(); };
				~ExclusiveObject() { this->owner.Unlock(); };

				ExclusiveObject() = delete;
				ExclusiveObject(const ExclusiveObject& other) = delete;
				ExclusiveObject(ExclusiveObject&& other) = delete;
				ExclusiveObject& operator=(const ExclusiveObject& other) = delete;
				ExclusiveObject& operator=(ExclusiveObject&& other) = delete;

				type& operator=(const type& a) { data() = a; return data(); };
				type& operator=(type&& a) { data() = a; return data(); };
				type& operator*() const { return data(); };
				type* operator->() const { return &data(); };

			protected:
				type& data() const { return owner.UnsafeRead(); };
				interlocked<type>& owner;
			};

		public: // construction and destruction
			typedef type Type;

			interlocked() : data() {};
			interlocked(const type& other) : data(other) {};
			interlocked(type&& other) : data(std::forward<type>(other)) {};
			interlocked(const interlocked& other) : data() { this->Copy(other); };
			interlocked(interlocked&& other) : data() { this->Copy(std::forward<interlocked>(other)); };
			~interlocked() {};

		public: // copy and clear
			interlocked& operator=(const interlocked& other) {
				this->Copy(other);
				return *this;
			};
			interlocked& operator=(interlocked&& other) {
				this->Copy(std::forward<interlocked<type>>(other));
				return *this;
			};
			void Copy(const interlocked<type>& copy) {
				if (&copy == this) return;
				decltype(auto) g1 = Guard();
				decltype(auto) g2 = copy.Guard();
				data = copy.data;
			};
			void Copy(interlocked<type>&& copy) {
				if (&copy == this) return;
				decltype(auto) g1 = Guard();
				decltype(auto) g2 = copy.Guard();
				data = copy.data;
			};
			void Clear() {
				decltype(auto) g = Guard();
				data = type();
			};

		public: // read and swap
			type Read() const {
				decltype(auto) g = Guard();
				return data;
			};
			void Swap(const type& replacement) {
				decltype(auto) g = Guard();
				data = replacement;
			};
			interlocked& operator=(const type& other) {
				Swap(other);
				return *this;
			};

			type* operator->() const { return &data; };

		public: // lock, unlock, and direct edit
			ExclusiveObject GetExclusive() const { return ExclusiveObject(*this); };

			[[nodiscard]] decltype(auto) Guard() const { return lock.guard(); };
			void Lock() const { lock.lock(); };
			void Unlock() const { lock.unlock(); };
			type& UnsafeRead() const { return data; };

		private:
			mutable type data;
			mutable MutexType lock;

		};
	};
	namespace parallel {
		/* 
		parallel_for (auto i = start; i < end; i++){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end.
		*/
		template<typename iteratorType, typename F>
		decltype(auto) For(iteratorType start, iteratorType end, F&& ToDo) {
			fibers::JobGroup group;

			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;
			//  typename std::tuple_element<0, typename utilities::function_traits<decltype(todo)>::arguments>::type;

			int n = (end - start); 
			std::vector< fibers::Job > jobs(n, fibers::Job());
			n = 0;
			for (iteratorType iter = start; iter < end; iter++) {
				jobs[n++] = fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter);
			}
			group.Queue(jobs);

			if constexpr (retNo) {
				group.Wait();
			}
			else {
				using outputType = typename utilities::function_traits<decltype(todo)>::result_type; 
				std::vector<std::shared_ptr<outputType>> toReturn;
				for (auto& anyO : group.Wait_Get()) toReturn.push_back(anyO.cast<std::shared_ptr<outputType>>());
				return toReturn;
			}
		};

		/* 
		parallel_for (auto i = start; i < end; i += step){ todo(i); }
		If the todo(i) returns anything, it will be collected into a vector at the end.
		*/
		template<typename iteratorType, typename F>
		decltype(auto) For(iteratorType start, iteratorType end, iteratorType step, F&& ToDo) {
			fibers::JobGroup group;

			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			int n = (end - start) / step;
			std::vector< fibers::Job > jobs(n, fibers::Job());
			n = 0;
			for (iteratorType iter = start; iter < end; iter += step) {
				jobs[n++] = fibers::Job([todo](iteratorType const& T) { return todo(T); }, (iteratorType)iter);
			}
			group.Queue(jobs);

			if constexpr (retNo) {
				group.Wait();
			}
			else {
				using outputType = typename utilities::function_traits<decltype(todo)>::result_type;
				std::vector<std::shared_ptr<outputType>> toReturn;
				for (auto& anyO : group.Wait_Get()) toReturn.push_back(anyO.cast<std::shared_ptr<outputType>>());
				return toReturn;
			}
		};

		/* 
		parallel_for (auto i = container.begin(); i != container.end(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. 
		*/
		template<typename containerType, typename F>
		decltype(auto) ForEach(containerType const& container, F&& ToDo) {
			fibers::JobGroup group;

			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			int n = 0;
			for (auto iter = container.begin(); iter != container.end(); iter++) n++;
			std::vector< fibers::Job > jobs(n, fibers::Job());
			n = 0;
			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs[n++] = fibers::Job([todo](typename containerType::const_iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(const_cast<typename containerType::value_type*>(&*T), [](typename containerType::value_type*) {})).cast()); }, (typename containerType::const_iterator)(iter));
			}
			group.Queue(jobs);

			if constexpr (retNo) {
				group.Wait();
			}
			else {
				using outputType = typename utilities::function_traits<decltype(todo)>::result_type;
				std::vector<std::shared_ptr<outputType>> toReturn;
				for (auto& anyO : group.Wait_Get()) toReturn.push_back(anyO.cast<std::shared_ptr<outputType>>());
				return toReturn;
			}
		};

		/* 
		parallel_for (auto i = container.cbegin(); i != container.cend(); i++){ todo(*i); }
		If the todo(*i) returns anything, it will be collected into a vector at the end. 
		*/
		template<typename containerType, typename F>
		decltype(auto) ForEach(containerType& container, F&& ToDo) {
			fibers::JobGroup group;

			decltype(auto) todo = std::function(std::forward<F>(ToDo));
			constexpr bool retNo = std::is_same<typename utilities::function_traits<decltype(todo)>::result_type, void>::value;

			int n = 0;
			for (auto iter = container.begin(); iter != container.end(); iter++) n++;
			std::vector< fibers::Job > jobs(n, fibers::Job());
			n = 0;

			for (auto iter = container.begin(); iter != container.end(); iter++) {
				jobs[n++] = fibers::Job([todo](typename containerType::iterator& T) { return todo(Any(std::shared_ptr<typename containerType::value_type>(&*T, [](typename containerType::value_type*) {})).cast()); }, (typename containerType::iterator)(iter));
			}
			group.Queue(jobs);

			if constexpr (retNo) {
				group.Wait();
			}
			else {
				using outputType = typename utilities::function_traits<decltype(todo)>::result_type;
				std::vector<std::shared_ptr<outputType>> toReturn;
				for (auto& anyO : group.Wait_Get()) toReturn.push_back(anyO.cast<std::shared_ptr<outputType>>());
				return toReturn;
			}
		};

		/* Generic form of a future<T>, which can be used to wait on and get the results of any job. */
		class promise {
		protected:
			std::shared_ptr< std::atomic<JobGroup*>> shared_state;
			std::shared_ptr<std::atomic<Any*>> result;

		public:
			promise() : shared_state(nullptr), result(nullptr) {};
			promise(Job const& job) :
				shared_state(std::shared_ptr<std::atomic<JobGroup*>>(new std::atomic<JobGroup*>(new JobGroup(job)), [](std::atomic<JobGroup*>* anyP) { if (anyP) { auto* p = anyP->exchange(nullptr); if (p) { delete p; } delete anyP; } })),
				result(std::shared_ptr<std::atomic<Any*>>(new std::atomic<Any*>(), [](std::atomic<Any*>* anyP) { if (anyP) { auto* p = anyP->exchange(nullptr); if (p) { delete p; } delete anyP; } })) {};
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
					auto p = shared_state->exchange(nullptr);
					if (p) {
						Any* p2 = result->exchange(new Any(p->Wait_Get()[0]));
						if (p2) {
							delete p2;
						}
						delete p;
					}
				}
				while (result && !result->load()) {
					YieldProcessor();
				}
			};

			Any get_any() const noexcept {
				if (result) {
					Any* p = result->load();
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
		class future_type {
		public:
			virtual ~future_type() {};
		};

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
					Any* p = result->load();
					if (p) {
						if constexpr (std::is_same<void, T>()) {
							return;
						} else {
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
					Any* p = result->load();
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
					Any* p = result->load();
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
