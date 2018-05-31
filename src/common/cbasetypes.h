/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2016  Hercules Dev Team
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef COMMON_CBASETYPES_H
#define COMMON_CBASETYPES_H

/*              +--------+-----------+--------+---------+
 *              | ILP32  |   LP64    |  ILP64 | (LL)P64 |
 * +------------+--------+-----------+--------+---------+
 * | ints       | 32-bit |   32-bit  | 64-bit |  32-bit |
 * | longs      | 32-bit |   64-bit  | 64-bit |  32-bit |
 * | long-longs | 64-bit |   64-bit  | 64-bit |  64-bit |
 * | pointers   | 32-bit |   64-bit  | 64-bit |  64-bit |
 * +------------+--------+-----------+--------+---------+
 * |    where   |   --   |   Tiger   |  Alpha | Windows |
 * |    used    |        |   Unix    |  Cray  |         |
 * |            |        | Sun & SGI |        |         |
 * +------------+--------+-----------+--------+---------+
 * Taken from http://developer.apple.com/macosx/64bit.html
 */

//////////////////////////////////////////////////////////////////////////
// basic include for all basics
// introduces types and global functions
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// setting some defines on platforms
//////////////////////////////////////////////////////////////////////////
#if (defined(__WIN32__) || defined(__WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER) || defined(__BORLANDC__)) && !defined(WIN32)
#define WIN32
#endif

#if defined(__MINGW32__) && !defined(MINGW)
#define MINGW
#endif

#if (defined(__CYGWIN__) || defined(__CYGWIN32__)) && !defined(CYGWIN)
#define CYGWIN
#endif

// __APPLE__ is the only predefined macro on MacOS
#if defined(__APPLE__)
#define __DARWIN__
#endif

// Standardize the ARM platform version, if available (the only values we're interested in right now are >= ARMv6)
#if defined(__ARMV6__) || defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) \
	|| defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) // gcc ARMv6
#define __ARM_ARCH_VERSION__ 6
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7S__) // gcc ARMv7
#define __ARM_ARCH_VERSION__ 7
#elif defined(_M_ARM) // MSVC
#define __ARM_ARCH_VERSION__ _M_ARM
#else
#define __ARM_ARCH_VERSION__ 0
#endif

// Necessary for __NetBSD_Version__ (defined as VVRR00PP00) on NetBSD
#ifdef __NETBSD__
#include <sys/param.h>
#endif // __NETBSD__

// 64bit OS
#if defined(_M_IA64) || defined(_M_X64) || defined(_WIN64) || defined(_LP64) || defined(_ILP64) || defined(__LP64__) || defined(__ppc64__)
#define __64BIT__
#endif

#if defined(_ILP64)
#error "this specific 64bit architecture is not supported"
#endif

// debug mode
#if defined(_DEBUG) && !defined(DEBUG)
#define DEBUG
#endif

// debug function name
#ifndef __NETBSD__
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#	if __GNUC__ >= 2
#		define __func__ __FUNCTION__
#	else
#		define __func__ ""
#	endif
#endif
#endif


// disable attributed stuff on non-GNU
#if !defined(__GNUC__) && !defined(MINGW)
#  define  __attribute__(x)
#endif

/// Feature/extension checking macros
#ifndef __has_extension /* Available in clang and gcc >= 3 */
#define __has_extension(x) 0
#endif
#ifndef __has_feature /* Available in clang and gcc >= 5 */
#define __has_feature(x) __has_extension(x)
#endif

//////////////////////////////////////////////////////////////////////////
// portable printf/scanf format macros and integer definitions
// NOTE: Visual C++ uses <inttypes.h> and <stdint.h> provided in /3rdparty
//////////////////////////////////////////////////////////////////////////
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>

// ILP64 isn't supported, so always 32 bits?
#ifndef UINT_MAX
#define UINT_MAX 0xffffffff
#endif

//////////////////////////////////////////////////////////////////////////
// Integers with guaranteed _exact_ size.
//////////////////////////////////////////////////////////////////////////

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef int8_t   sint8;
typedef int16_t  sint16;
typedef int32_t  sint32;
typedef int64_t  sint64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#undef UINT8_MIN
#undef UINT16_MIN
#undef UINT32_MIN
#undef UINT64_MIN
#define UINT8_MIN  ((uint8) UINT8_C(0x00))
#define UINT16_MIN ((uint16)UINT16_C(0x0000))
#define UINT32_MIN ((uint32)UINT32_C(0x00000000))
#define UINT64_MIN ((uint64)UINT64_C(0x0000000000000000))

#undef UINT8_MAX
#undef UINT16_MAX
#undef UINT32_MAX
#undef UINT64_MAX
#define UINT8_MAX  ((uint8) UINT8_C(0xFF))
#define UINT16_MAX ((uint16)UINT16_C(0xFFFF))
#define UINT32_MAX ((uint32)UINT32_C(0xFFFFFFFF))
#define UINT64_MAX ((uint64)UINT64_C(0xFFFFFFFFFFFFFFFF))

#undef SINT8_MIN
#undef SINT16_MIN
#undef SINT32_MIN
#undef SINT64_MIN
#define SINT8_MIN  ((sint8) INT8_C(0x80))
#define SINT16_MIN ((sint16)INT16_C(0x8000))
#define SINT32_MIN ((sint32)INT32_C(0x80000000))
#define SINT64_MIN ((sint32)INT64_C(0x8000000000000000))

#undef SINT8_MAX
#undef SINT16_MAX
#undef SINT32_MAX
#undef SINT64_MAX
#define SINT8_MAX  ((sint8) INT8_C(0x7F))
#define SINT16_MAX ((sint16)INT16_C(0x7FFF))
#define SINT32_MAX ((sint32)INT32_C(0x7FFFFFFF))
#define SINT64_MAX ((sint64)INT64_C(0x7FFFFFFFFFFFFFFF))

//////////////////////////////////////////////////////////////////////////
// Integers with guaranteed _minimum_ size.
// These could be larger than you expect,
// they are designed for speed.
//////////////////////////////////////////////////////////////////////////
typedef          long int   ppint;
typedef          long int   ppint8;
typedef          long int   ppint16;
typedef          long int   ppint32;

typedef unsigned long int   ppuint;
typedef unsigned long int   ppuint8;
typedef unsigned long int   ppuint16;
typedef unsigned long int   ppuint32;


//////////////////////////////////////////////////////////////////////////
// integer with exact processor width (and best speed)
//////////////////////////////
#include <stddef.h> // size_t

#if defined(WIN32) && !defined(MINGW) // does not have a signed size_t
//////////////////////////////
#if defined(_WIN64) // native 64bit windows platform
typedef __int64 ssize_t;
#else
typedef int     ssize_t;
#endif
//////////////////////////////
#endif
//////////////////////////////


//////////////////////////////////////////////////////////////////////////
// pointer sized integers
//////////////////////////////////////////////////////////////////////////
typedef intptr_t intptr;
typedef uintptr_t uintptr;


//////////////////////////////////////////////////////////////////////////
// Add a 'sysint' Type which has the width of the platform we're compiled for.
//////////////////////////////////////////////////////////////////////////
#if defined(__GNUC__)
	#if defined(__x86_64__)
		typedef int64 sysint;
		typedef uint64 usysint;
	#else
		typedef int32 sysint;
		typedef uint32 usysint;
	#endif
#elif defined(_MSC_VER)
	#if defined(_M_X64)
		typedef int64 sysint;
		typedef uint64 usysint;
	#else
		typedef int32 sysint;
		typedef uint32 usysint;
	#endif
#else
	#error Compiler / Platform is unsupported.
#endif


//////////////////////////////////////////////////////////////////////////
// some redefine of function redefines for some Compilers
//////////////////////////////////////////////////////////////////////////
#if defined(_MSC_VER) || defined(__BORLANDC__)
#define strcasecmp  stricmp
#define strncasecmp strnicmp
#define strncmpi    strnicmp
#if defined(__BORLANDC__) || _MSC_VER < 1900
#define snprintf    _snprintf
#endif
#else
#define strcmpi     strcasecmp
#define stricmp     strcasecmp
#define strncmpi    strncasecmp
#define strnicmp    strncasecmp
#endif
#if defined(_MSC_VER)
#define strtoull    _strtoui64
#define strtoll     _strtoi64
#endif

// keyword replacement
#ifdef _MSC_VER
// For MSVC (windows)
#define inline __inline
#define forceinline __forceinline
#define ra_align(n) __declspec(align(n))
#else
// For GCC
#define forceinline __attribute__((always_inline)) inline
#define ra_align(n) __attribute__(( aligned(n) ))
#endif

// Directives for the (clang) static analyzer
#ifdef __clang__
#define analyzer_noreturn __attribute__((analyzer_noreturn))
#else
#define analyzer_noreturn
#endif

// gcc version (if any) - borrowed from Mana Plus
#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 \
		+ __GNUC_MINOR__ * 100 \
		+ __GNUC_PATCHLEVEL__)
#else
#define GCC_VERSION 0
#endif

// Pragma macro only enabled on gcc >= 4.6 or clang - borrowed from Mana Plus
#if defined(__GNUC__) && (defined(__clang__) || GCC_VERSION >= 40600)
#define PRAGMA_GCC46(str) _Pragma(#str)
#else // ! defined(__GNUC__) && (defined(__clang__) || GCC_VERSION >= 40600)
#define PRAGMA_GCC46(str)
#endif // ! defined(__GNUC__) && (defined(__clang__) || GCC_VERSION >= 40600)

// Pragma macro only enabled on gcc >= 5 or clang - borrowed from Mana Plus
#if defined(__GNUC__) && (GCC_VERSION >= 50000)
#define PRAGMA_GCC5(str) _Pragma(#str)
#else // ! defined(__GNUC__) && (GCC_VERSION >= 50000)
#define PRAGMA_GCC5(str)
#endif // ! defined(__GNUC__) && (GCC_VERSION >= 50000)

// fallthrough attribute only enabled on gcc >= 7.0
#if defined(__GNUC__) && (GCC_VERSION >= 70000)
#define FALLTHROUGH __attribute__ ((fallthrough));
#else // ! defined(__GNUC__) && (GCC_VERSION >= 70000)
#define FALLTHROUGH
#endif // ! defined(__GNUC__) && (GCC_VERSION >= 70000)

// boolean types for C
#if !defined(_MSC_VER) || _MSC_VER >= 1800
// MSVC doesn't have stdbool.h yet as of Visual Studio 2012 (MSVC version 17.00)
// but it will support it in Visual Studio 2013 (MSVC version 18.00)
// http://blogs.msdn.com/b/vcblog/archive/2013/07/19/c99-library-support-in-visual-studio-2013.aspx
// GCC and Clang are assumed to be C99 compliant
#include <stdbool.h> // bool, true, false, __bool_true_false_are_defined
#endif // ! defined(_MSC_VER) || _MSC_VER >= 1800

#ifndef __bool_true_false_are_defined
// If stdbool.h is not available or does not define this
typedef char bool;
#define false (1==0)
#define true  (1==1)
#define __bool_true_false_are_defined
#endif // __bool_true_false_are_defined

//////////////////////////////////////////////////////////////////////////
// macro tools

#ifdef swap // just to be sure
#undef swap
#endif
// hmm only ints?
//#define swap(a,b) { int temp=a; a=b; b=temp;}
// if using macros then something that is type independent
//#define swap(a,b) ((a == b) || ((a ^= b), (b ^= a), (a ^= b)))
// Avoid "value computed is not used" warning and generates the same assembly code
//#define swap(a,b) if (a != b) ((a ^= b), (b ^= a), (a ^= b))
// but is vulnerable to 'if (foo) swap(bar, baz); else quux();', causing the else to nest incorrectly.
#define swap(a,b) do { if ((a) != (b)) { (a) ^= (b); (b) ^= (a); (a) ^= (b); } } while(0)
#define swap_ptr(a,b) do { if ((a) != (b)) (a) = (void*)((intptr_t)(a) ^ (intptr_t)(b)); (b) = (void*)((intptr_t)(a) ^ (intptr_t)(b)); (a) = (void*)((intptr_t)(a) ^ (intptr_t)(b)); } while(0)

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//////////////////////////////////////////////////////////////////////////
// should not happen
#ifndef NULL
#define NULL (void *)0
#endif

//////////////////////////////////////////////////////////////////////////
// Additional printf specifiers
#if defined(_MSC_VER)
#define PRIS_PREFIX "I"
#else // gcc
#define PRIS_PREFIX "z"
#endif
#define PRIdS PRIS_PREFIX "d"
#define PRIxS PRIS_PREFIX "x"
#define PRIuS PRIS_PREFIX "u"
#define PRIXS PRIS_PREFIX "X"
#define PRIoS PRIS_PREFIX "o"

//////////////////////////////////////////////////////////////////////////
// path separator

#if defined(WIN32)
#define PATHSEP '\\'
#define PATHSEP_STR "\\"
#elif defined(__APPLE__) && !defined(__MACH__)
// __MACH__ indicates OS X ( http://sourceforge.net/p/predef/wiki/OperatingSystems/ )
#define PATHSEP ':'
#define PATHSEP_STR ":"
#else
#define PATHSEP '/'
#define PATHSEP_STR "/"
#endif

//////////////////////////////////////////////////////////////////////////
// Has to be unsigned to avoid problems in some systems
// Problems arise when these functions expect an argument in the range [0,256[ and are fed a signed char.
#include <ctype.h>
#define ISALNUM(c) (isalnum((unsigned char)(c)))
#define ISALPHA(c) (isalpha((unsigned char)(c)))
#define ISCNTRL(c) (iscntrl((unsigned char)(c)))
#define ISDIGIT(c) (isdigit((unsigned char)(c)))
#define ISGRAPH(c) (isgraph((unsigned char)(c)))
#define ISLOWER(c) (islower((unsigned char)(c)))
#define ISPRINT(c) (isprint((unsigned char)(c)))
#define ISPUNCT(c) (ispunct((unsigned char)(c)))
#define ISSPACE(c) (isspace((unsigned char)(c)))
#define ISUPPER(c) (isupper((unsigned char)(c)))
#define ISXDIGIT(c) (isxdigit((unsigned char)(c)))
#define TOASCII(c) (toascii((unsigned char)(c)))
#define TOLOWER(c) (tolower((unsigned char)(c)))
#define TOUPPER(c) (toupper((unsigned char)(c)))

//////////////////////////////////////////////////////////////////////////
// length of a static array
#define ARRAYLENGTH(A) ( (int)(sizeof(A)/sizeof((A)[0])) )

//////////////////////////////////////////////////////////////////////////
// Make sure va_copy exists
#include <stdarg.h> // va_list, va_copy(?)
#if !defined(va_copy)
#if defined(__va_copy)
#define va_copy __va_copy
#else
#define va_copy(dst, src) ((void) memcpy(&(dst), &(src), sizeof(va_list)))
#endif
#endif


//////////////////////////////////////////////////////////////////////////
// Use the preprocessor to 'stringify' stuff (convert to a string).
// example:
//   #define TESTE blabla
//   QUOTE(TESTE) -> "TESTE"
//   EXPAND_AND_QUOTE(TESTE) -> "blabla"
#define QUOTE(x) #x
#define EXPAND_AND_QUOTE(x) QUOTE(x)


/* pointer size fix which fixes several gcc warnings */
#ifdef __64BIT__
	#define h64BPTRSIZE(y) ((intptr)(y))
#else
	#define h64BPTRSIZE(y) (y)
#endif

/** Support macros for marking blocks to memset to 0 */
#define BEGIN_ZEROED_BLOCK int8 HERC__zeroed_block_BEGIN
#define END_ZEROED_BLOCK int8 HERC__zeroed_block_END
#define ZEROED_BLOCK_POS(x) (&(x)->HERC__zeroed_block_BEGIN)
#define ZEROED_BLOCK_SIZE(x) ((char*)&((x)->HERC__zeroed_block_END) - (char*)&((x)->HERC__zeroed_block_BEGIN) + sizeof((x)->HERC__zeroed_block_END))

/** Support macros for marking structs as unavailable */
#define UNAVAILABLE_STRUCT int8 HERC__unavailable_struct

/** Static assertion (only on compilers that support it) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
// C11 version
#define STATIC_ASSERT(ex, msg) _Static_assert(ex, msg)
#elif __has_feature(c_static_assert)
// Clang support (as per http://clang.llvm.org/docs/LanguageExtensions.html)
#define STATIC_ASSERT(ex, msg) _Static_assert(ex, msg)
#elif defined(__GNUC__) && GCC_VERSION >= 40700
// GCC >= 4.7 is known to support it
#define STATIC_ASSERT(ex, msg) _Static_assert(ex, msg)
#elif defined(_MSC_VER)
// MSVC doesn't support it, but it accepts the C++ style version
#define STATIC_ASSERT(ex, msg) static_assert(ex, msg)
#else
// Otherise just ignore it until it's supported
#define STATIC_ASSERT(ex, msg)
#endif


#endif /* COMMON_CBASETYPES_H */
