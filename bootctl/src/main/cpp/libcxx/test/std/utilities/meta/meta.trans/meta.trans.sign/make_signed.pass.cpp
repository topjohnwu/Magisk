//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// make_signed

#include <type_traits>

#include "test_macros.h"

enum Enum {zero, one_};

#if TEST_STD_VER >= 11
enum BigEnum : unsigned long long // MSVC's ABI doesn't follow the Standard
#else
enum BigEnum
#endif
{
    bigzero,
    big = 0xFFFFFFFFFFFFFFFFULL
};

#if !defined(_LIBCPP_HAS_NO_INT128) && !defined(_LIBCPP_HAS_NO_STRONG_ENUMS)
enum HugeEnum : __uint128_t
{
    hugezero
};
#endif

template <class T, class U>
void test_make_signed()
{
    static_assert((std::is_same<typename std::make_signed<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::make_signed_t<T>, U>::value), "");
#endif
}

int main()
{
    test_make_signed< signed char, signed char >();
    test_make_signed< unsigned char, signed char >();
    test_make_signed< char, signed char >();
    test_make_signed< short, signed short >();
    test_make_signed< unsigned short, signed short >();
    test_make_signed< int, signed int >();
    test_make_signed< unsigned int, signed int >();
    test_make_signed< long, signed long >();
    test_make_signed< unsigned long, long >();
    test_make_signed< long long, signed long long >();
    test_make_signed< unsigned long long, signed long long >();
    test_make_signed< wchar_t, std::conditional<sizeof(wchar_t) == 4, int, short>::type >();
    test_make_signed< const wchar_t, std::conditional<sizeof(wchar_t) == 4, const int, const short>::type >();
    test_make_signed< const Enum, std::conditional<sizeof(Enum) == sizeof(int), const int, const signed char>::type >();
    test_make_signed< BigEnum, std::conditional<sizeof(long) == 4, long long, long>::type >();
#ifndef _LIBCPP_HAS_NO_INT128
    test_make_signed< __int128_t, __int128_t >();
    test_make_signed< __uint128_t, __int128_t >();
# ifndef _LIBCPP_HAS_NO_STRONG_ENUMS
    test_make_signed< HugeEnum, __int128_t >();
# endif
#endif
}
