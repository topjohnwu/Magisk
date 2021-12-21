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

// virtual error_condition default_error_condition(int ev) const;

#include <system_error>
#include <cassert>

int main()
{
    const std::error_category& e_cat = std::generic_category();
    std::error_condition e_cond = e_cat.default_error_condition(static_cast<int>(std::errc::not_a_directory));
    assert(e_cond.category() == e_cat);
    assert(e_cond.value() == static_cast<int>(std::errc::not_a_directory));
}
