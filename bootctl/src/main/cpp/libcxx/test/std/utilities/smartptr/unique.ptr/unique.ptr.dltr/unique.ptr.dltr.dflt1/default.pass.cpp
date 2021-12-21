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

// Test that default_delete<T[]> has a working default constructor

#include <memory>
#include <cassert>

struct A
{
    static int count;
    A() {++count;}
    A(const A&) {++count;}
    ~A() {--count;}
};

int A::count = 0;

int main()
{
    std::default_delete<A[]> d;
    A* p = new A[3];
    assert(A::count == 3);
    d(p);
    assert(A::count == 0);
}
