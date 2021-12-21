//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test underflow_error

#include <stdexcept>
#include <type_traits>
#include <cstring>
#include <string>
#include <cassert>

int main()
{
    static_assert((std::is_base_of<std::runtime_error, std::underflow_error>::value),
                 "std::is_base_of<std::runtime_error, std::underflow_error>::value");
    static_assert(std::is_polymorphic<std::underflow_error>::value,
                 "std::is_polymorphic<std::underflow_error>::value");
    {
    const char* msg = "underflow_error message";
    std::underflow_error e(msg);
    assert(std::strcmp(e.what(), msg) == 0);
    std::underflow_error e2(e);
    assert(std::strcmp(e2.what(), msg) == 0);
    e2 = e;
    assert(std::strcmp(e2.what(), msg) == 0);
    }
    {
    std::string msg("another underflow_error message");
    std::underflow_error e(msg);
    assert(e.what() == msg);
    std::underflow_error e2(e);
    assert(e2.what() == msg);
    e2 = e;
    assert(e2.what() == msg);
    }
}
