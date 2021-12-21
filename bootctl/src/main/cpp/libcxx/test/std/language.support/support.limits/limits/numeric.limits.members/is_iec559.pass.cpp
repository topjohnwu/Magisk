//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// is_iec559

#include <limits>

#include "test_macros.h"

template <class T, bool expected>
void
test()
{
    static_assert(std::numeric_limits<T>::is_iec559 == expected, "is_iec559 test 1");
    static_assert(std::numeric_limits<const T>::is_iec559 == expected, "is_iec559 test 2");
    static_assert(std::numeric_limits<volatile T>::is_iec559 == expected, "is_iec559 test 3");
    static_assert(std::numeric_limits<const volatile T>::is_iec559 == expected, "is_iec559 test 4");
}

int main()
{
    test<bool, false>();
    test<char, false>();
    test<signed char, false>();
    test<unsigned char, false>();
    test<wchar_t, false>();
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, false>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, false>();
    test<char32_t, false>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, false>();
    test<unsigned short, false>();
    test<int, false>();
    test<unsigned int, false>();
    test<long, false>();
    test<unsigned long, false>();
    test<long long, false>();
    test<unsigned long long, false>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, false>();
    test<__uint128_t, false>();
#endif
    test<float, true>();
    test<double, true>();
#if (defined(__ppc__) || defined(__ppc64__))
    test<long double, false>();
#else
    test<long double, true>();
#endif
}
