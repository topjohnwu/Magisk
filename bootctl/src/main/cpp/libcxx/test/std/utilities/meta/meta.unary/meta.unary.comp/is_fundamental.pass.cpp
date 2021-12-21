//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_fundamental

#include <type_traits>
#include <cstddef>         // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_fundamental()
{
    static_assert( std::is_fundamental<T>::value, "");
    static_assert( std::is_fundamental<const T>::value, "");
    static_assert( std::is_fundamental<volatile T>::value, "");
    static_assert( std::is_fundamental<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_fundamental_v<T>, "");
    static_assert( std::is_fundamental_v<const T>, "");
    static_assert( std::is_fundamental_v<volatile T>, "");
    static_assert( std::is_fundamental_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_fundamental()
{
    static_assert(!std::is_fundamental<T>::value, "");
    static_assert(!std::is_fundamental<const T>::value, "");
    static_assert(!std::is_fundamental<volatile T>::value, "");
    static_assert(!std::is_fundamental<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_fundamental_v<T>, "");
    static_assert(!std::is_fundamental_v<const T>, "");
    static_assert(!std::is_fundamental_v<volatile T>, "");
    static_assert(!std::is_fundamental_v<const volatile T>, "");
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
    test_is_fundamental<std::nullptr_t>();
    test_is_fundamental<void>();
    test_is_fundamental<short>();
    test_is_fundamental<unsigned short>();
    test_is_fundamental<int>();
    test_is_fundamental<unsigned int>();
    test_is_fundamental<long>();
    test_is_fundamental<unsigned long>();
    test_is_fundamental<long long>();
    test_is_fundamental<unsigned long long>();
    test_is_fundamental<bool>();
    test_is_fundamental<char>();
    test_is_fundamental<signed char>();
    test_is_fundamental<unsigned char>();
    test_is_fundamental<wchar_t>();
    test_is_fundamental<double>();
    test_is_fundamental<float>();
    test_is_fundamental<double>();
    test_is_fundamental<long double>();
    test_is_fundamental<char16_t>();
    test_is_fundamental<char32_t>();

    test_is_not_fundamental<char[3]>();
    test_is_not_fundamental<char[]>();
    test_is_not_fundamental<void *>();
    test_is_not_fundamental<FunctionPtr>();
    test_is_not_fundamental<int&>();
    test_is_not_fundamental<int&&>();
    test_is_not_fundamental<Union>();
    test_is_not_fundamental<Empty>();
    test_is_not_fundamental<incomplete_type>();
    test_is_not_fundamental<bit_zero>();
    test_is_not_fundamental<int*>();
    test_is_not_fundamental<const int*>();
    test_is_not_fundamental<Enum>();
    test_is_not_fundamental<NotEmpty>();
    test_is_not_fundamental<Abstract>();
}
