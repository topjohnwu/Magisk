//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_nothrow_default_constructible

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_nothrow_default_constructible()
{
    static_assert( std::is_nothrow_default_constructible<T>::value, "");
    static_assert( std::is_nothrow_default_constructible<const T>::value, "");
    static_assert( std::is_nothrow_default_constructible<volatile T>::value, "");
    static_assert( std::is_nothrow_default_constructible<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_nothrow_default_constructible_v<T>, "");
    static_assert( std::is_nothrow_default_constructible_v<const T>, "");
    static_assert( std::is_nothrow_default_constructible_v<volatile T>, "");
    static_assert( std::is_nothrow_default_constructible_v<const volatile T>, "");
#endif
}

template <class T>
void test_has_not_nothrow_default_constructor()
{
    static_assert(!std::is_nothrow_default_constructible<T>::value, "");
    static_assert(!std::is_nothrow_default_constructible<const T>::value, "");
    static_assert(!std::is_nothrow_default_constructible<volatile T>::value, "");
    static_assert(!std::is_nothrow_default_constructible<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_nothrow_default_constructible_v<T>, "");
    static_assert(!std::is_nothrow_default_constructible_v<const T>, "");
    static_assert(!std::is_nothrow_default_constructible_v<volatile T>, "");
    static_assert(!std::is_nothrow_default_constructible_v<const volatile T>, "");
#endif
}

class Empty
{
};

union Union {};

struct bit_zero
{
    int :  0;
};

struct A
{
    A();
};

#if TEST_STD_VER >= 11
struct DThrows
{
    DThrows()  noexcept(true) {}
    ~DThrows() noexcept(false) {}
};
#endif

int main()
{
    test_has_not_nothrow_default_constructor<void>();
    test_has_not_nothrow_default_constructor<int&>();
    test_has_not_nothrow_default_constructor<A>();
#if TEST_STD_VER >= 11
    test_has_not_nothrow_default_constructor<DThrows>(); // This is LWG2116
#endif

    test_is_nothrow_default_constructible<Union>();
    test_is_nothrow_default_constructible<Empty>();
    test_is_nothrow_default_constructible<int>();
    test_is_nothrow_default_constructible<double>();
    test_is_nothrow_default_constructible<int*>();
    test_is_nothrow_default_constructible<const int*>();
    test_is_nothrow_default_constructible<char[3]>();
    test_is_nothrow_default_constructible<bit_zero>();
}
