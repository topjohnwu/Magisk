//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_trivial

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_trivial()
{
    static_assert( std::is_trivial<T>::value, "");
    static_assert( std::is_trivial<const T>::value, "");
    static_assert( std::is_trivial<volatile T>::value, "");
    static_assert( std::is_trivial<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_trivial_v<T>, "");
    static_assert( std::is_trivial_v<const T>, "");
    static_assert( std::is_trivial_v<volatile T>, "");
    static_assert( std::is_trivial_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_trivial()
{
    static_assert(!std::is_trivial<T>::value, "");
    static_assert(!std::is_trivial<const T>::value, "");
    static_assert(!std::is_trivial<volatile T>::value, "");
    static_assert(!std::is_trivial<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_trivial_v<T>, "");
    static_assert(!std::is_trivial_v<const T>, "");
    static_assert(!std::is_trivial_v<volatile T>, "");
    static_assert(!std::is_trivial_v<const volatile T>, "");
#endif
}

struct A {};

class B
{
public:
    B();
};

int main()
{
    test_is_trivial<int> ();
    test_is_trivial<A> ();

    test_is_not_trivial<int&> ();
    test_is_not_trivial<volatile int&> ();
    test_is_not_trivial<B> ();
}
