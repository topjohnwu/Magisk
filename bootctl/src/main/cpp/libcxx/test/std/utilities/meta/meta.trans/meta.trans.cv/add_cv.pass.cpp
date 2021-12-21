//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// add_cv

#include <type_traits>

#include "test_macros.h"

template <class T, class U>
void test_add_cv_imp()
{
    static_assert((std::is_same<typename std::add_cv<T>::type, const volatile U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::add_cv_t<T>, U>::value), "");
#endif
}

template <class T>
void test_add_cv()
{
    test_add_cv_imp<T, const volatile T>();
    test_add_cv_imp<const T, const volatile T>();
    test_add_cv_imp<volatile T, volatile const T>();
    test_add_cv_imp<const volatile T, const volatile T>();
}

int main()
{
    test_add_cv<void>();
    test_add_cv<int>();
    test_add_cv<int[3]>();
    test_add_cv<int&>();
    test_add_cv<const int&>();
    test_add_cv<int*>();
    test_add_cv<const int*>();
}
