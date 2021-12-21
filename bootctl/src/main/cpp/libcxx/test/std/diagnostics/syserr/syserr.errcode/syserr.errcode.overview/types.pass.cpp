//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>
// class error_code

// Make sure that the error_code bits of <system_error> are self-contained.

#include <system_error>
#include "test_macros.h"

int main()
{
    std::error_code x;
    TEST_IGNORE_NODISCARD  x.category();   // returns a std::error_category &
    TEST_IGNORE_NODISCARD  x.default_error_condition(); // std::error_condition
    TEST_IGNORE_NODISCARD  x.message();    // returns a std::string
}
