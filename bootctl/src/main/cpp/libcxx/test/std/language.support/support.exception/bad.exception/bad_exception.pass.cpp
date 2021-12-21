//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bad_exception

#include <exception>
#include <type_traits>
#include <cassert>

int main()
{
    static_assert((std::is_base_of<std::exception, std::bad_exception>::value),
                 "std::is_base_of<std::exception, std::bad_exception>::value");
    static_assert(std::is_polymorphic<std::bad_exception>::value,
                 "std::is_polymorphic<std::bad_exception>::value");
    std::bad_exception b;
    std::bad_exception b2 = b;
    b2 = b;
    const char* w = b2.what();
    assert(w);
}
