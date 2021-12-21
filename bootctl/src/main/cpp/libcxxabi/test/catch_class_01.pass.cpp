//===---------------------- catch_class_01.cpp ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <exception>
#include <stdlib.h>
#include <assert.h>

struct A
{
    static int count;
    int id_;
    explicit A(int id) : id_(id) {count++;}
    A(const A& a) : id_(a.id_) {count++;}
    ~A() {count--;}
};

int A::count = 0;

void f1()
{
    throw A(3);
}

void f2()
{
    try
    {
        assert(A::count == 0);
        f1();
    }
    catch (A a)
    {
        assert(A::count != 0);
        assert(a.id_ == 3);
        throw;
    }
}

int main()
{
    try
    {
        f2();
        assert(false);
    }
    catch (const A& a)
    {
        assert(A::count != 0);
        assert(a.id_ == 3);
    }
    assert(A::count == 0);
}
