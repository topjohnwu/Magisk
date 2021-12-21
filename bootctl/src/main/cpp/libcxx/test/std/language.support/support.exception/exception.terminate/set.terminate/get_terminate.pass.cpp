//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test get_terminate

#include <exception>
#include <cstdlib>
#include <cassert>

void f1() {}
void f2() {}

int main()
{
    std::set_terminate(f1);
    assert(std::get_terminate() == f1);
    std::set_terminate(f2);
    assert(std::get_terminate() == f2);
}
