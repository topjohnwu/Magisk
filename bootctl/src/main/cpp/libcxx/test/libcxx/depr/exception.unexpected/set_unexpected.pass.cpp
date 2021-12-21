//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test set_unexpected

// MODULES_DEFINES: _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS
#define _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS
#include <exception>
#include <cassert>
#include <cstdlib>

void f1() {}
void f2() {}

void f3()
{
    std::exit(0);
}

int main()
{
    std::unexpected_handler old = std::set_unexpected(f1);
    // verify there is a previous unexpected handler
    assert(old);
    // verify f1 was replace with f2
    assert(std::set_unexpected(f2) == f1);
    // verify calling original unexpected handler calls terminate
    std::set_terminate(f3);
    (*old)();
    assert(0);
}
