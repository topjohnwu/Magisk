//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_volatile

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_volatile()
{
    static_assert(!std::is_volatile<T>::value, "");
    static_assert(!std::is_volatile<const T>::value, "");
    static_assert( std::is_volatile<volatile T>::value, "");
    static_assert( std::is_volatile<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_volatile_v<T>, "");
    static_assert(!std::is_volatile_v<const T>, "");
    static_assert( std::is_volatile_v<volatile T>, "");
    static_assert( std::is_volatile_v<const volatile T>, "");
#endif
}

struct A; // incomplete

int main()
{
    test_is_volatile<void>();
    test_is_volatile<int>();
    test_is_volatile<double>();
    test_is_volatile<int*>();
    test_is_volatile<const int*>();
    test_is_volatile<char[3]>();
    test_is_volatile<char[]>();

    test_is_volatile<A>();

    static_assert(!std::is_volatile<int&>::value, "");
    static_assert(!std::is_volatile<volatile int&>::value, "");
}
