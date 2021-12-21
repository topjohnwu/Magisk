//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// type_traits

// remove_cvref

#include <type_traits>

#include "test_macros.h"

template <class T, class U>
void test_remove_cvref()
{
    static_assert((std::is_same<typename std::remove_cvref<T>::type, U>::value), "");
    static_assert((std::is_same<         std::remove_cvref_t<T>,     U>::value), "");
}

int main()
{
    test_remove_cvref<void, void>();
    test_remove_cvref<int, int>();
    test_remove_cvref<const int, int>();
    test_remove_cvref<const volatile int, int>();
    test_remove_cvref<volatile int, int>();

// Doesn't decay
    test_remove_cvref<int[3],                 int[3]>();
    test_remove_cvref<int const [3],          int[3]>();
    test_remove_cvref<int volatile [3],       int[3]>();
    test_remove_cvref<int const volatile [3], int[3]>();
    test_remove_cvref<void(), void ()>();

    test_remove_cvref<int &, int>();
    test_remove_cvref<const int &, int>();
    test_remove_cvref<const volatile int &, int>();
    test_remove_cvref<volatile int &, int>();

    test_remove_cvref<int*, int*>();
    test_remove_cvref<int(int) const, int(int) const>();
    test_remove_cvref<int(int) volatile, int(int) volatile>();
    test_remove_cvref<int(int)  &, int(int)  &>();
    test_remove_cvref<int(int) &&, int(int) &&>();
}
