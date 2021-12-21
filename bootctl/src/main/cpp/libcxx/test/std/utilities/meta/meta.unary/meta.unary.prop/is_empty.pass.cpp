//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_empty

// T is a non-union class type with:
//  no non-static data members,
//  no unnamed bit-fields of non-zero length,
//  no virtual member functions,
//  no virtual base classes,
//  and no base class B for which is_empty_v<B> is false.


#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_empty()
{
    static_assert( std::is_empty<T>::value, "");
    static_assert( std::is_empty<const T>::value, "");
    static_assert( std::is_empty<volatile T>::value, "");
    static_assert( std::is_empty<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_empty_v<T>, "");
    static_assert( std::is_empty_v<const T>, "");
    static_assert( std::is_empty_v<volatile T>, "");
    static_assert( std::is_empty_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_empty()
{
    static_assert(!std::is_empty<T>::value, "");
    static_assert(!std::is_empty<const T>::value, "");
    static_assert(!std::is_empty<volatile T>::value, "");
    static_assert(!std::is_empty<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_empty_v<T>, "");
    static_assert(!std::is_empty_v<const T>, "");
    static_assert(!std::is_empty_v<volatile T>, "");
    static_assert(!std::is_empty_v<const volatile T>, "");
#endif
}

class Empty {};
struct NotEmpty { int foo; };

class VirtualFn
{
    virtual ~VirtualFn();
};

union Union {};

struct EmptyBase    : public Empty {};
struct VirtualBase  : virtual Empty {};
struct NotEmptyBase : public NotEmpty {};

struct StaticMember    { static int foo; };
struct NonStaticMember {        int foo; };

struct bit_zero
{
    int :  0;
};

struct bit_one
{
    int :  1;
};

int main()
{
    test_is_not_empty<void>();
    test_is_not_empty<int&>();
    test_is_not_empty<int>();
    test_is_not_empty<double>();
    test_is_not_empty<int*>();
    test_is_not_empty<const int*>();
    test_is_not_empty<char[3]>();
    test_is_not_empty<char[]>();
    test_is_not_empty<Union>();
    test_is_not_empty<NotEmpty>();
    test_is_not_empty<VirtualFn>();
    test_is_not_empty<VirtualBase>();
    test_is_not_empty<NotEmptyBase>();
    test_is_not_empty<NonStaticMember>();
//    test_is_not_empty<bit_one>();

    test_is_empty<Empty>();
    test_is_empty<EmptyBase>();
    test_is_empty<StaticMember>();
    test_is_empty<bit_zero>();
}
