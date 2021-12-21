//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test set_terminate

#include <exception>
#include <cstdlib>
#include <cassert>

void f1() {}
void f2() {}

int main()
{
    std::set_terminate(f1);
    assert(std::set_terminate(f2) == f1);
}
