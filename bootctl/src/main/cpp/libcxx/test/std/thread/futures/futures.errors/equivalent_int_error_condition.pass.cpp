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

// virtual bool equivalent(int code, const error_condition& condition) const;

#include <future>
#include <cassert>

int main()
{
    const std::error_category& e_cat = std::future_category();
    std::error_condition e_cond = e_cat.default_error_condition(5);
    assert(e_cat.equivalent(5, e_cond));
    assert(!e_cat.equivalent(6, e_cond));
}
