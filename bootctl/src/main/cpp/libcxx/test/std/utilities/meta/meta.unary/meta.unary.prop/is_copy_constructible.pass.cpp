//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_copy_constructible

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_copy_constructible()
{
    static_assert( std::is_copy_constructible<T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_copy_constructible_v<T>, "");
#endif
}

template <class T>
void test_is_not_copy_constructible()
{
    static_assert(!std::is_copy_constructible<T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_copy_constructible_v<T>, "");
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

class B
{
    B(const B&);
};

struct C
{
    C(C&);  // not const
    void operator=(C&);  // not const
};

int main()
{
    test_is_copy_constructible<A>();
    test_is_copy_constructible<int&>();
    test_is_copy_constructible<Union>();
    test_is_copy_constructible<Empty>();
    test_is_copy_constructible<int>();
    test_is_copy_constructible<double>();
    test_is_copy_constructible<int*>();
    test_is_copy_constructible<const int*>();
    test_is_copy_constructible<NotEmpty>();
    test_is_copy_constructible<bit_zero>();

    test_is_not_copy_constructible<char[3]>();
    test_is_not_copy_constructible<char[]>();
    test_is_not_copy_constructible<void>();
    test_is_not_copy_constructible<Abstract>();
    test_is_not_copy_constructible<C>();
#if TEST_STD_VER >= 11
    test_is_not_copy_constructible<B>();
#endif
}
