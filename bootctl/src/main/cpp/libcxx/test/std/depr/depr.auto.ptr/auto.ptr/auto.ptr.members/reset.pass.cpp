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

// void reset(X* p=0) throw();

#include <memory>
#include <cassert>

// REQUIRES: c++98 || c++03 || c++11 || c++14

#include "../A.h"

void
test()
{
    {
    A* p = new A(1);
    std::auto_ptr<A> ap(p);
    ap.reset();
    assert(ap.get() == 0);
    assert(A::count == 0);
    }
    assert(A::count == 0);
    {
    A* p = new A(1);
    std::auto_ptr<A> ap(p);
    ap.reset(p);
    assert(ap.get() == p);
    assert(A::count == 1);
    }
    assert(A::count == 0);
    {
    A* p = new A(1);
    std::auto_ptr<A> ap(p);
    A* p2 = new A(2);
    ap.reset(p2);
    assert(ap.get() == p2);
    assert(A::count == 1);
    }
    assert(A::count == 0);
}

int main()
{
    test();
}
