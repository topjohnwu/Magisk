//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test out_of_range

#include <stdexcept>
#include <type_traits>
#include <cstring>
#include <string>
#include <cassert>

int main()
{
    static_assert((std::is_base_of<std::logic_error, std::out_of_range>::value),
                 "std::is_base_of<std::logic_error, std::out_of_range>::value");
    static_assert(std::is_polymorphic<std::out_of_range>::value,
                 "std::is_polymorphic<std::out_of_range>::value");
    {
    const char* msg = "out_of_range message";
    std::out_of_range e(msg);
    assert(std::strcmp(e.what(), msg) == 0);
    std::out_of_range e2(e);
    assert(std::strcmp(e2.what(), msg) == 0);
    e2 = e;
    assert(std::strcmp(e2.what(), msg) == 0);
    }
    {
    std::string msg("another out_of_range message");
    std::out_of_range e(msg);
    assert(e.what() == msg);
    std::out_of_range e2(e);
    assert(e2.what() == msg);
    e2 = e;
    assert(e2.what() == msg);
    }
}
