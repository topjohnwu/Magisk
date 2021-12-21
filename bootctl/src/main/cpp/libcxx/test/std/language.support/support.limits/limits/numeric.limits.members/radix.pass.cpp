//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// radix

#include <limits>
#include <cfloat>

#include "test_macros.h"

template <class T, int expected>
void
test()
{
    static_assert(std::numeric_limits<T>::radix == expected, "radix test 1");
    static_assert(std::numeric_limits<const T>::radix == expected, "radix test 2");
    static_assert(std::numeric_limits<volatile T>::radix == expected, "radix test 3");
    static_assert(std::numeric_limits<const volatile T>::radix == expected, "radix test 4");
}

int main()
{
    test<bool, 2>();
    test<char, 2>();
    test<signed char, 2>();
    test<unsigned char, 2>();
    test<wchar_t, 2>();
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, 2>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, 2>();
    test<char32_t, 2>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, 2>();
    test<unsigned short, 2>();
    test<int, 2>();
    test<unsigned int, 2>();
    test<long, 2>();
    test<unsigned long, 2>();
    test<long long, 2>();
    test<unsigned long long, 2>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, 2>();
    test<__uint128_t, 2>();
#endif
    test<float, FLT_RADIX>();
    test<double, FLT_RADIX>();
    test<long double, FLT_RADIX>();
}
