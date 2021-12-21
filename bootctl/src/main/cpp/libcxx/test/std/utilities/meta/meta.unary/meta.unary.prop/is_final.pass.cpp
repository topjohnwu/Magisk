//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// type_traits

// is_final

#include <type_traits>
#include "test_macros.h"

struct P final { };
union U1 { };
union U2 final { };

template <class T>
void test_is_final()
{
    static_assert( std::is_final<T>::value, "");
    static_assert( std::is_final<const T>::value, "");
    static_assert( std::is_final<volatile T>::value, "");
    static_assert( std::is_final<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_final_v<T>, "");
    static_assert( std::is_final_v<const T>, "");
    static_assert( std::is_final_v<volatile T>, "");
    static_assert( std::is_final_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_final()
{
    static_assert(!std::is_final<T>::value, "");
    static_assert(!std::is_final<const T>::value, "");
    static_assert(!std::is_final<volatile T>::value, "");
    static_assert(!std::is_final<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_final_v<T>, "");
    static_assert(!std::is_final_v<const T>, "");
    static_assert(!std::is_final_v<volatile T>, "");
    static_assert(!std::is_final_v<const volatile T>, "");
#endif
}

int main ()
{
    test_is_not_final<int>();
    test_is_not_final<int*>();
    test_is_final    <P>();
    test_is_not_final<P*>();
    test_is_not_final<U1>();
    test_is_not_final<U1*>();
    test_is_final    <U2>();
    test_is_not_final<U2*>();
}
