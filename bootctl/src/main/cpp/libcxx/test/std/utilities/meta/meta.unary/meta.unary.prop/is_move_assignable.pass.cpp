//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_move_assignable

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_move_assignable()
{
    static_assert(( std::is_move_assignable<T>::value), "");
#if TEST_STD_VER > 14
    static_assert(( std::is_move_assignable_v<T>), "");
#endif
}

template <class T>
void test_is_not_move_assignable()
{
    static_assert((!std::is_move_assignable<T>::value), "");
#if TEST_STD_VER > 14
    static_assert((!std::is_move_assignable_v<T>), "");
#endif
}

class Empty
{
};

class NotEmpty
{
public:
    virtual ~NotEmpty();
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

int main()
{
    test_is_move_assignable<int> ();
    test_is_move_assignable<A> ();
    test_is_move_assignable<bit_zero> ();
    test_is_move_assignable<Union> ();
    test_is_move_assignable<NotEmpty> ();
    test_is_move_assignable<Empty> ();

#if TEST_STD_VER >= 11
    test_is_not_move_assignable<const int> ();
    test_is_not_move_assignable<int[]> ();
    test_is_not_move_assignable<int[3]> ();
#endif
    test_is_not_move_assignable<void> ();
}
