//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class X> class auto_ptr;

// auto_ptr& operator=(auto_ptr& a) throw();

// REQUIRES: c++98 || c++03 || c++11 || c++14

#include <memory>
#include <cassert>

#include "../A.h"

void
test()
{
    {
    A* p1 = new A(1);
    std::auto_ptr<A> ap1(p1);
    A* p2 = new A(2);
    std::auto_ptr<A> ap2(p2);
    assert(A::count == 2);
    assert(ap1.get() == p1);
    assert(ap2.get() == p2);
    std::auto_ptr<A>& apr = ap2 = ap1;
    assert(&apr == &ap2);
    assert(A::count == 1);
    assert(ap1.get() == 0);
    assert(ap2.get() == p1);
    }
    assert(A::count == 0);
}

int main()
{
    test();
}
