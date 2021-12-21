//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <future>

// template <> struct is_error_code_enum<future_errc> : public true_type {};

#include <future>
#include "test_macros.h"

int main()
{
    static_assert(std::is_error_code_enum  <std::future_errc>::value, "");
#if TEST_STD_VER > 14
    static_assert(std::is_error_code_enum_v<std::future_errc>, "");
#endif
}
