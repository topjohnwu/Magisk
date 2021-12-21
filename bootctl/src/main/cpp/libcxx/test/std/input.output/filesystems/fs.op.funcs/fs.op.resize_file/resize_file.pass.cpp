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

// void resize_file(const path& p, uintmax_t new_size);
// void resize_file(const path& p, uintmax_t new_size, error_code& ec) noexcept;

#include "filesystem_include.hpp"

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_resize_file_test_suite)

TEST_CASE(test_signatures)
{
    const path p; ((void)p);
    std::uintmax_t i; ((void)i);
    std::error_code ec; ((void)ec);

    ASSERT_SAME_TYPE(decltype(fs::resize_file(p, i)), void);
    ASSERT_SAME_TYPE(decltype(fs::resize_file(p, i, ec)), void);

    ASSERT_NOT_NOEXCEPT(fs::resize_file(p, i));
    ASSERT_NOEXCEPT(fs::resize_file(p, i, ec));
}

TEST_CASE(test_error_reporting)
{
    auto checkThrow = [](path const& f, std::uintmax_t s, const std::error_code& ec)
    {
#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            fs::resize_file(f, s);
            return false;
        } catch (filesystem_error const& err) {
            return err.path1() == f
                && err.path2() == ""
                && err.code() == ec;
        }
#else
        ((void)f); ((void)s); ((void)ec);
        return true;
#endif
    };
    scoped_test_env env;
    const path dne = env.make_env_path("dne");
    const path bad_sym = env.create_symlink(dne, "sym");
    const path dir = env.create_dir("dir1");
    const path cases[] = {
        dne, bad_sym, dir
    };
    for (auto& p : cases) {
        std::error_code ec;
        resize_file(p, 42, ec);
        TEST_REQUIRE(ec);
        TEST_CHECK(checkThrow(p, 42, ec));
    }
}

TEST_CASE(basic_resize_file_test)
{
    scoped_test_env env;
    const path file1 = env.create_file("file1", 42);
    const auto set_ec = std::make_error_code(std::errc::address_in_use);
    { // grow file
        const std::uintmax_t new_s = 100;
        std::error_code ec = set_ec;
        resize_file(file1, new_s, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(file_size(file1) == new_s);
    }
    { // shrink file
        const std::uintmax_t new_s = 1;
        std::error_code ec = set_ec;
        resize_file(file1, new_s, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(file_size(file1) == new_s);
    }
    { // shrink file to zero
        const std::uintmax_t new_s = 0;
        std::error_code ec = set_ec;
        resize_file(file1, new_s, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(file_size(file1) == new_s);
    }
    const path sym = env.create_symlink(file1, "sym");
    { // grow file via symlink
        const std::uintmax_t new_s = 1024;
        std::error_code ec = set_ec;
        resize_file(sym, new_s, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(file_size(file1) == new_s);
    }
}

TEST_SUITE_END()
