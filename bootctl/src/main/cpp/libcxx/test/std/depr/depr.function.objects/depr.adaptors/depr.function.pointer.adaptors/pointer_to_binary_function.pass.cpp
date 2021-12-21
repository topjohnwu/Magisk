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

// pointer_to_binary_function

#include <functional>
#include <type_traits>
#include <cassert>

double binary_f(int i, short j) {return i - j + .75;}

int main()
{
    typedef std::pointer_to_binary_function<int, short, double> F;
    static_assert((std::is_base_of<std::binary_function<int, short, double>, F>::value), "");
    const F f(binary_f);
    assert(f(36, 27) == 9.75);
}
