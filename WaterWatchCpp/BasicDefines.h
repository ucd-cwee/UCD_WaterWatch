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

#define NODISCARD						[[nodiscard]]
#define AUTO							decltype(auto)
#define INLINE							__forceinline
#define INLINE_EXTERN					extern inline

typedef unsigned char			byte;		// 8 bits // compare to Microsoft existing Byte. 
typedef unsigned short			word;		// 16 bits
typedef unsigned int			dword;		// 32 bits
typedef unsigned int			uint;
typedef unsigned long			ulong;
typedef int						handle;
typedef signed char				int8;
typedef unsigned char			uint8;
typedef short int				int16;
typedef unsigned short int		uint16;
typedef int						int32;
typedef unsigned int			uint32;
typedef long long				int64;
typedef unsigned long long		uint64;
typedef long double				u64;

#define INT8_SIGN_BIT		7
#define INT16_SIGN_BIT		15
#define INT32_SIGN_BIT		31
#define INT64_SIGN_BIT		63

#define INT8_SIGN_MASK		( 1 << INT8_SIGN_BIT )
#define INT16_SIGN_MASK		( 1 << INT16_SIGN_BIT )
#define INT32_SIGN_MASK		( 1UL << INT32_SIGN_BIT )
#define INT64_SIGN_MASK		( 1ULL << INT64_SIGN_BIT )


#define CONST_MAX( x, y )			( (x) > (y) ? (x) : (y) )

class vec3;

#ifndef NULL
#define NULL					((void *)0)
#endif

#ifndef BIT
#define BIT( num )				( 1 << ( num ) )
#endif

#define	MAX_STRING_CHARS		1024		// max length of a string
#define MAX_PRINT_MSG			16384		// buffer size for our various printf routines

// little/big endian conversion
short	BigShort(short l);
short	LittleShort(short l);
int		BigLong(int l);
int		LittleLong(int l);
float	BigFloat(float l);
float	LittleFloat(float l);
void	BigRevBytes(void* bp, int elsize, int elcount);
void	LittleRevBytes(void* bp, int elsize, int elcount);
void	LittleBitField(void* bp, int elsize);
void	Swap_Init(void);
bool	Swap_IsBigEndian(void);

// for base64
void	SixtetsForInt(::byte* out, int src);
int		IntForSixtets(::byte* in);

// move from Math.h to keep gcc happy
template<class T> INLINE T	Max(T x, T y) { return (x > y) ? x : y; }
template<class T> INLINE T	Min(T x, T y) { return (x < y) ? x : y; }

// CPU usage
#define BYTES_IN_A_MB 1048576

/*
class typenames {
public:
	class detail {
	public:
		using type_name_prober = void;
		template <typename T> static constexpr std::string_view wrapped_type_name() {
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
		};
		static constexpr std::size_t wrapped_type_name_prefix_length() { return wrapped_type_name<type_name_prober>().find(sv_type_name<type_name_prober>()); };
		static constexpr std::size_t wrapped_type_name_suffix_length() { return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - sv_type_name<type_name_prober>().length(); };
	};

	template <typename T> static constexpr std::string_view sv_type_name() {
		constexpr std::string_view wrapped_name = typenames::detail::wrapped_type_name<T>();
		constexpr auto prefix_length = typenames::detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = typenames::detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	};

	template <typename T> static constexpr const char* type_name() {
		constexpr std::string_view wrapped_name = typenames::detail::wrapped_type_name<T>();
		constexpr auto prefix_length = typenames::detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = typenames::detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length).data();
	};

	template <> static constexpr std::string_view sv_type_name<void>() { return "void"; };
	template <> static constexpr const char* type_name<void>() { return "void"; };
};
*/

class typenames {
public:
	template<typename T>
	struct identity { typedef T type; };

	class detail {
	public:
		using type_name_prober = void;
		template <typename T> static constexpr std::string_view wrapped_type_name() {
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
		};
		static constexpr std::size_t wrapped_type_name_prefix_length() { return wrapped_type_name<type_name_prober>().find(sv_type_name<type_name_prober>()); };
		static constexpr std::size_t wrapped_type_name_suffix_length() { return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - sv_type_name<type_name_prober>().length(); };
	};

	template <typename T>
	static constexpr std::string_view sv_type_name() {
		return sv_type_name(identity<T>());
	};

	template <typename T>
	static constexpr const char* type_name() {
		return type_name(identity<T>());
	};

private:
	template <typename T>
	static constexpr std::string_view sv_type_name(identity<T>)
	{
		constexpr std::string_view wrapped_name = typenames::detail::wrapped_type_name<T>();
		constexpr auto prefix_length = typenames::detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = typenames::detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	};

	template <typename T>
	static constexpr const char* type_name(identity<T>)
	{
		constexpr std::string_view wrapped_name = typenames::detail::wrapped_type_name<T>();
		constexpr auto prefix_length = typenames::detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = typenames::detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length).data();
	};

	static constexpr std::string_view sv_type_name(identity<void>) { return "void"; };
	static constexpr const char* type_name(identity<void>) { return "void";	};
};




#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif