//===------------------------- catch_ptr_02.cpp ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <cassert>

// Clang emits  warnings about exceptions of type 'Child' being caught by
// an earlier handler of type 'Base'. Congrats clang, you've just
// diagnosed the behavior under test.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wexceptions"
#endif

#if __cplusplus < 201103L
#define DISABLE_NULLPTR_TESTS
#endif

struct  A {};
A a;
const A ca = A();

void test1 ()
{
    try
    {
        throw &a;
        assert(false);
    }
    catch ( const A* )
    {
    }
    catch ( A *)
    {
        assert (false);
    }
}

void test2 ()
{
    try
     {
        throw &a;
        assert(false);
    }
    catch ( A* )
    {
    }
    catch ( const A *)
    {
         assert (false);
    }
}

void test3 ()
{
    try
    {
        throw &ca;
        assert(false);
    }
    catch ( const A* )
    {
    }
    catch ( A *)
    {
        assert (false);
    }
}

void test4 ()
{
    try
    {
        throw &ca;
        assert(false);
    }
    catch ( A *)
    {
        assert (false);
    }
    catch ( const A* )
    {
    }
}

struct base1 {int x;};
struct base2 {int x;};
struct derived : base1, base2 {};

void test5 ()
{
    try
    {
        throw (derived*)0;
        assert(false);
    }
    catch (base2 *p) {
        assert (p == 0);
    }
    catch (...)
    {
        assert (false);
    }
}

void test6 ()
{
#if !defined(DISABLE_NULLPTR_TESTS)
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (base2 *p) {
        assert (p == nullptr);
    }
    catch (...)
    {
        assert (false);
    }
#endif
}

void test7 ()
{
    try
    {
        throw (derived*)12;
        assert(false);
    }
    catch (base2 *p) {
        assert ((unsigned long)p == 12+sizeof(base1));
    }
    catch (...)
    {
        assert (false);
    }
}


struct vBase {};
struct vDerived : virtual public vBase {};

void test8 ()
{
    vDerived derived;
    try
    {
        throw &derived;
        assert(false);
    }
    catch (vBase *p) {
        assert(p != 0);
    }
    catch (...)
    {
        assert (false);
    }
}

void test9 ()
{
#if !defined(DISABLE_NULLPTR_TESTS)
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (vBase *p) {
        assert(p == 0);
    }
    catch (...)
    {
        assert (false);
    }
#endif
}

void test10 ()
{
    try
    {
        throw (vDerived*)0;
        assert(false);
    }
    catch (vBase *p) {
        assert(p == 0);
    }
    catch (...)
    {
        assert (false);
    }
}

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
}
