//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// epsilon()

#include <limits>
#include <cfloat>
#include <cassert>

#include "test_macros.h"

template <class T>
void
test(T expected)
{
    assert(std::numeric_limits<T>::epsilon() == expected);
    assert(std::numeric_limits<const T>::epsilon() == expected);
    assert(std::numeric_limits<volatile T>::epsilon() == expected);
    assert(std::numeric_limits<const volatile T>::epsilon() == expected);
}

int main()
{
    test<bool>(false);
    test<char>(0);
    test<signed char>(0);
    test<unsigned char>(0);
    test<wchar_t>(0);
#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    test<char8_t>(0);
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t>(0);
    test<char32_t>(0);
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<short>(0);
    test<unsigned short>(0);
    test<int>(0);
    test<unsigned int>(0);
    test<long>(0);
    test<unsigned long>(0);
    test<long long>(0);
    test<unsigned long long>(0);
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t>(0);
    test<__uint128_t>(0);
#endif
    test<float>(FLT_EPSILON);
    test<double>(DBL_EPSILON);
    test<long double>(LDBL_EPSILON);
}
