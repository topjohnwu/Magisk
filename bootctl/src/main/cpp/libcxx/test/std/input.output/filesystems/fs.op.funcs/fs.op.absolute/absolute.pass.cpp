//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// path absolute(const path& p, const path& base=current_path());

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_absolute_path_test_suite)

TEST_CASE(absolute_signature_test)
{
    const path p; ((void)p);
    std::error_code ec;
    ASSERT_NOT_NOEXCEPT(absolute(p));
    ASSERT_NOT_NOEXCEPT(absolute(p, ec));
}


TEST_CASE(basic_test)
{
    const fs::path cwd = fs::current_path();
    const struct {
      std::string input;
      std::string expect;
    } TestCases [] = {
        {"", cwd / ""},
        {"foo", cwd / "foo"},
        {"foo/", cwd / "foo/"},
        {"/already_absolute", "/already_absolute"}
    };
    for (auto& TC : TestCases) {
        std::error_code ec = GetTestEC();
        const path ret = absolute(TC.input, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(ret.is_absolute());
        TEST_CHECK(PathEq(ret, TC.expect));
    }
}

TEST_SUITE_END()
