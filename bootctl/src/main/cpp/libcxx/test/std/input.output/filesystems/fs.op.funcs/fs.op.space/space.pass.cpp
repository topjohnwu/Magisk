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

// space_info space(const path& p);
// space_info space(const path& p, error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <sys/statvfs.h>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

bool EqualDelta(std::uintmax_t x, std::uintmax_t y, std::uintmax_t delta) {
    if (x >= y) {
        return (x - y) <= delta;
    } else {
        return (y - x) <= delta;
    }
}

TEST_SUITE(filesystem_space_test_suite)

TEST_CASE(signature_test)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_SAME_TYPE(decltype(space(p)), space_info);
    ASSERT_SAME_TYPE(decltype(space(p, ec)), space_info);
    ASSERT_NOT_NOEXCEPT(space(p));
    ASSERT_NOEXCEPT(space(p, ec));
}

TEST_CASE(test_error_reporting)
{
    auto checkThrow = [](path const& f, const std::error_code& ec)
    {
#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            space(f);
            return false;
        } catch (filesystem_error const& err) {
            return err.path1() == f
                && err.path2() == ""
                && err.code() == ec;
        }
#else
        ((void)f); ((void)ec);
        return true;
#endif
    };
    const path cases[] = {
        "",
        StaticEnv::DNE,
        StaticEnv::BadSymlink
    };
    for (auto& p : cases) {
        const auto expect = static_cast<std::uintmax_t>(-1);
        std::error_code ec;
        space_info info = space(p, ec);
        TEST_CHECK(ec);
        TEST_CHECK(info.capacity == expect);
        TEST_CHECK(info.free == expect);
        TEST_CHECK(info.available == expect);
        TEST_CHECK(checkThrow(p, ec));
    }
}

TEST_CASE(basic_space_test)
{
    // All the test cases should reside on the same filesystem and therefore
    // should have the same expected result. Compute this expected result
    // one and check that it looks semi-sane.
    struct statvfs expect;
    TEST_REQUIRE(::statvfs(StaticEnv::Dir.c_str(), &expect) != -1);
    TEST_CHECK(expect.f_bavail > 0);
    TEST_CHECK(expect.f_bfree > 0);
    TEST_CHECK(expect.f_bsize > 0);
    TEST_CHECK(expect.f_blocks > 0);
    TEST_REQUIRE(expect.f_frsize > 0);
    auto do_mult = [&](std::uintmax_t val) {
        std::uintmax_t fsize = expect.f_frsize;
        std::uintmax_t new_val = val * fsize;
        TEST_CHECK(new_val / fsize == val); // Test for overflow
        return new_val;
    };
    const std::uintmax_t bad_value = static_cast<std::uintmax_t>(-1);
    const std::uintmax_t expect_capacity = do_mult(expect.f_blocks);
    const std::uintmax_t expect_free = do_mult(expect.f_bfree);
    const std::uintmax_t expect_avail = do_mult(expect.f_bavail);

    // Other processes running on the operating system may have changed
    // the amount of space available. Check that these are within tolerances.
    // Currently 5% of capacity
    const std::uintmax_t delta = expect_capacity / 20;
    const path cases[] = {
        StaticEnv::File,
        StaticEnv::Dir,
        StaticEnv::Dir2,
        StaticEnv::SymlinkToFile,
        StaticEnv::SymlinkToDir
    };
    for (auto& p : cases) {
        std::error_code ec = GetTestEC();
        space_info info = space(p, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(info.capacity != bad_value);
        TEST_CHECK(expect_capacity == info.capacity);
        TEST_CHECK(info.free != bad_value);
        TEST_CHECK(EqualDelta(expect_free, info.free, delta));
        TEST_CHECK(info.available != bad_value);
        TEST_CHECK(EqualDelta(expect_avail, info.available, delta));
    }
}

TEST_SUITE_END()
