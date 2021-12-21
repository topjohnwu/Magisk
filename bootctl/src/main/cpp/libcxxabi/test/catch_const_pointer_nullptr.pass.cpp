//===--------------------- catch_const_pointer_nullptr.cpp ----------------===//
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

#if __has_feature(cxx_nullptr)

struct A {};

void test1()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (A* p)
    {
        assert(!p);
    }
    catch (const A*)
    {
        assert(false);
    }
}


void test2()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (const A* p)
    {
        assert(!p);
    }
    catch (A*)
    {
        assert(false);
    }
}

void test3()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (const A* const p)
    {
        assert(!p);
    }
    catch (A*)
    {
        assert(false);
    }
}

void test4()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (A* p)
    {
        assert(!p);
    }
    catch (const A* const)
    {
        assert(false);
    }
}

void test5()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (A const* p)
    {
        assert(!p);
    }
    catch (A*)
    {
        assert(false);
    }
}

void test6()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (A* p)
    {
        assert(!p);
    }
    catch (A const*)
    {
        assert(false);
    }
}


#else

void test1() {}
void test2() {}
void test3() {}
void test4() {}
void test5() {}
void test6() {}

#endif

int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
}
