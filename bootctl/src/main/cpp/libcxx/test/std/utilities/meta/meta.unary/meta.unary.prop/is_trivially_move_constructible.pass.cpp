//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_trivially_move_constructible

// XFAIL: gcc-4.9

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_trivially_move_constructible()
{
    static_assert( std::is_trivially_move_constructible<T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_trivially_move_constructible_v<T>, "");
#endif
}

template <class T>
void test_has_not_trivial_move_constructor()
{
    static_assert(!std::is_trivially_move_constructible<T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_trivially_move_constructible_v<T>, "");
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

class Abstract
{
public:
    virtual ~Abstract() = 0;
};

struct A
{
    A(const A&);
};

#if TEST_STD_VER >= 11

struct MoveOnly1
{
    MoveOnly1(MoveOnly1&&);
};

struct MoveOnly2
{
    MoveOnly2(MoveOnly2&&) = default;
};

#endif

int main()
{
    test_has_not_trivial_move_constructor<void>();
    test_has_not_trivial_move_constructor<A>();
    test_has_not_trivial_move_constructor<Abstract>();
    test_has_not_trivial_move_constructor<NotEmpty>();

    test_is_trivially_move_constructible<Union>();
    test_is_trivially_move_constructible<Empty>();
    test_is_trivially_move_constructible<int>();
    test_is_trivially_move_constructible<double>();
    test_is_trivially_move_constructible<int*>();
    test_is_trivially_move_constructible<const int*>();
    test_is_trivially_move_constructible<bit_zero>();

#if TEST_STD_VER >= 11
    static_assert(!std::is_trivially_move_constructible<MoveOnly1>::value, "");
    static_assert( std::is_trivially_move_constructible<MoveOnly2>::value, "");
#endif
}
