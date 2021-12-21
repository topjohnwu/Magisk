//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test terminate_handler

#include <exception>
#include <type_traits>
#include <cassert>

void f() {}

int main()
{
    static_assert((std::is_same<std::terminate_handler, void(*)()>::value), "");
    std::terminate_handler p = f;
    assert(p == &f);
}
