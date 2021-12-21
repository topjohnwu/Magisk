//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// max()

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
    assert(std::numeric_limits<T>::max() == expected);
    assert(std::numeric_limits<T>::is_bounded);
    assert(std::numeric_limits<const T>::max() == expected);
    assert(std::numeric_limits<const T>::is_bounded);
    assert(std::numeric_limits<volatile T>::max() == expected);
    assert(std::numeric_limits<volatile T>::is_bounded);
    assert(std::numeric_limits<const volatile T>::max() == expected);
    assert(std::numeric_limits<const volatile T>::is_bounded);
}

int main()
{
    test<bool>(true);
    test<char>(CHAR_MAX);
    test<signed char>(SCHAR_MAX);
    test<unsigned char>(UCHAR_MAX);
    test<wchar_t>(WCHAR_MAX);
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t>(UCHAR_MAX); // ??
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t>(USHRT_MAX);
    test<char32_t>(UINT_MAX);
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short>(SHRT_MAX);
    test<unsigned short>(USHRT_MAX);
    test<int>(INT_MAX);
    test<unsigned int>(UINT_MAX);
    test<long>(LONG_MAX);
    test<unsigned long>(ULONG_MAX);
    test<long long>(LLONG_MAX);
    test<unsigned long long>(ULLONG_MAX);
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t>(__int128_t(__uint128_t(-1)/2));
    test<__uint128_t>(__uint128_t(-1));
#endif
    test<float>(FLT_MAX);
    test<double>(DBL_MAX);
    test<long double>(LDBL_MAX);
}
