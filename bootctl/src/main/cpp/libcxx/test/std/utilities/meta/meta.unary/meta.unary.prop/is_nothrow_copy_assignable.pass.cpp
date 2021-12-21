//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_nothrow_copy_assignable

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_has_nothrow_assign()
{
    static_assert( std::is_nothrow_copy_assignable<T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_nothrow_copy_assignable_v<T>, "");
#endif
}

template <class T>
void test_has_not_nothrow_assign()
{
    static_assert(!std::is_nothrow_copy_assignable<T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_nothrow_copy_assignable_v<T>, "");
#endif
}

class Empty
{
};

struct NotEmpty
{
    virtual ~NotEmpty();
};

union Union {};

struct bit_zero
{
    int :  0;
};

struct A
{
    A& operator=(const A&);
};

int main()
{
    test_has_nothrow_assign<int&>();
    test_has_nothrow_assign<Union>();
    test_has_nothrow_assign<Empty>();
    test_has_nothrow_assign<int>();
    test_has_nothrow_assign<double>();
    test_has_nothrow_assign<int*>();
    test_has_nothrow_assign<const int*>();
    test_has_nothrow_assign<NotEmpty>();
    test_has_nothrow_assign<bit_zero>();

    test_has_not_nothrow_assign<const int>();
    test_has_not_nothrow_assign<void>();
    test_has_not_nothrow_assign<A>();

}
