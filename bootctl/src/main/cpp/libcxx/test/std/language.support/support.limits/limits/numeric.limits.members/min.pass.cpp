//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// min()

#include <limits>
#include <climits>
#include <cwchar>
#include <cfloat>
#include <cassert>

#include "test_macros.h"

template <class T>
void
test(T expected)
{
    assert(std::numeric_limits<T>::min() == expected);
    assert(std::numeric_limits<T>::is_bounded || !std::numeric_limits<T>::is_signed);
    assert(std::numeric_limits<const T>::min() == expected);
    assert(std::numeric_limits<const T>::is_bounded || !std::numeric_limits<const T>::is_signed);
    assert(std::numeric_limits<volatile T>::min() == expected);
    assert(std::numeric_limits<volatile T>::is_bounded || !std::numeric_limits<volatile T>::is_signed);
    assert(std::numeric_limits<const volatile T>::min() == expected);
    assert(std::numeric_limits<const volatile T>::is_bounded || !std::numeric_limits<const volatile T>::is_signed);
}

int main()
{
    test<bool>(false);
    test<char>(CHAR_MIN);
    test<signed char>(SCHAR_MIN);
    test<unsigned char>(0);
    test<wchar_t>(WCHAR_MIN);
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t>(0);
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t>(0);
    test<char32_t>(0);
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short>(SHRT_MIN);
    test<unsigned short>(0);
    test<int>(INT_MIN);
    test<unsigned int>(0);
    test<long>(LONG_MIN);
    test<unsigned long>(0);
    test<long long>(LLONG_MIN);
    test<unsigned long long>(0);
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t>(-__int128_t(__uint128_t(-1)/2) - 1);
    test<__uint128_t>(0);
#endif
    test<float>(FLT_MIN);
    test<double>(DBL_MIN);
    test<long double>(LDBL_MIN);
}
