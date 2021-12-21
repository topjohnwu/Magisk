//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_void

#include <type_traits>
#include <cstddef>        // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_void()
{
    static_assert( std::is_void<T>::value, "");
    static_assert( std::is_void<const T>::value, "");
    static_assert( std::is_void<volatile T>::value, "");
    static_assert( std::is_void<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_void_v<T>, "");
    static_assert( std::is_void_v<const T>, "");
    static_assert( std::is_void_v<volatile T>, "");
    static_assert( std::is_void_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_void()
{
    static_assert(!std::is_void<T>::value, "");
    static_assert(!std::is_void<const T>::value, "");
    static_assert(!std::is_void<volatile T>::value, "");
    static_assert(!std::is_void<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_void_v<T>, "");
    static_assert(!std::is_void_v<const T>, "");
    static_assert(!std::is_void_v<volatile T>, "");
    static_assert(!std::is_void_v<const volatile T>, "");
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
    test_is_void<void>();

    test_is_not_void<int>();
    test_is_not_void<int*>();
    test_is_not_void<int&>();
    test_is_not_void<int&&>();
    test_is_not_void<double>();
    test_is_not_void<const int*>();
    test_is_not_void<char[3]>();
    test_is_not_void<char[]>();
    test_is_not_void<Union>();
    test_is_not_void<Empty>();
    test_is_not_void<bit_zero>();
    test_is_not_void<NotEmpty>();
    test_is_not_void<Abstract>();
    test_is_not_void<Enum>();
    test_is_not_void<FunctionPtr>();
    test_is_not_void<incomplete_type>();
}
