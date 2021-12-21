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

// pointer_to_unary_function

#include <functional>
#include <type_traits>
#include <cassert>

double unary_f(int i) {return 0.5 - i;}

int main()
{
    typedef std::pointer_to_unary_function<int, double> F;
    static_assert((std::is_base_of<std::unary_function<int, double>, F>::value), "");
    const F f(unary_f);
    assert(f(36) == -35.5);
}
