//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// XFAIL: suse-linux-enterprise-server-11
// XFAIL: with_system_cxx_lib=macosx10.12
// XFAIL: with_system_cxx_lib=macosx10.11
// XFAIL: with_system_cxx_lib=macosx10.10
// XFAIL: with_system_cxx_lib=macosx10.9
// XFAIL: with_system_cxx_lib=macosx10.7
// XFAIL: with_system_cxx_lib=macosx10.8

// <system_error>

// class error_category

// const error_category& generic_category();

#include <system_error>
#include <cassert>
#include <string>
#include <cerrno>

#include "test_macros.h"

void test_message_for_bad_value() {
    errno = E2BIG; // something that message will never generate
    const std::error_category& e_cat1 = std::generic_category();
    const std::string msg = e_cat1.message(-1);
    LIBCPP_ASSERT(msg == "Unknown error -1" || msg == "Unknown error");
    assert(errno == E2BIG);
}

int main()
{
    const std::error_category& e_cat1 = std::generic_category();
    std::string m1 = e_cat1.name();
    assert(m1 == "generic");
    {
        test_message_for_bad_value();
    }
}
