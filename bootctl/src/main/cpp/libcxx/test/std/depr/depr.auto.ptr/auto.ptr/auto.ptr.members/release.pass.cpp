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

// X* release() throw();

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
    A* p2 = ap.release();
    assert(p2 == p);
    assert(ap.get() == 0);
    delete p;
    }
    assert(A::count == 0);
}

int main()
{
    test();
}
