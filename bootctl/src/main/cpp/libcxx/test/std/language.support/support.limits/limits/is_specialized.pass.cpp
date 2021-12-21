//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test numeric_limits

// Specializations shall be provided for each arithmetic type, both floating
// point and integer, including bool. The member is_specialized shall be
// true for all such specializations of numeric_limits.

// Non-arithmetic standard types, such as complex<T> (26.3.2), shall not
// have specializations.

// From [numeric.limits]:

// The value of each member of a specialization of numeric_limits on a cv
// -qualified type cv T shall be equal to the value of the corresponding
// member of the specialization on the unqualified type T.

// More convenient to test it here.

#include <limits>
#include <complex>

template <class T>
void test()
{
    static_assert(std::numeric_limits<T>::is_specialized,
                 "std::numeric_limits<T>::is_specialized");
    static_assert(std::numeric_limits<const T>::is_specialized,
                 "std::numeric_limits<const T>::is_specialized");
    static_assert(std::numeric_limits<volatile T>::is_specialized,
                 "std::numeric_limits<volatile T>::is_specialized");
    static_assert(std::numeric_limits<const volatile T>::is_specialized,
                 "std::numeric_limits<const volatile T>::is_specialized");
}

int main()
{
    test<bool>();
    test<char>();
    test<wchar_t>();
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test<char16_t>();
    test<char32_t>();
#endif  // _LIBCPP_HAS_NO_UNICODE_CHARS
    test<signed char>();
    test<unsigned char>();
    test<signed short>();
    test<unsigned short>();
    test<signed int>();
    test<unsigned int>();
    test<signed long>();
    test<unsigned long>();
    test<signed long long>();
    test<unsigned long long>();
#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t>();
    test<__uint128_t>();
#endif
    test<float>();
    test<double>();
    test<long double>();
    static_assert(!std::numeric_limits<std::complex<double> >::is_specialized,
                 "!std::numeric_limits<std::complex<double> >::is_specialized");
}
