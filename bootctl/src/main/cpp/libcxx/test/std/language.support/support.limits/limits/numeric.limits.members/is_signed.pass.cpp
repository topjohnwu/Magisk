//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// is_signed

#include <limits>

#include "test_macros.h"

template <class T, bool expected>
void
test()
{
    static_assert(std::numeric_limits<T>::is_signed == expected, "is_signed test 1");
    static_assert(std::numeric_limits<const T>::is_signed == expected, "is_signed test 2");
    static_assert(std::numeric_limits<volatile T>::is_signed == expected, "is_signed test 3");
    static_assert(std::numeric_limits<const volatile T>::is_signed == expected, "is_signed test 4");
}

int main()
{
    test<bool, false>();
    test<char, char(-1) < char(0)>();
    test<signed char, true>();
    test<unsigned char, false>();
    test<wchar_t, wchar_t(-1) < wchar_t(0)>();
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, false>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, false>();
    test<char32_t, false>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, true>();
    test<unsigned short, false>();
    test<int, true>();
    test<unsigned int, false>();
    test<long, true>();
    test<unsigned long, false>();
    test<long long, true>();
    test<unsigned long long, false>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, true>();
    test<__uint128_t, false>();
#endif
    test<float, true>();
    test<double, true>();
    test<long double, true>();
}
