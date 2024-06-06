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
//#include <boost/lockfree/spsc_queue.hpp>
//#include <boost/thread/barrier.hpp>
//#include <boost/thread.hpp>
#include <iostream>
#include <memory>
#include <atomic>

#define CONST_MAX( x, y )			( (x) > (y) ? (x) : (y) )
namespace fibers {

	namespace utilities {
		namespace HasDefaultConstructor
		{
			template <class... T> struct Friend;
			struct testing_tag;

			// specialisation simply to check if default constructible
			template <class T> struct Friend<T, testing_tag> {
				// sfinae trick has to be nested in the Friend class
				// this candidate will be ignored if X does not have a default constructor
				template <class X, class = decltype(X())>
				static std::true_type test(X*);

				template <class X>
				static std::false_type test(...);

				static constexpr bool value = decltype(test<T>(0))::value;
			};
			template <class T> using has_any_default_constructor = Friend<T, testing_tag>;
			template <class T> constexpr bool HasDefaultConstructor_v() {
				return has_any_default_constructor < T >::value;
			};
		}

		__forceinline static void* Mem_Alloc64(const size_t& size) { if (!size) return nullptr; const size_t paddedSize = (size + 63) & ~63; return ::_aligned_malloc(paddedSize, 64); };
		__forceinline static void* Mem_Alloc16(const size_t& size) { if (!size) return nullptr; const size_t paddedSize = (size + 15) & ~15; return ::_aligned_malloc(paddedSize, 16); };
		__forceinline static void  Mem_Free64(void* ptr) { if (ptr) ::_aligned_free(ptr); };
		__forceinline static void  Mem_Free16(void* ptr) { if (ptr) ::_aligned_free(ptr); };
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

		class Hardware {
		public:
			static int GetNumCpuCores();
			static float GetPercentCpuLoad();
		private:
			enum e_LOGICAL_PROCESSOR_RELATIONSHIP_LOCAL {
				e_localRelationProcessorCore,
				e_localRelationNumaNode,
				e_localRelationCache,
				e_localRelationProcessorPackage
			};
			static __forceinline DWORD CountSetBits(ULONG_PTR bitMask) {
				DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
				DWORD bitSetCount = 0;
				ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

				for (DWORD i = 0; i <= LSHIFT; i++) {
					bitSetCount += ((bitMask & bitTest) ? 1 : 0);
					bitTest /= 2;
				}

				return bitSetCount;
			};
			class cweeCpuInfo_t {
			public:
				cweeCpuInfo_t() : processorPackageCount(0), processorCoreCount(0), logicalProcessorCount(0), numaNodeCount(0), cacheLevel() {};
				int processorPackageCount;
				int processorCoreCount;
				int logicalProcessorCount; // the value we care about -- indicated the number of actual "threads" that can run at once. 
				int numaNodeCount;
				class cacheInfo_t {
				public:
					cacheInfo_t() : count(0), associativity(0), lineSize(0), size(0) {};
					int count;
					int associativity;
					int lineSize;
					int size;
				};
				cacheInfo_t cacheLevel[3];
			};
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

		template<typename... Args> class Union {
		private:
#pragma region IMPLIMENTATION DETAILS
			using byte = unsigned char;
			template<int N> using NthTypeOf = typename std::remove_const<typename std::remove_reference<typename std::tuple_element<N, std::tuple<Args...>>::type>::type>::type;
			static constexpr size_t num_parameters = sizeof...(Args);
			template<int N>	static constexpr size_t SizeOfFirstN() {
				size_t d(0);
				if constexpr (num_parameters >= 1 && N >= 1) {
					d += sizeof(NthTypeOf<0>);
				}
				if constexpr (num_parameters >= 2 && N >= 2) {
					d += sizeof(NthTypeOf<1>);
				}
				if constexpr (num_parameters >= 3 && N >= 3) {
					d += sizeof(NthTypeOf<2>);
				}
				if constexpr (num_parameters >= 4 && N >= 4) {
					d += sizeof(NthTypeOf<3>);
				}
				if constexpr (num_parameters >= 5 && N >= 5) {
					d += sizeof(NthTypeOf<4>);
				}
				if constexpr (num_parameters >= 6 && N >= 6) {
					d += sizeof(NthTypeOf<5>);
				}
				if constexpr (num_parameters >= 7 && N >= 7) {
					d += sizeof(NthTypeOf<6>);
				}
				if constexpr (num_parameters >= 8 && N >= 8) {
					d += sizeof(NthTypeOf<7>);
				}
				if constexpr (num_parameters >= 9 && N >= 9) {
					d += sizeof(NthTypeOf<8>);
				}
				if constexpr (num_parameters >= 10 && N >= 10) {
					d += sizeof(NthTypeOf<9>);
				}
				if constexpr (num_parameters >= 11 && N >= 11) {
					d += sizeof(NthTypeOf<10>);
				}
				if constexpr (num_parameters >= 12 && N >= 12) {
					d += sizeof(NthTypeOf<11>);
				}
				if constexpr (num_parameters >= 13 && N >= 13) {
					d += sizeof(NthTypeOf<12>);
				}
				if constexpr (num_parameters >= 13 && N >= 14) {
					d += sizeof(NthTypeOf<13>);
				}
				if constexpr (num_parameters >= 14 && N >= 15) {
					d += sizeof(NthTypeOf<14>);
				}
				if constexpr (num_parameters >= 15 && N >= 16) {
					d += sizeof(NthTypeOf<15>);
				}
				return d;
			};
			static constexpr size_t SizeOfAll() {
				return SizeOfFirstN<num_parameters>();
			};
			static constexpr size_t sizeOfArgs = SizeOfAll();
			static constexpr size_t bitOffset_0 = SizeOfFirstN<0>();
			static constexpr size_t bitOffset_1 = SizeOfFirstN<1>();
			static constexpr size_t bitOffset_2 = SizeOfFirstN<2>();
			static constexpr size_t bitOffset_3 = SizeOfFirstN<3>();
			static constexpr size_t bitOffset_4 = SizeOfFirstN<4>();
			static constexpr size_t bitOffset_5 = SizeOfFirstN<5>();
			static constexpr size_t bitOffset_6 = SizeOfFirstN<6>();
			static constexpr size_t bitOffset_7 = SizeOfFirstN<7>();
			static constexpr size_t bitOffset_8 = SizeOfFirstN<8>();
			static constexpr size_t bitOffset_9 = SizeOfFirstN<9>();
			static constexpr size_t bitOffset_10 = SizeOfFirstN<10>();
			static constexpr size_t bitOffset_11 = SizeOfFirstN<11>();
			static constexpr size_t bitOffset_12 = SizeOfFirstN<12>();
			static constexpr size_t bitOffset_13 = SizeOfFirstN<13>();
			static constexpr size_t bitOffset_14 = SizeOfFirstN<14>();
			static constexpr size_t bitOffset_15 = SizeOfFirstN<15>();

#pragma endregion
		public:
#pragma region INTENDED PUBLIC FUCNTIONS AND USES
			/* ALLOC THE UNIONED DATA IN STACK */
			Union() noexcept : data{ 0 } { Alloc(&data[0]); };
			/* ALLOC THE UNIONED DATA FROM PARAMETERS */ template <typename T, typename = std::enable_if_t<!std::is_same_v<typename std::remove_reference<T>::type, Union>>, typename... TArgs>
			explicit Union(T&& b, TArgs&&... a) noexcept : data{ 0 } {
				InitAndSet(std::forward<T>(b), std::forward<TArgs>(a)...);
			};

			/* INIT AND COPY THE UNIONED DATA FROM ANOTHER UNION */
			Union(Union const& a) noexcept : data{ 0 } { InitAndCopy(a); };
			/* INIT AND TAKE THE UNIONED DATA FROM ANOTHER UNION */
			Union(Union&& a) noexcept : data{ 0 } { InitAndTake(std::forward<Union>(a)); };
			/* COPY THE UNIONED DATA FROM ANOTHER UNION */
			Union& operator=(Union const& a) { Copy(a); return *this; };
			/* TAKE THE UNIONED DATA FROM ANOTHER UNION */
			Union& operator=(Union&& a) { Take(std::forward<Union>(a)); return *this; };
			/* DESTROY THE UNION ONCE OUT-OF-SCOPE */
			~Union() { Delete(); };

			/* GET A REFERENCE TO THE N'th ITEM */  template<int N>
			constexpr NthTypeOf<N>& get() const noexcept {
				static_assert(N < num_parameters&& N >= 0, "Cannot access parameters beyond the allocated buffer.");
				constexpr size_t index = SizeOfFirstN<N>();
				return *static_cast<NthTypeOf<N>*>(static_cast<void*>(&data[index]));

				//constexpr NthTypeOf<N>* out = static_cast<NthTypeOf<N>*>(static_cast<void*>((static_cast<byte*>(&const_cast<byte&>(data[SizeOfFirstN<N>()])))));
				//return *out;
			};
			/* GET THE SIZE (in bytes) OF THE ENTIRE UNION */
			static constexpr size_t size() noexcept { return SizeOfAll(); };
			/* GET THE SIZE (in bytes) OF THE N'th ITEM */ template<int N>
			static constexpr size_t size() noexcept { return sizeof(NthTypeOf<N>); };

			/* COMPARE WITH ANOTHER UNION */
			friend bool operator==(Union const& a, Union const& b) {
				return Equals(a, b);
			};
			friend bool operator!=(Union const& a, Union const& b) {
				return !operator==(a, b);
			};
#pragma endregion
		private:
#pragma region DATA ARRAY (BYTES)
			mutable byte data[sizeOfArgs];
#pragma endregion
		private:
#pragma region STATIC UTILITY FUNCTIONS
			template<typename _type_> static bool is_empty(_type_* d, size_t size) { byte* buf = static_cast<byte*>(static_cast<void*>(d)); return buf[0] == 0 && 0 == ::memcmp(buf, buf + 1, size - 1); };
			template<typename _type_> static constexpr bool isPod() { return std::is_pod<_type_>::value; };
			template<typename _type_> static void InstantiateData(_type_* ptr) {
				if constexpr (isPod<_type_>()) {
					/* already cleared during instantiation */
				}
				else {
					if constexpr (HasDefaultConstructor::HasDefaultConstructor_v<_type_>()) {
						new (&ptr[0]) _type_;
					}
				}
			};
			template<typename _type_> static void InstantiateData(_type_* ptr, _type_&& srce) {
				if constexpr (isPod<_type_>()) {
					*ptr = std::forward<_type_>(srce);
				}
				else {
					if constexpr (std::is_move_constructible<_type_>::value) {
						new (&ptr[0]) _type_(std::forward<_type_>(srce));
					}
					else if constexpr (std::is_copy_constructible<_type_>::value) {
						new (&ptr[0]) _type_(srce);
					}
				}
			};
			template<typename _type_> static void InstantiateData(_type_* ptr, _type_ const& srce) {
				if constexpr (isPod<_type_>()) {
					*ptr = srce;
				}
				else {
					if constexpr (std::is_copy_constructible<_type_>::value) {
						new (&ptr[0]) _type_(srce);
					}
				}
			};
			template<typename _type_, typename _type2_, typename = std::enable_if_t<!std::is_same_v<_type2_, _type_>>>
			static void InstantiateData(_type_* ptr, _type2_&& srce) {
				if constexpr (isPod<_type_>()) {
					*ptr = std::forward<_type2_>(srce);
				}
				else {
					new (&ptr[0]) _type_(std::forward<_type2_>(srce));
				}
			};
			template<typename _type_, typename _type2_, typename = std::enable_if_t<!std::is_same_v<_type2_, _type_>>>
			static void InstantiateData(_type_* ptr, _type2_ const& srce) {
				if constexpr (isPod<_type_>()) {
					*ptr = srce;
				}
				else {
					new (&ptr[0]) _type_(srce);
				}
			};
			template<typename _type_> static void DestroyData(_type_* ptr) {
				if constexpr (isPod<_type_>()) { /* does not require clearing */ }
				else {
					if (!is_empty(ptr, sizeof(_type_))) {
						ptr[0].~_type_();
					}
				}
			};
			template <int N> static NthTypeOf<N>* PtrAt(byte* data) { return static_cast<NthTypeOf<N>*>(static_cast<void*>(&data[SizeOfFirstN<N>()])); };
			static void Alloc(byte* data) {
				if constexpr (num_parameters >= 1) { InstantiateData(PtrAt<0>(data)); }
				if constexpr (num_parameters >= 2) { InstantiateData(PtrAt<1>(data)); }
				if constexpr (num_parameters >= 3) { InstantiateData(PtrAt<2>(data)); }
				if constexpr (num_parameters >= 4) { InstantiateData(PtrAt<3>(data)); }
				if constexpr (num_parameters >= 5) { InstantiateData(PtrAt<4>(data)); }
				if constexpr (num_parameters >= 6) { InstantiateData(PtrAt<5>(data)); }
				if constexpr (num_parameters >= 7) { InstantiateData(PtrAt<6>(data)); }
				if constexpr (num_parameters >= 8) { InstantiateData(PtrAt<7>(data)); }
				if constexpr (num_parameters >= 9) { InstantiateData(PtrAt<8>(data)); }
				if constexpr (num_parameters >= 10) { InstantiateData(PtrAt<9>(data)); }
				if constexpr (num_parameters >= 11) { InstantiateData(PtrAt<10>(data)); }
				if constexpr (num_parameters >= 12) { InstantiateData(PtrAt<11>(data)); }
				if constexpr (num_parameters >= 13) { InstantiateData(PtrAt<12>(data)); }
				if constexpr (num_parameters >= 14) { InstantiateData(PtrAt<13>(data)); }
				if constexpr (num_parameters >= 15) { InstantiateData(PtrAt<14>(data)); }
				if constexpr (num_parameters >= 16) { InstantiateData(PtrAt<15>(data)); }
			};
#pragma endregion
		private:
#pragma region UTILITY FUNCTIONS
			template <int N> bool ElementIsZero() { return is_empty(PtrAt<N>(&data[0]), sizeof(NthTypeOf<N>)); };
			bool IsAllZero() { return is_empty(&data[0], sizeOfArgs); };
			void Clear() { ::memset(&data[0], 0, sizeOfArgs); };
			template <int N> void Clear() { ::memset(&get<N>(), 0, sizeof(NthTypeOf<N>)); };
			void Delete() {
				if (IsAllZero()) return; // sign that this item was not initialized, is all POD, or was recently "taken" over
				if constexpr (num_parameters >= 1) { DestroyData(PtrAt<0>(data)); }
				if constexpr (num_parameters >= 2) { DestroyData(PtrAt<1>(data)); }
				if constexpr (num_parameters >= 3) { DestroyData(PtrAt<2>(data)); }
				if constexpr (num_parameters >= 4) { DestroyData(PtrAt<3>(data)); }
				if constexpr (num_parameters >= 5) { DestroyData(PtrAt<4>(data)); }
				if constexpr (num_parameters >= 6) { DestroyData(PtrAt<5>(data)); }
				if constexpr (num_parameters >= 7) { DestroyData(PtrAt<6>(data)); }
				if constexpr (num_parameters >= 8) { DestroyData(PtrAt<7>(data)); }
				if constexpr (num_parameters >= 9) { DestroyData(PtrAt<8>(data)); }
				if constexpr (num_parameters >= 10) { DestroyData(PtrAt<9>(data)); }
				if constexpr (num_parameters >= 11) { DestroyData(PtrAt<10>(data)); }
				if constexpr (num_parameters >= 12) { DestroyData(PtrAt<11>(data)); }
				if constexpr (num_parameters >= 13) { DestroyData(PtrAt<12>(data)); }
				if constexpr (num_parameters >= 14) { DestroyData(PtrAt<13>(data)); }
				if constexpr (num_parameters >= 15) { DestroyData(PtrAt<14>(data)); }
				if constexpr (num_parameters >= 16) { DestroyData(PtrAt<15>(data)); }
				Clear();
			};

#pragma endregion
		private:
#pragma region SET WITH PARAMETER ARGS
			/* INIT DATA USING PARAMETER */ template <int N> void InitAndSetAt(NthTypeOf<N> const& a) {
				InstantiateData(PtrAt<N>(&data[0]), a);
			};
			/* INIT DATA USING PARAMETER */ template <int N> void InitAndSetAt(NthTypeOf<N>&& a) {
				InstantiateData(PtrAt<N>(&data[0]), std::forward<NthTypeOf<N>>(a));
			};
			/* INIT DATA USING PARAMETER */ template <int N, typename T, typename = std::enable_if_t<!std::is_same_v<T, NthTypeOf<N>>>> void InitAndSetAt(T&& a) {
				InstantiateData(PtrAt<N>(&data[0]), std::forward<T>(a));
			};
			/* EMPTY PARAMETER PACK -> END RECURSION */ void InitAndSetDataWith() { return; };
			/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void InitAndSetDataWith(const T& value, Targs&&... Fargs) {
				InitAndSetAt<num_parameters - (1 + sizeof...(Fargs))>(value);
				InitAndSetDataWith(std::forward<Targs>(Fargs)...);
			};
			/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void InitAndSetDataWith(T&& value, Targs&&... Fargs) {
				InitAndSetAt<num_parameters - (1 + sizeof...(Fargs))>(std::forward<T>(value));
				InitAndSetDataWith(std::forward<Targs>(Fargs)...);
			};
			/* SET WITH PARAMETER PACK */ template <typename... TArgs> void InitAndSet(TArgs const&... a) {
				static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
				InitAndSetDataWith(a...);
			};
			/* SET WITH PARAMETER PACK */ template <typename... TArgs> void InitAndSet(TArgs&&... a) {
				static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
				InitAndSetDataWith(std::forward<TArgs>(a)...);
			};

			/* SET DATA USING PARAMETER */ template <int N> void SetAt(NthTypeOf<N> const& a) { this->get<N>() = a; };
			/* SET DATA USING PARAMETER */ template <int N> void SetAt(NthTypeOf<N>&& a) { this->get<N>() = std::forward<NthTypeOf<N>>(a); };
			/* EMPTY PARAMETER PACK -> END RECURSION */ void SetDataWith() { return; };
			/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void SetDataWith(const T& value, Targs&&... Fargs) { SetAt<num_parameters - (1 + sizeof...(Fargs))>(value); SetDataWith(std::forward<Targs>(Fargs)...); };
			/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void SetDataWith(T&& value, Targs&&... Fargs) { SetAt<num_parameters - (1 + sizeof...(Fargs))>(std::forward<T>(value)); SetDataWith(std::forward<Targs>(Fargs)...); };
			/* SET WITH PARAMETER PACK */ template <typename... TArgs> void Set(TArgs const&... a) {
				static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
				SetDataWith(a...);
			};
			/* SET WITH PARAMETER PACK */ template <typename... TArgs> void Set(TArgs&&... a) {
				static_assert((sizeof...(TArgs)) == num_parameters, "Union initializer must use the same number of parametrers as are defined in the Union.");
				SetDataWith(std::forward<TArgs>(a)...);
			};
#pragma endregion
#pragma region COMPARE WITH CONST&
			template <int N> static bool EqualsAt(Union const& a, Union const& b) {
				return a.get<N>() == b.get<N>();
			};
			static bool Equals(Union const& a, Union const& b) {
				bool out = true;
				if constexpr (num_parameters >= 1) { out = out && EqualsAt<0>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 2) { out = out && EqualsAt<1>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 3) { out = out && EqualsAt<2>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 4) { out = out && EqualsAt<3>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 5) { out = out && EqualsAt<4>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 6) { out = out && EqualsAt<5>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 7) { out = out && EqualsAt<6>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 8) { out = out && EqualsAt<7>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 9) { out = out && EqualsAt<8>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 10) { out = out && EqualsAt<9>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 11) { out = out && EqualsAt<10>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 12) { out = out && EqualsAt<11>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 13) { out = out && EqualsAt<12>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 14) { out = out && EqualsAt<13>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 15) { out = out && EqualsAt<14>(a, b); if (!out) return out; }
				if constexpr (num_parameters >= 16) { out = out && EqualsAt<15>(a, b); if (!out) return out; }
				return out;
			};
#pragma endregion
#pragma region COPY FROM CONST&
			template <int N> void InitAndCopyAt(Union const& a) { InstantiateData(PtrAt<N>(&data[0]), a.get<N>()); };
			void InitAndCopy(Union const& a) {
				if constexpr (num_parameters >= 1) { InitAndCopyAt<0>(a); }
				if constexpr (num_parameters >= 2) { InitAndCopyAt<1>(a); }
				if constexpr (num_parameters >= 3) { InitAndCopyAt<2>(a); }
				if constexpr (num_parameters >= 4) { InitAndCopyAt<3>(a); }
				if constexpr (num_parameters >= 5) { InitAndCopyAt<4>(a); }
				if constexpr (num_parameters >= 6) { InitAndCopyAt<5>(a); }
				if constexpr (num_parameters >= 7) { InitAndCopyAt<6>(a); }
				if constexpr (num_parameters >= 8) { InitAndCopyAt<7>(a); }
				if constexpr (num_parameters >= 9) { InitAndCopyAt<8>(a); }
				if constexpr (num_parameters >= 10) { InitAndCopyAt<9>(a); }
				if constexpr (num_parameters >= 11) { InitAndCopyAt<10>(a); }
				if constexpr (num_parameters >= 12) { InitAndCopyAt<11>(a); }
				if constexpr (num_parameters >= 13) { InitAndCopyAt<12>(a); }
				if constexpr (num_parameters >= 14) { InitAndCopyAt<13>(a); }
				if constexpr (num_parameters >= 15) { InitAndCopyAt<14>(a); }
				if constexpr (num_parameters >= 16) { InitAndCopyAt<15>(a); }
			};

			template <int N> void CopyAt(Union const& a) { this->get<N>() = a.get<N>(); };
			void Copy(Union const& a) {
				if constexpr (num_parameters >= 1) { CopyAt<0>(a); }
				if constexpr (num_parameters >= 2) { CopyAt<1>(a); }
				if constexpr (num_parameters >= 3) { CopyAt<2>(a); }
				if constexpr (num_parameters >= 4) { CopyAt<3>(a); }
				if constexpr (num_parameters >= 5) { CopyAt<4>(a); }
				if constexpr (num_parameters >= 6) { CopyAt<5>(a); }
				if constexpr (num_parameters >= 7) { CopyAt<6>(a); }
				if constexpr (num_parameters >= 8) { CopyAt<7>(a); }
				if constexpr (num_parameters >= 9) { CopyAt<8>(a); }
				if constexpr (num_parameters >= 10) { CopyAt<9>(a); }
				if constexpr (num_parameters >= 11) { CopyAt<10>(a); }
				if constexpr (num_parameters >= 12) { CopyAt<11>(a); }
				if constexpr (num_parameters >= 13) { CopyAt<12>(a); }
				if constexpr (num_parameters >= 14) { CopyAt<13>(a); }
				if constexpr (num_parameters >= 15) { CopyAt<14>(a); }
				if constexpr (num_parameters >= 16) { CopyAt<15>(a); }
			};
#pragma endregion
#pragma region TAKE FROM &&
			template <int N> void InitAndTakeAt(Union&& a) {
				InstantiateData(PtrAt<N>(&data[0]), std::move(a.get<N>()));
				if constexpr (!isPod<NthTypeOf<N>>()) { a.Clear<N>(); }
			};
			void InitAndTake(Union&& a) {
				if constexpr (num_parameters >= 1) { InitAndTakeAt<0>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 2) { InitAndTakeAt<1>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 3) { InitAndTakeAt<2>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 4) { InitAndTakeAt<3>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 5) { InitAndTakeAt<4>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 6) { InitAndTakeAt<5>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 7) { InitAndTakeAt<6>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 8) { InitAndTakeAt<7>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 9) { InitAndTakeAt<8>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 10) { InitAndTakeAt<9>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 11) { InitAndTakeAt<10>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 12) { InitAndTakeAt<11>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 13) { InitAndTakeAt<12>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 14) { InitAndTakeAt<13>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 15) { InitAndTakeAt<14>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 16) { InitAndTakeAt<15>(std::forward<Union>(a)); }
				a.Clear();
			};

			template <int N> void TakeAt(Union&& a) {
				get<N>() = std::move(a.get<N>());
				if constexpr (!isPod<NthTypeOf<N>>()) { a.Clear<N>(); }
			};
			void Take(Union&& a) {
				if constexpr (num_parameters >= 1) { TakeAt<0>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 2) { TakeAt<1>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 3) { TakeAt<2>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 4) { TakeAt<3>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 5) { TakeAt<4>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 6) { TakeAt<5>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 7) { TakeAt<6>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 8) { TakeAt<7>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 9) { TakeAt<8>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 10) { TakeAt<9>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 11) { TakeAt<10>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 12) { TakeAt<11>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 13) { TakeAt<12>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 14) { TakeAt<13>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 15) { TakeAt<14>(std::forward<Union>(a)); }
				if constexpr (num_parameters >= 16) { TakeAt<15>(std::forward<Union>(a)); }
				a.Clear();
			};
#pragma endregion
		};

#if 1
		// SEE: https://github.com/microsoft/pmwcas/blob/master/src/mwcas/mwcas.h
		// REF: https://www.cl.cam.ac.uk/research/srg/netos/papers/2002-casn.pdf
		// REF: https://paulcavallaro.com/blog/the-bw-tree/
		// REF: https://paulcavallaro.com/blog/a-practical-multi-word-compare-and-swap-operation/
		// REF: https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf
		// REF: https://db.in.tum.de/~leis/papers/artsync.pdf
		// REF: https://github.com/wangziqi2013/BwTree/tree/master
		// UNRELATED, CHECK OUT https://github.com/BRL-CAD/brlcad
		// UNRELATED, CHECK OUT https://en.wikipedia.org/wiki/SolveSpace

#define RETURN_NOT_OK(s) do { auto _s = (s); if (!_s.ok()) return _s; } while (0);
#define IS_POWER_OF_TWO(x) (x && (x & (x - 1)) == 0)

		namespace garbage_collection {

			class Slice {
			public:
				/// Create an empty slice.
				Slice() : data_(""), size_(0) { }

				/// Create a slice that refers to d[0,n-1].
				Slice(const char* d, size_t n) : data_(d), size_(n) { }

				/// Create a slice that refers to the contents of "s"
				Slice(const std::string& s) : data_(s.data()), size_(s.size()) { }

				/// Create a slice that refers to s[0,strlen(s)-1]
				Slice(const char* s) : data_(s), size_(strlen(s)) { }

				/// Create a single slice from SliceParts using buf as storage.
				/// buf must exist as long as the returned Slice exists.
				Slice(const struct SliceParts& parts, std::string* buf);

				/// Return a pointer to the beginning of the referenced data
				const char* data() const {
					return data_;
				}

				/// Return the length (in bytes) of the referenced data
				size_t size() const {
					return size_;
				}

				/// Return true iff the length of the referenced data is zero
				bool empty() const {
					return size_ == 0;
				}

				/// Return the ith byte in the referenced data.
				/// REQUIRES: n < size()
				char operator[](size_t n) const {
					assert(n < size());
					return data_[n];
				}

				/// Change this slice to refer to an empty array
				void clear() {
					data_ = "";
					size_ = 0;
				}

				/// Drop the first "n" bytes from this slice.
				void remove_prefix(size_t n) {
					assert(n <= size());
					data_ += n;
					size_ -= n;
				}

				/// Drop the last "n" bytes from this slice.
				void remove_suffix(size_t n) {
					assert(n <= size());
					size_ -= n;
				}

				/// Return a string that contains the copy of the referenced data.
				std::string ToString(bool hex = false) const {
					return data();
				};

				/// Copies the slice to the specified destination.
				void copy(char* dest, size_t dest_size) const {
					assert(dest_size >= size_);
					memcpy(dest, data_, size_);
				}

				/// Copies the specified number of bytes of the slice to the specified
				/// destination.
				void copy(char* dest, size_t dest_size, size_t count) const {
					assert(count <= size_);
					assert(dest_size >= count);
					memcpy(dest, data_, count);
				}

				/// Three-way comparison.  Returns value:
				///   <  0 iff "*this" <  "b",
				///   == 0 iff "*this" == "b",
				///   >  0 iff "*this" >  "b"
				int compare(const Slice& b) const {
					const size_t min_len = (std::min)(size_, b.size_);
					int r = memcmp(data_, b.data_, min_len);
					if (r == 0) {
						if (size_ < b.size_) r = -1;
						else if (size_ > b.size_) r = +1;
					}
					return r;
				};

				/// Besides returning the comparison value, also sets out parameter "index" to
				/// be the least index
				/// such that "this[index]" != "b[index]".
				int compare_with_index(const Slice& b, size_t& index) const {
					const size_t min_len = (std::min)(size_, b.size_);
					int r = memcmp_with_index(data_, b.data_, min_len, index);
					if (r == 0) {
						if (size_ < b.size_) r = -1;
						else if (size_ > b.size_) r = +1;
					}
					return r;
				};

				/// Compares the first "len" bytes of "this" with the first "len" bytes of "b".
				int compare(const Slice& b, size_t len) const {
					const size_t min_len = (std::min)(size_, b.size_);
					int r = memcmp(data_, b.data_, (std::min)(min_len, len));
					if (r == 0 && min_len < len) {
						if (size_ < b.size_) r = -1;
						else if (size_ > b.size_) r = +1;
					}
					return r;
				};

				/// Besides returning the comparison value, also sets out parameter "index" to
				/// be the least index such that "this[index]" != "b[index]".
				int compare_with_index(const Slice& b, size_t len, size_t& index) const {
					const size_t min_len = (std::min)(size_, b.size_);
					int r = memcmp_with_index(data_, b.data_, (std::min)(min_len, len), index);
					if (r == 0 && min_len < len) {
						if (size_ < b.size_) r = -1;
						else if (size_ > b.size_) r = +1;
					}
					return r;
				};

				/// Return true iff "x" is a prefix of "*this"
				bool starts_with(const Slice& x) const {
					return ((size_ >= x.size_) &&
						(memcmp(data_, x.data_, x.size_) == 0));
				}

				/// Performs a memcmp() and also sets "index" to the index of the first
				/// disagreement (if any) between the buffers being compared.
				static int memcmp_with_index(const void* buf1, const void* buf2, size_t size,
					size_t& index) {
					// Currently using naive implementation, since this is called only during
					// consolidate/split.
					for (index = 0; index < size; ++index) {
						uint8_t byte1 = reinterpret_cast<const uint8_t*>(buf1)[index];
						uint8_t byte2 = reinterpret_cast<const uint8_t*>(buf2)[index];
						if (byte1 != byte2) {
							return (byte1 < byte2) ? -1 : +1;
						}
					}
					return 0;
				}

				friend bool		operator==(const Slice& x, const Slice& y) {
					return ((x.size() == y.size()) && (memcmp(x.data(), y.data(), x.size()) == 0));
				};
				friend bool		operator!=(const Slice& x, const Slice& y) {
					return !(x == y);
				};



			public:
				const char* data_;
				size_t size_;
			};
			class Status {
			
			public:
				enum Code {
					kOk = 0,
					kNotFound = 1,
					kCorruption = 2,
					kNotSupported = 3,
					kInvalidArgument = 4,
					kIOError = 5,
					kMergeInProgress = 6,
					kIncomplete = 7,
					kShutdownInProgress = 8,
					kTimedOut = 9,
					kAborted = 10,
					kBusy = 11,
					kOutOfMemory = 12,
					kKeyAlreadyExists = 13,
					kUnableToMerge = 14,
					kMwCASFailure = 15,
				};
				explicit Status(Code _code) : code_(_code), state_(nullptr) {}

			private:
				static const char* CopyState(const char* s) {
					return fibers::utilities::Mem_CopyString(s);
				};
				Status(Code _code, const Slice& msg, const Slice& msg2) : code_(_code), state_(nullptr) {
					std::string Str = msg.ToString() + ": " + msg2.ToString();
					state_ = CopyState(Str.c_str());
				};

			public:
				// Create a success status.
				Status() : code_(kOk), state_(nullptr) { }
				~Status() {
					delete[] state_;
				}

				// Copy the specified status.
				Status(const Status& s) {
					code_ = s.code_;
					state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
				};
				void operator=(const Status& s) {
					// The following condition catches both aliasing (when this == &s),
					// and the common case where both s and *this are ok.
					code_ = s.code_;
					if (state_ != s.state_) {
						delete[] state_;
						state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
					}
				};
				bool operator==(const Status& rhs) const {
					return (code_ == rhs.code_);
				};
				bool operator!=(const Status& rhs) const {
					return !(*this == rhs);
				};

				// Return a success status.
				static Status OK() {
					return Status();
				}

				// Return error status of an appropriate type.
				static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kNotFound, msg, msg2);
				}
				// Fast path for not found without malloc;
				static Status NotFound() {
					return Status(kNotFound);
				}
				static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kCorruption, msg, msg2);
				}
				static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kNotSupported, msg, msg2);
				}
				static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kInvalidArgument, msg, msg2);
				}
				static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kIOError, msg, msg2);
				}
				static Status MergeInProgress() {
					return Status(kMergeInProgress);
				}
				static Status UnableToMerge() {
					return Status(kUnableToMerge);
				}
				static Status Incomplete(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kIncomplete, msg, msg2);
				}
				static Status ShutdownInProgress() {
					return Status(kShutdownInProgress);
				}
				static Status ShutdownInProgress(const Slice& msg,
					const Slice& msg2 = Slice()) {
					return Status(kShutdownInProgress, msg, msg2);
				}
				static Status Aborted() {
					return Status(kAborted);
				}
				static Status Aborted(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kAborted, msg, msg2);
				}
				static Status Busy() {
					return Status(kBusy);
				}
				static Status Busy(const Slice& msg, const Slice& msg2 = Slice()) {
					return Status(kBusy, msg, msg2);
				}
				static Status OutOfMemory() {
					return Status(kOutOfMemory);
				}
				static Status TimedOut() {
					return Status(kTimedOut);
				}
				static Status KeyAlreadyExists() {
					return Status(kKeyAlreadyExists);
				}
				static Status MwCASFailure() {
					return Status(kMwCASFailure);
				}

				// Returns true iff the status indicates success.
				bool ok() const {
					return code() == kOk;
				}

				// Returns true iff the status indicates a NotFound error.
				bool IsNotFound() const {
					return code() == kNotFound;
				}

				// Returns true iff the status indicates a Corruption error.
				bool IsCorruption() const {
					return code() == kCorruption;
				}

				// Returns true iff the status indicates a NotSupported error.
				bool IsNotSupported() const {
					return code() == kNotSupported;
				}

				// Returns true iff the status indicates an InvalidArgument error.
				bool IsInvalidArgument() const {
					return code() == kInvalidArgument;
				}

				// Returns true iff the status indicates an IOError.
				bool IsIOError() const {
					return code() == kIOError;
				}

				// Returns true iff the status indicates Incomplete
				bool IsIncomplete() const {
					return code() == kIncomplete;
				}

				// Returns true iff the status indicates Shutdown In progress
				bool IsShutdownInProgress() const {
					return code() == kShutdownInProgress;
				}

				bool IsTimedOut() const {
					return code() == kTimedOut;
				}

				bool IsAborted() const {
					return code() == kAborted;
				}

				bool IsOutOfMemory() const {
					return code() == kOutOfMemory;
				}

				bool IsKeyAlreadyExists() const {
					return code() == kKeyAlreadyExists;
				}

				// Returns true iff the status indicates that a resource is Busy and
				// temporarily could not be acquired.
				bool IsBusy() const {
					return code() == kBusy;
				}

				bool IsMwCASFailure() const {
					return code() == kMwCASFailure;
				}

				// Return a string representation of this status suitable for printing.
				// Returns the string "OK" for success.
				std::string ToString() const {
					if (Code::kOk == code_) return "OK";
					if (state_) return state_;
					return "";
				};

				Code code() const {
					return code_;
				}
			private:
				// A nullptr state_ (which is always the case for OK) means the message
				// is empty.
				// of the following form:
				//    state_[0..3] == length of message
				//    state_[4..]  == message
				Code code_;
				const char* state_;
			};

			__forceinline uint32_t Murmur3_32(uint32_t h) {
				h ^= h >> 16;
				h *= 0x85ebca6b;
				h ^= h >> 13;
				h *= 0xc2b2ae35;
				h ^= h >> 16;
				return h;
			};
			__forceinline uint64_t Murmur3_64(uint64_t h) {
				h ^= h >> 33;
				h *= 0xff51afd7ed558ccd;
				h ^= h >> 33;
				h *= 0xc4ceb9fe1a85ec53;
				h ^= h >> 33;
				return h;
			};

			const static uintptr_t kAtomicAlignment = 4;

			// The minimum alignment to guarantee atomic operations on a platform for both value types and pointer types
			// The maximum size to guarantee atomic operations with the primitives below
#if defined (_M_I386)
#define AtomicAlignment  4
#define PointerAlignment 4
#define AtomicMaxSize    4
#else
#define AtomicAlignment  4
#define PointerAlignment 8
#define AtomicMaxSize    8
#endif

            // Identical for WIN32 and Linux
			template <typename T> T LdImm(T const* const source) {
				static_assert(sizeof(T) <= AtomicMaxSize, "Type must be aligned to native pointer alignment.");

				// The volatile qualifier has the side effect of being a load-aquire on IA64.
				// That's a result of overloading the volatile keyword.
				return *((volatile T*)source);
			}

			/// A load with acquire semantics for a type T
			template <typename T> inline T LdAq(T const* const source) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");
				assert((((uintptr_t)source) & (AtomicAlignment - 1)) == 0);

#ifdef WIN32
				// COMPILER-WISE:
				// a volatile read is not compiler reorderable w/r to reads. However,
				//  we put in a _ReadBarrier() just to make sure
				// PROCESSOR-WISE:
				// Common consensus is that X86 and X64 do NOT reorder loads.
				// IA64 volatile emits ld.aq,which ensures all loads complete before this one
				_ReadBarrier();
#else
				_mm_lfence();
#endif
				return *((volatile T*)source);
			}

			template <typename T> void StRel(T* destination, T value) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");

				// COMPILER-WISE:
				// A volatile write is not compiler reorderable w/r to writes
				// PROCESSOR-WISE:
				// X86 and X64 do not reorder stores. IA64 volatile emits st.rel,
				//  which ensures all stores complete before this one
				*((volatile T*)destination) = value;
			}

#ifdef WIN32
			template <typename T> T CompareExchange64(T* destination, T new_value,
				T comparand) {
				static_assert(sizeof(T) == 8, "CompareExchange64 only works on 64 bit values");

				return ::InterlockedCompareExchange64(
					reinterpret_cast<LONGLONG*>(destination), new_value, comparand);
			}

			template <typename T>
			T* CompareExchange64Ptr(T** destination, T* new_value, T* comparand) {
				return (T*)(::InterlockedCompareExchangePointer(
					reinterpret_cast<void**>(destination), new_value, comparand));
			}

			template <typename T> T CompareExchange32(T* destination, T new_value,
				T comparand) {
				static_assert(sizeof(T) == 4,
					"CompareExchange32 only works on 32 bit values");

				return ::InterlockedCompareExchange(reinterpret_cast<LONG*>(destination),
					new_value, comparand);
			}

			template <typename T> T FetchAdd64(T* destination, T add_value) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");
				return ::InterlockedAdd(reinterpret_cast<LONG*>(destination), add_value);
			}

			template <typename T> T Decrement64(T* destination) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");
				return ::InterlockedDecrement64(destination);
			}

			template <typename T> T Decrement32(T* destination) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");
				return ::InterlockedDecrement(destination);
			}

			class Barrier {
			public:
				Barrier(uint64_t thread_count)
					: wait_count_{ thread_count }
					, thread_count_{ thread_count }
					, windows_semaphore_{ ::CreateSemaphore(0, 0, 1024, 0) } {
				}

				~Barrier() {
					::CloseHandle(windows_semaphore_);
				}

				void CountAndWait() {
					if (0 == --wait_count_) {
						wait_count_.store(thread_count_, std::memory_order_release);
						::ReleaseSemaphore(windows_semaphore_,
							static_cast<LONG>(thread_count_ - 1), 0);
					}
					else {
						::WaitForSingleObject(windows_semaphore_, INFINITE);
					}
				}

			private:
				std::atomic<uint64_t> wait_count_;
				const uint64_t thread_count_;
				const HANDLE windows_semaphore_;
			};

#else
			template <typename T> T CompareExchange64(T* destination, T new_value,
				T comparand) {
				static_assert(sizeof(T) == 8,
					"CompareExchange64 only works on 64 bit values");
				::__atomic_compare_exchange_n(destination, &comparand, new_value, false,
					__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
				return comparand;
			}

			template <typename T>
			T* CompareExchange64Ptr(T** destination, T* new_value, T* comparand) {
				::__atomic_compare_exchange_n(destination, &comparand, new_value, false,
					__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
				return comparand;
			}

			template <typename T> T CompareExchange32(T* destination, T new_value,
				T comparand) {
				static_assert(sizeof(T) == 4,
					"CompareExchange32 only works on 32 bit values");
				::__atomic_compare_exchange_n(destination, &comparand, new_value, false,
					__ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
				return comparand;
			}

			template <typename T> T FetchAdd64(T* destination, T add_value) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");
				return ::__atomic_fetch_add(destination, add_value, __ATOMIC_SEQ_CST);
			}

			template <typename T> T Decrement64(T* destination) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");
				return ::__atomic_sub_fetch(destination, 1);
			}

			template <typename T> T Decrement32(T* destination) {
				static_assert(sizeof(T) <= AtomicMaxSize,
					"Type must be aligned to native pointer alignment.");
				return ::__atomic_sub_fetch(destination, 1);
			}

			class Barrier {
			public:
				Barrier(uint64_t thread_count)
					: wait_count_{ thread_count } {
				}

				~Barrier() {}

				void CountAndWait() {
					uint64_t c = --wait_count_;
					while (wait_count_ != 0) {}
				}

			private:
				std::atomic<uint64_t> wait_count_;
			};


#endif






			/// A "timestamp" that is used to determine when it is safe to reuse memory in
			/// data structures that are protected with an EpochManager. Epochs are
			/// opaque to threads and data structures that use the EpochManager. They
			/// may receive Epochs from some of the methods, but they never need to
			/// perform any computation on them, other than to pass them back to the
			/// EpochManager on future calls (for example, EpochManager::GetCurrentEpoch()
			/// and EpochManager::IsSafeToReclaim()).
			typedef uint64_t Epoch;
			typedef std::list<std::pair<uint64_t*, uint64_t> > TlsList;

			extern std::unordered_map<std::thread::id, std::shared_ptr<TlsList>>* registry_;
			extern std::mutex* registryMutex_;

			/// Used to ensure that concurrent accesses to data structures don't reuse
			/// memory that some threads may be accessing. Specifically, for many lock-free
			/// data structures items are "unlinked" when they are removed. Unlinked items
			/// cannot be disposed until it is guaranteed that no threads are accessing or
			/// will ever access the memory associated with the item again. EpochManager
			/// makes it easy for data structures to determine if it is safe to reuse
			/// memory by "timestamping" removed items and the entry/exit of threads
			/// into the protected code region.
			///
			/// Practically, a developer "protects" some region of code by marking it
			/// with calls to Protect() and Unprotect(). The developer must guarantee that
			/// no pointers to internal data structure items are retained beyond the
			/// Unprotect() call. Up until Unprotect(), pointers to internal items in
			/// a data structure may remain safe for access (see the specific data
			/// structures that use this class via IsSafeToReclaim() for documentation on
			/// what items are safe to hold pointers to within the protected region).
			///
			/// Data structure developers must "swap" elements out of their structures
			/// atomically and with a sequentially consistent store operation. This ensures
			/// that all threads call Protect() in the future will not see the deleted item.
			/// Afterward, the removed item must be associated with the current Epoch
			/// (acquired via GetCurrentEpoch()). Data structures can use any means to
			/// track the association between the removed item and the Epoch it was
			/// removed during. Such removed elements must be retained and remain safe for
			/// access until IsSafeToReclaim() returns true (which indicates no threads are
			/// accessing or ever will access the item again).
			class EpochManager {
			public:
				EpochManager() : current_epoch_{ 1 } , safe_to_reclaim_epoch_{ 0 } , epoch_table_{ nullptr } { };
				~EpochManager() {
					Uninitialize();
				};

				/**
				* Initialize an uninitialized EpochManager. This method must be used before
				* it is safe to use an instance via any other members. Calling this on an
				* initialized instance has no effect.
				*
				* \retval S_OK Initialization was successful and instance is ready for use.
				* \retval S_FALSE This instance was already initialized; no action was taken.
				* \retval E_OUTOFMEMORY Initialization failed due to lack of heap space, the
				*      instance was left safely in an uninitialized state.
				*/
				Status Initialize() {
					if (epoch_table_) return Status::OK();

					auto new_table = std::make_shared<MinEpochTable>();

					if (new_table == nullptr) return Status::Corruption("Out of memory");
					RETURN_NOT_OK(new_table->Initialize());

					current_epoch_ = 1;
					safe_to_reclaim_epoch_ = 0;
					epoch_table_ = new_table;

					return Status::OK();
				};
				/**
				* Uninitialize an initialized EpochManager. This method must be used before
				* it is safe to destroy or re-initialize an EpochManager. The caller is
				* responsible for ensuring no threads are protected (have started a Protect()
				* without having completed an Unprotect() and that no threads will call
				* Protect()/Unprotect() while the manager is uninitialized; failing to do
				* so results in undefined behavior. Calling Uninitialize() on an uninitialized
				* instance has no effect.
				*
				* \return Success or or may return other error codes indicating a
				*       failure deallocating the thread local storage used by the EpochManager
				*       internally. Even for returns other than success the object is safely
				*       left in an uninitialized state, though some thread local resources may
				*       not have been reclaimed properly.
				* \retval S_OK Success.
				* \retval S_FALSE Success; instance was already uninitialized, so no effect.
				*/
				Status Uninitialize() {
					if (!epoch_table_) return Status::OK();

					Status s = epoch_table_->Uninitialize();

					// Keep going anyway. Even if the inner table fails to completely
					// clean up we want to clean up as much as possible.
					epoch_table_ = nullptr;
					current_epoch_ = 1;
					safe_to_reclaim_epoch_ = 0;

					return s;
				};

				/// Enter the thread into the protected code region, which guarantees
				/// pointer stability for records in client data structures. After this
				/// call, accesses to protected data structure items are guaranteed to be
				/// safe, even if the item is concurrently removed from the structure.
				///
				/// Behavior is undefined if Protect() is called from an already
				/// protected thread. Upon creation, threads are unprotected.
				/// \return S_OK indicates thread may now enter the protected region. Any
				///      other return indicates a fatal problem accessing the thread local
				///      storage; the thread may not enter the protected region. Most likely
				///      the library has entered some non-serviceable state.
				Status Protect() {
					return epoch_table_->Protect(
						current_epoch_.load(std::memory_order_relaxed));
				};

				/// Exit the thread from the protected code region. The thread must
				/// promise not to access pointers to elements in the protected data
				/// structures beyond this call.
				///
				/// Behavior is undefined if Unprotect() is called from an already
				/// unprotected thread.
				/// \return S_OK indicates thread successfully exited protected region. Any
				///      other return indicates a fatal problem accessing the thread local
				///      storage; the thread may not have successfully exited the protected
				///      region. Most likely the library has entered some non-serviceable
				///      state.
				Status Unprotect() {
					return epoch_table_->Unprotect(
						current_epoch_.load(std::memory_order_relaxed));
				};

				/// Get a snapshot of the current global Epoch. This is used by
				/// data structures to fetch an Epoch that is recorded along with
				/// a removed element.
				Epoch GetCurrentEpoch() {
					return current_epoch_.load(std::memory_order_seq_cst);
				};

				/// Returns true if an item tagged with \a epoch (which was returned by
				/// an earlier call to GetCurrentEpoch()) is safe to reclaim and reuse.
				/// If false is returned the caller then others threads may still be
				/// concurrently accessed the object inquired about.
				bool IsSafeToReclaim(Epoch epoch) {
					return epoch <= safe_to_reclaim_epoch_.load(std::memory_order_relaxed);
				};

				/// Returns true if the calling thread is already in the protected code
				/// region (i.e., have already called Protected()).
				bool IsProtected() {
					return epoch_table_->IsProtected();
				};

				/**
				* Increment the current epoch; this should be called "occasionally" to
				* ensure that items removed from client data structures can eventually be
				* removed. Roughly, items removed from data structures cannot be reclaimed
				* until the epoch in which they were removed ends and all threads that may
				* have operated in the protected region during that Epoch have exited the
				* protected region. As a result, the current epoch should be bumped whenever
				* enough items have been removed from data structures that they represent
				* a significant amount of memory. Bumping the epoch unnecessarily may impact
				* performance, since it is an atomic operation and invalidates a read-hot
				* object in the cache of all of the cores.
				*
				* Only called by GarbageList.
				*/
				void BumpCurrentEpoch() {
					Epoch newEpoch = current_epoch_.fetch_add(1, std::memory_order_seq_cst);
					ComputeNewSafeToReclaimEpoch(newEpoch);
				};

			public:
				/**
				* Looks at all of the threads in the protected region and the current
				* Epoch and updates the Epoch that is guaranteed to be safe for
				* reclamation (stored in #m_safeToReclaimEpoch). This must be called
				* occasionally to ensure the system makes garbage collection progress.
				* For now, it's called every time bumpCurrentEpoch() is called, which
				* might work as a reasonable heuristic for when this should be called.
				*/
				void ComputeNewSafeToReclaimEpoch(Epoch currentEpoch) {
					safe_to_reclaim_epoch_.store(
						epoch_table_->ComputeNewSafeToReclaimEpoch(currentEpoch),
						std::memory_order_release);
				};

				/// Keeps track of which threads are executing in region protected by
				/// its parent EpochManager. This table does most of the work of the
				/// EpochManager. It allocates a slot in thread local storage. When
				/// threads enter the protected region for the first time it assigns
				/// the thread a slot in the table and stores its address in thread
				/// local storage. On Protect() and Unprotect() by a thread it updates
				/// the table entry that tracks whether the thread is currently operating
				/// in the protected region, and, if so, a conservative estimate of how
				/// early it might have entered.
				class MinEpochTable {
				public:

					/// Entries should be exactly cacheline sized to prevent contention
					/// between threads.
					enum { CACHELINE_SIZE = 64 };

					/// Default number of entries managed by the MinEpochTable
					static const uint64_t kDefaultSize = 128;

					MinEpochTable() : table_{ nullptr }, size_{} { };
					/**
					* Initialize an uninitialized table. This method must be used before
					* it is safe to use an instance via any other members. Calling this on an
					* initialized instance has no effect.
					*
					* \param size The initial number of distinct threads to support calling
					*       Protect()/Unprotect(). This must be a power of two. If the table runs
					*       out of space to track threads, then calls may stall. Internally, the
					*       table may allocate additional tables to solve this, or it may reclaim
					*       entries in the table after a long idle periods of by some threads.
					*       If this number is too large it may slow down threads performing
					*       space reclamation, since this table must be scanned occasionally to
					*       make progress.
					* TODO(stutsman) Table growing and entry reclamation are not yet implemented.
					* Currently, the manager supports precisely size distinct threads over the
					* lifetime of the manager until it begins permanently spinning in all calls to
					* Protect().
					*
					* \retval S_OK Initialization was successful and instance is ready for use.
					* \retval S_FALSE Instance was already initialized; instance is ready for use.
					* \retval E_INVALIDARG \a size was not a power of two.
					* \retval E_OUTOFMEMORY Initialization failed due to lack of heap space, the
					*       instance was left safely in an uninitialized state.
					* \retval HRESULT_FROM_WIN32(TLS_OUT_OF_INDEXES) Initialization failed because
					*       TlsAlloc() failed; the table was safely left in an uninitialized state.
					*/
					Status Initialize(uint64_t size = MinEpochTable::kDefaultSize) {
						if (table_) return Status::OK();

						if (!IS_POWER_OF_TWO(size)) return Status::InvalidArgument("size not a power of two");

						Entry* new_table = new Entry[size];
						if (!new_table) return Status::Corruption("Out of memory");

						// Ensure the table is cacheline size aligned.
						assert(!(reinterpret_cast<uintptr_t>(new_table) & (CACHELINE_SIZE - 1)));

						table_ = new_table;
						size_ = size;

						return Status::OK();
					};
					/**
					* Uninitialize an initialized table. This method must be used before
					* it is safe to destroy or re-initialize an table. The caller is
					* responsible for ensuring no threads are protected (have started a Protect()
					* without having completed an Unprotect() and that no threads will call
					* Protect()/Unprotect() while the manager is uninitialized; failing to do
					* so results in undefined behavior. Calling Uninitialize() on an uninitialized
					* instance has no effect.
					*
					* \return May return other error codes indicating a failure deallocating the
					*      thread local storage used by the table internally. Even for returns
					*      other than success the object is safely left in an uninitialized state,
					*      though some thread local resources may not have been reclaimed
					*      properly.
					* \retval S_OK Success; resources were reclaimed and table is uninitialized.
					* \retval S_FALSE Success; no effect, since table was already uninitialized.
					*/
					Status Uninitialize() {
						if (!table_) return Status::OK();

						size_ = 0;
						delete[] table_;
						table_ = nullptr;

						return Status::OK();
					};
					/**
					* Enter the thread into the protected code region, which guarantees
					* pointer stability for records in client data structures. After this
					* call, accesses to protected data structure items are guaranteed to be
					* safe, even if the item is concurrently removed from the structure.
					*
					* Behavior is undefined if Protect() is called from an already
					* protected thread. Upon creation, threads are unprotected.
					*
					* \param currentEpoch A sequentially consistent snapshot of the current
					*      global epoch. It is okay that this may be stale by the time it
					*      actually gets entered into the table.
					* \return S_OK indicates thread may now enter the protected region. Any
					*      other return indicates a fatal problem accessing the thread local
					*      storage; the thread may not enter the protected region. Most likely
					*      the library has entered some non-serviceable state.
					*/
					Status Protect(Epoch currentEpoch) {
						Entry* entry = nullptr;
						RETURN_NOT_OK(GetEntryForThread(&entry));

						entry->last_unprotected_epoch = 0;
#if 1
						entry->protected_epoch.store(currentEpoch, std::memory_order_release);
						// TODO: For this to really make sense according to the spec we
						// need a (relaxed) load on entry->protected_epoch. What we want to
						// ensure is that loads "above" this point in this code don't leak down
						// and access data structures before it is safe.
						// Consistent with http://preshing.com/20130922/acquire-and-release-fences/
						// but less clear whether it is consistent with stdc++.
						std::atomic_thread_fence(std::memory_order_acquire);
#else
						entry->m_protectedEpoch.exchange(currentEpoch, std::memory_order_acq_rel);
#endif
						return Status::OK();
					};
					/**
					* Exit the thread from the protected code region. The thread must
					* promise not to access pointers to elements in the protected data
					* structures beyond this call.
					*
					* Behavior is undefined if Unprotect() is called from an already
					* unprotected thread.
					*
					* \param currentEpoch A any rough snapshot of the current global epoch, so
					*      long as it is greater than or equal to the value used on the thread's
					*      corresponding call to Protect().
					* \return S_OK indicates thread successfully exited protected region. Any
					*      other return indicates a fatal problem accessing the thread local
					*      storage; the thread may not have successfully exited the protected
					*      region. Most likely the library has entered some non-serviceable
					*      state.
					*/
					Status Unprotect(Epoch currentEpoch) {
						Entry* entry = nullptr;
						RETURN_NOT_OK(GetEntryForThread(&entry));
						auto hash{ std::hash<std::thread::id>()(std::this_thread::get_id()) };

						entry->last_unprotected_epoch = currentEpoch;
						std::atomic_thread_fence(std::memory_order_release);
						entry->protected_epoch.store(0, std::memory_order_relaxed);
						return Status::OK();
					};
					/**
					* Looks at all of the threads in the protected region and \a currentEpoch
					* and returns the latest Epoch that is guaranteed to be safe for reclamation.
					* That is, all items removed and tagged with a lower Epoch than returned by
					* this call may be safely reused.
					*
					* \param currentEpoch A snapshot of the current global Epoch; it is okay
					*      that the snapshot may lag the true current epoch slightly.
					* \return An Epoch that can be compared to Epochs associated with items
					*      removed from data structures. If an Epoch associated with a removed
					*      item is less or equal to the returned value, then it is guaranteed
					*      that no future thread will access the item, and it can be reused
					*      (by calling, free() on it, for example). The returned value will
					*      never be equal to or greater than the global epoch at any point, ever.
					*      That ensures that removed items in one Epoch can never be freed
					*      within the same Epoch.
					*/
					Epoch ComputeNewSafeToReclaimEpoch(Epoch currentEpoch) {
						Epoch oldest_call = currentEpoch;
						for (uint64_t i = 0; i < size_; ++i) {
							Entry& entry = table_[i];
							// If any other thread has flushed a protected epoch to the cache
							// hierarchy we're guaranteed to see it even with relaxed access.
							Epoch entryEpoch =
								entry.protected_epoch.load(std::memory_order_acquire);
							if (entryEpoch != 0 && entryEpoch < oldest_call) {
								oldest_call = entryEpoch;
							}
						}
						// The latest safe epoch is the one just before the earlier unsafe one.
						return oldest_call - 1;
					};

					/// An entry tracks the protected/unprotected state of a single
					/// thread. Threads (conservatively) the Epoch when they entered
					/// the protected region, and more loosely when they left.
					/// Threads compete for entries and atomically lock them using a
					/// compare-and-swap on the #m_threadId member.
					struct Entry {
						/// Construct an Entry in an unlocked and ready to use state.
						Entry()
							: protected_epoch{ 0 },
							last_unprotected_epoch{ 0 },
							thread_id{ 0 } {
						}

						/// Threads record a snapshot of the global epoch during Protect().
						/// Threads reset this to 0 during Unprotect().
						/// It is safe that this value may actually lag the real current
						/// epoch by the time it is actually stored. This value is set
						/// with a sequentially-consistent store, which guarantees that
						/// it precedes any pointers that were removed (with sequential
						/// consistency) from data structures before the thread entered
						/// the epoch. This is critical to ensuring that a thread entering
						/// a protected region can never see a pointer to a data item that
						/// was already "unlinked" from a protected data structure. If an
						/// item is "unlinked" while this field is non-zero, then the thread
						/// associated with this entry may be able to access the unlinked
						/// memory still. This is safe, because the value stored here must
						/// be less than the epoch value associated with the deleted item
						/// (by sequential consistency, the snapshot of the epoch taken
						/// during the removal operation must have happened before the
						/// snapshot taken just before this field was updated during
						/// Protect()), which will prevent its reuse until this (and all
						/// other threads that could access the item) have called
						/// Unprotect().
						std::atomic<Epoch> protected_epoch; // 8 bytes

						/// Stores the approximate epoch under which the thread last
						/// completed an Unprotect(). This need not be very accurate; it
						/// is used to determine if a thread's slot can be preempted.
						Epoch last_unprotected_epoch;        //  8 bytes

						/// ID of the thread associated with this entry. Entries are
						/// locked by threads using atomic compare-and-swap. See
						/// reserveEntry() for details.
						/// XXX(tzwang): on Linux pthread_t is 64-bit
						std::atomic<uint64_t> thread_id;    //  8 bytes

						/// Ensure that each Entry is CACHELINE_SIZE.
						char ___padding[40];

						// -- Allocation policy to ensure alignment --

						/// Provides cacheline aligned allocation for the table.
						/// Note: We'll want to be even smarter for NUMA. We'll want to
						/// allocate slots that reside in socket-local DRAM to threads.
						void* operator new[](uint64_t count) {
#ifdef WIN32
							return _aligned_malloc(count, CACHELINE_SIZE);
#else
							void* mem = nullptr;
							int n = posix_memalign(&mem, CACHELINE_SIZE, count);
							return mem;
#endif
						}

						void operator delete[](void* p) {
#ifdef WIN32
							/// _aligned_malloc-specific delete.
							return _aligned_free(p);
#else
							free(p);
#endif
						}

						/// Don't allow single-entry allocations. We don't ever do them.
						/// No definition is provided so that programs that do single
						/// allocations will fail to link.
						void* operator new(uint64_t count);

						/// Don't allow single-entry deallocations. We don't ever do them.
						/// No definition is provided so that programs that do single
						/// deallocations will fail to link.
						void operator delete(void* p);
					};
					static_assert(sizeof(Entry) == CACHELINE_SIZE, "Unexpected table entry size");

				public:

					/**
					* Get a pointer to the thread-specific state needed for a thread to
					* Protect()/Unprotect(). If no thread-specific Entry has been allocated
					* yet, then one it transparently allocated and its address is stashed
					* in the thread's local storage.
					*
					* \param[out] entry Points to an address that is populated with
					*      a pointer to the thread's Entry upon return. It is illegal to
					*      pass nullptr.
					* \return S_OK if the thread's entry was discovered or allocated; in such
					*      a successful call \a entry points to a pointer to the Entry.
					*      Any other return value means there was a problem accessing or
					*      setting values in the thread's local storage. The value pointed
					*      to by entry remains unchanged, but the library may have entered
					*      a non-serviceable state.
					*/
					Status GetEntryForThread(Entry** entry) {
						thread_local Entry* tls = nullptr;
						if (tls) {
							*entry = tls;
							return Status::OK();
						}

						// No entry index was found in TLS, so we need to reserve a new entry
						// and record its index in TLS
						Entry* reserved = ReserveEntryForThread();
						tls = *entry = reserved;

						RegisterTls((uint64_t*)&tls, (uint64_t)nullptr);

						return Status::OK();
					};
				// private:

					void RegisterTls(uint64_t* ptr, uint64_t val) {
						auto id = std::this_thread::get_id();

						std::unique_lock<std::mutex> lock(*registryMutex_);
						if (registry_->find(id) == registry_->end()) {
							registry_->emplace(id, std::make_shared<TlsList>());
						}
						registry_->operator[](id)->emplace_back(ptr, val);
					}
					void ClearTls(bool destroy) {
						auto id_ = std::this_thread::get_id(); 
						
						std::unique_lock<std::mutex> lock(*registryMutex_);

						auto iter = registry_->find(id_);
						if (iter != registry_->end()) {
							std::shared_ptr<TlsList> list = iter->second;
							for (auto& entry : *list) {
								*entry.first = entry.second;
							}
							list->clear();							
						}
					}
					void ClearRegistry(bool destroy) {
						std::unique_lock<std::mutex> lock(*registryMutex_);
						for (auto& r : *registry_) {
							std::shared_ptr<TlsList> list = r.second;
							for (auto& entry : *list) {
								*entry.first = entry.second;
							}
							list->clear();
						}
						if (destroy) {
							registry_->clear();
						}
					}

				public:
					/**
					* Does the heavy lifting of reserveEntryForThread() and is really just
					* split out for easy unit testing. This method relies on the fact that no
					* thread will ever have ID on Windows 0.
					* http://msdn.microsoft.com/en-us/library/windows/desktop/ms686746(v=vs.85).aspx
					*/
					Entry* ReserveEntry(uint64_t startIndex, uint64_t threadId) {
						for (;;) {
							// Reserve an entry in the table.
							for (uint64_t i = 0; i < size_; ++i) {
								uint64_t indexToTest = (startIndex + i) & (size_ - 1);
								Entry& entry = table_[indexToTest];
								if (entry.thread_id == 0) {
									uint64_t expected = 0;
									// Atomically grab a slot. No memory barriers needed.
									// Once the threadId is in place the slot is locked.
									bool success =
										entry.thread_id.compare_exchange_strong(expected,
											threadId, std::memory_order_relaxed);
									if (success) {
										return &table_[indexToTest];
									}
									// Ignore the CAS failure since the entry must be populated,
									// just move on to the next entry.
								}
							}
							ReclaimOldEntries();
						}
					};
					/**
					* Allocate a new Entry to track a thread's protected/unprotected status and
					* return a pointer to it. This should only be called once for a thread.
					*/
					Entry* ReserveEntryForThread() {
						auto hash = std::hash<std::thread::id>()(std::this_thread::get_id());

						uint64_t current_thread_id = static_cast<uint64_t>(hash);
						uint64_t startIndex = Murmur3_64(current_thread_id);
						return ReserveEntry(startIndex, current_thread_id);
					};
					void ReleaseEntryForThread() { /* Not implemented? */ };
					void ReclaimOldEntries() { /* Not implemented? */  };
					bool IsProtected() {
						Entry* entry = nullptr;
						Status s = GetEntryForThread(&entry);
						// It's myself checking my own protected_epoch, safe to use relaxed
						return entry->protected_epoch.load(std::memory_order_relaxed) != 0;
					};

				public:

					/// Thread protection status entries. Threads lock entries the first time
					/// the call Protect() (see reserveEntryForThread()). See documentation for
					/// the fields to specifics of how threads use their Entries to guarantee
					/// memory-stability.
					Entry* table_;

					/// The number of entries #m_table. Currently, this is fixed after
					/// Initialize() and never changes or grows. If #m_table runs out
					/// of entries, then the current implementation will deadlock threads.
					uint64_t size_;
				};

				/// A notion of time for objects that are removed from data structures.
				/// Objects in data structures are timestamped with this Epoch just after
				/// they have been (sequentially consistently) "unlinked" from a structure.
				/// Threads also use this Epoch to mark their entry into a protected region
				/// (also in sequentially consistent way). While a thread operates in this
				/// region "unlinked" items that they may be accessing will not be reclaimed.
				std::atomic<Epoch> current_epoch_;

				/// Caches the most recent result of ComputeNewSafeToReclaimEpoch() so
				/// that fast decisions about whether an object can be reused or not
				/// (in IsSafeToReclaim()). Effectively, this is periodically computed
				/// by taking the minimum of the protected Epochs in #m_epochTable and
				/// #current_epoch_.
				std::atomic<Epoch> safe_to_reclaim_epoch_;

				/// Keeps track of which threads are executing in region protected by
				/// its parent EpochManager. On Protect() and Unprotect() by a thread it
				/// updates the table entry that tracks whether the thread is currently
				/// operating in the protected region, and, if so, a conservative estimate
				/// of how early it might have entered. See MinEpochTable for more details.
				std::shared_ptr<MinEpochTable> epoch_table_;

				EpochManager(const EpochManager&) = delete;
				EpochManager& operator=(const EpochManager&) = delete;
				EpochManager(EpochManager&&) = delete; 
				EpochManager& operator=(EpochManager&&) = delete;
			};

			/// Enters an epoch on construction and exits it on destruction. Makes it
			/// easy to ensure epoch protection boundaries tightly adhere to stack life
			/// time even with complex control flow.
			class EpochGuard {
			public:
				explicit EpochGuard(EpochManager* epoch_manager) : epoch_manager_{ epoch_manager }, unprotect_at_exit_(true) { epoch_manager_->Protect(); };

				/// Offer the option of having protext called on \a epoch_manager.
				/// When protect = false this implies "attach" semantics and the caller should
				/// have already called Protect. Behavior is undefined otherwise.
				explicit EpochGuard(EpochManager* epoch_manager, bool protect) : epoch_manager_{ epoch_manager }, unprotect_at_exit_(protect) { if (protect) { epoch_manager_->Protect(); } };

				~EpochGuard() { if (unprotect_at_exit_ && epoch_manager_) { epoch_manager_->Unprotect(); } };

				/// Release the current epoch manger. It is up to the caller to manually
				/// Unprotect the epoch returned. Unprotect will not be called upon EpochGuard
				/// desruction.
				EpochManager* Release() { EpochManager* ret = epoch_manager_; epoch_manager_ = nullptr; return ret; };

			private:
				/// The epoch manager responsible for protect/unprotect.
				EpochManager* epoch_manager_;

				/// Whether the guard should call unprotect when going out of scope.
				bool unprotect_at_exit_;
			};

			/// Interface for the GarbageList; used to make it easy to drop is mocked out
			/// garbage lists for unit testing. See GarbageList template below for
			/// full documentation.
			class IGarbageList {
			public:
				typedef void
				(*DestroyCallback)(void* callback_context, void* object);

				IGarbageList() {}

				virtual ~IGarbageList() {}

				virtual Status Initialize(EpochManager* epoch_manager, size_t size = 4 * 1024 * 1024) {
					((void)epoch_manager);
					((void)epoch_manager);
					return Status::OK();
				}

				virtual Status Uninitialize() {
					return Status::OK();
				}

				virtual Status Push(void* removed_item, DestroyCallback destroy_callback,
					void* context) = 0;
			};

			/// Tracks items that have been removed from a data structure but to which
			/// there may still be concurrent accesses using the item from other threads.
			/// GarbageList works together with the EpochManager to ensure that items
			/// placed on the list are only destructed and freed when it is safe to do so.
			///
			/// Lock-free data structures use this template by creating an instance specific
			/// to the type of the item they will place on the list. When an element is
			/// has been "removed" from the data structure it should call Push() to
			/// transfer responsibility for the item over to the garbage list.
			/// Occasionally, Push() operations will check to see if objects on the list are
			/// ready for reuse, freeing them up if it is safe to do so. The user of the
			/// GarbageList provides a callback that is invoked so custom logic can be used
			/// to reclaim resources.
			class GarbageList : public IGarbageList {
			public:
				/// Holds a pointer to an object in the garbage list along with the Epoch
				/// in which it was removed and a chain field so that it can be linked into
				/// a queue.
				struct Item {
					/// Epoch in which the #m_removedItem was removed from the data
					/// structure. In practice, due to delay between the actual removal
					/// operation and the push onto the garbage list, #m_removalEpoch may
					/// be later than when the actual remove happened, but that is safe
					/// since the invariant is that the epoch stored here needs to be
					/// greater than or equal to the current global epoch in which the
					/// item was actually removed.
					Epoch removal_epoch;

					/// Function provided by user on Push() called when an object
					/// that was pushed to the list is safe for reclamation. When invoked the
					/// function is passed a pointer to an object that is safe to destroy and
					/// free along with #m_pbDestroyCallbackContext. The function must
					/// perform all needed destruction and release any resources associated
					/// with the object.
					DestroyCallback destroy_callback;

					/// Passed along with a pointer to the object to destroy to
					/// #m_destroyCallback; it threads state to destroyCallback calls so they
					/// can access, for example, the allocator from which the object was
					/// allocated.
					void* destroy_callback_context;

					/// Point to the object that is enqueued for destruction. Concurrent
					/// accesses may still be ongoing to the object, so absolutely no
					/// changes should be made to the value it refers to until
					/// #m_removalEpoch is deemed safe for reclamation by the
					/// EpochManager.
					void* removed_item;
				};
				static_assert(std::is_pod<Item>::value, "Item should be POD");

				/// Construct a GarbageList in an uninitialized state.
				GarbageList()
					: epoch_manager_{}
					, tail_{}
					, item_count_{}
					, items_{} {
				}

				/// Uninitialize the GarbageList (if still initialized) and destroy it.
				virtual ~GarbageList() {
					Uninitialize();
				}

				/// Initialize the GarbageList and associate it with an EpochManager.
				/// This must be called on a newly constructed instance before it
				/// is safe to call other methods. If the GarbageList is already
				/// initialized then it will have no effect.
				///
				/// \param pEpochManager
				///      EpochManager that is used to determine when it is safe to reclaim
				///      items pushed onto the list. Must not be nullptr.
				/// \param nItems
				///      Number of addresses that can be held aside for pointer stability.
				///      If this number is too small the system runs the risk of deadlock.
				///      Must be a power of two.
				///
				/// \retval S_OK
				///      The instance is now initialized and ready for use.
				/// \retval S_FALSE
				///      The instance was already initialized; no effect.
				/// \retval E_INVALIDARG
				///      \a nItems wasn't a power of two.
				virtual Status Initialize(EpochManager* epoch_manager,
					size_t item_count = 128 * 1024) {
					if (epoch_manager_) return Status::OK();

					if (!epoch_manager) return Status::InvalidArgument("Null pointer");

					if (!item_count || !IS_POWER_OF_TWO(item_count)) {
						return Status::InvalidArgument("items not a power of two");
					}

					size_t nItemArraySize = sizeof(*items_) * item_count;
					items_ = static_cast<Item*>(fibers::utilities::Mem_Alloc64(nItemArraySize));
					// posix_memalign((void**)&items_, 64, nItemArraySize);

					if (!items_) return Status::Corruption("Out of memory");

					for (size_t i = 0; i < item_count; ++i) new(&items_[i]) Item{};

					item_count_ = item_count;
					tail_ = 0;
					epoch_manager_ = epoch_manager;

					return Status::OK();
				}

				/// Uninitialize the GarbageList and disassociate from its EpochManager;
				/// for each item still on the list call its destructor and free it.
				/// Careful: objects freed by this call will NOT obey the epoch protocol,
				/// so it is important that this thread is only called when it is clear
				/// that no other threads may still be concurrently accessing items
				/// on the list.
				///
				/// \retval S_OK
				///      The instance is now uninitialized; resources were released.
				/// \retval S_FALSE
				///      The instance was already uninitialized; no effect.
				virtual Status Uninitialize() {
					if (!epoch_manager_) return Status::OK();

					for (size_t i = 0; i < item_count_; ++i) {
						Item& item = items_[i];
						if (item.removed_item) {
							item.destroy_callback(
								item.destroy_callback_context,
								item.removed_item);
							item.removed_item = nullptr;
							item.removal_epoch = 0;
						}
					}

					fibers::utilities::Mem_Free64(items_);

					items_ = nullptr;
					tail_ = 0;
					item_count_ = 0;
					epoch_manager_ = nullptr;

					return Status::OK();
				}

				/// Append an item to the reclamation queue; the item will be stamped
				/// with an epoch and will not be reclaimed until the EpochManager confirms
				/// that no threads can ever access the item again. Once an item is ready
				/// for removal the destruction callback passed to Initialize() will be
				/// called which must free all resources associated with the object
				/// INCLUDING the memory backing the object.
				///
				/// \param removed_item
				///      Item to place on the list; it will remain live until
				///      the EpochManager indicates that no threads will ever access it
				///      again, after which the destruction callback will be invoked on it.
				/// \param callback
				///      Function to call when the object that was pushed to the list is safe
				///      for reclamation. When invoked the, function is passed a pointer to
				///      an object that is safe to destroy and free along with
				///      \a pvDestroyCallbackContext. The function must perform
				///      all needed destruction and release any resources associated with
				///      the object. Must not be nullptr.
				/// \param context
				///      Passed along with a pointer to the object to destroy to
				///      \a destroyCallback; it threads state to destroyCallback calls so
				///      they can access, for example, the allocator from which the object
				///      was allocated. Left uninterpreted, so may be nullptr.
				virtual Status Push(void* removed_item, DestroyCallback callback, void* context) {
					Epoch removal_epoch = epoch_manager_->GetCurrentEpoch();
					const uint64_t invalid_epoch = ~0llu;

					for (;;) {
						int64_t slot = (tail_.fetch_add(1) - 1) & (item_count_ - 1);

						// Everytime we work through 25% of the capacity of the list roll
						// the epoch over.
						if (((slot << 2) & (item_count_ - 1)) == 0)
							epoch_manager_->BumpCurrentEpoch();

						Item& item = items_[slot];

						Epoch priorItemEpoch = item.removal_epoch;
						if (priorItemEpoch == invalid_epoch) {
							// Someone is modifying this slot. Try elsewhere.
							continue;
						}

						Epoch result = CompareExchange64<Epoch>(&item.removal_epoch, invalid_epoch, priorItemEpoch);
						if (result != priorItemEpoch) {
							// Someone else is now modifying the slot or it has been
							// replaced with a new item. If someone replaces the old item
							// with a new one of the same epoch number, that's ok.
							continue;
						}

						// Ensure it is safe to free the old entry.
						if (priorItemEpoch) {
							if (!epoch_manager_->IsSafeToReclaim(priorItemEpoch)) {
								// Uh-oh, we couldn't free the old entry. Things aren't looking
								// good, but maybe it was just the result of a race. Replace the
								// epoch number we mangled and try elsewhere.
								*((volatile Epoch*)&item.removal_epoch) = priorItemEpoch;
								continue;
							}
							item.destroy_callback(item.destroy_callback_context,
								item.removed_item);
						}

						// Now populate the entry with the new item.
						item.destroy_callback = callback;
						item.destroy_callback_context = context;
						item.removed_item = removed_item;
						*((volatile Epoch*)&item.removal_epoch) = removal_epoch;

						return Status::OK();
					}
				}

				/// Scavenge items that are safe to be reused - useful when the user cannot
				/// wait until the garbage list is full. Currently (May 2016) the only user is
				/// MwCAS' descriptor pool which we'd like to keep small. Tedious to tune the
				/// descriptor pool size vs. garbage list size, so there is this function.
				int32_t Scavenge() {
					const uint64_t invalid_epoch = ~0llu;
					auto max_slot = tail_.load(std::memory_order_relaxed);
					int32_t scavenged = 0;

					for (int64_t slot = 0; slot < item_count_; ++slot) {
						auto& item = items_[slot];
						Epoch priorItemEpoch = item.removal_epoch;
						if (priorItemEpoch == 0 || priorItemEpoch == invalid_epoch) {
							// Someone is modifying this slot. Try elsewhere.
							continue;
						}

						Epoch result = CompareExchange64<Epoch>(&item.removal_epoch,
							invalid_epoch, priorItemEpoch);
						if (result != priorItemEpoch) {
							// Someone else is now modifying the slot or it has been
							// replaced with a new item. If someone replaces the old item
							// with a new one of the same epoch number, that's ok.
							continue;
						}

						if (priorItemEpoch) {
							if (!epoch_manager_->IsSafeToReclaim(priorItemEpoch)) {
								// Uh-oh, we couldn't free the old entry. Things aren't looking
								// good, but maybe it was just the result of a race. Replace the
								// epoch number we mangled and try elsewhere.
								*((volatile Epoch*)&item.removal_epoch) = priorItemEpoch;
								continue;
							}
							item.destroy_callback(item.destroy_callback_context,
								item.removed_item);
						}

						// Now reset the entry
						item.destroy_callback = nullptr;
						item.destroy_callback_context = nullptr;
						item.removed_item = nullptr;
						*((volatile Epoch*)&item.removal_epoch) = 0;
					}

					return scavenged;
				}

				/// Returns (a pointer to) the epoch manager associated with this garbage list.
				EpochManager* GetEpoch() {
					return epoch_manager_;
				}

			private:
				/// EpochManager instance that is used to determine when it is safe to
				/// free up items. Specifically, it is used to stamp items during Push()
				/// with the current epoch, and it is used in to ensure
				/// that deletion of each item on the list is safe.
				EpochManager* epoch_manager_;

				/// Point in the #m_items ring where the next pushed address will be placed.
				/// Also indicates the next address that will be freed on the next push.
				/// Atomically incremented within Push().
				std::atomic<int64_t> tail_;

				/// Size of the #m_items array. Must be a power of two.
				size_t item_count_;

				/// Ring of addresses the addresses pushed to the list and metadata about
				/// them needed to determine when it is safe to free them and how they
				/// should be freed. This is filled as a ring; when a new Push() comes that
				/// would replace an already occupied slot the entry in the slot is freed,
				/// if possible.
				Item* items_;
			};

		}  // namespace pmwcas

#undef RETURN_NOT_OK
#undef IS_POWER_OF_TWO

#define MWCAS_CAPACITY 7
#define MWCAS_RETRY_THRESHOLD 10
#define MWCAS_SLEEP_TIME 10

		// utility
		namespace dbgroup::atomic::mwcas {
			/*######################################################################################
			 * Global enum and constants
			 *####################################################################################*/

			 /// The maximum number of target words of MwCAS
			constexpr size_t kMwCASCapacity = MWCAS_CAPACITY;

			/// The maximum number of retries for preventing busy loops.
			constexpr size_t kRetryNum = MWCAS_RETRY_THRESHOLD;

			/// A sleep time for preventing busy loops [us].
			static constexpr auto kShortSleep = std::chrono::microseconds{ MWCAS_SLEEP_TIME };

			/*######################################################################################
			 * Global utility functions
			 *####################################################################################*/

			 /**
			  * @tparam T a MwCAS target class.
			  * @retval true if a target class can be updated by MwCAS.
			  * @retval false otherwise.
			  */
			template <class T> constexpr auto CanMwCAS() -> bool {
				if constexpr (sizeof(uint64_t) == sizeof(T)) {
					return true;
				}
				else {
					if constexpr (std::is_same_v<T, uint64_t> || std::is_pointer_v<T>) {
						return true;
					}
					else {
						return false;
					}
				}
			};

		}  // namespace dbgroup::atomic::mwcas
		// common
		namespace dbgroup::atomic::mwcas::component
		{
			/*######################################################################################
			 * Global enum and constants
			 *####################################################################################*/

			 /// Assumes that the length of one word is 8 bytes
			constexpr size_t kWordSize = 8;

			/// Assumes that the size of one cache line is 64 bytes
			constexpr size_t kCacheLineSize = 64;

			/*######################################################################################
			 * Global utility structs
			 *####################################################################################*/

			 /**
			  * @brief An union to convert MwCAS target data into uint64_t.
			  *
			  * @tparam T a type of target data
			  */
			template <class T>
			union CASTargetConverter {
				const T target_data;
				const uint64_t converted_data;

				explicit constexpr CASTargetConverter(const uint64_t converted) : converted_data{ converted } {}

				explicit constexpr CASTargetConverter(const T target) : target_data{ target } {}
			};

			/**
			 * @brief Specialization for unsigned long type.
			 *
			 */
			template <>
			union CASTargetConverter<uint64_t> {
				const uint64_t target_data;
				const uint64_t converted_data;

				explicit constexpr CASTargetConverter(const uint64_t target) : target_data{ target } {}
			};

		}  // namespace dbgroup::atomic::mwcas::component
		// field 
		namespace dbgroup::atomic::mwcas::component
		{
			/**
			 * @brief A class to represent a MwCAS target field.
			 *
			 */
			class MwCASField
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct an empty field for MwCAS.
				  *
				  */
				constexpr MwCASField() : target_bit_arr_{}, mwcas_flag_{ 0 } {}

				/**
				 * @brief Construct a MwCAS field with given data.
				 *
				 * @tparam T a target class to be embedded.
				 * @param target_data target data to be embedded.
				 * @param is_mwcas_descriptor a flag to indicate this field contains a descriptor.
				 */
				template <class T>
				explicit constexpr MwCASField(  //
					T target_data,
					bool is_mwcas_descriptor = false)
					: target_bit_arr_{ ConvertToUint64(target_data) }, mwcas_flag_{ is_mwcas_descriptor }
				{
					// static check to validate MwCAS targets
					static_assert(sizeof(T) == kWordSize);  // NOLINT
					static_assert(std::is_trivially_copyable_v<T>);
					static_assert(std::is_copy_constructible_v<T>);
					static_assert(std::is_move_constructible_v<T>);
					static_assert(std::is_copy_assignable_v<T>);
					static_assert(std::is_move_assignable_v<T>);
					static_assert(CanMwCAS<T>());
				}

				constexpr MwCASField(const MwCASField&) = default;
				constexpr MwCASField(MwCASField&&) = default;

				constexpr auto operator=(const MwCASField& obj)->MwCASField & = default;
				constexpr auto operator=(MwCASField&&)->MwCASField & = default;

				/*####################################################################################
				 * Public destructor
				 *##################################################################################*/

				 /**
				  * @brief Destroy the MwCASField object.
				  *
				  */
				~MwCASField() = default;

				/*####################################################################################
				 * Public operators
				 *##################################################################################*/

				auto
					operator==(const MwCASField& obj) const  //
					-> bool
				{
					return memcmp(this, &obj, sizeof(MwCASField)) == 0;
				}

				auto
					operator!=(const MwCASField& obj) const  //
					-> bool
				{
					return memcmp(this, &obj, sizeof(MwCASField)) != 0;
				}

				/*####################################################################################
				 * Public getters/setters
				 *##################################################################################*/

				 /**
				  * @retval true if this field contains a descriptor.
				  * @retval false otherwise.
				  */
				[[nodiscard]] constexpr auto
					IsMwCASDescriptor() const  //
					-> bool
				{
					return mwcas_flag_;
				}

				/**
				 * @tparam T an expected class of data.
				 * @return data retained in this field.
				 */
				template <class T>
				[[nodiscard]] constexpr auto
					GetTargetData() const  //
					-> T
				{
					if constexpr (std::is_same_v<T, uint64_t>) {
						return target_bit_arr_;
					}
					else if constexpr (std::is_pointer_v<T>) {
						return reinterpret_cast<T>(target_bit_arr_);  // NOLINT
					}
					else {
						return CASTargetConverter<T>{target_bit_arr_}.target_data;  // NOLINT
					}
				}

			private:
				/*####################################################################################
				 * Internal utility functions
				 *##################################################################################*/

				 /**
				  * @brief Conver given data into uint64_t.
				  *
				  * @tparam T a class of given data.
				  * @param data data to be converted.
				  * @return data converted to uint64_t.
				  */
				template <class T>
				constexpr auto
					ConvertToUint64(const T data)  //
					-> uint64_t
				{
					if constexpr (std::is_same_v<T, uint64_t>) {
						return data;
					}
					else if constexpr (std::is_pointer_v<T>) {
						return reinterpret_cast<uint64_t>(data);  // NOLINT
					}
					else {
						return CASTargetConverter<T>{data}.converted_data;  // NOLINT
					}
				}

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// An actual target data
				uint64_t target_bit_arr_ : 63;

				/// Representing whether this field contains a MwCAS descriptor
				uint64_t mwcas_flag_ : 1;
			};

			// CAS target words must be one word
			static_assert(sizeof(MwCASField) == kWordSize);

		}  // namespace dbgroup::atomic::mwcas::component
		// target
		namespace dbgroup::atomic::mwcas::component
		{
			/**
			 * @brief A class to represent a MwCAS target.
			 *
			 */
			class MwCASTarget
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct an empty MwCAS target.
				  *
				  */
				constexpr MwCASTarget() = default;

				/**
				 * @brief Construct a new MwCAS target based on given information.
				 *
				 * @tparam T a class of MwCAS targets.
				 * @param addr a target memory address.
				 * @param old_val an expected value of the target address.
				 * @param new_val an desired value of the target address.
				 */
				template <class T>
				constexpr MwCASTarget(  //
					void* addr,
					const T old_val,
					const T new_val,
					const std::memory_order fence)
					: addr_{ static_cast<std::atomic<MwCASField> *>(addr) },
					old_val_{ old_val },
					new_val_{ new_val },
					fence_{ fence }
				{
				}

				constexpr MwCASTarget(const MwCASTarget&) = default;
				constexpr MwCASTarget(MwCASTarget&&) = default;

				constexpr auto operator=(const MwCASTarget& obj)->MwCASTarget & = default;
				constexpr auto operator=(MwCASTarget&&)->MwCASTarget & = default;

				/*####################################################################################
				 * Public destructor
				 *##################################################################################*/

				 /**
				  * @brief Destroy the MwCASTarget object.
				  *
				  */
				~MwCASTarget() = default;

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Embed a descriptor into this target address to linearlize MwCAS operations.
				  *
				  * @param desc_addr a memory address of a target descriptor.
				  * @retval true if the descriptor address is successfully embedded.
				  * @retval false otherwise.
				  */
				auto
					EmbedDescriptor(const MwCASField desc_addr)  //
					-> bool
				{
					for (size_t i = 1; true; ++i) {
						// try to embed a MwCAS decriptor
						auto expected = addr_->load(std::memory_order_relaxed);
						if (expected == old_val_
							&& addr_->compare_exchange_strong(expected, desc_addr, std::memory_order_relaxed)) {
							return true;
						}
						if (!expected.IsMwCASDescriptor() || i >= kRetryNum) return false;

						// retry if another desctiptor is embedded
					}
				}

				/**
				 * @brief Update a value of this target address.
				 *
				 */
				void
					RedoMwCAS()
				{
					addr_->store(new_val_, fence_);
				}

				/**
				 * @brief Revert a value of this target address.
				 *
				 */
				void
					UndoMwCAS()
				{
					addr_->store(old_val_, std::memory_order_relaxed);
				}

			private:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// A target memory address
				std::atomic<MwCASField>* addr_{};

				/// An expected value of a target field
				MwCASField old_val_{};

				/// An inserting value into a target field
				MwCASField new_val_{};

				/// A fence to be inserted when embedding a new value.
				std::memory_order fence_{ std::memory_order_seq_cst };
			};

		}  // namespace dbgroup::atomic::mwcas::component
		// descriptor
		namespace dbgroup::atomic::mwcas
		{
			/**
			 * @brief A class to manage a MwCAS (multi-words compare-and-swap) operation.
			 *
			 */
			class alignas(component::kCacheLineSize) MwCASDescriptor
			{
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using MwCASTarget = component::MwCASTarget;
				using MwCASField = component::MwCASField;

			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct an empty descriptor for MwCAS operations.
				  *
				  */
				constexpr MwCASDescriptor() = default;

				constexpr MwCASDescriptor(const MwCASDescriptor&) = default;
				constexpr MwCASDescriptor(MwCASDescriptor&&) = default;

				constexpr auto operator=(const MwCASDescriptor& obj)->MwCASDescriptor & = default;
				constexpr auto operator=(MwCASDescriptor&&)->MwCASDescriptor & = default;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the MwCASDescriptor object.
				  *
				  */
				~MwCASDescriptor() = default;

				/*####################################################################################
				 * Public getters/setters
				 *##################################################################################*/

				 /**
				  * @return the number of registered MwCAS targets
				  */
				[[nodiscard]] constexpr auto
					Size() const  //
					-> size_t
				{
					return target_count_;
				}

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Read a value from a given memory address.
				  * \e NOTE: if a memory address is included in MwCAS target fields, it must be read
				  * via this function.
				  *
				  * @tparam T an expected class of a target field
				  * @param addr a target memory address to read
				  * @param fence a flag for controling std::memory_order.
				  * @return a read value
				  */
				template <class T>
				static auto
					Read(  //
						const void* addr,
						const std::memory_order fence = std::memory_order_seq_cst)  //
					-> T
				{
					const auto* target_addr = static_cast<const std::atomic<MwCASField> *>(addr);

					MwCASField target_word{};
					while (true) {
						for (size_t i = 1; true; ++i) {
							target_word = target_addr->load(fence);
							if (!target_word.IsMwCASDescriptor()) return target_word.GetTargetData<T>();
							if (i > kRetryNum) break;
						}

						// wait to prevent busy loop
						std::this_thread::sleep_for(kShortSleep);
					}
				}

				/**
				 * @brief Add a new MwCAS target to this descriptor.
				 *
				 * @tparam T a class of a target
				 * @param addr a target memory address
				 * @param old_val an expected value of a target field
				 * @param new_val an inserting value into a target field
				 * @param fence a flag for controling std::memory_order.
				 */
				template <class T>
				constexpr void
					AddMwCASTarget(  //
						void* addr,
						const T old_val,
						const T new_val,
						const std::memory_order fence = std::memory_order_seq_cst)
				{
					assert(target_count_ < kMwCASCapacity);

					targets_[target_count_++] = MwCASTarget{ addr, old_val, new_val, fence };
				}

				/**
				 * @brief Perform a MwCAS operation by using registered targets.
				 *
				 * @retval true if a MwCAS operation succeeds
				 * @retval false if a MwCAS operation fails
				 */
				auto
					MwCAS()  //
					-> bool
				{
					const MwCASField desc_addr{ this, true };

					// serialize MwCAS operations by embedding a descriptor
					auto mwcas_success = true;
					size_t embedded_count = 0;
					for (size_t i = 0; i < target_count_; ++i, ++embedded_count) {
						if (!targets_[i].EmbedDescriptor(desc_addr)) {
							// if a target field has been already updated, MwCAS fails
							mwcas_success = false;
							break;
						}
					}

					// complete MwCAS
					if (mwcas_success) {
						for (size_t i = 0; i < embedded_count; ++i) {
							targets_[i].RedoMwCAS();
						}
					}
					else {
						for (size_t i = 0; i < embedded_count; ++i) {
							targets_[i].UndoMwCAS();
						}
					}

					return mwcas_success;
				}

			private:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// Target entries of MwCAS
				MwCASTarget targets_[kMwCASCapacity];

				/// The number of registered MwCAS targets
				size_t target_count_{ 0 };
			};

		}  // namespace dbgroup::atomic::mwcas

		/* Converts any small POD-style struct into an atomic struct using multi-word compare and swap operations. 
		Capacity is about 56 bytes, or about 7 pointers / integers. Can be used for a POD collection (like a struct) or non-standard POD items like floats to long doubles.
		*/
		template <typename Arg> struct CAS_Container {
		public:
			static constexpr size_t NumWords{ 1 + sizeof(Arg) / 8 };
		protected:
			union ContainerImpl {
				Arg data;
				uint64_t addresses[1 + sizeof(Arg) / 8];
			};
			ContainerImpl data;
			CAS_Container<Arg> Copy() const {
				using namespace dbgroup::atomic::mwcas;
				CAS_Container<Arg> temp;
				for (size_t index = 0; index < NumWords; index++) {
					temp.Word(index) = MwCASDescriptor::Read<uint64_t>(&Word(index));
				}
				return temp;
			};
			uint64_t& Word(size_t index) {
				return data.addresses[index];
			};
			const uint64_t& Word(size_t index) const {
				return data.addresses[index];
			};
			const Arg& Data() const { return data.data; };
			Arg& Data() { return data.data; };

		public:
			constexpr CAS_Container() : data{} { 
				for (size_t i = 0; i < (1 + sizeof(Arg) / 8); i++) { data.addresses[i] = 0; }
				static_assert(std::is_pod_v<Arg>, "Compare-and-swap operations only work with POD-type structs.");
			};
			constexpr CAS_Container(Arg&& a) : data{} { 
				for (size_t i = 0; i < (1 + sizeof(Arg) / 8); i++) { data.addresses[i] = 0; } 
				data.data = std::forward<Arg>(a); 
				static_assert(std::is_pod_v<Arg>, "Compare-and-swap operations only work with POD-type structs.");
			};
			constexpr CAS_Container(Arg const& a) : data{} { 
				for (size_t i = 0; i < (1 + sizeof(Arg) / 8); i++) { data.addresses[i] = 0; } 
				data.data = a; 
				static_assert(std::is_pod_v<Arg>, "Compare-and-swap operations only work with POD-type structs.");
			};

			~CAS_Container() = default;
			constexpr CAS_Container(const CAS_Container&) = default;
			constexpr CAS_Container& operator=(const CAS_Container&) = default;
			constexpr CAS_Container(CAS_Container&&) = default;
			constexpr CAS_Container& operator=(CAS_Container&&) = default;

		public:
			bool CompareSwap(Arg const& compare, Arg const& input) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::utilities::CAS_Container<Arg> 
					OldCopy(compare), 
					UpdateCopy(input);
				size_t 
					index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor desc{};

					// prepare the swap target(s)
					for (index = 0; index < NumWords; index++) {
						desc.AddMwCASTarget(&Word(index), OldCopy.Word(index), UpdateCopy.Word(index));
					}

					// try multi-word compare and swap
					if (desc.MwCAS()) return true;
					else return false;
				}
			}; // returns the previous value while changing the underlying value
			Arg Swap(Arg const& input, bool allowMiss = false) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::utilities::CAS_Container<Arg> OldCopy, UpdateCopy;
				size_t index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor::Read<uint64_t>(&Word(index));
					}

					// update the actual data
					UpdateCopy.Data() = input;

					// prepare the swap target(s)
					for (index = 0; index < NumWords; index++) {
						desc.AddMwCASTarget(&Word(index), OldCopy.Word(index), UpdateCopy.Word(index));
					}

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
					else if (allowMiss) break;
				}
				return OldCopy.Data();
			}; // returns the previous value while changing the underlying value
			Arg Swap(Arg&& input, bool allowMiss = false) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::utilities::CAS_Container<Arg> OldCopy, UpdateCopy;
				size_t index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor::Read<uint64_t>(&Word(index));
					}

					// update the actual data
					UpdateCopy.Data() = std::forward<Arg>(input);

					// prepare the swap target(s)
					for (index = 0; index < NumWords; index++) {
						desc.AddMwCASTarget(&Word(index), OldCopy.Word(index), UpdateCopy.Word(index));
					}

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
					else if (allowMiss) break;
				}
				return OldCopy.Data();
			}; // returns the previous value while changing the underlying value
			Arg Add(Arg const& input) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::utilities::CAS_Container<Arg> OldCopy, UpdateCopy;
				size_t index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor::Read<uint64_t>(&Word(index));
					}

					// update the actual data
					UpdateCopy.Data() += input;

					// prepare the swap target(s)
					for (index = 0; index < NumWords; index++) {
						desc.AddMwCASTarget(&Word(index), OldCopy.Word(index), UpdateCopy.Word(index));
					}

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
				}
				return OldCopy.Data();
			}; // returns the previous value while incrementing the actual counter
			Arg Add(Arg && input) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				fibers::utilities::CAS_Container<Arg> OldCopy, UpdateCopy;
				size_t index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor::Read<uint64_t>(&Word(index));
					}

					// update the actual data
					UpdateCopy.Data() += std::forward<Arg>(input);

					// prepare the swap target(s)
					for (index = 0; index < NumWords; index++) {
						desc.AddMwCASTarget(&Word(index), OldCopy.Word(index), UpdateCopy.Word(index));
					}

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
				}
				return OldCopy.Data();
			}; // returns the previous value while incrementing the actual counter

		public: // std::atomic compatability
			Arg fetch_add(Arg const& v) {
				return Add(v);
			}; // returns the previous value while incrementing the actual counter
			Arg fetch_sub(Arg const& v) {
				return Add(-v);
			}; // returns the previous value while decrementing the actual counter
			Arg exchange(Arg const& v) {
				return Swap(v);
			}; // returns the previous value while setting the value to the input
			Arg load() const {
				return Copy().data.data;
			}; // gets the value
			void store(Arg const& v) {
				Swap(v);
				return;
			}; // sets the value to the input

		};

		template <typename... Args> struct MultiItemCAS {
		private:
			template<int N> using NthTypeOf = typename std::remove_const<typename std::remove_reference<typename std::tuple_element<N, std::tuple<Args...>>::type>::type>::type;
			static constexpr size_t num_parameters = sizeof...(Args);

		public:
			MultiItemCAS() : container() {};
			MultiItemCAS(Args*... items) : container(items...) {};
			MultiItemCAS(MultiItemCAS const&) = delete;
			MultiItemCAS(MultiItemCAS &&) = delete;
			MultiItemCAS& operator=(MultiItemCAS const&) = delete;
			MultiItemCAS& operator=(MultiItemCAS &&) = delete;
			~MultiItemCAS() {};

		private:
			template <int index> decltype(auto) GetCurrentValue(fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor& desc) const {
				return desc.Read<NthTypeOf<index>>(container.get<index>());
			};
			void CaptureOldValues(Union< Args... >& OldCopy, size_t& index, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor& desc) const {
				for (index = 0; index < num_parameters; index++) {
					switch (index) {
					case 0: if constexpr (num_parameters > 0) OldCopy.get<0>() = GetCurrentValue<0>(desc); break;
					case 1: if constexpr (num_parameters > 1) OldCopy.get<1>() = GetCurrentValue<1>(desc); break;
					case 2: if constexpr (num_parameters > 2) OldCopy.get<2>() = GetCurrentValue<2>(desc); break;
					case 3: if constexpr (num_parameters > 3) OldCopy.get<3>() = GetCurrentValue<3>(desc); break;
					case 4: if constexpr (num_parameters > 4) OldCopy.get<4>() = GetCurrentValue<4>(desc); break;
					case 5: if constexpr (num_parameters > 5) OldCopy.get<5>() = GetCurrentValue<5>(desc); break;
					case 6: if constexpr (num_parameters > 6) OldCopy.get<6>() = GetCurrentValue<6>(desc); break;
					case 7: if constexpr (num_parameters > 7) OldCopy.get<7>() = GetCurrentValue<7>(desc); break;
					case 8: if constexpr (num_parameters > 8) OldCopy.get<8>() = GetCurrentValue<8>(desc); break;
					case 9: if constexpr (num_parameters > 9) OldCopy.get<9>() = GetCurrentValue<9>(desc); break;
					default: break;
					}
				}
			};

			template <typename... IncomingArgs>
			void CaptureOldValuesAndModify(Union< Args... >& OldCopy, Union< Args... >& NewValues, Union< IncomingArgs... >& Functors, size_t& index, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor& desc) const {
				for (index = 0; index < num_parameters; index++) {
					switch (index) {
					case 0: if constexpr (num_parameters > 0) NewValues.get<0>() = Functors.get<0>()(OldCopy.get<0>() = GetCurrentValue<0>(desc)); break;
					case 1: if constexpr (num_parameters > 1) NewValues.get<1>() = Functors.get<1>()(OldCopy.get<1>() = GetCurrentValue<1>(desc)); break;
					case 2: if constexpr (num_parameters > 2) NewValues.get<2>() = Functors.get<2>()(OldCopy.get<2>() = GetCurrentValue<2>(desc)); break;
					case 3: if constexpr (num_parameters > 3) NewValues.get<3>() = Functors.get<3>()(OldCopy.get<3>() = GetCurrentValue<3>(desc)); break;
					case 4: if constexpr (num_parameters > 4) NewValues.get<4>() = Functors.get<4>()(OldCopy.get<4>() = GetCurrentValue<4>(desc)); break;
					case 5: if constexpr (num_parameters > 5) NewValues.get<5>() = Functors.get<5>()(OldCopy.get<5>() = GetCurrentValue<5>(desc)); break;
					case 6: if constexpr (num_parameters > 6) NewValues.get<6>() = Functors.get<6>()(OldCopy.get<6>() = GetCurrentValue<6>(desc)); break;
					case 7: if constexpr (num_parameters > 7) NewValues.get<7>() = Functors.get<7>()(OldCopy.get<7>() = GetCurrentValue<7>(desc)); break;
					case 8: if constexpr (num_parameters > 8) NewValues.get<8>() = Functors.get<8>()(OldCopy.get<8>() = GetCurrentValue<8>(desc)); break;
					case 9: if constexpr (num_parameters > 9) NewValues.get<9>() = Functors.get<9>()(OldCopy.get<9>() = GetCurrentValue<9>(desc)); break;
					default: break;
					}
				}
			};

			template <int index> decltype(auto) PrepareSwapTarget(Union< Args... >& OldCopy, Union< Args... >& NewValues, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor& desc) const {
				desc.AddMwCASTarget(container.get<index>(), OldCopy.get<index>(), NewValues.get<index>());
			};
			void PrepareSwapTargets(Union< Args... >& OldCopy, Union< Args... >& NewValues, size_t& index, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor& desc) {
				for (index = 0; index < num_parameters; index++) {
					switch (index) {
					case 0: if constexpr (num_parameters > 0) PrepareSwapTarget<0>(OldCopy, NewValues, desc); break;
					case 1: if constexpr (num_parameters > 1) PrepareSwapTarget<1>(OldCopy, NewValues, desc); break;
					case 2: if constexpr (num_parameters > 2) PrepareSwapTarget<2>(OldCopy, NewValues, desc); break;
					case 3: if constexpr (num_parameters > 3) PrepareSwapTarget<3>(OldCopy, NewValues, desc); break;
					case 4: if constexpr (num_parameters > 4) PrepareSwapTarget<4>(OldCopy, NewValues, desc); break;
					case 5: if constexpr (num_parameters > 5) PrepareSwapTarget<5>(OldCopy, NewValues, desc); break;
					case 6: if constexpr (num_parameters > 6) PrepareSwapTarget<6>(OldCopy, NewValues, desc); break;
					case 7: if constexpr (num_parameters > 7) PrepareSwapTarget<7>(OldCopy, NewValues, desc); break;
					case 8: if constexpr (num_parameters > 8) PrepareSwapTarget<8>(OldCopy, NewValues, desc); break;
					case 9: if constexpr (num_parameters > 9) PrepareSwapTarget<9>(OldCopy, NewValues, desc); break;
					default: break;
					}
				}
			};

		public:
			template <typename... IncomingArgs>
			Union< Args... > Swap(IncomingArgs&&... newValues) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				Union< Args... > OldCopy;
				Union< Args... > UpdateCopy(std::forward<IncomingArgs>(newValues)...);

				size_t
					index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor desc{};

					// capture the old values
					CaptureOldValues(OldCopy, index, desc);

					// update the actual data
					// ... already done with the provided values

					// prepare the swap target(s)
					PrepareSwapTargets(OldCopy, UpdateCopy, index, desc);

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
				}

				return OldCopy;
			}; // returns the previous value while changing the underlying value
			
			template <typename... IncomingArgs>
			Union< Args... > SwapWithFunctions(IncomingArgs&&... newValues) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				Union< IncomingArgs... > Functors(std::forward<IncomingArgs>(newValues)...);
				Union< Args... > OldCopy;
				Union< Args... > UpdateCopy;

				size_t
					index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor desc{};

					// capture the old values
					CaptureOldValuesAndModify(OldCopy, UpdateCopy, Functors, index, desc);

					// update the actual data
					// ... already done with the provided values during the capture

					// prepare the swap target(s)
					PrepareSwapTargets(OldCopy, UpdateCopy, index, desc);

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
				}

				return OldCopy;
			}; // returns the previous value while changing the underlying value

			template <int index>
			decltype(auto) Read() {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;
				MwCASDescriptor desc{};
				return GetCurrentValue<index>(desc);
			};

		private:
			Union< Args*... > container;
		};





#undef MWCAS_SLEEP_TIME
#undef MWCAS_RETRY_THRESHOLD
#undef MWCAS_CAPACITY

#endif


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
		template <typename mutex = synchronization::mutex> class shared_mutex {
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
				if constexpr (isFloatingPoint) return fibers::utilities::CAS_Container<type>();
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
			constexpr atomic_number() : value(static_cast<type>(0)) {};
			constexpr atomic_number(const type& a) : value(a) {};
			constexpr atomic_number(type&& a) : value(std::forward<type>(a)) {};
			atomic_number(const atomic_number& other) : value(other.load()) {};
			atomic_number(atomic_number&& other) : value(other.load()) {};
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
					value.Add(-(i.load()));
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
	










#if 0
		template <typename key> class OrderedSet {
		private:
			static constexpr bool isPod{ std::is_pod<key>::value };

			struct node {
				key Position;
				synchronization::atomic_ptr<node> parent;
				synchronization::atomic_ptr<node> next;
				synchronization::atomic_ptr<node> prev;
				synchronization::atomic_number<long> numChildren;
				synchronization::atomic_ptr<node> firstChild;
				synchronization::atomic_ptr<node> lastChild;
				
				synchronization::TicketSpinLock nodeLock;
				// synchronization::shared_mutex nodeLock;				
			};

			utilities::Allocator< node, 128, isPod> nodeAllocator;

			synchronization::atomic_number<long> Num;
			synchronization::atomic_ptr<node> root;
			synchronization::atomic_ptr<node> first;
			synchronization::atomic_ptr<node> last;

			decltype(auto) AllocNode() {
				return nodeAllocator.Alloc(); // retrieve and clear a previously used node.
			};
			void FreeNode(node* Node) {
				nodeAllocator.Free(Node); // returns the node to the list for later re-use.
			};

			void SwapSingleNode(node* oldNode, node* newNode) {
				auto nodeLock{ std::scoped_lock(oldNode->nodeLock) };
				auto nodeLock{ std::scoped_lock(oldNode->nodeLock) };



				auto parentLock{ std::scoped_lock(oldNode->parent) };




				auto leftLock{ std::scoped_lock(oldNode->prev) };
				auto rightLock{ std::scoped_lock(oldNode->prev) };



				newNode->parent = oldNode->parent;
			}





			void SplitNode(node* Node) {







				assert(Node->parent); // the incoming node must have a valid parent to be able to be "split". This includes the root, which requires some modifications to work with this system.










				// lock this and neighboring nodes down for changes
				auto nodeLock1{ std::scoped_lock(Node->nodeLock) };






				{
					long long i;
					node* child, * newNode;

					// allocate a new node
					newNode = AllocNode();
					newNode->parent = Node->parent;

					// divide the children over the two nodes
					child = Node->firstChild;
					child->parent = newNode;
					for (i = 3; i < Node->numChildren; i += 2) {
						child = child->next;
						child->parent = newNode;
					}

					newNode->key = child->key;
					newNode->numChildren = Node->numChildren / 2;
					newNode->firstChild = Node->firstChild;
					newNode->lastChild = child;

					Node->numChildren -= newNode->numChildren;
					Node->firstChild = child->next;

					child->next->prev = nullptr;
					child->next = nullptr;

					// add the new child to the parent before the split node
					assert(Node->parent->numChildren < maxChildrenPerNode);

					if (Node->prev) {
						Node->prev->next = newNode;
					}
					else {
						Node->parent->firstChild = newNode;
					}
					newNode->prev = Node->prev;
					newNode->next = Node;
					Node->prev = newNode;

					Node->parent->numChildren++;
				}
			};







		private:
			utilities::Allocator< element, 128, std::is_pod<key>::value> elementAlloc;
			utilities::Allocator< node, 128, true> nodeAlloc;
			node* MakeNode() { return nodeAlloc.Alloc(); };
			element* MakeElement() { return elementAlloc.Alloc();};

		public:
			OrderedSet() : nodeAllocator(), Num(0), root(nullptr), first(nullptr), last(nullptr) { 
				root = nodeAllocator.Alloc(); 
			};




			// returns true if the key did not already exist. Otherwise returns false.
			bool try_insert(key const& newKey) {

			};
			// returns true if the key exists. Otherwise return false.
			bool contains(key const& findKey) {

			};
			// returns true if the key did exist. Otherwise returns false.
			bool try_remove(key const& findKey) {

			};
			




		};
#endif













		namespace impl {	
			template <typename KeyType = int> class SetNode;
			template <typename KeyType = int> class SetNodePtrInfo {
			public:
				SetNode<KeyType>* nodeRef;
				bool flag;
				bool mark;
				bool thread;
				SetNodePtrInfo() noexcept {}
				SetNodePtrInfo(SetNode<KeyType>* nodeRef, bool flag, bool mark, bool thread) noexcept :
					nodeRef(nodeRef),
					flag(flag),
					mark(mark),
					thread(thread) {}
			};
			template <typename KeyType> class SetNode {
			public:
				SetNode() noexcept {}
				KeyType k;
				std::atomic<SetNodePtrInfo<KeyType>> child[2];
				std::atomic<SetNode*> backLink;
				SetNode* preLink;
			};
		};

		template <typename KeyType = int>
		class Set {
		public:
			Set() : count(0), nodeAllocator(std::make_shared<utilities::Allocator<impl::SetNode<KeyType>, 10, std::is_pod<KeyType>::value>>()) {
				root[0].k = -std::numeric_limits<KeyType>::max();
				root[0].child[0] = impl::SetNodePtrInfo<KeyType>(&root[0], 0, 0, 1);
				root[0].child[1] = impl::SetNodePtrInfo<KeyType>(&root[1], 0, 0, 1);
				root[0].backLink = &root[1];
				root[0].preLink = nullptr;

				root[1].k = std::numeric_limits<KeyType>::max();
				root[1].child[0] = impl::SetNodePtrInfo<KeyType>(&root[0], 0, 0, 0);
				root[1].child[1] = impl::SetNodePtrInfo<KeyType>(nullptr, 0, 0, 1);
				root[1].backLink = nullptr;
				root[1].preLink = nullptr;
			}
			Set(Set const&) = default;
			Set(Set&&) = default;
			Set& operator=(Set const&) = default;
			Set& operator=(Set&&) = default;
			~Set() = default;

			bool contains(KeyType k) {
				auto* prev = &root[1];
				auto* curr = &root[0];
				int dir;
				dir = locate(prev, curr, k);
				if (dir == -1) {
					return 0;
				}

				if (dir == 2)
					return true;
				else
					return false;
			};
			KeyType findLargestKeyLessThanOrEqualTo(KeyType k) {
				auto* prev = &root[1];
				auto* curr = &root[0];
				int dir;
				dir = locate(prev, curr, k);
				impl::SetNodePtrInfo<KeyType> curr_child;

				switch (dir) {
				case 0:
				case 1:
					curr_child = curr->child[0];
					if (curr_child.nodeRef) {
						auto* backLink = curr_child.nodeRef->backLink.load();
						if (backLink)
							return backLink->k;
					}
					break;
				case 2:
					return curr->k;
				default:
					return -std::numeric_limits<KeyType>::max();
				}
			};
			KeyType findSmallestKeyGreaterThanOrEqualTo(KeyType k) {
				auto* prev = &root[1];
				auto* curr = &root[0];
				int dir;
				dir = locate(prev, curr, k);
				impl::SetNodePtrInfo<KeyType> curr_child;

				switch (dir) {
				case 0:
				case 1:
					curr_child = curr->child[0];
					if (curr_child.nodeRef) {
						return curr_child.nodeRef->k;
					}
					break;
				case 2:
					return curr->k;
				default:
					return -std::numeric_limits<KeyType>::max();
				}
			};
			bool remove(KeyType k) {
				impl::SetNode<KeyType>* prev = &root[1];
				impl::SetNode<KeyType>* curr = &root[0];
				bool result{ false };
				int dir;

				//dir = locate(prev, curr, k); // looks for k -- if found, the "curr" is set the it's node, and prev is .. what is it?
				//if (dir == 2) {
				//	impl::SetNode<KeyType>* back = curr->backLink;
				//	result = tryFlag(prev, curr, back, true); // flags them down
				//	
				//	cleanFlag(prev, curr, back, true);
				//	if (result) {
				//		count.fetch_add(-1);
				//	}
				//}
				//return result;

				double epsilon = 0.5;
				dir = locate(prev, curr, (double)k - epsilon); 
				if (dir == -1) { return 0; }
				impl::SetNodePtrInfo<KeyType> next = curr->child[dir];
				if (k == next.nodeRef->k) {
					result = tryFlag(curr, next.nodeRef, prev, true);
					impl::SetNodePtrInfo<KeyType> curr_child = curr->child[dir];
					if (curr_child.nodeRef == next.nodeRef) {
						cleanFlag(curr, next.nodeRef, prev, true);
					}
					if (result) {
						count.fetch_add(-1);
					}
				}
				return result;
			};
			bool add(KeyType k) {
				impl::SetNode<KeyType>* prev = &root[1];
				impl::SetNode<KeyType>* curr = &root[0];

				impl::SetNode<KeyType>* node = nodeAllocator->Alloc();
				node->k = k;
				node->child[0] = impl::SetNodePtrInfo<KeyType>(node, 0, 0, 1);

				auto cmp1 = impl::SetNodePtrInfo<KeyType>(nullptr, 0, 0, 1);
				auto cmp2 = impl::SetNodePtrInfo<KeyType>(node, 0, 0, 0);

				int dir;
				while (true) {					
					dir = locate(prev, curr, k);
					if (dir == -1) {
						nodeAllocator->Free(node);
						return false;
					}
					else if (dir == 2) {
						nodeAllocator->Free(node);
						return false;
					}
					else {
						impl::SetNodePtrInfo<KeyType> R = curr->child[dir];
						cmp1.nodeRef = R.nodeRef;
						node->child[1] = cmp1;
						node->backLink = curr;

						bool result = CAS(curr->child[dir], cmp1, cmp2);

						if (result) {
							count.fetch_add(1);
							return true;
						}
						else {
							//This part is for helping
							impl::SetNodePtrInfo<KeyType> newR = curr->child[dir];
							if (newR.nodeRef == R.nodeRef) {
								impl::SetNode<KeyType>* newCurr = prev;
								if (newR.mark)
									cleanMark(curr, dir);
								else if (newR.flag)
									cleanFlag(curr, R.nodeRef, prev, true);

								curr = newCurr;
								prev = newCurr->backLink;
							}
						}
					}
				}
			};
			size_t Num() const {
				return (size_t)(long)count;
			};

		private:
			int locate(impl::SetNode<KeyType>*& prev, impl::SetNode<KeyType>*& curr, KeyType k) {
				while (true) {
					auto dir = cmp(k, curr->k);

					if (dir == 2)
						return dir;

					else {
						impl::SetNodePtrInfo<KeyType> R = curr->child[dir];
						if (R.mark == 1 && dir == 1) {
							impl::SetNode<KeyType>* newPrev = prev->backLink;
							//cleanMarked(prev, curr, dir); //cleanMarked function is not clear yet
							cleanMark(curr, dir);
							prev = newPrev;
							auto pDir = cmp(k, prev->k);
							impl::SetNodePtrInfo<KeyType> temp = prev->child[pDir];
							curr = temp.nodeRef;
							continue;
						}

						if (R.thread) {
							if (dir == 0 || k < R.nodeRef->k) {
								return dir;
							}
							else {
								prev = curr;
								curr = R.nodeRef;
							}
						}
						else {
							prev = curr;
							curr = R.nodeRef;
						}
					}
				}
			};
			bool tryFlag(impl::SetNode<KeyType>*& prev, impl::SetNode<KeyType>*& curr, impl::SetNode<KeyType>*& back, bool isThread) {

				std::atomic<impl::SetNodePtrInfo<KeyType>> atomicSetNodePoKeyTypeerInfo;
				while (true) {

					auto pDir = cmp(curr->k, prev->k) & 1; //curr-> and prev->K will be same when they are pointing to the same node. This is only possible by the threaded left-link.
					bool t = isThread;

					impl::SetNodePtrInfo<KeyType> test = impl::SetNodePtrInfo<KeyType>(curr, 0, 0, t);
					impl::SetNodePtrInfo<KeyType> replace = impl::SetNodePtrInfo<KeyType>(curr, 1, 0, t);
					//compareSetNodePointerInfo(prev->child[pDir], test, "inside try Flag 1");
					bool result = CAS(prev->child[pDir], test, replace);

					if (result) {
						//			prKeyTypef("tryFlag: Flagged successfully: prev->k %d, curr->k %d\n", prev->k, curr->k);
						return true;
					}
					else {

						impl::SetNodePtrInfo<KeyType> temp = prev->child[pDir];
						if (temp.nodeRef == curr) {
							if (temp.flag)
								return false;
							else if (temp.mark)
								cleanMark(prev, pDir);
							prev = back;	//back is provided as third parameter. It helps to step back and restart.
							auto newPDir = cmp(curr->k, prev->k);
							impl::SetNodePtrInfo<KeyType> temp = prev->child[newPDir];
							impl::SetNode<KeyType>* newCurr = temp.nodeRef;
							locate(prev, newCurr, curr->k);
							if (newCurr != curr)
								return false;
							prev = prev->backLink;
						}
					}

				}
			};
			void tryMark(impl::SetNode<KeyType>* curr, int dir) {

				while (true) {
					impl::SetNode<KeyType>* back = curr->backLink;
					impl::SetNodePtrInfo<KeyType> next = curr->child[dir];
					if (next.mark)
						break;
					else if (next.flag) {
						if (!next.thread) {
							cleanFlag(curr, next.nodeRef, back, false);
							continue;
						}
						else if (next.thread && dir) {
							cleanFlag(curr, next.nodeRef, back, true);
							continue;
						}
					}

					bool result = CAS(curr->child[dir], impl::SetNodePtrInfo<KeyType>(next.nodeRef, 0, 0, next.thread), impl::SetNodePtrInfo<KeyType>(next.nodeRef, 0, 1, next.thread));
					if (result)
						break;

				}

			};
			void cleanFlag(impl::SetNode<KeyType>*& prev, impl::SetNode<KeyType>*& curr, impl::SetNode<KeyType>*& back, bool isThread) {
				std::atomic<impl::SetNodePtrInfo<KeyType>> atomicSetNodePoKeyTypeerInfo;
				if (isThread) {
					while (true) {
						impl::SetNodePtrInfo<KeyType> next = curr->child[1];
						if (next.mark)
							break;
						else if (next.flag) {

							if (back == next.nodeRef)
								back = back->backLink;

							impl::SetNode<KeyType>* backSetNode = curr->backLink;
							cleanFlag(curr, next.nodeRef, backSetNode, next.thread);

							if (back == next.nodeRef) {
								auto pDir = cmp(prev->k, backSetNode->k);
								//prev = back->child[pDir];
								impl::SetNode<KeyType>* back_temp = back;
								impl::SetNodePtrInfo<KeyType> back_child = back_temp->child[pDir];
								prev = back_child.nodeRef;
							}
						}
						else {
							if (curr->preLink != prev) //step 2: set the prelink.
								curr->preLink = prev;

							//step: 3: mark the outgoing right link
							bool result = CAS(curr->child[1], impl::SetNodePtrInfo<KeyType>(next.nodeRef, 0, 0, next.thread), impl::SetNodePtrInfo<KeyType>(next.nodeRef, 0, 1, next.thread));

							impl::SetNodePtrInfo<KeyType> temp = curr->child[1];
							if (result) {

								//					puts("CleanFlag 1: successfully marked right link");
								//					prKeyTypef("curr->k %d, temp.nodeRef->k %d, temp.mark %d\n", curr->k, temp.nodeRef->k, temp.mark);
								//					exit(0);
								break;
							}


						}
					}
					cleanMark(curr, 1);
				}
				else {

					impl::SetNodePtrInfo<KeyType> right = curr->child[1];
					//		prKeyTypef("inside cleanFlag2: prev->k %d, curr->k %d, right.nodeRef->k %d, right.flag %d, right.mark %d, right.thread %d\n",
					//				prev->k, curr->k, right.nodeRef->k, right.flag, right.mark, right.thread );
							//exit(0);
					if (right.mark) {
						impl::SetNodePtrInfo<KeyType> left = curr->child[0];
						impl::SetNode<KeyType>* preSetNode = curr->preLink;

						if (left.nodeRef != preSetNode) { //this is cat 3 node
			//				puts("Clean flag 3: Entered to step 6");
							//exit(0);
							tryMark(curr, 0);
							cleanMark(curr, 0);
						}
						else {
							auto pDir = cmp(curr->k, prev->k);
							if (left.nodeRef == curr) { //cat 1 node

								CAS(prev->child[pDir], impl::SetNodePtrInfo<KeyType>(curr, 1, 0, 0), impl::SetNodePtrInfo<KeyType>(right.nodeRef, 0, 0, right.thread)); //what is f.

								if (!right.thread) {
									right.nodeRef->backLink.compare_exchange_weak(curr, prev);
								}
								//					prKeyTypef("removed %d\n",curr->k);
							}
							else {


								bool result = CAS(preSetNode->child[1], impl::SetNodePtrInfo<KeyType>(curr, 1, 0, 1), impl::SetNodePtrInfo<KeyType>(right.nodeRef, 0, 0, right.thread));
								//					prKeyTypef("swap success1: %d\n", result);

								if (!right.thread) {
									result = right.nodeRef->backLink.compare_exchange_strong(curr, prev);
									//						prKeyTypef("swap success2: %d\n", result);
								}


								result = CAS(prev->child[pDir], impl::SetNodePtrInfo<KeyType>(curr, 1, 0, 0), impl::SetNodePtrInfo<KeyType>(preSetNode, 0, 0, right.thread));
								//					prKeyTypef("swap success3: %d\n", result);

								result = preSetNode->backLink.compare_exchange_strong(curr, prev);
								//					prKeyTypef("swap success4: %d\n", result);
								//					std::cout<<"removed: "<<curr->k<<'\n';
							}
						}
					}
					else if (right.thread && right.flag) {
						impl::SetNode<KeyType>* delSetNode = right.nodeRef;
						impl::SetNode<KeyType>* parent = delSetNode->backLink;
						while (true) {

							auto pDir = cmp(delSetNode->k, parent->k); //changed from paper
							impl::SetNodePtrInfo<KeyType> temp = parent->child[pDir];
							if (temp.mark)
								cleanMark(parent, pDir);
							else if (temp.flag)
								break;
							else if (CAS(parent->child[pDir], impl::SetNodePtrInfo<KeyType>(delSetNode, 0, 0, 0), impl::SetNodePtrInfo<KeyType>(delSetNode, 1, 0, 0))) {
								//					puts("cleanFlag 4: step 5 done. Parent link of the node to be deleted flagged successfully");
								break;
							}

						}

						impl::SetNode<KeyType>* backSetNode = parent->backLink;

						//			prKeyTypef("parent->k %d, delSetNode->k %d, backSetNode->k %d\n", parent->k, delSetNode->k, backSetNode->k);
									//exit(0);
						cleanFlag(parent, delSetNode, backSetNode, false); //changed to false. I think, "true" was a mistake.
					}
				}
			};;
			void cleanMark(impl::SetNode<KeyType>*& curr, KeyType markDir) {

				std::atomic<impl::SetNodePtrInfo<KeyType>> atomicSetNodePoKeyTypeerInfo;
				impl::SetNodePtrInfo<KeyType> left = curr->child[0];
				impl::SetNodePtrInfo<KeyType> right = curr->child[1];
				//	puts("inside clean Mark:");
				//	prKeyTypef("curr %d, marDir %d, curr->child[0]->k %d, curr->child[1]->k: %d\n", curr->k, markDir, left.nodeRef->k, right.nodeRef->k);

				if (markDir) {

					//KeyType pDir = markDir;
					impl::SetNode<KeyType>* delSetNode = curr; //Not sure
					while (true) {

						impl::SetNode<KeyType>* preSetNode = delSetNode->preLink;
						//			std::cout<<"preSetNode->k :"<<preSetNode->k<<'\n';

						impl::SetNode<KeyType>* parent = delSetNode->backLink;
						auto pDir = cmp(delSetNode->k, parent->k);
						impl::SetNodePtrInfo<KeyType> parent_child = parent->child[pDir];

						//step 4 for category 1 and 2. Acutally step 5: flag the incoming parent link
						if (preSetNode == left.nodeRef) { //category 1 or 2 node.
			//				puts("clearn mark: inside cat 1 and 2");

							impl::SetNode<KeyType>* parent = delSetNode->backLink;
							impl::SetNode<KeyType>* back = parent->backLink;
							//				prKeyTypef("delSetNode->k %d, parent->k %d, back-> %d\n", delSetNode->k, parent->k, back->k);
							tryFlag(parent, curr, back, false); // why threaded? So, I changed it to non-threaded

			//				prKeyTypef("Clean Mark 4: parent_child.nodeRef %d, curr %d, parent_child->k %d, curr->k %d\n", parent_child.nodeRef, curr, parent_child.nodeRef->k, curr->k);
							if (parent_child.nodeRef == curr) { // If the link still persists
								cleanFlag(parent, curr, back, false);
								break;
							}
						}
						else {
							//category 3 node.
							//This step 4 for category 3 node. Step 4: flag the incoming parent link of the predecessor.
							impl::SetNode<KeyType>* preParent = preSetNode->backLink;
							impl::SetNodePtrInfo<KeyType> temp = preParent->child[1]; // child[1] because predecessor of cat3 node is always the right child of it's parent.
							impl::SetNode<KeyType>* backSetNode = preParent->backLink;

							//				prKeyTypef("clean Mark 2: preSetNode->k %d, preParent->k %d, backSetNode->k %d\n", preSetNode->k , preParent->k, backSetNode->k );
							//				prKeyTypef("clean Mark 3: temp.noderef->k: %d, temp.flag: %d, temp.mark: %d, temp.thread: %d\n", temp.nodeRef->k, temp.flag, temp.mark, temp.thread);


							if (temp.mark)
								cleanMark(preParent, 1);
							else if (temp.flag) {
								cleanFlag(preParent, preSetNode, backSetNode, false); //changed isThreaded parameter to false
								break;
							}
							else if (CAS(preParent->child[pDir], impl::SetNodePtrInfo<KeyType>(preSetNode, 0, 0, 0), impl::SetNodePtrInfo<KeyType>(preSetNode, 1, 0, 0))) { //
			//					prKeyTypef("incoming parentlink pred successfully flagged\n");

								cleanFlag(preParent, preSetNode, backSetNode, false); //changed isThreaded parameter to false.
								break;
							}
						}
					}
				}
				else {
					if (right.mark) {

						impl::SetNode<KeyType>* preSetNode = curr->preLink;
						tryMark(preSetNode, 0);
						cleanMark(preSetNode, 0);
					}
					else if (right.thread && right.flag) {

						impl::SetNode<KeyType>* delSetNode = right.nodeRef;
						impl::SetNode<KeyType>* delSetNodePa = delSetNode->backLink;
						impl::SetNode<KeyType>* preParent = curr->backLink;
						auto pDir = cmp(delSetNode->k, delSetNodePa->k);
						impl::SetNodePtrInfo<KeyType> delSetNodeL = delSetNode->child[0];
						impl::SetNodePtrInfo<KeyType> delSetNodeR = delSetNode->child[1];

						KeyType res1 = -1, res2 = -1, res3 = -1, res4 = -1, res5 = -1, res6 = -1, res7 = -1, res8 = -1;

						res1 = CAS(preParent->child[1], impl::SetNodePtrInfo<KeyType>(curr, 1, 0, 0), impl::SetNodePtrInfo<KeyType>(left.nodeRef, left.flag, 0, left.thread));

						if (!left.thread) {
							res2 = left.nodeRef->backLink.compare_exchange_weak(curr, preParent);

						}

						res3 = CAS(curr->child[0], impl::SetNodePtrInfo<KeyType>(left.nodeRef, 0, 1, left.thread), impl::SetNodePtrInfo<KeyType>(delSetNodeL.nodeRef, 0, 0, 0));

						res4 = delSetNodeL.nodeRef->backLink.compare_exchange_strong(delSetNode, curr);

						res5 = CAS(curr->child[1], impl::SetNodePtrInfo<KeyType>(right.nodeRef, 1, 0, 1), impl::SetNodePtrInfo<KeyType>(delSetNodeR.nodeRef, 0, 0, delSetNodeR.thread));

						if (!delSetNodeR.thread) {
							res6 = delSetNodeR.nodeRef->backLink.compare_exchange_strong(delSetNode, curr);

						}

						res7 = CAS(delSetNodePa->child[pDir], impl::SetNodePtrInfo<KeyType>(delSetNode, 1, 0, 0), impl::SetNodePtrInfo<KeyType>(curr, 0, 0, 0));

						res8 = curr->backLink.compare_exchange_strong(preParent, delSetNodePa);

					}
				}
			};
			static int cmp(KeyType const& x, KeyType const& y) {
				if (x == y)
					return 2;
				else if (x > y)
					return 1;
				else
					return 0;
			};
			bool CAS(std::atomic<impl::SetNodePtrInfo<KeyType>>& oldValue, impl::SetNodePtrInfo<KeyType> newValue, impl::SetNodePtrInfo<KeyType> replacement) {
				bool result{ false };
				for (int i = 0; i < TIMES && !result; i++) {
					result = oldValue.compare_exchange_strong(newValue, replacement);
				}
				return result;
			};

			std::atomic<long> count;
			std::shared_ptr<utilities::Allocator<impl::SetNode<KeyType>, 10, std::is_pod<KeyType>::value>> nodeAllocator;
			impl::SetNode<KeyType> root[2];
			static constexpr int TIMES = 4;
		};

		namespace impl {
			template <typename KeyType, typename ObjType> class MapNode;
			template <typename KeyType, typename ObjType> class MapNodePtrInfo {
			public:
				MapNode<KeyType, ObjType>* nodeRef;
				bool flag;
				bool mark;
				bool thread;
				MapNodePtrInfo() noexcept {}
				MapNodePtrInfo(MapNode<KeyType, ObjType>* nodeRef, bool flag, bool mark, bool thread) noexcept :
					nodeRef(nodeRef),
					flag(flag),
					mark(mark),
					thread(thread) {}
			};
			template <typename KeyType, typename ObjType> class MapNode {
			public:
				MapNode() noexcept {}
				KeyType k;
				std::shared_ptr<ObjType> obj;
				std::atomic<MapNodePtrInfo<KeyType, ObjType>> child[2];
				std::atomic<MapNode*> backLink;
				MapNode* preLink;
			};
		};

		template <typename KeyType = int, typename ObjType = int>
		class Map {
		public:
			Map() : count(0), nodeAllocator(std::make_shared<utilities::Allocator<impl::MapNode<KeyType, ObjType>, 10>>()) {
				root[0].k = -std::numeric_limits<KeyType>::max();
				root[0].child[0] = impl::MapNodePtrInfo<KeyType, ObjType>(&root[0], 0, 0, 1);
				root[0].child[1] = impl::MapNodePtrInfo<KeyType, ObjType>(&root[1], 0, 0, 1);
				root[0].backLink = &root[1];
				root[0].preLink = nullptr;

				root[1].k = std::numeric_limits<KeyType>::max();
				root[1].child[0] = impl::MapNodePtrInfo<KeyType, ObjType>(&root[0], 0, 0, 0);
				root[1].child[1] = impl::MapNodePtrInfo<KeyType, ObjType>(nullptr, 0, 0, 1);
				root[1].backLink = nullptr;
				root[1].preLink = nullptr;
			}
			Map(Map const&) = default;
			Map(Map&&) = default;
			Map& operator=(Map const&) = default;
			Map& operator=(Map&&) = default;
			~Map() = default;

			bool contains(KeyType k) {
				auto* prev = &root[1];
				auto* curr = &root[0];
				int dir;
				dir = locate(prev, curr, k);

				if (dir == 2)
					return true;
				else
					return false;
			};
			bool remove(KeyType k) {
				impl::MapNode<KeyType, ObjType>* prev = &root[1];
				impl::MapNode<KeyType, ObjType>* curr = &root[0];
				bool result{ false };
				double epsilon = 0.5;
				int dir;
				dir = locate(prev, curr, (double)k - epsilon);
				if (dir == -1) { return 0; }

				impl::MapNodePtrInfo<KeyType, ObjType> next = curr->child[dir];

				if (k == next.nodeRef->k) {
					result = tryFlag(curr, next.nodeRef, prev, true);

					impl::MapNodePtrInfo<KeyType, ObjType> curr_child = curr->child[dir];
					if (curr_child.nodeRef == next.nodeRef) {
						cleanFlag(curr, next.nodeRef, prev, true);
					}

					if (result) {
						next.nodeRef->obj = nullptr;
						nodeAllocator->Free(next.nodeRef);
						count.fetch_add(-1);
					}
				}
				return result;
			};
			std::shared_ptr<ObjType> add(KeyType k, ObjType const& obj) {
				impl::MapNode<KeyType, ObjType>* prev = &root[1];
				impl::MapNode<KeyType, ObjType>* curr = &root[0];

				impl::MapNode<KeyType, ObjType>* node = nodeAllocator->Alloc();
				std::shared_ptr<ObjType> objPtr{ std::make_shared<ObjType>(obj) };

				node->obj = objPtr;
				node->k = k;
				node->child[0] = impl::MapNodePtrInfo<KeyType, ObjType>(node, 0, 0, 1);

				auto cmp1 = impl::MapNodePtrInfo<KeyType, ObjType>(nullptr, 0, 0, 1);
				auto cmp2 = impl::MapNodePtrInfo<KeyType, ObjType>(node, 0, 0, 0);

				int dir;
				while (true) {
					dir = locate(prev, curr, k);
					if (dir == -1) {
						nodeAllocator->Free(node);

						return nullptr;
					}
					else if (dir == 2) { // it exists already
						nodeAllocator->Free(node);

						*curr->obj = obj;

						return curr->obj;
					}
					else {
						impl::MapNodePtrInfo<KeyType, ObjType> R = curr->child[dir];
						cmp1.nodeRef = R.nodeRef;
						node->child[1] = cmp1;
						node->backLink = curr;

						bool result = CAS(curr->child[dir], cmp1, cmp2);

						if (result) {
							count.fetch_add(1);
							return objPtr;
						}
						else {
							//This part is for helping
							impl::MapNodePtrInfo<KeyType, ObjType> newR = curr->child[dir];
							if (newR.nodeRef == R.nodeRef) {
								impl::MapNode<KeyType, ObjType>* newCurr = prev;
								if (newR.mark)
									cleanMark(curr, dir);
								else if (newR.flag)
									cleanFlag(curr, R.nodeRef, prev, true);

								curr = newCurr;
								prev = newCurr->backLink;
							}
						}
					}
				}
			};
			std::shared_ptr<ObjType> add(KeyType k, ObjType&& obj) {
				impl::MapNode<KeyType, ObjType>* prev = &root[1];
				impl::MapNode<KeyType, ObjType>* curr = &root[0];

				impl::MapNode<KeyType, ObjType>* node = nodeAllocator->Alloc();
				std::shared_ptr<ObjType> objPtr{ std::make_shared<ObjType>(std::forward<ObjType>(obj)) };

				node->obj = objPtr;
				node->k = k;
				node->child[0] = impl::MapNodePtrInfo<KeyType, ObjType>(node, 0, 0, 1);

				auto cmp1 = impl::MapNodePtrInfo<KeyType, ObjType>(nullptr, 0, 0, 1);
				auto cmp2 = impl::MapNodePtrInfo<KeyType, ObjType>(node, 0, 0, 0);

				int dir;
				while (true) {
					dir = locate(prev, curr, k);
					if (dir == -1) {
						nodeAllocator->Free(node);

						return nullptr;
					}
					else if (dir == 2) { // it exists already
						nodeAllocator->Free(node);

						*curr->obj = obj;

						return curr->obj;
					}
					else {
						impl::MapNodePtrInfo<KeyType, ObjType> R = curr->child[dir];
						cmp1.nodeRef = R.nodeRef;
						node->child[1] = cmp1;
						node->backLink = curr;

						bool result = CAS(curr->child[dir], cmp1, cmp2);

						if (result) {
							count.fetch_add(1);
							return objPtr;
						}
						else {
							//This part is for helping
							impl::MapNodePtrInfo<KeyType, ObjType> newR = curr->child[dir];
							if (newR.nodeRef == R.nodeRef) {
								impl::MapNode<KeyType, ObjType>* newCurr = prev;
								if (newR.mark)
									cleanMark(curr, dir);
								else if (newR.flag)
									cleanFlag(curr, R.nodeRef, prev, true);

								curr = newCurr;
								prev = newCurr->backLink;
							}
						}
					}
				}
			};
			size_t Num() const {
				return (size_t)(long)count;
			};
			
			std::shared_ptr<ObjType> operator[](KeyType k) {
				impl::MapNode<KeyType, ObjType>* prev = &root[1];
				impl::MapNode<KeyType, ObjType>* curr = &root[0];
				bool result{ false };
				double epsilon = 0.5;

				if (locate(prev, curr, (double)k - epsilon) == 2) {
					return curr->obj;
				}
				else {
					return add(k, ObjType());
				}				
			};
			const std::shared_ptr<ObjType> at(KeyType k) const {
				impl::MapNode<KeyType, ObjType>* prev = &root[1];
				impl::MapNode<KeyType, ObjType>* curr = &root[0];
				bool result{ false };
				double epsilon{ 0.5 };
				if (locate(prev, curr, (double)k - epsilon) == 2) {
					return curr->obj;
				}
				else {
					return nullptr;
				}
			};
			std::shared_ptr<ObjType> at(KeyType k) {
				return operator[](k);
			};

		private:
			int locate(impl::MapNode<KeyType, ObjType>*& prev, impl::MapNode<KeyType, ObjType>*& curr, KeyType k) {
				while (true) {
					auto dir = cmp(k, curr->k);

					if (dir == 2) {
						return dir; // exact find
					}
					else {
						impl::MapNodePtrInfo<KeyType, ObjType> R = curr->child[dir];
						if (R.mark == 1 && dir == 1) {
							impl::MapNode<KeyType, ObjType>* newPrev = prev->backLink;
							//cleanMarked(prev, curr, dir); //cleanMarked function is not clear yet
							cleanMark(curr, dir);
							prev = newPrev;
							auto pDir = cmp(k, prev->k);
							impl::MapNodePtrInfo<KeyType, ObjType> temp = prev->child[pDir];
							curr = temp.nodeRef;
							continue;
						}

						if (R.thread) {
							if (dir == 0 || k < R.nodeRef->k) {
								return dir;
							}
							else {
								prev = curr;
								curr = R.nodeRef;
							}
						}
						else {
							prev = curr;
							curr = R.nodeRef;
						}
					}
				}

			};
			bool tryFlag(impl::MapNode<KeyType, ObjType>*& prev, impl::MapNode<KeyType, ObjType>*& curr, impl::MapNode<KeyType, ObjType>*& back, bool isThread) {

				std::atomic<impl::MapNodePtrInfo<KeyType, ObjType>> atomicMapNodePoKeyTypeerInfo;
				while (true) {

					auto pDir = cmp(curr->k, prev->k) & 1; //curr-> and prev->K will be same when they are poKeyTypeing to the same node. This is only possible by the threaded left-link.
					bool t = isThread;

					impl::MapNodePtrInfo<KeyType, ObjType> test = impl::MapNodePtrInfo<KeyType, ObjType>(curr, 0, 0, t);
					impl::MapNodePtrInfo<KeyType, ObjType> replace = impl::MapNodePtrInfo<KeyType, ObjType>(curr, 1, 0, t);
					//compareMapNodePoKeyTypeerInfo(prev->child[pDir], test, "inside try Flag 1");
					bool result = CAS(prev->child[pDir], test, replace);

					if (result) {
						//			prKeyTypef("tryFlag: Flagged successfully: prev->k %d, curr->k %d\n", prev->k, curr->k);
						return true;
					}
					else {

						impl::MapNodePtrInfo<KeyType, ObjType> temp = prev->child[pDir];
						if (temp.nodeRef == curr) {
							if (temp.flag)
								return false;
							else if (temp.mark)
								cleanMark(prev, pDir);
							prev = back;	//back is provided as third parameter. It helps to step back and restart.
							auto newPDir = cmp(curr->k, prev->k);
							impl::MapNodePtrInfo<KeyType, ObjType> temp = prev->child[newPDir];
							impl::MapNode<KeyType, ObjType>* newCurr = temp.nodeRef;
							locate(prev, newCurr, curr->k);
							if (newCurr != curr)
								return false;
							prev = prev->backLink;
						}
					}

				}
			};
			void tryMark(impl::MapNode<KeyType, ObjType>* curr, int dir) {

				while (true) {
					impl::MapNode<KeyType, ObjType>* back = curr->backLink;
					impl::MapNodePtrInfo<KeyType, ObjType> next = curr->child[dir];
					if (next.mark)
						break;
					else if (next.flag) {
						if (!next.thread) {
							cleanFlag(curr, next.nodeRef, back, false);
							continue;
						}
						else if (next.thread && dir) {
							cleanFlag(curr, next.nodeRef, back, true);
							continue;
						}
					}

					bool result = CAS(curr->child[dir], impl::MapNodePtrInfo<KeyType, ObjType>(next.nodeRef, 0, 0, next.thread), impl::MapNodePtrInfo<KeyType, ObjType>(next.nodeRef, 0, 1, next.thread));
					if (result)
						break;

				}

			};
			void cleanFlag(impl::MapNode<KeyType, ObjType>*& prev, impl::MapNode<KeyType, ObjType>*& curr, impl::MapNode<KeyType, ObjType>*& back, bool isThread) {
				std::atomic<impl::MapNodePtrInfo<KeyType, ObjType>> atomicMapNodePoKeyTypeerInfo;
				if (isThread) {
					while (true) {
						impl::MapNodePtrInfo<KeyType, ObjType> next = curr->child[1];
						if (next.mark)
							break;
						else if (next.flag) {

							if (back == next.nodeRef)
								back = back->backLink;

							impl::MapNode<KeyType, ObjType>* backMapNode = curr->backLink;
							cleanFlag(curr, next.nodeRef, backMapNode, next.thread);

							if (back == next.nodeRef) {
								auto pDir = cmp(prev->k, backMapNode->k);
								//prev = back->child[pDir];
								impl::MapNode<KeyType, ObjType>* back_temp = back;
								impl::MapNodePtrInfo<KeyType, ObjType> back_child = back_temp->child[pDir];
								prev = back_child.nodeRef;
							}
						}
						else {
							if (curr->preLink != prev) //step 2: set the prelink.
								curr->preLink = prev;

							//step: 3: mark the outgoing right link
							bool result = CAS(curr->child[1], impl::MapNodePtrInfo<KeyType, ObjType>(next.nodeRef, 0, 0, next.thread), impl::MapNodePtrInfo<KeyType, ObjType>(next.nodeRef, 0, 1, next.thread));

							impl::MapNodePtrInfo<KeyType, ObjType> temp = curr->child[1];
							if (result) {

								//					puts("CleanFlag 1: successfully marked right link");
								//					prKeyTypef("curr->k %d, temp.nodeRef->k %d, temp.mark %d\n", curr->k, temp.nodeRef->k, temp.mark);
								//					exit(0);
								break;
							}


						}
					}
					cleanMark(curr, 1);
				}
				else {

					impl::MapNodePtrInfo<KeyType, ObjType> right = curr->child[1];
					//		prKeyTypef("inside cleanFlag2: prev->k %d, curr->k %d, right.nodeRef->k %d, right.flag %d, right.mark %d, right.thread %d\n",
					//				prev->k, curr->k, right.nodeRef->k, right.flag, right.mark, right.thread );
							//exit(0);
					if (right.mark) {
						impl::MapNodePtrInfo<KeyType, ObjType> left = curr->child[0];
						impl::MapNode<KeyType, ObjType>* preMapNode = curr->preLink;

						if (left.nodeRef != preMapNode) { //this is cat 3 node
			//				puts("Clean flag 3: Entered to step 6");
							//exit(0);
							tryMark(curr, 0);
							cleanMark(curr, 0);
						}
						else {
							auto pDir = cmp(curr->k, prev->k);
							if (left.nodeRef == curr) { //cat 1 node

								CAS(prev->child[pDir], impl::MapNodePtrInfo<KeyType, ObjType>(curr, 1, 0, 0), impl::MapNodePtrInfo<KeyType, ObjType>(right.nodeRef, 0, 0, right.thread)); //what is f.

								if (!right.thread) {
									right.nodeRef->backLink.compare_exchange_weak(curr, prev);
								}
								//					prKeyTypef("removed %d\n",curr->k);
							}
							else {


								bool result = CAS(preMapNode->child[1], impl::MapNodePtrInfo<KeyType, ObjType>(curr, 1, 0, 1), impl::MapNodePtrInfo<KeyType, ObjType>(right.nodeRef, 0, 0, right.thread));
								//					prKeyTypef("swap success1: %d\n", result);

								if (!right.thread) {
									result = right.nodeRef->backLink.compare_exchange_strong(curr, prev);
									//						prKeyTypef("swap success2: %d\n", result);
								}


								result = CAS(prev->child[pDir], impl::MapNodePtrInfo<KeyType, ObjType>(curr, 1, 0, 0), impl::MapNodePtrInfo<KeyType, ObjType>(preMapNode, 0, 0, right.thread));
								//					prKeyTypef("swap success3: %d\n", result);

								result = preMapNode->backLink.compare_exchange_strong(curr, prev);
								//					prKeyTypef("swap success4: %d\n", result);
								//					std::cout<<"removed: "<<curr->k<<'\n';
							}
						}
					}
					else if (right.thread && right.flag) {
						impl::MapNode<KeyType, ObjType>* delMapNode = right.nodeRef;
						impl::MapNode<KeyType, ObjType>* parent = delMapNode->backLink;
						while (true) {

							auto pDir = cmp(delMapNode->k, parent->k); //changed from paper
							impl::MapNodePtrInfo<KeyType, ObjType> temp = parent->child[pDir];
							if (temp.mark)
								cleanMark(parent, pDir);
							else if (temp.flag)
								break;
							else if (CAS(parent->child[pDir], impl::MapNodePtrInfo<KeyType, ObjType>(delMapNode, 0, 0, 0), impl::MapNodePtrInfo<KeyType, ObjType>(delMapNode, 1, 0, 0))) {
								//					puts("cleanFlag 4: step 5 done. Parent link of the node to be deleted flagged successfully");
								break;
							}

						}

						impl::MapNode<KeyType, ObjType>* backMapNode = parent->backLink;

						//			prKeyTypef("parent->k %d, delMapNode->k %d, backMapNode->k %d\n", parent->k, delMapNode->k, backMapNode->k);
									//exit(0);
						cleanFlag(parent, delMapNode, backMapNode, false); //changed to false. I think, "true" was a mistake.
					}
				}
			};;
			void cleanMark(impl::MapNode<KeyType, ObjType>*& curr, KeyType markDir) {

				std::atomic<impl::MapNodePtrInfo<KeyType, ObjType>> atomicMapNodePoKeyTypeerInfo;
				impl::MapNodePtrInfo<KeyType, ObjType> left = curr->child[0];
				impl::MapNodePtrInfo<KeyType, ObjType> right = curr->child[1];
				//	puts("inside clean Mark:");
				//	prKeyTypef("curr %d, marDir %d, curr->child[0]->k %d, curr->child[1]->k: %d\n", curr->k, markDir, left.nodeRef->k, right.nodeRef->k);

				if (markDir) {

					//KeyType pDir = markDir;
					impl::MapNode<KeyType, ObjType>* delMapNode = curr; //Not sure
					while (true) {

						impl::MapNode<KeyType, ObjType>* preMapNode = delMapNode->preLink;
						//			std::cout<<"preMapNode->k :"<<preMapNode->k<<'\n';

						impl::MapNode<KeyType, ObjType>* parent = delMapNode->backLink;
						auto pDir = cmp(delMapNode->k, parent->k);
						impl::MapNodePtrInfo<KeyType, ObjType> parent_child = parent->child[pDir];

						//step 4 for category 1 and 2. Acutally step 5: flag the incoming parent link
						if (preMapNode == left.nodeRef) { //category 1 or 2 node.
			//				puts("clearn mark: inside cat 1 and 2");

							impl::MapNode<KeyType, ObjType>* parent = delMapNode->backLink;
							impl::MapNode<KeyType, ObjType>* back = parent->backLink;
							//				prKeyTypef("delMapNode->k %d, parent->k %d, back-> %d\n", delMapNode->k, parent->k, back->k);
							tryFlag(parent, curr, back, false); // why threaded? So, I changed it to non-threaded

			//				prKeyTypef("Clean Mark 4: parent_child.nodeRef %d, curr %d, parent_child->k %d, curr->k %d\n", parent_child.nodeRef, curr, parent_child.nodeRef->k, curr->k);
							if (parent_child.nodeRef == curr) { // If the link still persists
								cleanFlag(parent, curr, back, false);
								break;
							}
						}
						else {
							//category 3 node.
							//This step 4 for category 3 node. Step 4: flag the incoming parent link of the predecessor.
							impl::MapNode<KeyType, ObjType>* preParent = preMapNode->backLink;
							impl::MapNodePtrInfo<KeyType, ObjType> temp = preParent->child[1]; // child[1] because predecessor of cat3 node is always the right child of it's parent.
							impl::MapNode<KeyType, ObjType>* backMapNode = preParent->backLink;

							//				prKeyTypef("clean Mark 2: preMapNode->k %d, preParent->k %d, backMapNode->k %d\n", preMapNode->k , preParent->k, backMapNode->k );
							//				prKeyTypef("clean Mark 3: temp.noderef->k: %d, temp.flag: %d, temp.mark: %d, temp.thread: %d\n", temp.nodeRef->k, temp.flag, temp.mark, temp.thread);


							if (temp.mark)
								cleanMark(preParent, 1);
							else if (temp.flag) {
								cleanFlag(preParent, preMapNode, backMapNode, false); //changed isThreaded parameter to false
								break;
							}
							else if (CAS(preParent->child[pDir], impl::MapNodePtrInfo<KeyType, ObjType>(preMapNode, 0, 0, 0), impl::MapNodePtrInfo<KeyType, ObjType>(preMapNode, 1, 0, 0))) { //
			//					prKeyTypef("incoming parentlink pred successfully flagged\n");

								cleanFlag(preParent, preMapNode, backMapNode, false); //changed isThreaded parameter to false.
								break;
							}
						}
					}
				}
				else {
					if (right.mark) {

						impl::MapNode<KeyType, ObjType>* preMapNode = curr->preLink;
						tryMark(preMapNode, 0);
						cleanMark(preMapNode, 0);
					}
					else if (right.thread && right.flag) {

						impl::MapNode<KeyType, ObjType>* delMapNode = right.nodeRef;
						impl::MapNode<KeyType, ObjType>* delMapNodePa = delMapNode->backLink;
						impl::MapNode<KeyType, ObjType>* preParent = curr->backLink;
						auto pDir = cmp(delMapNode->k, delMapNodePa->k);
						impl::MapNodePtrInfo<KeyType, ObjType> delMapNodeL = delMapNode->child[0];
						impl::MapNodePtrInfo<KeyType, ObjType> delMapNodeR = delMapNode->child[1];

						KeyType res1 = -1, res2 = -1, res3 = -1, res4 = -1, res5 = -1, res6 = -1, res7 = -1, res8 = -1;

						res1 = CAS(preParent->child[1], impl::MapNodePtrInfo<KeyType, ObjType>(curr, 1, 0, 0), impl::MapNodePtrInfo<KeyType, ObjType>(left.nodeRef, left.flag, 0, left.thread));

						if (!left.thread) {
							res2 = left.nodeRef->backLink.compare_exchange_weak(curr, preParent);

						}

						res3 = CAS(curr->child[0], impl::MapNodePtrInfo<KeyType, ObjType>(left.nodeRef, 0, 1, left.thread), impl::MapNodePtrInfo<KeyType, ObjType>(delMapNodeL.nodeRef, 0, 0, 0));

						res4 = delMapNodeL.nodeRef->backLink.compare_exchange_strong(delMapNode, curr);

						res5 = CAS(curr->child[1], impl::MapNodePtrInfo<KeyType, ObjType>(right.nodeRef, 1, 0, 1), impl::MapNodePtrInfo<KeyType, ObjType>(delMapNodeR.nodeRef, 0, 0, delMapNodeR.thread));

						if (!delMapNodeR.thread) {
							res6 = delMapNodeR.nodeRef->backLink.compare_exchange_strong(delMapNode, curr);

						}

						res7 = CAS(delMapNodePa->child[pDir], impl::MapNodePtrInfo<KeyType, ObjType>(delMapNode, 1, 0, 0), impl::MapNodePtrInfo<KeyType, ObjType>(curr, 0, 0, 0));

						res8 = curr->backLink.compare_exchange_strong(preParent, delMapNodePa);

					}
				}
			};
			static int cmp(KeyType const& x, KeyType const& y) {
				if (x == y)
					return 2;
				else if (x > y)
					return 1;
				else
					return 0;
			};
			bool CAS(std::atomic<impl::MapNodePtrInfo<KeyType, ObjType>>& oldValue, impl::MapNodePtrInfo<KeyType, ObjType> newValue, impl::MapNodePtrInfo<KeyType, ObjType> replacement) {
				bool result{ false };
				for (int i = 0; i < TIMES && !result; i++) {
					result = oldValue.compare_exchange_strong(newValue, replacement);
				}
				return result;
			};

			std::atomic<long> count;
			std::shared_ptr < utilities::Allocator<impl::MapNode<KeyType, ObjType>, 10> > nodeAllocator;
			impl::MapNode<KeyType, ObjType> root[2];
			static constexpr int TIMES = 4;
		};








		template< class objType, class keyType, int maxChildrenPerNode = 10, bool ForceObjectPOD = false>
		class Tree {
		public:
#define LOCK_NODE_CONCAT_(a, b) a##b
#define LOCK_NODE_CONCAT(a, b) LOCK_NODE_CONCAT_(a, b)
#define LOCK_NODE(node) decltype(auto) LOCK_NODE_CONCAT(scopelock_, __LINE__) { std::scoped_lock((node)->lock) }
#define READ_TREE() decltype(auto) LOCK_NODE_CONCAT(scopelock_, __LINE__) { std::shared_lock(this->lock) }
#define WRITE_TREE() decltype(auto) LOCK_NODE_CONCAT(scopelock_, __LINE__) { std::scoped_lock(this->lock) }
			struct TreeNode {
				fibers::synchronization::atomic_number<int> 
					lock;                       // atomic lock that can be quickly "forgotten" without a destructor
				keyType	  
					key;						// key used for sorting
				objType*  
					object;						// if != NULL pointer to object stored in leaf node
				TreeNode*
					parent;                     // parent node
				TreeNode*
					next;						// next sibling
				TreeNode*
					prev;						// prev sibling
				fibers::synchronization::atomic_number<int>
					numChildren;				// number of children
				TreeNode* 
					firstChild;					// first child
				TreeNode* 
					lastChild;					// last child
			};
			__forceinline static TreeNode* InitNode(TreeNode* p) {
				if (p) {
					LOCK_NODE(p);

					p->key = 0;
					p->object = nullptr;
					p->parent = nullptr;
					p->next = nullptr;
					p->prev = nullptr;
					p->numChildren = 0;
					p->firstChild = nullptr;
					p->lastChild = nullptr;
				}
				return p;				
			};

		public:
			typedef TreeNode _iterType;
			struct it_state {
				_iterType _node;
				_iterType* node = &_node;
				inline void begin(const Tree* ref) {
					{
						READ_TREE();
						node = ref->root;
					}
					node = ref->GetNextLeaf(node);
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
			SETUP_STL_ITERATOR(Tree, _iterType, it_state);

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

		protected:
			mutable std::shared_mutex // fibers::synchronization::CriticalMutexLock
				lock; // actual lock
			fibers::synchronization::atomic_ptr<TreeNode>
				root; // root
			utilities::Allocator<objType, maxChildrenPerNode, ForceObjectPOD>
				objAllocator; // thread-safe
			utilities::Allocator<TreeNode, maxChildrenPerNode, true>
				nodeAllocator; // thread-safe, forced to be POD (quickly forgets allocations)

		public:
			Tree() : root(nullptr), objAllocator(), nodeAllocator() {
				static_assert(maxChildrenPerNode >= 4);
				Init();
			};
			Tree(int toReserve) :
				root(nullptr),
				objAllocator(toReserve),
				nodeAllocator(toReserve * 1.25)
			{
				static_assert(maxChildrenPerNode >= 4);
				Init();
			};
			Tree(const Tree& obj) = delete;
			Tree(Tree&& obj) = delete;
			Tree& operator=(const Tree& obj) = delete;
			Tree& operator=(Tree&& obj) = delete;
			~Tree() { Shutdown(); };

			void Reserve(long long num) {
				objAllocator.Reserve(num);
				nodeAllocator.Reserve(num * 1.25); // approximately 25% more for 'overage'
			};

			// Thread-safe. Adds an object/key pair to the tree.
			TreeNode* Add(objType const& object, keyType const& key, bool addUnique = true) {
				TreeNode* node, * child, * newNode; objType* OBJ;

				// Ensure the root exists (thread-safe due to blocking)
				if (root == nullptr) {
					this->lock.lock();
				}
				if (root == nullptr) {
					root = AllocNode();
					this->lock.unlock();
				}

				// Check that the key does not already exist (thread-safe)
				if (addUnique) {
					node = NodeFind(key);
					if (node && node->object) {
						*node->object = const_cast<objType&>(object);
						return node;
					}
				}

				// Split the root if needed (thread-safe)
				{
					fibers::synchronization::atomic_number<int>* rootLock{ nullptr };
					if (!rootLock && root->numChildren >= maxChildrenPerNode) {
						this->lock.lock();
						rootLock = &root->lock;
						rootLock->lock();						
					}
					if (rootLock && root->numChildren >= maxChildrenPerNode) {
						newNode = AllocNode();
						LOCK_NODE(newNode);
						newNode->key = root->key;
						newNode->firstChild = root;
						newNode->lastChild = root;
						newNode->numChildren = 1;

						SplitNode<false>(root, newNode);
						{
							
							root = newNode;
							rootLock->unlock();
							this->lock.unlock();
						}
					}
				}

				newNode = AllocNode();
				LOCK_NODE(newNode);
				newNode->key = key;
				OBJ = objAllocator.Alloc(); {
					*OBJ = const_cast<objType&>(object);
				}
				newNode->object = OBJ;

				READ_TREE();

				for (node = root.Get(); node->firstChild != nullptr; node = child) {
					LOCK_NODE(node);

					if (key > node->key) node->key = key;
					
					// find the first child with a key larger equal to the key of the new node
					child = node->firstChild;
					{
						auto* childLock = &child->lock;
						childLock->lock();
						while (child->next) {
							if (key <= child->key) {
								break;
							}

							child = child->next;
							child->lock.lock();
							childLock->unlock();
							childLock = &child->lock;
						}
						childLock->unlock();
					}

					LOCK_NODE(child);
					{
						if (child->object) {
							newNode->parent = node;

							if (key <= child->key) {
								// insert new node before child
								if (child->prev) {
									LOCK_NODE(child->prev);

									newNode->prev = child->prev;
									newNode->next = child;
									child->prev->next = newNode;
									child->prev = newNode;
								}
								else {
									newNode->prev = child->prev;
									newNode->next = child;
									node->firstChild = newNode;
									child->prev = newNode;
								}
							}
							else {
								// insert new node after child
								if (child->next) {
									LOCK_NODE(child->next);

									newNode->prev = child;
									newNode->next = child->next;
									child->next->prev = newNode;
									child->next = newNode;
								}
								else {
									newNode->prev = child;
									newNode->next = child->next;									
									node->lastChild = newNode;									
									child->next = newNode;
								}								
							}
							node->numChildren++;

							return newNode;
						}

						// make sure the child has room to store another node
						if (child->numChildren >= maxChildrenPerNode) {
							SplitNode<false>(child);
							LOCK_NODE(child->prev);
							if (key <= child->prev->key) {
								child = child->prev;
							}
						}
					}
				}

				LOCK_NODE(root);
				// we only end up here if the root node is empty
				newNode->parent = root;
				root->key = key;
				root->firstChild = newNode;
				root->lastChild = newNode;
				root->numChildren++;

				return newNode;
			};

			void Add(std::vector<std::pair<keyType, objType>> const& data, bool addUnique = true) { for (auto& source : data) Add(source.second, source.first, addUnique); };
			void Remove(TreeNode* node) {
				if (!node) return;

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
						// PARENT SHOULD ALREADY BE LOCKED
						// REMOVE THIS LOCK WHEN WE GET TO UPDATING THIS SECTION OF CODE
						LOCK_NODE(parent);
						SplitNode<true>(parent); // mark this false if the parent's parent is locked already
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
				FreeNode(node); // Non-locking.

				// remove the root node if it has a single internal node as child
				if (root->numChildren == 1 && root->firstChild->object == nullptr) {
					oldRoot = root;
					root->firstChild->parent = nullptr;
					root = root->firstChild;
					FreeNode(oldRoot); // Non-locking.
				}
			};				// remove an object node from the tree

			TreeNode* NodeFindByIndex(int index) const {
				if (index < 0) return nullptr;
				else if (index >= GetNodeCount()) return nullptr;
				else {
					READ_TREE();
					return NodeFindByIndex(index, const_cast<TreeNode*>(root.Get()));
				}
			};
			TreeNode* NodeFind(keyType  const& key) const {
				READ_TREE();
				return NodeFind(key, const_cast<TreeNode*>(root.Get()));
			};								// find an object using the given key;
			TreeNode* NodeFindSmallestLargerEqual(keyType const& key) const {
				READ_TREE();
				return NodeFindSmallestLargerEqual(key, const_cast<TreeNode*>(root.Get()));
			};			// find an object with the smallest key larger equal the given key;
			TreeNode* NodeFindLargestSmallerEqual(keyType const& key) const {
				READ_TREE();
				return NodeFindLargestSmallerEqual(key, const_cast<TreeNode*>(root.Get()));
			};			// find an object with the largest key smaller equal the given key;
			objType* Find(keyType  const& key) const {
				READ_TREE();
				TreeNode* node = NodeFind(key, root.Get());
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};									// find an object using the given key;
			objType* FindSmallestLargerEqual(keyType const& key) const {
				READ_TREE();
				TreeNode* node = NodeFindSmallestLargerEqual(key, root.Get());
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};				// find an object with the smallest key larger equal the given key;
			objType* FindLargestSmallerEqual(keyType const& key) const {
				READ_TREE();
				TreeNode* node = NodeFindLargestSmallerEqual(key, root.Get());
				if (node == nullptr) {
					return nullptr;
				}
				else {
					return node->object;
				}
			};				// find an object with the largest key smaller equal the given key;

		public:
			// Thread-safe. Returns the number of allocated objects.
			long long GetNodeCount() const { return objAllocator.GetAllocCount(); };
			// Thread-safe. Returns the reservation size of the object allocator.
			long long GetReservedCount() const { return objAllocator.GetTotalCount(); };

		protected:
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
			};		// goes through all nodes of the tree;
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

		protected:
			// Thread-safe. Initializes the tree and allocators.
			void	  Init() {
				WRITE_TREE();

				root = AllocNode();
				objAllocator.Free(objAllocator.Alloc());
			};
			// Thread-safe. Shutsdown the allocators. Need re-initialization to use again.
			void	  Shutdown() {
				WRITE_TREE();

				//nodeAllocator.Clear();
				//objAllocator.Clear();
				root = nullptr;
			};
			// Thread-safe. Allocates and initializes a tree node (not an object)
			TreeNode* AllocNode() { return InitNode(nodeAllocator.Alloc()); };
			// Thread-safe. Non-locking.
			void      FreeNode(TreeNode* node) {
				if (node && node->object) {
					objAllocator.Free(node->object);
				}
				nodeAllocator.Free(node);
			};
			// Thread-safe. Locks the shared parent, node1, and node2. Node1 will be deleted. Node2 will be returned. All children of Node1 will be moved to Node2.
			TreeNode* MergeNodes(TreeNode* node1, TreeNode* node2) {
				assert(node1->parent == node2->parent);
				assert(node1->next == node2 && node2->prev == node1);
				assert(node1->object == nullptr && node2->object == nullptr);
				assert(node1->numChildren >= 1 && node2->numChildren >= 1);

				LOCK_NODE(node1);
				LOCK_NODE(node2);
				LOCK_NODE(node1->parent);

				TreeNode* child = node1->firstChild;
				if (child) {
					child->lock.lock();
					while (child->next) {			
						child->parent = node2;
						child->lock.unlock();
						child = child->next;
						child->lock.lock();
					}
					child->parent = node2;
					child->next = node2->firstChild;
					child->lock.unlock();
				}

				{
					LOCK_NODE(node2->firstChild);
					node2->firstChild->prev = child;
					node2->firstChild = node1->firstChild;
				}
				node2->numChildren += node1->numChildren;

				// unlink the first node from the parent
				if (node1->prev) {
					LOCK_NODE(node1->prev);
					node1->prev->next = node2;
				}
				else {
					node1->parent->firstChild = node2;
				}
				node2->prev = node1->prev;
				node2->parent->numChildren--;

				FreeNode(node1); // Non-locking.

				return node2;
			};
			// Thread-safe. Optionally locks the node's parent. Node should already be locked. A new node is created and the children are distributed among them.
			template <bool LockParent = true>
			void	  SplitNode(TreeNode* node, TreeNode* newParent = nullptr) {
				// allocate a new node
				TreeNode* newNode{ AllocNode() };

				LOCK_NODE(newNode);

				long long 
					i;
				TreeNode* 
					child;

				if (node->prev) {
					LOCK_NODE(node->prev);

					// Set the parents
					if (newParent) node->parent = newParent;
					newNode->parent = node->parent;

					if constexpr (LockParent) {
						LOCK_NODE(LockParent);

						node->prev->next = newNode;

						// divide the children over the two nodes
						child = node->firstChild;
						if (child) {
							auto* thisChildLock = &child->lock;
							thisChildLock->lock(); {
								child->parent = newNode;
								for (i = 3; i < node->numChildren; i += 2) {
									child = child->next;
									child->lock.lock();
									child->parent = newNode;
									thisChildLock->unlock();
									thisChildLock = &child->lock;
								}
								LOCK_NODE(child->next);

								newNode->key = child->key;
								newNode->numChildren = node->numChildren / 2;
								newNode->firstChild = node->firstChild;
								newNode->lastChild = child;

								node->numChildren -= newNode->numChildren;
								node->firstChild = child->next;

								child->next->prev = nullptr;
								child->next = nullptr;
							}
							thisChildLock->unlock();
						}

						newNode->prev = node->prev;
						newNode->next = node;
						node->prev = newNode;
						node->parent->numChildren++;
					}
					else {
						node->prev->next = newNode;

						// divide the children over the two nodes
						child = node->firstChild;
						if (child) {
							auto* thisChildLock = &child->lock;
							thisChildLock->lock(); {
								child->parent = newNode;
								for (i = 3; i < node->numChildren; i += 2) {
									child = child->next;
									child->lock.lock();
									child->parent = newNode;
									thisChildLock->unlock();
									thisChildLock = &child->lock;
								}
								LOCK_NODE(child->next);

								newNode->key = child->key;
								newNode->numChildren = node->numChildren / 2;
								newNode->firstChild = node->firstChild;
								newNode->lastChild = child;

								node->numChildren -= newNode->numChildren;
								node->firstChild = child->next;

								child->next->prev = nullptr;
								child->next = nullptr;
							}
							thisChildLock->unlock();
						}

						newNode->prev = node->prev;
						newNode->next = node;
						node->prev = newNode;
						node->parent->numChildren++;
					}
				}
				else {
					// Set the parents
					if (newParent) node->parent = newParent;
					newNode->parent = node->parent;

					if constexpr (LockParent) {
						LOCK_NODE(LockParent);

						node->parent->firstChild = newNode;

						// divide the children over the two nodes
						child = node->firstChild;
						if (child) {
							auto* thisChildLock = &child->lock;
							thisChildLock->lock(); {
								child->parent = newNode;
								for (i = 3; i < node->numChildren; i += 2) {
									child = child->next;
									child->lock.lock();
									child->parent = newNode;
									thisChildLock->unlock();
									thisChildLock = &child->lock;
								}
								LOCK_NODE(child->next);

								newNode->key = child->key;
								newNode->numChildren = node->numChildren / 2;
								newNode->firstChild = node->firstChild;
								newNode->lastChild = child;

								node->numChildren -= newNode->numChildren;
								node->firstChild = child->next;

								child->next->prev = nullptr;
								child->next = nullptr;
							}
							thisChildLock->unlock();
						}

						newNode->prev = node->prev;
						newNode->next = node;
						node->prev = newNode;
						node->parent->numChildren++;
					}
					else {
						node->parent->firstChild = newNode;

						// divide the children over the two nodes
						child = node->firstChild;
						if (child) {
							auto* thisChildLock = &child->lock;
							thisChildLock->lock(); {
								child->parent = newNode;
								for (i = 3; i < node->numChildren; i += 2) {
									child = child->next;
									child->lock.lock();
									child->parent = newNode;
									thisChildLock->unlock();
									thisChildLock = &child->lock;
								}
								LOCK_NODE(child->next);

								newNode->key = child->key;
								newNode->numChildren = node->numChildren / 2;
								newNode->firstChild = node->firstChild;
								newNode->lastChild = child;

								node->numChildren -= newNode->numChildren;
								node->firstChild = child->next;

								child->next->prev = nullptr;
								child->next = nullptr;
							}
							thisChildLock->unlock();
						}

						newNode->prev = node->prev;
						newNode->next = node;
						node->prev = newNode;
						node->parent->numChildren++;
					}
				}
			};

#undef LOCK_NODE
#undef LOCK_NODE_CONCAT
#undef LOCK_NODE_CONCAT_
		};



	};

#if 0
	namespace containers {
		 // Setting MAX_LEVELS to 1 essentially makes this data structure the Harris-Michael lock-free list (see list.c).
#define CACHE_LINE_SIZE  64 // 64 byte cache line on x86 and x86-64
#define CACHE_LINE_SCALE 6  // log base 2 of the cache line size

#define EXPECT_TRUE(x)      __builtin_expect(!!(x), 1)
#define EXPECT_FALSE(x)     __builtin_expect(!!(x), 0)

#ifndef NBD_SINGLE_THREADED

#define MAX_NUM_THREADS  32 // make this whatever you want, but make it a power of 2

#define SYNC_SWAP(addr,x)         __sync_lock_test_and_set(addr,x)
#define SYNC_CAS(addr,old,x)      __sync_val_compare_and_swap(addr,old,x)
#define SYNC_ADD(addr,n)          __sync_add_and_fetch(addr,n)
#define SYNC_FETCH_AND_OR(addr,x) __sync_fetch_and_or(addr,x)
#else// NBD_SINGLE_THREADED

#define MAX_NUM_THREADS  1

#define SYNC_SWAP(addr,x)         ({ typeof(*(addr)) _old = *(addr); *(addr)  = (x); _old; })
#define SYNC_CAS(addr,old,x)      ({ typeof(*(addr)) _old = *(addr); *(addr)  = (x); _old; })
//#define SYNC_CAS(addr,old,x)    ({ typeof(*(addr)) _old = *(addr); if ((old) == _old) { *(addr)  = (x); } _old; })
#define SYNC_ADD(addr,n)          ({ typeof(*(addr)) _old = *(addr); *(addr) += (n); _old; })
#define SYNC_FETCH_AND_OR(addr,x) ({ typeof(*(addr)) _old = *(addr); *(addr) |= (x); _old; })

#endif//NBD_SINGLE_THREADED

#define COUNT_TRAILING_ZEROS __builtin_ctz

#define MASK(n)     ((1ULL << (n)) - 1)

#define TRUE  1
#define FALSE 0

#ifdef NBD32
#define TAG1         (1U << 31)
#define TAG2         (1U << 30)
#else
#define TAG1         (1ULL << 63)
#define TAG2         (1ULL << 62)
#endif
#define TAG_VALUE(v, tag) ((v) |  tag)
#define IS_TAGGED(v, tag) ((v) &  tag)
#define STRIP_TAG(v, tag) ((v) & ~tag)

#define DOES_NOT_EXIST 0
#define ERROR_INVALID_OPTION      (-1)
#define ERROR_INVALID_ARGUMENT    (-2)
#define ERROR_UNSUPPORTED_FEATURE (-3)
#define ERROR_TXN_NOT_RUNNING     (-4)

#define VOLATILE_DEREF(x) (*((volatile typeof(x))(x)))

		//typedef unsigned long long uint64_t;
		//typedef unsigned int       uint32_t;
		//typedef unsigned short     uint16_t;
		//typedef unsigned char      uint8_t;

		using markable_t = size_t;

#define MAX_LEVELS 24

		using map_key_t = uint64_t;
		using map_val_t = uint64_t;

		typedef int      (*cmp_fun_t)   (void*, void*);
		typedef void* (*clone_fun_t) (void*);
		typedef uint32_t(*hash_fun_t)  (void*);

		typedef struct datatype {
			cmp_fun_t   cmp;
			hash_fun_t  hash;
			clone_fun_t clone;
		} datatype_t;

		enum unlink {
			FORCE_UNLINK,
			ASSIST_UNLINK,
			DONT_UNLINK
		};

		typedef struct node {
			map_key_t key;
			map_val_t val;
			unsigned num_levels;
			markable_t next[1];
		} node_t;

		struct sl_iter {
			node_t* next;
		};

		struct sl {
			node_t* head;
			const datatype_t* key_type;
			int high_water; // max historic number of levels
		};

		// Marking the <next> field of a node logically removes it from the list
#if 0
		static inline markable_t  MARK_NODE(node_t* x) { return TAG_VALUE((markable_t)x, 0x1); }
		static inline int        HAS_MARK(markable_t x) { return (IS_TAGGED(x, 0x1) == 0x1); }
		static inline node_t* GET_NODE(markable_t x) { assert(!HAS_MARK(x)); return (node_t*)x; }
		static inline node_t* STRIP_MARK(markable_t x) { return ((node_t*)STRIP_TAG(x, 0x1)); }
#else
#define  MARK_NODE(x) TAG_VALUE((markable_t)(x), 0x1)
#define   HAS_MARK(x) (IS_TAGGED((x), 0x1) == 0x1)
#define   GET_NODE(x) ((node_t *)(x))
#define STRIP_MARK(x) ((node_t *)STRIP_TAG((x), 0x1))
#endif

		typedef struct sl skiplist_t;
		typedef struct sl_iter sl_iter_t;








		static int random_levels(skiplist_t* sl) {
			uint64_t r = nbd_rand();
			int z = __builtin_ctz(r);
			int levels = (int)(z / 1.5);
			if (levels == 0)
				return 1;
			if (levels > sl->high_water) {
				levels = SYNC_ADD(&sl->high_water, 1);
				TRACE("s2", "random_levels: increased high water mark to %lld", sl->high_water, 0);
			}
			if (levels > MAX_LEVELS) { levels = MAX_LEVELS; }
			return levels;
		}

		static node_t* node_alloc(int num_levels, map_key_t key, map_val_t val) {
			assert(num_levels >= 0 && num_levels <= MAX_LEVELS);
			size_t sz = sizeof(node_t) + (num_levels - 1) * sizeof(node_t*);
			node_t* item = (node_t*)nbd_malloc(sz);
			memset(item, 0, sz);
			item->key = key;
			item->val = val;
			item->num_levels = num_levels;
			TRACE("s2", "node_alloc: new node %p (%llu levels)", item, num_levels);
			return item;
		}

		skiplist_t* sl_alloc(const datatype_t* key_type) {
			skiplist_t* sl = (skiplist_t*)nbd_malloc(sizeof(skiplist_t));
			sl->key_type = key_type;
			sl->high_water = 1;
			sl->head = node_alloc(MAX_LEVELS, 0, 0);
			memset(sl->head->next, 0, MAX_LEVELS * sizeof(skiplist_t*));
			return sl;
		}

		void sl_free(skiplist_t* sl) {
			node_t* item = GET_NODE(sl->head->next[0]);
			while (item) {
				node_t* next = STRIP_MARK(item->next[0]);
				if (sl->key_type != NULL) {
					nbd_free((void*)item->key);
				}
				nbd_free(item);
				item = next;
			}
		}

		size_t sl_count(skiplist_t* sl) {
			size_t count = 0;
			node_t* item = GET_NODE(sl->head->next[0]);
			while (item) {
				if (!HAS_MARK(item->next[0])) {
					count++;
				}
				item = STRIP_MARK(item->next[0]);
			}
			return count;
		}

		static node_t* find_preds(node_t** preds, node_t** succs, int n, skiplist_t* sl, map_key_t key, enum unlink unlink) {
			node_t* pred = sl->head;
			node_t* item = NULL;
			TRACE("s2", "find_preds: searching for key %p in skiplist (head is %p)", key, pred);
			int d = 0;

			// Traverse the levels of <sl> from the top level to the bottom
			for (int level = sl->high_water - 1; level >= 0; --level) {
				markable_t next = pred->next[level];
				if (next == DOES_NOT_EXIST && level >= n)
					continue;
				TRACE("s3", "find_preds: traversing level %p starting at %p", level, pred);
				if (EXPECT_FALSE(HAS_MARK(next))) {
					TRACE("s2", "find_preds: pred %p is marked for removal (next %p); retry", pred, next);
					ASSERT(level == pred->num_levels - 1 || HAS_MARK(pred->next[level + 1]));
					return find_preds(preds, succs, n, sl, key, unlink); // retry
				}
				item = GET_NODE(next);
				while (item != NULL) {
					next = item->next[level];

					// A tag means an item is logically removed but not physically unlinked yet.
					while (EXPECT_FALSE(HAS_MARK(next))) {
						TRACE("s3", "find_preds: found marked item %p (next is %p)", item, next);
						if (unlink == DONT_UNLINK) {

							// Skip over logically removed items.
							item = STRIP_MARK(next);
							if (EXPECT_FALSE(item == NULL))
								break;
							next = item->next[level];
						}
						else {

							// Unlink logically removed items.
							markable_t other = SYNC_CAS(&pred->next[level], (markable_t)item, (markable_t)STRIP_MARK(next));
							if (other == (markable_t)item) {
								TRACE("s3", "find_preds: unlinked item from pred %p", pred, 0);
								item = STRIP_MARK(next);
							}
							else {
								TRACE("s3", "find_preds: lost race to unlink item pred %p's link changed to %p", pred, other);
								if (HAS_MARK(other))
									return find_preds(preds, succs, n, sl, key, unlink); // retry
								item = GET_NODE(other);
							}
							next = (item != NULL) ? item->next[level] : DOES_NOT_EXIST;
						}
					}

					if (EXPECT_FALSE(item == NULL)) {
						TRACE("s3", "find_preds: past the last item in the skiplist", 0, 0);
						break;
					}

					TRACE("s4", "find_preds: visiting item %p (next is %p)", item, next);
					TRACE("s4", "find_preds: key %p val %p", STRIP_MARK(item->key), item->val);

					if (EXPECT_TRUE(sl->key_type == NULL)) {
						d = item->key - key;
					}
					else {
						d = sl->key_type->cmp((void*)item->key, (void*)key);
					}

					if (d > 0)
						break;
					if (d == 0 && unlink != FORCE_UNLINK)
						break;

					pred = item;
					item = GET_NODE(next);
				}

				TRACE("s3", "find_preds: found pred %p next %p", pred, item);

				if (level < n) {
					if (preds != NULL) {
						preds[level] = pred;
					}
					if (succs != NULL) {
						succs[level] = item;
					}
				}
			}

			if (d == 0) {
				TRACE("s2", "find_preds: found matching item %p in skiplist, pred is %p", item, pred);
				return item;
			}
			TRACE("s2", "find_preds: found proper place for key %p in skiplist, pred is %p. returning null", key, pred);
			return NULL;
		}

		// Fast find that does not help unlink partially removed nodes and does not return the node's predecessors.
		map_val_t sl_lookup(skiplist_t* sl, map_key_t key) {
			TRACE("s1", "sl_lookup: searching for key %p in skiplist %p", key, sl);
			node_t* item = find_preds(NULL, NULL, 0, sl, key, DONT_UNLINK);

			// If we found an <item> matching the <key> return its value.
			if (item != NULL) {
				map_val_t val = item->val;
				if (val != DOES_NOT_EXIST) {
					TRACE("s1", "sl_lookup: found item %p. val %p. returning item", item, item->val);
					return val;
				}
			}

			TRACE("s1", "sl_lookup: no item in the skiplist matched the key", 0, 0);
			return DOES_NOT_EXIST;
		}

		map_key_t sl_min_key(skiplist_t* sl) {
			node_t* item = GET_NODE(sl->head->next[0]);
			while (item != NULL) {
				markable_t next = item->next[0];
				if (!HAS_MARK(next))
					return item->key;
				item = STRIP_MARK(next);
			}
			return DOES_NOT_EXIST;
		}

		static map_val_t update_item(node_t* item, map_val_t expectation, map_val_t new_val) {
			map_val_t old_val = item->val;

			// If the item's value is DOES_NOT_EXIST it means another thread removed the node out from under us.
			if (EXPECT_FALSE(old_val == DOES_NOT_EXIST)) {
				TRACE("s2", "update_item: lost a race to another thread removing the item. retry", 0, 0);
				return DOES_NOT_EXIST; // retry
			}

			if (EXPECT_FALSE(expectation == CAS_EXPECT_DOES_NOT_EXIST)) {
				TRACE("s1", "update_item: the expectation was not met; the skiplist was not changed", 0, 0);
				return old_val; // failure
			}

			// Use a CAS and not a SWAP. If the CAS fails it means another thread removed the node or updated its
			// value. If another thread removed the node but it is not unlinked yet and we used a SWAP, we could
			// replace DOES_NOT_EXIST with our value. Then another thread that is updating the value could think it
			// succeeded and return our value even though it should return DOES_NOT_EXIST.
			if (old_val == SYNC_CAS(&item->val, old_val, new_val)) {
				TRACE("s1", "update_item: the CAS succeeded. updated the value of the item", 0, 0);
				return old_val; // success
			}
			TRACE("s2", "update_item: lost a race. the CAS failed. another thread changed the item's value", 0, 0);

			// retry
			return update_item(item, expectation, new_val); // tail call
		}

		map_val_t sl_cas(skiplist_t* sl, map_key_t key, map_val_t expectation, map_val_t new_val) {
			TRACE("s1", "sl_cas: key %p skiplist %p", key, sl);
			TRACE("s1", "sl_cas: expectation %p new value %p", expectation, new_val);
			ASSERT((int64_t)new_val > 0);

			node_t* preds[MAX_LEVELS];
			node_t* nexts[MAX_LEVELS];
			node_t* new_item = NULL;
			int n = random_levels(sl);
			node_t* old_item = find_preds(preds, nexts, n, sl, key, ASSIST_UNLINK);

			// If there is already an item in the skiplist that matches the key just update its value.
			if (old_item != NULL) {
				map_val_t ret_val = update_item(old_item, expectation, new_val);
				if (ret_val != DOES_NOT_EXIST)
					return ret_val;

				// If we lose a race with a thread removing the item we tried to update then we have to retry.
				return sl_cas(sl, key, expectation, new_val); // tail call
			}

			if (EXPECT_FALSE(expectation != CAS_EXPECT_DOES_NOT_EXIST && expectation != CAS_EXPECT_WHATEVER)) {
				TRACE("s1", "sl_cas: the expectation was not met, the skiplist was not changed", 0, 0);
				return DOES_NOT_EXIST; // failure, the caller expected an item for the <key> to already exist
			}

			// Create a new node and insert it into the skiplist.
			TRACE("s3", "sl_cas: attempting to insert a new item between %p and %p", preds[0], nexts[0]);
			map_key_t new_key = sl->key_type == NULL ? key : (map_key_t)sl->key_type->clone((void*)key);
			new_item = node_alloc(n, new_key, new_val);

			// Set <new_item>'s next pointers to their proper values
			markable_t next = new_item->next[0] = (markable_t)nexts[0];
			for (int level = 1; level < new_item->num_levels; ++level) {
				new_item->next[level] = (markable_t)nexts[level];
			}

			// Link <new_item> into <sl> from the bottom level up. After <new_item> is inserted into the bottom level
			// it is officially part of the skiplist.
			node_t* pred = preds[0];
			markable_t other = SYNC_CAS(&pred->next[0], next, (markable_t)new_item);
			if (other != next) {
				TRACE("s3", "sl_cas: failed to change pred's link: expected %p found %p", next, other);

				// Lost a race to another thread modifying the skiplist. Free the new item we allocated and retry.
				if (sl->key_type != NULL) {
					nbd_free((void*)new_key);
				}
				nbd_free(new_item);
				return sl_cas(sl, key, expectation, new_val); // tail call
			}

			TRACE("s3", "sl_cas: successfully inserted a new item %p at the bottom level", new_item, 0);

			ASSERT(new_item->num_levels <= MAX_LEVELS);
			for (int level = 1; level < new_item->num_levels; ++level) {
				TRACE("s3", "sl_cas: inserting the new item %p at level %p", new_item, level);
				do {
					node_t* pred = preds[level];
					ASSERT(new_item->next[level] == (markable_t)nexts[level] || new_item->next[level] == MARK_NODE(nexts[level]));
					TRACE("s3", "sl_cas: attempting to to insert the new item between %p and %p", pred, nexts[level]);

					markable_t other = SYNC_CAS(&pred->next[level], (markable_t)nexts[level], (markable_t)new_item);
					if (other == (markable_t)nexts[level])
						break; // successfully linked <new_item> into the skiplist at the current <level>
					TRACE("s3", "sl_cas: lost a race. failed to change pred's link. expected %p found %p", nexts[level], other);

					// Find <new_item>'s new preds and nexts.
					find_preds(preds, nexts, new_item->num_levels, sl, key, ASSIST_UNLINK);

					for (int i = level; i < new_item->num_levels; ++i) {
						markable_t old_next = new_item->next[i];
						if ((markable_t)nexts[i] == old_next)
							continue;

						// Update <new_item>'s inconsistent next pointer before trying again. Use a CAS so if another thread
						// is trying to remove the new item concurrently we do not stomp on the mark it places on the item.
						TRACE("s3", "sl_cas: attempting to update the new item's link from %p to %p", old_next, nexts[i]);
						other = SYNC_CAS(&new_item->next[i], old_next, (markable_t)nexts[i]);
						ASSERT(other == old_next || other == MARK_NODE(old_next));

						// If another thread is removing this item we can stop linking it into to skiplist
						if (HAS_MARK(other)) {
							find_preds(NULL, NULL, 0, sl, key, FORCE_UNLINK); // see comment below
							return DOES_NOT_EXIST;
						}
					}
				} while (1);
			}

			// In case another thread was in the process of removing the <new_item> while we were added it, we have to
			// make sure it is completely unlinked before we return. We might have lost a race and inserted the new item
			// at some level after the other thread thought it was fully removed. That is a problem because once a thread
			// thinks it completely unlinks a node it queues it to be freed
			if (HAS_MARK(new_item->next[new_item->num_levels - 1])) {
				find_preds(NULL, NULL, 0, sl, key, FORCE_UNLINK);
			}

			return DOES_NOT_EXIST; // success, inserted a new item
		}

		map_val_t sl_remove(skiplist_t* sl, map_key_t key) {
			TRACE("s1", "sl_remove: removing item with key %p from skiplist %p", key, sl);
			node_t* preds[MAX_LEVELS];
			node_t* item = find_preds(preds, NULL, sl->high_water, sl, key, ASSIST_UNLINK);
			if (item == NULL) {
				TRACE("s3", "sl_remove: remove failed, an item with a matching key does not exist in the skiplist", 0, 0);
				return DOES_NOT_EXIST;
			}

			// Mark <item> at each level of <sl> from the top down. If multiple threads try to concurrently remove
			// the same item only one of them should succeed. Marking the bottom level establishes which of them succeeds.
			markable_t old_next = 0;
			for (int level = item->num_levels - 1; level >= 0; --level) {
				markable_t next;
				old_next = item->next[level];
				do {
					TRACE("s3", "sl_remove: marking item at level %p (next %p)", level, old_next);
					next = old_next;
					old_next = SYNC_CAS(&item->next[level], next, MARK_NODE((node_t*)next));
					if (HAS_MARK(old_next)) {
						TRACE("s2", "sl_remove: %p is already marked for removal by another thread (next %p)", item, old_next);
						if (level == 0)
							return DOES_NOT_EXIST;
						break;
					}
				} while (next != old_next);
			}

			// Atomically swap out the item's value in case another thread is updating the item while we are
			// removing it. This establishes which operation occurs first logically, the update or the remove.
			map_val_t val = SYNC_SWAP(&item->val, DOES_NOT_EXIST);
			TRACE("s2", "sl_remove: replaced item %p's value with DOES_NOT_EXIT", item, 0);

			// unlink the item
			find_preds(NULL, NULL, 0, sl, key, FORCE_UNLINK);

			// free the node
			if (sl->key_type != NULL) {
				rcu_defer_free((void*)item->key);
			}
			rcu_defer_free(item);

			return val;
		}

		void sl_print(skiplist_t* sl, int verbose) {

			if (verbose) {
				for (int level = MAX_LEVELS - 1; level >= 0; --level) {
					node_t* item = sl->head;
					if (item->next[level] == DOES_NOT_EXIST)
						continue;
					printf("(%d) ", level);
					int i = 0;
					while (item) {
						markable_t next = item->next[level];
						printf("%s%p ", HAS_MARK(next) ? "*" : "", item);
						item = STRIP_MARK(next);
						if (i++ > 30) {
							printf("...");
							break;
						}
					}
					printf("\n");
					fflush(stdout);
				}
				node_t* item = sl->head;
				int i = 0;
				while (item) {
					int is_marked = HAS_MARK(item->next[0]);
					printf("%s%p:0x%llx ", is_marked ? "*" : "", item, (uint64_t)item->key);
					if (item != sl->head) {
						printf("[%d]", item->num_levels);
					}
					else {
						printf("[HEAD]");
					}
					for (int level = 1; level < item->num_levels; ++level) {
						node_t* next = STRIP_MARK(item->next[level]);
						is_marked = HAS_MARK(item->next[0]);
						printf(" %p%s", next, is_marked ? "*" : "");
						if (item == sl->head && item->next[level] == DOES_NOT_EXIST)
							break;
					}
					printf("\n");
					fflush(stdout);
					item = STRIP_MARK(item->next[0]);
					if (i++ > 30) {
						printf("...\n");
						break;
					}
				}
			}
			printf("levels:%-2d  count:%-6lld \n", sl->high_water, (uint64_t)sl_count(sl));
		}

		sl_iter_t* sl_iter_begin(skiplist_t* sl, map_key_t key) {
			sl_iter_t* iter = (sl_iter_t*)nbd_malloc(sizeof(sl_iter_t));
			if (key != DOES_NOT_EXIST) {
				find_preds(NULL, &iter->next, 1, sl, key, DONT_UNLINK);
			}
			else {
				iter->next = GET_NODE(sl->head->next[0]);
			}
			return iter;
		}

		map_val_t sl_iter_next(sl_iter_t* iter, map_key_t* key_ptr) {
			assert(iter);
			node_t* item = iter->next;
			while (item != NULL && HAS_MARK(item->next[0])) {
				item = STRIP_MARK(item->next[0]);
			}
			if (item == NULL) {
				iter->next = NULL;
				return DOES_NOT_EXIST;
			}
			iter->next = STRIP_MARK(item->next[0]);
			if (key_ptr != NULL) {
				*key_ptr = item->key;
			}
			return item->val;
		}

		void sl_iter_free(sl_iter_t* iter) {
			nbd_free(iter);
		}







	};
#endif

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
		void Dispatch(context& ctx, uint32_t jobCount, const std::function<void(JobArgs)>& task, size_t sharedmemory_size = 0) noexcept;

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
			auto Dispatch(uint32_t jobCount, const std::function<void(JobArgs)>& task) { return impl::Dispatch(ctx, jobCount, task); };
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