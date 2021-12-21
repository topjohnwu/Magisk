//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// round_style

#include <limits>

#include "test_macros.h"

template <class T, std::float_round_style expected>
void
test()
{
    static_assert(std::numeric_limits<T>::round_style == expected, "round_style test 1");
    static_assert(std::numeric_limits<const T>::round_style == expected, "round_style test 2");
    static_assert(std::numeric_limits<volatile T>::round_style == expected, "round_style test 3");
    static_assert(std::numeric_limits<const volatile T>::round_style == expected, "round_style test 4");
}

int main()
{
    test<bool, std::round_toward_zero>();
    test<char, std::round_toward_zero>();
    test<signed char, std::round_toward_zero>();
    test<unsigned char, std::round_toward_zero>();
    test<wchar_t, std::round_toward_zero>();
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, std::round_toward_zero>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, std::round_toward_zero>();
    test<char32_t, std::round_toward_zero>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, std::round_toward_zero>();
    test<unsigned short, std::round_toward_zero>();
    test<int, std::round_toward_zero>();
    test<unsigned int, std::round_toward_zero>();
    test<long, std::round_toward_zero>();
    test<unsigned long, std::round_toward_zero>();
    test<long long, std::round_toward_zero>();
    test<unsigned long long, std::round_toward_zero>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, std::round_toward_zero>();
    test<__uint128_t, std::round_toward_zero>();
#endif
    test<float, std::round_to_nearest>();
    test<double, std::round_to_nearest>();
    test<long double, std::round_to_nearest>();
}
