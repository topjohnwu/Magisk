//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// remove_volatile

#include <type_traits>

#include "test_macros.h"

template <class T, class U>
void test_remove_volatile_imp()
{
    static_assert((std::is_same<typename std::remove_volatile<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::remove_volatile_t<T>, U>::value), "");
#endif
}

template <class T>
void test_remove_volatile()
{
    test_remove_volatile_imp<T, T>();
    test_remove_volatile_imp<const T, const T>();
    test_remove_volatile_imp<volatile T, T>();
    test_remove_volatile_imp<const volatile T, const T>();
}

int main()
{
    test_remove_volatile<void>();
    test_remove_volatile<int>();
    test_remove_volatile<int[3]>();
    test_remove_volatile<int&>();
    test_remove_volatile<const int&>();
    test_remove_volatile<int*>();
    test_remove_volatile<volatile int*>();
}
