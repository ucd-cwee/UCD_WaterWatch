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
					*ptr = static_cast<_type_>(srce);
				}
				else {
					new (&ptr[0]) _type_(std::forward<_type2_>(srce));
				}
			};
			template<typename _type_, typename _type2_, typename = std::enable_if_t<!std::is_same_v<_type2_, _type_>>>
			static void InstantiateData(_type_* ptr, _type2_ const& srce) {
				if constexpr (isPod<_type_>()) {
					*ptr = static_cast<_type_>(srce);
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

#define MWCAS_CAPACITY 14
#define MWCAS_RETRY_THRESHOLD 10
#define MWCAS_SLEEP_TIME 10
#define BZTREE_PAGE_SIZE 1024
#define BZTREE_MAX_DELTA_RECORD_NUM 64
#define BZTREE_MAX_DELETED_SPACE_SIZE (BZTREE_PAGE_SIZE / 8)
#define BZTREE_MIN_FREE_SPACE_SIZE (BZTREE_PAGE_SIZE / 8)
#define BZTREE_MIN_NODE_SIZE (BZTREE_PAGE_SIZE / 16)
#define BZTREE_MAX_MERGED_SIZE (BZTREE_PAGE_SIZE / 2)
#define BZTREE_MAX_VARIABLE_DATA_SIZE 128
#define DBGROUP_MAX_THREAD_NUM 128
#define CPP_UTILITY_SPINLOCK_RETRY_NUM 10
#define CPP_UTILITY_BACKOFF_TIME 10
#define BW_TREE_PAGE_SIZE 1024
static __forceinline constexpr size_t LOG2(size_t n) { return ((n < 2) ? 1 : 1 + LOG2(n / 2)); };
#define BW_TREE_DELTA_RECORD_NUM_THRESHOLD (2 * LOG2(BW_TREE_PAGE_SIZE / 256))
#define BW_TREE_MAX_DELTA_RECORD_NUM 64
#define BW_TREE_MIN_NODE_SIZE (BW_TREE_PAGE_SIZE / 16)
#define BW_TREE_MAX_VARIABLE_DATA_SIZE 128
#define BW_TREE_RETRY_THRESHOLD 10
#define BW_TREE_SLEEP_TIME 10

		// ATOMIC
		// utility
		namespace dbgroup::atomic::mwcas {
			/*######################################################################################
			 * Global enum and constants
			 *####################################################################################*/

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
			constexpr size_t kCacheLineSize = 64; // e.g. maximum of 8 words simultaneously? 

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
			template <int kMwCASCapacity>
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
				  * \e NOTE: if a memory address is included in MwCAS target fields, it must be read via this function.
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
					size_t i;
					MwCASField target_word{};
					while (true) {
						for (i = 0; i < kRetryNum; ++i) {
							target_word = target_addr->load(fence);
							if (!target_word.IsMwCASDescriptor()) return target_word.GetTargetData<T>();
						}
						std::this_thread::sleep_for(kShortSleep); // wait to prevent busy loop
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

		// THREAD
		// common
		namespace dbgroup::thread
		{
			/*##############################################################################
			 * Global constants
			 *############################################################################*/

			 /// @brief An alias of the acquire memory order.
			constexpr std::memory_order kAcquire = std::memory_order_acquire;

			/// @brief An alias of the release memory order.
			constexpr std::memory_order kRelease = std::memory_order_release;

			/// @brief An alias of the relaxed memory order.
			constexpr std::memory_order kRelaxed = std::memory_order_relaxed;

			/// @brief The maximum number of threads used in a process.
			constexpr size_t kMaxThreadNum = DBGROUP_MAX_THREAD_NUM;

			/// @brief The expected cache-line size.
			constexpr size_t kCashLineSize = 64;

		}  // namespace dbgroup::thread
		// id_manager
		namespace dbgroup::thread {
			extern std::shared_ptr<std::atomic_bool[]> id_vec;

			/**
			 * @brief A singleton class for managing IDs for each thread.
			 */
			class IDManager {
			public:
				// static std::atomic_bool id_vec[kMaxThreadNum];

				/*############################################################################
				 * No constructors and destructor
				 *##########################################################################*/

				IDManager() = delete;
				~IDManager() = delete;

				IDManager(const IDManager&) = delete;
				IDManager(IDManager&&) noexcept = delete;

				auto operator=(const IDManager&)->IDManager & = delete;
				auto operator=(IDManager&&) noexcept -> IDManager & = delete;

				/*############################################################################
				 * Public static utilities
				 *##########################################################################*/

				 /**
				  * @return The unique thread ID in [0, DBGROUP_MAX_THREAD_NUM).
				  */
				[[nodiscard]] static auto GetThreadID()  //
					-> size_t {
					return GetHeartBeater().GetID();
				};

				/**
				 * @return A weak pointer object to check heart beats of the current thread.
				 */
				[[nodiscard]] static auto GetHeartBeat()  //
					-> std::weak_ptr<size_t> {
					return GetHeartBeater().GetHeartBeat();
				};

			private:
				/*############################################################################
				 * Internal classes
				 *##########################################################################*/

				 /**
				  * @brief A class for managing heart beats of threads.
				  *
				  * This class is used with thread local storages and stops heart beats of the
				  * corresponding thread when it has exited.
				  */
				class HeartBeater {
				public:
					/*##########################################################################
					 * Public constructors and assignment operators
					 *########################################################################*/

					constexpr HeartBeater() = default;
					HeartBeater(const HeartBeater&) = delete;
					HeartBeater(HeartBeater&&) noexcept = delete;
					auto operator=(const HeartBeater& obj)->HeartBeater & = delete;
					auto operator=(HeartBeater&&) noexcept -> HeartBeater & = delete;

					/*##########################################################################
					 * Public destructor
					 *########################################################################*/

					 /**
					  * @brief Destroy the object.
					  *
					  * This destructor removes the heart beat and thread reservation flags.
					  */
					~HeartBeater() {
						id_vec[*id_].store(false, kRelaxed);
					};

					/*##########################################################################
					 * Public getters
					 *########################################################################*/

					 /**
					  * @retval true if this object has a unique thread ID.
					  * @retval false otherwise.
					  */
					[[nodiscard]] auto HasID() const  //
						-> bool {
						return id_.use_count() > 0;
					};

					/**
					 * @return The assigned ID for this object.
					 */
					[[nodiscard]] auto GetID() const  //
						-> size_t {
						return *id_;
					};

					/**
					 * @return A weak pointer object to check heart beats of this object.
					 */
					[[nodiscard]] auto GetHeartBeat() const  //
						-> std::weak_ptr<size_t> {
						return std::weak_ptr<size_t>{id_};
					};

					/*##########################################################################
					 * Public setters
					 *########################################################################*/

					 /**
					  * @brief Set a unique thread ID to this object.
					  *
					  * @param id The thread ID to be assigned.
					  */
					void SetID(  //
						const size_t id) {
						id_ = std::make_shared<size_t>(id);
					};

				private:
					/*##########################################################################
					 * Internal member variables
					 *########################################################################*/

					 /// @brief The assigned ID for this object.
					std::shared_ptr<size_t> id_{};
				};

				/*############################################################################
				 * Internal static utilities
				 *##########################################################################*/

				 /**
				  * @brief Get the reference to a heart beater.
				  *
				  * When a thread calls this function for the first time, it prepares its
				  * unique ID and heart beat manager.
				  *
				  * @return The reference to a heart beater.
				  */
				[[nodiscard]] static auto GetHeartBeater()  //
					-> const HeartBeater& {
					thread_local HeartBeater hb{};
					if (!hb.HasID()) {
						auto id = std::hash<std::thread::id>{}(std::this_thread::get_id()) % kMaxThreadNum;
						while (true) {
							auto& dst = id_vec[id];
							auto reserved = dst.load(kRelaxed);
							if (!reserved && dst.compare_exchange_strong(reserved, true, kRelaxed)) {
								hb.SetID(id);
								break;
							}
							if (++id >= kMaxThreadNum) {
								id = 0;
							}
						}
					}
					return hb;
				};
			};

		}  // namespace dbgroup::thread

		// MEMORY
		// utility
		namespace dbgroup::memory
		{
			/*##############################################################################
			 * Global constants
			 *############################################################################*/

			 /// @brief The default time interval for garbage collection (10 ms).
			constexpr size_t kDefaultGCTime = 10000;

			/// @brief The default number of worker threads for garbage collection.
			constexpr size_t kDefaultGCThreadNum = 1;

			/// @brief The default alignment size for dynamically allocated instances.
			constexpr size_t kDefaultAlignment = __STDCPP_DEFAULT_NEW_ALIGNMENT__;

			/// @brief An alias of the acquire memory order.
			constexpr std::memory_order kAcquire = std::memory_order_acquire;

			/// @brief An alias of the release memory order.
			constexpr std::memory_order kRelease = std::memory_order_release;

			/// @brief An alias of the relaxed memory order.
			constexpr std::memory_order kRelaxed = std::memory_order_relaxed;

			/*##############################################################################
			 * Turning parameters
			 *############################################################################*/

			 /// @brief The page size of virtual memory addresses.
			constexpr size_t kVMPageSize = 4096;

			/// @brief The size of words.
			constexpr size_t kWordSize = 8;

			/// @brief The expected cache line size.
			constexpr size_t kCashLineSize = 64;

			/// @brief The size of buffers for retaining garbages.
			constexpr size_t kGarbageBufSize = (kVMPageSize - 4 * kWordSize) / (2 * kWordSize);

			/*##############################################################################
			 * Utility classes
			 *############################################################################*/

			 /**
			  * @brief A default GC information.
			  *
			  */
			struct DefaultTarget {
				/// @brief Use the void type to avoid calling a destructor.
				using T = void;

				/// @brief Do not reuse pages after GC (release immediately).
				static constexpr bool kReusePages = false;
			};

			template <typename type>
			struct Target {
				/// @brief Use the void type to avoid calling a destructor.
				using T = type;

				/// @brief Do not reuse pages after GC (release immediately).
				static constexpr bool kReusePages = std::is_pod<T>::value;
			};

			/*##############################################################################
			 * Utility functions
			 *############################################################################*/

			 /**
			  * @brief Allocate a memory region with alignments.
			  *
			  * @tparam T A target class.
			  * @param size The size of target class.
			  * @return The address of an allocated one.
			  */
			template <class T>
			inline auto
				Allocate(                     //
					size_t size = sizeof(T))  //
				-> T*
			{
				if constexpr (alignof(T) <= kDefaultAlignment) {
					return reinterpret_cast<T*>(::operator new(size));
				}
				else {
					return reinterpret_cast<T*>(::operator new(size, static_cast<std::align_val_t>(alignof(T))));
				}
			}

			/**
			 * @brief A deleter function to release aligned pages.
			 *
			 * @tparam T A target class.
			 * @param ptr The address of allocations to be released.
			 */
			template <class T>
			inline void
				Release(  //
					void* ptr)
			{
				if constexpr (alignof(T) <= kDefaultAlignment) {
					::operator delete(ptr);
				}
				else {
					::operator delete(ptr, static_cast<std::align_val_t>(alignof(T)));
				}
			}

		}  // namespace dbgroup::memory
		// common
		namespace dbgroup::memory
		{
			/// We do not use type checks in PMDK.
			constexpr uint64_t kDefaultPMDKType = 0;

		}  // namespace dbgroup::memory
		// epoch
		namespace dbgroup::memory::component
		{
			/**
			 * @brief A class to represent epochs for epoch-based garbage collection.
			 *
			 */
			class Epoch
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct a new instance.
				  *
				  */
				constexpr Epoch() = default;

				Epoch(const Epoch&) = delete;
				auto operator=(const Epoch&)->Epoch & = delete;
				Epoch(Epoch&& orig) = delete;
				auto operator=(Epoch&& orig)->Epoch & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the Epoch object.
				  *
				  */
				~Epoch() = default;

				/*####################################################################################
				 * Public getters/setters
				 *##################################################################################*/

				 /**
				  * @return size_t a current epoch value.
				  */
				[[nodiscard]] auto
					GetCurrentEpoch() const  //
					-> size_t
				{
					return current_->load(std::memory_order_acquire);
				}

				/**
				 * @return size_t a protected epoch value.
				 */
				[[nodiscard]] auto
					GetProtectedEpoch() const  //
					-> size_t
				{
					return entered_.load(std::memory_order_relaxed);
				}

				/**
				 * @param global_epoch a pointer to the global epoch.
				 */
				void
					SetGlobalEpoch(std::atomic_size_t* global_epoch)
				{
					current_ = global_epoch;
				}

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Keep a current epoch value to protect new garbages.
				  *
				  */
				void
					EnterEpoch()
				{
					entered_.store(GetCurrentEpoch(), std::memory_order_relaxed);
				}

				/**
				 * @brief Release a protected epoch value to allow GC to delete old garbages.
				 *
				 */
				void
					LeaveEpoch()
				{
					entered_.store(std::numeric_limits<size_t>::max(), std::memory_order_relaxed);
				}

			private:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// A current epoch.
				std::atomic_size_t* current_{ nullptr };

				/// A snapshot to denote a protected epoch.
				std::atomic_size_t entered_{ std::numeric_limits<size_t>::max() };
			};

		}  // namespace dbgroup::memory::component
		// epoch_guard
		namespace dbgroup::memory::component
		{
			/**
			 * @brief A class to protect epochs based on the scoped locking pattern.
			 *
			 */
			class EpochGuard
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				constexpr EpochGuard() = default;

				/**
				 * @brief Construct a new instance and protect a current epoch.
				 *
				 * @param epoch a reference to a target epoch.
				 */
				explicit EpochGuard(Epoch* epoch) : epoch_{ epoch } { epoch_->EnterEpoch(); }

				constexpr EpochGuard(EpochGuard&& obj) noexcept
				{
					epoch_ = obj.epoch_;
					obj.epoch_ = nullptr;
				}

				constexpr auto
					operator=(EpochGuard&& obj) noexcept  //
					-> EpochGuard&
				{
					epoch_ = obj.epoch_;
					obj.epoch_ = nullptr;
					return *this;
				}

				// delete the copy constructor/assignment
				EpochGuard(const EpochGuard&) = delete;
				auto operator=(const EpochGuard&)->EpochGuard & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the instace and release a protected epoch.
				  *
				  */
				~EpochGuard()
				{
					if (epoch_ != nullptr) {
						epoch_->LeaveEpoch();
					}
				}

				/*####################################################################################
				 * Public getters
				 *##################################################################################*/

				 /**
				  * @return the epoch value protected by this object.
				  */
				[[nodiscard]] auto
					GetProtectedEpoch() const  //
					-> size_t
				{
					return epoch_->GetProtectedEpoch();
				}

			private:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// A reference to a target epoch.
				Epoch* epoch_{ nullptr };
			};

			class EpochWrapper
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				constexpr EpochWrapper() = default;

				/**
				 * @brief Construct a new instance and protect a current epoch.
				 *
				 * @param epoch a reference to a target epoch.
				 */
				explicit EpochWrapper(Epoch* epoch) : epoch_{ epoch } { epoch_->EnterEpoch(); }

				constexpr EpochWrapper(EpochWrapper&& obj) noexcept
				{
					epoch_ = obj.epoch_;
					obj.epoch_ = nullptr;
				}

				constexpr auto
					operator=(EpochWrapper&& obj) noexcept  //
					-> EpochWrapper&
				{
					epoch_ = obj.epoch_;
					obj.epoch_ = nullptr;
					return *this;
				}

				// delete the copy constructor/assignment
				EpochWrapper(const EpochWrapper&) = delete;
				auto operator=(const EpochWrapper&)->EpochWrapper & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the instace and release a protected epoch.
				  *
				  */
				~EpochWrapper()
				{
					if (epoch_ != nullptr) {
						epoch_->LeaveEpoch();
					}
				}

				/*####################################################################################
				 * Public getters
				 *##################################################################################*/

				 /**
				  * @return the epoch value protected by this object.
				  */
				[[nodiscard]] auto
					GetProtectedEpoch() const  //
					-> size_t
				{
					return epoch_->GetProtectedEpoch();
				}

			private:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// A reference to a target epoch.
				Epoch* epoch_{ nullptr };
			};


		}  // namespace dbgroup::memory::component
		// garbage_list
		namespace dbgroup::memory::component
		{
			/**
			 * @brief A class for representing garbage lists.
			 *
			 * @tparam Target a target class of garbage collection.
			 */
			template <class Target>
			class alignas(kCashLineSize) GarbageList {
			public:
				/*####################################################################################
				 * Public global constants
				 *##################################################################################*/

				 /// The size of buffers for retaining garbages.
				static constexpr size_t kBufferSize = (kVMPageSize - 4 * kWordSize) / (2 * kWordSize);

				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using T = typename Target::T;
				using IDManager = dbgroup::thread::IDManager;

				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct a new GarbageList object.
				  *
				  */
				GarbageList() = default;

				GarbageList(const GarbageList&) = delete;
				GarbageList(GarbageList&&) = delete;

				auto operator=(const GarbageList&)->GarbageList & = delete;
				auto operator=(GarbageList&&)->GarbageList & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the GarbageList object.
				  *
				  * If the list contains unreleased garbage, the destructor will forcibly release it.
				  */
				~GarbageList()
				{
					auto* head = Target::kReusePages ? head_.load(std::memory_order_relaxed) : mid_;
					if (head != nullptr) {
						GarbageBuffer::Clear(&head, std::numeric_limits<size_t>::max());
						delete head;
					}
				}

				/*####################################################################################
				 * Public utility functions for worker threads
				 *##################################################################################*/

				 /**
				  * @brief Add a new garbage instance.
				  *
				  * @param epoch an epoch value when a garbage is added.
				  * @param garbage_ptr a pointer to a target garbage.
				  */
				void
					AddGarbage(  //
						const size_t epoch,
						T* garbage_ptr)
				{
					AssignCurrentThreadIfNeeded();
					GarbageBuffer::AddGarbage(&tail_, epoch, garbage_ptr);
				}

				/**
				 * @brief Reuse a released memory page if it exists in the list.
				 *
				 * @retval nullptr if the list does not have reusable pages.
				 * @retval a memory page otherwise.
				 */
				auto
					GetPageIfPossible()  //
					-> void*
				{
					AssignCurrentThreadIfNeeded();
					return GarbageBuffer::ReusePage(&head_);
				}

				/*####################################################################################
				 * Public utility functions for GC threads
				 *##################################################################################*/

				 /**
				  * @brief Release registered garbage if possible.
				  *
				  * @param protected_epoch an epoch value to check whether garbage can be freed.
				  */
				void
					ClearGarbage(const size_t protected_epoch)
				{
					std::unique_lock guard{ mtx_, std::defer_lock };
					if (!guard.try_lock() || mid_ == nullptr) return;

					// destruct or release garbages
					if constexpr (!Target::kReusePages) {
						GarbageBuffer::Clear(&mid_, protected_epoch);
					}
					else {
						if (!heartbeat_.expired()) {
							GarbageBuffer::Destruct(&mid_, protected_epoch);
							return;
						}

						auto* head = head_.load(std::memory_order_relaxed);
						if (head != nullptr) {
							mid_ = head;
						}
						GarbageBuffer::Clear(&mid_, protected_epoch);
						head_.store(mid_, std::memory_order_relaxed);
					}

					// check this list is alive
					if (!heartbeat_.expired() || !mid_->Empty()) return;

					// release buffers if the thread has exitted
					delete mid_;
					head_.store(nullptr, std::memory_order_relaxed);
					mid_ = nullptr;
					tail_ = nullptr;
				}

			private:
				/*####################################################################################
				 * Internal classes
				 *##################################################################################*/

				 /**
				  * @brief A class to represent a buffer of garbage instances.
				  *
				  */
				class alignas(kVMPageSize) GarbageBuffer
				{
				public:
					/*##################################################################################
					 * Public constructors and assignment operators
					 *################################################################################*/

					 /**
					  * @brief Construct a new instance.
					  *
					  */
					constexpr GarbageBuffer() = default;

					GarbageBuffer(const GarbageBuffer&) = delete;
					auto operator=(const GarbageBuffer&)->GarbageBuffer & = delete;
					GarbageBuffer(GarbageBuffer&&) = delete;
					auto operator=(GarbageBuffer&&)->GarbageBuffer & = delete;

					/*##################################################################################
					 * Public destructors
					 *################################################################################*/

					 /**
					  * @brief Destroy the instance.
					  *
					  */
					~GarbageBuffer() = default;

					/*##################################################################################
					 * Public getters/setters
					 *################################################################################*/

					 /**
					  * @retval true if this list is empty.
					  * @retval false otherwise
					  */
					[[nodiscard]] auto
						Empty() const  //
						-> bool
					{
						const auto end_pos = end_pos_.load(std::memory_order_acquire);
						const auto size = end_pos - begin_pos_.load(std::memory_order_relaxed);

						return (size == 0) && (end_pos < kBufferSize);
					}

					/*##################################################################################
					 * Public utility functions
					 *################################################################################*/

					 /**
					  * @brief Add a new garbage instance to a specified buffer.
					  *
					  * If the buffer becomes full, create a new garbage buffer and link them.
					  *
					  * @param buf_addr the address of the pointer of a target buffer.
					  * @param epoch an epoch value when a garbage is added.
					  * @param garbage a new garbage instance.
					  */
					static void
						AddGarbage(  //
							GarbageBuffer** buf_addr,
							const size_t epoch,
							T* garbage)
					{
						auto* buf = *buf_addr;
						const auto pos = buf->end_pos_.load(std::memory_order_relaxed);

						// insert a new garbage
						buf->garbage_.at(pos).epoch = epoch;
						buf->garbage_.at(pos).ptr = garbage;

						// check whether the list is full
						if (pos >= kBufferSize - 1) {
							auto* new_tail = new GarbageBuffer{};
							buf->next_ = new_tail;
							*buf_addr = new_tail;
						}

						// increment the end position
						buf->end_pos_.fetch_add(1, std::memory_order_release);
					}

					/**
					 * @brief Reuse a released memory page.
					 *
					 * @param buf_addr the address of the pointer of a target buffer.
					 * @retval a memory page if exist.
					 * @retval nullptr otherwise.
					 */
					static auto
						ReusePage(std::atomic<GarbageBuffer*>* buf_addr)  //
						-> void*
					{
						auto* buf = buf_addr->load(std::memory_order_relaxed);
						const auto pos = buf->begin_pos_.load(std::memory_order_relaxed);
						const auto mid_pos = buf->mid_pos_.load(std::memory_order_acquire);

						// check whether there are released garbage
						if (pos >= mid_pos) return nullptr;

						// get a released page
						buf->begin_pos_.fetch_add(1, std::memory_order_relaxed);
						auto* page = buf->garbage_.at(pos).ptr;

						// check whether all the pages in the list are reused
						if (pos >= kBufferSize - 1) {
							buf_addr->store(buf->next_, std::memory_order_relaxed);
							delete buf;
						}

						return page;
					}

					/**
					 * @brief Destruct garbage where their epoch is less than a protected one.
					 *
					 * @param buf_addr the address of the pointer of a target buffer.
					 * @param protected_epoch a protected epoch.
					 */
					static void
						Destruct(  //
							GarbageBuffer** buf_addr,
							const size_t protected_epoch)
					{
						while (true) {
							// release unprotected garbage
							auto* buf = *buf_addr;
							const auto end_pos = buf->end_pos_.load(std::memory_order_acquire);
							auto pos = buf->mid_pos_.load(std::memory_order_relaxed);
							for (; pos < end_pos; ++pos) {
								if (buf->garbage_.at(pos).epoch >= protected_epoch) break;

								// only call destructor to reuse pages
								if constexpr (!std::is_same_v<T, void>) {
									buf->garbage_.at(pos).ptr->~T();
								}
							}

							// update the position to make visible destructed garbage
							buf->mid_pos_.store(pos, std::memory_order_release);
							if (pos < kBufferSize) return;

							// release the next buffer recursively
							*buf_addr = buf->next_;
						}
					}

					/**
					 * @brief Release garbage where their epoch is less than a protected one.
					 *
					 * @param buf_addr the address of the pointer of a target buffer.
					 * @param protected_epoch a protected epoch.
					 */
					static void
						Clear(  //
							GarbageBuffer** buf_addr,
							const size_t protected_epoch)
					{
						while (true) {
							auto* buf = *buf_addr;
							const auto mid_pos = buf->mid_pos_.load(std::memory_order_relaxed);
							const auto end_pos = buf->end_pos_.load(std::memory_order_acquire);

							// release unprotected garbage
							auto pos = buf->begin_pos_.load(std::memory_order_relaxed);
							for (; pos < mid_pos; ++pos) {
								// the garbage has been already destructed
								Release<Target>(buf->garbage_.at(pos).ptr);
							}
							for (; pos < end_pos; ++pos) {
								if (buf->garbage_.at(pos).epoch >= protected_epoch) break;

								auto* ptr = buf->garbage_.at(pos).ptr;
								if constexpr (!std::is_same_v<T, void>) {
									ptr->~T();
								}
								Release<Target>(ptr);
							}
							buf->begin_pos_.store(pos, std::memory_order_relaxed);
							buf->mid_pos_.store(pos, std::memory_order_relaxed);

							if (pos < kBufferSize) return;

							// release the next buffer recursively
							*buf_addr = buf->next_;
							delete buf;
						}
					}

				private:
					/*##################################################################################
					 * Internal classes
					 *################################################################################*/

					 /**
					  * @brief A class to represent the pair of an epoch value and a registered garbage.
					  *
					  */
					struct Garbage {
						/// An epoch value when the garbage is registered.
						size_t epoch{};

						/// A pointer to the registered garbage.
						T* ptr{};
					};

					/*##################################################################################
					 * Internal member variables
					 *################################################################################*/

					 /// The index to represent a head position.
					std::atomic_size_t begin_pos_{ 0 };

					/// The end of released indexes
					std::atomic_size_t mid_pos_{ 0 };

					/// A buffer of garbage instances with added epochs.
					std::array<Garbage, kBufferSize> garbage_{};

					/// The index to represent a tail position.
					std::atomic_size_t end_pos_{ 0 };

					/// A pointer to a next garbage buffer.
					GarbageBuffer* next_{ nullptr };
				};

				/*####################################################################################
				 * Internal utilities
				 *##################################################################################*/

				 /**
				  * @brief Assign this list to the current thread.
				  *
				  */
				void
					AssignCurrentThreadIfNeeded()
				{
					if (!heartbeat_.expired()) return;

					std::lock_guard guard{ mtx_ };
					if (tail_ == nullptr) {
						tail_ = new GarbageBuffer{};
						mid_ = tail_;
						if constexpr (Target::kReusePages) {
							head_.store(tail_, std::memory_order_relaxed);
						}
					}
					heartbeat_ = IDManager::GetHeartBeat();
				}

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// A flag for indicating the corresponding thread has exited.
				std::weak_ptr<size_t> heartbeat_{};

				/// A garbage list that has destructed pages.
				std::atomic<GarbageBuffer*> head_{ nullptr };

				/// A garbage list that has free space for new garbages.
				GarbageBuffer* tail_{ nullptr };

				/// A dummy array for cache line alignments
				uint64_t padding_[4]{};

				/// A mutex instance for modifying buffer pointers.
				std::mutex mtx_{};

				/// A garbage list that has not destructed pages.
				GarbageBuffer* mid_{ nullptr };
			};

		}  // namespace dbgroup::memory::component
		// epoch_manager
		namespace dbgroup::memory
		{
			/**
			 * @brief A class to manage epochs for epoch-based garbage collection.
			 *
			 */
			class EpochManager
			{
			public:
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using IDManager = dbgroup::thread::IDManager;
				using Epoch = component::Epoch;
				using EpochGuard = component::EpochGuard;

				/*####################################################################################
				 * Public constants
				 *##################################################################################*/

				 /// The capacity of nodes for protected epochs.
				static constexpr size_t kCapacity = 256;

				/// The initial value of epochs.
				static constexpr size_t kInitialEpoch = kCapacity;

				/// The minimum value of epochs.
				static constexpr size_t kMinEpoch = 0;

				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct a new instance.
				  *
				  */
				EpochManager()
				{
					auto& protected_epochs = ProtectedNode::GetProtectedEpochs(kInitialEpoch, protected_lists_);
					protected_epochs.emplace_back(kInitialEpoch);
				}

				EpochManager(const EpochManager&) = delete;
				auto operator=(const EpochManager&)->EpochManager & = delete;
				EpochManager(EpochManager&&) = delete;
				auto operator=(EpochManager&&)->EpochManager & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the instance.
				  *
				  */
				~EpochManager()
				{
					// remove the retained protected epochs
					[[maybe_unused]] const auto dummy = global_epoch_.load(std::memory_order_acquire);
					auto* pro_next = protected_lists_;
					while (pro_next != nullptr) {
						auto* current = pro_next;
						pro_next = current->next;
						delete current;
					}
				}

				/*####################################################################################
				 * Public getters
				 *##################################################################################*/

				 /**
				  * @return a current global epoch value.
				  */
				[[nodiscard]] auto
					GetCurrentEpoch() const  //
					-> size_t
				{
					return global_epoch_.load(std::memory_order_relaxed);
				}

				/**
				 * @return the minimum protected epoch value.
				 */
				[[nodiscard]] auto
					GetMinEpoch() const  //
					-> size_t
				{
					return min_epoch_.load(std::memory_order_relaxed);
				}

				/**
				 * @brief Get protected epoch values as shared_ptr.
				 *
				 * Protected epoch values are sorted by descending order and include the current epoch
				 * value. Note that the returned vector cannot be modified because it is referred from
				 * multiple threads concurrently.
				 *
				 * @return protected epoch values.
				 */
				[[nodiscard]] auto
					GetProtectedEpochs()  //
					-> std::pair<EpochGuard, const std::vector<size_t>&>
				{
					auto&& guard = CreateEpochGuard();
					const auto e = guard.GetProtectedEpoch();
					const auto& protected_epochs = ProtectedNode::GetProtectedEpochs(e, protected_lists_);

					return { std::move(guard), protected_epochs };
				}

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Create a guard instance based on the scoped locking pattern.
				  *
				  * @return EpochGuard a created epoch guard.
				  */
				[[nodiscard]] auto
					CreateEpochGuard()  //
					-> EpochGuard
				{
					auto& tls = tls_fields_[IDManager::GetThreadID()];
					if (tls.heartbeat.expired()) {
						tls.epoch.SetGlobalEpoch(&global_epoch_);
						tls.heartbeat = IDManager::GetHeartBeat();
					}

					return EpochGuard{ &(tls.epoch) };
				}

				/**
				 * @brief Increment a current epoch value.
				 *
				 * This function also updates protected epoch values.
				 */
				void
					ForwardGlobalEpoch()
				{
					const auto cur_epoch = global_epoch_.load(std::memory_order_relaxed);
					const auto next_epoch = cur_epoch + 1;

					// create a new node if needed
					if ((next_epoch & kLowerMask) == 0UL) {
						protected_lists_ = new ProtectedNode{ next_epoch, protected_lists_ };
					}

					// update protected epoch values
					auto& protected_epochs = ProtectedNode::GetProtectedEpochs(next_epoch, protected_lists_);
					CollectProtectedEpochs(cur_epoch, protected_epochs);
					RemoveOutDatedLists(protected_epochs);

					// store the max/min epoch values for efficiency
					global_epoch_.store(next_epoch, std::memory_order_release);
					min_epoch_.store(protected_epochs.back(), std::memory_order_relaxed);
				}

			private:
				/*####################################################################################
				 * Internal structs
				 *##################################################################################*/

				 /**
				  * @brief A class for representing thread local epoch storages.
				  *
				  */
				struct alignas(kCashLineSize) TLSEpoch {
					/// An epoch object for each thread.
					Epoch epoch{};

					/// A flag for indicating the corresponding thread has exited.
					std::weak_ptr<size_t> heartbeat{};
				};

				/**
				 * @brief A class of nodes for composing a linked list of epochs in each thread.
				 *
				 */
				class alignas(kCashLineSize) ProtectedNode
				{
				public:
					/*##################################################################################
					 * Public constructors and assignment operators
					 *################################################################################*/

					 /**
					  * @brief Construct a new instance.
					  *
					  * @param epoch upper bits of epoch values to be retained in this node.
					  * @param next a pointer to a next node.
					  */
					ProtectedNode(  //
						const size_t epoch,
						ProtectedNode* next)
						: next{ next }, upper_epoch_(epoch)
					{
					}

					ProtectedNode(const ProtectedNode&) = delete;
					auto operator=(const ProtectedNode&)->ProtectedNode & = delete;
					ProtectedNode(ProtectedNode&&) = delete;
					auto operator=(ProtectedNode&&)->ProtectedNode & = delete;

					/*##################################################################################
					 * Public destructors
					 *################################################################################*/

					 /**
					  * @brief Destroy the instance.
					  *
					  */
					~ProtectedNode() = default;

					/*##################################################################################
					 * Public utility functions
					 *################################################################################*/

					 /**
					  * @brief Get protected epoch values based on a given epoch.
					  *
					  * @param epoch a target epoch value.
					  * @param node the head pointer of a linked list.
					  * @return the protected epochs.
					  */
					[[nodiscard]] static auto
						GetProtectedEpochs(  //
							const size_t epoch,
							ProtectedNode* node)  //
						-> std::vector<size_t>&
					{
						// go to the target node
						const auto upper_epoch = epoch & kUpperMask;
						while (node->upper_epoch_ > upper_epoch) {
							node = node->next;
						}

						return node->epoch_lists_.at(epoch & kLowerMask);
					}

					/**
					 * @return the upper bits of the current epoch.
					 */
					[[nodiscard]] constexpr auto
						GetUpperBits() const  //
						-> size_t
					{
						return upper_epoch_;
					}

					/*##################################################################################
					 * Public member variables
					 *################################################################################*/

					 /// A pointer to the next node.
					ProtectedNode* next{ nullptr };  // NOLINT

				private:
					/*##################################################################################
					 * Internal member variables
					 *################################################################################*/

					 /// The upper bits of epoch values to be retained in this node.
					size_t upper_epoch_{};

					/// The list of protected epochs.
					std::array<std::vector<size_t>, kCapacity> epoch_lists_{};
				};

				/*####################################################################################
				 * Internal constants
				 *##################################################################################*/

				 /// The expected maximum number of threads.
				static constexpr size_t kMaxThreadNum = dbgroup::thread::kMaxThreadNum;

				/// A bitmask for extracting lower bits from epochs.
				static constexpr size_t kLowerMask = kCapacity - 1UL;

				/// A bitmask for extracting upper bits from epochs.
				static constexpr size_t kUpperMask = ~kLowerMask;

				/*####################################################################################
				 * Internal utility functions
				 *##################################################################################*/

				 /**
				  * @brief Collect epoch value for epoch-based protection.
				  *
				  * This function also removes dead epochs from the internal list while computing.
				  *
				  * @param cur_epoch the current global epoch value.
				  * @param protected_epochs protected epoch values.
				  */
				void
					CollectProtectedEpochs(  //
						const size_t cur_epoch,
						std::vector<size_t>& protected_epochs)
				{
					protected_epochs.reserve(kMaxThreadNum);
					protected_epochs.emplace_back(cur_epoch + 1);  // reserve the next epoch
					protected_epochs.emplace_back(cur_epoch);

					for (size_t i = 0; i < kMaxThreadNum; ++i) {
						auto& tls = tls_fields_[i];
						if (tls.heartbeat.expired()) continue;

						const auto protected_epoch = tls.epoch.GetProtectedEpoch();
						if (protected_epoch < std::numeric_limits<size_t>::max()) {
							protected_epochs.emplace_back(protected_epoch);
						}
					}

					// remove duplicate values
					std::sort(protected_epochs.begin(), protected_epochs.end(), std::greater<size_t>{});
					auto&& end_iter = std::unique(protected_epochs.begin(), protected_epochs.end());
					protected_epochs.erase(end_iter, protected_epochs.end());
				}

				/**
				 * @brief Remove unprotected epoch nodes from a linked-list.
				 *
				 * @param protected_epochs protected epoch values.
				 */
				void
					RemoveOutDatedLists(std::vector<size_t>& protected_epochs)
				{
					const auto& it_end = protected_epochs.cend();
					auto&& it = protected_epochs.cbegin();
					auto protected_epoch = *it & kUpperMask;

					// remove out-dated lists
					auto* prev = protected_lists_;
					auto* current = protected_lists_;
					while (current->next != nullptr) {
						const auto upper_bits = current->GetUpperBits();
						if (protected_epoch == upper_bits) {
							// this node is still referred, so skip
							prev = current;
							current = current->next;

							// search the next protected epoch
							do {
								if (++it == it_end) {
									protected_epoch = kMinEpoch;
									break;
								}
								protected_epoch = *it & kUpperMask;
							} while (protected_epoch == upper_bits);
							continue;
						}

						// remove the out-dated list
						prev->next = current->next;
						delete current;
						current = prev->next;
					}
				}

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// A global epoch counter.
				std::atomic_size_t global_epoch_{ kInitialEpoch };

				/// The minimum protected ecpoch value.
				std::atomic_size_t min_epoch_{ kInitialEpoch };

				/// The head pointer of a linked list of epochs.
				ProtectedNode* protected_lists_{ new ProtectedNode{kInitialEpoch, nullptr} };

				/// The array of epochs to use as thread local storages.
				TLSEpoch tls_fields_[kMaxThreadNum]{};
			};

		}  // namespace dbgroup::memory
		// epoch_based_gc 
		namespace dbgroup::memory
		{
			/**
			 * @brief A class to manage garbage collection.
			 *
			 * @tparam T a target class of garbage collection.
			 */
			template <class... GCTargets>
			class EpochBasedGC
			{
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using IDManager = dbgroup::thread::IDManager;
				using Epoch = component::Epoch;
				using EpochGuard = component::EpochGuard;
				using Clock_t = ::std::chrono::high_resolution_clock;

				template <class Target>
				using GarbageList = component::GarbageList<Target>;

			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct a new instance.
				  *
				  * @param gc_interval_micro_sec the duration of interval for GC.
				  * @param gc_thread_num the maximum number of threads to perform GC.
				  */
				constexpr explicit EpochBasedGC(  //
					const size_t gc_interval_micro_sec = kDefaultGCTime,
					const size_t gc_thread_num = kDefaultGCThreadNum)
					: gc_interval_{ gc_interval_micro_sec }, gc_thread_num_{ gc_thread_num }
				{
					InitializeGarbageLists<DefaultTarget, GCTargets...>();
					cleaner_threads_.reserve(gc_thread_num_);
				}

				EpochBasedGC(const EpochBasedGC&) = delete;
				EpochBasedGC(EpochBasedGC&&) = delete;

				auto operator=(const EpochBasedGC&)->EpochBasedGC & = delete;
				auto operator=(EpochBasedGC&&)->EpochBasedGC & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the instance.
				  *
				  * If protected garbage remains, this destructor waits for them to be free.
				  */
				~EpochBasedGC()
				{
					// stop garbage collection
					StopGC();
				}

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Create a guard instance to protect garbage based on the scoped locking
				  * pattern.
				  *
				  * @return EpochGuard a created epoch guard.
				  */
				auto
					CreateEpochGuard()  //
					-> EpochGuard
				{
					return epoch_manager_.CreateEpochGuard();
				}

				/**
				 * @brief Add a new garbage instance.
				 *
				 * @tparam Target a class for representing target garbage.
				 * @param garbage_ptr a pointer to a target garbage.
				 */
				template <class Target = DefaultTarget>
				void
					AddGarbage(const void* garbage_ptr)
				{
					auto* ptr = static_cast<typename Target::T*>(const_cast<void*>(garbage_ptr));
					GetGarbageList<Target>()->AddGarbage(epoch_manager_.GetCurrentEpoch(), ptr);
				}

				/**
				 * @brief Add a new garbage instance.
				 *
				 * @tparam Target a class for representing target garbage.
				 * @param garbage_ptr a pointer to a target garbage.
				 */
				template <typename T>
				void
					Push(T* garbage_ptr)
				{
					GetGarbageList<Target<T>>()->AddGarbage(epoch_manager_.GetCurrentEpoch(), garbage_ptr);
				}


				/**
				 * @brief Reuse a released memory page if it exists.
				 *
				 * @tparam Target a class for representing target garbage.
				 * @retval nullptr if there are no reusable pages.
				 * @retval a memory page.
				 */
				template <class Target = DefaultTarget>
				auto
					GetPageIfPossible()  //
					-> void*
				{
					static_assert(Target::kReusePages);
					return GetGarbageList<Target>()->GetPageIfPossible();
				}

				/*####################################################################################
				 * Public GC control functions
				 *##################################################################################*/

				 /**
				  * @brief Start garbage collection.
				  *
				  * @retval true if garbage collection has started.
				  * @retval false if garbage collection is already running.
				  */
				auto
					StartGC()  //
					-> bool
				{
					if (gc_is_running_.load(std::memory_order_relaxed)) return false;

					gc_is_running_.store(true, std::memory_order_relaxed);
					gc_thread_ = std::thread{ &EpochBasedGC::RunGC, this };
					return true;
				}

				/**
				 * @brief Stop garbage collection.
				 *
				 * @retval true if garbage collection has stopped.
				 * @retval false if garbage collection is not running.
				 */
				auto
					StopGC()  //
					-> bool
				{
					if (!gc_is_running_.load(std::memory_order_relaxed)) return false;

					gc_is_running_.store(false, std::memory_order_relaxed);
					gc_thread_.join();

					DestroyGarbageLists<DefaultTarget, GCTargets...>();
					return true;
				}

			private:
				/*####################################################################################
				 * Internal constants
				 *##################################################################################*/

				 /// The expected maximum number of threads.
				static constexpr size_t kMaxThreadNum = dbgroup::thread::kMaxThreadNum;

				/*####################################################################################
				 * Internal utilities for initialization and finalization
				 *##################################################################################*/

				 /**
				  * @brief A dummy function for creating type aliases.
				  *
				  */
				template <class Target, class... Tails>
				static auto
					ConvToTuple()
				{
					using ListsPtr = std::unique_ptr<GarbageList<Target>[]>;

					if constexpr (sizeof...(Tails) > 0) {
						return std::tuple_cat(std::tuple<ListsPtr>{}, ConvToTuple<Tails...>());
					}
					else {
						return std::tuple<ListsPtr>{};
					}
				}

				/**
				 * @brief Create the space for garbage lists for all the target garbage.
				 *
				 * @tparam Target the current class in garbage targets.
				 * @tparam Tails the remaining classes in garbage targets.
				 */
				template <class Target, class... Tails>
				void
					InitializeGarbageLists()
				{
					using ListsPtr = std::unique_ptr<GarbageList<Target>[]>;

					auto& lists = std::get<ListsPtr>(garbage_lists_);
					lists.reset(new GarbageList<Target>[kMaxThreadNum]);

					if constexpr (sizeof...(Tails) > 0) {
						InitializeGarbageLists<Tails...>();
					}
				}

				/**
				 * @brief Destroy all the garbage lists for destruction.
				 *
				 * @tparam Target the current class in garbage targets.
				 * @tparam Tails the remaining classes in garbage targets.
				 */
				template <class Target, class... Tails>
				void
					DestroyGarbageLists()
				{
					using ListsPtr = std::unique_ptr<GarbageList<Target>[]>;

					auto& lists = std::get<ListsPtr>(garbage_lists_);
					lists.reset(nullptr);

					if constexpr (sizeof...(Tails) > 0) {
						DestroyGarbageLists<Tails...>();
					}
				}

				/*####################################################################################
				 * Internal utility functions
				 *##################################################################################*/

				 /**
				  * @tparam Target a class for representing target garbage.
				  * @return the head of a linked list of garbage nodes and its mutex object.
				  */
				template <class Target>
				[[nodiscard]] auto
					GetGarbageList()
				{
					using ListsPtr = std::unique_ptr<GarbageList<Target>[]>;

					return &(std::get<ListsPtr>(garbage_lists_)[IDManager::GetThreadID()]);
				}

				/**
				 * @brief Clear registered garbage if possible.
				 *
				 * @tparam Target the current class in garbage targets.
				 * @tparam Tails the remaining classes in garbage targets.
				 * @param protected_epoch an epoch value to be protected.
				 */
				template <class Target, class... Tails>
				void
					ClearGarbage(const size_t protected_epoch)
				{
					using ListsPtr = std::unique_ptr<GarbageList<Target>[]>;

					auto& lists = std::get<ListsPtr>(garbage_lists_);
					for (size_t i = 0; i < kMaxThreadNum; ++i) {
						lists[i].ClearGarbage(protected_epoch);
					}

					if constexpr (sizeof...(Tails) > 0) {
						ClearGarbage<Tails...>(protected_epoch);
					}
				}

				/**
				 * @brief Run a procedure of garbage collection.
				 *
				 */
				void
					RunGC()
				{
					// create cleaner threads
					for (size_t i = 0; i < gc_thread_num_; ++i) {
						cleaner_threads_.emplace_back([&]() {
							for (auto wake_time = Clock_t::now() + gc_interval_;  //
								gc_is_running_.load(std::memory_order_relaxed);  //
								wake_time += gc_interval_)                       //
							{
								// release unprotected garbage
								ClearGarbage<DefaultTarget, GCTargets...>(epoch_manager_.GetMinEpoch());

								// wait until the next epoch
								std::this_thread::sleep_until(wake_time);
							}
							});
					}

					// manage the global epoch
					for (auto wake_time = Clock_t::now() + gc_interval_;  //
						gc_is_running_.load(std::memory_order_relaxed);  //
						wake_time += gc_interval_)                       //
					{
						// wait until the next epoch
						std::this_thread::sleep_until(wake_time);
						epoch_manager_.ForwardGlobalEpoch();
					}

					// wait all the cleaner threads return
					for (auto&& t : cleaner_threads_) {
						t.join();
					}
					cleaner_threads_.clear();
				}

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// The duration of garbage collection in micro seconds.
				const std::chrono::microseconds gc_interval_{};

				/// The maximum number of cleaner threads
				const size_t gc_thread_num_{ 1 };

				/// An epoch manager.
				EpochManager epoch_manager_{};

				/// A mutex to protect liked garbage lists
				std::shared_mutex garbage_lists_lock_{};

				/// A thread to run garbage collection.
				std::thread gc_thread_{};

				/// Worker threads to release garbage
				std::vector<std::thread> cleaner_threads_{};

				/// A flag to check whether garbage collection is running.
				std::atomic_bool gc_is_running_{ false };

				/// The heads of linked lists for each GC target.
				decltype(ConvToTuple<DefaultTarget, GCTargets...>()) garbage_lists_ =
					ConvToTuple<DefaultTarget, GCTargets...>();
			};

		}  // namespace dbgroup::memory

		// BW TREE
		// utility
		namespace dbgroup::index::bw_tree
		{
			/*######################################################################################
			 * Global constants
			 *####################################################################################*/

			 /// the default time interval for garbage collection [us].
			constexpr size_t kDefaultGCTime = 10000;

			/// the default number of worker threads for garbage collection.
			constexpr size_t kDefaultGCThreadNum = 1;

			/// a flag for indicating closed intervals
			constexpr bool kClosed = true;

			/// a flag for indicating closed intervals
			constexpr bool kOpen = false;

			/*######################################################################################
			 * Utility enum and classes
			 *####################################################################################*/

			 /**
			  * @brief Return codes for APIs of a Bw-tree.
			  *
			  */
			enum ReturnCode {
				kKeyNotExist = -2,
				kKeyExist,
				kSuccess = 0,
			};

			/**
			 * @brief Compare binary keys as CString.
			 *
			 * NOTE: the end of every key must be '\\0'.
			 *
			 */
			struct CompareAsCString {
				constexpr auto
					operator()(  //
						const void* a,
						const void* b) const noexcept  //
					-> bool
				{
					if (a == nullptr) return false;
					if (b == nullptr) return true;
					return strcmp(static_cast<const char*>(a), static_cast<const char*>(b)) < 0;
				}
			};

			/**
			 * @tparam T a target class.
			 * @retval true if a target class is variable-length data.
			 * @retval false otherwise.
			 */
			template <class T>
			constexpr auto
				IsVarLenData()  //
				-> bool
			{
				if constexpr (std::is_same_v<T, char*> || std::is_same_v<T, std::byte*>) {
					return true;
				}
				else {
					return false;
				}
			}

			/**
			 * @param val a target value.
			 * @return the binary logarithm of a given value.
			 */
			constexpr auto
				Log2(const size_t val)  //
				-> size_t
			{
				if (val == 0) return 0;
				return (val == 1) ? 0 : Log2(val >> 1UL) + 1;
			}

			/*######################################################################################
			 * Tuning parameters for Bw-tree
			 *####################################################################################*/

			 /// The default page size of each node.
			constexpr size_t kPageSize = BW_TREE_PAGE_SIZE;

			/// The page size of virtual memory addresses.
			constexpr size_t kVMPageSize = 4096;

			/// The number of delta records for invoking consolidation.
			constexpr size_t kDeltaRecordThreshold = BW_TREE_DELTA_RECORD_NUM_THRESHOLD;

			/// The number of delta records for invoking consolidation.
			constexpr size_t kMaxDeltaRecordNum = BW_TREE_MAX_DELTA_RECORD_NUM;

			/// Waiting for other threads if the number of delta records exceeds this threshold.
			constexpr size_t kMinNodeSize = BW_TREE_MIN_NODE_SIZE;

			/// The maximun size of variable-length data
			constexpr size_t kMaxVarDataSize = BW_TREE_MAX_VARIABLE_DATA_SIZE;

			/// The maximum number of retries for preventing busy loops.
			constexpr size_t kRetryNum = BW_TREE_RETRY_THRESHOLD;

			/// Assumes that one word is represented by 8 bytes.
			constexpr size_t kWordSize = 8;

			/// Assumes that one cache line is represented by 64 bytes.
			constexpr size_t kCacheLineSize = 64;

			/// A sleep time for preventing busy loops [us].
			constexpr auto kShortSleep = std::chrono::microseconds{ BW_TREE_SLEEP_TIME };

			/// a flag for indicating optimized page layouts for fixed-length data.
			constexpr bool kOptimizeForFixLenData = false;

		}  // namespace dbgroup::index::bw_tree
		// common
		namespace dbgroup::index::bw_tree::component
		{
			/*######################################################################################
			 * Internal enum and classes
			 *####################################################################################*/

			 /// Alias for representing logical page IDs.
			using PageID = uint64_t;

			/**
			 * @brief Internal return codes for representing results of delta-chain traversal.
			 *
			 */
			enum DeltaRC {
				kReachBaseNode = 0,
				kRecordFound,
				kRecordNotFound,
				kNodeRemoved,
				kKeyIsInSibling,
				kAbortMerge,
			};

			/**
			 * @brief A flag for distinguishing leaf/internal nodes.
			 *
			 */
			enum NodeType : uint16_t {
				kLeaf = 0,
				kInner,
			};

			/**
			 * @brief A flag for representing the types of delta records.
			 *
			 */
			enum DeltaType : uint16_t {
				kNotDelta = 0,
				kInsert,
				kModify,
				kDelete,
				kRemoveNode,
				kMerge,
			};

			/*######################################################################################
			 * Internal constants
			 *####################################################################################*/

			 /// bits for word alignments.
			constexpr size_t kWordAlign = kWordSize - 1;

			/// bits for cache line alignments.
			constexpr size_t kCacheAlign = kCacheLineSize - 1;

			/// the NULL value for uintptr_t
			constexpr uintptr_t kNullPtr = 0;

			/// leave free space for later modifications.
			constexpr size_t kNodeCapacityForBulkLoading = kPageSize * 0.9;

			/// The alignment size for internal pages.
			constexpr size_t kPageAlign = kPageSize < kVMPageSize ? kPageSize : kVMPageSize;

			/*######################################################################################
			 * Internal utility classes
			 *####################################################################################*/

			 /**
			  * @brief A dummy struct for representing internal pages.
			  *
			  */
			struct alignas(kPageAlign) NodePage : public dbgroup::memory::DefaultTarget {
				// reuse pages
				static constexpr bool kReusePages = true;

				/// @brief A dummy member variable to ensure the page size.
				uint8_t dummy[kPageSize];
			};

			/**
			 * @brief A struct for representing GC delta pages.
			 *
			 */
			struct alignas(kCacheLineSize) DeltaPage : public dbgroup::memory::DefaultTarget {
				// reuse pages
				static constexpr bool kReusePages = true;
			};

			/*######################################################################################
			 * Internal utility functions
			 *####################################################################################*/

			 /**
			  * @brief Shift a memory address by byte offsets.
			  *
			  * @param addr an original address.
			  * @param offset an offset to shift.
			  * @return void* a shifted address.
			  */
			constexpr auto
				ShiftAddr(  //
					const void* addr,
					const size_t offset)  //
				-> void*
			{
				return static_cast<std::byte*>(const_cast<void*>(addr)) + offset;
			}

			/**
			 * @brief Parse an entry of bulkload according to key's type.
			 *
			 * @tparam Entry std::pair or std::tuple for containing entries.
			 * @param entry a bulkload entry.
			 * @retval 1st: a target key.
			 * @retval 2nd: a target payload.
			 * @retval 3rd: the length of a target key.
			 * @retval 4th: the length of a target payload.
			 */
			template <class Entry>
			constexpr auto
				ParseEntry(const Entry& entry)  //
				-> std::tuple<std::tuple_element_t<0, Entry>, std::tuple_element_t<1, Entry>, size_t, size_t>
			{
				using Key = std::tuple_element_t<0, Entry>;
				using Payload = std::tuple_element_t<1, Entry>;

				constexpr auto kTupleSize = std::tuple_size_v<Entry>;
				static_assert(2 <= kTupleSize && kTupleSize <= 4);

				if constexpr (kTupleSize == 4) {
					return entry;
				}
				else if constexpr (kTupleSize == 3) {
					const auto& [key, payload, key_len] = entry;
					return { key, payload, key_len, sizeof(Payload) };
				}
				else {
					const auto& [key, payload] = entry;
					return { key, payload, sizeof(Key), sizeof(Payload) };
				}
			}

			/**
			 * @brief Parse an entry of bulkload according to key's type.
			 *
			 * @tparam Entry std::pair or std::tuple for containing entries.
			 * @param entry a bulkload entry.
			 * @retval 1st: a target key.
			 * @retval 2nd: the length of a target key.
			 */
			template <class Entry>
			constexpr auto
				ParseKey(const Entry& entry)  //
				-> std::pair<std::tuple_element_t<0, Entry>, size_t>
			{
				using Key = std::tuple_element_t<0, Entry>;

				constexpr auto kTupleSize = std::tuple_size_v<Entry>;
				static_assert(2 <= kTupleSize && kTupleSize <= 4);

				if constexpr (kTupleSize == 4) {
					const auto& [key, payload, key_len, pay_len] = entry;
					return { key, key_len };
				}
				else if constexpr (kTupleSize == 3) {
					const auto& [key, payload, key_len] = entry;
					return { key, key_len };
				}
				else {
					const auto& [key, payload] = entry;
					return { key, sizeof(Key) };
				}
			}

			template <class T>
			inline auto
				DeepCopy(  //
					const T& obj,
					[[maybe_unused]] const size_t len)  //
				-> T
			{
				return T{ obj };
			}

		}  // namespace dbgroup::index::bw_tree::component
		// logical_ptr
		namespace dbgroup::index::bw_tree::component
		{
			/**
			 * @brief A class for wrapping physical pointers in logical ones.
			 *
			 */
			class LogicalPtr
			{
			public:
				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				constexpr LogicalPtr() = default;

				LogicalPtr(const LogicalPtr&) = delete;
				LogicalPtr(LogicalPtr&&) = delete;

				auto operator=(const LogicalPtr&)->LogicalPtr & = delete;
				auto operator=(LogicalPtr&&)->LogicalPtr & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				~LogicalPtr() = default;

				/*####################################################################################
				 * Public utility functions
				 *##################################################################################*/

				 /**
				  * @brief Load a current physical pointer from this logical ID.
				  *
				  * Note that this function sets an acquire fence.
				  *
				  * @tparam T a target class to be loaded.
				  * @return an address of a base node or a delta record.
				  */
				template <class T>
				[[nodiscard]] auto
					Load() const  //
					-> T
				{
					static_assert(IsValidTarget<T>());

					return reinterpret_cast<T>(physical_ptr_.load(std::memory_order_acquire));
				}

				/**
				 * @brief Store a physical pointer into this logical ID.
				 *
				 * Note that this function sets a release fence.
				 *
				 * @tparam T a target class to be stored.
				 * @param desired a physical pointer to be stored.
				 */
				template <class T>
				void
					Store(const T desired)
				{
					static_assert(IsValidTarget<T>());

					physical_ptr_.store(reinterpret_cast<uintptr_t>(desired), std::memory_order_release);
				}

				/**
				 * @brief Clear a stored address from this logical ID.
				 *
				 */
				void
					Clear()
				{
					physical_ptr_.store(kNullPtr, std::memory_order_relaxed);
				}

				/**
				 * @brief Perform a weak-CAS operation with given expected/desired values.
				 *
				 * Note that this function sets a release fence.
				 *
				 * @tparam T1 a class of an expected value.
				 * @tparam T2 a class of a desired value.
				 * @param expected an expected value to be compared.
				 * @param desired a desired value to be stored.
				 * @return true if this CAS operation succeeds.
				 * @return false otherwise.
				 */
				template <class T1, class T2>
				auto
					CASWeak(  //
						const T1 expected,
						const T2 desired)  //
					-> bool
				{
					static_assert(IsValidTarget<T1>());
					static_assert(IsValidTarget<T2>());

					auto old_ptr = reinterpret_cast<uintptr_t>(expected);
					return physical_ptr_.compare_exchange_weak(  //
						old_ptr,                                 //
						reinterpret_cast<uintptr_t>(desired),    //
						std::memory_order_release);
				}

				/**
				 * @brief Perform a strong-CAS operation with given expected/desired values.
				 *
				 * Note that this function sets a release fence.
				 *
				 * @tparam T1 a class of an expected value.
				 * @tparam T2 a class of a desired value.
				 * @param expected an expected value to be compared.
				 * @param desired a desired value to be stored.
				 * @return true if this CAS operation succeeds.
				 * @return false otherwise.
				 */
				template <class T1, class T2>
				auto
					CASStrong(  //
						const T1 expected,
						const T2 desired)  //
					-> bool
				{
					static_assert(IsValidTarget<T1>());
					static_assert(IsValidTarget<T2>());

					auto old_ptr = reinterpret_cast<uintptr_t>(expected);
					return physical_ptr_.compare_exchange_strong(  //
						old_ptr,                                   //
						reinterpret_cast<uintptr_t>(desired),      //
						std::memory_order_release);
				}

			private:
				/*####################################################################################
				 * Internal utility functions
				 *##################################################################################*/

				 /**
				  * @tparam T a target class to be wrapped.
				  * @retval true if a given template class is valid as physical pointers.
				  * @retval false otherwise.
				  */
				template <class T>
				static constexpr auto
					IsValidTarget()  //
					-> bool
				{
					if constexpr (std::is_same_v<T, uintptr_t>) {
						return true;
					}
					else if constexpr (std::is_pointer_v<T>) {
						return true;
					}
					else {
						return false;
					}
				}

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 // an address of a node or a delta record.
				std::atomic_uintptr_t physical_ptr_{ kNullPtr };
			};

		}  // namespace dbgroup::index::bw_tree::component
		// delta_chain
		namespace dbgroup::index::bw_tree::component
		{
			/**
			 * @brief A class for managing procedures for delta-chains.
			 *
			 * @tparam DeltaRecord a class of delta records.
			 */
			template <class DeltaRecord>
			class DeltaChain
			{
			public:
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using Key = typename DeltaRecord::Key;
				using Comp = typename DeltaRecord::Comp;
				using Record = typename DeltaRecord::Record;
				using ConsolidateInfo = std::pair<const void*, const void*>;

				/*####################################################################################
				 * Public utilities
				 *##################################################################################*/

				 /**
				  * @brief Traverse a delta-chain to search a child node with a given key.
				  *
				  * @param delta the head record in a delta-chain.
				  * @param key a target key to be searched.
				  * @param closed a flag for including the same key.
				  * @param out_ptr an output pointer if needed.
				  * @retval kRecordFound if a delta record (in out_ptr) has a corresponding child.
				  * @retval kReachBaseNode if a base node (in out_ptr) has a corresponding child.
				  * @retval kKeyIsInSibling if the target key is not in this node due to other SMOs.
				  * @retval kNodeRemoved if this node is removed by other SMOs.
				  */
				static auto
					SearchChildNode(  //
						const DeltaRecord* delta,
						const Key& key,
						const bool closed,
						uintptr_t& out_ptr)  //
					-> DeltaRC
				{
					auto has_smo = false;

					// traverse a delta chain
					for (; true; delta = delta->GetNext()) {
						switch (delta->GetDeltaType()) {
						case kInsert:
							if (delta->LowKeyIsLE(key, closed) && delta->HighKeyIsGE(key, !closed)) {
								// this index-entry delta directly indicates a child node
								out_ptr = delta->template GetPayload<uintptr_t>();
								return kRecordFound;
							}
							break;

						case kDelete:
							if (delta->LowKeyIsLE(key, closed) && delta->HighKeyIsGE(key, !closed)) {
								// this index-entry delta directly indicates a child node
								out_ptr = delta->GetPayloadAtomically();
								return kRecordFound;
							}
							break;

						case kRemoveNode:
							return kNodeRemoved;

						case kMerge:
							// check whether the merged node contains a target key
							if (delta->LowKeyIsLE(key, closed)) {
								// check whether the node contains a target key
								const auto* merged_node = delta->template GetPayload<DeltaRecord*>();
								if (!has_smo && !delta->HighKeyIsGE(key, !closed)) {
									out_ptr = merged_node->template GetNext<uintptr_t>();
									return kKeyIsInSibling;
								}

								// a target record may be in the merged node
								out_ptr = reinterpret_cast<uintptr_t>(merged_node);
								return kReachBaseNode;
							}
							has_smo = true;
							break;

						case kNotDelta:
						default:
							if (!has_smo && !delta->HighKeyIsGE(key, !closed)) {
								// a sibling node includes a target key
								out_ptr = delta->template GetNext<uintptr_t>();
								return kKeyIsInSibling;
							}

							// reach a base page
							out_ptr = reinterpret_cast<uintptr_t>(delta);
							return kReachBaseNode;
						}
					}
				}

				/**
				 * @brief Traverse a delta-chain to search a record with a given key.
				 *
				 * @param delta the head record in a delta-chain.
				 * @param key a target key to be searched.
				 * @param out_ptr an output pointer if needed.
				 * @retval kRecordFound if a delta record (in out_ptr) has the given key.
				 * @retval kReachBaseNode if a base node (in out_ptr) may have the given key.
				 * @retval kKeyIsInSibling if the target key is not in this node due to other SMOs.
				 * @retval kNodeRemoved if this node is removed by other SMOs.
				 */
				static auto
					SearchRecord(  //
						const DeltaRecord* delta,
						const Key& key,
						uintptr_t& out_ptr)  //
					-> DeltaRC
				{
					auto has_smo = false;

					// traverse a delta chain
					for (; true; delta = delta->GetNext()) {
						switch (delta->GetDeltaType()) {
						case kInsert:
						case kModify:
							// check whether a target record is inserted
							if (delta->HasSameKey(key)) {
								out_ptr = reinterpret_cast<uintptr_t>(delta);
								return kRecordFound;
							}
							break;

						case kDelete:
							// check whether a target record is deleted
							if (delta->HasSameKey(key)) return kRecordNotFound;
							break;

						case kRemoveNode:
							return kNodeRemoved;

						case kMerge:
							// check whether the merged node contains a target key
							if (delta->LowKeyIsLE(key, kClosed)) {
								// check whether the node contains a target key
								const auto* merged_node = delta->template GetPayload<DeltaRecord*>();
								if (!has_smo && !delta->HighKeyIsGE(key, kOpen)) {
									out_ptr = merged_node->template GetNext<uintptr_t>();
									return kKeyIsInSibling;
								}

								// a target record may be in the merged node
								out_ptr = reinterpret_cast<uintptr_t>(merged_node);
								return kReachBaseNode;
							}
							has_smo = true;
							break;

						case kNotDelta:
						default:
							// check whether the node contains a target key
							if (!has_smo && !delta->HighKeyIsGE(key, kOpen)) {
								out_ptr = delta->template GetNext<uintptr_t>();
								return kKeyIsInSibling;
							}

							// a target record may be in the base node
							out_ptr = reinterpret_cast<uintptr_t>(delta);
							return kReachBaseNode;
						}
					}
				}

				/**
				 * @brief Traverse a delta-chain to search a record with given keys.
				 *
				 * @param delta the head record in a delta-chain.
				 * @param key a target key to be searched.
				 * @param sib_key a sibling key to be searched.
				 * @param out_ptr an output pointer if needed.
				 * @param key_found a flag for indicating a target key has been found.
				 * @param sib_key_found a flag for indicating a sibling key has been found.
				 * @retval kRecordFound if a delta record (in out_ptr) has the given key.
				 * @retval kReachBaseNode if a base node (in out_ptr) may have the given key.
				 * @retval kKeyIsInSibling if the target key is not in this node due to other SMOs.
				 * @retval kNodeRemoved if this node is removed by other SMOs.
				 */
				static auto
					SearchForMerge(  //
						const DeltaRecord* delta,
						const Key& key,
						const std::optional<Key>& sib_key,
						uintptr_t& out_ptr,
						bool& key_found,
						bool& sib_key_found)  //
					-> DeltaRC
				{
					auto has_smo = false;

					// traverse a delta chain
					for (; true; delta = delta->GetNext()) {
						switch (delta->GetDeltaType()) {
						case kInsert:
							// check whether a target record is inserted
							if (!key_found && delta->HasSameKey(key)) {
								key_found = true;
								if (sib_key_found) return kRecordFound;
							}
							if (!sib_key_found && delta->HasSameKey(*sib_key)) {
								sib_key_found = true;
								if (key_found) return kRecordFound;
							}
							break;

						case kDelete:
							// check whether a target record is deleted
							if (!key_found && delta->HasSameKey(key)) return kAbortMerge;
							if (!sib_key_found && delta->HasSameKey(*sib_key)) return kAbortMerge;  // merged node
							break;

						case kRemoveNode:
							return kNodeRemoved;

						case kMerge:
							// check whether the merged node contains a target key
							if (delta->LowKeyIsLE(key, kClosed)) {
								// check whether the node contains a target key
								const auto* merged_node = delta->template GetPayload<DeltaRecord*>();
								if (!has_smo && !key_found && !delta->HighKeyIsGE(key, kOpen)) {
									out_ptr = merged_node->template GetNext<uintptr_t>();
									return kKeyIsInSibling;
								}

								// a target record may be in the merged node
								out_ptr = reinterpret_cast<uintptr_t>(merged_node);
								return kReachBaseNode;
							}
							if (!sib_key_found && delta->HasSameKey(*sib_key)) {
								sib_key_found = true;
								if (key_found) return kRecordFound;
							}
							has_smo = true;
							break;

						case kNotDelta:
						default:
							// check whether the node contains a target key
							if (!key_found) {
								if (!delta->IsLeftmost() && delta->HasSameKey(key)) return kAbortMerge;
								if (!has_smo && !delta->HighKeyIsGE(key, kOpen)) {
									out_ptr = delta->template GetNext<uintptr_t>();
									return kKeyIsInSibling;
								}
							}

							// a target record may be in the base node
							out_ptr = reinterpret_cast<uintptr_t>(delta);
							return kReachBaseNode;
						}
					}
				}

				/**
				 * @brief Traverse a delta-chain to check this node is valid for modifying this tree.
				 *
				 * @param delta the head record in a delta-chain.
				 * @param key a target key to be searched.
				 * @param closed a flag for including the same key.
				 * @param out_ptr an output pointer if needed.
				 * @retval kReachBaseNode if this node is valid for the given key.
				 * @retval kKeyIsInSibling if the target key is not in this node due to other SMOs.
				 * @retval kNodeRemoved if this node is removed by other SMOs.
				 */
				static auto
					Validate(  //
						const DeltaRecord* delta,
						const Key& key,
						const bool closed,
						uintptr_t& out_ptr)  //
					-> DeltaRC
				{
					// traverse a delta chain
					for (; true; delta = delta->GetNext()) {
						switch (delta->GetDeltaType()) {
						case kRemoveNode:
							return kNodeRemoved;

						case kMerge:
							// check whether the node contains a target key
							if (!delta->HighKeyIsGE(key, !closed)) {
								const auto* merged_node = delta->template GetPayload<DeltaRecord*>();
								out_ptr = merged_node->template GetNext<uintptr_t>();
								return kKeyIsInSibling;
							}
							return kReachBaseNode;

						case kNotDelta:
							// check whether the node contains a target key
							if (!delta->HighKeyIsGE(key, !closed)) {
								out_ptr = delta->template GetNext<uintptr_t>();
								return kKeyIsInSibling;
							}
							return kReachBaseNode;

						default:
							break;  // do nothing
						}
					}
				}

				/**
				 * @brief Sort delta records for consolidation.
				 *
				 * @param delta the head record in a delta-chain.
				 * @param arr a vector for storing sorted records.
				 * @param nodes a vector for storing base nodes and corresponding separator keys.
				 * @return The number of delta records.
				 */
				static auto
					Sort(  //
						const DeltaRecord* delta,
						std::array<Record, kMaxDeltaRecordNum>& arr,
						std::vector<const void*>& nodes)  //
					-> size_t
				{
					// traverse and sort a delta chain
					size_t rec_num = 0;
					for (; true; delta = delta->GetNext()) {
						switch (delta->GetDeltaType()) {
						case kInsert:
						case kModify:
						case kDelete:
							delta->AddByInsertionSortTo(arr, rec_num);
							break;

						case kMerge: {
							// keep the merged node and the corresponding separator key
							nodes.emplace_back(delta->GetPayload());
							break;
						}

						case kNotDelta:
						default:
							nodes.emplace_back(delta);
							return rec_num;
						}
					}
				}
			};

		}  // namespace dbgroup::index::bw_tree::component
		// mapping table
		namespace dbgroup::index::bw_tree::component
		{
			/**
			 * @brief A class for managing locigal IDs.
			 *
			 * @tparam Node a class for representing base nodes.
			 * @tparam Delta a class for representing delta records.
			 */
			template <class Node, class Delta>
			class alignas(kVMPageSize) MappingTable
			{
			public:
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				template <class T>
				class alignas(kVMPageSize) Arr
				{
				public:
					auto
						Get(const size_t pos)  //
						-> T&
					{
						return elements_[pos];
					}

				private:
					T elements_[0];
				};

				using Row = Arr<LogicalPtr>;
				using Table = Arr<std::atomic<Row*>>;

				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct a new MappingTable object.
				  *
				  */
				MappingTable()
				{
					auto* row = dbgroup::memory::Allocate<Row>(kVMPageSize);
					auto* table = dbgroup::memory::Allocate<Table>(kVMPageSize);
					table->Get(0).store(row, std::memory_order_relaxed);
					tables_[0].store(table, std::memory_order_relaxed);
				}

				MappingTable(const MappingTable&) = delete;
				MappingTable(MappingTable&&) = delete;

				auto operator=(const MappingTable&)->MappingTable & = delete;
				auto operator=(MappingTable&&)->MappingTable & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the MappingTable object.
				  *
				  * This destructor will deletes all the actual pages of a Bw-tree.
				  */
				~MappingTable()
				{
					const auto cur_id = cnt_.load(std::memory_order_relaxed);
					const auto table_id = (cur_id >> kTabShift) & kIDMask;
					for (size_t i = 0; i < table_id; ++i) {
						const auto is_last = (i + 1) == table_id;
						const auto row_num = is_last ? ((cur_id >> kRowShift) & kIDMask) : kArrayCapacity;
						const auto col_num = is_last ? (cur_id & kIDMask) : kArrayCapacity;

						auto* table = tables_[i].load(std::memory_order_relaxed);
						for (size_t j = 0; j < row_num; ++j) {
							auto* row = table->Get(j).load(std::memory_order_relaxed);
							for (size_t k = 0; k < col_num; ++k) {
								ReleaseLogicalPtr(row->Get(k));
							}
							dbgroup::memory::Release<Row>(row);
						}
						dbgroup::memory::Release<Table>(table);
					}
				}

				/*####################################################################################
				 * Public getters/setters
				 *##################################################################################*/

				 /**
				  * @brief Get a new page ID.
				  *
				  * @return a reserved page ID.
				  */
				auto
					GetNewPageID()  //
					-> uint64_t
				{
					while (true) {
						auto cur_id = cnt_.load(std::memory_order_relaxed);
						assert(((cur_id >> kTabShift) & kIDMask) < kArrayCapacity);

						if ((cur_id & kIDMask) < kArrayCapacity) {
							// try reserving a new ID
							auto new_id = cur_id + kColIDUnit;
							if (cnt_.compare_exchange_weak(cur_id, new_id, std::memory_order_relaxed)) {
								// check the current row is full
								if ((new_id & kIDMask) < kArrayCapacity) return cur_id;

								// check the current table is full
								new_id += kRowIDUnit;
								const auto row_id = (new_id >> kRowShift) & kIDMask;
								if (row_id < kArrayCapacity) {
									// prepare a new row
									auto* row = dbgroup::memory::Allocate<Row>(kVMPageSize);
									auto* table = tables_[(new_id >> kTabShift) & kIDMask].load(std::memory_order_relaxed);
									table->Get(row_id).store(row, std::memory_order_relaxed);
									cnt_.store(new_id & ~kIDMask, std::memory_order_relaxed);
									return cur_id;
								}

								// prepare a new table
								auto* row = dbgroup::memory::Allocate<Row>(kVMPageSize);
								auto* table = dbgroup::memory::Allocate<Table>(kVMPageSize);
								table->Get(0).store(row, std::memory_order_relaxed);
								new_id += kTabIDUnit;
								tables_[(new_id >> kTabShift) & kIDMask].store(table, std::memory_order_relaxed);
								cnt_.store(new_id & ~(kTabIDUnit - 1UL), std::memory_order_relaxed);
								return cur_id;
							}
						}
					}
				}

				/**
				 * @param id a source page ID.
				 * @return an address of a logical pointer.
				 */
				[[nodiscard]] auto
					GetLogicalPtr(const uint64_t id) const  //
					-> LogicalPtr*
				{
					auto* table = tables_[(id >> kTabShift) & kIDMask].load(std::memory_order_relaxed);
					auto* row = table->Get((id >> kRowShift) & kIDMask).load(std::memory_order_relaxed);
					return const_cast<LogicalPtr*>(&(row->Get(id & kIDMask)));
				}

				/*####################################################################################
				 * Public utilities
				 *##################################################################################*/

				 /**
				  * @brief Collect statistical data of this tree.
				  *
				  * @retval 1st: the number of nodes (always zero).
				  * @retval 2nd: the actual usage in bytes.
				  * @retval 3rd: the virtual usage in bytes.
				  */
				auto
					CollectStatisticalData()  //
					-> std::tuple<size_t, size_t, size_t>
				{
					const auto id = cnt_.load(std::memory_order_relaxed);
					const auto tab_id = (id >> kTabShift) & kIDMask;
					const auto row_id = (id >> kRowShift) & kIDMask;
					const auto col_id = id & kIDMask;

					const auto reserved = (tab_id * (kArrayCapacity + 1) + row_id + 3) * kVMPageSize;
					const auto empty_num = 2 * kArrayCapacity + kTableCapacity - col_id - row_id - tab_id - 3;
					const auto used = reserved - empty_num * kWordSize;

					return { 0, used, reserved };
				}

			private:
				/*####################################################################################
				 * Internal classes
				 *##################################################################################*/

				 /*####################################################################################
				  * Internal constants
				  *##################################################################################*/

				  /// the begin bit position of row IDs.
				static constexpr size_t kRowShift = 16;

				/// the begin bit position of table IDs.
				static constexpr size_t kTabShift = 32;

				/// the begin bit position for indicating null pointers.
				static constexpr size_t kMSBShift = 63;

				/// the unit value for incrementing column IDs.
				static constexpr uint64_t kColIDUnit = 1UL;

				/// the unit value for incrementing row IDs.
				static constexpr uint64_t kRowIDUnit = 1UL << kRowShift;

				/// the unit value for incrementing table IDs.
				static constexpr uint64_t kTabIDUnit = 1UL << kTabShift;

				/// a bit mask for extracting IDs.
				static constexpr uint64_t kIDMask = 0xFFFFUL;

				/// the capacity of each array (rows and columns).
				static constexpr size_t kArrayCapacity = kVMPageSize / kWordSize;

				/// the capacity of a table.
				static constexpr size_t kTableCapacity = (kVMPageSize - kCacheLineSize) / kWordSize;

				/*####################################################################################
				 * Internal utilities
				 *##################################################################################*/

				 /**
				  * @brief Release delta records and nodes in a given logical pointer.
				  *
				  * @param lid a target logical pointer.
				  */
				void
					ReleaseLogicalPtr(LogicalPtr& lid)
				{
					auto* rec = lid.template Load<Delta*>();

					// delete delta records
					while (rec != nullptr && rec->GetDeltaType() != kNotDelta) {
						auto* next = rec->GetNext();
						if (rec->GetDeltaType() == kMerge) {
							dbgroup::memory::Release<NodePage>(rec->template GetPayload<void*>());
						}

						dbgroup::memory::Release<DeltaPage>(rec);
						rec = next;
					}
					dbgroup::memory::Release<NodePage>(rec);
				}

				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// an atomic counter for incrementing page IDs.
				std::atomic_uint64_t cnt_{ 1UL << kMSBShift };

				/// padding space for the cache line alignment.
				size_t padding_[(kCacheLineSize - kWordSize) / kWordSize]{};

				/// mapping tables.
				std::atomic<Table*> tables_[kTableCapacity]{};
			};

		}  // namespace dbgroup::index::bw_tree::component
		// record iterator
		namespace dbgroup::index::bw_tree
		{
			// forward declaratoin
			template <class K, class V, class C>
			class BwTree;

			namespace component
			{
				/**
				 * @brief A class for representing an iterator of scan results.
				 *
				 */
				template <class Key, class Payload, class Comp>
				class RecordIterator
				{
				public:
					/*####################################################################################
					 * Type aliases
					 *##################################################################################*/

					using ScanKey = std::optional<std::tuple<const Key&, size_t, bool>>;
					using BwTree_t = BwTree<Key, Payload, Comp>;
					using Node_t = typename BwTree_t::Node_t;
					friend class BwTree_t;

					/*####################################################################################
					 * Public constructors and assignment operators
					 *##################################################################################*/

					 /**
					  * @brief Construct a new object as an initial iterator.
					  *
					  * @param bw_tree a pointer to an index.
					  * @param node a copied node for scanning results.
					  * @param begin_pos the begin position of a current node.
					  * @param end_pos the end position of a current node.
					  * @param end_key an optional end-point key.
					  * @param is_end a flag for indicating a current node is rightmost in scan-range.
					  */
					RecordIterator(  //
						BwTree_t* bw_tree,
						Node_t* node,
						size_t begin_pos,
						size_t end_pos,
						const ScanKey end_key,
						const bool is_end)
						: bw_tree_{ bw_tree },
						node_{ node },
						rec_count_{ end_pos },
						current_pos_{ begin_pos },
						end_key_{ std::move(end_key) },
						is_end_{ is_end }
					{
					}

					/**
					 * @brief Construct a new object for sibling scanning.
					 *
					 * @param node a copied node for scanning results.
					 * @param begin_pos the begin position of a current node.
					 * @param end_pos the end position of a current node.
					 * @param is_end a flag for indicating a current node is rightmost in scan-range.
					 */
					RecordIterator(  //
						Node_t* node,
						size_t begin_pos,
						size_t end_pos,
						const bool is_end)
						: node_{ node }, rec_count_{ end_pos }, current_pos_{ begin_pos }, is_end_{ is_end }
					{
					}

					RecordIterator() = default;

					RecordIterator(const RecordIterator& r) : node_{ r.node_ }, rec_count_{ r.rec_count_ }, current_pos_{ r.current_pos_ }, is_end_{ r.is_end_ } {}
					RecordIterator(RecordIterator&& r) : node_{ r.node_ }, rec_count_{ r.rec_count_ }, current_pos_{ r.current_pos_ }, is_end_{ r.is_end_ } {}

					// auto operator=(const RecordIterator&)->RecordIterator & = delete;
					constexpr auto
						operator=(RecordIterator const& obj) noexcept  //
						-> RecordIterator&
					{
						node_ = obj.node_;
						rec_count_ = obj.rec_count_;
						current_pos_ = obj.current_pos_;
						is_end_ = obj.is_end_;

						return *this;
					}

					constexpr auto
						operator=(RecordIterator&& obj) noexcept  //
						-> RecordIterator&
					{
						node_ = obj.node_;
						rec_count_ = obj.rec_count_;
						current_pos_ = obj.current_pos_;
						is_end_ = obj.is_end_;

						return *this;
					}

					/*####################################################################################
					 * Public destructors
					 *##################################################################################*/

					 /**
					  * @brief Destroy the iterator and a retained node if exist.
					  *
					  */
					~RecordIterator() = default;

					/*####################################################################################
					 * Public operators for iterators
					 *##################################################################################*/

					 /**
					  * @retval true if this iterator indicates a live record.
					  * @retval false otherwise.
					  */
					explicit
						operator bool()
					{
						return HasRecord();
					}

					/**
					 * @return a current key and payload pair.
					 */
					auto
						operator*() const  //
						-> std::pair<Key, Payload>
					{
						return node_->template GetRecord<Payload>(current_pos_);
					}

					/**
					 * @brief Forward this iterator.
					 *
					 */
					constexpr void
						operator++()
					{
						++current_pos_;
					}

					/**
					 * @brief Forward this iterator.
					 *
					 */
					constexpr void
						operator++(int)
					{
						++current_pos_;
					}

					/**
					 * @brief Forward this iterator.
					 *
					 */
					constexpr void
						operator--()
					{
						--current_pos_;
					}

					/**
					 * @brief Forward this iterator.
					 *
					 */
					constexpr void
						operator--(int)
					{
						--current_pos_;
					}

					/**
					 * @return a current key and payload pair.
					 */
					constexpr auto
						operator-(const RecordIterator& r) const //
						-> long long
					{
						return static_cast<long long>(current_pos_) - static_cast<long long>(r.current_pos_);
					}

					/**
					 * @brief Forward this iterator.
					 *
					 */
					constexpr auto
						operator==(const RecordIterator& r) const
						-> bool
					{
						if (r.is_end_) {
							return !const_cast<RecordIterator*>(this)->HasRecord();
						}
						else if (is_end_) {
							return !const_cast<RecordIterator&>(r).HasRecord();
						}
						else {
							return current_pos_ == r.current_pos_;
						}
					}


					/*####################################################################################
					 * Public getters/setters
					 *##################################################################################*/

					 /**
					  * @brief Check if there are any records left.
					  *
					  * NOTE: this may call a scanning function internally to get a sibling node.
					  *
					  * @retval true if there are any records or next node left.
					  * @retval false otherwise.
					  */
					[[nodiscard]] auto
						HasRecord()  //
						-> bool
					{
						while (true) {
							if (current_pos_ < rec_count_) return true;  // records remain in this node
							if (is_end_) return false;                   // this node is the end of range-scan

							// go to the next sibling node and continue scanning
							const auto& next_key = node_->GetHighKey();
							const auto sib_pid = node_->template GetNext<PageID>();
							*this = bw_tree_->SiblingScan(sib_pid, node_, next_key, end_key_);
						}
					}

					/**
					 * @return a key of a current record
					 */
					[[nodiscard]] auto
						GetKey() const  //
						-> Key
					{
						return node_->GetKey(current_pos_);
					}

					/**
					 * @return a payload of a current record
					 */
					[[nodiscard]] auto
						GetPayload() const  //
						-> Payload
					{
						return node_->template GetPayload<Payload>(current_pos_);
					}

				protected:
					/*####################################################################################
					 * Internal member variables
					 *##################################################################################*/

					 /// a pointer to a BwTree_t for sibling scanning.
					BwTree_t* bw_tree_{ nullptr };

					/// the pointer to a node that includes partial scan results.
					Node_t* node_{ nullptr };

					/// the number of records in this node.
					size_t rec_count_{ 0 };

					/// the position of a current record.
					size_t current_pos_{ 0 };

					/// the end key given from a user.
					ScanKey end_key_{};

					/// a flag for indicating a current node is rightmost in scan-range.
					bool is_end_{ true };
				};

			}  // namespace component
		}  // namespace dbgroup::index::bw_tree
		// fixed length delta record
		namespace dbgroup::index::bw_tree::component::fixlen
		{
			template <class Key_t, class Comp_t>
			class DeltaRecord_SizeTest
			{
			public:
				using Key = Key_t;
				using Comp = Comp_t;

			public:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// a flag for indicating whether this node is a leaf or internal node.
				uint16_t is_inner_ : 1;

				/// a flag for indicating the types of a delta record.
				uint16_t delta_type_ : 3;

				/// a flag for indicating whether this delta record has a lowest-key.
				uint16_t has_low_key_ : 1;

				/// a flag for indicating whether this delta record has a highest-key.
				uint16_t has_high_key_ : 1;

				/// a blank block for alignment.
				uint16_t : 0;

				/// the number of delta records in this chain.
				uint16_t rec_count_{ 0 };

				/// the size of this logical node in bytes.
				uint32_t node_size_{ 0 };

				/// the pointer to the next node.
				uintptr_t next_{ kNullPtr };

				/// metadata of an embedded record
				Key key_{};

				/// metadata of a highest key
				Key high_key_{};

				/// an actual data block for records
				std::byte payload_[0]{};

			};

			/**
			 * @brief A class to represent delta records in Bw-tree.
			 *
			 * @tparam Key a target key class.
			 * @tparam Comp a comparetor class for keys.
			 */
			template <class Key_t, class Comp_t>
			class DeltaRecord
			{
			public:
				using Key = Key_t;
				using Comp = Comp_t;

			public:
				/*####################################################################################
				 * Internal member variables
				 *##################################################################################*/

				 /// a flag for indicating whether this node is a leaf or internal node.
				uint16_t is_inner_ : 1;

				/// a flag for indicating the types of a delta record.
				uint16_t delta_type_ : 3;

				/// a flag for indicating whether this delta record has a lowest-key.
				uint16_t has_low_key_ : 1;

				/// a flag for indicating whether this delta record has a highest-key.
				uint16_t has_high_key_ : 1;

				/// a blank block for alignment.
				uint16_t : 0;

				/// the number of delta records in this chain.
				uint16_t rec_count_{ 0 };

				/// the size of this logical node in bytes.
				uint32_t node_size_{ 0 };

				/// the pointer to the next node.
				uintptr_t next_{ kNullPtr };

				/// metadata of an embedded record
				Key key_{};

				/// metadata of a highest key
				Key high_key_{};

				/// an actual data block for records
				std::byte payload_[0]{}; // variable-length object, whose size is determined at runtime, with all other items being the header for it.

			public:
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using Key = Key_t;
				using Comp = Comp_t;

				/*####################################################################################
				 * Public classes
				 *##################################################################################*/

				 /**
				  * @brief A class to sort delta records.
				  *
				  */
				struct Record {
					Key key;
					const void* ptr;
				};

				/*####################################################################################
				 * Public constructors for inserting/deleting records in leaf nodes
				 *##################################################################################*/

				 /**
				  * @brief Construct a new delta record for inserting/modifying a record.
				  *
				  * @tparam T a target payload class.
				  * @param delta_type an insert or modify delta.
				  * @param key a key to be inserted.
				  * @param key_len the length of a target key.
				  * @param payload a payload to be inserted.
				  */
				template <class T>
				DeltaRecord(  //
					const DeltaType delta_type,
					const Key& key,
					[[maybe_unused]] const size_t key_len,
					const T& payload)
					: is_inner_{ kLeaf }, delta_type_{ delta_type }, has_low_key_{ 1 }, has_high_key_{ 0 }, key_{ key }
				{
					SetPayload(payload);
				}

				/**
				 * @brief Construct a new delta record for deleting a record.
				 *
				 * @param key a key to be deleted.
				 * @param key_len the length of a target key.
				 */
				explicit DeltaRecord(  //
					const Key& key,
					[[maybe_unused]] const size_t key_len)
					: is_inner_{ kLeaf }, delta_type_{ kDelete }, has_low_key_{ 1 }, has_high_key_{ 0 }, key_{ key }
				{
				}

				/*####################################################################################
				 * Public constructors for performing SMOs
				 *##################################################################################*/

				 /**
				  * @brief Construct a new delta record for splitting/merging a node.
				  *
				  * @param d_type a split or merge delta.
				  * @param r_node a split/merged right node.
				  * @param r_pid the page ID of a split/merged right node.
				  */
				DeltaRecord(  //
					const DeltaType d_type,
					const DeltaRecord* r_node,
					const PageID r_pid)
					: is_inner_{ d_type == kInsert ? static_cast<uint16_t>(kInner) : r_node->is_inner_ },
					delta_type_{ d_type },
					has_low_key_{ 1 },
					has_high_key_{ r_node->has_high_key_ },
					key_{ r_node->key_ },
					high_key_{ r_node->high_key_ }
				{
					// set a sibling node
					SetPayload(r_pid);
				}

				/**
				 * @brief Construct a new delta record for deleting an index-entry.
				 *
				 * @param removed_node a removed node.
				 */
				explicit DeltaRecord(const DeltaRecord* removed_node)
					: is_inner_{ kInner },
					delta_type_{ kDelete },
					has_low_key_{ 1 },
					has_high_key_{ removed_node->has_high_key_ },
					key_{ removed_node->key_ },
					high_key_{ removed_node->high_key_ }
				{
					// set a sibling node
					auto* payload = reinterpret_cast<std::atomic<PageID> *>(ShiftAddr(this, kPayOffset));
					payload->store(kNullPtr, std::memory_order_relaxed);
				}

				/**
				 * @brief Construct a new delta record for removing a node.
				 *
				 * @param is_leaf a flag for indicating leaf nodes.
				 */
				explicit DeltaRecord(const bool is_leaf)
					: is_inner_{ static_cast<uint16_t>(!is_leaf) },
					delta_type_{ kRemoveNode },
					has_low_key_{ 0 },
					has_high_key_{ 0 }
				{
				}

				/*####################################################################################
				 * Public assignment operators
				 *##################################################################################*/

				DeltaRecord(const DeltaRecord&) = delete;
				DeltaRecord(DeltaRecord&&) noexcept = delete;

				auto operator=(const DeltaRecord&)->DeltaRecord & = delete;
				auto operator=(DeltaRecord&&) noexcept -> DeltaRecord & = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				~DeltaRecord() = default;

				/*####################################################################################
				 * Public getters/setters for a header
				 *##################################################################################*/

				 /**
				  * @retval true if this is a leaf node.
				  * @retval false otherwise.
				  */
				[[nodiscard]] constexpr auto
					IsLeaf() const  //
					-> bool
				{
					return is_inner_ == kLeaf;
				}

				/**
				 * @retval true if this node is leftmost in its tree level.
				 * @retval false otherwise.
				 */
				[[nodiscard]] constexpr auto
					IsLeftmost() const  //
					-> bool
				{
					return has_low_key_ == 0;
				}

				/**
				 * @param key a target key to be compared.
				 * @retval true if this record has the same key with a given one.
				 * @retval false otherwise.
				 */
				[[nodiscard]] constexpr auto
					HasSameKey(const Key& key) const  //
					-> bool
				{
					return !Comp{}(key, key_) && !Comp{}(key_, key);
				}

				/**
				 * @param key a target key to be compared.
				 * @param closed a flag for including the same key.
				 * @retval true if the lowest key is less than or equal to a given key.
				 * @retval false otherwise.
				 */
				[[nodiscard]] constexpr auto
					LowKeyIsLE(  //
						const Key& key,
						const bool closed) const  //
					-> bool
				{
					return Comp{}(key_, key) || (closed && !Comp{}(key, key_));
				}

				/**
				 * @param key a target key to be compared.
				 * @param closed a flag for including the same key.
				 * @retval true if the highest key is greater than a given key.
				 * @retval false otherwise.
				 */
				[[nodiscard]] constexpr auto
					HighKeyIsGE(  //
						const Key& key,
						const bool closed) const  //
					-> bool
				{
					return !has_high_key_ || Comp{}(key, high_key_) || (closed && !Comp{}(high_key_, key));
				}

				/**
				 * @retval true if there is a delta chain.
				 * @retval false otherwise.
				 */
				[[nodiscard]] constexpr auto
					NeedConsolidation() const  //
					-> bool
				{
					return delta_type_ != kNotDelta
						&& (rec_count_ >= kDeltaRecordThreshold || node_size_ > kPageSize);
				}

				/**
				 * @retval true if there is a delta chain.
				 * @retval false otherwise.
				 */
				[[nodiscard]] constexpr auto
					NeedWaitSMOs() const  //
					-> bool
				{
					return delta_type_ != kNotDelta && (rec_count_ >= kMaxDeltaRecordNum || node_size_ > kPageSize);
				}

				/**
				 * @return the modification type of this delta record.
				 */
				[[nodiscard]] constexpr auto
					GetDeltaType() const  //
					-> DeltaType
				{
					return static_cast<DeltaType>(delta_type_);
				}

				/**
				 * @return the byte length of this node.
				 */
				[[nodiscard]] constexpr auto
					GetNodeSize() const  //
					-> size_t
				{
					return node_size_;
				}

				/**
				 * @return the number of delta records in this chain.
				 */
				[[nodiscard]] constexpr auto
					GetRecordCount() const  //
					-> size_t
				{
					return rec_count_;
				}

				/**
				 * @retval 1st: the data usage of this node.
				 * @retval 2nd: the number of delta records in this node.
				 */
				[[nodiscard]] constexpr auto
					GetNodeUsage() const  //
					-> std::pair<size_t, size_t>
				{
					size_t delta_num = 0;
					const auto* cur = this;
					for (; cur->delta_type_ != kNotDelta; cur = cur->GetNext()) {
						++delta_num;
					}

					return { cur->node_size_, delta_num };
				}

				/**
				 * @brief Get the next pointer of a delta record or a base node.
				 *
				 * @tparam T an expected class to be loaded.
				 * @return a pointer to the next object.
				 */
				template <class T = DeltaRecord*>
				[[nodiscard]] constexpr auto
					GetNext() const  //
					-> T
				{
					if constexpr (std::is_pointer<T>::value) {
						return reinterpret_cast<T>(const_cast<uintptr_t&>(next_));
					}
					else {
						return static_cast<T>(const_cast<uintptr_t&>(next_));
					}

					// return reinterpret_cast<T>(const_cast<uintptr_t&>(next_));
				}

				/**
				 * @return a highest key in a target record if exist.
				 */
				[[nodiscard]] constexpr auto
					GetHighKey() const  //
					-> std::optional<Key>
				{
					if (!has_high_key_) return std::nullopt;
					return high_key_;
				}

				/**
				 * @brief Update the delta-modification type of this record with a given one.
				 *
				 * @param type a modification type to be updated.
				 */
				void
					SetDeltaType(const DeltaType type)
				{
					delta_type_ = type;
				}

				/**
				 * @brief Set a given pointer as the next one.
				 *
				 * @param next a pointer to be set as the next one.
				 * @param diff the difference in node sizes.
				 */
				void
					SetNext(  //
						const DeltaRecord* next,
						const int64_t diff)
				{
					rec_count_ = (next->delta_type_ == kNotDelta) ? 1 : next->rec_count_ + 1;
					node_size_ = next->node_size_ + diff;
					next_ = reinterpret_cast<uintptr_t>(next);
				}

				/*####################################################################################
				 * Public getters/setters for records
				 *##################################################################################*/

				 /**
				  * @return the length of a key in this record.
				  */
				[[nodiscard]] constexpr auto
					GetKeyLength() const  //
					-> size_t
				{
					return kKeyLen;
				}

				/**
				 * @return a key in this record.
				 */
				[[nodiscard]] auto
					GetKey() const  //
					-> Key
				{
					return key_;
				}

				/**
				 * @tparam T a class of expected payloads.
				 * @return a payload in this record.
				 */
				template <class T = void*>
				[[nodiscard]] auto
					GetPayload() const  //
					-> T
				{
					T payload{};
					memcpy(&payload, reinterpret_cast<const T*>(payload_), sizeof(T));
					return payload;
				}

				/**
				 * @tparam T a class of expected payloads.
				 * @return a payload in this record.
				 */
				[[nodiscard]] auto
					GetPayloadAtomically() const  //
					-> uintptr_t
				{
					const auto* pay_addr = reinterpret_cast<std::atomic_uintptr_t*>(ShiftAddr(this, kPayOffset));
					while (true) {
						for (size_t i = 1; true; ++i) {
							const auto payload = pay_addr->load(std::memory_order_relaxed);
							if (payload != kNullPtr) return payload;
							if (i >= kRetryNum) break;
						}
						std::this_thread::sleep_for(kShortSleep);
					}
				}

				/**
				 * @brief Set a merged-left child node to complete deleting an index-entry.
				 *
				 * @param left_pid the page ID of a mereged-left child node.
				 */
				void
					SetSiblingPID(const PageID left_pid)
				{
					auto* payload = reinterpret_cast<std::atomic<PageID> *>(ShiftAddr(this, kPayOffset));
					payload->store(left_pid, std::memory_order_relaxed);
				}

				/*####################################################################################
				 * Public utilities
				 *##################################################################################*/

				 /**
				  * @brief Compute the maximum size of delta records with given template classes.
				  *
				  * @tparam Payload a class of payloads.
				  * @return the maximum size of delta records in bytes.
				  */
				template <class Payload>
				[[nodiscard]] static constexpr auto
					GetMaxDeltaSize()  //
					-> size_t
				{
					constexpr auto kPayLen = (sizeof(Payload) > kPtrLen) ? sizeof(Payload) : kPtrLen;
					return (kHeaderLen + kPayLen + kCacheAlign) & ~kCacheAlign;
				}

				/**
				 * @brief Insert this delta record to a given container.
				 *
				 * @param records a set of records to be inserted this delta record.
				 * @param[in,out] count The number of delta records.
				 */
				void
					AddByInsertionSortTo(  //
						std::array<Record, kMaxDeltaRecordNum>& records,
						size_t& count) const
				{
					// copy a current key
					Record cur{ key_, this };

					// search an inserting position
					size_t i = 0;
					for (; i < count && Comp{}(records[i].key, cur.key); ++i) {
						// skip lower keys
					}

					// shift upper records if needed
					if (i >= count) {  // push back a new record
						++count;
					}
					else if (Comp{}(cur.key, records[i].key)) {  // insert a new record
						memmove(&(records[i + 1]), &(records[i]), sizeof(Record) * (count - i));
						++count;
					}
					else {  // there is the latest record
						return;
					}
					records[i] = std::move(cur);
				}

			private:
				/*####################################################################################
				 * Internal constants
				 *##################################################################################*/

				 /// Header length in bytes.
				static constexpr size_t kHeaderLen{ sizeof(DeltaRecord_SizeTest<Key, Comp>) };

				/// the length of keys.
				static constexpr size_t kKeyLen = sizeof(Key);

				/// the length of child pointers.
				static constexpr size_t kPtrLen = sizeof(PageID);

				/// an offset value for atomic operations.
				static constexpr size_t kPayOffset = (kHeaderLen + kWordAlign) & ~kWordAlign;

				/*####################################################################################
				 * Internal getters/setters
				 *##################################################################################*/

				 /**
				  * @brief Set a target payload directly.
				  *
				  * @tparam T a class of expected payloads.
				  * @param payload a target payload to be set.
				  */
				template <class T>
				void
					SetPayload(const T& payload)
				{
					memcpy(payload_, &payload, sizeof(T));
				}
			};

		}  // namespace dbgroup::index::bw_tree::component::fixlen
		// fixed length node
		namespace dbgroup::index::bw_tree::component::fixlen
		{
			template <class Key, class Comp>
			class Node_SizeTest
			{
			public:

				/// a flag for indicating whether this node is a leaf or internal node.
				uint16_t is_inner_ : 1;

				/// a flag for indicating the types of delta records.
				uint16_t delta_type_ : 3;

				/// a flag for indicating whether this delta record has a lowest-key.
				uint16_t has_low_key_ : 1;

				/// a flag for indicating whether this delta record has a highest-key.
				uint16_t has_high_key_ : 1;

				/// a blank block for alignment.
				uint16_t : 0;

				/// the number of records in this node.
				uint16_t rec_count_{ 0 };

				/// the size of this node in bytes.
				uint32_t node_size_{ 0 };

				/// the pointer to a sibling node.
				uintptr_t next_{ kNullPtr };

				/// the lowest key of this node.
				Key low_key_{};

				/// the highest key of this node.
				Key high_key_{};

				/// an actual data block for records.
				Key keys_[0]{};
			};

			/**
			 * @brief A class for represent leaf/internal nodes in Bw-tree.
			 *
			 * @tparam Key a target key class.
			 * @tparam Comp a comparetor class for keys.
			 */
			template <class Key, class Comp>
			class Node
			{
			public:
				/*####################################################################################
				 * Type aliases
				 *##################################################################################*/

				using ScanKey = std::optional<std::tuple<const Key&, size_t, bool>>;
				using ConsolidateInfo = std::pair<const void*, const void*>;
				template <class Entry>
				using BulkIter = typename std::vector<Entry>::const_iterator;
				using NodeEntry = std::tuple<Key, PageID, size_t>;

			public:

				/// a flag for indicating whether this node is a leaf or internal node.
				uint16_t is_inner_ : 1;

				/// a flag for indicating the types of delta records.
				uint16_t delta_type_ : 3;

				/// a flag for indicating whether this delta record has a lowest-key.
				uint16_t has_low_key_ : 1;

				/// a flag for indicating whether this delta record has a highest-key.
				uint16_t has_high_key_ : 1;

				/// a blank block for alignment.
				uint16_t : 0;

				/// the number of records in this node.
				uint16_t rec_count_{ 0 };

				/// the size of this node in bytes.
				uint32_t node_size_{ kHeaderLen };

				/// the pointer to a sibling node.
				uintptr_t next_{ kNullPtr };

				/// the lowest key of this node.
				Key low_key_{};

				/// the highest key of this node.
				Key high_key_{};

				/// an actual data block for records.
				Key keys_[0]{}; // variable-length object, whose size is determined at runtime, with all other items being the header for it.

			public:

				/*####################################################################################
				 * Public classes
				 *##################################################################################*/

				 /**
				  * @brief A class to sort delta records.
				  *
				  */
				struct Record {
					Key key;
					const void* ptr;
				};

				/*####################################################################################
				 * Public constructors and assignment operators
				 *##################################################################################*/

				 /**
				  * @brief Construct an initial root node.
				  *
				  */
				constexpr explicit Node(const bool is_inner = false)
					: is_inner_{ static_cast<NodeType>(is_inner) },
					delta_type_{ kNotDelta },
					has_low_key_{ 0 },
					has_high_key_{ 0 }
				{
				}

				/**
				 * @brief Construct a new root node.
				 *
				 * @param split_d a split-delta record.
				 * @param left_pid the page ID of a split-left child.
				 */
				Node(  //
					const Node* split_d,
					const PageID left_pid)
					: is_inner_{ kInner },
					delta_type_{ kNotDelta },
					has_low_key_{ 0 },
					has_high_key_{ 0 },
					rec_count_{ 2 },
					node_size_{ kHeaderLen + 2 * (kKeyLen + kPtrLen) }
				{
					keys_[1] = split_d->low_key_;
					const auto offset = SetPayload(kPageSize, left_pid) - kPtrLen;
					memcpy(ShiftAddr(this, offset), split_d->keys_, kPtrLen);
				}

				Node(const Node&) = delete;
				Node(Node&&) = delete;

				Node& operator=(const Node&) = delete;
				Node& operator=(Node&&) = delete;

				/*####################################################################################
				 * Public destructors
				 *##################################################################################*/

				 /**
				  * @brief Destroy the node object.
				  *
				  */
				~Node() = default;

				/*####################################################################################
				 * Public getters/setters
				 *##################################################################################*/

				 /**
				  * @retval true if this is a leaf node.
				  * @retval false otherwise.
				  */
				[[nodiscard]] constexpr auto
					IsLeaf() const  //
					-> bool
				{
					return is_inner_ == kLeaf;
				}

				/**
				 * @retval true if this node is leftmost in its tree level.
				 * @retval false otherwise.
				 */
				[[nodiscard]] constexpr auto
					IsLeftmost() const  //
					-> bool
				{
					return has_low_key_ == 0;
				}

				/**
				 * @return the byte length of this node.
				 */
				[[nodiscard]] constexpr auto
					GetNodeSize() const  //
					-> size_t
				{
					return node_size_;
				}

				/**
				 * @return the byte length to be modified by SMOs.
				 */
				[[nodiscard]] constexpr auto
					GetNodeDiff() const  //
					-> size_t
				{
					return node_size_ - kHeaderLen;
				}

				/**
				 * @return the number of records in this node.
				 */
				[[nodiscard]] constexpr auto
					GetRecordCount() const  //
					-> size_t
				{
					return rec_count_;
				}

				/**
				 * @brief Get the next pointer of a delta record, a base node, or a logical ID.
				 *
				 * Note that this funcion returns a logical ID if this is a base node.
				 *
				 * @tparam T a expected class to be loaded.
				 * @return a pointer to the next object.
				 */
				template <class T = const Node*>
				[[nodiscard]] constexpr auto
					GetNext() const  //
					-> T
				{
					if constexpr (std::is_pointer<T>::value) {
						return reinterpret_cast<T>(const_cast<uintptr_t&>(next_));
					}
					else {
						return static_cast<T>(const_cast<uintptr_t&>(next_));
					}

					// return reinterpret_cast<T>(next_);
				}

				/**
				 * @return The length of a lowest key.
				 */
				[[nodiscard]] constexpr auto
					GetLowKeyLen() const  //
					-> size_t
				{
					return sizeof(Key) * has_low_key_;
				}

				/**
				 * @brief Get the lowest key in this node.
				 *
				 * If this node is the leftmost node in its level, this returns std::nullopt.
				 *
				 * @return the lowest key if exist.
				 */
				[[nodiscard]] auto
					GetLowKey() const  //
					-> Key
				{
					return low_key_;
				}

				/**
				 * @brief Copy and return a highest key for scanning.
				 *
				 * NOTE: this function does not check the existence of a highest key.
				 *
				 * @return the highest key in this node.
				 */
				[[nodiscard]] auto
					GetHighKey() const  //
					-> Key
				{
					return high_key_;
				}

				/**
				 * @brief Set the size of this node for scanning.
				 *
				 */
				void
					SetNodeSizeForScan()
				{
					node_size_ = 2 * kPageSize;
				}

				/**
				 * @brief Set a sibling node.
				 *
				 * @param pid the page ID of the sibling node.
				 */
				void
					SetNext(const PageID pid)
				{
					next_ = pid;
				}

				/*####################################################################################
				 * Public getters/setters for records
				 *##################################################################################*/

				 /**
				  * @param pos the position of a target record.
				  * @return a key in a target record.
				  */
				[[nodiscard]] auto
					GetKey(const size_t pos) const  //
					-> const Key&
				{
					return keys_[pos];
				}

				/**
				 * @tparam T a class of a target payload.
				 * @param pos the position of a target record.
				 * @return a payload in a target record.
				 */
				template <class T>
				[[nodiscard]] auto
					GetPayload(const size_t pos) const  //
					-> T
				{
					T payload{};
					memcpy(&payload, GetPayloadAddr<T>(pos), sizeof(T));
					return payload;
				}

				/**
				 * @tparam T a class of a target payload.
				 * @param pos the position of a target record.
				 * @retval 1st: a key in a target record.
				 * @retval 2nd: a payload in a target record.
				 */
				template <class T>
				[[nodiscard]] auto
					GetRecord(const size_t pos) const  //
					-> std::pair<Key, T>
				{
					return { keys_[pos], GetPayload<T>(pos) };
				}

				///**
				// * @brief Get the rightmost child node.
				// *
				// * If this object is actually a delta record, this function traverses a delta-chain
				// * and returns the right most child from a base node.
				// *
				// * @return the page ID of the rightmost child node.
				// */
				//[[nodiscard]] auto
				//	GetRightmostChild() const  //
				//	-> PageID
				//{
				//	// To-Do
				//}

				/**
				 * @brief Get the leftmost child node.
				 *
				 * If this object is actually a delta record, this function traverses a delta-chain
				 * and returns the left most child from a base node.
				 *
				 * @return the page ID of the leftmost child node.
				 */
				[[nodiscard]] auto
					GetLeftmostChild() const  //
					-> PageID
				{
					const Node* cur = this;
					for (; cur->delta_type_ != kNotDelta; cur = cur->template GetNext<const Node*>()) {
						// go to the next delta record or base node
					}

					// get a leftmost node
					return cur->template GetPayload<PageID>(0);
				}

				/*####################################################################################
				 * Public utilities
				 *##################################################################################*/

				 /**
				  * @brief Get the position of a specified key by using binary search.
				  *
				  * If there is no specified key in this node, this returns the minimum position that
				  * is greater than the specified key.
				  *
				  * NOTE: This function assumes that the given key must be in the range of this node.
				  * If the given key is greater than the highest key of this node, this function will
				  * returns incorrect results.
				  *
				  * @param key a target key.
				  * @return the pair of record's existence and the searched position.
				  */
				[[nodiscard]] auto
					SearchRecord(const Key& key) const  //
					-> std::pair<DeltaRC, size_t>
				{
					int64_t begin_pos = is_inner_ & ~static_cast<size_t>(has_low_key_);
					int64_t end_pos = rec_count_ - 1;
					while (begin_pos <= end_pos) {
						const size_t pos = (begin_pos + end_pos) >> 1UL;  // NOLINT
						const auto& index_key = keys_[pos];

						if (Comp{}(key, index_key)) {  // a target key is in a left side
							end_pos = pos - 1;
						}
						else if (Comp{}(index_key, key)) {  // a target key is in a right side
							begin_pos = pos + 1;
						}
						else {  // find an equivalent key
							return { kRecordFound, pos };
						}
					}

					return { kRecordNotFound, begin_pos };
				}

				/**
				 * @brief Get the corresponding child node with a specified key.
				 *
				 * If there is no specified key in this node, this returns the child in the minimum
				 * position that is greater than the specified key.
				 *
				 * @param key a target key.
				 * @param closed a flag for including the same key.
				 * @return the page ID of searched child node.
				 */
				[[nodiscard]] auto
					SearchChild(  //
						const Key& key,
						const bool closed) const  //
					-> PageID
				{
					int64_t begin_pos = 1;
					int64_t end_pos = rec_count_ - 1;
					while (begin_pos <= end_pos) {
						const size_t pos = (begin_pos + end_pos) >> 1UL;  // NOLINT
						const auto& index_key = keys_[pos];

						if (Comp{}(key, index_key)) {  // a target key is in a left side
							end_pos = pos - 1;
						}
						else if (Comp{}(index_key, key)) {  // a target key is in a right side
							begin_pos = pos + 1;
						}
						else {  // find an equivalent key
							begin_pos = pos + static_cast<size_t>(closed);
							break;
						}
					}

					return GetPayload<PageID>(begin_pos - 1);
				}

				/**
				 * @brief Get the end position of records for scanning and check it has been finished.
				 *
				 * @param end_key a pair of a target key and its closed/open-interval flag.
				 * @retval 1st: true if this node is end of scanning.
				 * @retval 2nd: the end position for scanning.
				 */
				[[nodiscard]] auto
					SearchEndPositionFor(const ScanKey& end_key) const  //
					-> std::pair<bool, size_t>
				{
					const auto is_end = IsRightmostOf(end_key);
					size_t end_pos{};
					if (is_end && end_key) {
						const auto& [e_key, e_key_len, e_closed] = *end_key;
						const auto [rc, pos] = SearchRecord(e_key);
						end_pos = (rc == kRecordFound && e_closed) ? pos + 1 : pos;
					}
					else {
						end_pos = rec_count_;
					}

					return { is_end, end_pos };
				}

				/*####################################################################################
				 * Public utilities for consolidation
				 *##################################################################################*/

				 /**
				  * @brief Copy a lowest key for consolidation or set an initial used page size for
				  * splitting.
				  *
				  * @param node_addr an original node that has a lowest key.
				  * @return an initial offset.
				  */
				auto
					CopyLowKeyFrom(const void* node_addr)  //
					-> size_t
				{
					// prepare a node that has the lowest key
					if (node_addr == this) {
						// this node is a split-right node, and so the leftmost record has the lowest key
						has_low_key_ = 1;
						low_key_ = keys_[0];
					}
					else {
						// this node is a consolidated node, and so the given node has the lowest key
						const auto* node = reinterpret_cast<const Node*>(node_addr);
						has_low_key_ = node->has_low_key_;
						low_key_ = node->low_key_;
					}

					return kPageSize;  // dummy value
				}

				/**
				 * @brief Copy a highest key from a given consolidated node.
				 *
				 * @param node_addr an original node that has a highest key.
				 * @param offset an offset to the bottom of free space.
				 * @param is_split_left a flag for indicating this node is a split-left one.
				 */
				void
					CopyHighKeyFrom(  //
						const void* node_addr,
						[[maybe_unused]] size_t offset,
						const bool is_split_left = false)
				{
					// prepare a node that has the highest key, and copy the next logical ID
					const auto* node = reinterpret_cast<const Node*>(node_addr);
					if (is_split_left) {
						has_high_key_ = 1;
						high_key_ = node->keys_[0];
					}
					else {
						has_high_key_ = node->has_high_key_;
						high_key_ = node->high_key_;
						next_ = node->next_;
					}
				}

				/**
				 * @brief Copy a record from a base node in the leaf level.
				 *
				 * @param node a target base node.
				 * @param orig_node an original base node.
				 * @param pos the position of a target record.
				 * @param offset an offset to the bottom of free space.
				 * @param r_node a split-right node for switching.
				 * @return an offset to the copied record.
				 */
				template <class T>
				static auto
					CopyRecordFrom(  //
						Node*& node,
						const Node* orig_node,
						const size_t pos,
						size_t offset,
						Node*& r_node)  //
					-> size_t
				{
					constexpr auto kRecLen = sizeof(Key) + sizeof(T);

					// copy a record from the given node
					node->keys_[node->rec_count_++] = orig_node->keys_[pos];
					offset -= sizeof(T);
					memcpy(ShiftAddr(node, offset), orig_node->template GetPayloadAddr<T>(pos), sizeof(T));
					node->node_size_ += kRecLen;

					if (r_node != nullptr && node->node_size_ > (kPageSize - kHeaderLen) / 2) {
						// switch to the split-right node
						node = r_node;
						r_node = nullptr;
						offset = kPageSize;
					}

					return offset;
				}

				/**
				 * @brief Copy a record from a delta record in the leaf level.
				 *
				 * @param node a target base node.
				 * @param rec_ptr an original delta record.
				 * @param offset an offset to the bottom of free space.
				 * @param r_node a split-right node for switching.
				 * @return an offset to the copied record.
				 */
				template <class T>
				static auto
					CopyRecordFrom(  //
						Node*& node,
						const void* rec_ptr,
						size_t offset,
						Node*& r_node)  //
					-> size_t
				{
					constexpr auto kRecLen = sizeof(Key) + sizeof(T);

					const auto* rec = reinterpret_cast<const Node*>(rec_ptr);
					if (rec->delta_type_ != kDelete) {
						// copy a record from the given node
						node->keys_[node->rec_count_++] = rec->low_key_;
						offset -= sizeof(T);
						memcpy(ShiftAddr(node, offset), rec->keys_, sizeof(T));
						node->node_size_ += kRecLen;

						if (r_node != nullptr && node->node_size_ > (kPageSize - kHeaderLen) / 2) {
							// switch to the split-right node
							node = r_node;
							r_node = nullptr;
							offset = kPageSize;
						}
					}

					return offset;
				}

				/*####################################################################################
				 * Public bulkload API
				 *##################################################################################*/

				 /**
				  * @brief Create a node with the maximum number of records for bulkloading.
				  *
				  * @tparam Entry a container of a key/payload pair.
				  * @param iter the begin position of target records.
				  * @param iter_end the end position of target records.
				  * @param prev_node a left sibling node.
				  * @param this_pid the logical ID of a this node.
				  * @param nodes the container of construcred nodes.
				  * @param is_inner a flag for indicating inner nodes.
				  */
				template <class Entry>
				void
					Bulkload(  //
						BulkIter<Entry>& iter,
						const BulkIter<Entry>& iter_end,
						Node* prev_node,
						PageID this_pid,
						std::vector<NodeEntry>& nodes,
						[[maybe_unused]] const bool is_inner)
				{
					using Payload = std::tuple_element_t<1, Entry>;

					constexpr auto kRecLen = kKeyLen + sizeof(Payload);

					// extract and insert entries into this node
					auto offset = kPageSize;
					for (; iter < iter_end; ++iter) {
						// check whether the node has sufficient space
						if (node_size_ + kRecLen > kNodeCapacityForBulkLoading) break;
						node_size_ += kRecLen;

						// insert an entry into this node
						const auto& [key, payload, key_len, pay_len] = ParseEntry(*iter);
						offset = SetPayload(offset, payload);
						keys_[rec_count_++] = key;
					}

					// set a lowest key
					has_low_key_ = 1;
					low_key_ = keys_[0];

					// link the sibling nodes if exist
					if (prev_node != nullptr) {
						prev_node->LinkNext(this_pid, this);
					}

					nodes.emplace_back(low_key_, this_pid, kKeyLen);
				}

				/**
				 * @brief Link border nodes between partial trees.
				 *
				 * @tparam MappingTable a class for representing mapping tables.
				 * @param left_pid the page ID of a highest border node in a left tree.
				 * @param right_pid the page ID of a highest border node in a right tree.
				 * @param m_table a mapping table to resolve page IDs.
				 */
				template <class MappingTable>
				static void
					LinkVerticalBorderNodes(  //
						PageID left_pid,
						PageID right_pid,
						const MappingTable& m_table)
				{
					if (left_pid == kNullPtr) return;

					while (true) {
						const auto* left_lptr = m_table.GetLogicalPtr(left_pid);
						auto* left_node = left_lptr->template Load<Node*>();
						const auto* right_lptr = m_table.GetLogicalPtr(right_pid);
						auto* right_node = right_lptr->template Load<Node*>();

						left_node->LinkNext(right_pid, right_node);
						if (left_node->is_inner_ == kLeaf) return;  // all the border nodes are linked

						// go down to the lower level
						right_pid = right_node->template GetPayload<PageID>(0);
						left_pid = left_node->template GetPayload<PageID>(left_node->rec_count_ - 1);
					}
				}

				/**
				 * @brief Remove the leftmost keys from the leftmost nodes.
				 *
				 * @tparam MappingTable a class for representing mapping tables.
				 * @param pid the logical ID of a root node.
				 * @param m_table a mapping table to resolve page IDs.
				 */
				template <class MappingTable>
				static void
					RemoveLeftmostKeys(  //
						PageID pid,
						const MappingTable& m_table)
				{
					while (true) {
						// remove the lowest key
						const auto* lptr = m_table.GetLogicalPtr(pid);
						auto* node = lptr->template Load<Node*>();
						node->has_low_key_ = 0;
						if (node->is_inner_ == kLeaf) return;

						// go down to the lower level
						pid = node->template GetPayload<PageID>(0);
					}
				}

			private:
				/*####################################################################################
				 * Internal constants
				 *##################################################################################*/

				 /// Header length in bytes.
				static constexpr size_t kHeaderLen = sizeof(Node_SizeTest<Key, Comp>);

				/// the length of keys.
				static constexpr size_t kKeyLen = sizeof(Key);

				/// the length of child pointers.
				static constexpr size_t kPtrLen = sizeof(PageID);

				/*####################################################################################
				 * Internal getters/setters
				 *##################################################################################*/

				 /**
				  * @param end_key a pair of a target key and its closed/open-interval flag.
				  * @retval true if this node is a rightmost node for the given key.
				  * @retval false otherwise.
				  */
				[[nodiscard]] auto
					IsRightmostOf(const ScanKey& end_key) const  //
					-> bool
				{
					if (!has_high_key_) return true;  // the rightmost node
					if (!end_key) return false;       // perform full scan

					const auto& [end_k, dummy, closed] = *end_key;
					return Comp{}(end_k, high_key_) || (!closed && !Comp{}(high_key_, end_k));
				}

				/**
				 * @param pos the position of a target record.
				 * @return an address of a target payload.
				 */
				template <class T>
				[[nodiscard]] constexpr auto
					GetPayloadAddr(const size_t pos) const  //
					-> void*
				{
					constexpr size_t kPageAlign = kPageSize - 1;
					const auto page_size = (node_size_ + kPageAlign) & ~kPageAlign;
					return ShiftAddr(this, page_size - sizeof(T) * (pos + 1));
				}

				/**
				 * @brief Set a target payload directly.
				 *
				 * @tparam T a class of payloads.
				 * @param offset an offset to the bottom of free space.
				 * @param payload a target payload to be set.
				 * @return an offset to the set payload.
				 */
				template <class T>
				auto
					SetPayload(  //
						size_t offset,
						const T& payload)  //
					-> size_t
				{
					offset -= sizeof(T);
					memcpy(ShiftAddr(this, offset), &payload, sizeof(T));
					return offset;
				}

				/*####################################################################################
				 * Internal utilities
				 *##################################################################################*/

				 /**
				  * @brief Link this node to a right sibling node.
				  *
				  * @param right_pid the page ID of a right sibling node.
				  * @param right_node the right sibling node.
				  */
				void
					LinkNext(  //
						const PageID right_pid,
						Node* right_node)
				{
					// set a sibling link
					next_ = right_pid;

					// copy the lowest key in the right node as a highest key in this node
					has_high_key_ = 1;
					high_key_ = right_node->low_key_;
				}

				/*####################################################################################
				 * Internal variables
				 *##################################################################################*/

			};

		}  // namespace dbgroup::index::bw_tree::component::fixlen
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
					FindSmallestLargerEqual(const Key& key)
					-> RecordIterator_t
				{
					ScanKey begin_key(std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true })); // true = may include the value if found

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

						if (record && record.GetKey() >= key) {
							// success. Accounts for ~ 95% - 99% of cases. 
							return record;
						}
						else {
							// We are at the end of a node, and the solution is either on the next node, or the solution does not exist. 
							if (record.node_->has_high_key_) {
								// go to the next sibling/node and continue scanning
								const auto& next_key = record.node_->GetHighKey();
								const auto sib_pid = record.node_->template GetNext<PageID>();
								record = this->SiblingScan(sib_pid, record.node_, next_key, std::nullopt);

								RecordIterator_t record2{ record };
								while (record && record.GetKey() < key) {
									record++;
									if (record) {
										record2++;
									}
								}
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

					return RecordIterator_t{ this, node, begin_pos, begin_pos + 1, std::nullopt, true };
				};

				/**
				 * @brief Try to find the node the largest key that is equal to or less than the requested key. (e.g. search for 55.5, returns 55)
				 *
				 * @param key a key to search for.
				 * @return an iterator to access the scanned record.
				 */
				auto
					FindLargestSmallerEqual(const Key& key)
					-> RecordIterator_t
				{
					ScanKey keyFind{ std::tuple<const Key&, size_t, bool>({ key, (size_t)(sizeof(Key)), true }) };

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

					return RecordIterator_t{ node, begin_pos, end_pos, is_end };
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
		}  // namespace dbgroup::index::bw_tree

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
					temp.Word(index) = MwCASDescriptor<NumWords>::template Read<uint64_t>(&Word(index));
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
					MwCASDescriptor<NumWords> desc{};

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
					MwCASDescriptor<NumWords> desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor<NumWords>::template Read<uint64_t>(&Word(index));
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
					MwCASDescriptor<NumWords> desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor<NumWords>::template Read<uint64_t>(&Word(index));
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
					MwCASDescriptor<NumWords> desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor<NumWords>::template Read<uint64_t>(&Word(index));
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
					MwCASDescriptor<NumWords> desc{};

					// capture the old words
					for (index = 0; index < NumWords; index++) {
						OldCopy.Word(index) = UpdateCopy.Word(index) = MwCASDescriptor<NumWords>::template Read<uint64_t>(&Word(index));
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
#define SWITCH_FOR_0_to_16 switch (Index) { \
			REPEATFOR(0); REPEATFOR(1); REPEATFOR(2); REPEATFOR(3); \
			REPEATFOR(4); REPEATFOR(5); REPEATFOR(6); REPEATFOR(7); \
			REPEATFOR(8); REPEATFOR(9); REPEATFOR(10); REPEATFOR(11); \
			REPEATFOR(12); REPEATFOR(13); \
		    default: break; }

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
			template <int index> NthTypeOf<index> GetCurrentValue(fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor< num_parameters >& desc) const {
				if (container.get<index>()) {
					return desc.Read<NthTypeOf<index>>(container.get<index>());
				}
				else {
					return 0;
				}				
			};
			void CaptureOldValues(Union< Args... >& OldCopy, size_t& Index, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor< num_parameters >& desc) const {
#define REPEATFOR(index) case index: if constexpr (num_parameters > index) OldCopy.get<index>() = GetCurrentValue<index>(desc); break
				for (Index = 0; Index < num_parameters; Index++) {
					SWITCH_FOR_0_to_16;
				}
#undef REPEATFOR
			};

			template <typename... IncomingArgs>
			void CaptureOldValuesAndModify(Union< Args... >& OldCopy, Union< Args... >& NewValues, Union< IncomingArgs... >& Functors, size_t& Index, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor< num_parameters >& desc) const {
#define REPEATFOR(index) case index: if constexpr (num_parameters > index) NewValues.get<index>() = Functors.get<index>()(OldCopy.get<index>() = GetCurrentValue<index>(desc)); break
				for (Index = 0; Index < num_parameters; Index++) {
					SWITCH_FOR_0_to_16;
				}
#undef REPEATFOR
			};

			template <int index> decltype(auto) PrepareSwapTarget(Union< Args... >& OldCopy, Union< Args... >& NewValues, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor< num_parameters >& desc) const {
				if (container.get<index>()) {
					desc.AddMwCASTarget(container.get<index>(), OldCopy.get<index>(), NewValues.get<index>());
				}
			};
			void PrepareSwapTargets(Union< Args... >& OldCopy, Union< Args... >& NewValues, size_t& Index, fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor< num_parameters >& desc) {
#define REPEATFOR(index) case index: if constexpr (num_parameters > index) PrepareSwapTarget<index>(OldCopy, NewValues, desc); break			
				for (Index = 0; Index < num_parameters; Index++) {
					SWITCH_FOR_0_to_16;
				}
#undef REPEATFOR
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
					MwCASDescriptor< num_parameters > desc{};

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
					MwCASDescriptor< num_parameters > desc{};

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

			template <typename... IncomingArgs>
			bool TrySwap(Union< Args... >& OldCopy, IncomingArgs&&... newValues) {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;

				Union< Args... > UpdateCopy(std::forward<IncomingArgs>(newValues)...);

				size_t
					index;

				// continue until a MwCAS operation succeeds
				while (true) {
					// create a MwCAS descriptor
					MwCASDescriptor< num_parameters > desc{};

					// prepare the swap target(s)
					PrepareSwapTargets(OldCopy, UpdateCopy, index, desc);

					// try multi-word compare and swap
					if (desc.MwCAS()) return true;
					else return false;
				}

				return false;
			}; // returns the previous value while changing the underlying value

			template <int index>
			decltype(auto) Read() const {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;
				MwCASDescriptor< num_parameters > desc{};
				return GetCurrentValue<index>(desc);
			};

			Union< Args... > ReadAll() const {
				using fibers::utilities::dbgroup::atomic::mwcas::MwCASDescriptor;
				MwCASDescriptor< num_parameters > desc{};

				Union< Args... > OldCopy;
				size_t index;
				CaptureOldValues(OldCopy, index, desc);

				return OldCopy;
			};


		private:
			Union< Args*... > container;
#undef SWITCH_FOR_0_to_16
		};


#undef MWCAS_CAPACITY
#undef MWCAS_RETRY_THRESHOLD
#undef MWCAS_SLEEP_TIME
#undef BZTREE_PAGE_SIZE
#undef BZTREE_MAX_DELTA_RECORD_NUM
#undef BZTREE_MAX_DELETED_SPACE_SIZE
#undef BZTREE_MIN_FREE_SPACE_SIZE
#undef BZTREE_MIN_NODE_SIZE
#undef BZTREE_MAX_MERGED_SIZE
#undef BZTREE_MAX_VARIABLE_DATA_SIZE
#undef DBGROUP_MAX_THREAD_NUM
#undef CPP_UTILITY_SPINLOCK_RETRY_NUM
#undef CPP_UTILITY_BACKOFF_TIME
#undef BW_TREE_PAGE_SIZE
#undef BW_TREE_DELTA_RECORD_NUM_THRESHOLD
#undef BW_TREE_MAX_DELTA_RECORD_NUM
#undef BW_TREE_MIN_NODE_SIZE
#undef BW_TREE_MAX_VARIABLE_DATA_SIZE
#undef BW_TREE_RETRY_THRESHOLD
#undef BW_TREE_SLEEP_TIME

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