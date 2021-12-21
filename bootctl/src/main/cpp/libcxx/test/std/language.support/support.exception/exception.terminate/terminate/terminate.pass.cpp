//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test terminate

#include <exception>
#include <cstdlib>
#include <cassert>

void f1()
{
    std::exit(0);
}

int main()
{
    std::set_terminate(f1);
    std::terminate();
    assert(false);
}
