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
		__forceinline static void* Mem_ClearedAlloc(const size_t& size) { void* mem = Mem_Alloc16(size); ::memset(mem, 0, size); return mem; };
		__forceinline static void  Mem_Free(void* ptr) { Mem_Free16(ptr); }
		__forceinline static void* Mem_Alloc(const size_t size) { return Mem_ClearedAlloc(size); }
		__forceinline static char* Mem_CopyString(const char* in) { size_t L{ strlen(in) + 1 }; char* out = (char*)Mem_Alloc(L); ::strncpy(out, in, L - 1);  return out; };

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
			class CpuInfo_t {
			public:
				CpuInfo_t() : processorPackageCount(0), processorCoreCount(0), logicalProcessorCount(0), numaNodeCount(0), cacheLevel() {};
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
	};
};

namespace fibers {
	namespace utilities {

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
		private:
			static constexpr inline unsigned long long constexpr_pow(unsigned long long x, unsigned long long y) {
				return y == 0 ? 1.0 : x * constexpr_pow(x, y - 1);
			};

		public:
			static constexpr size_t NumWords{ 1 + sizeof(Arg) / 8 };
			static constexpr Arg MaxV{ constexpr_pow(2, (DBL_MANT_DIG - 6.0) / 1.7) };

		protected:
			union ContainerImpl {
				Arg data;
				uint64_t addresses[NumWords];
			};
			ContainerImpl data;
			CAS_Container<Arg> Copy() const {
				using namespace dbgroup::atomic::mwcas;
				CAS_Container<Arg> temp;
				MwCASDescriptor<NumWords> desc{};

				for (size_t index = 0; index < NumWords; index++) {
					temp.Word(index) = desc.Read< uint64_t>(&Word(index));
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

			static constexpr Arg Bound(Arg const& a) { return std::max<Arg>(-MaxV, std::min<Arg>(MaxV, a)); };
			static constexpr Arg MakeUnsignedAndBound(Arg const& a) { return Bound(a) + MaxV; };
			static constexpr Arg MakeSigned(Arg const& a) { return a - MaxV; };

		public:
			
			constexpr CAS_Container() : data{ static_cast<Arg>(0) } { static_assert(std::is_pod_v<Arg>, "Compare-and-swap operations only work with POD-type structs."); };
			constexpr CAS_Container(Arg const& a) : data{ MakeUnsignedAndBound(a) } { static_assert(std::is_pod_v<Arg>, "Compare-and-swap operations only work with POD-type structs."); };

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
					UpdateCopy.Data() = MakeUnsignedAndBound(input);

					// prepare the swap target(s)
					for (index = 0; index < NumWords; index++) {
						desc.AddMwCASTarget(&Word(index), OldCopy.Word(index), UpdateCopy.Word(index));
					}

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
					else if (allowMiss) break;
				}
				return OldCopy.load();
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
					UpdateCopy.Data() = MakeUnsignedAndBound(UpdateCopy.load() + input);

					// prepare the swap target(s)
					for (index = 0; index < NumWords; index++) {
						desc.AddMwCASTarget(&Word(index), OldCopy.Word(index), UpdateCopy.Word(index));
					}

					// try multi-word compare and swap
					if (desc.MwCAS()) break;
				}
				return OldCopy.load();
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
				const CAS_Container<Arg> x = Copy();
				return CAS_Container::MakeSigned(x.Data());
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
			MultiItemCAS(MultiItemCAS&&) = delete;
			MultiItemCAS& operator=(MultiItemCAS const&) = delete;
			MultiItemCAS& operator=(MultiItemCAS&&) = delete;
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
	};

};