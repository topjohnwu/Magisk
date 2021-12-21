//===---------------------- catch_class_04.cpp ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

/*
    This test checks that adjustedPtr is correct as there exist offsets in this
    object for the various subobjects, all of which have a unique id_ to
    check against.  It also checks that virtual bases work properly
*/

// UNSUPPORTED: libcxxabi-no-exceptions

#include <exception>
#include <stdlib.h>
#include <assert.h>

// Clang emits  warnings about exceptions of type 'Child' being caught by
// an earlier handler of type 'Base'. Congrats clang, you've just
// diagnosed the behavior under test.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wexceptions"
#endif

struct B
{
    static int count;
    int id_;
    explicit B(int id) : id_(id) {count++;}
    B(const B& a) : id_(a.id_) {count++;}
    ~B() {count--;}
};

int B::count = 0;

struct C1
    : virtual B
{
    static int count;
    int id_;
    explicit C1(int id) : B(id-2), id_(id) {count++;}
    C1(const C1& a) : B(a.id_-2), id_(a.id_) {count++;}
    ~C1() {count--;}
};

int C1::count = 0;

struct C2
    : virtual private B
{
    static int count;
    int id_;
    explicit C2(int id) : B(id-2), id_(id) {count++;}
    C2(const C2& a) : B(a.id_-2), id_(a.id_) {count++;}
    ~C2() {count--;}
};

int C2::count = 0;

struct A
    : C1, C2
{
    static int count;
    int id_;
    explicit A(int id) : B(id+3), C1(id-1), C2(id-2), id_(id) {count++;}
    A(const A& a) : B(a.id_+3), C1(a.id_-1), C2(a.id_-2),  id_(a.id_) {count++;}
    ~A() {count--;}
};

int A::count = 0;

A a(5);

void f1()
{
    throw &a;
    assert(false);
}

void f2()
{
    try
    {
        f1();
        assert(false);
    }
    catch (const A* a)  // can catch A
    {
        assert(a->id_ == 5);
        assert(static_cast<const C1*>(a)->id_ == 4);
        assert(static_cast<const C2*>(a)->id_ == 3);
        assert(static_cast<const B*>(a)->id_ == 8);
        throw;
    }
    catch (const C1*)
    {
        assert(false);
    }
    catch (const C2*)
    {
        assert(false);
    }
    catch (const B*)
    {
        assert(false);
    }
}

void f3()
{
    try
    {
        f2();
        assert(false);
    }
    catch (const B* a)  // can catch B
    {
        assert(static_cast<const B*>(a)->id_ == 8);
        throw;
    }
    catch (const C1* c1)
    {
        assert(false);
    }
    catch (const C2*)
    {
        assert(false);
    }
}

void f4()
{
    try
    {
        f3();
        assert(false);
    }
    catch (const C2* c2)  // can catch C2
    {
        assert(c2->id_ == 3);
        throw;
    }
    catch (const B* a)
    {
        assert(false);
    }
    catch (const C1*)
    {
        assert(false);
    }
}

void f5()
{
    try
    {
        f4();
        assert(false);
    }
    catch (const C1* c1)  // can catch C1
    {
        assert(c1->id_ == 4);
        assert(static_cast<const B*>(c1)->id_ == 8);
        throw;
    }
    catch (const B* a)
    {
        assert(false);
    }
    catch (const C2*)
    {
        assert(false);
    }
}

int main()
{
    try
    {
        f5();
        assert(false);
    }
    catch (...)
    {
    }
}
