//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>
// XFAIL: c++98, c++03

// #include <initializer_list>

#include <utility>

int main()
{
    std::initializer_list<int> x;
    (void)x;
}

