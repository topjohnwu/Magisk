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

// X& operator*() const throw();

// REQUIRES: c++98 || c++03 || c++11 || c++14

#include <memory>
#include <cassert>

#include "../A.h"

void
test()
{
    {
    A* p = new A(1);
    const std::auto_ptr<A> ap(p);
    assert((*ap).id() == 1);
    *ap = A(3);
    assert((*ap).id() == 3);
    }
    assert(A::count == 0);
}

int main()
{
    test();
}
