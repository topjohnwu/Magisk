//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// digits

#include <limits>
#include <cfloat>

#include "test_macros.h"

template <class T, int expected>
void
test()
{
    static_assert(std::numeric_limits<T>::digits == expected, "digits test 1");
    static_assert(std::numeric_limits<const T>::digits == expected, "digits test 2");
    static_assert(std::numeric_limits<volatile T>::digits == expected, "digits test 3");
    static_assert(std::numeric_limits<const volatile T>::digits == expected, "digits test 4");
}

int main()
{
    test<bool, 1>();
    test<char, std::numeric_limits<char>::is_signed ? 7 : 8>();
    test<signed char, 7>();
    test<unsigned char, 8>();
    test<wchar_t, std::numeric_limits<wchar_t>::is_signed ? sizeof(wchar_t)*8-1 : sizeof(wchar_t)*8>();
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, 8>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, 16>();
    test<char32_t, 32>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, 15>();
    test<unsigned short, 16>();
    test<int, 31>();
    test<unsigned int, 32>();
    test<long, sizeof(long) == 4 ? 31 : 63>();
    test<unsigned long, sizeof(long) == 4 ? 32 : 64>();
    test<long long, 63>();
    test<unsigned long long, 64>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, 127>();
    test<__uint128_t, 128>();
#endif
    test<float, FLT_MANT_DIG>();
    test<double, DBL_MANT_DIG>();
    test<long double, LDBL_MANT_DIG>();
}
