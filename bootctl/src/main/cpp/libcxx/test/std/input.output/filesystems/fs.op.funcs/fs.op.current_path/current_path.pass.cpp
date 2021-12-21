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

// path current_path();
// path current_path(error_code& ec);
// void current_path(path const&);
// void current_path(path const&, std::error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_current_path_path_test_suite)

TEST_CASE(current_path_signature_test)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(current_path());
    ASSERT_NOT_NOEXCEPT(current_path(ec));
    ASSERT_NOT_NOEXCEPT(current_path(p));
    ASSERT_NOEXCEPT(current_path(p, ec));
}

TEST_CASE(current_path_test)
{
    std::error_code ec;
    const path p = current_path(ec);
    TEST_REQUIRE(!ec);
    TEST_CHECK(p.is_absolute());
    TEST_CHECK(is_directory(p));

    const path p2 = current_path();
    TEST_CHECK(p2 == p);
}

TEST_CASE(current_path_after_change_test)
{
    const path new_path = StaticEnv::Dir;
    current_path(new_path);
    TEST_CHECK(current_path() == new_path);
}

TEST_CASE(current_path_is_file_test)
{
    const path p = StaticEnv::File;
    std::error_code ec;
    const path old_p = current_path();
    current_path(p, ec);
    TEST_CHECK(ec);
    TEST_CHECK(old_p == current_path());
}

TEST_CASE(set_to_non_absolute_path)
{
    const path base = StaticEnv::Dir;
    current_path(base);
    const path p = StaticEnv::Dir2.filename();
    std::error_code ec;
    current_path(p, ec);
    TEST_CHECK(!ec);
    const path new_cwd = current_path();
    TEST_CHECK(new_cwd == StaticEnv::Dir2);
    TEST_CHECK(new_cwd.is_absolute());
}

TEST_CASE(set_to_empty)
{
    const path p = "";
    std::error_code ec;
    const path old_p = current_path();
    current_path(p, ec);
    TEST_CHECK(ec);
    TEST_CHECK(old_p == current_path());
}

TEST_SUITE_END()
