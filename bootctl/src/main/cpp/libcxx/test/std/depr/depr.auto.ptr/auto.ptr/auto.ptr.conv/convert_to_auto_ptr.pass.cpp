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

// template<class Y> operator auto_ptr<Y>() throw();

// REQUIRES: c++98 || c++03 || c++11 || c++14

#include <memory>
#include <cassert>

#include "../AB.h"

std::auto_ptr<B>
source()
{
    return std::auto_ptr<B>(new B(1));
}

void
test()
{
    std::auto_ptr<A> ap2(source());
}

int main()
{
    test();
}
