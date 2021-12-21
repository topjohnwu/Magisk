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

// template<class Y> auto_ptr& operator=(auto_ptr<Y>& a) throw();

#include <memory>
#include <cassert>

#include "../AB.h"

void
test()
{
    {
    B* p1 = new B(1);
    const std::auto_ptr<B> ap1(p1);
    A* p2 = new A(2);
    std::auto_ptr<A> ap2(p2);
    assert(A::count == 2);
    assert(B::count == 1);
    assert(ap1.get() == p1);
    assert(ap2.get() == p2);
    std::auto_ptr<A>& apr = ap2 = ap1;
    assert(&apr == &ap2);
    assert(A::count == 1);
    assert(B::count == 1);
    assert(ap1.get() == 0);
    assert(ap2.get() == p1);
    }
    assert(A::count == 0);
    assert(B::count == 0);
}

int main()
{
    test();
}
