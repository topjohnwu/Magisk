//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test get_new_handler

#include <new>
#include <cassert>

void f1() {}
void f2() {}

int main()
{
    assert(std::get_new_handler() == 0);
    std::set_new_handler(f1);
    assert(std::get_new_handler() == f1);
    std::set_new_handler(f2);
    assert(std::get_new_handler() == f2);
}
