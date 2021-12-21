//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// digits10

#include <limits>
#include <cfloat>

#include "test_macros.h"

template <class T, int expected>
void
test()
{
    static_assert(std::numeric_limits<T>::digits10 == expected, "digits10 test 1");
    static_assert(std::numeric_limits<T>::is_bounded, "digits10 test 5");
    static_assert(std::numeric_limits<const T>::digits10 == expected, "digits10 test 2");
    static_assert(std::numeric_limits<const T>::is_bounded, "digits10 test 6");
    static_assert(std::numeric_limits<volatile T>::digits10 == expected, "digits10 test 3");
    static_assert(std::numeric_limits<volatile T>::is_bounded, "digits10 test 7");
    static_assert(std::numeric_limits<const volatile T>::digits10 == expected, "digits10 test 4");
    static_assert(std::numeric_limits<const volatile T>::is_bounded, "digits10 test 8");
}

int main()
{
    test<bool, 0>();
    test<char, 2>();
    test<signed char, 2>();
    test<unsigned char, 2>();
    test<wchar_t, 5*sizeof(wchar_t)/2-1>();  // 4 -> 9 and 2 -> 4
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, 2>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, 4>();
    test<char32_t, 9>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, 4>();
    test<unsigned short, 4>();
    test<int, 9>();
    test<unsigned int, 9>();
    test<long, sizeof(long) == 4 ? 9 : 18>();
    test<unsigned long, sizeof(long) == 4 ? 9 : 19>();
    test<long long, 18>();
    test<unsigned long long, 19>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, 38>();
    test<__uint128_t, 38>();
#endif
    test<float, FLT_DIG>();
    test<double, DBL_DIG>();
    test<long double, LDBL_DIG>();
}
