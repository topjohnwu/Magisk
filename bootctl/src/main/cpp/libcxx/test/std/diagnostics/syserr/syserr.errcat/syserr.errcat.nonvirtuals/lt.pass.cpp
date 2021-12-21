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

// bool operator<(const error_category& rhs) const;

#include <system_error>
#include <cassert>

int main()
{
    const std::error_category& e_cat1 = std::generic_category();
    const std::error_category& e_cat2 = std::generic_category();
    const std::error_category& e_cat3 = std::system_category();
    assert(!(e_cat1 < e_cat2) && !(e_cat2 < e_cat1));
    assert((e_cat1 < e_cat3) || (e_cat3 < e_cat1));
}
