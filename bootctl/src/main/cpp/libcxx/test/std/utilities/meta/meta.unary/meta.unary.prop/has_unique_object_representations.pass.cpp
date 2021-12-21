//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: clang-3, clang-4, clang-5, apple-clang, gcc-4, gcc-5, gcc-6

// type_traits

// has_unique_object_representations

#include <type_traits>

#include "test_macros.h"

template <class T>
void test_has_unique_object_representations()
{
    static_assert( std::has_unique_object_representations<T>::value, "");
    static_assert( std::has_unique_object_representations<const T>::value, "");
    static_assert( std::has_unique_object_representations<volatile T>::value, "");
    static_assert( std::has_unique_object_representations<const volatile T>::value, "");

    static_assert( std::has_unique_object_representations_v<T>, "");
    static_assert( std::has_unique_object_representations_v<const T>, "");
    static_assert( std::has_unique_object_representations_v<volatile T>, "");
    static_assert( std::has_unique_object_representations_v<const volatile T>, "");
}

template <class T>
void test_has_not_has_unique_object_representations()
{
    static_assert(!std::has_unique_object_representations<T>::value, "");
    static_assert(!std::has_unique_object_representations<const T>::value, "");
    static_assert(!std::has_unique_object_representations<volatile T>::value, "");
    static_assert(!std::has_unique_object_representations<const volatile T>::value, "");

    static_assert(!std::has_unique_object_representations_v<T>, "");
    static_assert(!std::has_unique_object_representations_v<const T>, "");
    static_assert(!std::has_unique_object_representations_v<volatile T>, "");
    static_assert(!std::has_unique_object_representations_v<const volatile T>, "");
}

class Empty
{
};

class NotEmpty
{
    virtual ~NotEmpty();
};

union EmptyUnion {};
struct NonEmptyUnion {int x; unsigned y;};

struct bit_zero
{
    int :  0;
};

class Abstract
{
    virtual ~Abstract() = 0;
};

struct A
{
    ~A();
    unsigned foo;
};

struct B
{
   char bar;
   int foo;
};


int main()
{
    test_has_not_has_unique_object_representations<void>();
    test_has_not_has_unique_object_representations<Empty>();
    test_has_not_has_unique_object_representations<EmptyUnion>();
    test_has_not_has_unique_object_representations<NotEmpty>();
    test_has_not_has_unique_object_representations<bit_zero>();
    test_has_not_has_unique_object_representations<Abstract>();
    test_has_not_has_unique_object_representations<B>();

//  I would expect all three of these to have unique representations.
//  I would also expect that there are systems where they do not.
//     test_has_not_has_unique_object_representations<int&>();
//     test_has_not_has_unique_object_representations<int *>();
//     test_has_not_has_unique_object_representations<double>();


    test_has_unique_object_representations<unsigned>();
    test_has_unique_object_representations<NonEmptyUnion>();
    test_has_unique_object_representations<char[3]>();
    test_has_unique_object_representations<char[]>();

}
