//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_abstract

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_abstract()
{
    static_assert( std::is_abstract<T>::value, "");
    static_assert( std::is_abstract<const T>::value, "");
    static_assert( std::is_abstract<volatile T>::value, "");
    static_assert( std::is_abstract<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_abstract_v<T>, "");
    static_assert( std::is_abstract_v<const T>, "");
    static_assert( std::is_abstract_v<volatile T>, "");
    static_assert( std::is_abstract_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_abstract()
{
    static_assert(!std::is_abstract<T>::value, "");
    static_assert(!std::is_abstract<const T>::value, "");
    static_assert(!std::is_abstract<volatile T>::value, "");
    static_assert(!std::is_abstract<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_abstract_v<T>, "");
    static_assert(!std::is_abstract_v<const T>, "");
    static_assert(!std::is_abstract_v<volatile T>, "");
    static_assert(!std::is_abstract_v<const volatile T>, "");
#endif
}

class Empty
{
};

class NotEmpty
{
    virtual ~NotEmpty();
};

union Union {};

struct bit_zero
{
    int :  0;
};

class Abstract
{
    virtual ~Abstract() = 0;
};

template <class>
struct AbstractTemplate {
  virtual void test() = 0;
};

template <>
struct AbstractTemplate<double> {};

int main()
{
    test_is_not_abstract<void>();
    test_is_not_abstract<int&>();
    test_is_not_abstract<int>();
    test_is_not_abstract<double>();
    test_is_not_abstract<int*>();
    test_is_not_abstract<const int*>();
    test_is_not_abstract<char[3]>();
    test_is_not_abstract<char[]>();
    test_is_not_abstract<Union>();
    test_is_not_abstract<Empty>();
    test_is_not_abstract<bit_zero>();
    test_is_not_abstract<NotEmpty>();

    test_is_abstract<Abstract>();
    test_is_abstract<AbstractTemplate<int> >();
    test_is_not_abstract<AbstractTemplate<double> >();
}
