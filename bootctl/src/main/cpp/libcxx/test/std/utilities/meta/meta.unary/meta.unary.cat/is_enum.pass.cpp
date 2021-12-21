//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_enum

#include <type_traits>
#include <cstddef>        // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_enum()
{
    static_assert( std::is_enum<T>::value, "");
    static_assert( std::is_enum<const T>::value, "");
    static_assert( std::is_enum<volatile T>::value, "");
    static_assert( std::is_enum<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_enum_v<T>, "");
    static_assert( std::is_enum_v<const T>, "");
    static_assert( std::is_enum_v<volatile T>, "");
    static_assert( std::is_enum_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_enum()
{
    static_assert(!std::is_enum<T>::value, "");
    static_assert(!std::is_enum<const T>::value, "");
    static_assert(!std::is_enum<volatile T>::value, "");
    static_assert(!std::is_enum<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_enum_v<T>, "");
    static_assert(!std::is_enum_v<const T>, "");
    static_assert(!std::is_enum_v<volatile T>, "");
    static_assert(!std::is_enum_v<const volatile T>, "");
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
    test_is_enum<Enum>();

    test_is_not_enum<std::nullptr_t>();
    test_is_not_enum<void>();
    test_is_not_enum<int>();
    test_is_not_enum<int&>();
    test_is_not_enum<int&&>();
    test_is_not_enum<int*>();
    test_is_not_enum<double>();
    test_is_not_enum<const int*>();
    test_is_not_enum<char[3]>();
    test_is_not_enum<char[]>();
    test_is_not_enum<Union>();
    test_is_not_enum<Empty>();
    test_is_not_enum<bit_zero>();
    test_is_not_enum<NotEmpty>();
    test_is_not_enum<Abstract>();
    test_is_not_enum<FunctionPtr>();
    test_is_not_enum<incomplete_type>();
}
