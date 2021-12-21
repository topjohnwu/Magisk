//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_nothrow_assignable

#include <type_traits>
#include "test_macros.h"

template <class T, class U>
void test_is_nothrow_assignable()
{
    static_assert(( std::is_nothrow_assignable<T, U>::value), "");
#if TEST_STD_VER > 14
    static_assert(( std::is_nothrow_assignable_v<T, U>), "");
#endif
}

template <class T, class U>
void test_is_not_nothrow_assignable()
{
    static_assert((!std::is_nothrow_assignable<T, U>::value), "");
#if TEST_STD_VER > 14
    static_assert((!std::is_nothrow_assignable_v<T, U>), "");
#endif
}

struct A
{
};

struct B
{
    void operator=(A);
};

struct C
{
    void operator=(C&);  // not const
};

int main()
{
    test_is_nothrow_assignable<int&, int&> ();
    test_is_nothrow_assignable<int&, int> ();
#if TEST_STD_VER >= 11
    test_is_nothrow_assignable<int&, double> ();
#endif

    test_is_not_nothrow_assignable<int, int&> ();
    test_is_not_nothrow_assignable<int, int> ();
    test_is_not_nothrow_assignable<B, A> ();
    test_is_not_nothrow_assignable<A, B> ();
    test_is_not_nothrow_assignable<C, C&> ();
}
