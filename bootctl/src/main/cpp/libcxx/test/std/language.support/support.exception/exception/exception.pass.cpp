//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test exception

#include <exception>
#include <type_traits>
#include <cassert>

int main()
{
    static_assert(std::is_polymorphic<std::exception>::value,
                 "std::is_polymorphic<std::exception>::value");
    std::exception b;
    std::exception b2 = b;
    b2 = b;
    const char* w = b2.what();
    assert(w);
}
