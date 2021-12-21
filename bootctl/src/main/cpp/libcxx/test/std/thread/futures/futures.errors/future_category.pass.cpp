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

#include <future>
#include <cstring>
#include <cassert>

int main()
{
    const std::error_category& ec = std::future_category();
    assert(std::strcmp(ec.name(), "future") == 0);
}
