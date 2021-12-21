//===--------------- catch_member_function_pointer_01.cpp -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// GCC incorrectly allows PMF type "void (T::*)()" to be caught as "void (T::*)() const"
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69375
// XFAIL: gcc
// UNSUPPORTED: libcxxabi-no-exceptions
#include <cassert>

struct A
{
    void foo() {}
    void bar() const {}
};

typedef void (A::*mf1)();
typedef void (A::*mf2)() const;

struct B : public A
{
};

typedef void (B::*dmf1)();
typedef void (B::*dmf2)() const;

template <class Tp>
bool can_convert(Tp) { return true; }

template <class>
bool can_convert(...) { return false; }


void test1()
{
    try
    {
        throw &A::foo;
        assert(false);
    }
    catch (mf2)
    {
        assert(false);
    }
    catch (mf1)
    {
    }
}

void test2()
{
    try
    {
        throw &A::bar;
        assert(false);
    }
    catch (mf1)
    {
        assert(false);
    }
    catch (mf2)
    {
    }
}



void test_derived()
{
    try
    {
        throw (mf1)0;
        assert(false);
    }
    catch (dmf2)
    {
       assert(false);
    }
    catch (dmf1)
    {
       assert(false);
    }
    catch (mf1)
    {
    }

    try
    {
        throw (mf2)0;
        assert(false);
    }
    catch (dmf1)
    {
       assert(false);
    }
    catch (dmf2)
    {
       assert(false);
    }
    catch (mf2)
    {
    }

    assert(!can_convert<mf1>((dmf1)0));
    assert(!can_convert<mf2>((dmf1)0));
    try
    {
        throw (dmf1)0;
        assert(false);
    }
    catch (mf2)
    {
       assert(false);
    }
    catch (mf1)
    {
       assert(false);
    }
    catch (...)
    {
    }

    assert(!can_convert<mf1>((dmf2)0));
    assert(!can_convert<mf2>((dmf2)0));
    try
    {
        throw (dmf2)0;
        assert(false);
    }
    catch (mf2)
    {
       assert(false);
    }
    catch (mf1)
    {
        assert(false);
    }
    catch (...)
    {
    }
}

void test_void()
{
    assert(!can_convert<void*>(&A::foo));
    try
    {
        throw &A::foo;
        assert(false);
    }
    catch (void*)
    {
        assert(false);
    }
    catch(...)
    {
    }
}

int main()
{
    test1();
    test2();
    test_derived();
    test_void();
}
