//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++98 || c++03 || c++11 || c++14

// test unexpected

#include <exception>
#include <cstdlib>
#include <cassert>

void f1()
{
    std::exit(0);
}

int main()
{
    std::set_unexpected(f1);
    std::unexpected();
    assert(false);
}
