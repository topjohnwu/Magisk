//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_object

#include <type_traits>
#include <cstddef>         // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_object()
{
    static_assert( std::is_object<T>::value, "");
    static_assert( std::is_object<const T>::value, "");
    static_assert( std::is_object<volatile T>::value, "");
    static_assert( std::is_object<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_object_v<T>, "");
    static_assert( std::is_object_v<const T>, "");
    static_assert( std::is_object_v<volatile T>, "");
    static_assert( std::is_object_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_object()
{
    static_assert(!std::is_object<T>::value, "");
    static_assert(!std::is_object<const T>::value, "");
    static_assert(!std::is_object<volatile T>::value, "");
    static_assert(!std::is_object<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_object_v<T>, "");
    static_assert(!std::is_object_v<const T>, "");
    static_assert(!std::is_object_v<volatile T>, "");
    static_assert(!std::is_object_v<const volatile T>, "");
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
// An object type is a (possibly cv-qualified) type that is not a function type,
// not a reference type, and not a void type.

    test_is_object<std::nullptr_t>();
    test_is_object<void *>();
    test_is_object<char[3]>();
    test_is_object<char[]>();
    test_is_object<int>();
    test_is_object<int*>();
    test_is_object<Union>();
    test_is_object<int*>();
    test_is_object<const int*>();
    test_is_object<Enum>();
    test_is_object<incomplete_type>();
    test_is_object<bit_zero>();
    test_is_object<NotEmpty>();
    test_is_object<Abstract>();
    test_is_object<FunctionPtr>();
    test_is_object<int Empty::*>();
    test_is_object<void (Empty::*)(int)>();

    test_is_not_object<void>();
    test_is_not_object<int&>();
    test_is_not_object<int&&>();
    test_is_not_object<int(int)>();
}
