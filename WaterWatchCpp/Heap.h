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

enum memTag_t {
#define MEM_TAG( x )	TAG_##x,
	MEM_TAG(UNSET)			// This should never be used
	MEM_TAG(DEBUG)			// Memory we don't care about, because it won't be in a retail build
	MEM_TAG(NEW)			// Memory allocated with new which hasn't been given an explicit tag
	MEM_TAG(BLOCKALLOC)		// Memory allocated with cweeBlockAlloc which hasn't been given an explicit tag
	MEM_TAG(LIST)			// Memory used by cweeThreadedLists
	MEM_TAG(TEMP)			// Memory which should be automatically freed at the end of the function
	MEM_TAG(MATH)			// Memory used by VecX and MatX
	MEM_TAG(FX)				// Memory used by VecX and MatX
	MEM_TAG(STRING)			// Memory used by strings
	MEM_TAG(JOBLIST)		// Memory used by mutlithreading job lists
	MEM_TAG(RUNTIME_DB)		// Memory used by mutlithreading job lists
#undef MEM_TAG
	TAG_NUM_TAGS,
};

//#define	FORCE_DISCRETE_BLOCK_ALLOCS
//#define	FORCE_INIT_LOCAL_MEM

static const int MAX_TAGS = 256;

#undef			new
#undef			delete

INLINE void* Mem_Alloc16(const size_t& size, const memTag_t& tag) { if (!size) return NULL; const size_t paddedSize = (size + 15) & ~15; return _aligned_malloc(paddedSize, 16); };
INLINE void Mem_Free16(void* ptr) { if (ptr) _aligned_free(ptr); };
INLINE void* Mem_ClearedAlloc(const size_t& size, const memTag_t& tag) { void* mem = Mem_Alloc16(size, tag); memset(mem, 0, size); return mem; };
INLINE void		Mem_Free(void* ptr) { Mem_Free16(ptr); }
#ifndef FORCE_INIT_LOCAL_MEM
INLINE void* Mem_Alloc(const size_t& size, const memTag_t& tag) { return Mem_Alloc16(size, tag); }
#else
INLINE void* Mem_Alloc(const size_t size, const memTag_t tag) { return Mem_ClearedAlloc(size, tag); }
#endif
INLINE char* Mem_CopyString(const char* in) { char* out = (char*)Mem_Alloc((size_t)(strlen(in) + 1), TAG_STRING); strcpy(out, in); return out; };


void* operator new(size_t s);
void  operator delete(void* p);
void* operator new[](size_t s);
void  operator delete[](void* p);

void* operator new(size_t s, memTag_t tag);
void  operator delete(void* p, memTag_t tag);
void* operator new[](size_t s, memTag_t tag);
void  operator delete[](void* p, memTag_t tag);

INLINE void* operator new(size_t s) { return Mem_Alloc(s, TAG_NEW); }
INLINE void		operator delete(void* p) { Mem_Free(p); }
INLINE void* operator new[](size_t s) { return Mem_Alloc(s, TAG_NEW); }
INLINE void		operator delete[](void* p) { Mem_Free(p); }

INLINE void* operator new(size_t s, memTag_t tag) { return Mem_Alloc(s, tag); }
INLINE void		operator delete(void* p, memTag_t tag) { Mem_Free(p); }
INLINE void* operator new[](size_t s, memTag_t tag) { return Mem_Alloc(s, tag); }
INLINE void		operator delete[](void* p, memTag_t tag) { Mem_Free(p); }

/*
================================================
cweeTempArray is an array that is automatically free'd when it goes out of scope.
There is no "cast" operator because these are very unsafe.

The template parameter MUST BE POD!

Compile time asserting POD-ness of the template parameter is complicated due
to our vector classes that need a default constructor but are otherwise
considered POD.
================================================
*/
template < class T >
class cweeTempArray {
public:
	cweeTempArray(cweeTempArray<T>& other) {
		this->num = other.num;
		this->buffer = other.buffer;
		other.num = 0;
		other.buffer = NULL;
	};
	cweeTempArray(size_t num) {
		this->num = num;
		buffer = (T*)Mem_Alloc((size_t)(num * sizeof(T)), TAG_TEMP);
	};

	~cweeTempArray() {
		Mem_Free(buffer);
	};

	T& operator [](size_t i) { assert(i < num); return buffer[i]; }
	const T& operator [](size_t i) const { assert(i < num); return buffer[i]; }

	T* Ptr() { return buffer; }
	const T* Ptr() const { return buffer; }

	size_t Size() const { return (size_t)(num * sizeof(T)); }
	unsigned int Num() const { return num; }

	void Zero() { memset(Ptr(), 0, Size()); }

private:
	T* buffer;		// Ensure this buffer comes first, so this == &this->buffer
	size_t	num;
};

#pragma region cweeUnion
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
template<typename... Args> class cweeUnion {
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
	cweeUnion() noexcept : data{ 0 } { Alloc(&data[0]); };
	/* ALLOC THE UNIONED DATA FROM PARAMETERS */ template <typename T, typename = std::enable_if_t<!std::is_same_v<typename std::remove_reference<T>::type, cweeUnion>>, typename... TArgs>
	explicit cweeUnion(T&& b, TArgs&&... a) noexcept : data{ 0 } {
		InitAndSet(std::forward<T>(b), std::forward<TArgs>(a)...);
	};

	/* INIT AND COPY THE UNIONED DATA FROM ANOTHER UNION */ 
	cweeUnion(cweeUnion const& a) noexcept : data{ 0 } { InitAndCopy(a); };
	/* INIT AND TAKE THE UNIONED DATA FROM ANOTHER UNION */ 
	cweeUnion(cweeUnion&& a) noexcept : data{ 0 } { InitAndTake(std::forward<cweeUnion>(a)); };
	/* COPY THE UNIONED DATA FROM ANOTHER UNION */ 
	cweeUnion& operator=(cweeUnion const& a) { Copy(a); return *this; };
	/* TAKE THE UNIONED DATA FROM ANOTHER UNION */ 
	cweeUnion& operator=(cweeUnion&& a) { Take(std::forward<cweeUnion>(a)); return *this; };
	/* DESTROY THE UNION ONCE OUT-OF-SCOPE */ 
	~cweeUnion() { Delete(); };

	/* GET A REFERENCE TO THE N'th ITEM */  template<int N> 
	constexpr NthTypeOf<N>& get() const noexcept {
		static_assert(N < num_parameters && N >= 0, "Cannot access parameters beyond the allocated buffer.");
		constexpr size_t index = SizeOfFirstN<N>();
		return *((NthTypeOf<N>*)(void*)(&data[index]));

		// constexpr NthTypeOf<N>* out = static_cast<NthTypeOf<N>*>(static_cast<void*>((static_cast<byte*>(&const_cast<byte&>(data[SizeOfFirstN<N>()])))));
		// return *out;
	};
	/* GET THE SIZE (in bytes) OF THE ENTIRE UNION */ 
	static constexpr size_t size() noexcept { return SizeOfAll(); };
	/* GET THE SIZE (in bytes) OF THE N'th ITEM */ template<int N> 
	static constexpr size_t size() noexcept { return sizeof(NthTypeOf<N>); };

	/* COMPARE WITH ANOTHER UNION */
	friend bool operator==(cweeUnion const& a, cweeUnion const& b) { 
		return Equals(a,b);
	};
	friend bool operator!=(cweeUnion const& a, cweeUnion const& b) {
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
		static_assert((sizeof...(TArgs)) == num_parameters, "cweeUnion initializer must use the same number of parametrers as are defined in the Union.");
		InitAndSetDataWith(a...);
	};
	/* SET WITH PARAMETER PACK */ template <typename... TArgs> void InitAndSet(TArgs&&... a) {
		static_assert((sizeof...(TArgs)) == num_parameters, "cweeUnion initializer must use the same number of parametrers as are defined in the Union.");
		InitAndSetDataWith(std::forward<TArgs>(a)...);
	};

	/* SET DATA USING PARAMETER */ template <int N> void SetAt(NthTypeOf<N> const& a) { this->get<N>() = a; };
	/* SET DATA USING PARAMETER */ template <int N> void SetAt(NthTypeOf<N>&& a) { this->get<N>() = std::forward<NthTypeOf<N>>(a); };
	/* EMPTY PARAMETER PACK -> END RECURSION */ void SetDataWith() { return; };
	/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void SetDataWith(const T& value, Targs&&... Fargs) { SetAt<num_parameters - (1 + sizeof...(Fargs))>(value); SetDataWith(std::forward<Targs>(Fargs)...); };
	/* RECURSIVELY UNPACK THE PARAMETER PACK */ template<typename T, typename... Targs> void SetDataWith(T&& value, Targs&&... Fargs) { SetAt<num_parameters - (1 + sizeof...(Fargs))>(std::forward<T>(value)); SetDataWith(std::forward<Targs>(Fargs)...); };
	/* SET WITH PARAMETER PACK */ template <typename... TArgs> void Set(TArgs const&... a) {
		static_assert((sizeof...(TArgs)) == num_parameters, "cweeUnion initializer must use the same number of parametrers as are defined in the Union.");
		SetDataWith(a...);
	};
	/* SET WITH PARAMETER PACK */ template <typename... TArgs> void Set(TArgs&&... a) {
		static_assert((sizeof...(TArgs)) == num_parameters, "cweeUnion initializer must use the same number of parametrers as are defined in the Union.");
		SetDataWith(std::forward<TArgs>(a)...);
	};
#pragma endregion
#pragma region COMPARE WITH CONST&
	template <int N> static bool EqualsAt(cweeUnion const& a, cweeUnion const& b) { 
		return a.get<N>() == b.get<N>();
	};
	static bool Equals(cweeUnion const& a, cweeUnion const& b) {
		bool out = true;
		if constexpr (num_parameters >= 1) { out = out && EqualsAt<0>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 2) {  out = out && EqualsAt<1>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 3) {  out = out && EqualsAt<2>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 4) {  out = out && EqualsAt<3>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 5) {  out = out && EqualsAt<4>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 6) {  out = out && EqualsAt<5>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 7) {  out = out && EqualsAt<6>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 8) {  out = out && EqualsAt<7>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 9) {  out = out && EqualsAt<8>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 10) {  out = out && EqualsAt<9>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 11) {  out = out && EqualsAt<10>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 12) {  out = out && EqualsAt<11>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 13) {  out = out && EqualsAt<12>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 14) {  out = out && EqualsAt<13>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 15) {  out = out && EqualsAt<14>(a, b); if (!out) return out; }
		if constexpr (num_parameters >= 16) {  out = out && EqualsAt<15>(a, b); if (!out) return out; }
		return out;
	};
#pragma endregion
#pragma region COPY FROM CONST&
	template <int N> void InitAndCopyAt(cweeUnion const& a) { InstantiateData(PtrAt<N>(&data[0]), a.get<N>()); };
	void InitAndCopy(cweeUnion const& a) {
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

	template <int N> void CopyAt(cweeUnion const& a) { this->get<N>() = a.get<N>(); };
	void Copy(cweeUnion const& a) {
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
	template <int N> void InitAndTakeAt(cweeUnion&& a) {
		InstantiateData(PtrAt<N>(&data[0]), std::move(a.get<N>()));
		if constexpr (!isPod<NthTypeOf<N>>()) { a.Clear<N>(); }
	};
	void InitAndTake(cweeUnion&& a) {
		if constexpr (num_parameters >= 1) { InitAndTakeAt<0>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 2) { InitAndTakeAt<1>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 3) { InitAndTakeAt<2>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 4) { InitAndTakeAt<3>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 5) { InitAndTakeAt<4>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 6) { InitAndTakeAt<5>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 7) { InitAndTakeAt<6>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 8) { InitAndTakeAt<7>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 9) { InitAndTakeAt<8>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 10) { InitAndTakeAt<9>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 11) { InitAndTakeAt<10>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 12) { InitAndTakeAt<11>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 13) { InitAndTakeAt<12>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 14) { InitAndTakeAt<13>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 15) { InitAndTakeAt<14>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 16) { InitAndTakeAt<15>(std::forward<cweeUnion>(a)); }
		a.Clear();
	};

	template <int N> void TakeAt(cweeUnion&& a) {
		get<N>() = std::move(a.get<N>());
		if constexpr (!isPod<NthTypeOf<N>>()) { a.Clear<N>(); }
	};
	void Take(cweeUnion&& a) {
		if constexpr (num_parameters >= 1) { TakeAt<0>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 2) { TakeAt<1>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 3) { TakeAt<2>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 4) { TakeAt<3>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 5) { TakeAt<4>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 6) { TakeAt<5>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 7) { TakeAt<6>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 8) { TakeAt<7>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 9) { TakeAt<8>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 10) { TakeAt<9>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 11) { TakeAt<10>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 12) { TakeAt<11>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 13) { TakeAt<12>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 14) { TakeAt<13>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 15) { TakeAt<14>(std::forward<cweeUnion>(a)); }
		if constexpr (num_parameters >= 16) { TakeAt<15>(std::forward<cweeUnion>(a)); }
		a.Clear();
	};
#pragma endregion
};
template <typename... Args> inline cweeUnion<Args...> make_cwee_union(Args&&... Fargs) { return cweeUnion<Args...>(std::forward<Args>(Fargs)...); };
#pragma endregion

#pragma region cweeTogglePtr
template<typename type1, typename type2> class cweeTogglePtr {
private:
	cweeUnion<bool, void*> data;

public:
	cweeTogglePtr() : data(cweeUnion<bool, void*>(false, nullptr)) {};

	bool Mode() const {
		return data.get<0>();
	};

	template<typename T>
	T* TryGet() const {
		if constexpr (std::is_same<T, type1>::value) { // requesting first type
			if (!data.get<0>() && data.get<1>()) {
				return static_cast<type1*>(data.get<1>());
			}
		}
		else if constexpr (std::is_same<T, type2>::value) { // requesting second type
			if (data.get<0>() && data.get<1>()) {
				return static_cast<type2*>(data.get<1>());
			}
		}
		return nullptr;

	};

	template<typename T>
	void Set(T* incoming) {
		if constexpr (std::is_same<T, type1>::value) { // setting first type
			data.get<0>() = false;
			data.get<1>() = (void*)incoming;
		}
		else if constexpr (std::is_same<T, type2>::value) { // setting second type
			data.get<0>() = false;
			data.get<1>() = (void*)incoming;
		}
	};
};
#pragma endregion

template<typename A, typename B>
class cweePair {
public:
	using first_type = typename A;
	using second_type = typename B;

	cweePair<A, B>() {};
	cweePair<A, B>(const A& in1, const B& in2) {
		first = in1;
		second = in2;
	};

	cweePair<A, B>& operator=(const cweePair<A, B>& other) {
		first = other.first;
		second = other.second;

		return *this;
	};
	friend bool			operator==(const cweePair& a, const cweePair& b) {
		if ((b.first == a.first) && (b.second == a.second)) return true;
		return false;
	};
	friend bool			operator!=(const cweePair& a, const cweePair& b) {
		return !operator==(a);
	};

	template< typename _otherA_, typename _otherB_ >
	operator cweePair<_otherA_, _otherB_>() const {
		cweePair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};
	template< typename _otherA_, typename _otherB_ >
	operator cweePair<_otherA_, _otherB_>() {
		cweePair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};

	template< typename _otherA_, typename _otherB_ >
	operator std::pair<_otherA_, _otherB_>() const {
		std::pair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};
	template< typename _otherA_, typename _otherB_ >
	operator std::pair<_otherA_, _otherB_>() {
		std::pair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};

	mutable A first;
	mutable B second;
};

/*
================================================
cweeBlockAlloc is a block-based allocator for fixed-size objects.
All objects are properly constructed and destructed.
================================================
*/
#define BLOCK_ALLOC_ALIGNMENT 16

template<class _type_, int _blockSize_, memTag_t memTag = TAG_BLOCKALLOC>
class cweeBlockAlloc {
public:
	cweeBlockAlloc(bool clear = true) : // = false
		blocks(NULL),
		free(NULL),
		total(0),
		active(0),
		allowAllocs(true),
		clearAllocs(clear)
	{};
	cweeBlockAlloc(int toReserve) :
		blocks(NULL),
		free(NULL),
		total(0),
		active(0),
		allowAllocs(true),
		clearAllocs(true)
	{
		Reserve(toReserve);
	};
	~cweeBlockAlloc() {
		Shutdown();
	};

	// returns total size of allocated memory
	size_t				Allocated() const { return total * sizeof(_type_); }

	// returns total size of allocated memory including size of (*this)
	size_t				Size() const { return sizeof(*this) + Allocated(); }

	void				Shutdown() {
		while (blocks != NULL) {
			cweeBlock* block = blocks;
			blocks = blocks->next;
			Mem_Free(block);
		}
		blocks = NULL;
		free = NULL;
		total = active = 0;
	};
	INLINE void			SetFixedBlocks(long long numBlocks);
	INLINE void			FreeEmptyBlocks();

	static constexpr bool isPod() { return std::is_pod<_type_>::value; };
	_type_*				Alloc() {
#ifdef FORCE_DISCRETE_BLOCK_ALLOCS
		// for debugging tools
		return new _type_;
#else
		if (free == NULL) {
			if (!allowAllocs) {
				return NULL;
			}
			AllocNewBlock();
		}

		active++;
		element_t* element = free;
		free = free->next;
		element->next = NULL;

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
#endif
	};
	template <typename T, typename... Fargs>
	_type_*				Alloc(T arg1, Fargs... Args) {
#ifdef FORCE_DISCRETE_BLOCK_ALLOCS
		// for debugging tools
		return new _type_(arg1, Args...);
#else
		if (free == NULL) {
			if (!allowAllocs) {
				return NULL;
			}
			AllocNewBlock();
		}

		active++;
		element_t* element = free;
		free = free->next;
		element->next = NULL;

		_type_* t = (_type_*)element->buffer;
		new (t) _type_(arg1, Args...);
		
		return t;
#endif
	};
	void				Free(_type_* element) {
#ifdef FORCE_DISCRETE_BLOCK_ALLOCS
		// for debugging tools
		delete element;
#else
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
#endif
	};
	INLINE void			Reserve(long long num) {
#ifdef FORCE_DISCRETE_BLOCK_ALLOCS
		// for debugging tools	
#else
#if 0
		while (total < num) {
			AllocNewBlock();
		}
#else
#if 0
		num -= total;
		if (num > 0) {
			cweeTempArray< _type_* > arr = cweeTempArray< _type_* >(num);
			for (long long i = 0; i < num; i++) {
				arr[i] = Alloc();
			}
			for (long long i = 0; i < num; i++) {
				Free(arr[i]);
			}
		}
#else
		if (total < num) {
			std::vector< _type_* > arr; arr.reserve(2 * (num - total));
			while (total < num) {
				arr.push_back(Alloc());
			}
			for (_type_* p : arr) {
				Free(p);
			}
		}
#endif
#endif


#endif
	};
	long long			GetTotalCount() const { return total; }
	long long			GetAllocCount() const { return active; }
	long long			GetFreeCount() const { return total - active; }

private:
	union element_t {
		_type_* data;
		element_t* next;
		::byte			buffer[(CONST_MAX(sizeof(_type_), sizeof(element_t*)) + (BLOCK_ALLOC_ALIGNMENT - 1)) & ~(BLOCK_ALLOC_ALIGNMENT - 1)];
	};

	class cweeBlock {
	public:
		element_t		elements[_blockSize_];
		cweeBlock*		next;
		element_t*		free;		// list with free elements in this block (temp used only by FreeEmptyBlocks)
		long long		freeCount;	// number of free elements in this block (temp used only by FreeEmptyBlocks)
	};

	cweeBlock* blocks;
	element_t* free;
	long long			total;
	long long			active;
	bool				allowAllocs;
	bool				clearAllocs;

	void			AllocNewBlock() {
		cweeBlock* block = (cweeBlock*)Mem_Alloc((size_t)(sizeof(cweeBlock)), memTag);
		block->next = blocks;
		blocks = block;
		for (int i = 0; i < _blockSize_; i++) {
			block->elements[i].next = free;
			free = &block->elements[i];
			assert((((UINT_PTR)free) & (BLOCK_ALLOC_ALIGNMENT - 1)) == 0);
		}
		total += _blockSize_;
	};
};

template<class _type_, int _blockSize_, memTag_t memTag>
INLINE void cweeBlockAlloc<_type_, _blockSize_, memTag>::SetFixedBlocks(long long numBlocks) {
	long long currentNumBlocks = 0;
	for (cweeBlock* block = blocks; block != NULL; block = block->next) {
		currentNumBlocks++;
	}
	for (long long i = currentNumBlocks; i < numBlocks; i++) {
		AllocNewBlock();
	}
	allowAllocs = false;
}

template<class _type_, int _blockSize_, memTag_t memTag>
INLINE void cweeBlockAlloc<_type_, _blockSize_, memTag>::FreeEmptyBlocks() {
	// first count how many free elements are in each block and build up a free chain per block
	for (cweeBlock* block = blocks; block != NULL; block = block->next) {
		block->free = NULL;
		block->freeCount = 0;
	}
	for (element_t* element = free; element != NULL; ) {
		element_t* next = element->next;
		for (cweeBlock* block = blocks; block != NULL; block = block->next) {
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
	cweeBlock* prevBlock = NULL;
	for (cweeBlock* block = blocks; block != NULL; ) {
		cweeBlock* next = block->next;
		if (block->freeCount == _blockSize_) {
			if (prevBlock == NULL) {
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
	free = NULL;
	for (cweeBlock* block = blocks; block != NULL; block = block->next) {
		for (element_t* element = block->free; element != NULL; ) {
			element_t* next = element->next;
			element->next = free;
			free = element;
			element = next;
		}
	}
}

/*
==============================================================================

	Dynamic allocator, simple wrapper for normal allocations which can
	be interchanged with cweeDynamicBlockAlloc.

	No constructor is called for the 'type'.
	Allocated blocks are always 16 byte aligned.

==============================================================================
*/

template<class _type_, size_t BlockSize = 128>
class cweeAllocWrapper {
public:
	cweeAllocWrapper() : alloc() {
		alloc.Free(alloc.Alloc());
	};
	~cweeAllocWrapper() {};

	_type_* Alloc() {
		AUTO p = alloc.Alloc();
		return p;
	};
	template <typename T, typename... Fargs>
	_type_* Alloc(T arg1, Fargs... Args) {
		AUTO p = alloc.Alloc(arg1, Args...);
		return p;
	};
	void	Free(_type_* element) {
		alloc.Free(element);
	};
	void	Clean() {
		alloc.FreeEmptyBlocks();
	};
	long long	GetTotalCount() {
		long long out;
		out = alloc.GetTotalCount();
		return out;
	};
	long long	GetAllocCount() {
		long long out;
		out = alloc.GetAllocCount();
		return out;
	};
	void	Clear() {
		// alloc.Shutdown();
	};
	void	Reserve(long long n) {
		alloc.Reserve(n);
	};

private:
	cweeBlockAlloc<_type_, BlockSize> alloc;
};

template<class _type_, size_t BlockSize = 128>
class cweeAlloc {
private:
	static constexpr bool isPod() { return std::is_pod<_type_>::value; };
public:
	cweeAlloc() : lock(), ptrs(), alloc() {};
	cweeAlloc(int toReserve) : lock(), ptrs(), alloc(toReserve) {};
	~cweeAlloc() { Clear(); };

	_type_* Alloc() {
		lock.lock();
		AUTO p = alloc.Alloc();
		if constexpr (!isPod()) {
			ptrs.insert(p);
		}
		lock.unlock();
		return p;
	};
	template <typename T, typename... Fargs>
	_type_* Alloc(T arg1, Fargs... Args) {
		lock.lock();
		AUTO p = alloc.Alloc(arg1, Args...);
		if constexpr (!isPod()) {
			ptrs.insert(p);
		}
		lock.unlock();
		return p;
	};
	void	Free(_type_* element) {
		lock.lock();
		if constexpr (!isPod()) {
			ptrs.erase(element);
		}
		alloc.Free(element);
		lock.unlock();
	};
	void	Clean() {
		lock.lock();
		alloc.FreeEmptyBlocks();
		lock.unlock();
	};
	long long	GetTotalCount() const {
		long long out;
		lock.lock();
		out = alloc.GetTotalCount();
		lock.unlock();
		return out;
	};
	long long	GetAllocCount() const {
		long long out;
		lock.lock();
		if constexpr (!isPod()) {
			out = ptrs.size();
		}
		else {
			out = alloc.GetAllocCount();
		}
		lock.unlock();
		return out;
	};
	void	Clear() {
		lock.lock();
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
		lock.unlock();
	};
	void	Reserve(long long n) {
		lock.lock();
		alloc.Reserve(n);
		lock.unlock();
	};

private:
	mutable std::mutex lock;
	std::set<_type_*> ptrs;
	cweeBlockAlloc<_type_, BlockSize> alloc;
};


template<class type, int baseBlockSize, int minBlockSize>
class cweeDynamicAlloc {
public:
	cweeDynamicAlloc();
	~cweeDynamicAlloc();

	void							Init();
	void							Shutdown();
	void							SetFixedBlocks(int numBlocks) {}
	void							SetLockMemory(bool lock) {}
	void							FreeEmptyBaseBlocks() {}

	type* Alloc(const int num);
	type* Resize(type* ptr, const int num);
	void							Free(type* ptr);
	const char* CheckMemory(const type* ptr) const;

	int								GetNumBaseBlocks() const { return 0; }
	int								GetBaseBlockMemory() const { return 0; }
	int								GetNumUsedBlocks() const { return numUsedBlocks; }
	int								GetUsedBlockMemory() const { return usedBlockMemory; }
	int								GetNumFreeBlocks() const { return 0; }
	int								GetFreeBlockMemory() const { return 0; }
	int								GetNumEmptyBaseBlocks() const { return 0; }

private:
	int								numUsedBlocks;			// number of used blocks
	int								usedBlockMemory;		// total memory in used blocks

	int								numAllocs;
	int								numResizes;
	int								numFrees;

	void							Clear();
};

template<class type, int baseBlockSize, int minBlockSize>
cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::cweeDynamicAlloc() {
	Clear();
}

template<class type, int baseBlockSize, int minBlockSize>
cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::~cweeDynamicAlloc() {
	Shutdown();
}

template<class type, int baseBlockSize, int minBlockSize>
void cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::Init() {
}

template<class type, int baseBlockSize, int minBlockSize>
void cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::Shutdown() {
	Clear();
}

template<class type, int baseBlockSize, int minBlockSize>
type* cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::Alloc(const int num) {
	numAllocs++;
	if (num <= 0) {
		return NULL;
	}
	numUsedBlocks++;
	usedBlockMemory += num * sizeof(type);
	return static_cast<type*>(Mem_Alloc16((size_t)(num * sizeof(type)), TAG_BLOCKALLOC));
}

template<class type, int baseBlockSize, int minBlockSize>
type* cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::Resize(type* ptr, const int num) {

	numResizes++;

	if (ptr == NULL) {
		return Alloc(num);
	}

	if (num <= 0) {
		Free(ptr);
		return NULL;
	}

	assert(0);
	return ptr;
}

template<class type, int baseBlockSize, int minBlockSize>
void cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::Free(type* ptr) {
	numFrees++;
	if (ptr == NULL) {
		return;
	}
	Mem_Free16(ptr);
}

template<class type, int baseBlockSize, int minBlockSize>
const char* cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::CheckMemory(const type* ptr) const {
	return NULL;
}

template<class type, int baseBlockSize, int minBlockSize>
void cweeDynamicAlloc<type, baseBlockSize, minBlockSize>::Clear() {
	numUsedBlocks = 0;
	usedBlockMemory = 0;
	numAllocs = 0;
	numResizes = 0;
	numFrees = 0;
}

/*
==============================================================================

	Fast dynamic block allocator.

	No constructor is called for the 'type'.
	Allocated blocks are always 16 byte aligned.

==============================================================================
*/

#include "BTree.h"

template<class type>
class cweeDynamicBlock {
public:
	type* GetMemory() const { return (type*)(((::byte*)this) + sizeof(cweeDynamicBlock<type>)); }
	int								GetSize() const { return abs(size); }
	void							SetSize(int s, bool isBaseBlock) { size = isBaseBlock ? -s : s; }
	bool							IsBaseBlock() const { return (size < 0); }
	int								size = 0;					// size in bytes of the block
	cweeDynamicBlock<type>* prev = NULL;					// previous memory block
	cweeDynamicBlock<type>* next = NULL;					// next memory block
	cweeBTreeNode<cweeDynamicBlock<type>, int>* node = NULL;			// node in the B-Tree with free blocks
};

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_ = TAG_BLOCKALLOC>
class cweeDynamicBlockAlloc {
public:
	cweeDynamicBlockAlloc();
	~cweeDynamicBlockAlloc();

	void							Init();
	void							Shutdown();
	void							SetFixedBlocks(int numBlocks);
	void							SetLockMemory(bool lock);
	void							FreeEmptyBaseBlocks();

	type* Alloc(const int num);
	type* Resize(type* ptr, const int num);
	void							Free(type* ptr);
	const char* CheckMemory(const type* ptr) const;

	int								GetNumBaseBlocks() const { return numBaseBlocks; }
	int								GetBaseBlockMemory() const { return baseBlockMemory; }
	int								GetNumUsedBlocks() const { return numUsedBlocks; }
	int								GetUsedBlockMemory() const { return usedBlockMemory; }
	int								GetNumFreeBlocks() const { return numFreeBlocks; }
	int								GetFreeBlockMemory() const { return freeBlockMemory; }
	int								GetNumEmptyBaseBlocks() const;

private:
	cweeDynamicBlock<type>* firstBlock;				// first block in list in order of increasing address
	cweeDynamicBlock<type>* lastBlock;				// last block in list in order of increasing address
	cweeBTree<cweeDynamicBlock<type>, int, 4>freeTree;			// B-Tree with free memory blocks
	bool							allowAllocs = true;			// allow base block allocations
	bool							lockMemory = false;				// lock memory so it cannot get swapped out
	int								numBaseBlocks = 0;			// number of base blocks
	int								baseBlockMemory = 0;		// total memory in base blocks
	int								numUsedBlocks = 0;			// number of used blocks
	int								usedBlockMemory = 0;		// total memory in used blocks
	int								numFreeBlocks = 0;			// number of free blocks
	int								freeBlockMemory = 0;		// total memory in free blocks

	int								numAllocs = 0;
	int								numResizes = 0;
	int								numFrees = 0;

	memTag_t						tag;

	void							Clear();
	cweeDynamicBlock<type>* AllocInternal(const int num);
	cweeDynamicBlock<type>* ResizeInternal(cweeDynamicBlock<type>* block, const int num);
	void							FreeInternal(cweeDynamicBlock<type>* block);
	void							LinkFreeInternal(cweeDynamicBlock<type>* block);
	void							UnlinkFreeInternal(cweeDynamicBlock<type>* block);
	void							CheckMemory() const;
};


template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::cweeDynamicBlockAlloc() {
	tag = _tag_;
	Clear();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::~cweeDynamicBlockAlloc() {
	Shutdown();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::Init() {
	freeTree.Init();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::Shutdown() {
	cweeDynamicBlock<type>* block;

	for (block = firstBlock; block != NULL; block = block->next) {
		if (block->node == NULL) {
			FreeInternal(block);
		}
	}

	for (block = firstBlock; block != NULL; block = firstBlock) {
		firstBlock = block->next;
		assert(block->IsBaseBlock());
		Mem_Free16(block);
	}

	freeTree.Shutdown();

	Clear();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::SetFixedBlocks(int numBlocks) {
	cweeDynamicBlock<type>* block;

	for (int i = numBaseBlocks; i < numBlocks; i++) {
		block = (cweeDynamicBlock<type>*) Mem_Alloc16((size_t)baseBlockSize, _tag_);
		block->SetSize(baseBlockSize - (int)sizeof(cweeDynamicBlock<type>), true);
		block->next = NULL;
		block->prev = lastBlock;
		if (lastBlock) {
			lastBlock->next = block;
		}
		else {
			firstBlock = block;
		}
		lastBlock = block;
		block->node = NULL;

		FreeInternal(block);

		numBaseBlocks++;
		baseBlockMemory += baseBlockSize;
	}

	allowAllocs = false;
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::SetLockMemory(bool lock) {
	lockMemory = lock;
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::FreeEmptyBaseBlocks() {
	cweeDynamicBlock<type>* block, * next;

	for (block = firstBlock; block != NULL; block = next) {
		next = block->next;

		if (block->IsBaseBlock() && block->node != NULL && (next == NULL || next->IsBaseBlock())) {
			UnlinkFreeInternal(block);
			if (block->prev) {
				block->prev->next = block->next;
			}
			else {
				firstBlock = block->next;
			}
			if (block->next) {
				block->next->prev = block->prev;
			}
			else {
				lastBlock = block->prev;
			}
			numBaseBlocks--;
			baseBlockMemory -= block->GetSize() + (int)sizeof(cweeDynamicBlock<type>);
			Mem_Free16(block);
		}
	}

}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
int cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::GetNumEmptyBaseBlocks() const {
	int numEmptyBaseBlocks;
	cweeDynamicBlock<type>* block;

	numEmptyBaseBlocks = 0;
	for (block = firstBlock; block != NULL; block = block->next) {
		if (block->IsBaseBlock() && block->node != NULL && (block->next == NULL || block->next->IsBaseBlock())) {
			numEmptyBaseBlocks++;
		}
	}
	return numEmptyBaseBlocks;
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
type* cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::Alloc(const int num) {
	cweeDynamicBlock<type>* block;

	numAllocs++;

	if (num <= 0) {
		return NULL;
	}

	block = AllocInternal(num);
	if (block == NULL) {
		return NULL;
	}
	block = ResizeInternal(block, num);
	if (block == NULL) {
		return NULL;
	}

	numUsedBlocks++;
	usedBlockMemory += block->GetSize();

	return block->GetMemory();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
type* cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::Resize(type* ptr, const int num) {
	numResizes++;
	if (ptr == NULL) {
		return Alloc(num);
	}
	if (num <= 0) {
		Free(ptr);
		return NULL;
	}
	cweeDynamicBlock<type>* block = (cweeDynamicBlock<type>*) (((::byte*)ptr) - (int)sizeof(cweeDynamicBlock<type>));
	usedBlockMemory -= block->GetSize();
	block = ResizeInternal(block, num);
	if (block == NULL) {
		return NULL;
	}
	usedBlockMemory += block->GetSize();
	return block->GetMemory();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::Free(type* ptr) {

	numFrees++;
	if (ptr == NULL) {
		return;
	}
	cweeDynamicBlock<type>* block = (cweeDynamicBlock<type>*) (((::byte*)ptr) - (int)sizeof(cweeDynamicBlock<type>));
	numUsedBlocks--;
	usedBlockMemory -= block->GetSize();
	FreeInternal(block);
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
const char* cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::CheckMemory(const type* ptr) const {
	cweeDynamicBlock<type>* block;

	if (ptr == NULL) {
		return NULL;
	}

	block = (cweeDynamicBlock<type>*) (((byte*)ptr) - (int)sizeof(cweeDynamicBlock<type>));

	if (block->node != NULL) {
		return "memory has been freed";
	}
	return NULL;
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::Clear() {
	firstBlock = NULL;
	lastBlock = NULL;
	allowAllocs = true;
	lockMemory = false;
	numBaseBlocks = 0;
	baseBlockMemory = 0;
	numUsedBlocks = 0;
	usedBlockMemory = 0;
	numFreeBlocks = 0;
	freeBlockMemory = 0;
	numAllocs = 0;
	numResizes = 0;
	numFrees = 0;
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
cweeDynamicBlock<type>* cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::AllocInternal(const int num) {
	cweeDynamicBlock<type>* block;
	int alignedBytes = (num * sizeof(type) + 15) & ~15;

	block = freeTree.FindSmallestLargerEqual(alignedBytes);
	if (block && block != NULL && block != nullptr) {
		UnlinkFreeInternal(block);
	}
	else if (allowAllocs) {
		int allocSize = Max(baseBlockSize, alignedBytes + (int)sizeof(cweeDynamicBlock<type>));
		block = (cweeDynamicBlock<type>*) Mem_Alloc16((size_t)allocSize, _tag_);
		block->SetSize(allocSize - (int)sizeof(cweeDynamicBlock<type>), true);
		block->next = NULL;
		block->prev = lastBlock;
		if (lastBlock) {
			lastBlock->next = block;
		}
		else {
			firstBlock = block;
		}
		lastBlock = block;
		block->node = NULL;

		numBaseBlocks++;
		baseBlockMemory += allocSize;
	}

	return block;
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
cweeDynamicBlock<type>* cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::ResizeInternal(cweeDynamicBlock<type>* block, const int num) {
	int alignedBytes = (num * sizeof(type) + 15) & ~15;
	// if the new size is larger
	if (alignedBytes > block->GetSize()) {

		cweeDynamicBlock<type>* nextBlock = block->next;

		// try to annexate the next block if it's free
		if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL &&
			block->GetSize() + (int)sizeof(cweeDynamicBlock<type>) + nextBlock->GetSize() >= alignedBytes) {

			UnlinkFreeInternal(nextBlock);
			block->SetSize(block->GetSize() + (int)sizeof(cweeDynamicBlock<type>) + nextBlock->GetSize(), block->IsBaseBlock());
			block->next = nextBlock->next;
			if (nextBlock->next) {
				nextBlock->next->prev = block;
			}
			else {
				lastBlock = block;
			}
		}
		else {
			// allocate a new block and copy
			cweeDynamicBlock<type>* oldBlock = block;
			block = AllocInternal(num);
			if (block == NULL) {
				return NULL;
			}
			memcpy(block->GetMemory(), oldBlock->GetMemory(), oldBlock->GetSize());
			FreeInternal(oldBlock);
		}
	}

	// if the unused space at the end of this block is large enough to hold a block with at least one element
	if (block->GetSize() - alignedBytes - (int)sizeof(cweeDynamicBlock<type>) < Max(minBlockSize, (int)sizeof(type))) {
		return block;
	}

	cweeDynamicBlock<type>* newBlock;

	newBlock = (cweeDynamicBlock<type>*) (((::byte*)block) + (int)sizeof(cweeDynamicBlock<type>) + alignedBytes);
	try {
		newBlock->SetSize(block->GetSize() - alignedBytes - (int)sizeof(cweeDynamicBlock<type>), false);
	}
	catch (...) {}
	newBlock->next = block->next;
	newBlock->prev = block;
	if (newBlock->next != NULL) {
		newBlock->next->prev = newBlock;
	}
	else {
		lastBlock = newBlock;
	}
	newBlock->node = NULL;
	block->next = newBlock;
	block->SetSize(alignedBytes, block->IsBaseBlock());

	FreeInternal(newBlock);

	return block;
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::FreeInternal(cweeDynamicBlock<type>* block) {

	assert(block->node == NULL);
	// try to merge with a next free block
	cweeDynamicBlock<type>* nextBlock = block->next;
	if (nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL) {
		UnlinkFreeInternal(nextBlock);
		block->SetSize(block->GetSize() + (int)sizeof(cweeDynamicBlock<type>) + nextBlock->GetSize(), block->IsBaseBlock());
		block->next = nextBlock->next;
		if (nextBlock->next) {
			nextBlock->next->prev = block;
		}
		else {
			lastBlock = block;
		}
	}

	// try to merge with a previous free block
	cweeDynamicBlock<type>* prevBlock = block->prev;
	//if (prevBlock && !block->IsBaseBlock() && prevBlock->node != NULL) {
	if (prevBlock && !prevBlock->IsBaseBlock() && prevBlock->node != NULL) {
		UnlinkFreeInternal(prevBlock);
		prevBlock->SetSize(prevBlock->GetSize() + (int)sizeof(cweeDynamicBlock<type>) + block->GetSize(), prevBlock->IsBaseBlock());
		prevBlock->next = block->next;
		if (block->next) {
			block->next->prev = prevBlock;
		}
		else {
			lastBlock = prevBlock;
		}
		LinkFreeInternal(prevBlock);
	}
	else {
		LinkFreeInternal(block);
	}
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
INLINE void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::LinkFreeInternal(cweeDynamicBlock<type>* block) {
	block->node = freeTree.Add(block, block->GetSize());
	numFreeBlocks++;
	freeBlockMemory += block->GetSize();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
INLINE void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::UnlinkFreeInternal(cweeDynamicBlock<type>* block) {
	freeTree.Remove(block->node);
	block->node = NULL;
	numFreeBlocks--;
	freeBlockMemory -= block->GetSize();
}

template<class type, int baseBlockSize, int minBlockSize, memTag_t _tag_>
void cweeDynamicBlockAlloc<type, baseBlockSize, minBlockSize, _tag_>::CheckMemory() const {
	cweeDynamicBlock<type>* block;

	for (block = firstBlock; block != NULL; block = block->next) {
		// make sure the block is properly linked
		if (block->prev == NULL) {
			assert(firstBlock == block);
		}
		else {
			assert(block->prev->next == block);
		}
		if (block->next == NULL) {
			assert(lastBlock == block);
		}
		else {
			assert(block->next->prev == block);
		}
	}
}
