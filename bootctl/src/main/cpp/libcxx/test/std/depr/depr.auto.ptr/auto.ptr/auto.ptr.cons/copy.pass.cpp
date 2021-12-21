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

// auto_ptr(auto_ptr& a) throw();

// REQUIRES: c++98 || c++03 || c++11 || c++14

#include <memory>
#include <cassert>

#include "../A.h"

void
test()
{
    {
    A* p = new A(1);
    std::auto_ptr<A> ap1(p);
    std::auto_ptr<A> ap2(ap1);
    assert(ap1.get() == 0);
    assert(ap2.get() == p);
    assert(A::count == 1);
    }
    assert(A::count == 0);
}

int main()
{
    test();
}
