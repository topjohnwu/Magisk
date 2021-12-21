//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bad_cast

#include <typeinfo>
#include <type_traits>
#include <cassert>

int main()
{
    static_assert((std::is_base_of<std::exception, std::bad_cast>::value),
                 "std::is_base_of<std::exception, std::bad_cast>::value");
    static_assert(std::is_polymorphic<std::bad_cast>::value,
                 "std::is_polymorphic<std::bad_cast>::value");
    std::bad_cast b;
    std::bad_cast b2 = b;
    b2 = b;
    const char* w = b2.what();
    assert(w);
}
