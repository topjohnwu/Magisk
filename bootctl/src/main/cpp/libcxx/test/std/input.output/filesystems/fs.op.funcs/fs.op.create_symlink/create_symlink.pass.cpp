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

// void create_symlink(const path& existing_symlink, const path& new_symlink);
// void create_symlink(const path& existing_symlink, const path& new_symlink,
//                   error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_create_symlink_test_suite)

TEST_CASE(test_signatures)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(fs::create_symlink(p, p));
    ASSERT_NOEXCEPT(fs::create_symlink(p, p, ec));
}

TEST_CASE(test_error_reporting)
{
    scoped_test_env env;
    const path file = env.create_file("file1", 42);
    const path file2 = env.create_file("file2", 55);
    const path sym = env.create_symlink(file, "sym");
    { // destination exists
        std::error_code ec;
        fs::create_symlink(sym, file2, ec);
        TEST_REQUIRE(ec);
    }
}

TEST_CASE(create_symlink_basic)
{
    scoped_test_env env;
    const path file = env.create_file("file", 42);
    const path file_sym = env.create_symlink(file, "file_sym");
    const path dir = env.create_dir("dir");
    const path dir_sym = env.create_symlink(dir, "dir_sym");
    {
        const path dest = env.make_env_path("dest1");
        std::error_code ec;
        fs::create_symlink(file_sym, dest, ec);
        TEST_REQUIRE(!ec);
        TEST_CHECK(is_symlink(dest));
        TEST_CHECK(equivalent(dest, file));
    }
    {
        const path dest = env.make_env_path("dest2");
        std::error_code ec;
        fs::create_symlink(dir_sym, dest, ec);
        TEST_REQUIRE(!ec);
        TEST_CHECK(is_symlink(dest));
        TEST_CHECK(equivalent(dest, dir));
    }
}


TEST_SUITE_END()
