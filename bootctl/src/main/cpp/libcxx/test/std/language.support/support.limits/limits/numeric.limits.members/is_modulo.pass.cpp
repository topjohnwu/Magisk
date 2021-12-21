//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// is_modulo

#include <limits>

#include "test_macros.h"

template <class T, bool expected>
void
test()
{
    static_assert(std::numeric_limits<T>::is_modulo == expected, "is_modulo test 1");
    static_assert(std::numeric_limits<const T>::is_modulo == expected, "is_modulo test 2");
    static_assert(std::numeric_limits<volatile T>::is_modulo == expected, "is_modulo test 3");
    static_assert(std::numeric_limits<const volatile T>::is_modulo == expected, "is_modulo test 4");
}

int main()
{
    test<bool, false>();
//    test<char, false>(); // don't know
    test<signed char, false>();
    test<unsigned char, true>();
//    test<wchar_t, false>(); // don't know
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, true>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, true>();
    test<char32_t, true>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, false>();
    test<unsigned short, true>();
    test<int, false>();
    test<unsigned int, true>();
    test<long, false>();
    test<unsigned long, true>();
    test<long long, false>();
    test<unsigned long long, true>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, false>();
    test<__uint128_t, true>();
#endif
    test<float, false>();
    test<double, false>();
    test<long double, false>();
}
