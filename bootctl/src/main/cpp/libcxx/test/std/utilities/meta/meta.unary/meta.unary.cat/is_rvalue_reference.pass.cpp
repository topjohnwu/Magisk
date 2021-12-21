//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_rvalue_reference

// UNSUPPORTED: c++98, c++03

#include <type_traits>
#include <cstddef>        // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_rvalue_reference()
{
    static_assert( std::is_rvalue_reference<T>::value, "");
    static_assert( std::is_rvalue_reference<const T>::value, "");
    static_assert( std::is_rvalue_reference<volatile T>::value, "");
    static_assert( std::is_rvalue_reference<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_rvalue_reference_v<T>, "");
    static_assert( std::is_rvalue_reference_v<const T>, "");
    static_assert( std::is_rvalue_reference_v<volatile T>, "");
    static_assert( std::is_rvalue_reference_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_rvalue_reference()
{
    static_assert(!std::is_rvalue_reference<T>::value, "");
    static_assert(!std::is_rvalue_reference<const T>::value, "");
    static_assert(!std::is_rvalue_reference<volatile T>::value, "");
    static_assert(!std::is_rvalue_reference<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_rvalue_reference_v<T>, "");
    static_assert(!std::is_rvalue_reference_v<const T>, "");
    static_assert(!std::is_rvalue_reference_v<volatile T>, "");
    static_assert(!std::is_rvalue_reference_v<const volatile T>, "");
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

enum Enum {zero, one};
struct incomplete_type;

typedef void (*FunctionPtr)();

int main()
{
    test_is_rvalue_reference<int&&>();

    test_is_not_rvalue_reference<std::nullptr_t>();
    test_is_not_rvalue_reference<void>();
    test_is_not_rvalue_reference<int>();
    test_is_not_rvalue_reference<int*>();
    test_is_not_rvalue_reference<int&>();
    test_is_not_rvalue_reference<double>();
    test_is_not_rvalue_reference<const int*>();
    test_is_not_rvalue_reference<char[3]>();
    test_is_not_rvalue_reference<char[]>();
    test_is_not_rvalue_reference<Union>();
    test_is_not_rvalue_reference<Enum>();
    test_is_not_rvalue_reference<FunctionPtr>();
    test_is_not_rvalue_reference<Empty>();
    test_is_not_rvalue_reference<bit_zero>();
    test_is_not_rvalue_reference<NotEmpty>();
    test_is_not_rvalue_reference<Abstract>();
    test_is_not_rvalue_reference<incomplete_type>();
}
