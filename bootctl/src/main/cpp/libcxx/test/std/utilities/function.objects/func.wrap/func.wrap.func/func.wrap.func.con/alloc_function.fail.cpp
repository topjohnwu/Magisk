//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
// XFAIL: c++98, c++03, c++11, c++14

// class function<R(ArgTypes...)>

// template<class A> function(allocator_arg_t, const A&, const function&);
//
// This signature was removed in C++17


#include <functional>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::function<void(int)> F;
    F f1;
    F f2(std::allocator_arg, std::allocator<int>(), f1);
}
