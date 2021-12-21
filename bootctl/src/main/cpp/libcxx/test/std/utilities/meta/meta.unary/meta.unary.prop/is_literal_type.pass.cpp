//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_literal_type

#include <type_traits>
#include <cstddef>       // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_literal_type()
{
    static_assert( std::is_literal_type<T>::value, "");
    static_assert( std::is_literal_type<const T>::value, "");
    static_assert( std::is_literal_type<volatile T>::value, "");
    static_assert( std::is_literal_type<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_literal_type_v<T>, "");
    static_assert( std::is_literal_type_v<const T>, "");
    static_assert( std::is_literal_type_v<volatile T>, "");
    static_assert( std::is_literal_type_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_literal_type()
{
    static_assert(!std::is_literal_type<T>::value, "");
    static_assert(!std::is_literal_type<const T>::value, "");
    static_assert(!std::is_literal_type<volatile T>::value, "");
    static_assert(!std::is_literal_type<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_literal_type_v<T>, "");
    static_assert(!std::is_literal_type_v<const T>, "");
    static_assert(!std::is_literal_type_v<volatile T>, "");
    static_assert(!std::is_literal_type_v<const volatile T>, "");
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

typedef void (*FunctionPtr)();

int main()
{
#if TEST_STD_VER >= 11
    test_is_literal_type<std::nullptr_t>();
#endif

// Before C++14, void was not a literal type
// In C++14, cv-void is a literal type
#if TEST_STD_VER < 14
    test_is_not_literal_type<void>();
#else
    test_is_literal_type<void>();
#endif

    test_is_literal_type<int>();
    test_is_literal_type<int*>();
    test_is_literal_type<const int*>();
    test_is_literal_type<int&>();
#if TEST_STD_VER >= 11
    test_is_literal_type<int&&>();
#endif
    test_is_literal_type<double>();
    test_is_literal_type<char[3]>();
    test_is_literal_type<char[]>();
    test_is_literal_type<Empty>();
    test_is_literal_type<bit_zero>();
    test_is_literal_type<Union>();
    test_is_literal_type<Enum>();
    test_is_literal_type<FunctionPtr>();

    test_is_not_literal_type<NotEmpty>();
    test_is_not_literal_type<Abstract>();
}
