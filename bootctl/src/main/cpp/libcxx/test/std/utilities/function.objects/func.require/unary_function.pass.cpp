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
// unary_function was removed in C++17

// unary_function

#include <functional>
#include <type_traits>

int main()
{
    typedef std::unary_function<int, bool> uf;
    static_assert((std::is_same<uf::argument_type, int>::value), "");
    static_assert((std::is_same<uf::result_type, bool>::value), "");
}
