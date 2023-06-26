
#ifndef __UNORDERED_LIST_H__
#define __UNORDERED_LIST_H__

#pragma hdrstop

#pragma region "OPTIONS"
/*!
OPTION:		Use custom memory allocator instead of new/delete for creation of _type_ on heap.
WHY:		Generally improved performance.
DEFAULT:	ON.
*/
#define useAllocatedMemory		

/*!
OPTION:		Use one of the cweeMutex types instead of the Safe_Ptr methodology.
WHY:		Improved performance and stability under most situations.
DEFAULT:	ON.
*/
#define useCweeMutex

/*!
OPTION:		Use the TSL Robin Hood unordered map instead of the STL unordered map.
WHY:		Researching benefits. Higher performance is expected.
DEFAULT:	OFF.
*/
#define useRobinMap

/*!
OPTION:		When using a cweeMutex, select to use the atomic-int style mutex instead of the critical section style mutex.
WHY:		Avoids explicit mutexes, but will result in topping-off the CPU, but may be a more "honest" representation of CPU load?
DEFAULT:	OFF.
*/
//#define useAtomicMutex

/*!
OPTION:		Use an atomic integer instead of an STL integer for the key counter.
WHY:		Less race conditions. However there does not seem to be a benefit at this time.
DEFAULT:	OFF.
*/
//#define useAtomicInt

/*!
OPTION:		Use granularity to reserve assets ahead of their actual use.
WHY:		Improved performance when allocating large number of assets in series.
DEFAULT:	ON.
*/
// #define useGranularity GRANULARITY_SCALER

/*!
OPTION:		Use a map of atomic integers to keep track of protected items
WHY:		Potentially faster to look-up whether it is safe to erase an object.
DEFAULT:	ON.
*/
#define useProtectedMap

/*!
OPTION:		Use the Robin Map instead of unordered list for other containers. 
WHY:		Potentially faster.
DEFAULT:	ON.
*/
#define useRobinHash

#pragma endregion

#pragma region "INCLUDES BASED ON SELECTED OPTIONS"

#ifdef useRobinMap
#include "robin_map.h"
//#pragma message("cweeUnorderedMap is compiling with Robin Map.")
#elif
//#pragma message("cweeUnorderedMap is compiling with std::UnorderedMap.")
#endif
#ifndef useCweeMutex
#include "safe_ptr.h"
//#pragma message("cweeUnorderedMap is compiling with SafePtr.")
#else
//#pragma message("cweeUnorderedMap is compiling with sysCweeMutex.")
#endif
#ifdef useRobinHash

#pragma region robinHash

//                 ______  _____                 ______                _________
//  ______________ ___  /_ ___(_)_______         ___  /_ ______ ______ ______  /
//  __  ___/_  __ \__  __ \__  / __  __ \        __  __ \_  __ \_  __ \_  __  /
//  _  /    / /_/ /_  /_/ /_  /  _  / / /        _  / / // /_/ // /_/ // /_/ /
//  /_/     \____/ /_.___/ /_/   /_/ /_/ ________/_/ /_/ \____/ \____/ \__,_/
//                                      _/_____/
//
// Fast & memory efficient hashtable based on robin hood hashing for C++11/14/17/20
// https://github.com/martinus/robin-hood-hashing
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2021 Martin Ankerl <http://martin.ankerl.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef ROBIN_HOOD_H_INCLUDED
#define ROBIN_HOOD_H_INCLUDED

// see https://semver.org/
#define ROBIN_HOOD_VERSION_MAJOR 3  // for incompatible API changes
#define ROBIN_HOOD_VERSION_MINOR 11 // for adding functionality in a backwards-compatible manner
#define ROBIN_HOOD_VERSION_PATCH 5  // for backwards-compatible bug fixes

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <memory> // only to support hash of smart pointers
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#if __cplusplus >= 201703L
#    include <string_view>
#endif

// #define ROBIN_HOOD_LOG_ENABLED
#ifdef ROBIN_HOOD_LOG_ENABLED
#    include <iostream>
#    define ROBIN_HOOD_LOG(...) \
        std::cout << __FUNCTION__ << "@" << __LINE__ << ": " << __VA_ARGS__ << std::endl;
#else
#    define ROBIN_HOOD_LOG(x)
#endif

// #define ROBIN_HOOD_TRACE_ENABLED
#ifdef ROBIN_HOOD_TRACE_ENABLED
#    include <iostream>
#    define ROBIN_HOOD_TRACE(...) \
        std::cout << __FUNCTION__ << "@" << __LINE__ << ": " << __VA_ARGS__ << std::endl;
#else
#    define ROBIN_HOOD_TRACE(x)
#endif

// #define ROBIN_HOOD_COUNT_ENABLED
#ifdef ROBIN_HOOD_COUNT_ENABLED
#    include <iostream>
#    define ROBIN_HOOD_COUNT(x) ++counts().x;
namespace robin_hood {
	struct Counts {
		uint64_t shiftUp{};
		uint64_t shiftDown{};
	};
	inline std::ostream& operator<<(std::ostream& os, Counts const& c) {
		return os << c.shiftUp << " shiftUp" << std::endl << c.shiftDown << " shiftDown" << std::endl;
	}

	static Counts& counts() {
		static Counts counts{};
		return counts;
	}
} // namespace robin_hood
#else
#    define ROBIN_HOOD_COUNT(x)
#endif

// all non-argument macros should use this facility. See
// https://www.fluentcpp.com/2019/05/28/better-macros-better-flags/
#define ROBIN_HOOD(x) ROBIN_HOOD_PRIVATE_DEFINITION_##x()

// mark unused members with this macro
#define ROBIN_HOOD_UNUSED(identifier)

// bitness
#if SIZE_MAX == UINT32_MAX
#    define ROBIN_HOOD_PRIVATE_DEFINITION_BITNESS() 32
#elif SIZE_MAX == UINT64_MAX
#    define ROBIN_HOOD_PRIVATE_DEFINITION_BITNESS() 64
#else
#    error Unsupported bitness
#endif

// endianess
#ifdef _MSC_VER
#    define ROBIN_HOOD_PRIVATE_DEFINITION_LITTLE_ENDIAN() 1
#    define ROBIN_HOOD_PRIVATE_DEFINITION_BIG_ENDIAN() 0
#else
#    define ROBIN_HOOD_PRIVATE_DEFINITION_LITTLE_ENDIAN() \
        (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#    define ROBIN_HOOD_PRIVATE_DEFINITION_BIG_ENDIAN() (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#endif

// inline
#ifdef _MSC_VER
#    define ROBIN_HOOD_PRIVATE_DEFINITION_NOINLINE() __declspec(noinline)
#else
#    define ROBIN_HOOD_PRIVATE_DEFINITION_NOINLINE() __attribute__((noinline))
#endif

// exceptions
#if !defined(__cpp_exceptions) && !defined(__EXCEPTIONS) && !defined(_CPPUNWIND)
#    define ROBIN_HOOD_PRIVATE_DEFINITION_HAS_EXCEPTIONS() 0
#else
#    define ROBIN_HOOD_PRIVATE_DEFINITION_HAS_EXCEPTIONS() 1
#endif

// count leading/trailing bits
#if !defined(ROBIN_HOOD_DISABLE_INTRINSICS)
#    ifdef _MSC_VER
#        if ROBIN_HOOD(BITNESS) == 32
#            define ROBIN_HOOD_PRIVATE_DEFINITION_BITSCANFORWARD() _BitScanForward
#        else
#            define ROBIN_HOOD_PRIVATE_DEFINITION_BITSCANFORWARD() _BitScanForward64
#        endif
#        include <intrin.h>
#        pragma intrinsic(ROBIN_HOOD(BITSCANFORWARD))
#        define ROBIN_HOOD_COUNT_TRAILING_ZEROES(x)                                       \
            [](size_t mask) noexcept -> int {                                             \
                unsigned long index;                                                      \
                return ROBIN_HOOD(BITSCANFORWARD)(&index, mask) ? static_cast<int>(index) \
                                                                : ROBIN_HOOD(BITNESS);    \
            }(x)
#    else
#        if ROBIN_HOOD(BITNESS) == 32
#            define ROBIN_HOOD_PRIVATE_DEFINITION_CTZ() __builtin_ctzl
#            define ROBIN_HOOD_PRIVATE_DEFINITION_CLZ() __builtin_clzl
#        else
#            define ROBIN_HOOD_PRIVATE_DEFINITION_CTZ() __builtin_ctzll
#            define ROBIN_HOOD_PRIVATE_DEFINITION_CLZ() __builtin_clzll
#        endif
#        define ROBIN_HOOD_COUNT_LEADING_ZEROES(x) ((x) ? ROBIN_HOOD(CLZ)(x) : ROBIN_HOOD(BITNESS))
#        define ROBIN_HOOD_COUNT_TRAILING_ZEROES(x) ((x) ? ROBIN_HOOD(CTZ)(x) : ROBIN_HOOD(BITNESS))
#    endif
#endif

// fallthrough
#ifndef __has_cpp_attribute // For backwards compatibility
#    define __has_cpp_attribute(x) 0
#endif
#if __has_cpp_attribute(clang::fallthrough)
#    define ROBIN_HOOD_PRIVATE_DEFINITION_FALLTHROUGH() [[clang::fallthrough]]
#elif __has_cpp_attribute(gnu::fallthrough)
#    define ROBIN_HOOD_PRIVATE_DEFINITION_FALLTHROUGH() [[gnu::fallthrough]]
#else
#    define ROBIN_HOOD_PRIVATE_DEFINITION_FALLTHROUGH()
#endif

// likely/unlikely
#ifdef _MSC_VER
#    define ROBIN_HOOD_LIKELY(condition) condition
#    define ROBIN_HOOD_UNLIKELY(condition) condition
#else
#    define ROBIN_HOOD_LIKELY(condition) __builtin_expect(condition, 1)
#    define ROBIN_HOOD_UNLIKELY(condition) __builtin_expect(condition, 0)
#endif

// detect if native wchar_t type is availiable in MSVC
#ifdef _MSC_VER
#    ifdef _NATIVE_WCHAR_T_DEFINED
#        define ROBIN_HOOD_PRIVATE_DEFINITION_HAS_NATIVE_WCHART() 1
#    else
#        define ROBIN_HOOD_PRIVATE_DEFINITION_HAS_NATIVE_WCHART() 0
#    endif
#else
#    define ROBIN_HOOD_PRIVATE_DEFINITION_HAS_NATIVE_WCHART() 1
#endif

// detect if MSVC supports the pair(std::piecewise_construct_t,...) consructor being constexpr
#ifdef _MSC_VER
#    if _MSC_VER <= 1900
#        define ROBIN_HOOD_PRIVATE_DEFINITION_BROKEN_CONSTEXPR() 1
#    else
#        define ROBIN_HOOD_PRIVATE_DEFINITION_BROKEN_CONSTEXPR() 0
#    endif
#else
#    define ROBIN_HOOD_PRIVATE_DEFINITION_BROKEN_CONSTEXPR() 0
#endif

// workaround missing "is_trivially_copyable" in g++ < 5.0
// See https://stackoverflow.com/a/31798726/48181
#if defined(__GNUC__) && __GNUC__ < 5
#    define ROBIN_HOOD_IS_TRIVIALLY_COPYABLE(...) __has_trivial_copy(__VA_ARGS__)
#else
#    define ROBIN_HOOD_IS_TRIVIALLY_COPYABLE(...) std::is_trivially_copyable<__VA_ARGS__>::value
#endif

// helpers for C++ versions, see https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html
#define ROBIN_HOOD_PRIVATE_DEFINITION_CXX() __cplusplus
#define ROBIN_HOOD_PRIVATE_DEFINITION_CXX98() 199711L
#define ROBIN_HOOD_PRIVATE_DEFINITION_CXX11() 201103L
#define ROBIN_HOOD_PRIVATE_DEFINITION_CXX14() 201402L
#define ROBIN_HOOD_PRIVATE_DEFINITION_CXX17() 201703L

#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX17)
#    define ROBIN_HOOD_PRIVATE_DEFINITION_NODISCARD() [[nodiscard]]
#else
#    define ROBIN_HOOD_PRIVATE_DEFINITION_NODISCARD()
#endif

namespace robin_hood {

#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX14)
#    define ROBIN_HOOD_STD std
#else

	// c++11 compatibility layer
	namespace ROBIN_HOOD_STD {
		template <class T>
		struct alignment_of
			: std::integral_constant<std::size_t, alignof(typename std::remove_all_extents<T>::type)> {};

		template <class T, T... Ints>
		class integer_sequence {
		public:
			using value_type = T;
			static_assert(std::is_integral<value_type>::value, "not integral type");
			static constexpr std::size_t size() noexcept {
				return sizeof...(Ints);
			}
		};
		template <std::size_t... Inds>
		using index_sequence = integer_sequence<std::size_t, Inds...>;

		namespace detail_ {
			template <class T, T Begin, T End, bool>
			struct IntSeqImpl {
				using TValue = T;
				static_assert(std::is_integral<TValue>::value, "not integral type");
				static_assert(Begin >= 0 && Begin < End, "unexpected argument (Begin<0 || Begin<=End)");

				template <class, class>
				struct IntSeqCombiner;

				template <TValue... Inds0, TValue... Inds1>
				struct IntSeqCombiner<integer_sequence<TValue, Inds0...>, integer_sequence<TValue, Inds1...>> {
					using TResult = integer_sequence<TValue, Inds0..., Inds1...>;
				};

				using TResult =
					typename IntSeqCombiner<typename IntSeqImpl<TValue, Begin, Begin + (End - Begin) / 2,
					(End - Begin) / 2 == 1>::TResult,
					typename IntSeqImpl<TValue, Begin + (End - Begin) / 2, End,
					(End - Begin + 1) / 2 == 1>::TResult>::TResult;
			};

			template <class T, T Begin>
			struct IntSeqImpl<T, Begin, Begin, false> {
				using TValue = T;
				static_assert(std::is_integral<TValue>::value, "not integral type");
				static_assert(Begin >= 0, "unexpected argument (Begin<0)");
				using TResult = integer_sequence<TValue>;
			};

			template <class T, T Begin, T End>
			struct IntSeqImpl<T, Begin, End, true> {
				using TValue = T;
				static_assert(std::is_integral<TValue>::value, "not integral type");
				static_assert(Begin >= 0, "unexpected argument (Begin<0)");
				using TResult = integer_sequence<TValue, Begin>;
			};
		} // namespace detail_

		template <class T, T N>
		using make_integer_sequence = typename detail_::IntSeqImpl<T, 0, N, (N - 0) == 1>::TResult;

		template <std::size_t N>
		using make_index_sequence = make_integer_sequence<std::size_t, N>;

		template <class... T>
		using index_sequence_for = make_index_sequence<sizeof...(T)>;

	} // namespace ROBIN_HOOD_STD

#endif

	namespace detail {

		// make sure we static_cast to the correct type for hash_int
#if ROBIN_HOOD(BITNESS) == 64
		using SizeT = uint64_t;
#else
		using SizeT = uint32_t;
#endif

		template <typename T>
		T rotr(T x, unsigned k) {
			return (x >> k) | (x << (8U * sizeof(T) - k));
		}

		// This cast gets rid of warnings like "cast from 'uint8_t*' {aka 'unsigned char*'} to
		// 'uint64_t*' {aka 'long unsigned int*'} increases required alignment of target type". Use with
		// care!
		template <typename T>
		inline T reinterpret_cast_no_cast_align_warning(void* ptr) noexcept {
			return reinterpret_cast<T>(ptr);
		}

		template <typename T>
		inline T reinterpret_cast_no_cast_align_warning(void const* ptr) noexcept {
			return reinterpret_cast<T>(ptr);
		}

		// make sure this is not inlined as it is slow and dramatically enlarges code, thus making other
		// inlinings more difficult. Throws are also generally the slow path.
		template <typename E, typename... Args>
		[[noreturn]] ROBIN_HOOD(NOINLINE)
#if ROBIN_HOOD(HAS_EXCEPTIONS)
			void doThrow(Args&&... args) {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
			throw E(std::forward<Args>(args)...);
		}
#else
			void doThrow(Args&&... ROBIN_HOOD_UNUSED(args) /*unused*/) {
			abort();
		}
#endif

		template <typename E, typename T, typename... Args>
		T* assertNotNull(T* t, Args&&... args) {
			if (ROBIN_HOOD_UNLIKELY(nullptr == t)) {
				doThrow<E>(std::forward<Args>(args)...);
			}
			return t;
		}

		template <typename T>
		inline T unaligned_load(void const* ptr) noexcept {
			// using memcpy so we don't get into unaligned load problems.
			// compiler should optimize this very well anyways.
			T t;
			std::memcpy(&t, ptr, sizeof(T));
			return t;
		}

		// Allocates bulks of memory for objects of type T. This deallocates the memory in the destructor,
		// and keeps a linked list of the allocated memory around. Overhead per allocation is the size of a
		// pointer.
		template <typename T, size_t MinNumAllocs = 4, size_t MaxNumAllocs = 256>
		class BulkPoolAllocator {
		public:
			BulkPoolAllocator() noexcept = default;

			// does not copy anything, just creates a new allocator.
			BulkPoolAllocator(const BulkPoolAllocator& ROBIN_HOOD_UNUSED(o) /*unused*/) noexcept
				: mHead(nullptr)
				, mListForFree(nullptr) {}

			BulkPoolAllocator(BulkPoolAllocator&& o) noexcept
				: mHead(o.mHead)
				, mListForFree(o.mListForFree) {
				o.mListForFree = nullptr;
				o.mHead = nullptr;
			}

			BulkPoolAllocator& operator=(BulkPoolAllocator&& o) noexcept {
				reset();
				mHead = o.mHead;
				mListForFree = o.mListForFree;
				o.mListForFree = nullptr;
				o.mHead = nullptr;
				return *this;
			}

			BulkPoolAllocator&
				// NOLINTNEXTLINE(bugprone-unhandled-self-assignment,cert-oop54-cpp)
				operator=(const BulkPoolAllocator& ROBIN_HOOD_UNUSED(o) /*unused*/) noexcept {
				// does not do anything
				return *this;
			}

			~BulkPoolAllocator() noexcept {
				reset();
			}

			// Deallocates all allocated memory.
			void reset() noexcept {
				while (mListForFree) {
					T* tmp = *mListForFree;
					ROBIN_HOOD_LOG("std::free")
						std::free(mListForFree);
					mListForFree = reinterpret_cast_no_cast_align_warning<T**>(tmp);
				}
				mHead = nullptr;
			}

			// allocates, but does NOT initialize. Use in-place new constructor, e.g.
			//   T* obj = pool.allocate();
			//   ::new (static_cast<void*>(obj)) T();
			T* allocate() {
				T* tmp = mHead;
				if (!tmp) {
					tmp = performAllocation();
				}

				mHead = *reinterpret_cast_no_cast_align_warning<T**>(tmp);
				return tmp;
			}

			// does not actually deallocate but puts it in store.
			// make sure you have already called the destructor! e.g. with
			//  obj->~T();
			//  pool.deallocate(obj);
			void deallocate(T* obj) noexcept {
				*reinterpret_cast_no_cast_align_warning<T**>(obj) = mHead;
				mHead = obj;
			}

			// Adds an already allocated block of memory to the allocator. This allocator is from now on
			// responsible for freeing the data (with free()). If the provided data is not large enough to
			// make use of, it is immediately freed. Otherwise it is reused and freed in the destructor.
			void addOrFree(void* ptr, const size_t numBytes) noexcept {
				// calculate number of available elements in ptr
				if (numBytes < ALIGNMENT + ALIGNED_SIZE) {
					// not enough data for at least one element. Free and return.
					ROBIN_HOOD_LOG("std::free")
						std::free(ptr);
				}
				else {
					ROBIN_HOOD_LOG("add to buffer")
						add(ptr, numBytes);
				}
			}

			void swap(BulkPoolAllocator<T, MinNumAllocs, MaxNumAllocs>& other) noexcept {
				using std::swap;
				swap(mHead, other.mHead);
				swap(mListForFree, other.mListForFree);
			}

		private:
			// iterates the list of allocated memory to calculate how many to alloc next.
			// Recalculating this each time saves us a size_t member.
			// This ignores the fact that memory blocks might have been added manually with addOrFree. In
			// practice, this should not matter much.
			ROBIN_HOOD(NODISCARD) size_t calcNumElementsToAlloc() const noexcept {
				auto tmp = mListForFree;
				size_t numAllocs = MinNumAllocs;

				while (numAllocs * 2 <= MaxNumAllocs && tmp) {
					auto x = reinterpret_cast<T***>(tmp);
					tmp = *x;
					numAllocs *= 2;
				}

				return numAllocs;
			}

			// WARNING: Underflow if numBytes < ALIGNMENT! This is guarded in addOrFree().
			void add(void* ptr, const size_t numBytes) noexcept {
				const size_t numElements = (numBytes - ALIGNMENT) / ALIGNED_SIZE;

				auto data = reinterpret_cast<T**>(ptr);

				// link free list
				auto x = reinterpret_cast<T***>(data);
				*x = mListForFree;
				mListForFree = data;

				// create linked list for newly allocated data
				auto* const headT =
					reinterpret_cast_no_cast_align_warning<T*>(reinterpret_cast<char*>(ptr) + ALIGNMENT);

				auto* const head = reinterpret_cast<char*>(headT);

				// Visual Studio compiler automatically unrolls this loop, which is pretty cool
				for (size_t i = 0; i < numElements; ++i) {
					*reinterpret_cast_no_cast_align_warning<char**>(head + i * ALIGNED_SIZE) =
						head + (i + 1) * ALIGNED_SIZE;
				}

				// last one points to 0
				*reinterpret_cast_no_cast_align_warning<T**>(head + (numElements - 1) * ALIGNED_SIZE) =
					mHead;
				mHead = headT;
			}

			// Called when no memory is available (mHead == 0).
			// Don't inline this slow path.
			ROBIN_HOOD(NOINLINE) T* performAllocation() {
				size_t const numElementsToAlloc = calcNumElementsToAlloc();

				// alloc new memory: [prev |T, T, ... T]
				size_t const bytes = ALIGNMENT + ALIGNED_SIZE * numElementsToAlloc;
				ROBIN_HOOD_LOG("std::malloc " << bytes << " = " << ALIGNMENT << " + " << ALIGNED_SIZE
					<< " * " << numElementsToAlloc)
					add(assertNotNull<std::bad_alloc>(std::malloc(bytes)), bytes);
				return mHead;
			}

			// enforce byte alignment of the T's
#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX14)
			static constexpr size_t ALIGNMENT =
				(std::max)(std::alignment_of<T>::value, std::alignment_of<T*>::value);
#else
			static const size_t ALIGNMENT =
				(ROBIN_HOOD_STD::alignment_of<T>::value > ROBIN_HOOD_STD::alignment_of<T*>::value)
				? ROBIN_HOOD_STD::alignment_of<T>::value
				: +ROBIN_HOOD_STD::alignment_of<T*>::value; // the + is for walkarround
#endif

			static constexpr size_t ALIGNED_SIZE = ((sizeof(T) - 1) / ALIGNMENT + 1) * ALIGNMENT;

			static_assert(MinNumAllocs >= 1, "MinNumAllocs");
			static_assert(MaxNumAllocs >= MinNumAllocs, "MaxNumAllocs");
			static_assert(ALIGNED_SIZE >= sizeof(T*), "ALIGNED_SIZE");
			static_assert(0 == (ALIGNED_SIZE % sizeof(T*)), "ALIGNED_SIZE mod");
			static_assert(ALIGNMENT >= sizeof(T*), "ALIGNMENT");

			T* mHead{ nullptr };
			T** mListForFree{ nullptr };
		};

		template <typename T, size_t MinSize, size_t MaxSize, bool IsFlat>
		struct NodeAllocator;

		// dummy allocator that does nothing
		template <typename T, size_t MinSize, size_t MaxSize>
		struct NodeAllocator<T, MinSize, MaxSize, true> {

			// we are not using the data, so just free it.
			void addOrFree(void* ptr, size_t ROBIN_HOOD_UNUSED(numBytes) /*unused*/) noexcept {
				ROBIN_HOOD_LOG("std::free")
					std::free(ptr);
			}
		};

		template <typename T, size_t MinSize, size_t MaxSize>
		struct NodeAllocator<T, MinSize, MaxSize, false> : public BulkPoolAllocator<T, MinSize, MaxSize> {};

		// c++14 doesn't have is_nothrow_swappable, and clang++ 6.0.1 doesn't like it either, so I'm making
		// my own here.
		namespace swappable {
#if ROBIN_HOOD(CXX) < ROBIN_HOOD(CXX17)
			using std::swap;
			template <typename T>
			struct nothrow {
				static const bool value = noexcept(swap(std::declval<T&>(), std::declval<T&>()));
			};
#else
			template <typename T>
			struct nothrow {
				static const bool value = std::is_nothrow_swappable<T>::value;
			};
#endif
		} // namespace swappable

	} // namespace detail

	struct is_transparent_tag {};

	// A custom pair implementation is used in the map because std::pair is not is_trivially_copyable,
	// which means it would  not be allowed to be used in std::memcpy. This struct is copyable, which is
	// also tested.
	template <typename T1, typename T2>
	struct pair {
		using first_type = T1;
		using second_type = T2;

		template <typename U1 = T1, typename U2 = T2,
			typename = typename std::enable_if<std::is_default_constructible<U1>::value&&
			std::is_default_constructible<U2>::value>::type>
			constexpr pair() noexcept(noexcept(U1()) && noexcept(U2()))
			: first()
			, second() {}

		// pair constructors are explicit so we don't accidentally call this ctor when we don't have to.
		explicit constexpr pair(std::pair<T1, T2> const& o) noexcept(
			noexcept(T1(std::declval<T1 const&>())) && noexcept(T2(std::declval<T2 const&>())))
			: first(o.first)
			, second(o.second) {}

		// pair constructors are explicit so we don't accidentally call this ctor when we don't have to.
		explicit constexpr pair(std::pair<T1, T2>&& o) noexcept(noexcept(
			T1(std::move(std::declval<T1&&>()))) && noexcept(T2(std::move(std::declval<T2&&>()))))
			: first(std::move(o.first))
			, second(std::move(o.second)) {}

		constexpr pair(T1&& a, T2&& b) noexcept(noexcept(
			T1(std::move(std::declval<T1&&>()))) && noexcept(T2(std::move(std::declval<T2&&>()))))
			: first(std::move(a))
			, second(std::move(b)) {}

		template <typename U1, typename U2>
		constexpr pair(U1&& a, U2&& b) noexcept(noexcept(T1(std::forward<U1>(
			std::declval<U1&&>()))) && noexcept(T2(std::forward<U2>(std::declval<U2&&>()))))
			: first(std::forward<U1>(a))
			, second(std::forward<U2>(b)) {}

		template <typename... U1, typename... U2>
		// MSVC 2015 produces error "C2476: ‘constexpr’ constructor does not initialize all members"
		// if this constructor is constexpr
#if !ROBIN_HOOD(BROKEN_CONSTEXPR)
		constexpr
#endif
			pair(std::piecewise_construct_t /*unused*/, std::tuple<U1...> a,
				std::tuple<U2...>
				b) noexcept(noexcept(pair(std::declval<std::tuple<U1...>&>(),
					std::declval<std::tuple<U2...>&>(),
					ROBIN_HOOD_STD::index_sequence_for<U1...>(),
					ROBIN_HOOD_STD::index_sequence_for<U2...>())))
			: pair(a, b, ROBIN_HOOD_STD::index_sequence_for<U1...>(),
				ROBIN_HOOD_STD::index_sequence_for<U2...>()) {
		}

		// constructor called from the std::piecewise_construct_t ctor
		template <typename... U1, size_t... I1, typename... U2, size_t... I2>
		pair(std::tuple<U1...>& a, std::tuple<U2...>& b, ROBIN_HOOD_STD::index_sequence<I1...> /*unused*/, ROBIN_HOOD_STD::index_sequence<I2...> /*unused*/) noexcept(
			noexcept(T1(std::forward<U1>(std::get<I1>(
				std::declval<std::tuple<
				U1...>&>()))...)) && noexcept(T2(std::
					forward<U2>(std::get<I2>(
						std::declval<std::tuple<U2...>&>()))...)))
			: first(std::forward<U1>(std::get<I1>(a))...)
			, second(std::forward<U2>(std::get<I2>(b))...) {
			// make visual studio compiler happy about warning about unused a & b.
			// Visual studio's pair implementation disables warning 4100.
			(void)a;
			(void)b;
		}

		void swap(pair<T1, T2>& o) noexcept((detail::swappable::nothrow<T1>::value) &&
			(detail::swappable::nothrow<T2>::value)) {
			using std::swap;
			swap(first, o.first);
			swap(second, o.second);
		}

		T1 first;  // NOLINT(misc-non-private-member-variables-in-classes)
		T2 second; // NOLINT(misc-non-private-member-variables-in-classes)
	};

	template <typename A, typename B>
	inline void swap(pair<A, B>& a, pair<A, B>& b) noexcept(
		noexcept(std::declval<pair<A, B>&>().swap(std::declval<pair<A, B>&>()))) {
		a.swap(b);
	}

	template <typename A, typename B>
	inline constexpr bool operator==(pair<A, B> const& x, pair<A, B> const& y) {
		return (x.first == y.first) && (x.second == y.second);
	}
	template <typename A, typename B>
	inline constexpr bool operator!=(pair<A, B> const& x, pair<A, B> const& y) {
		return !(x == y);
	}
	template <typename A, typename B>
	inline constexpr bool operator<(pair<A, B> const& x, pair<A, B> const& y) noexcept(noexcept(
		std::declval<A const&>() < std::declval<A const&>()) && noexcept(std::declval<B const&>() <
			std::declval<B const&>())) {
		return x.first < y.first || (!(y.first < x.first) && x.second < y.second);
	}
	template <typename A, typename B>
	inline constexpr bool operator>(pair<A, B> const& x, pair<A, B> const& y) {
		return y < x;
	}
	template <typename A, typename B>
	inline constexpr bool operator<=(pair<A, B> const& x, pair<A, B> const& y) {
		return !(x > y);
	}
	template <typename A, typename B>
	inline constexpr bool operator>=(pair<A, B> const& x, pair<A, B> const& y) {
		return !(x < y);
	}

	inline size_t hash_bytes(void const* ptr, size_t len) noexcept {
		static constexpr uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
		static constexpr uint64_t seed = UINT64_C(0xe17a1465);
		static constexpr unsigned int r = 47;

		auto const* const data64 = static_cast<uint64_t const*>(ptr);
		uint64_t h = seed ^ (len * m);

		size_t const n_blocks = len / 8;
		for (size_t i = 0; i < n_blocks; ++i) {
			auto k = detail::unaligned_load<uint64_t>(data64 + i);

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		auto const* const data8 = reinterpret_cast<uint8_t const*>(data64 + n_blocks);
		switch (len & 7U) {
		case 7:
			h ^= static_cast<uint64_t>(data8[6]) << 48U;
			ROBIN_HOOD(FALLTHROUGH); // FALLTHROUGH
		case 6:
			h ^= static_cast<uint64_t>(data8[5]) << 40U;
			ROBIN_HOOD(FALLTHROUGH); // FALLTHROUGH
		case 5:
			h ^= static_cast<uint64_t>(data8[4]) << 32U;
			ROBIN_HOOD(FALLTHROUGH); // FALLTHROUGH
		case 4:
			h ^= static_cast<uint64_t>(data8[3]) << 24U;
			ROBIN_HOOD(FALLTHROUGH); // FALLTHROUGH
		case 3:
			h ^= static_cast<uint64_t>(data8[2]) << 16U;
			ROBIN_HOOD(FALLTHROUGH); // FALLTHROUGH
		case 2:
			h ^= static_cast<uint64_t>(data8[1]) << 8U;
			ROBIN_HOOD(FALLTHROUGH); // FALLTHROUGH
		case 1:
			h ^= static_cast<uint64_t>(data8[0]);
			h *= m;
			ROBIN_HOOD(FALLTHROUGH); // FALLTHROUGH
		default:
			break;
		}

		h ^= h >> r;

		// not doing the final step here, because this will be done by keyToIdx anyways
		// h *= m;
		// h ^= h >> r;
		return static_cast<size_t>(h);
	}

	inline size_t hash_int(uint64_t x) noexcept {
		// tried lots of different hashes, let's stick with murmurhash3. It's simple, fast, well tested,
		// and doesn't need any special 128bit operations.
		x ^= x >> 33U;
		x *= UINT64_C(0xff51afd7ed558ccd);
		x ^= x >> 33U;

		// not doing the final step here, because this will be done by keyToIdx anyways
		// x *= UINT64_C(0xc4ceb9fe1a85ec53);
		// x ^= x >> 33U;
		return static_cast<size_t>(x);
	}

	// A thin wrapper around std::hash, performing an additional simple mixing step of the result.
	template <typename T, typename Enable = void>
	struct hash : public std::hash<T> {
		size_t operator()(T const& obj) const
			noexcept(noexcept(std::declval<std::hash<T>>().operator()(std::declval<T const&>()))) {
			// call base hash
			auto result = std::hash<T>::operator()(obj);
			// return mixed of that, to be save against identity has
			return hash_int(static_cast<detail::SizeT>(result));
		}
	};

	template <typename CharT>
	struct hash<std::basic_string<CharT>> {
		size_t operator()(std::basic_string<CharT> const& str) const noexcept {
			return hash_bytes(str.data(), sizeof(CharT) * str.size());
		}
	};

#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX17)
	template <typename CharT>
	struct hash<std::basic_string_view<CharT>> {
		size_t operator()(std::basic_string_view<CharT> const& sv) const noexcept {
			return hash_bytes(sv.data(), sizeof(CharT) * sv.size());
		}
	};
#endif

	template <class T>
	struct hash<T*> {
		size_t operator()(T* ptr) const noexcept {
			return hash_int(reinterpret_cast<detail::SizeT>(ptr));
		}
	};

	template <class T>
	struct hash<std::unique_ptr<T>> {
		size_t operator()(std::unique_ptr<T> const& ptr) const noexcept {
			return hash_int(reinterpret_cast<detail::SizeT>(ptr.get()));
		}
	};

	template <class T>
	struct hash<std::shared_ptr<T>> {
		size_t operator()(std::shared_ptr<T> const& ptr) const noexcept {
			return hash_int(reinterpret_cast<detail::SizeT>(ptr.get()));
		}
	};

	template <class T>
	struct hash<cweeSharedPtr<T>> {
		size_t operator()(cweeSharedPtr<T> const& ptr) const noexcept {
			return hash_int(reinterpret_cast<detail::SizeT>(ptr.get()));
		}
	};

	template <typename Enum>
	struct hash<Enum, typename std::enable_if<std::is_enum<Enum>::value>::type> {
		size_t operator()(Enum e) const noexcept {
			using Underlying = typename std::underlying_type<Enum>::type;
			return hash<Underlying>{}(static_cast<Underlying>(e));
		}
	};

#define ROBIN_HOOD_HASH_INT(T)                           \
    template <>                                          \
    struct hash<T> {                                     \
        size_t operator()(T const& obj) const noexcept { \
            return hash_int(static_cast<uint64_t>(obj)); \
        }                                                \
    }

#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
	// see https://en.cppreference.com/w/cpp/utility/hash
	ROBIN_HOOD_HASH_INT(bool);
	ROBIN_HOOD_HASH_INT(char);
	ROBIN_HOOD_HASH_INT(signed char);
	ROBIN_HOOD_HASH_INT(unsigned char);
	ROBIN_HOOD_HASH_INT(char16_t);
	ROBIN_HOOD_HASH_INT(char32_t);
#if ROBIN_HOOD(HAS_NATIVE_WCHART)
	ROBIN_HOOD_HASH_INT(wchar_t);
#endif
	ROBIN_HOOD_HASH_INT(short);
	ROBIN_HOOD_HASH_INT(unsigned short);
	ROBIN_HOOD_HASH_INT(int);
	ROBIN_HOOD_HASH_INT(unsigned int);
	ROBIN_HOOD_HASH_INT(long);
	ROBIN_HOOD_HASH_INT(long long);
	ROBIN_HOOD_HASH_INT(unsigned long);
	ROBIN_HOOD_HASH_INT(unsigned long long);
#if defined(__GNUC__) && !defined(__clang__)
#    pragma GCC diagnostic pop
#endif
	namespace detail {

		template <typename T>
		struct void_type {
			using type = void;
		};

		template <typename T, typename = void>
		struct has_is_transparent : public std::false_type {};

		template <typename T>
		struct has_is_transparent<T, typename void_type<typename T::is_transparent>::type>
			: public std::true_type {};

		// using wrapper classes for hash and key_equal prevents the diamond problem when the same type
		// is used. see https://stackoverflow.com/a/28771920/48181
		template <typename T>
		struct WrapHash : public T {
			WrapHash() = default;
			explicit WrapHash(T const& o) noexcept(noexcept(T(std::declval<T const&>())))
				: T(o) {}
		};

		template <typename T>
		struct WrapKeyEqual : public T {
			WrapKeyEqual() = default;
			explicit WrapKeyEqual(T const& o) noexcept(noexcept(T(std::declval<T const&>())))
				: T(o) {}
		};

		// A highly optimized hashmap implementation, using the Robin Hood algorithm.
		//
		// In most cases, this map should be usable as a drop-in replacement for std::unordered_map, but
		// be about 2x faster in most cases and require much less allocations.
		//
		// This implementation uses the following memory layout:
		//
		// [Node, Node, ... Node | info, info, ... infoSentinel ]
		//
		// * Node: either a DataNode that directly has the std::pair<key, val> as member,
		//   or a DataNode with a pointer to std::pair<key,val>. Which DataNode representation to use
		//   depends on how fast the swap() operation is. Heuristically, this is automatically choosen
		//   based on sizeof(). there are always 2^n Nodes.
		//
		// * info: Each Node in the map has a corresponding info byte, so there are 2^n info bytes.
		//   Each byte is initialized to 0, meaning the corresponding Node is empty. Set to 1 means the
		//   corresponding node contains data. Set to 2 means the corresponding Node is filled, but it
		//   actually belongs to the previous position and was pushed out because that place is already
		//   taken.
		//
		// * infoSentinel: Sentinel byte set to 1, so that iterator's ++ can stop at end() without the
		//   need for a idx variable.
		//
		// According to STL, order of templates has effect on throughput. That's why I've moved the
		// boolean to the front.
		// https://www.reddit.com/r/cpp/comments/ahp6iu/compile_time_binary_size_reductions_and_cs_future/eeguck4/
		template <bool IsFlat, size_t MaxLoadFactor100, typename Key, typename T, typename Hash,
			typename KeyEqual>
			class Table
			: public WrapHash<Hash>,
			public WrapKeyEqual<KeyEqual>,
			detail::NodeAllocator<
			typename std::conditional<
			std::is_void<T>::value, Key,
			robin_hood::pair<typename std::conditional<IsFlat, Key, Key const>::type, T>>::type,
			4, 16384, IsFlat> {
			public:
				static constexpr bool is_flat = IsFlat;
				static constexpr bool is_map = !std::is_void<T>::value;
				static constexpr bool is_set = !is_map;
				static constexpr bool is_transparent =
					has_is_transparent<Hash>::value && has_is_transparent<KeyEqual>::value;

				using key_type = Key;
				using mapped_type = T;
				using value_type = typename std::conditional<
					is_set, Key,
					robin_hood::pair<typename std::conditional<is_flat, Key, Key const>::type, T>>::type;
				using size_type = size_t;
				using hasher = Hash;
				using key_equal = KeyEqual;
				using Self = Table<IsFlat, MaxLoadFactor100, key_type, mapped_type, hasher, key_equal>;

			private:
				static_assert(MaxLoadFactor100 > 10 && MaxLoadFactor100 < 100,
					"MaxLoadFactor100 needs to be >10 && < 100");

				using WHash = WrapHash<Hash>;
				using WKeyEqual = WrapKeyEqual<KeyEqual>;

				// configuration defaults

				// make sure we have 8 elements, needed to quickly rehash mInfo
				static constexpr size_t InitialNumElements = sizeof(uint64_t);
				static constexpr uint32_t InitialInfoNumBits = 5;
				static constexpr uint8_t InitialInfoInc = 1U << InitialInfoNumBits;
				static constexpr size_t InfoMask = InitialInfoInc - 1U;
				static constexpr uint8_t InitialInfoHashShift = 0;
				using DataPool = detail::NodeAllocator<value_type, 4, 16384, IsFlat>;

				// type needs to be wider than uint8_t.
				using InfoType = uint32_t;

				// DataNode ////////////////////////////////////////////////////////

				// Primary template for the data node. We have special implementations for small and big
				// objects. For large objects it is assumed that swap() is fairly slow, so we allocate these
				// on the heap so swap merely swaps a pointer.
				template <typename M, bool>
				class DataNode {};

				// Small: just allocate on the stack.
				template <typename M>
				class DataNode<M, true> final {
				public:
					template <typename... Args>
					explicit DataNode(M& ROBIN_HOOD_UNUSED(map) /*unused*/, Args&&... args) noexcept(
						noexcept(value_type(std::forward<Args>(args)...)))
						: mData(std::forward<Args>(args)...) {}

					DataNode(M& ROBIN_HOOD_UNUSED(map) /*unused*/, DataNode<M, true>&& n) noexcept(
						std::is_nothrow_move_constructible<value_type>::value)
						: mData(std::move(n.mData)) {}

					// doesn't do anything
					void destroy(M& ROBIN_HOOD_UNUSED(map) /*unused*/) noexcept {}
					void destroyDoNotDeallocate() noexcept {}

					value_type const* operator->() const noexcept {
						return &mData;
					}
					value_type* operator->() noexcept {
						return &mData;
					}

					const value_type& operator*() const noexcept {
						return mData;
					}

					value_type& operator*() noexcept {
						return mData;
					}

					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_map, typename VT::first_type&>::type getFirst() noexcept {
						return mData.first;
					}
					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_set, VT&>::type getFirst() noexcept {
						return mData;
					}

					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_map, typename VT::first_type const&>::type
						getFirst() const noexcept {
						return mData.first;
					}
					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_set, VT const&>::type getFirst() const noexcept {
						return mData;
					}

					template <typename MT = mapped_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_map, MT&>::type getSecond() noexcept {
						return mData.second;
					}

					template <typename MT = mapped_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_set, MT const&>::type getSecond() const noexcept {
						return mData.second;
					}

					void swap(DataNode<M, true>& o) noexcept(
						noexcept(std::declval<value_type>().swap(std::declval<value_type>()))) {
						mData.swap(o.mData);
					}

				private:
					value_type mData;
				};

				// big object: allocate on heap.
				template <typename M>
				class DataNode<M, false> {
				public:
					template <typename... Args>
					explicit DataNode(M& map, Args&&... args)
						: mData(map.allocate()) {
						::new (static_cast<void*>(mData)) value_type(std::forward<Args>(args)...);
					}

					DataNode(M& ROBIN_HOOD_UNUSED(map) /*unused*/, DataNode<M, false>&& n) noexcept
						: mData(std::move(n.mData)) {}

					void destroy(M& map) noexcept {
						// don't deallocate, just put it into list of datapool.
						mData->~value_type();
						map.deallocate(mData);
					}

					void destroyDoNotDeallocate() noexcept {
						mData->~value_type();
					}

					value_type const* operator->() const noexcept {
						return mData;
					}

					value_type* operator->() noexcept {
						return mData;
					}

					const value_type& operator*() const {
						return *mData;
					}

					value_type& operator*() {
						return *mData;
					}

					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_map, typename VT::first_type&>::type getFirst() noexcept {
						return mData->first;
					}
					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_set, VT&>::type getFirst() noexcept {
						return *mData;
					}

					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_map, typename VT::first_type const&>::type
						getFirst() const noexcept {
						return mData->first;
					}
					template <typename VT = value_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_set, VT const&>::type getFirst() const noexcept {
						return *mData;
					}

					template <typename MT = mapped_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_map, MT&>::type getSecond() noexcept {
						return mData->second;
					}

					template <typename MT = mapped_type>
					ROBIN_HOOD(NODISCARD)
						typename std::enable_if<is_map, MT const&>::type getSecond() const noexcept {
						return mData->second;
					}

					void swap(DataNode<M, false>& o) noexcept {
						using std::swap;
						swap(mData, o.mData);
					}

				private:
					value_type* mData;
				};

				using Node = DataNode<Self, IsFlat>;

				// helpers for insertKeyPrepareEmptySpot: extract first entry (only const required)
				ROBIN_HOOD(NODISCARD) key_type const& getFirstConst(Node const& n) const noexcept {
					return n.getFirst();
				}

				// in case we have void mapped_type, we are not using a pair, thus we just route k through.
				// No need to disable this because it's just not used if not applicable.
				ROBIN_HOOD(NODISCARD) key_type const& getFirstConst(key_type const& k) const noexcept {
					return k;
				}

				// in case we have non-void mapped_type, we have a standard robin_hood::pair
				template <typename Q = mapped_type>
				ROBIN_HOOD(NODISCARD)
					typename std::enable_if<!std::is_void<Q>::value, key_type const&>::type
					getFirstConst(value_type const& vt) const noexcept {
					return vt.first;
				}

				// Cloner //////////////////////////////////////////////////////////

				template <typename M, bool UseMemcpy>
				struct Cloner;

				// fast path: Just copy data, without allocating anything.
				template <typename M>
				struct Cloner<M, true> {
					void operator()(M const& source, M& target) const {
						auto const* const src = reinterpret_cast<char const*>(source.mKeyVals);
						auto* tgt = reinterpret_cast<char*>(target.mKeyVals);
						auto const numElementsWithBuffer = target.calcNumElementsWithBuffer(target.mMask + 1);
						std::copy(src, src + target.calcNumBytesTotal(numElementsWithBuffer), tgt);
					}
				};

				template <typename M>
				struct Cloner<M, false> {
					void operator()(M const& s, M& t) const {
						auto const numElementsWithBuffer = t.calcNumElementsWithBuffer(t.mMask + 1);
						std::copy(s.mInfo, s.mInfo + t.calcNumBytesInfo(numElementsWithBuffer), t.mInfo);

						for (size_t i = 0; i < numElementsWithBuffer; ++i) {
							if (t.mInfo[i]) {
								::new (static_cast<void*>(t.mKeyVals + i)) Node(t, *s.mKeyVals[i]);
							}
						}
					}
				};

				// Destroyer ///////////////////////////////////////////////////////

				template <typename M, bool IsFlatAndTrivial>
				struct Destroyer {};

				template <typename M>
				struct Destroyer<M, true> {
					void nodes(M& m) const noexcept {
						m.mNumElements = 0;
					}

					void nodesDoNotDeallocate(M& m) const noexcept {
						m.mNumElements = 0;
					}
				};

				template <typename M>
				struct Destroyer<M, false> {
					void nodes(M& m) const noexcept {
						m.mNumElements = 0;
						// clear also resets mInfo to 0, that's sometimes not necessary.
						auto const numElementsWithBuffer = m.calcNumElementsWithBuffer(m.mMask + 1);

						for (size_t idx = 0; idx < numElementsWithBuffer; ++idx) {
							if (0 != m.mInfo[idx]) {
								Node& n = m.mKeyVals[idx];
								n.destroy(m);
								n.~Node();
							}
						}
					}

					void nodesDoNotDeallocate(M& m) const noexcept {
						m.mNumElements = 0;
						// clear also resets mInfo to 0, that's sometimes not necessary.
						auto const numElementsWithBuffer = m.calcNumElementsWithBuffer(m.mMask + 1);
						for (size_t idx = 0; idx < numElementsWithBuffer; ++idx) {
							if (0 != m.mInfo[idx]) {
								Node& n = m.mKeyVals[idx];
								n.destroyDoNotDeallocate();
								n.~Node();
							}
						}
					}
				};

				// Iter ////////////////////////////////////////////////////////////

				struct fast_forward_tag {};

				// generic iterator for both const_iterator and iterator.
				template <bool IsConst>
				// NOLINTNEXTLINE(hicpp-special-member-functions,cppcoreguidelines-special-member-functions)
				class Iter {
				private:
					using NodePtr = typename std::conditional<IsConst, Node const*, Node*>::type;

				public:
					using difference_type = std::ptrdiff_t;
					using value_type = typename Self::value_type;
					using reference = typename std::conditional<IsConst, value_type const&, value_type&>::type;
					using pointer = typename std::conditional<IsConst, value_type const*, value_type*>::type;
					using iterator_category = std::forward_iterator_tag;

					// default constructed iterator can be compared to itself, but WON'T return true when
					// compared to end().
					Iter() = default;

					// Rule of zero: nothing specified. The conversion constructor is only enabled for
					// iterator to const_iterator, so it doesn't accidentally work as a copy ctor.

					// Conversion constructor from iterator to const_iterator.
					template <bool OtherIsConst,
						typename = typename std::enable_if<IsConst && !OtherIsConst>::type>
						// NOLINTNEXTLINE(hicpp-explicit-conversions)
						Iter(Iter<OtherIsConst> const& other) noexcept
						: mKeyVals(other.mKeyVals)
						, mInfo(other.mInfo) {}

					Iter(NodePtr valPtr, uint8_t const* infoPtr) noexcept
						: mKeyVals(valPtr)
						, mInfo(infoPtr) {}

					Iter(NodePtr valPtr, uint8_t const* infoPtr,
						fast_forward_tag ROBIN_HOOD_UNUSED(tag) /*unused*/) noexcept
						: mKeyVals(valPtr)
						, mInfo(infoPtr) {
						fastForward();
					}

					template <bool OtherIsConst,
						typename = typename std::enable_if<IsConst && !OtherIsConst>::type>
						Iter& operator=(Iter<OtherIsConst> const& other) noexcept {
						mKeyVals = other.mKeyVals;
						mInfo = other.mInfo;
						return *this;
					}

					// prefix increment. Undefined behavior if we are at end()!
					Iter& operator++() noexcept {
						mInfo++;
						mKeyVals++;
						fastForward();
						return *this;
					}

					Iter operator++(int) noexcept {
						Iter tmp = *this;
						++(*this);
						return tmp;
					}

					reference operator*() const {
						return **mKeyVals;
					}

					pointer operator->() const {
						return &**mKeyVals;
					}

					template <bool O>
					bool operator==(Iter<O> const& o) const noexcept {
						return mKeyVals == o.mKeyVals;
					}

					template <bool O>
					bool operator!=(Iter<O> const& o) const noexcept {
						return mKeyVals != o.mKeyVals;
					}

				private:
					// fast forward to the next non-free info byte
					// I've tried a few variants that don't depend on intrinsics, but unfortunately they are
					// quite a bit slower than this one. So I've reverted that change again. See map_benchmark.
					void fastForward() noexcept {
						size_t n = 0;
						while (0U == (n = detail::unaligned_load<size_t>(mInfo))) {
							mInfo += sizeof(size_t);
							mKeyVals += sizeof(size_t);
						}
#if defined(ROBIN_HOOD_DISABLE_INTRINSICS)
						// we know for certain that within the next 8 bytes we'll find a non-zero one.
						if (ROBIN_HOOD_UNLIKELY(0U == detail::unaligned_load<uint32_t>(mInfo))) {
							mInfo += 4;
							mKeyVals += 4;
						}
						if (ROBIN_HOOD_UNLIKELY(0U == detail::unaligned_load<uint16_t>(mInfo))) {
							mInfo += 2;
							mKeyVals += 2;
						}
						if (ROBIN_HOOD_UNLIKELY(0U == *mInfo)) {
							mInfo += 1;
							mKeyVals += 1;
						}
#else
#    if ROBIN_HOOD(LITTLE_ENDIAN)
						auto inc = ROBIN_HOOD_COUNT_TRAILING_ZEROES(n) / 8;
#    else
						auto inc = ROBIN_HOOD_COUNT_LEADING_ZEROES(n) / 8;
#    endif
						mInfo += inc;
						mKeyVals += inc;
#endif
					}

					friend class Table<IsFlat, MaxLoadFactor100, key_type, mapped_type, hasher, key_equal>;
					NodePtr mKeyVals{ nullptr };
					uint8_t const* mInfo{ nullptr };
				};

				////////////////////////////////////////////////////////////////////

				// highly performance relevant code.
				// Lower bits are used for indexing into the array (2^n size)
				// The upper 1-5 bits need to be a reasonable good hash, to save comparisons.
				template <typename HashKey>
				void keyToIdx(HashKey&& key, size_t* idx, InfoType* info) const {
					// In addition to whatever hash is used, add another mul & shift so we get better hashing.
					// This serves as a bad hash prevention, if the given data is
					// badly mixed.
					auto h = static_cast<uint64_t>(WHash::operator()(key));

					h *= mHashMultiplier;
					h ^= h >> 33U;

					// the lower InitialInfoNumBits are reserved for info.
					*info = mInfoInc + static_cast<InfoType>((h & InfoMask) >> mInfoHashShift);
					*idx = (static_cast<size_t>(h) >> InitialInfoNumBits) & mMask;
				}

				// forwards the index by one, wrapping around at the end
				void next(InfoType* info, size_t* idx) const noexcept {
					*idx = *idx + 1;
					*info += mInfoInc;
				}

				void nextWhileLess(InfoType* info, size_t* idx) const noexcept {
					// unrolling this by hand did not bring any speedups.
					while (*info < mInfo[*idx]) {
						next(info, idx);
					}
				}

				// Shift everything up by one element. Tries to move stuff around.
				void
					shiftUp(size_t startIdx,
						size_t const insertion_idx) noexcept(std::is_nothrow_move_assignable<Node>::value) {
					auto idx = startIdx;
					::new (static_cast<void*>(mKeyVals + idx)) Node(std::move(mKeyVals[idx - 1]));
					while (--idx != insertion_idx) {
						mKeyVals[idx] = std::move(mKeyVals[idx - 1]);
					}

					idx = startIdx;
					while (idx != insertion_idx) {
						ROBIN_HOOD_COUNT(shiftUp)
							mInfo[idx] = static_cast<uint8_t>(mInfo[idx - 1] + mInfoInc);
						if (ROBIN_HOOD_UNLIKELY(mInfo[idx] + mInfoInc > 0xFF)) {
							mMaxNumElementsAllowed = 0;
						}
						--idx;
					}
				}

				void shiftDown(size_t idx) noexcept(std::is_nothrow_move_assignable<Node>::value) {
					// until we find one that is either empty or has zero offset.
					// TODO(martinus) we don't need to move everything, just the last one for the same
					// bucket.
					mKeyVals[idx].destroy(*this);

					// until we find one that is either empty or has zero offset.
					while (mInfo[idx + 1] >= 2 * mInfoInc) {
						ROBIN_HOOD_COUNT(shiftDown)
							mInfo[idx] = static_cast<uint8_t>(mInfo[idx + 1] - mInfoInc);
						mKeyVals[idx] = std::move(mKeyVals[idx + 1]);
						++idx;
					}

					mInfo[idx] = 0;
					// don't destroy, we've moved it
					// mKeyVals[idx].destroy(*this);
					mKeyVals[idx].~Node();
				}

				// copy of find(), except that it returns iterator instead of const_iterator.
				template <typename Other>
				ROBIN_HOOD(NODISCARD)
					size_t findIdx(Other const& key) const {
					size_t idx{};
					InfoType info{};
					keyToIdx(key, &idx, &info);

					do {
						// unrolling this twice gives a bit of a speedup. More unrolling did not help.
						if (info == mInfo[idx] &&
							ROBIN_HOOD_LIKELY(WKeyEqual::operator()(key, mKeyVals[idx].getFirst()))) {
							return idx;
						}
						next(&info, &idx);
						if (info == mInfo[idx] &&
							ROBIN_HOOD_LIKELY(WKeyEqual::operator()(key, mKeyVals[idx].getFirst()))) {
							return idx;
						}
						next(&info, &idx);
					} while (info <= mInfo[idx]);

					// nothing found!
					return mMask == 0 ? 0
						: static_cast<size_t>(std::distance(
							mKeyVals, reinterpret_cast_no_cast_align_warning<Node*>(mInfo)));
				}

				void cloneData(const Table& o) {
					Cloner<Table, IsFlat&& ROBIN_HOOD_IS_TRIVIALLY_COPYABLE(Node)>()(o, *this);
				}

				// inserts a keyval that is guaranteed to be new, e.g. when the hashmap is resized.
				// @return True on success, false if something went wrong
				void insert_move(Node&& keyval) {
					// we don't retry, fail if overflowing
					// don't need to check max num elements
					if (0 == mMaxNumElementsAllowed && !try_increase_info()) {
						throwOverflowError();
					}

					size_t idx{};
					InfoType info{};
					keyToIdx(keyval.getFirst(), &idx, &info);

					// skip forward. Use <= because we are certain that the element is not there.
					while (info <= mInfo[idx]) {
						idx = idx + 1;
						info += mInfoInc;
					}

					// key not found, so we are now exactly where we want to insert it.
					auto const insertion_idx = idx;
					auto const insertion_info = static_cast<uint8_t>(info);
					if (ROBIN_HOOD_UNLIKELY(insertion_info + mInfoInc > 0xFF)) {
						mMaxNumElementsAllowed = 0;
					}

					// find an empty spot
					while (0 != mInfo[idx]) {
						next(&info, &idx);
					}

					auto& l = mKeyVals[insertion_idx];
					if (idx == insertion_idx) {
						::new (static_cast<void*>(&l)) Node(std::move(keyval));
					}
					else {
						shiftUp(idx, insertion_idx);
						l = std::move(keyval);
					}

					// put at empty spot
					mInfo[insertion_idx] = insertion_info;

					++mNumElements;
				}

			public:
				using iterator = Iter<false>;
				using const_iterator = Iter<true>;

				Table() noexcept(noexcept(Hash()) && noexcept(KeyEqual()))
					: WHash()
					, WKeyEqual() {
					ROBIN_HOOD_TRACE(this)
				}

				// Creates an empty hash map. Nothing is allocated yet, this happens at the first insert.
				// This tremendously speeds up ctor & dtor of a map that never receives an element. The
				// penalty is payed at the first insert, and not before. Lookup of this empty map works
				// because everybody points to DummyInfoByte::b. parameter bucket_count is dictated by the
				// standard, but we can ignore it.
				explicit Table(
					size_t ROBIN_HOOD_UNUSED(bucket_count) /*unused*/, const Hash& h = Hash{},
					const KeyEqual& equal = KeyEqual{}) noexcept(noexcept(Hash(h)) && noexcept(KeyEqual(equal)))
					: WHash(h)
					, WKeyEqual(equal) {
					ROBIN_HOOD_TRACE(this)
				}

				template <typename Iter>
				Table(Iter first, Iter last, size_t ROBIN_HOOD_UNUSED(bucket_count) /*unused*/ = 0,
					const Hash& h = Hash{}, const KeyEqual& equal = KeyEqual{})
					: WHash(h)
					, WKeyEqual(equal) {
					ROBIN_HOOD_TRACE(this)
						insert(first, last);
				}

				Table(std::initializer_list<value_type> initlist,
					size_t ROBIN_HOOD_UNUSED(bucket_count) /*unused*/ = 0, const Hash& h = Hash{},
					const KeyEqual& equal = KeyEqual{})
					: WHash(h)
					, WKeyEqual(equal) {
					ROBIN_HOOD_TRACE(this)
						insert(initlist.begin(), initlist.end());
				}

				Table(Table&& o) noexcept
					: WHash(std::move(static_cast<WHash&>(o)))
					, WKeyEqual(std::move(static_cast<WKeyEqual&>(o)))
					, DataPool(std::move(static_cast<DataPool&>(o))) {
					ROBIN_HOOD_TRACE(this)
						if (o.mMask) {
							mHashMultiplier = std::move(o.mHashMultiplier);
							mKeyVals = std::move(o.mKeyVals);
							mInfo = std::move(o.mInfo);
							mNumElements = std::move(o.mNumElements);
							mMask = std::move(o.mMask);
							mMaxNumElementsAllowed = std::move(o.mMaxNumElementsAllowed);
							mInfoInc = std::move(o.mInfoInc);
							mInfoHashShift = std::move(o.mInfoHashShift);
							// set other's mask to 0 so its destructor won't do anything
							o.init();
						}
				}

				Table& operator=(Table&& o) noexcept {
					ROBIN_HOOD_TRACE(this)
						if (&o != this) {
							if (o.mMask) {
								// only move stuff if the other map actually has some data
								destroy();
								mHashMultiplier = std::move(o.mHashMultiplier);
								mKeyVals = std::move(o.mKeyVals);
								mInfo = std::move(o.mInfo);
								mNumElements = std::move(o.mNumElements);
								mMask = std::move(o.mMask);
								mMaxNumElementsAllowed = std::move(o.mMaxNumElementsAllowed);
								mInfoInc = std::move(o.mInfoInc);
								mInfoHashShift = std::move(o.mInfoHashShift);
								WHash::operator=(std::move(static_cast<WHash&>(o)));
								WKeyEqual::operator=(std::move(static_cast<WKeyEqual&>(o)));
								DataPool::operator=(std::move(static_cast<DataPool&>(o)));

								o.init();

							}
							else {
								// nothing in the other map => just clear us.
								clear();
							}
						}
					return *this;
				}

				Table(const Table& o)
					: WHash(static_cast<const WHash&>(o))
					, WKeyEqual(static_cast<const WKeyEqual&>(o))
					, DataPool(static_cast<const DataPool&>(o)) {
					ROBIN_HOOD_TRACE(this)
						if (!o.empty()) {
							// not empty: create an exact copy. it is also possible to just iterate through all
							// elements and insert them, but copying is probably faster.

							auto const numElementsWithBuffer = calcNumElementsWithBuffer(o.mMask + 1);
							auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);

							ROBIN_HOOD_LOG("std::malloc " << numBytesTotal << " = calcNumBytesTotal("
								<< numElementsWithBuffer << ")")
								mHashMultiplier = o.mHashMultiplier;
							mKeyVals = static_cast<Node*>(
								detail::assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));
							// no need for calloc because clonData does memcpy
							mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
							mNumElements = o.mNumElements;
							mMask = o.mMask;
							mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
							mInfoInc = o.mInfoInc;
							mInfoHashShift = o.mInfoHashShift;
							cloneData(o);
						}
				}

				// Creates a copy of the given map. Copy constructor of each entry is used.
				// Not sure why clang-tidy thinks this doesn't handle self assignment, it does
				// NOLINTNEXTLINE(bugprone-unhandled-self-assignment,cert-oop54-cpp)
				Table& operator=(Table const& o) {
					ROBIN_HOOD_TRACE(this)
						if (&o == this) {
							// prevent assigning of itself
							return *this;
						}

					// we keep using the old allocator and not assign the new one, because we want to keep
					// the memory available. when it is the same size.
					if (o.empty()) {
						if (0 == mMask) {
							// nothing to do, we are empty too
							return *this;
						}

						// not empty: destroy what we have there
						// clear also resets mInfo to 0, that's sometimes not necessary.
						destroy();
						init();
						WHash::operator=(static_cast<const WHash&>(o));
						WKeyEqual::operator=(static_cast<const WKeyEqual&>(o));
						DataPool::operator=(static_cast<DataPool const&>(o));

						return *this;
					}

					// clean up old stuff
					Destroyer<Self, IsFlat&& std::is_trivially_destructible<Node>::value>{}.nodes(*this);

					if (mMask != o.mMask) {
						// no luck: we don't have the same array size allocated, so we need to realloc.
						if (0 != mMask) {
							// only deallocate if we actually have data!
							ROBIN_HOOD_LOG("std::free")
								std::free(mKeyVals);
						}

						auto const numElementsWithBuffer = calcNumElementsWithBuffer(o.mMask + 1);
						auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);
						ROBIN_HOOD_LOG("std::malloc " << numBytesTotal << " = calcNumBytesTotal("
							<< numElementsWithBuffer << ")")
							mKeyVals = static_cast<Node*>(
								detail::assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));

						// no need for calloc here because cloneData performs a memcpy.
						mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
						// sentinel is set in cloneData
					}
					WHash::operator=(static_cast<const WHash&>(o));
					WKeyEqual::operator=(static_cast<const WKeyEqual&>(o));
					DataPool::operator=(static_cast<DataPool const&>(o));
					mHashMultiplier = o.mHashMultiplier;
					mNumElements = o.mNumElements;
					mMask = o.mMask;
					mMaxNumElementsAllowed = o.mMaxNumElementsAllowed;
					mInfoInc = o.mInfoInc;
					mInfoHashShift = o.mInfoHashShift;
					cloneData(o);

					return *this;
				}

				// Swaps everything between the two maps.
				void swap(Table& o) {
					ROBIN_HOOD_TRACE(this)
						using std::swap;
					swap(o, *this);
				}

				// Clears all data, without resizing.
				void clear() {
					ROBIN_HOOD_TRACE(this)
						if (empty()) {
							// don't do anything! also important because we don't want to write to
							// DummyInfoByte::b, even though we would just write 0 to it.
							return;
						}

					Destroyer<Self, IsFlat&& std::is_trivially_destructible<Node>::value>{}.nodes(*this);

					auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);
					// clear everything, then set the sentinel again
					uint8_t const z = 0;
					std::fill(mInfo, mInfo + calcNumBytesInfo(numElementsWithBuffer), z);
					mInfo[numElementsWithBuffer] = 1;

					mInfoInc = InitialInfoInc;
					mInfoHashShift = InitialInfoHashShift;
				}

				// Destroys the map and all it's contents.
				~Table() {
					ROBIN_HOOD_TRACE(this)
						destroy();
				}

				// Checks if both tables contain the same entries. Order is irrelevant.
				bool operator==(const Table& other) const {
					ROBIN_HOOD_TRACE(this)
						if (other.size() != size()) {
							return false;
						}
					for (auto const& otherEntry : other) {
						if (!has(otherEntry)) {
							return false;
						}
					}

					return true;
				}

				bool operator!=(const Table& other) const {
					ROBIN_HOOD_TRACE(this)
						return !operator==(other);
				}

				template <typename Q = mapped_type>
				typename std::enable_if<!std::is_void<Q>::value, Q&>::type operator[](const key_type& key) {
					ROBIN_HOOD_TRACE(this)
						auto idxAndState = insertKeyPrepareEmptySpot(key);
					switch (idxAndState.second) {
					case InsertionState::key_found:
						break;

					case InsertionState::new_node:
						::new (static_cast<void*>(&mKeyVals[idxAndState.first]))
							Node(*this, std::piecewise_construct, std::forward_as_tuple(key),
								std::forward_as_tuple());
						break;

					case InsertionState::overwrite_node:
						mKeyVals[idxAndState.first] = Node(*this, std::piecewise_construct,
							std::forward_as_tuple(key), std::forward_as_tuple());
						break;

					case InsertionState::overflow_error:
						throwOverflowError();
					}

					return mKeyVals[idxAndState.first].getSecond();
				}

				template <typename Q = mapped_type>
				typename std::enable_if<!std::is_void<Q>::value, Q&>::type operator[](key_type&& key) {
					ROBIN_HOOD_TRACE(this)
						auto idxAndState = insertKeyPrepareEmptySpot(key);
					switch (idxAndState.second) {
					case InsertionState::key_found:
						break;

					case InsertionState::new_node:
						::new (static_cast<void*>(&mKeyVals[idxAndState.first]))
							Node(*this, std::piecewise_construct, std::forward_as_tuple(std::move(key)),
								std::forward_as_tuple());
						break;

					case InsertionState::overwrite_node:
						mKeyVals[idxAndState.first] =
							Node(*this, std::piecewise_construct, std::forward_as_tuple(std::move(key)),
								std::forward_as_tuple());
						break;

					case InsertionState::overflow_error:
						throwOverflowError();
					}

					return mKeyVals[idxAndState.first].getSecond();
				}

				template <typename Iter>
				void insert(Iter first, Iter last) {
					for (; first != last; ++first) {
						// value_type ctor needed because this might be called with std::pair's
						insert(value_type(*first));
					}
				}

				void insert(std::initializer_list<value_type> ilist) {
					for (auto&& vt : ilist) {
						insert(std::move(vt));
					}
				}

				template <typename... Args>
				std::pair<iterator, bool> emplace(Args&&... args) {
					ROBIN_HOOD_TRACE(this)
						Node n {
						*this, std::forward<Args>(args)...
					};
					auto idxAndState = insertKeyPrepareEmptySpot(getFirstConst(n));
					switch (idxAndState.second) {
					case InsertionState::key_found:
						n.destroy(*this);
						break;

					case InsertionState::new_node:
						::new (static_cast<void*>(&mKeyVals[idxAndState.first])) Node(*this, std::move(n));
						break;

					case InsertionState::overwrite_node:
						mKeyVals[idxAndState.first] = std::move(n);
						break;

					case InsertionState::overflow_error:
						n.destroy(*this);
						throwOverflowError();
						break;
					}

					return std::make_pair(iterator(mKeyVals + idxAndState.first, mInfo + idxAndState.first),
						InsertionState::key_found != idxAndState.second);
				}

				template <typename... Args>
				iterator emplace_hint(const_iterator position, Args&&... args) {
					(void)position;
					return emplace(std::forward<Args>(args)...).first;
				}

				template <typename... Args>
				std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
					return try_emplace_impl(key, std::forward<Args>(args)...);
				}

				template <typename... Args>
				std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
					return try_emplace_impl(std::move(key), std::forward<Args>(args)...);
				}

				template <typename... Args>
				iterator try_emplace(const_iterator hint, const key_type& key, Args&&... args) {
					(void)hint;
					return try_emplace_impl(key, std::forward<Args>(args)...).first;
				}

				template <typename... Args>
				iterator try_emplace(const_iterator hint, key_type&& key, Args&&... args) {
					(void)hint;
					return try_emplace_impl(std::move(key), std::forward<Args>(args)...).first;
				}

				template <typename Mapped>
				std::pair<iterator, bool> insert_or_assign(const key_type& key, Mapped&& obj) {
					return insertOrAssignImpl(key, std::forward<Mapped>(obj));
				}

				template <typename Mapped>
				std::pair<iterator, bool> insert_or_assign(key_type&& key, Mapped&& obj) {
					return insertOrAssignImpl(std::move(key), std::forward<Mapped>(obj));
				}

				template <typename Mapped>
				iterator insert_or_assign(const_iterator hint, const key_type& key, Mapped&& obj) {
					(void)hint;
					return insertOrAssignImpl(key, std::forward<Mapped>(obj)).first;
				}

				template <typename Mapped>
				iterator insert_or_assign(const_iterator hint, key_type&& key, Mapped&& obj) {
					(void)hint;
					return insertOrAssignImpl(std::move(key), std::forward<Mapped>(obj)).first;
				}

				std::pair<iterator, bool> insert(const value_type& keyval) {
					ROBIN_HOOD_TRACE(this)
						return emplace(keyval);
				}

				iterator insert(const_iterator hint, const value_type& keyval) {
					(void)hint;
					return emplace(keyval).first;
				}

				std::pair<iterator, bool> insert(value_type&& keyval) {
					return emplace(std::move(keyval));
				}

				iterator insert(const_iterator hint, value_type&& keyval) {
					(void)hint;
					return emplace(std::move(keyval)).first;
				}

				// Returns 1 if key is found, 0 otherwise.
				size_t count(const key_type& key) const { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						auto kv = mKeyVals + findIdx(key);
					if (kv != reinterpret_cast_no_cast_align_warning<Node*>(mInfo)) {
						return 1;
					}
					return 0;
				}

				template <typename OtherKey, typename Self_ = Self>
				// NOLINTNEXTLINE(modernize-use-nodiscard)
				typename std::enable_if<Self_::is_transparent, size_t>::type count(const OtherKey& key) const {
					ROBIN_HOOD_TRACE(this)
						auto kv = mKeyVals + findIdx(key);
					if (kv != reinterpret_cast_no_cast_align_warning<Node*>(mInfo)) {
						return 1;
					}
					return 0;
				}

				bool contains(const key_type& key) const { // NOLINT(modernize-use-nodiscard)
					return 1U == count(key);
				}

				template <typename OtherKey, typename Self_ = Self>
				// NOLINTNEXTLINE(modernize-use-nodiscard)
				typename std::enable_if<Self_::is_transparent, bool>::type contains(const OtherKey& key) const {
					return 1U == count(key);
				}

				// Returns a reference to the value found for key.
				// Throws std::out_of_range if element cannot be found
				template <typename Q = mapped_type>
				// NOLINTNEXTLINE(modernize-use-nodiscard)
				typename std::enable_if<!std::is_void<Q>::value, Q&>::type at(key_type const& key) {
					ROBIN_HOOD_TRACE(this)
						auto kv = mKeyVals + findIdx(key);
					if (kv == reinterpret_cast_no_cast_align_warning<Node*>(mInfo)) {
						doThrow<std::out_of_range>("key not found");
					}
					return kv->getSecond();
				}

				// Returns a reference to the value found for key.
				// Throws std::out_of_range if element cannot be found
				template <typename Q = mapped_type>
				// NOLINTNEXTLINE(modernize-use-nodiscard)
				typename std::enable_if<!std::is_void<Q>::value, Q const&>::type at(key_type const& key) const {
					ROBIN_HOOD_TRACE(this)
						auto kv = mKeyVals + findIdx(key);
					if (kv == reinterpret_cast_no_cast_align_warning<Node*>(mInfo)) {
						doThrow<std::out_of_range>("key not found");
					}
					return kv->getSecond();
				}

				const_iterator find(const key_type& key) const { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						const size_t idx = findIdx(key);
					return const_iterator{ mKeyVals + idx, mInfo + idx };
				}

				template <typename OtherKey>
				const_iterator find(const OtherKey& key, is_transparent_tag /*unused*/) const {
					ROBIN_HOOD_TRACE(this)
						const size_t idx = findIdx(key);
					return const_iterator{ mKeyVals + idx, mInfo + idx };
				}

				template <typename OtherKey, typename Self_ = Self>
				typename std::enable_if<Self_::is_transparent, // NOLINT(modernize-use-nodiscard)
					const_iterator>::type  // NOLINT(modernize-use-nodiscard)
					find(const OtherKey& key) const {              // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						const size_t idx = findIdx(key);
					return const_iterator{ mKeyVals + idx, mInfo + idx };
				}

				iterator find(const key_type& key) {
					ROBIN_HOOD_TRACE(this)
						const size_t idx = findIdx(key);
					return iterator{ mKeyVals + idx, mInfo + idx };
				}

				template <typename OtherKey>
				iterator find(const OtherKey& key, is_transparent_tag /*unused*/) {
					ROBIN_HOOD_TRACE(this)
						const size_t idx = findIdx(key);
					return iterator{ mKeyVals + idx, mInfo + idx };
				}

				template <typename OtherKey, typename Self_ = Self>
				typename std::enable_if<Self_::is_transparent, iterator>::type find(const OtherKey& key) {
					ROBIN_HOOD_TRACE(this)
						const size_t idx = findIdx(key);
					return iterator{ mKeyVals + idx, mInfo + idx };
				}

				iterator begin() {
					ROBIN_HOOD_TRACE(this)
						if (empty()) {
							return end();
						}
					return iterator(mKeyVals, mInfo, fast_forward_tag{});
				}
				const_iterator begin() const { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						return cbegin();
				}
				const_iterator cbegin() const { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						if (empty()) {
							return cend();
						}
					return const_iterator(mKeyVals, mInfo, fast_forward_tag{});
				}

				iterator end() {
					ROBIN_HOOD_TRACE(this)
						// no need to supply valid info pointer: end() must not be dereferenced, and only node
						// pointer is compared.
						return iterator{ reinterpret_cast_no_cast_align_warning<Node*>(mInfo), nullptr };
				}
				const_iterator end() const { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						return cend();
				}
				const_iterator cend() const { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						return const_iterator{ reinterpret_cast_no_cast_align_warning<Node*>(mInfo), nullptr };
				}

				iterator erase(const_iterator pos) {
					ROBIN_HOOD_TRACE(this)
						// its safe to perform const cast here
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
						return erase(iterator{ const_cast<Node*>(pos.mKeyVals), const_cast<uint8_t*>(pos.mInfo) });
				}

				// Erases element at pos, returns iterator to the next element.
				iterator erase(iterator pos) {
					ROBIN_HOOD_TRACE(this)
						// we assume that pos always points to a valid entry, and not end().
						auto const idx = static_cast<size_t>(pos.mKeyVals - mKeyVals);

					shiftDown(idx);
					--mNumElements;

					if (*pos.mInfo) {
						// we've backward shifted, return this again
						return pos;
					}

					// no backward shift, return next element
					return ++pos;
				}

				size_t erase(const key_type& key) {
					ROBIN_HOOD_TRACE(this)
						size_t idx {};
					InfoType info{};
					keyToIdx(key, &idx, &info);

					// check while info matches with the source idx
					do {
						if (info == mInfo[idx] && WKeyEqual::operator()(key, mKeyVals[idx].getFirst())) {
							shiftDown(idx);
							--mNumElements;
							return 1;
						}
						next(&info, &idx);
					} while (info <= mInfo[idx]);

					// nothing found to delete
					return 0;
				}

				// reserves space for the specified number of elements. Makes sure the old data fits.
				// exactly the same as reserve(c).
				void rehash(size_t c) {
					// forces a reserve
					reserve(c, true);
				}

				// reserves space for the specified number of elements. Makes sure the old data fits.
				// Exactly the same as rehash(c). Use rehash(0) to shrink to fit.
				void reserve(size_t c) {
					// reserve, but don't force rehash
					reserve(c, false);
				}

				// If possible reallocates the map to a smaller one. This frees the underlying table.
				// Does not do anything if load_factor is too large for decreasing the table's size.
				void compact() {
					ROBIN_HOOD_TRACE(this)
						auto newSize = InitialNumElements;
					while (calcMaxNumElementsAllowed(newSize) < mNumElements && newSize != 0) {
						newSize *= 2;
					}
					if (ROBIN_HOOD_UNLIKELY(newSize == 0)) {
						throwOverflowError();
					}

					ROBIN_HOOD_LOG("newSize > mMask + 1: " << newSize << " > " << mMask << " + 1")

						// only actually do anything when the new size is bigger than the old one. This prevents to
						// continuously allocate for each reserve() call.
						if (newSize < mMask + 1) {
							rehashPowerOfTwo(newSize, true);
						}
				}

				size_type size() const noexcept { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						return mNumElements;
				}

				size_type max_size() const noexcept { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						return static_cast<size_type>(-1);
				}

				ROBIN_HOOD(NODISCARD) bool empty() const noexcept {
					ROBIN_HOOD_TRACE(this)
						return 0 == mNumElements;
				}

				float max_load_factor() const noexcept { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						return MaxLoadFactor100 / 100.0F;
				}

				// Average number of elements per bucket. Since we allow only 1 per bucket
				float load_factor() const noexcept { // NOLINT(modernize-use-nodiscard)
					ROBIN_HOOD_TRACE(this)
						return static_cast<float>(size()) / static_cast<float>(mMask + 1);
				}

				ROBIN_HOOD(NODISCARD) size_t mask() const noexcept {
					ROBIN_HOOD_TRACE(this)
						return mMask;
				}

				ROBIN_HOOD(NODISCARD) size_t calcMaxNumElementsAllowed(size_t maxElements) const noexcept {
					if (ROBIN_HOOD_LIKELY(maxElements <= (std::numeric_limits<size_t>::max)() / 100)) {
						return maxElements * MaxLoadFactor100 / 100;
					}

					// we might be a bit inprecise, but since maxElements is quite large that doesn't matter
					return (maxElements / 100) * MaxLoadFactor100;
				}

				ROBIN_HOOD(NODISCARD) size_t calcNumBytesInfo(size_t numElements) const noexcept {
					// we add a uint64_t, which houses the sentinel (first byte) and padding so we can load
					// 64bit types.
					return numElements + sizeof(uint64_t);
				}

				ROBIN_HOOD(NODISCARD)
					size_t calcNumElementsWithBuffer(size_t numElements) const noexcept {
					auto maxNumElementsAllowed = calcMaxNumElementsAllowed(numElements);
					return numElements + (std::min)(maxNumElementsAllowed, (static_cast<size_t>(0xFF)));
				}

				// calculation only allowed for 2^n values
				ROBIN_HOOD(NODISCARD) size_t calcNumBytesTotal(size_t numElements) const {
#if ROBIN_HOOD(BITNESS) == 64
					return numElements * sizeof(Node) + calcNumBytesInfo(numElements);
#else
					// make sure we're doing 64bit operations, so we are at least safe against 32bit overflows.
					auto const ne = static_cast<uint64_t>(numElements);
					auto const s = static_cast<uint64_t>(sizeof(Node));
					auto const infos = static_cast<uint64_t>(calcNumBytesInfo(numElements));

					auto const total64 = ne * s + infos;
					auto const total = static_cast<size_t>(total64);

					if (ROBIN_HOOD_UNLIKELY(static_cast<uint64_t>(total) != total64)) {
						throwOverflowError();
					}
					return total;
#endif
				}

			private:
				template <typename Q = mapped_type>
				ROBIN_HOOD(NODISCARD)
					typename std::enable_if<!std::is_void<Q>::value, bool>::type has(const value_type& e) const {
					ROBIN_HOOD_TRACE(this)
						auto it = find(e.first);
					return it != end() && it->second == e.second;
				}

				template <typename Q = mapped_type>
				ROBIN_HOOD(NODISCARD)
					typename std::enable_if<std::is_void<Q>::value, bool>::type has(const value_type& e) const {
					ROBIN_HOOD_TRACE(this)
						return find(e) != end();
				}

				void reserve(size_t c, bool forceRehash) {
					ROBIN_HOOD_TRACE(this)
						auto const minElementsAllowed = (std::max)(c, mNumElements);
					auto newSize = InitialNumElements;
					while (calcMaxNumElementsAllowed(newSize) < minElementsAllowed && newSize != 0) {
						newSize *= 2;
					}
					if (ROBIN_HOOD_UNLIKELY(newSize == 0)) {
						throwOverflowError();
					}

					ROBIN_HOOD_LOG("newSize > mMask + 1: " << newSize << " > " << mMask << " + 1")

						// only actually do anything when the new size is bigger than the old one. This prevents to
						// continuously allocate for each reserve() call.
						if (forceRehash || newSize > mMask + 1) {
							rehashPowerOfTwo(newSize, false);
						}
				}

				// reserves space for at least the specified number of elements.
				// only works if numBuckets if power of two
				// True on success, false otherwise
				void rehashPowerOfTwo(size_t numBuckets, bool forceFree) {
					ROBIN_HOOD_TRACE(this)

						Node* const oldKeyVals = mKeyVals;
					uint8_t const* const oldInfo = mInfo;

					const size_t oldMaxElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);

					// resize operation: move stuff
					initData(numBuckets);
					if (oldMaxElementsWithBuffer > 1) {
						for (size_t i = 0; i < oldMaxElementsWithBuffer; ++i) {
							if (oldInfo[i] != 0) {
								// might throw an exception, which is really bad since we are in the middle of
								// moving stuff.
								insert_move(std::move(oldKeyVals[i]));
								// destroy the node but DON'T destroy the data.
								oldKeyVals[i].~Node();
							}
						}

						// this check is not necessary as it's guarded by the previous if, but it helps
						// silence g++'s overeager "attempt to free a non-heap object 'map'
						// [-Werror=free-nonheap-object]" warning.
						if (oldKeyVals != reinterpret_cast_no_cast_align_warning<Node*>(&mMask)) {
							// don't destroy old data: put it into the pool instead
							if (forceFree) {
								std::free(oldKeyVals);
							}
							else {
								DataPool::addOrFree(oldKeyVals, calcNumBytesTotal(oldMaxElementsWithBuffer));
							}
						}
					}
				}

				ROBIN_HOOD(NOINLINE) void throwOverflowError() const {
#if ROBIN_HOOD(HAS_EXCEPTIONS)
					throw std::overflow_error("robin_hood::map overflow");
#else
					abort();
#endif
				}

				template <typename OtherKey, typename... Args>
				std::pair<iterator, bool> try_emplace_impl(OtherKey&& key, Args&&... args) {
					ROBIN_HOOD_TRACE(this)
						auto idxAndState = insertKeyPrepareEmptySpot(key);
					switch (idxAndState.second) {
					case InsertionState::key_found:
						break;

					case InsertionState::new_node:
						::new (static_cast<void*>(&mKeyVals[idxAndState.first])) Node(
							*this, std::piecewise_construct, std::forward_as_tuple(std::forward<OtherKey>(key)),
							std::forward_as_tuple(std::forward<Args>(args)...));
						break;

					case InsertionState::overwrite_node:
						mKeyVals[idxAndState.first] = Node(*this, std::piecewise_construct,
							std::forward_as_tuple(std::forward<OtherKey>(key)),
							std::forward_as_tuple(std::forward<Args>(args)...));
						break;

					case InsertionState::overflow_error:
						throwOverflowError();
						break;
					}

					return std::make_pair(iterator(mKeyVals + idxAndState.first, mInfo + idxAndState.first),
						InsertionState::key_found != idxAndState.second);
				}

				template <typename OtherKey, typename Mapped>
				std::pair<iterator, bool> insertOrAssignImpl(OtherKey&& key, Mapped&& obj) {
					ROBIN_HOOD_TRACE(this)
						auto idxAndState = insertKeyPrepareEmptySpot(key);
					switch (idxAndState.second) {
					case InsertionState::key_found:
						mKeyVals[idxAndState.first].getSecond() = std::forward<Mapped>(obj);
						break;

					case InsertionState::new_node:
						::new (static_cast<void*>(&mKeyVals[idxAndState.first])) Node(
							*this, std::piecewise_construct, std::forward_as_tuple(std::forward<OtherKey>(key)),
							std::forward_as_tuple(std::forward<Mapped>(obj)));
						break;

					case InsertionState::overwrite_node:
						mKeyVals[idxAndState.first] = Node(*this, std::piecewise_construct,
							std::forward_as_tuple(std::forward<OtherKey>(key)),
							std::forward_as_tuple(std::forward<Mapped>(obj)));
						break;

					case InsertionState::overflow_error:
						throwOverflowError();
						break;
					}

					return std::make_pair(iterator(mKeyVals + idxAndState.first, mInfo + idxAndState.first),
						InsertionState::key_found != idxAndState.second);
				}

				void initData(size_t max_elements) {
					mNumElements = 0;
					mMask = max_elements - 1;
					mMaxNumElementsAllowed = calcMaxNumElementsAllowed(max_elements);

					auto const numElementsWithBuffer = calcNumElementsWithBuffer(max_elements);

					// malloc & zero mInfo. Faster than calloc everything.
					auto const numBytesTotal = calcNumBytesTotal(numElementsWithBuffer);
					ROBIN_HOOD_LOG("std::calloc " << numBytesTotal << " = calcNumBytesTotal("
						<< numElementsWithBuffer << ")")
						mKeyVals = reinterpret_cast<Node*>(
							detail::assertNotNull<std::bad_alloc>(std::malloc(numBytesTotal)));
					mInfo = reinterpret_cast<uint8_t*>(mKeyVals + numElementsWithBuffer);
					std::memset(mInfo, 0, numBytesTotal - numElementsWithBuffer * sizeof(Node));

					// set sentinel
					mInfo[numElementsWithBuffer] = 1;

					mInfoInc = InitialInfoInc;
					mInfoHashShift = InitialInfoHashShift;
				}

				enum class InsertionState { overflow_error, key_found, new_node, overwrite_node };

				// Finds key, and if not already present prepares a spot where to pot the key & value.
				// This potentially shifts nodes out of the way, updates mInfo and number of inserted
				// elements, so the only operation left to do is create/assign a new node at that spot.
				template <typename OtherKey>
				std::pair<size_t, InsertionState> insertKeyPrepareEmptySpot(OtherKey&& key) {
					for (int i = 0; i < 256; ++i) {
						size_t idx{};
						InfoType info{};
						keyToIdx(key, &idx, &info);
						nextWhileLess(&info, &idx);

						// while we potentially have a match
						while (info == mInfo[idx]) {
							if (WKeyEqual::operator()(key, mKeyVals[idx].getFirst())) {
								// key already exists, do NOT insert.
								// see http://en.cppreference.com/w/cpp/container/unordered_map/insert
								return std::make_pair(idx, InsertionState::key_found);
							}
							next(&info, &idx);
						}

						// unlikely that this evaluates to true
						if (ROBIN_HOOD_UNLIKELY(mNumElements >= mMaxNumElementsAllowed)) {
							if (!increase_size()) {
								return std::make_pair(size_t(0), InsertionState::overflow_error);
							}
							continue;
						}

						// key not found, so we are now exactly where we want to insert it.
						auto const insertion_idx = idx;
						auto const insertion_info = info;
						if (ROBIN_HOOD_UNLIKELY(insertion_info + mInfoInc > 0xFF)) {
							mMaxNumElementsAllowed = 0;
						}

						// find an empty spot
						while (0 != mInfo[idx]) {
							next(&info, &idx);
						}

						if (idx != insertion_idx) {
							shiftUp(idx, insertion_idx);
						}
						// put at empty spot
						mInfo[insertion_idx] = static_cast<uint8_t>(insertion_info);
						++mNumElements;
						return std::make_pair(insertion_idx, idx == insertion_idx
							? InsertionState::new_node
							: InsertionState::overwrite_node);
					}

					// enough attempts failed, so finally give up.
					return std::make_pair(size_t(0), InsertionState::overflow_error);
				}

				bool try_increase_info() {
					ROBIN_HOOD_LOG("mInfoInc=" << mInfoInc << ", numElements=" << mNumElements
						<< ", maxNumElementsAllowed="
						<< calcMaxNumElementsAllowed(mMask + 1))
						if (mInfoInc <= 2) {
							// need to be > 2 so that shift works (otherwise undefined behavior!)
							return false;
						}
					// we got space left, try to make info smaller
					mInfoInc = static_cast<uint8_t>(mInfoInc >> 1U);

					// remove one bit of the hash, leaving more space for the distance info.
					// This is extremely fast because we can operate on 8 bytes at once.
					++mInfoHashShift;
					auto const numElementsWithBuffer = calcNumElementsWithBuffer(mMask + 1);

					for (size_t i = 0; i < numElementsWithBuffer; i += 8) {
						auto val = unaligned_load<uint64_t>(mInfo + i);
						val = (val >> 1U) & UINT64_C(0x7f7f7f7f7f7f7f7f);
						std::memcpy(mInfo + i, &val, sizeof(val));
					}
					// update sentinel, which might have been cleared out!
					mInfo[numElementsWithBuffer] = 1;

					mMaxNumElementsAllowed = calcMaxNumElementsAllowed(mMask + 1);
					return true;
				}

				// True if resize was possible, false otherwise
				bool increase_size() {
					// nothing allocated yet? just allocate InitialNumElements
					if (0 == mMask) {
						initData(InitialNumElements);
						return true;
					}

					auto const maxNumElementsAllowed = calcMaxNumElementsAllowed(mMask + 1);
					if (mNumElements < maxNumElementsAllowed && try_increase_info()) {
						return true;
					}

					ROBIN_HOOD_LOG("mNumElements=" << mNumElements << ", maxNumElementsAllowed="
						<< maxNumElementsAllowed << ", load="
						<< (static_cast<double>(mNumElements) * 100.0 /
							(static_cast<double>(mMask) + 1)))

						if (mNumElements * 2 < calcMaxNumElementsAllowed(mMask + 1)) {
							// we have to resize, even though there would still be plenty of space left!
							// Try to rehash instead. Delete freed memory so we don't steadyily increase mem in case
							// we have to rehash a few times
							nextHashMultiplier();
							rehashPowerOfTwo(mMask + 1, true);
						}
						else {
							// we've reached the capacity of the map, so the hash seems to work nice. Keep using it.
							rehashPowerOfTwo((mMask + 1) * 2, false);
						}
					return true;
				}

				void nextHashMultiplier() {
					// adding an *even* number, so that the multiplier will always stay odd. This is necessary
					// so that the hash stays a mixing function (and thus doesn't have any information loss).
					mHashMultiplier += UINT64_C(0xc4ceb9fe1a85ec54);
				}

				void destroy() {
					if (0 == mMask) {
						// don't deallocate!
						return;
					}

					Destroyer<Self, IsFlat&& std::is_trivially_destructible<Node>::value>{}
					.nodesDoNotDeallocate(*this);

					// This protection against not deleting mMask shouldn't be needed as it's sufficiently
					// protected with the 0==mMask check, but I have this anyways because g++ 7 otherwise
					// reports a compile error: attempt to free a non-heap object 'fm'
					// [-Werror=free-nonheap-object]
					if (mKeyVals != reinterpret_cast_no_cast_align_warning<Node*>(&mMask)) {
						ROBIN_HOOD_LOG("std::free")
							std::free(mKeyVals);
					}
				}

				void init() noexcept {
					mKeyVals = reinterpret_cast_no_cast_align_warning<Node*>(&mMask);
					mInfo = reinterpret_cast<uint8_t*>(&mMask);
					mNumElements = 0;
					mMask = 0;
					mMaxNumElementsAllowed = 0;
					mInfoInc = InitialInfoInc;
					mInfoHashShift = InitialInfoHashShift;
				}

				// members are sorted so no padding occurs
				uint64_t mHashMultiplier = UINT64_C(0xc4ceb9fe1a85ec53);                // 8 byte  8
				Node* mKeyVals = reinterpret_cast_no_cast_align_warning<Node*>(&mMask); // 8 byte 16
				uint8_t* mInfo = reinterpret_cast<uint8_t*>(&mMask);                    // 8 byte 24
				size_t mNumElements = 0;                                                // 8 byte 32
				size_t mMask = 0;                                                       // 8 byte 40
				size_t mMaxNumElementsAllowed = 0;                                      // 8 byte 48
				InfoType mInfoInc = InitialInfoInc;                                     // 4 byte 52
				InfoType mInfoHashShift = InitialInfoHashShift;                         // 4 byte 56
																// 16 byte 56 if NodeAllocator
		};

	} // namespace detail

	// map

	template <typename Key, typename T, typename Hash = hash<Key>,
		typename KeyEqual = std::equal_to<Key>, size_t MaxLoadFactor100 = 80>
		using unordered_flat_map = detail::Table<true, MaxLoadFactor100, Key, T, Hash, KeyEqual>;

	template <typename Key, typename T, typename Hash = hash<Key>,
		typename KeyEqual = std::equal_to<Key>, size_t MaxLoadFactor100 = 80>
		using unordered_node_map = detail::Table<false, MaxLoadFactor100, Key, T, Hash, KeyEqual>;

	template <typename Key, typename T, typename Hash = hash<Key>,
		typename KeyEqual = std::equal_to<Key>, size_t MaxLoadFactor100 = 80>
		using unordered_map =
		detail::Table<sizeof(robin_hood::pair<Key, T>) <= sizeof(size_t) * 6 &&
		std::is_nothrow_move_constructible<robin_hood::pair<Key, T>>::value &&
		std::is_nothrow_move_assignable<robin_hood::pair<Key, T>>::value,
		MaxLoadFactor100, Key, T, Hash, KeyEqual>;

	// set

	template <typename Key, typename Hash = hash<Key>, typename KeyEqual = std::equal_to<Key>,
		size_t MaxLoadFactor100 = 80>
		using unordered_flat_set = detail::Table<true, MaxLoadFactor100, Key, void, Hash, KeyEqual>;

	template <typename Key, typename Hash = hash<Key>, typename KeyEqual = std::equal_to<Key>,
		size_t MaxLoadFactor100 = 80>
		using unordered_node_set = detail::Table<false, MaxLoadFactor100, Key, void, Hash, KeyEqual>;

	template <typename Key, typename Hash = hash<Key>, typename KeyEqual = std::equal_to<Key>,
		size_t MaxLoadFactor100 = 80>
		using unordered_set = detail::Table<sizeof(Key) <= sizeof(size_t) * 6 &&
		std::is_nothrow_move_constructible<Key>::value &&
		std::is_nothrow_move_assignable<Key>::value,
		MaxLoadFactor100, Key, void, Hash, KeyEqual>;

} // namespace robin_hood

#endif

#pragma endregion

#endif
#pragma endregion

template< typename _type_, int _size_ = 16 >
class cweeUnorderedList;

template<typename T>
class cweeUnorderedListReferenceObject {
public:
	typedef T		Type;
	cweeUnorderedListReferenceObject() {};
	cweeUnorderedListReferenceObject(const cweeSharedPtr < cweeUnorderedList< T > > sourceList, int itemIndex);

	template<typename Q, typename = std::enable_if_t<!std::is_same_v<T, Q>> > cweeUnorderedListReferenceObject(const cweeSharedPtr < cweeUnorderedList< Q > > sourceList, int itemIndex) {
		// static_assert(typeid(Q) == typeid(cweeAny));

		which = itemIndex;
		anyItems = sourceList;

		if (auto shared = anyItems) {
			shared->PreventDeletion(which);
			shared->Lock();
			cweeAny* ptr = shared->UnsafeRead(which);
			shared->Unlock();
			if (ptr) {
				item = ptr->cast<T*>();
			}
		}
	};
	~cweeUnorderedListReferenceObject();
	cweeUnorderedListReferenceObject(const cweeUnorderedListReferenceObject& other);
	cweeUnorderedListReferenceObject(cweeUnorderedListReferenceObject&& other);

	cweeUnorderedListReferenceObject<T>& operator=(const cweeUnorderedListReferenceObject<T>& other);

	cweeUnorderedListReferenceObject<T>& Take(cweeUnorderedListReferenceObject&& other) {
		// assumes that the 'other' has already prevented deletion - we are not going to re-lock, but instead just take ownership away
		if (items) items->AllowDeletion(which);
		if (anyItems) anyItems->AllowDeletion(which);

		which = other.which;
		items = other.items;
		anyItems = other.anyItems;
		item = other.item;

		other.which = 0;
		other.items = nullptr;
		other.anyItems = nullptr;
		other.item = nullptr;

		return *this;
	};

	void SwapPtrs(T* ptr, int index) {
		if (items) {
			items->AllowDeletion(which);
			items->PreventDeletion(index);
		}
		if (anyItems) {
			anyItems->AllowDeletion(which);
			anyItems->PreventDeletion(index);
		}

		which = index;
		item = ptr;
	};

	void Clear();

	const int Index() const;

	const T* operator->() const;
	T* operator->();
	T& operator*() {
		return *operator->();
	};
	T* Ptr();
	const T* Ptr() const;
	const T& operator*() const {
		return *operator->();
	};
	cweeUnorderedListReferenceObject<T>& operator=(const T& input);	

	template<typename Q> cweeUnorderedListReferenceObject<T>& operator=(const Q& input) {
		if (item) {
			*item = input;
		}
		return *this;
	};

	explicit operator bool();
	explicit operator bool() const;

	operator T();
	operator T() const;

protected:
	int which = -1;
	cweeSharedPtr < cweeUnorderedList< T > > items;//  = nullptr;
	cweeSharedPtr < cweeUnorderedList< cweeAny > > anyItems;// = nullptr;
	T* item = nullptr;
};

template<typename T>
class cweeUnorderedListManagedObject {
public:
	typedef T		Type;

	cweeUnorderedListManagedObject();
	cweeUnorderedListManagedObject(const cweeUnorderedList< T >* sourceList, int itemIndex) {
		which = itemIndex;
		items = const_cast<cweeUnorderedList<T>*>(sourceList);

		items->PreventDeletion(which);
		items->Lock();
		item = items->UnsafeRead(which);
		items->Unlock();
	};
	~cweeUnorderedListManagedObject();
	cweeUnorderedListManagedObject(const cweeUnorderedListManagedObject& other);
	cweeUnorderedListManagedObject(cweeUnorderedListManagedObject&& other);

	cweeUnorderedListManagedObject<T>& operator=(const cweeUnorderedListManagedObject& other);

	cweeUnorderedListManagedObject<T>& Take(cweeUnorderedListManagedObject&& other);

	void SwapPtrs(T* ptr, int index);

	void Clear();

	const int Index() const;

	const T* operator->() const;
	T* operator->();
	T& operator*() {
		return *operator->();
	};
	T* Ptr();
	const T* Ptr() const;
	const T& operator*() const {
		return *operator->();
	};
	cweeUnorderedListManagedObject<T>& operator=(const T& input);
	template<typename Q> cweeUnorderedListManagedObject<T>& operator=(const Q& input) {
		if (item) {
			*item = input;
		}
		return *this;
	};

	explicit operator bool();
	explicit operator bool() const;

	operator T();
	operator T() const;

protected:
	int which;
	cweeUnorderedList< T >* items;
	T* item;
};

#pragma region "CWEEUNORDEREDLIST CLASS DECLARATIONS BASED ON SELECTED OPTIONS"
/*!
Thread-safe list that performs garbage collection and manages read/write/create/delete operations on data. Intended to act as a multi-threaded database.
*/
template< typename _type_, int _size_ >
class cweeUnorderedList {
public:
	typedef _type_		Type;

	typedef cweeUnorderedListManagedObject<_type_> _iterType;

	struct it_state {
		mutable cweeThreadedList<int>	listOfIndexes;
		mutable int pos = 0;
		mutable _iterType iter;
		inline void begin(const cweeUnorderedList* ref) { listOfIndexes = ref->GetList(); pos = 0; }
		inline void next(const cweeUnorderedList* ref) { ++pos; }
		inline void end(const cweeUnorderedList* ref) { listOfIndexes = ref->GetList(); pos = listOfIndexes.Num(); }
		inline _iterType& get(cweeUnorderedList* ref) { iter = ref->GetPtr(listOfIndexes[pos]); return iter; }
		inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
		inline long long distance(const it_state& s) const { return pos - s.pos; };
		// Optional to allow operator--() and reverse iterators:
		inline void prev(const cweeUnorderedList* ref) { --pos; }
		// Optional to allow `const_iterator`:
		inline const _iterType& get(const cweeUnorderedList* ref) const { iter = ref->GetPtr(listOfIndexes[pos]); return iter; }
	};
	SETUP_STL_ITERATOR(cweeUnorderedList, _iterType, it_state);

	/*!
	Default constructor
	*/
	cweeUnorderedList() : lock(), num(0), allocator(), list(), indexList(), list_version(0), lastSearchID(-1), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1), protected_indexes(), assetNameToIndexMap(), eraseQueue() {
		indexList.Lock();
		indexList.UnsafeRead()->SetGranularity(16);
		indexList.Unlock();
	};

	/*!
	Default copy
	*/
	cweeUnorderedList(const cweeUnorderedList& other) : lock(), num(0), allocator(), list(), indexList(), list_version(0), lastSearchID(-1), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1), protected_indexes(), assetNameToIndexMap(), eraseQueue() {
		this->Copy(other); // instantiate by copying provided object
	};

	cweeUnorderedList(cweeUnorderedList&& other) : lock(), num(0), allocator(), list(), indexList(), list_version(0), lastSearchID(-1), lastResult(list.end()), lastVersion(-1), CreatedListVersion(0), lastCreatedListVersion(-1), protected_indexes(), assetNameToIndexMap(), eraseQueue() {
		this->Copy(std::forward<cweeUnorderedList<_type_, _size_>>(other)); // instantiate by copying provided object
	};

	/*!
	Default copy
	*/
	cweeUnorderedList<_type_, _size_>& operator=(const cweeUnorderedList<_type_, _size_>& other);

	/*!
	Default destructor
	*/
	~cweeUnorderedList();

	/*!
	Convert UnorderedList to a list of keys
	*/
	operator cweeThreadedList<int>();

	/*!
	Convert UnorderedList to a list of keys
	*/
	operator cweeThreadedList<int>() const;

	/*!
	Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
	*/
	_type_			operator[](int index) const;

	/*!
	Get a copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
	*/
	_type_			operator[](int index);

	/*!
	Allocate memory for object in heap and retrieve key to later access and edit the object.
	*/
	int				Append(void);

	/*!
	Allocate memory for object in heap and retrieve key to later access and edit the object.
	*/
	template <typename otherType>
	int				Append(const otherType& obj) {
		SharedLock();
		list_version.Increment();
		CreatedListVersion.Increment();
		int which;
		/* Get index for new allocation */
		which = num.Increment();
		/* Allocate a _type_ and store at specified index */
#ifdef useAllocatedMemory
		allocator.Lock();
		_type_* allocatedObj = allocator.UnsafeRead()->Alloc();
		allocator.Unlock();
#else
		_type_* allocatedObj = new _type_;
#endif

		*allocatedObj = obj;

#ifdef useGranularity
		if (granularity <= num)
		{
			granularity = useGranularity * granularity.GetValue();
			list.reserve(granularity.GetValue());
		}
#endif

		if (which >= 0) list[which] = allocatedObj;
		else {
#ifdef useAllocatedMemory
			allocator.Lock();
			allocator.UnsafeRead()->Free(allocatedObj);
			allocator.Unlock();
#else
			delete allocatedObj;
#endif
		}

		SharedUnlock();
		return which;
	};

	/*!
	Swap if the object exists, otherwise append at the index and then swap.
	*/
	void			Emplace(int index, const _type_& obj);

	/*!
	Get a copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
	*/
	_type_			Read(int index, const bool threadSafePtr = false);

	/*!
	Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
	*/
	_type_			Read(int index, const bool threadSafePtr = false) const;

	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeUnorderedList lifetime does.  */
	cweeUnorderedListManagedObject<_type_>	GetPtr(int index);

	/*! Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the udnerlying cweeUnorderedList lifetime does.  */
	cweeUnorderedListManagedObject<_type_>	GetPtr(int index) const;

	/*!
	True/False if the index exists on the heap
	*/
	bool			Check(int index);

	/*!
	Clear the current list and create a copy of the incoming list.
	*/
	void			Copy(const cweeUnorderedList<_type_, _size_>& copy, const bool threadSafePtr = false);

	/*!
	Copy the _type_ object from the stack into the heap at the index specified. If the index does not exist on the heap, nothing happens.
	*/
	void			Swap(int index, const _type_& replacement);

	/*!
	get the number of objects on the heap managed by this list
	*/
	int				Num(void) const;

	/*!
	get the number of objects on the heap managed by this list
	*/
	int				UnsafeNum(void) const;

	/*!
	get a list of all indexes currently on the heap that are currently valid
	*/
	cweeThreadedList<int> GetList(void) const {
		cweeThreadedList<int> out;

		if (lastCreatedListVersion == CreatedListVersion) {
			indexList.Lock();
			out = *indexList.UnsafeRead();
			indexList.Unlock();
			return out;
		}
		int size = Num();

		//SharedLock();
		//SharedUnlock(); // forces the list to erase if needed

		SharedLock();
		//eraseQueue.Lock();
		indexList.Lock();
		indexList.UnsafeRead()->Clear();
		indexList.UnsafeRead()->SetGranularity(size + 16);
		for (auto& kv : list) {
			//if (eraseQueue.UnsafeRead()->FindIndex(kv.first) < 0) {
			indexList.UnsafeRead()->Append(kv.first);
			//}		
		}
		lastCreatedListVersion = CreatedListVersion;
		indexList.Unlock();
		//eraseQueue.Unlock();

		indexList.Lock();
		out = *indexList.UnsafeRead();
		indexList.Unlock();

		SharedUnlock();
		return out;
	};

	/*!
	Erase the indexed object from the heap and free the memory for the list to re-use.
	*/
	void			Erase(int index);

	/*!
	Clear the current list and free all memory back to the operating system
	*/
	void			Clear(void) {
#if 1
		Lock();

		auto List = UnsafeList();
		for (auto& it : List) {
			UnsafeErase(it);
		}

		{
			eraseQueue.Lock();
			if (eraseQueue.UnsafeRead()->Num() > 0) {
				protected_indexes.Lock();
				for (int i = 0; i < eraseQueue.UnsafeRead()->Num(); i++) {
					auto& x = eraseQueue.UnsafeRead()->operator[](i);
					if (protected_indexes.UnsafeRead()->operator[](x).GetValue() <= 0) {
						// do erasure
						if (list.count(x) != 0) {
							list_version.Increment();
							CreatedListVersion.Increment();
							auto it = list.find(x);
							if (it != list.end()) {
#ifdef useAllocatedMemory
								allocator.Lock();
								if ((_type_*)it->second) {
									// ((cweeBlockAlloc < _type_, _size_, TAG_LIST >*)allocator.UnsafeRead())->Free((_type_*)it->second);
									allocator.UnsafeRead()->Free((_type_*)it->second);
								}
								allocator.Unlock();
#else
								delete it->second;
#endif
#ifndef useRobinMap
								it->second = nullptr;
								list.erase(it->first);
#else
								list.erase(it->first);
#endif
							}
						}

						eraseQueue.UnsafeRead()->RemoveIndexFast(i); // no longer need to erase
						i--; // stay here
					}
				}
				protected_indexes.Unlock();
			}
			eraseQueue.Unlock();
		}

#ifdef useAllocatedMemory
		allocator.Lock();
		allocator.UnsafeRead()->FreeEmptyBlocks();
		allocator.Unlock();
#endif
		num = 0;

		list_version = 0;
		CreatedListVersion = 0;

		indexList = cweeThreadedList<int>();
		lastSearchID = -1;
		typedef decltype(list) listType;
		list = listType();
		lastResult = list.end();
		lastVersion = -1;
		lastCreatedListVersion = -1;

		Unlock();

#else

		bool succeeded = false;
		do {
			Lock();
			{
				bool allowed = true;
				std::vector<int> intList = UnsafeList();
				for (auto it : intList) if (CheckCanDeleteIndex(it) == false) allowed = false;
				if (allowed == true) {
					succeeded = true;

					for (auto it : intList) {
						AllowDeletion(it);
						UnsafeErase(it);
					}

#ifdef useAllocatedMemory
					allocator.Lock();
					allocator.UnsafeRead()->FreeEmptyBlocks();
					allocator.Unlock();
#endif

					num = 0;
					list_version = 0;
					CreatedListVersion = 0;

					indexList = UnsafeList();
					lastSearchID = -1;
					lastResult = list.end();
					lastVersion = -1;
					lastCreatedListVersion = -1;



				}
			}
			Unlock();
		} while (succeeded == false);


#endif
	};

	/*!
	Get approximate number of bytes referenced by this list.
	*/
	const size_t	BytesUsed(void);

	/*!
	Iterates through list to find the member with the matching "::Name" member. If no item exists with this member, returns -1. If found, returns the index of the member.
	This is a utility function, and is not going to give great performance compared to other
	*/
	const int		FindIndexWithName(const cweeStr& name) const;

	/*!
	Iterates through the list to return the list of ::Name members. May return empty if the object of this UnorderedList has no member named ::Name
	*/
	cweeThreadedList<cweeStr> GetNames() const;

	/*!
	Get the name of the object at the specified index. May return empty if the object of this UnorderedList has no member named ::Name
	*/
	cweeStr			GetName(int index) const;

	/*!
	prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock.
	A "Unlock" must be called to re-enable access to the list.
	*/
	void			Lock(void) const;

	/*!
	Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior.
	*/
	void			Unlock(void) const;

	/*!
	After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping.
	*/
	_type_* UnsafeRead(int index) const;

	/*!
	After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list.
	*/
	cweeThreadedList<int> UnsafeList(void) const;

	/*!
	Serialize the unordered list into a single string with control character formatting (\a, \t, \n, etc.)
	*/
	cweeStr			Serialize(const cweeStr& delimiter = "\r", int option = -1, const bool threadSafePtr = false) const;

	/*!
	Deserialize the cweeStr to return this unordered list into a match for a previous form
	*/
	void			Deserialize(cweeStr& in, const cweeStr& delimiter = "\r");

	/*!
	Prevent deletion of the specified index until allowed. Does not lock the object for race conditions,
	but guarrantees it won't be deleted.
	Typical use case: accessing an unordered list within an object of an unordered list without locking
	the topmost list.
	*/
	void			PreventDeletion(int index) const;

	/*!
	Allow deletion of the specified index if previously prevented. Does not lock the object for race conditions,
	but guarrantees it won't be deleted.
	Typical use case: accessing an unordered list within an object of an unordered list without locking
	the topmost list.
	*/
	void			AllowDeletion(int index) const;

	/*!
	Check deletion list for the specified index if prevented. Does not lock the object for race conditions.
	*/
	bool			CheckCanDelete(int index) const;

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<cweeUnorderedListManagedObject<_type_>> Select(std::function<bool(const _type_*)> predicate) const {
		cweeThreadedList<cweeUnorderedListManagedObject<_type_>> out;
		cweeUnorderedListManagedObject<_type_> x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(x);
			}
			Unlock();
		}
		return out;
	};

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](int x){ return (x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<cweeUnorderedListManagedObject<_type_>> Select(std::function<bool(const _type_*)> predicate) {
		cweeThreadedList<cweeUnorderedListManagedObject<_type_>> out;
		cweeUnorderedListManagedObject<_type_> x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(x);
			}
			Unlock();
		}
		return out;
	};

	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](int x){ return (x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<int> SelectIndexes(std::function<bool(const _type_*)> predicate) const {
		cweeThreadedList<int> out;
		cweeUnorderedListManagedObject<_type_> x;
		for (auto& i : GetList()) {
			x = this->GetPtr(i);
			Lock();
			if (x && predicate(x.Ptr())) {
				out.Append(i);
			}
			Unlock();
		}
		return out;
	};

	/*!
	After calling "Lock", Append new data at a specified key location, regardless of the counter.
	*/
	void			UnsafeAppendAt(int index);

	/*!
	After calling "Lock", Append new data at the end of the list;
	*/
	int				UnsafeAppend();

	/*!
	After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing.
	*/
	void			UnsafeErase(int index);

private:

	/*!
	Check to see if the index is allowed to be deleted at this time.
	*/
	bool			CheckCanDeleteIndex(int index);

	/*!
	Mutex Lock to prevent race conditions, and shared_lock methods for advanced locking.
	*/
#ifndef useCweeMutex
	/*!
	Mutex Lock to prevent race conditions
	*/
	mutable sf::contention_free_shared_mutex<>									lock;

	/*!
	Duplicate method to allow for advanced locking mechanisms.
	*/
	void SharedLock(void) const;

	/*!
	Duplicate method to allow for advanced locking mechanisms.
	*/
	void SharedUnlock(void) const;
#else
	/*!
	Mutex Lock to prevent race conditions
	*/
#ifndef useAtomicMutex
	/*!
	Mutex Lock to prevent race conditions. cweeSysMutex uses C++ CriticalSection // shown to be perfectly stable // cweeSysMutex also appears to result in less wait time than cweeMutex?
	*/
	mutable cweeSysMutex														lock;
#else
	/*!
	Mutex Lock to prevent race conditions. cweeMutex used atomic Int to allocate access
	*/
	mutable cweeMutex															lock;
#endif

	/*!
	Duplicate method to allow for advanced locking mechanisms.
	*/
	void SharedLock(void) const;

	/*!
	Duplicate method to allow for advanced locking mechanisms.
	*/
	void SharedUnlock(void) const;
#endif

	/*!
	Counter to represent the new key for each upcoming append
	*/
#ifdef useAtomicInt	
	/*!
	Counter to represent the new key for each upcoming append
	*/
	mutable cweeSysInterlockedInteger											num;
#else
	/*!
	Counter to represent the new key for each upcoming append
	*/
	mutable cweeSysInterlockedInteger											num;
#endif


	/*!
	Memory allocator for data type.
	*/
#ifdef useAllocatedMemory
	/*!
	Memory allocator for data type.
	*/
	mutable cweeUnpooledInterlocked < cweeBlockAlloc < _type_, _size_, TAG_LIST > >	allocator;
#endif

	/*
	Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions.
	*/
#ifndef useRobinMap
	/*
	Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions.
	*/
	mutable std::unordered_map<int, _type_*>									list;
#else
	/*
	Map between key and heap ptr. Cannot use PTR directly to allow for multithread-safe instant deletes, using the keys to control race conditions.
	*/
#ifndef useRobinHash
	mutable tsl::robin_map<int, _type_*>										list;
#else
	mutable tsl::robin_map<int, _type_*, robin_hood::hash<int>, std::equal_to<int>, std::allocator<std::pair<int, _type_*>>, true>	list;
#endif

#endif

#ifdef useGranularity
	/* Optimized search parameters */
	mutable cweeSysInterlockedInteger											granularity = 16;
#endif
	/* Optimized search parameters */
	mutable cweeUnpooledInterlocked < cweeThreadedList<int>	>					indexList;
	/* Optimized search parameters */
	mutable cweeSysInterlockedInteger											list_version;// = 0;
	/* Optimized search parameters */
	mutable cweeSysInterlockedInteger											lastSearchID;// = -1;
	/* Optimized search parameters */
#ifndef useRobinMap
	/* Optimized search parameters */
	mutable typename std::unordered_map<int, _type_*>::iterator					lastResult = list.end();
#else
	/* Optimized search parameters */
#ifndef useRobinHash
	mutable typename tsl::robin_map<int, _type_*>::iterator						lastResult = list.end();
#else
	mutable typename tsl::robin_map<int, _type_*, robin_hood::hash<int>, std::equal_to<int>, std::allocator<std::pair<int, _type_*>>, true>::iterator						lastResult;//  = list.end();
#endif
#endif
	/* Optimized search parameters */
	mutable cweeSysInterlockedInteger											lastVersion;//  = -1;
	/* Optimized search parameters */
	mutable cweeSysInterlockedInteger											CreatedListVersion;// = 0;
	/* Optimized search parameters */
	mutable cweeSysInterlockedInteger											lastCreatedListVersion;// = -1;
	/* Optimized access without concern of deletion */
#ifndef useProtectedMap
	mutable cweeUnpooledInterlocked<cweeThreadedList<int>>						protected_indexes;
#else
#ifndef useRobinHash
	mutable cweeUnpooledInterlocked < tsl::robin_map<int, cweeSysInterlockedInteger> > 	protected_indexes;
#else
	mutable cweeUnpooledInterlocked < tsl::robin_map<int, cweeSysInterlockedInteger, robin_hood::hash<int>, std::equal_to<int>, std::allocator<std::pair<int, cweeSysInterlockedInteger>>, true> > 	protected_indexes;
#endif

#endif

	/* Optimized access for cweeStr names when searching for indexes with a matching name */
#ifndef useRobinHash
	mutable cweeUnpooledInterlocked < std::unordered_map<std::string, int> > 	assetNameToIndexMap;
#else
	mutable cweeUnpooledInterlocked < tsl::robin_map<std::string, int, robin_hood::hash<std::string>, std::equal_to<std::string>, std::allocator<std::pair<std::string, int>>, true> > assetNameToIndexMap;
#endif
	/* Optimized search parameters */
	mutable cweeUnpooledInterlocked < cweeThreadedList<int>	>					eraseQueue;
};
#pragma endregion

#pragma region "CWEEUNORDEREDLIST CLASS DEFINITIONS BASED ON SELECTED OPTIONS"

/*!
Default copy
*/
template< typename _type_, int _size_ >
INLINE cweeUnorderedList<_type_, _size_>& cweeUnorderedList<_type_, _size_>::operator=(const cweeUnorderedList<_type_, _size_>& other) {
	this->Copy(other);
	return *this;
};

/*!
Default destructor
*/
template< typename _type_, int _size_ >
INLINE cweeUnorderedList<_type_, _size_>::~cweeUnorderedList() {
	Clear(); // free memory
};

/*!
Convert UnorderedList to a list of keys
*/
template< typename _type_, int _size_ >
INLINE cweeUnorderedList<_type_, _size_>::operator cweeThreadedList<int>() {
	return GetList();
};

/*!
Convert UnorderedList to a list of keys
*/
template< typename _type_, int _size_ >
INLINE cweeUnorderedList<_type_, _size_>::operator cweeThreadedList<int>() const {
	return GetList();
};

/*!
Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
*/
template< typename _type_, int _size_ >
INLINE _type_			cweeUnorderedList<_type_, _size_>::operator[](int index) const
{
	return Read(index);
};

/*!
Get a copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
*/
template< typename _type_, int _size_ >
INLINE _type_			cweeUnorderedList<_type_, _size_>::operator[](int index)
{
	return Read(index);
};

/*!
Allocate memory for object in heap and retrieve key to later access and edit the object.
*/
template< typename _type_, int _size_ >
INLINE int				cweeUnorderedList<_type_, _size_>::Append(void) {
	SharedLock();
	list_version.Increment();
	CreatedListVersion.Increment();
	int which = -1;
	/* Get index for new allocation */
	which = num.Increment();
	/* Allocate a _type_ and store at specified index */
#ifdef useAllocatedMemory
	allocator.Lock();
	_type_* allocatedObj = allocator.UnsafeRead()->Alloc();
	allocator.Unlock();
#else
	_type_* allocatedObj = new _type_;
#endif

#ifdef useGranularity
	if (granularity <= num)
	{
		granularity = useGranularity * granularity.GetValue();
		list.reserve(granularity.GetValue());
	}
#endif

	if (which >= 0) list[which] = allocatedObj;
	else {
#ifdef useAllocatedMemory
		allocator.Lock();
		allocator.UnsafeRead()->Free(allocatedObj);
		allocator.Unlock();
#else
		delete allocatedObj;
#endif
	}

	SharedUnlock();
	return which;
	};

/*!
Swap if the object exists, otherwise append at the index and then swap.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::Emplace(int index, const _type_& obj) {
	Lock();
	auto ptr = UnsafeRead(index);
	if (ptr) {
		// already exists
		*ptr = obj;
	}
	else {
		// does not yet exist
		UnsafeAppendAt(index);
		ptr = UnsafeRead(index);
		if (ptr) {
			*ptr = obj;
		}
	}
	Unlock();
}

/*!
Get a copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
*/
template< typename _type_, int _size_ >
INLINE _type_			cweeUnorderedList<_type_, _size_>::Read(int index, const bool threadSafePtr) {
	if (threadSafePtr) {
		PreventDeletion(index);
		SharedLock();
		{
			auto it = lastResult;
			if (lastSearchID == index && list.count(lastSearchID.GetValue()) != 0 && lastVersion == list_version)	it = lastResult;
			else { it = list.find(index); lastSearchID = index; lastResult = it; lastVersion = list_version; }

			if (it != list.end()) {
				_type_* ptr = it->second;
				SharedUnlock();
				_type_ out = *ptr;
				AllowDeletion(index);
				return out;
			}
			else
			{
				SharedUnlock();
				AllowDeletion(index);
				return _type_();
			}
		}
	}
	else {

		SharedLock();
		{
			auto it = lastResult;
			if (lastSearchID == index && list.count(lastSearchID.GetValue()) != 0 && lastVersion == list_version)	it = lastResult;
			else { it = list.find(index); lastSearchID = index; lastResult = it; lastVersion = list_version; }

			if (it != list.end()) {
				_type_ out = *it->second;
				SharedUnlock();
				return out;
			}
			else
			{
				SharedUnlock();
				return _type_();
			}
		}

	}
};

/*!
Get a read-only copy of the object from the heap. if the object does not exist on the heap, an empty object will be made on the stack.
*/
template< typename _type_, int _size_ >
INLINE _type_			cweeUnorderedList<_type_, _size_>::Read(int index, const bool threadSafePtr) const {
	if (threadSafePtr) {
		PreventDeletion(index);
		SharedLock();
		{
			auto it = lastResult;
			if (lastSearchID == index && list.count(lastSearchID.GetValue()) != 0 && lastVersion == list_version)	it = lastResult;
			else { it = list.find(index); lastSearchID = index; lastResult = it; lastVersion = list_version; }

			if (it != list.end()) {
				_type_* ptr = it->second;
				SharedUnlock();
				_type_ out = *ptr;
				AllowDeletion(index);
				return out;
			}
			else
			{
				SharedUnlock();
				AllowDeletion(index);
				return _type_();
			}
		}
	}
	else {
		SharedLock();
		{
			auto it = lastResult;
			if (lastSearchID == index && list.count(lastSearchID.GetValue()) != 0 && lastVersion == list_version)	it = lastResult;
			else { it = list.find(index); lastSearchID = index; lastResult = it; lastVersion = list_version; }

			if (it != list.end()) {
				_type_ out = *it->second;
				SharedUnlock();
				return out;
			}
			else
			{
				SharedUnlock();
				return _type_();
			}
		}
	}
};

/*!
Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the underlying cweeUnorderedList lifetime does.
*/
template< typename _type_, int _size_ >
INLINE cweeUnorderedListManagedObject<_type_>	cweeUnorderedList<_type_, _size_>::GetPtr(int index) {
	return cweeUnorderedListManagedObject<_type_>(this, index);
};

/*!
Gets a pointer (or null) to the underlyding data. This is a managed object, meaning its lifetime must end before the underlying cweeUnorderedList lifetime does.
*/
template< typename _type_, int _size_ >
INLINE cweeUnorderedListManagedObject<_type_>	cweeUnorderedList<_type_, _size_>::GetPtr(int index) const {
	return cweeUnorderedListManagedObject<_type_>(this, index);
};

/*!
True/False if the index exists on the heap
*/
template< typename _type_, int _size_ >
INLINE bool			cweeUnorderedList<_type_, _size_>::Check(int index) {
	bool result = true;
	if (lastCreatedListVersion == CreatedListVersion) {
		SharedLock();
		indexList.Lock();
		result = (indexList.UnsafeRead()->FindIndex(index) != -1);
		indexList.Unlock();
		SharedUnlock();
		return result;
	}
	SharedLock();
	result = (list.count(index) != 0);
	SharedUnlock();
	return result;
};

/*!
Clear the current list and create a copy of the incoming list.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::Copy(const cweeUnorderedList<_type_, _size_>& copy, const bool threadSafePtr) {
#if 0
	// delete "extra" allocations from our list
	cweeThreadedList<int> thisList = GetList();
	cweeThreadedList<int> List = copy.GetList();
	for (int i = 0; i < Num(); i++) {
		if (List.FindIndex(thisList[i]) == -1) {
			Erase(thisList[i]);
		}
	}


	// create data for "missing" allocations from our list		
	thisList = GetList();
	Lock();
	for (int i = 0; i < List.Num(); i++)
		if (thisList.FindIndex(List[i]) == -1)
			UnsafeAppendAt(List[i]);
	Unlock();

	// Go through and perform swaps
	copy.Lock();
	Lock();
	for (int i = 0; i < List.Num(); i++) {
		auto readReal = UnsafeRead(List[i]);
		auto readCopy = copy.UnsafeRead(List[i]);
		if (readReal != nullptr && readCopy != nullptr)	*readReal = *readCopy;
		//Swap(List[i], copy.Read(List[i]));
	}
	Unlock();
	copy.Unlock();
#elif 0

	Clear();

	cweeThreadedList<int> List = copy.GetList();

	// Go through and perform swaps
	Lock();
	for (int i = 0; i < List.Num(); i++) {
		auto readCopy = copy.UnsafeRead(List[i]);
		if (readCopy) {
			UnsafeAppendAt(List[i]);
			auto access = UnsafeRead(List[i]);
			if (access)
				*access = *readCopy;
		}
	}
	Unlock();
#else

	Clear();

	cweeThreadedList<int> List = copy.GetList();

	if (threadSafePtr) {
		// Go through and perform swaps					
		for (int i = 0; i < List.Num(); i++) {
			copy.PreventDeletion(List[i]);
			copy.Lock();
			auto readCopy = copy.UnsafeRead(List[i]);
			copy.Unlock();

			if (readCopy) {
				PreventDeletion(List[i]);
				Lock();
				UnsafeAppendAt(List[i]);
				auto access = UnsafeRead(List[i]);
				Unlock();

				if (access) *access = *readCopy;

				AllowDeletion(List[i]);
			}

			copy.AllowDeletion(List[i]);
}
}
	else {
		// Go through and perform swaps		
		for (int i = 0; i < List.Num(); i++) {
			copy.Lock();
			auto readCopy = copy.UnsafeRead(List[i]);
			if (readCopy) {
				Lock();
				UnsafeAppendAt(List[i]);
				auto access = UnsafeRead(List[i]);
				if (access) *access = *readCopy;
				Unlock();
			}
			copy.Unlock();
		}
	}








#endif
}

/*!
Copy the _type_ object from the stack into the heap at the index specified. If the index does not exist on the heap, nothing happens.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::Swap(int index, const _type_& replacement) {
	if (Check(index)) {
		while (!CheckCanDeleteIndex(index)) {};
		Lock();
		auto it = lastResult;
		if (lastSearchID == index && list.count(lastSearchID.GetValue()) != 0 && lastVersion == list_version)	it = lastResult;
		else { it = list.find(index); lastSearchID = index; lastResult = it; lastVersion = list_version; }
		list_version.Increment();
		if (it != list.end()) *it->second = replacement;
		Unlock();
	}
};

/*!
get the number of objects on the heap managed by this list
*/
template< typename _type_, int _size_ >
INLINE int				cweeUnorderedList<_type_, _size_>::Num(void) const {
	int i = 0;
	if (lastCreatedListVersion == CreatedListVersion) {
		indexList.Lock();
		i = indexList.UnsafeRead()->Num();
		indexList.Unlock();
		return i;
	}

	SharedLock();
	i = list.size();
	SharedUnlock();
	return i;
};

/*!
get the number of objects on the heap managed by this list
*/
template< typename _type_, int _size_ >
INLINE int				cweeUnorderedList<_type_, _size_>::UnsafeNum(void) const {
	int i = 0;
	if (lastCreatedListVersion == CreatedListVersion) {
		indexList.Lock();
		i = indexList.UnsafeRead()->Num();
		indexList.Unlock();		
		return i;
	}
	i = list.size();
	return i;
};

/*!
get a list of all indexes currently on the heap that are currently valid
*/
//template< typename _type_, int _size_ >
//INLINE cweeThreadedList<int> cweeUnorderedList<_type_, _size_>::GetList(void) const {
//	cweeThreadedList<int> out;
//
//	if (lastCreatedListVersion == CreatedListVersion) {
//		indexList.Lock();
//		out = *indexList.UnsafeRead();
//		indexList.Unlock();
//		return out;
//	}
//	int size = Num();
//
//	//SharedLock();
//	//SharedUnlock(); // forces the list to erase if needed
//
//	SharedLock();
//	//eraseQueue.Lock();
//	indexList.Lock();
//	indexList.UnsafeRead()->Clear();
//	indexList.UnsafeRead()->SetGranularity(size + 16);
//	for (auto& kv : list) {
//		//if (eraseQueue.UnsafeRead()->FindIndex(kv.first) < 0) {
//			indexList.UnsafeRead()->Append(kv.first);
//		//}		
//	}
//	lastCreatedListVersion = CreatedListVersion;
//	indexList.Unlock();
//	//eraseQueue.Unlock();
//
//	indexList.Lock();
//	out = *indexList.UnsafeRead();
//	indexList.Unlock();
//
//	SharedUnlock();
//	return out;
//};

/*!
Erase the indexed object from the heap and free the memory for the list to re-use.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::Erase(int index) {
	if (Check(index)) {
		list_version.Increment();
		CreatedListVersion.Increment();

		eraseQueue.Lock();
		eraseQueue.UnsafeRead()->Append(index);
		eraseQueue.Unlock();
	}
};

///*!
//Clear the current list and free all memory back to the operating system
//*/
//template< typename _type_, int _size_ >
//INLINE void			cweeUnorderedList<_type_, _size_>::Clear(void) {
//#if 1
//	Lock();
//
//	auto List = UnsafeList();
//	for (auto& it : List) {
//		UnsafeErase(it);
//	}
//
//	{
//		eraseQueue.Lock();
//		if (eraseQueue.UnsafeRead()->Num() > 0) {
//			protected_indexes.Lock();
//			for (int i = 0; i < eraseQueue.UnsafeRead()->Num(); i++) {
//				auto& x = eraseQueue.UnsafeRead()->operator[](i);
//				if (protected_indexes.UnsafeRead()->operator[](x).GetValue() <= 0) {
//					// do erasure
//					if (list.count(x) != 0) {
//						list_version.Increment();
//						CreatedListVersion.Increment();
//						auto it = list.find(x);
//						if (it != list.end()) {
//#ifdef useAllocatedMemory
//							allocator.Lock();
//							if ((_type_*)it->second) {
//								// ((cweeBlockAlloc < _type_, _size_, TAG_LIST >*)allocator.UnsafeRead())->Free((_type_*)it->second);
//								allocator.UnsafeRead()->Free((_type_*)it->second);
//							}
//							allocator.Unlock();
//#else
//							delete it->second;
//#endif
//#ifndef useRobinMap
//							it->second = nullptr;
//							list.erase(it->first);
//#else
//							list.erase(it->first);
//#endif
//						}
//					}
//
//					eraseQueue.UnsafeRead()->RemoveIndexFast(i); // no longer need to erase
//					i--; // stay here
//				}
//			}
//			protected_indexes.Unlock();
//		}
//		eraseQueue.Unlock();
//	}
//
//#ifdef useAllocatedMemory
//	allocator.Lock();
//	allocator.UnsafeRead()->FreeEmptyBlocks();
//	allocator.Unlock();
//#endif
//	num = 0;
//
//	list_version = 0;
//	CreatedListVersion = 0;
//
//	indexList = cweeThreadedList<int>();
//	lastSearchID = -1;
//	typedef typename decltype(list) listType;
//	list = listType();
//	lastResult = list.end();
//	lastVersion = -1;
//	lastCreatedListVersion = -1;
//
//	Unlock();
//
//#else
//
//	bool succeeded = false;
//	do {
//		Lock();
//		{
//			bool allowed = true;
//			std::vector<int> intList = UnsafeList();
//			for (auto it : intList) if (CheckCanDeleteIndex(it) == false) allowed = false;
//			if (allowed == true) {
//				succeeded = true;
//
//				for (auto it : intList) {
//					AllowDeletion(it);
//					UnsafeErase(it);
//				}
//
//#ifdef useAllocatedMemory
//				allocator.Lock();
//				allocator.UnsafeRead()->FreeEmptyBlocks();
//				allocator.Unlock();
//#endif
//
//				num = 0;
//				list_version = 0;
//				CreatedListVersion = 0;
//
//				indexList = UnsafeList();
//				lastSearchID = -1;
//				lastResult = list.end();
//				lastVersion = -1;
//				lastCreatedListVersion = -1;
//
//
//
//			}
//		}
//		Unlock();
//	} while (succeeded == false);
//
//
//#endif
//};

/*!
Get approximate number of bytes referenced by this list.
*/
template< typename _type_, int _size_ >
INLINE const size_t	cweeUnorderedList<_type_, _size_>::BytesUsed(void) {
#ifdef useAllocatedMemory
	//Lock();
	allocator.Lock();
	size_t i(allocator.UnsafeRead()->GetTotalCount());
	allocator.Unlock();
	//Unlock();
	return i;
#else
	return static_cast<size_t>(Num() * (sizeof(_type_) + sizeof(_type_*)));
#endif
				}

/*!
Iterates through list to find the member with the matching "::Name" member. If no item exists with this member, returns -1. If found, returns the index of the member.
This is a utility function, and is not going to give great performance compared to other
*/
template< typename _type_, int _size_ >
INLINE const int		cweeUnorderedList<_type_, _size_>::FindIndexWithName(const cweeStr& name) const {
	int out = -1;
	_type_* ptr = nullptr;
	cweeStr sample;
	bool read;

	// Optimized search
	{
		assetNameToIndexMap.Lock();
		const auto found = assetNameToIndexMap.UnsafeRead()->find(name.c_str());
		if (found != assetNameToIndexMap.UnsafeRead()->end()) {
			out = found->second;
			Lock();
			ptr = UnsafeRead(out);
			if (ptr) {
				read = cweeGetName(*ptr, sample); // must check the name of the remembered index - in case it has changed since before. Ideally this is still a 1-time check compared to N-size
				Unlock(); // unlock ASAP. 
				if (read) {
					if (sample != name) {
						out = -1;
						assetNameToIndexMap.UnsafeRead()->clear(); // this map has updated and can no longer be trusted.
					}
				}
			}
			else Unlock();
		}
		assetNameToIndexMap.Unlock();

		if (out >= 0) return out;
}

	for (auto& index : GetList()) {
		Lock();
		ptr = UnsafeRead(index);
		if (ptr) {
			read = cweeGetName(*ptr, sample);
			Unlock(); // unlock ASAP. 
			if (read)
				if (sample == name)
					out = index; // do string compare out-of-lock
		}
		else
			Unlock();
		if (out >= 0) break;
	}

	// Optimized search record
	if (out >= 0) { // record if successful
		assetNameToIndexMap.Lock();
		assetNameToIndexMap.UnsafeRead()->operator[](name.c_str()) = out;
		assetNameToIndexMap.Unlock();
	}

	return out;
			}

/*!
Iterates through the list to return the list of ::Name members. May return empty if the object of this UnorderedList has no member named ::Name
*/
template< typename _type_, int _size_ >
INLINE cweeThreadedList<cweeStr> cweeUnorderedList<_type_, _size_>::GetNames() const {
	cweeThreadedList<cweeStr> out(Num() + 16);
	cweeStr sample;
	bool read;
	for (auto& index : GetList()) {
		Lock();
		auto ptr = UnsafeRead(index);
		if (ptr) {
			read = cweeGetName(*ptr, sample); Unlock(); // unlock ASAP. 
			if (read) out.Append(sample);
		}
		else Unlock();
	}
	return out;
}

/*!
Get the name of the object at the specified index. May return empty if the object of this UnorderedList has no member named ::Name
*/
template< typename _type_, int _size_ >
INLINE cweeStr			cweeUnorderedList<_type_, _size_>::GetName(int index) const {
	cweeStr out;
	Lock();
	auto ptr = UnsafeRead(index);
	if (ptr) cweeGetName(*ptr, out);
	Unlock();
	return out;
}

/*!
prevent multi-thread access to the list. Only the "Unsafe*" operations and "Unlock" are valid after this call or else the app will deadlock.
A "Unlock" must be called to re-enable access to the list.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::Lock(void) const {//was non-const
#ifndef useCweeMutex
	lock.lock();
#else
	lock.Lock();

	//lock.Lock();
	//lock.UnsafeRead()->Lock();
	//lock.Unlock();

#endif
}

/*!
Only call this after calling "Lock". Multiple unlocks in a row is undefined behavior.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::Unlock(void) const { //was non-const
#if 1

#ifndef useProtectedMap

	// assumes already locked! 
	eraseQueue.Lock();
	if (eraseQueue.UnsafeRead()->Num() > 0) {
		protected_indexes.Lock();
		for (int i = 0; i < eraseQueue.UnsafeRead()->Num(); i++) {
			auto& x = eraseQueue.UnsafeRead()->operator[](i);
			if (protected_indexes.UnsafeRead()->Num() == 0 || protected_indexes.UnsafeRead()->FindIndex(x) == -1) {
				// do erasure
				if (list.count(x) != 0) {
					list_version.Increment();
					CreatedListVersion.Increment();
					auto it = list.find(x);
					if (it != list.end()) {
#ifdef useAllocatedMemory
						allocator.Lock();
						if ((_type_*)it->second) {
							((cweeBlockAlloc < _type_, _size_, TAG_LIST >*)allocator.UnsafeRead())->Free((_type_*)it->second);
						}
						allocator.Unlock();
#else
						delete it->second;
#endif
#ifndef useRobinMap
						it->second = nullptr;
						list.erase(it->first);
#else
						list.erase(it->first);
#endif
					}
				}

				eraseQueue.UnsafeRead()->RemoveIndexFast(i); // no longer need to erase
				i--; // stay here
			}
		}
		protected_indexes.Unlock();
	}
	eraseQueue.Unlock();

#else

	// assumes already locked! 
	eraseQueue.Lock();
	if (eraseQueue.UnsafeRead()->Num() > 0) {
		protected_indexes.Lock();
		for (int i = 0; i < eraseQueue.UnsafeRead()->Num(); i++) {
			auto& x = eraseQueue.UnsafeRead()->operator[](i);
			if (protected_indexes.UnsafeRead()->operator[](x).GetValue() <= 0) {
				// do erasure
				if (list.count(x) != 0) {
					list_version.Increment();
					CreatedListVersion.Increment();
					auto it = list.find(x);
					if (it != list.end()) {
#ifdef useAllocatedMemory
						allocator.Lock();
						if ((_type_*)it->second) {
							// ((cweeBlockAlloc < _type_, _size_, TAG_LIST >*)allocator.UnsafeRead())->Free((_type_*)it->second);
							allocator.UnsafeRead()->Free((_type_*)it->second);
						}
						allocator.Unlock();
#else
						delete it->second;
#endif
#ifndef useRobinMap
						it->second = nullptr;
						list.erase(it->first);
#else
						list.erase(it->first);
#endif
					}
				}

				eraseQueue.UnsafeRead()->RemoveIndexFast(i); // no longer need to erase
				i--; // stay here
			}
		}
		protected_indexes.Unlock();
	}
	eraseQueue.Unlock();


#endif





#endif
#ifndef useCweeMutex
	lock.unlock();
#else
	lock.Unlock();



#endif

					};

/*!
After calling "Lock", this will allow access to directly edit the specified object on the heap without swapping.
*/
template< typename _type_, int _size_ >
INLINE _type_* cweeUnorderedList<_type_, _size_>::UnsafeRead(int index) const { // was non-const
	{
		auto it = list.find(index);
		if (it != list.end()) {
			return it->second;
		}
		return nullptr;
}
				}

/*!
After calling "Lock", this will allow access to get a list of all valid indexes on the heap managed by this list.
*/
template< typename _type_, int _size_ >
INLINE cweeThreadedList<int> cweeUnorderedList<_type_, _size_>::UnsafeList(void) const {
	cweeThreadedList<int> out;

	if (lastCreatedListVersion == CreatedListVersion) {
		indexList.Lock();
		out = *indexList.UnsafeRead();
		indexList.Unlock();

		return out;
	}
	int size = list.size();

	indexList.Lock();
	indexList.UnsafeRead()->Clear();
	indexList.UnsafeRead()->SetGranularity(size + 16);
	for (auto& kv : list) indexList.UnsafeRead()->Append(kv.first);
	lastCreatedListVersion = CreatedListVersion;
	indexList.Unlock();

	indexList.Lock();
	out = *indexList.UnsafeRead();
	indexList.Unlock();

	return out;
};

/*!
Serialize the unordered list into a single string with control character formatting (\a, \t, \n, etc.)
*/
template< typename _type_, int _size_ >
INLINE cweeStr			cweeUnorderedList<_type_, _size_>::Serialize(const cweeStr& delimiter, int option, const bool threadSafePtr) const {
	/*
	Desired Form: (no tabs whatsoever for this. Tabs are reserved for sub-objects to be serialized.
	{
		1\n
		cweeStr (asset 1)\n  // number above is the INT for the unordered_map, cweeStr here should be used to create the object.
		15\n
		cweeStr (asset 2)\n // number above is the INT for the unordered_map, cweeStr here should be used to create the object. Not required to be in-order!
		16\n
		cweeStr (asset 3)\n // number above is the INT for the unordered_map, cweeStr here should be used to create the object.
		...
	}
	*/
	cweeStr out;
	std::vector<int> objects = GetList();
	std::sort(objects.begin(), objects.end());
	if (objects.size() == 0) out += " "; // ensure non-empty for downstream objects in case this object is empty. 	
	cweeStr serialized; bool test;
	for (auto& x : objects) {
		serialized = ""; test = false;
		out.AddToDelimiter(x, delimiter);

		if (threadSafePtr) {
			PreventDeletion(x);
			Lock();
			_type_* obj = UnsafeRead(x);
			Unlock();
			if (obj) test = cweeSerialize(*obj, serialized, option);
			AllowDeletion(x);
		}
		else {
			Lock();
			_type_* obj = UnsafeRead(x);
			if (obj) test = cweeSerialize(*obj, serialized, option);
			Unlock();
		}

		if (test) out.AddToDelimiter(serialized, delimiter);
		else out.AddToDelimiter(" ", delimiter);
	}
	return out;
}

/*!
Deserialize the cweeStr to return this unordered list into a match for a previous form
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::Deserialize(cweeStr& in, const cweeStr& delimiter) {
	cweeParser incomingList(in, delimiter, true);
	in.Clear();
	int numLines = incomingList.getNumVars();

#if 1
	Clear(); // clear current list
	for (int i = 0; i < (numLines - 1); i += 2) { // 0, then 2, then 4... etc.
		cweeStr& mapInt = incomingList[i];
		cweeStr& serializedObj = incomingList[i + 1];

		Lock();
		UnsafeAppendAt((int)mapInt);
		auto ptr = UnsafeRead((int)mapInt);
		if (ptr) {
			cweeDeserialize(*ptr, serializedObj);
		}
		Unlock();
		mapInt.Clear();
		serializedObj.Clear();
	}

#else

	Clear(); // clear current list
	Lock(); // Lock down the list and ensure nothing else happens meanwhile.
	for (int i = 0; i < (numLines - 1); i += 2) { // 0, then 2, then 4... etc.
		cweeStr& mapInt = incomingList[i];
		cweeStr& serializedObj = incomingList[i + 1];

#ifdef useAllocatedMemory
		allocator.Lock();
		_type_* allocatedObj = allocator.UnsafeRead()->Alloc();
		allocator.Unlock();
#else
		_type_* allocatedObj = new _type_;
#endif
		cweeDeserialize(*allocatedObj, serializedObj);
		list[(int)mapInt] = allocatedObj;

		mapInt.Clear();
		serializedObj.Clear();
	}

	int maxNum = 0;
	std::vector<int> thisList = GetList();
	std::sort(thisList.begin(), thisList.end());
	if (thisList.size() > 0) maxNum = (thisList[thisList.size() - 1]);

	num = maxNum;
	Unlock();

#endif

}

/*!
Prevent deletion of the specified index until allowed. Does not lock the object for race conditions,
but guarrantees it won't be deleted.
Typical use case: accessing an unordered list within an object of an unordered list without locking
the topmost list.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::PreventDeletion(int index) const {

#ifndef useProtectedMap
	protected_indexes.Lock();
	protected_indexes.UnsafeRead()->Append(index);
	protected_indexes.Unlock();
#else
	protected_indexes.Lock();
	protected_indexes.UnsafeRead()->operator[](index).Increment();
	protected_indexes.Unlock();
#endif

	};

/*!
Allow deletion of the specified index if previously prevented. Does not lock the object for race conditions,
but guarrantees it won't be deleted.
Typical use case: accessing an unordered list within an object of an unordered list without locking
the topmost list.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::AllowDeletion(int index) const {
#ifndef useProtectedMap
	protected_indexes.Lock();
	protected_indexes.UnsafeRead()->Remove(index);
	protected_indexes.Unlock();
#else
	protected_indexes.Lock();
	if (protected_indexes.UnsafeRead()->operator[](index).Decrement() == 0) {
		protected_indexes.UnsafeRead()->erase(index);
}
	protected_indexes.Unlock();
#endif
};

/*!
Check deletion list for the specified index if prevented. Does not lock the object for race conditions.
*/
template< typename _type_, int _size_ >
INLINE bool			cweeUnorderedList<_type_, _size_>::CheckCanDelete(int index) const {
	bool out;
#ifndef useProtectedMap
	protected_indexes.Lock();
	out = (protected_indexes.UnsafeRead()->Find(index) ? false : true);
	protected_indexes.Unlock();
#else
	protected_indexes.Lock();
	out = protected_indexes.UnsafeRead()->operator[](index).GetValue() > 0 ? false : true;
	protected_indexes.Unlock();
#endif
	return out;
};

/*!
After calling "Lock", Append new data at a specified key location, regardless of the counter.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::UnsafeAppendAt(int index) {
	list_version.Increment();
	CreatedListVersion.Increment();
	int which = index;
	if (num.GetValue() <= which) num.SetValue(which + 1);
	/* Allocate a _type_ and store at specified index */
#ifdef useAllocatedMemory
	allocator.Lock();
	_type_* allocatedObj = allocator.UnsafeRead()->Alloc();
	allocator.Unlock();
#else
	_type_* allocatedObj = new _type_;
#endif

#ifdef useGranularity
	if (granularity <= num)
	{
		granularity = useGranularity * granularity.GetValue();
		list.reserve(granularity.GetValue());
	}
#endif

	list[which] = allocatedObj;
};

/*!
After calling "Lock", Append new data at the end of the list;
*/
template< typename _type_, int _size_ >
INLINE int				cweeUnorderedList<_type_, _size_>::UnsafeAppend() {
	list_version.Increment();
	CreatedListVersion.Increment();
	int which = -1;
	/* Get index for new allocation */
	which = num.Increment();
	/* Allocate a _type_ and store at specified index */
#ifdef useAllocatedMemory
	allocator.Lock();
	_type_* allocatedObj = allocator.UnsafeRead()->Alloc();
	allocator.Unlock();
#else
	_type_* allocatedObj = new _type_;
#endif

#ifdef useGranularity
	if (granularity <= num)
	{
		granularity = useGranularity * granularity.GetValue();
		list.reserve(granularity.GetValue());
	}
#endif

	if (which >= 0) list[which] = allocatedObj;
	else {
#ifdef useAllocatedMemory
		allocator.Lock();
		allocator.UnsafeRead()->Free(allocatedObj);
		allocator.Unlock();
#else
		delete allocatedObj;
#endif
	}
	return which;
};

/*!
After calling "Lock", this will allow access to directly erase the specified object on the heap without interuption or chance for mid-erase viewing.
*/
template< typename _type_, int _size_ >
INLINE void			cweeUnorderedList<_type_, _size_>::UnsafeErase(int index) {
	list_version.Increment();
	CreatedListVersion.Increment();

	eraseQueue.Lock();
	eraseQueue.UnsafeRead()->Append(index);
	eraseQueue.Unlock();
};

/*!
Check to see if the index is allowed to be deleted at this time.
*/
template< typename _type_, int _size_ >
INLINE bool			cweeUnorderedList<_type_, _size_>::CheckCanDeleteIndex(int index) {
	bool out;
#ifndef useProtectedMap
	protected_indexes.Lock();
	out = (protected_indexes.UnsafeRead()->Find(index) ? false : true);
	protected_indexes.Unlock();
#else
	protected_indexes.Lock();
	out = protected_indexes.UnsafeRead()->operator[](index).GetValue() > 0 ? false : true;
	protected_indexes.Unlock();
#endif
	return out;
};

#ifndef useCweeMutex
template< typename _type_, int _size_ >
INLINE void cweeUnorderedList<_type_, _size_>::SharedLock(void) const {
	lock.lock_shared();
}

template< typename _type_, int _size_ >
INLINE void cweeUnorderedList<_type_, _size_>::SharedUnlock(void) const {
	lock.unlock_shared();
}
#else
template< typename _type_, int _size_ >
INLINE void cweeUnorderedList<_type_, _size_>::SharedLock(void) const {
	lock.Lock();
}

template< typename _type_, int _size_ >
INLINE void cweeUnorderedList<_type_, _size_>::SharedUnlock(void) const {
	lock.Unlock();
}
#endif

#pragma endregion

#pragma region "CWEEUNORDEREDLISTREFERENCEOBJECT CLASS DEFINITIONS"

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::cweeUnorderedListReferenceObject(const cweeSharedPtr < cweeUnorderedList< T > > sourceList, int itemIndex) {
	which = itemIndex;
	items = sourceList;

	if (items) {
		items->PreventDeletion(which);
		items->Lock();
		item = items->UnsafeRead(which);
		items->Unlock();
	}
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::~cweeUnorderedListReferenceObject() {
	if (items) {
		items->AllowDeletion(which);
	}
	if (anyItems) {
		anyItems->AllowDeletion(which);
	}
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::cweeUnorderedListReferenceObject(const cweeUnorderedListReferenceObject& other) {
	which = other.which;
	if (other.items) {
		items = other.items;
		items->PreventDeletion(which);
	}
	if (other.anyItems) {
		anyItems = other.anyItems;
		anyItems->PreventDeletion(which);
	}
	item = other.item;
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::cweeUnorderedListReferenceObject(cweeUnorderedListReferenceObject&& other) {
	// assumes that the 'other' has already prevented deletion - we are not going to re-lock, but instead just take ownership away
	which = other.which;
	items = other.items;
	anyItems = other.anyItems;
	item = other.item;


	other.which = 0;
	other.items = nullptr;
	other.anyItems = nullptr;
	other.item = nullptr;	
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>& cweeUnorderedListReferenceObject<T>::operator=(const cweeUnorderedListReferenceObject<T>& other) {
	// undo whatever it is I have 
	if (items) items->AllowDeletion(which);
	if (anyItems) anyItems->AllowDeletion(which);

	which = other.which;
	items = other.items;
	anyItems = other.anyItems;
	if (items) items->PreventDeletion(which);
	if (anyItems) anyItems->PreventDeletion(which);
	item = other.item;

	return *this;
};

template<typename T>
INLINE void cweeUnorderedListReferenceObject<T>::Clear() {
	if (items) {
		items->AllowDeletion(which);
	}
	if (anyItems) {
		anyItems->AllowDeletion(which);
	}

	which = -1;
	items.reset();
	anyItems.reset();
	item = nullptr;
};

template<typename T>
INLINE const int cweeUnorderedListReferenceObject<T>::Index() const {
	return which;
};

template<typename T>
INLINE const T* cweeUnorderedListReferenceObject<T>::operator->() const {
	return item;
};

template<typename T>
INLINE T* cweeUnorderedListReferenceObject<T>::operator->() {
	return item;
};

template<typename T>
INLINE T* cweeUnorderedListReferenceObject<T>::Ptr() {
	return item;
};

template<typename T>
INLINE const T* cweeUnorderedListReferenceObject<T>::Ptr() const {
	return item;
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>& cweeUnorderedListReferenceObject<T>::operator=(const T& input) {
	if (item) {
		*item = input;
	}
	return *this;
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::operator bool() {
	return item;
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::operator bool() const {
	return item;
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::operator T() {
	return *item;
};

template<typename T>
INLINE cweeUnorderedListReferenceObject<T>::operator T() const {
	return *item;
};

#pragma endregion

#pragma region "CWEEUNORDEREDLISTMANAGEDOBJECT CLASS DEFINITIONS"

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::cweeUnorderedListManagedObject() {
	which = 0;
	items = nullptr;
	item = nullptr;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::~cweeUnorderedListManagedObject() {
	if (items) items->AllowDeletion(which);
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::cweeUnorderedListManagedObject(const cweeUnorderedListManagedObject& other) {
	which = other.which;
	items = other.items;
	if (items) items->PreventDeletion(which);
	item = other.item;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::cweeUnorderedListManagedObject(cweeUnorderedListManagedObject&& other) {
	// assumes that the 'other' has already prevented deletion - we are not going to re-lock, but instead just take ownership away
	which = other.which;
	items = other.items;
	item = other.item;

	other.which = 0;
	other.items = nullptr;
	other.item = nullptr;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>& cweeUnorderedListManagedObject<T>::Take(cweeUnorderedListManagedObject&& other) {
	// assumes that the 'other' has already prevented deletion - we are not going to re-lock, but instead just take ownership away
	if (items) items->AllowDeletion(which);

	which = other.which;
	items = other.items;
	item = other.item;

	other.which = 0;
	other.items = nullptr;
	other.item = nullptr;

	return *this;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>& cweeUnorderedListManagedObject<T>::operator=(const cweeUnorderedListManagedObject& other) {
	// undo whatever it is I have 
	if (items) items->AllowDeletion(which);

	which = other.which;
	items = other.items;
	if (items) items->PreventDeletion(which);
	item = other.item;

	return *this;
};

template<typename T>
INLINE void cweeUnorderedListManagedObject<T>::SwapPtrs(T* ptr, int index) {
	if (items) {
		items->AllowDeletion(which);
		items->PreventDeletion(index);
	}

	which = index;
	item = ptr;
};

template<typename T>
INLINE void cweeUnorderedListManagedObject<T>::Clear() {
	if (items) {
		items->AllowDeletion(which);
	}
	which = -1;
	item = nullptr;
	items = nullptr;
};

template<typename T>
INLINE const int cweeUnorderedListManagedObject<T>::Index() const {
	return which;
};

template<typename T>
INLINE const T* cweeUnorderedListManagedObject<T>::operator->() const {
	return item;
};

template<typename T>
INLINE T* cweeUnorderedListManagedObject<T>::operator->() {
	return item;
};

template<typename T>
INLINE T* cweeUnorderedListManagedObject<T>::Ptr() {
	return item;
};

template<typename T>
INLINE const T* cweeUnorderedListManagedObject<T>::Ptr() const {
	return item;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>& cweeUnorderedListManagedObject<T>::operator=(const T& input) {
	if (item) {
		*item = input;
	}
	return *this;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::operator bool() {
	return item;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::operator bool() const {
	return item;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::operator T() {
	return *item;
};

template<typename T>
INLINE cweeUnorderedListManagedObject<T>::operator T() const {
	return *item;
};

#pragma endregion

template <typename T>
class cweeListObj : public cweeBaseObj {
	static_assert(std::is_base_of<cweeBaseObj, T>::value, "Template Type 'T' must derive from cweeBaseObj");

public:
	typedef cweeUnorderedListReferenceObject<T> _iterType;
	struct it_state {
		mutable cweeThreadedList<int>	listOfIndexes;
		mutable int pos = 0;
		mutable _iterType iter;
		inline void begin(const cweeListObj* ref) { listOfIndexes = ref->GetList(); pos = 0; }
		inline void next(const cweeListObj* ref) { ++pos; }
		inline void end(const cweeListObj* ref) { listOfIndexes = ref->GetList(); pos = listOfIndexes.Num(); }
		inline _iterType& get(cweeListObj* ref) { iter = ref->At(listOfIndexes[pos]); return iter; }
		inline bool cmp(const it_state& s) const { return (pos == s.pos) ? false : true; }
		inline long long distance(const it_state& s) const { return pos - s.pos; };
		// Optional to allow operator--() and reverse iterators:
		inline void prev(const cweeListObj* ref) { --pos; }
		// Optional to allow `const_iterator`:
		inline const _iterType& get(const cweeListObj* ref) const { iter = ref->At(listOfIndexes[pos]); return iter; }
	};
	SETUP_STL_ITERATOR(cweeListObj, _iterType, it_state);

	cweeListObj() : cweeBaseObj(), data(new cweeUnorderedList<T>()) { IncrementLevel(); };
	~cweeListObj() {};

	virtual cweeStr		ToString()  const {
		cweeStr out;

		for (auto& x : data->GetList()) {
			auto ptr = this->operator[](x);
			if (ptr) {
				out.AddToDelimiter(ptr->ToString(), ", ");
			}
		}

		out.Insert("[", 0);
		out.Append("]");

		return out;
	};
	virtual void		Deserialize(cweeStr& inbound) {
		Clear();

		cweeStr delim = cweeStr::printf(":%s_%i:", Type().c_str(), GetLevel());
		delim.ReplaceInline(" ", "");
		cweeParser x(inbound, delim, true);
		inbound.Clear();
		for (int i = 0; (i + 1) < x.getNumVars(); i += 2) {
			int index = (int)x[i];

			T obj = T();
			obj.SetLevel(GetLevel() + 1);
			obj.Deserialize(x[i + 1]);
			obj.parent = cweeSharedPtr<cweeListObj>(this, [](cweeListObj* p) {/*Do not delete 'this'.*/});

			cweeUnorderedList<T>* ptr = data.get();
			if (ptr) {
				ptr->Lock(); // locked append to ensure the full object 'appears' suddenly without missing data
				ptr->UnsafeAppendAt(index);
				T* dPtr = ptr->UnsafeRead(index);
				if (dPtr) {
					*dPtr = obj;
				}
				ptr->Unlock();
			}
		}
	};
	virtual cweeStr		Serialize()  const {
		cweeStr out; cweeStr delim = cweeStr::printf(":%s_%i:", Type().c_str(), GetLevel());
		delim.ReplaceInline(" ", "");
		for (auto& x : data->GetList()) {
			auto ptr = this->operator[](x);
			if (ptr) {
				out.AddToDelimiter(x, delim);
				out.AddToDelimiter(ptr->Serialize(), delim);
			}
		}
		return out;
	};
	virtual void		Update() {
		for (auto& x : data->GetList()) {
			auto ptr = this->operator[](x);
			if (ptr) {
				ptr->Update();
			}
		}
	};
	virtual void		IncrementLevel() {
		level.Increment();
		for (auto& x : data->GetList()) {
			auto ptr = this->operator[](x);
			if (ptr) {
				ptr->IncrementLevel();
			}
		}
	};
	virtual void		DecrementLevel() {
		level.Decrement();
		for (auto& x : data->GetList()) {
			auto ptr = this->operator[](x);
			if (ptr) {
				ptr->DecrementLevel();
			}
		}
	};
	virtual void		SetLevel(int v) {
		level.SetValue(v);
		for (auto& x : data->GetList()) {
			auto ptr = this->operator[](x);
			if (ptr) {
				ptr->SetLevel(v);
			}
		}
	};
	virtual void		Clear() {
		data->Clear();
	};
	// copy operator
	auto& operator=(const cweeListObj<T>& obj) {
		Clear();

		for (auto& x : obj.GetList()) {
			auto ptr = obj[x];
			if (ptr) {
				T out;
				out = *ptr.Ptr(); // copy operator

				data->Lock(); // locked append to ensure the full object 'appears' suddenly without missing data
				data->UnsafeAppendAt(x);
				T* dPtr = data->UnsafeRead(x);
				if (dPtr) {
					*dPtr = out;
					dPtr->SetLevel(GetLevel() + 1);
					dPtr->parent = cweeSharedPtr<cweeListObj>(this, [](cweeListObj* p) {/*Do not delete 'this'.*/});
				}
				data->Unlock();
			}
		}

		return *this;
	};

	virtual int			Num() { return data->Num(); };
	virtual cweeThreadedList<int>	GetList() const {
		return data->GetList();
	};
	virtual int			Append(const T& obj) {
		int index = -1;

		data->Lock();
		index = data->UnsafeAppend();
		T* dPtr = data->UnsafeRead(index);
		if (dPtr) {
			*dPtr = obj;
			dPtr->parent = cweeSharedPtr<cweeListObj>(this, [](cweeListObj* p) {/*Do not delete 'this'.*/});
			dPtr->SetLevel(GetLevel() + 1);
		}
		data->Unlock();

		return index;
	};
	virtual int			Append() {
		int index = -1;

		data->Lock();
		index = data->UnsafeAppend();
		T* dPtr = data->UnsafeRead(index);
		if (dPtr) {
			dPtr->parent = cweeSharedPtr<cweeListObj>(this, [](cweeListObj* p) {/*Do not delete 'this'.*/});
			dPtr->SetLevel(GetLevel() + 1);
		}
		data->Unlock();

		return index;
	};
	virtual void		Erase(int index) {
		data->Lock();
		data->UnsafeErase(index);
		data->Unlock();
	};
	virtual cweeUnorderedListReferenceObject<T> operator[](int index) const {
		return cweeUnorderedListReferenceObject<T>(data, index);
	};
	virtual cweeUnorderedListReferenceObject<T> operator[](const cweeStr& name) const {
		return cweeUnorderedListReferenceObject<T>(data, FindIndexWithName(name));
	};
	virtual cweeUnorderedListReferenceObject<T> At(int index) const {
		return operator[](index);
	};
	virtual cweeUnorderedListReferenceObject<T> At(const cweeStr& name) const {
		return operator[](name);
	};
	virtual int			FindIndexWithName(const cweeStr& name) const {
		return data->FindIndexWithName(name);
	};

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](const int* x){ return (*x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
	}
	*/
	cweeThreadedList<cweeUnorderedListReferenceObject<T>> Select(std::function<bool(const T*)> predicate) const {
		cweeThreadedList<cweeUnorderedListReferenceObject<T>> out;		
		cweeUnorderedListReferenceObject<T> x;
		for (auto& i : GetList()){
			x = this->operator[](i);

			if (x && predicate(x.Ptr())) {
				out.Append(x);
			}
		}
		return out;
	};

	/*
	Lambda-based select function that provides the pointers to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int*> ptrsWithValuesGreaterThanFive = obj.Select([](const int* x){ return (*x > 5); });
	for (auto& z : ptrsWithValuesGreaterThanFive){
		std::cout << *z << std::endl;
		*z = -1; // modifies the original list
	}
	*/
	cweeThreadedList<cweeUnorderedListReferenceObject<T>> Select(std::function<bool(const T*)> predicate) {
		cweeThreadedList<cweeUnorderedListReferenceObject<T>> out;
		cweeUnorderedListReferenceObject<T> x; 
		for (auto& i : GetList()) {
			x = this->operator[](i);

			if (x && predicate(x.Ptr())) {
				out.Append(x);
			}
		}
		return out;
	};

	/*
	Lambda-based select function that provides the indexes to underlying data that meets the required lambda function

	cweeThreadedList<int> obj;
	obj = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	cweeThreadedList<int> indexesWithValuesGreaterThanFive = obj.SelectIndexes([](const int* x){ return (*x > 5); });
	for (auto& z : indexesWithValuesGreaterThanFive){
		std::cout << obj[z] << std::endl;
	}
	*/
	cweeThreadedList<int> SelectIndexes(std::function<bool(const T*)> predicate) const {
		cweeThreadedList<int> out;
		cweeUnorderedListReferenceObject<T> x; 
		for (auto& i : GetList()) {
			x = this->operator[](i);
			if (x && predicate(x.Ptr())) {
				out.Append(i);
			}
		}
		return out;
	};

protected:
	cweeSharedPtr<cweeUnorderedList<T>> data;

};

#endif