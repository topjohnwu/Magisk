//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_standard_layout

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_standard_layout()
{
    static_assert( std::is_standard_layout<T>::value, "");
    static_assert( std::is_standard_layout<const T>::value, "");
    static_assert( std::is_standard_layout<volatile T>::value, "");
    static_assert( std::is_standard_layout<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_standard_layout_v<T>, "");
    static_assert( std::is_standard_layout_v<const T>, "");
    static_assert( std::is_standard_layout_v<volatile T>, "");
    static_assert( std::is_standard_layout_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_standard_layout()
{
    static_assert(!std::is_standard_layout<T>::value, "");
    static_assert(!std::is_standard_layout<const T>::value, "");
    static_assert(!std::is_standard_layout<volatile T>::value, "");
    static_assert(!std::is_standard_layout<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_standard_layout_v<T>, "");
    static_assert(!std::is_standard_layout_v<const T>, "");
    static_assert(!std::is_standard_layout_v<volatile T>, "");
    static_assert(!std::is_standard_layout_v<const volatile T>, "");
#endif
}

template <class T1, class T2>
struct pair
{
    T1 first;
    T2 second;
};

int main()
{
    test_is_standard_layout<int> ();
    test_is_standard_layout<int[3]> ();
    test_is_standard_layout<pair<int, double> > ();

    test_is_not_standard_layout<int&> ();
}
