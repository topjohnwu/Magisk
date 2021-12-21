//===---------------------- catch_class_02.cpp ----------------------------===//
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

struct B
{
    static int count;
    int id_;
    explicit B(int id) : id_(id) {count++;}
    B(const B& a) : id_(a.id_) {count++;}
    ~B() {count--;}
};

int B::count = 0;

struct A
    : B
{
    static int count;
    int id_;
    explicit A(int id) : B(id-1), id_(id) {count++;}
    A(const A& a) : B(a.id_-1), id_(a.id_) {count++;}
    ~A() {count--;}
};

int A::count = 0;

void f1()
{
    assert(A::count == 0);
    assert(B::count == 0);
    A a(3);
    assert(A::count == 1);
    assert(B::count == 1);
    throw a;
    assert(false);
}

void f2()
{
    try
    {
        assert(A::count == 0);
        f1();
    assert(false);
    }
    catch (A a)
    {
        assert(A::count != 0);
        assert(B::count != 0);
        assert(a.id_ == 3);
        throw;
    }
    catch (B b)
    {
        assert(false);
    }
}

int main()
{
    try
    {
        f2();
        assert(false);
    }
    catch (const B& b)
    {
        assert(B::count != 0);
        assert(b.id_ == 2);
    }
    assert(A::count == 0);
    assert(B::count == 0);
}
