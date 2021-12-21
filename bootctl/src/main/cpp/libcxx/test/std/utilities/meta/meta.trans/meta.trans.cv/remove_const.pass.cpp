//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// remove_const

#include <type_traits>

#include "test_macros.h"

template <class T, class U>
void test_remove_const_imp()
{
    static_assert((std::is_same<typename std::remove_const<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::remove_const_t<T>, U>::value), "");
#endif
}

template <class T>
void test_remove_const()
{
    test_remove_const_imp<T, T>();
    test_remove_const_imp<const T, T>();
    test_remove_const_imp<volatile T, volatile T>();
    test_remove_const_imp<const volatile T, volatile T>();
}

int main()
{
    test_remove_const<void>();
    test_remove_const<int>();
    test_remove_const<int[3]>();
    test_remove_const<int&>();
    test_remove_const<const int&>();
    test_remove_const<int*>();
    test_remove_const<const int*>();
}
