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

// void copy_symlink(const path& existing_symlink, const path& new_symlink);
// void copy_symlink(const path& existing_symlink, const path& new_symlink,
//                   error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_copy_symlink_test_suite)

TEST_CASE(test_signatures)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(fs::copy_symlink(p, p));
    ASSERT_NOEXCEPT(fs::copy_symlink(p, p, ec));
}


TEST_CASE(test_error_reporting)
{
    auto checkThrow = [](path const& f, path const& t, const std::error_code& ec)
    {
#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            fs::copy_symlink(f, t);
            return true;
        } catch (filesystem_error const& err) {
            return err.path1() == f
                && err.code() == ec;
        }
#else
        ((void)f); ((void)t); ((void)ec);
        return true;
#endif
    };

    scoped_test_env env;
    const path file = env.create_file("file1", 42);
    const path file2 = env.create_file("file2", 55);
    const path sym = env.create_symlink(file, "sym");
    const path dir = env.create_dir("dir");
    const path dne = env.make_env_path("dne");
    { // from is a file, not a symlink
        std::error_code ec;
        fs::copy_symlink(file, dne, ec);
        TEST_REQUIRE(ec);
        TEST_CHECK(checkThrow(file, dne, ec));
    }
    { // from is a file, not a symlink
        std::error_code ec;
        fs::copy_symlink(dir, dne, ec);
        TEST_REQUIRE(ec);
        TEST_CHECK(checkThrow(dir, dne, ec));
    }
    { // destination exists
        std::error_code ec;
        fs::copy_symlink(sym, file2, ec);
        TEST_REQUIRE(ec);
    }
}

TEST_CASE(copy_symlink_basic)
{
    scoped_test_env env;
    const path dir = env.create_dir("dir");
    const path dir_sym = env.create_symlink(dir, "dir_sym");
    const path file = env.create_file("file", 42);
    const path file_sym = env.create_symlink(file, "file_sym");
    { // test for directory symlinks
        const path dest = env.make_env_path("dest1");
        std::error_code ec;
        fs::copy_symlink(dir_sym, dest, ec);
        TEST_REQUIRE(!ec);
        TEST_CHECK(is_symlink(dest));
        TEST_CHECK(equivalent(dest, dir));
    }
    { // test for file symlinks
        const path dest = env.make_env_path("dest2");
        std::error_code ec;
        fs::copy_symlink(file_sym, dest, ec);
        TEST_REQUIRE(!ec);
        TEST_CHECK(is_symlink(dest));
        TEST_CHECK(equivalent(dest, file));
    }
}


TEST_SUITE_END()
