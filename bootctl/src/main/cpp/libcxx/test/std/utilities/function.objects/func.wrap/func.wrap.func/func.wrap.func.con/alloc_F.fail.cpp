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

// template<class F, class A> function(allocator_arg_t, const A&, F);
//
// This signature was removed in C++17

#include <functional>
#include <cassert>

#include "test_macros.h"

void foo(int) {}

int main()
{
    std::function<void(int)> f(std::allocator_arg, std::allocator<int>(), foo);
}
