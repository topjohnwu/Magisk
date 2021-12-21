//===----------------- catch_member_data_pointer_01.cpp -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <cassert>

struct A
{
    A() : i(0), j(0) {} // explicitly initialize 'i' to prevent warnings
    const int i;
    int j;
};

typedef const int A::*md1;
typedef       int A::*md2;

struct B : public A
{
    B() : k(0), l(0) {} // explicitly initialize 'k' to prevent warnings.
    const int k;
    int l;
};

typedef const int B::*der1;
typedef       int B::*der2;

void test1()
{
    try
    {
        throw &A::i;
        assert(false);
    }
    catch (md2)
    {
        assert(false);
    }
    catch (md1)
    {
    }
}

// Check that cv qualified conversions are allowed.
void test2()
{
    try
    {
        throw &A::j;
    }
    catch (md2)
    {
    }
    catch (...)
    {
        assert(false);
    }

    try
    {
        throw &A::j;
        assert(false);
    }
    catch (md1)
    {
    }
    catch (...)
    {
        assert(false);
    }
}

// Check that Base -> Derived conversions are NOT allowed.
void test3()
{
    try
    {
        throw &A::i;
        assert(false);
    }
    catch (md2)
    {
        assert(false);
    }
    catch (der2)
    {
        assert(false);
    }
    catch (der1)
    {
        assert(false);
    }
    catch (md1)
    {
    }
}

// Check that Base -> Derived conversions NOT are allowed with different cv
// qualifiers.
void test4()
{
    try
    {
        throw &A::j;
        assert(false);
    }
    catch (der2)
    {
        assert(false);
    }
    catch (der1)
    {
        assert(false);
    }
    catch (md2)
    {
    }
    catch (...)
    {
        assert(false);
    }
}

// Check that no Derived -> Base conversions are allowed.
void test5()
{
    try
    {
        throw &B::k;
        assert(false);
    }
    catch (md1)
    {
        assert(false);
    }
    catch (md2)
    {
        assert(false);
    }
    catch (der1)
    {
    }

    try
    {
        throw &B::l;
        assert(false);
    }
    catch (md1)
    {
        assert(false);
    }
    catch (md2)
    {
        assert(false);
    }
    catch (der2)
    {
    }
}

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
}
