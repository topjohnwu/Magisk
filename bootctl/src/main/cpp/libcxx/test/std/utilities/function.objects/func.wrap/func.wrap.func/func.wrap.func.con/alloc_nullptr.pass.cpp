//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
// REQUIRES: c++98 || c++03 || c++11 || c++14

// class function<R(ArgTypes...)>

// template<class A> function(allocator_arg_t, const A&, nullptr_t);
//
// This signature was removed in C++17

#include <functional>
#include <cassert>

#include "min_allocator.h"

int main()
{
    std::function<int(int)> f(std::allocator_arg, bare_allocator<int>(), nullptr);
    assert(!f);
}
