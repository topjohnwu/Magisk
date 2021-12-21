//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// integral

#include <type_traits>

template <class T>
void test_integral_imp()
{
    static_assert(!std::is_reference<T>::value, "");
    static_assert( std::is_arithmetic<T>::value, "");
    static_assert( std::is_fundamental<T>::value, "");
    static_assert( std::is_object<T>::value, "");
    static_assert( std::is_scalar<T>::value, "");
    static_assert(!std::is_compound<T>::value, "");
    static_assert(!std::is_member_pointer<T>::value, "");
}

template <class T>
void test_integral()
{
    test_integral_imp<T>();
    test_integral_imp<const T>();
    test_integral_imp<volatile T>();
    test_integral_imp<const volatile T>();
}

int main()
{
    test_integral<bool>();
    test_integral<char>();
    test_integral<signed char>();
    test_integral<unsigned char>();
    test_integral<wchar_t>();
    test_integral<short>();
    test_integral<unsigned short>();
    test_integral<int>();
    test_integral<unsigned int>();
    test_integral<long>();
    test_integral<unsigned long>();
    test_integral<long long>();
    test_integral<unsigned long long>();
#ifndef _LIBCPP_HAS_NO_INT128
    test_integral<__int128_t>();
    test_integral<__uint128_t>();
#endif
}
