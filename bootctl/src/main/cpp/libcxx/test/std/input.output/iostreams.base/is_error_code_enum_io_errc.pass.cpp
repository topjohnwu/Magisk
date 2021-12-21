//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++03

// <ios>

// template <> struct is_error_code_enum<io_errc> : public true_type {};

#include <ios>
#include "test_macros.h"

int main()
{
    static_assert(std::is_error_code_enum  <std::io_errc>::value, "");
#if TEST_STD_VER > 14
    static_assert(std::is_error_code_enum_v<std::io_errc>, "");
#endif
}
