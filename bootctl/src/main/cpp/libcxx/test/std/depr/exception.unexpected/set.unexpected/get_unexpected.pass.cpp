//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++98 || c++03 || c++11 || c++14

// test get_unexpected

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

    std::unexpected_handler old = std::get_unexpected();
    // verify there is a previous unexpected handler
    assert(old);
    std::set_unexpected(f1);
    assert(std::get_unexpected() == f1);
    // verify f1 was replace with f2
    std::set_unexpected(f2);
    assert(std::get_unexpected() == f2);
    // verify calling original unexpected handler calls terminate
    std::set_terminate(f3);
    (*old)();
    assert(0);
}
