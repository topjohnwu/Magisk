//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bad_alloc

#include <new>
#include <type_traits>
#include <cassert>

int main()
{
    static_assert((std::is_base_of<std::exception, std::bad_alloc>::value),
                 "std::is_base_of<std::exception, std::bad_alloc>::value");
    static_assert(std::is_polymorphic<std::bad_alloc>::value,
                 "std::is_polymorphic<std::bad_alloc>::value");
    std::bad_alloc b;
    std::bad_alloc b2 = b;
    b2 = b;
    const char* w = b2.what();
    assert(w);
}
