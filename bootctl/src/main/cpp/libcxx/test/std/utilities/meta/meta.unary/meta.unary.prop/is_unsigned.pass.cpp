//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_unsigned

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_unsigned()
{
    static_assert( std::is_unsigned<T>::value, "");
    static_assert( std::is_unsigned<const T>::value, "");
    static_assert( std::is_unsigned<volatile T>::value, "");
    static_assert( std::is_unsigned<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_unsigned_v<T>, "");
    static_assert( std::is_unsigned_v<const T>, "");
    static_assert( std::is_unsigned_v<volatile T>, "");
    static_assert( std::is_unsigned_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_unsigned()
{
    static_assert(!std::is_unsigned<T>::value, "");
    static_assert(!std::is_unsigned<const T>::value, "");
    static_assert(!std::is_unsigned<volatile T>::value, "");
    static_assert(!std::is_unsigned<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_unsigned_v<T>, "");
    static_assert(!std::is_unsigned_v<const T>, "");
    static_assert(!std::is_unsigned_v<volatile T>, "");
    static_assert(!std::is_unsigned_v<const volatile T>, "");
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
    test_is_not_unsigned<void>();
    test_is_not_unsigned<int&>();
    test_is_not_unsigned<Class>();
    test_is_not_unsigned<int*>();
    test_is_not_unsigned<const int*>();
    test_is_not_unsigned<char[3]>();
    test_is_not_unsigned<char[]>();
    test_is_not_unsigned<int>();
    test_is_not_unsigned<double>();
    test_is_not_unsigned<A>();

    test_is_unsigned<bool>();
    test_is_unsigned<unsigned>();

#ifndef _LIBCPP_HAS_NO_INT128
    test_is_unsigned<__uint128_t>();
    test_is_not_unsigned<__int128_t>();
#endif
}
