//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test new_handler

#include <new>
#include <type_traits>
#include <cassert>

void f() {}

int main()
{
    static_assert((std::is_same<std::new_handler, void(*)()>::value), "");
    std::new_handler p = f;
    assert(p == &f);
}
