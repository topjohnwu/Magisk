//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_arithmetic

#include <type_traits>
#include <cstddef>         // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_arithmetic()
{
    static_assert( std::is_arithmetic<T>::value, "");
    static_assert( std::is_arithmetic<const T>::value, "");
    static_assert( std::is_arithmetic<volatile T>::value, "");
    static_assert( std::is_arithmetic<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_arithmetic_v<T>, "");
    static_assert( std::is_arithmetic_v<const T>, "");
    static_assert( std::is_arithmetic_v<volatile T>, "");
    static_assert( std::is_arithmetic_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_arithmetic()
{
    static_assert(!std::is_arithmetic<T>::value, "");
    static_assert(!std::is_arithmetic<const T>::value, "");
    static_assert(!std::is_arithmetic<volatile T>::value, "");
    static_assert(!std::is_arithmetic<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_arithmetic_v<T>, "");
    static_assert(!std::is_arithmetic_v<const T>, "");
    static_assert(!std::is_arithmetic_v<volatile T>, "");
    static_assert(!std::is_arithmetic_v<const volatile T>, "");
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
    test_is_arithmetic<short>();
    test_is_arithmetic<unsigned short>();
    test_is_arithmetic<int>();
    test_is_arithmetic<unsigned int>();
    test_is_arithmetic<long>();
    test_is_arithmetic<unsigned long>();
    test_is_arithmetic<bool>();
    test_is_arithmetic<char>();
    test_is_arithmetic<signed char>();
    test_is_arithmetic<unsigned char>();
    test_is_arithmetic<wchar_t>();
    test_is_arithmetic<double>();

    test_is_not_arithmetic<std::nullptr_t>();
    test_is_not_arithmetic<void>();
    test_is_not_arithmetic<int&>();
    test_is_not_arithmetic<int&&>();
    test_is_not_arithmetic<int*>();
    test_is_not_arithmetic<const int*>();
    test_is_not_arithmetic<char[3]>();
    test_is_not_arithmetic<char[]>();
    test_is_not_arithmetic<Union>();
    test_is_not_arithmetic<Enum>();
    test_is_not_arithmetic<FunctionPtr>();
    test_is_not_arithmetic<Empty>();
    test_is_not_arithmetic<incomplete_type>();
    test_is_not_arithmetic<bit_zero>();
    test_is_not_arithmetic<NotEmpty>();
    test_is_not_arithmetic<Abstract>();
}
