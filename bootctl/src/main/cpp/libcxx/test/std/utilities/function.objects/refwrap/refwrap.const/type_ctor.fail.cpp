//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// reference_wrapper

// reference_wrapper(T&&) = delete;

// XFAIL: c++98, c++03

#include <functional>
#include <cassert>

int main()
{
    std::reference_wrapper<const int> r(3);
}
