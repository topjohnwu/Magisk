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

// template <class Fn, class T>
//   binder2nd<Fn>
//   bind2nd(const Fn& op, const T& x);

#include <functional>
#include <cassert>

#include "../test_func.h"

int main()
{
    assert(std::bind2nd(test_func(1), 5)(10) == 5.);
}
