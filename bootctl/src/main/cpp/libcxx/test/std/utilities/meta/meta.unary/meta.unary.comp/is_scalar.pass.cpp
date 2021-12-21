//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_scalar

#include <type_traits>
#include <cstddef>         // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_scalar()
{
    static_assert( std::is_scalar<T>::value, "");
    static_assert( std::is_scalar<const T>::value, "");
    static_assert( std::is_scalar<volatile T>::value, "");
    static_assert( std::is_scalar<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_scalar_v<T>, "");
    static_assert( std::is_scalar_v<const T>, "");
    static_assert( std::is_scalar_v<volatile T>, "");
    static_assert( std::is_scalar_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_scalar()
{
    static_assert(!std::is_scalar<T>::value, "");
    static_assert(!std::is_scalar<const T>::value, "");
    static_assert(!std::is_scalar<volatile T>::value, "");
    static_assert(!std::is_scalar<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_scalar_v<T>, "");
    static_assert(!std::is_scalar_v<const T>, "");
    static_assert(!std::is_scalar_v<volatile T>, "");
    static_assert(!std::is_scalar_v<const volatile T>, "");
#endif
}

class incomplete_type;

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

typedef void (*FunctionPtr)();


int main()
{
//  Arithmetic types (3.9.1), enumeration types, pointer types, pointer to member types (3.9.2),
//    std::nullptr_t, and cv-qualified versions of these types (3.9.3)
//    are collectively called scalar types.

    test_is_scalar<std::nullptr_t>();
    test_is_scalar<short>();
    test_is_scalar<unsigned short>();
    test_is_scalar<int>();
    test_is_scalar<unsigned int>();
    test_is_scalar<long>();
    test_is_scalar<unsigned long>();
    test_is_scalar<bool>();
    test_is_scalar<char>();
    test_is_scalar<signed char>();
    test_is_scalar<unsigned char>();
    test_is_scalar<wchar_t>();
    test_is_scalar<double>();
    test_is_scalar<int*>();
    test_is_scalar<const int*>();
    test_is_scalar<int Empty::*>();
    test_is_scalar<void (Empty::*)(int)>();
    test_is_scalar<Enum>();
    test_is_scalar<FunctionPtr>();

    test_is_not_scalar<void>();
    test_is_not_scalar<int&>();
    test_is_not_scalar<int&&>();
    test_is_not_scalar<char[3]>();
    test_is_not_scalar<char[]>();
    test_is_not_scalar<Union>();
    test_is_not_scalar<Empty>();
    test_is_not_scalar<incomplete_type>();
    test_is_not_scalar<bit_zero>();
    test_is_not_scalar<NotEmpty>();
    test_is_not_scalar<Abstract>();
    test_is_not_scalar<int(int)>();
}
