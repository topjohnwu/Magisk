//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// default_delete

#include <memory>
#include <cassert>

struct A
{
    static int count;
    A() {++count;}
    A(const A&) {++count;}
    virtual ~A() {--count;}
};

int A::count = 0;

struct B
    : public A
{
    static int count;
    B() {++count;}
    B(const B&) {++count;}
    virtual ~B() {--count;}
};

int B::count = 0;

int main()
{
    std::default_delete<B> d2;
    std::default_delete<A> d1 = d2;
    A* p = new B;
    assert(A::count == 1);
    assert(B::count == 1);
    d1(p);
    assert(A::count == 0);
    assert(B::count == 0);
}
