//===----------------- catch_member_pointer_nullptr.cpp -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <cassert>

#if __has_feature(cxx_nullptr)

struct A
{
    const int i;
    int j;
};

typedef const int A::*md1;
typedef       int A::*md2;

void test1()
{
    try
    {
        throw nullptr;
        assert(false);
    }
    catch (md2 p)
    {
        assert(!p);
    }
    catch (md1)
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
    catch (md1 p)
    {
        assert(!p);
    }
    catch (md2)
    {
        assert(false);
    }
}

#else

void test1()
{
}

void test2()
{
}

#endif

int main()
{
    test1();
    test2();
}
