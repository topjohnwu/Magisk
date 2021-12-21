//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// class error_category

// virtual string message(int ev) const = 0;

#include <system_error>
#include <cassert>
#include <string>

#include <stdio.h>

int main()
{
    const std::error_category& e_cat1 = std::generic_category();
    const std::error_category& e_cat2 = std::system_category();
    std::string m1 = e_cat1.message(5);
    std::string m2 = e_cat2.message(5);
    std::string m3 = e_cat2.message(6);
    assert(!m1.empty());
    assert(!m2.empty());
    assert(!m3.empty());
    assert(m1 == m2);
    assert(m1 != m3);
}
