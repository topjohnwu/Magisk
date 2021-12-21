//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// const error_category& iostream_category();

#include <ios>
#include <cassert>
#include <string>

int main()
{
    const std::error_category& e_cat1 = std::iostream_category();
    std::string m1 = e_cat1.name();
    assert(m1 == "iostream");
}
