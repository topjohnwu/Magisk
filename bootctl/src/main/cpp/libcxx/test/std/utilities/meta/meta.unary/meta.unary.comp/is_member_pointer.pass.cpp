//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_member_pointer

#include <type_traits>
#include <cstddef>         // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_member_pointer()
{
    static_assert( std::is_member_pointer<T>::value, "");
    static_assert( std::is_member_pointer<const T>::value, "");
    static_assert( std::is_member_pointer<volatile T>::value, "");
    static_assert( std::is_member_pointer<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_member_pointer_v<T>, "");
    static_assert( std::is_member_pointer_v<const T>, "");
    static_assert( std::is_member_pointer_v<volatile T>, "");
    static_assert( std::is_member_pointer_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_member_pointer()
{
    static_assert(!std::is_member_pointer<T>::value, "");
    static_assert(!std::is_member_pointer<const T>::value, "");
    static_assert(!std::is_member_pointer<volatile T>::value, "");
    static_assert(!std::is_member_pointer<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_member_pointer_v<T>, "");
    static_assert(!std::is_member_pointer_v<const T>, "");
    static_assert(!std::is_member_pointer_v<volatile T>, "");
    static_assert(!std::is_member_pointer_v<const volatile T>, "");
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

    test_is_member_pointer<int Empty::*>();
    test_is_member_pointer<void (Empty::*)(int)>();

    test_is_not_member_pointer<std::nullptr_t>();
    test_is_not_member_pointer<void>();
    test_is_not_member_pointer<void *>();
    test_is_not_member_pointer<int>();
    test_is_not_member_pointer<int*>();
    test_is_not_member_pointer<const int*>();
    test_is_not_member_pointer<int&>();
    test_is_not_member_pointer<int&&>();
    test_is_not_member_pointer<double>();
    test_is_not_member_pointer<char[3]>();
    test_is_not_member_pointer<char[]>();
    test_is_not_member_pointer<Union>();
    test_is_not_member_pointer<Empty>();
    test_is_not_member_pointer<incomplete_type>();
    test_is_not_member_pointer<bit_zero>();
    test_is_not_member_pointer<NotEmpty>();
    test_is_not_member_pointer<Abstract>();
    test_is_not_member_pointer<int(int)>();
    test_is_not_member_pointer<Enum>();
    test_is_not_member_pointer<FunctionPtr>();
}
