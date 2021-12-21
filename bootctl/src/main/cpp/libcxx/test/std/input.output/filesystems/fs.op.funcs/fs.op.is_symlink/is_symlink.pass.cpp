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

// bool is_symlink(file_status s) noexcept
// bool is_symlink(path const& p);
// bool is_symlink(path const& p, std::error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(is_symlink_test_suite)

TEST_CASE(signature_test)
{
    file_status s; ((void)s);
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOEXCEPT(is_symlink(s));
    ASSERT_NOEXCEPT(is_symlink(p, ec));
    ASSERT_NOT_NOEXCEPT(is_symlink(p));
}

TEST_CASE(is_symlink_status_test)
{
    struct TestCase {
        file_type type;
        bool expect;
    };
    const TestCase testCases[] = {
        {file_type::none, false},
        {file_type::not_found, false},
        {file_type::regular, false},
        {file_type::directory, false},
        {file_type::symlink, true},
        {file_type::block, false},
        {file_type::character, false},
        {file_type::fifo, false},
        {file_type::socket, false},
        {file_type::unknown, false}
    };
    for (auto& TC : testCases) {
        file_status s(TC.type);
        TEST_CHECK(is_symlink(s) == TC.expect);
    }
}

TEST_CASE(static_env_test)
{
    struct TestCase {
        path p;
        bool expect;
    };
    const TestCase testCases[] = {
        {StaticEnv::File, false},
        {StaticEnv::Dir, false},
        {StaticEnv::SymlinkToFile, true},
        {StaticEnv::SymlinkToDir, true},
        {StaticEnv::BadSymlink, true}
    };
    for (auto& TC : testCases) {
        TEST_CHECK(is_symlink(TC.p) == TC.expect);
    }
}

TEST_CASE(test_exist_not_found)
{
    const path p = StaticEnv::DNE;
    TEST_CHECK(is_symlink(p) == false);
    std::error_code ec;
    TEST_CHECK(is_symlink(p, ec) == false);
    TEST_CHECK(ec);
}

TEST_CASE(test_is_symlink_fails)
{
    scoped_test_env env;
    const path dir = env.create_dir("dir");
    const path file = env.create_file("dir/file", 42);
    permissions(dir, perms::none);

    std::error_code ec;
    TEST_CHECK(is_symlink(file, ec) == false);
    TEST_CHECK(ec);

    TEST_CHECK_THROW(filesystem_error, is_symlink(file));
}

TEST_SUITE_END()
