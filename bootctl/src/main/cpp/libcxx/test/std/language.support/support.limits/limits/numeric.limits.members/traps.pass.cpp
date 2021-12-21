//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// traps

#include <limits>

#include "test_macros.h"

#if defined(__i386__) || defined(__x86_64__) || defined(__pnacl__) || \
    defined(__wasm__)
static const bool integral_types_trap = true;
#else
static const bool integral_types_trap = false;
#endif

template <class T, bool expected>
void
test()
{
    static_assert(std::numeric_limits<T>::traps == expected, "traps test 1");
    static_assert(std::numeric_limits<const T>::traps == expected, "traps test 2");
    static_assert(std::numeric_limits<volatile T>::traps == expected, "traps test 3");
    static_assert(std::numeric_limits<const volatile T>::traps == expected, "traps test 4");
}

int main()
{
    test<bool, false>();
    test<char, integral_types_trap>();
    test<signed char, integral_types_trap>();
    test<unsigned char, integral_types_trap>();
    test<wchar_t, integral_types_trap>();
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t, integral_types_trap>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t, integral_types_trap>();
    test<char32_t, integral_types_trap>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short, integral_types_trap>();
    test<unsigned short, integral_types_trap>();
    test<int, integral_types_trap>();
    test<unsigned int, integral_types_trap>();
    test<long, integral_types_trap>();
    test<unsigned long, integral_types_trap>();
    test<long long, integral_types_trap>();
    test<unsigned long long, integral_types_trap>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t, integral_types_trap>();
    test<__uint128_t, integral_types_trap>();
#endif
    test<float, false>();
    test<double, false>();
    test<long double, false>();
}
