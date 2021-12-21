//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_signed

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_signed()
{
    static_assert( std::is_signed<T>::value, "");
    static_assert( std::is_signed<const T>::value, "");
    static_assert( std::is_signed<volatile T>::value, "");
    static_assert( std::is_signed<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_signed_v<T>, "");
    static_assert( std::is_signed_v<const T>, "");
    static_assert( std::is_signed_v<volatile T>, "");
    static_assert( std::is_signed_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_signed()
{
    static_assert(!std::is_signed<T>::value, "");
    static_assert(!std::is_signed<const T>::value, "");
    static_assert(!std::is_signed<volatile T>::value, "");
    static_assert(!std::is_signed<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_signed_v<T>, "");
    static_assert(!std::is_signed_v<const T>, "");
    static_assert(!std::is_signed_v<volatile T>, "");
    static_assert(!std::is_signed_v<const volatile T>, "");
#endif
}

class Class
{
public:
    ~Class();
};

struct A; // incomplete

int main()
{
    test_is_not_signed<void>();
    test_is_not_signed<int&>();
    test_is_not_signed<Class>();
    test_is_not_signed<int*>();
    test_is_not_signed<const int*>();
    test_is_not_signed<char[3]>();
    test_is_not_signed<char[]>();
    test_is_not_signed<bool>();
    test_is_not_signed<unsigned>();
    test_is_not_signed<A>();

    test_is_signed<int>();
    test_is_signed<double>();

#ifndef _LIBCPP_HAS_NO_INT128
    test_is_signed<__int128_t>();
    test_is_not_signed<__uint128_t>();
#endif
}
