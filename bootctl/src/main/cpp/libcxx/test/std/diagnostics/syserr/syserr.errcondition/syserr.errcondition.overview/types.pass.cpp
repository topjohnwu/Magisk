//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>
// class error_condition

// Make sure that the error_condition bits of <system_error> are self-contained.

#include <system_error>
#include "test_macros.h"

int main()
{
    std::error_condition x = std::errc(0);
    TEST_IGNORE_NODISCARD  x.category();   // returns a std::error_condition &
    TEST_IGNORE_NODISCARD  x.message();    // returns a std::string
}
