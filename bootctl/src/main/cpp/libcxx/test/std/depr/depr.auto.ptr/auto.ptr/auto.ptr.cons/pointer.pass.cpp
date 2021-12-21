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

// explicit auto_ptr(X* p =0) throw();

// REQUIRES: c++98 || c++03 || c++11 || c++14

#include <memory>
#include <cassert>

#include "../A.h"

void
test()
{
    {
    A* p = new A(1);
    std::auto_ptr<A> ap(p);
    assert(ap.get() == p);
    assert(A::count == 1);
    }
    assert(A::count == 0);
    {
    std::auto_ptr<A> ap;
    assert(ap.get() == 0);
    }
}

int main()
{
    test();
}
