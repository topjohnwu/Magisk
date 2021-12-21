//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// add_pointer
// If T names a referenceable type or a (possibly cv-qualified) void type then
//    the member typedef type shall name the same type as remove_reference_t<T>*;
//    otherwise, type shall name T.

#include <type_traits>
#include "test_macros.h"

template <class T, class U>
void test_add_pointer()
{
    static_assert((std::is_same<typename std::add_pointer<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::add_pointer_t<T>,     U>::value), "");
#endif
}

template <class F>
void test_function0()
{
    static_assert((std::is_same<typename std::add_pointer<F>::type, F*>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::add_pointer_t<F>, F*>::value), "");
#endif
}

template <class F>
void test_function1()
{
    static_assert((std::is_same<typename std::add_pointer<F>::type, F>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::add_pointer_t<F>, F>::value), "");
#endif
}

struct Foo {};

int main()
{
    test_add_pointer<void, void*>();
    test_add_pointer<int, int*>();
    test_add_pointer<int[3], int(*)[3]>();
    test_add_pointer<int&, int*>();
    test_add_pointer<const int&, const int*>();
    test_add_pointer<int*, int**>();
    test_add_pointer<const int*, const int**>();
    test_add_pointer<Foo, Foo*>();

//  LWG 2101 specifically talks about add_pointer and functions.
//  The term of art is "a referenceable type", which a cv- or ref-qualified function is not.
    test_function0<void()>();
#if TEST_STD_VER >= 11
    test_function1<void() const>();
    test_function1<void() &>();
    test_function1<void() &&>();
    test_function1<void() const &>();
    test_function1<void() const &&>();
#endif

//  But a cv- or ref-qualified member function *is* "a referenceable type"
    test_function0<void (Foo::*)()>();
#if TEST_STD_VER >= 11
    test_function0<void (Foo::*)() const>();
    test_function0<void (Foo::*)() &>();
    test_function0<void (Foo::*)() &&>();
    test_function0<void (Foo::*)() const &>();
    test_function0<void (Foo::*)() const &&>();
#endif
}
