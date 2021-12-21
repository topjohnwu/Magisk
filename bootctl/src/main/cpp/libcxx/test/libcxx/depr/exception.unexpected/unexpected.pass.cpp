//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test unexpected

// MODULES_DEFINES: _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS
#define _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS
#include <exception>
#include <cstdlib>
#include <cassert>

void fexit()
{
    std::exit(0);
}

int main()
{
    std::set_unexpected(fexit);
    std::unexpected();
    assert(false);
}
