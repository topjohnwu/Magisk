//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <future>

// const error_category& future_category();

// virtual bool equivalent(const error_code& code, int condition) const;

#include <future>
#include <cassert>

int main()
{
    const std::error_category& e_cat = std::future_category();
    assert(e_cat.equivalent(std::error_code(5, e_cat), 5));
    assert(!e_cat.equivalent(std::error_code(5, e_cat), 6));
}
