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

// bool is_directory(file_status s) noexcept
// bool is_directory(path const& p);
// bool is_directory(path const& p, std::error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(is_directory_test_suite)

TEST_CASE(signature_test)
{
    file_status s; ((void)s);
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOEXCEPT(is_directory(s));
    ASSERT_NOEXCEPT(is_directory(p, ec));
    ASSERT_NOT_NOEXCEPT(is_directory(p));
}

TEST_CASE(is_directory_status_test)
{
    struct TestCase {
        file_type type;
        bool expect;
    };
    const TestCase testCases[] = {
        {file_type::none, false},
        {file_type::not_found, false},
        {file_type::regular, false},
        {file_type::directory, true},
        {file_type::symlink, false},
        {file_type::block, false},
        {file_type::character, false},
        {file_type::fifo, false},
        {file_type::socket, false},
        {file_type::unknown, false}
    };
    for (auto& TC : testCases) {
        file_status s(TC.type);
        TEST_CHECK(is_directory(s) == TC.expect);
    }
}

TEST_CASE(test_exist_not_found)
{
    const path p = StaticEnv::DNE;
    TEST_CHECK(is_directory(p) == false);
}

TEST_CASE(static_env_test)
{
    TEST_CHECK(is_directory(StaticEnv::Dir));
    TEST_CHECK(is_directory(StaticEnv::SymlinkToDir));
    TEST_CHECK(!is_directory(StaticEnv::File));
}

TEST_CASE(test_is_directory_fails)
{
    scoped_test_env env;
    const path dir = env.create_dir("dir");
    const path dir2 = env.create_dir("dir/dir2");
    permissions(dir, perms::none);

    std::error_code ec;
    TEST_CHECK(is_directory(dir2, ec) == false);
    TEST_CHECK(ec);

    TEST_CHECK_THROW(filesystem_error, is_directory(dir2));
}

TEST_SUITE_END()
