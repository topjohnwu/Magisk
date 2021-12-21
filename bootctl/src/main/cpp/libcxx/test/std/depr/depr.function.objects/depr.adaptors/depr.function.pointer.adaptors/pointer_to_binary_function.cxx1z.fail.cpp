//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
// pointer_to_binary_function
// UNSUPPORTED: c++98, c++03, c++11, c++14

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

double binary_f(int i, short j) {return i - j + .75;}

int main()
{
    typedef std::pointer_to_binary_function<int, short, double> F;
}
