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
#include <cstddef>        // for std::nullptr_t
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
    test_is_member_pointer<int Abstract::*>();
    test_is_member_pointer<double NotEmpty::*>();
    test_is_member_pointer<FunctionPtr Empty::*>();
    test_is_member_pointer<void (Empty::*)()>();

    test_is_not_member_pointer<std::nullptr_t>();
    test_is_not_member_pointer<void>();
    test_is_not_member_pointer<int>();
    test_is_not_member_pointer<int&>();
    test_is_not_member_pointer<int&&>();
    test_is_not_member_pointer<int*>();
    test_is_not_member_pointer<double>();
    test_is_not_member_pointer<const int*>();
    test_is_not_member_pointer<char[3]>();
    test_is_not_member_pointer<char[]>();
    test_is_not_member_pointer<Union>();
    test_is_not_member_pointer<Enum>();
    test_is_not_member_pointer<FunctionPtr>();
    test_is_not_member_pointer<Empty>();
    test_is_not_member_pointer<bit_zero>();
    test_is_not_member_pointer<NotEmpty>();
    test_is_not_member_pointer<Abstract>();
    test_is_not_member_pointer<incomplete_type>();

#if TEST_STD_VER >= 11
  test_is_member_pointer<int (Empty::*)(int, ...) const>();
  test_is_member_pointer<int (Empty::*)(int, long, long) const noexcept>();
  test_is_member_pointer<int (Empty::*)() & noexcept>();
#endif
}
