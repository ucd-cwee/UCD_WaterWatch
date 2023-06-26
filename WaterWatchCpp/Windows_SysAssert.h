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

class cweeAssert {
public:
	static bool AssertFailed(const char* file, int line, const char* expression) {
		static volatile bool skipAllAssertions = false; // Set this to true to skip ALL assertions, including ones YOU CAUSE!
		if (skipAllAssertions) {
			return false;
		}
		return true;
	};
};

#undef assert
// cwee_assert is useful for cases where some external library (think MFC, etc.) decides it's a good idea to redefine assert on us
#define cwee_assert( x )	(void)( ( !!( x ) ) || ( cweeAssert::AssertFailed( __FILE__, __LINE__, #x ) ) )
#define assert( x )		__analysis_assume( x ) ; cwee_assert( x )
#define verify( x )		( ( x ) ? true : ( cweeAssert::AssertFailed( __FILE__, __LINE__, #x ), false ) )
#define releaseassert( x )	(void)( ( !!( x ) ) || ( cweeAssert::AssertFailed( __FILE__, __LINE__, #x ) ) );
#define release_assert( x )	releaseassert( x )
#define assert_2_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) &  1 ) == 0 )
#define assert_4_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) &  3 ) == 0 )
#define assert_8_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) &  7 ) == 0 )
#define assert_16_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 15 ) == 0 )
#define assert_32_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 31 ) == 0 )
#define assert_64_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 63 ) == 0 )
#define assert_128_byte_aligned( ptr )		assert( ( ((UINT_PTR)(ptr)) & 127 ) == 0 )
#define assert_aligned_to_type_size( ptr )	assert( ( ((UINT_PTR)(ptr)) & ( sizeof( (ptr)[0] ) - 1 ) ) == 0 )

template<bool> struct compile_time_assert_failed;
template<> struct compile_time_assert_failed<true> {};
template<int x> struct compile_time_assert_test {};
#define compile_time_assert_join2( a, b )	a##b
#define compile_time_assert_join( a, b )	compile_time_assert_join2(a,b)
#define compile_time_assert( x )			typedef compile_time_assert_test<sizeof(compile_time_assert_failed<(bool)(x)>)> compile_time_assert_join(compile_time_assert_typedef_, __LINE__)

#define assert_sizeof( type, size )						compile_time_assert( sizeof( type ) == size )
#define assert_sizeof_8_byte_multiple( type )			compile_time_assert( ( sizeof( type ) &  7 ) == 0 )
#define assert_sizeof_16_byte_multiple( type )			compile_time_assert( ( sizeof( type ) & 15 ) == 0 )
#define assert_offsetof( type, field, offset )			compile_time_assert( offsetof( type, field ) == offset )
#define assert_offsetof_8_byte_multiple( type, field )	compile_time_assert( ( offsetof( type, field ) & 7 ) == 0 )
#define assert_offsetof_16_byte_multiple( type, field )	compile_time_assert( ( offsetof( type, field ) & 15 ) == 0 )