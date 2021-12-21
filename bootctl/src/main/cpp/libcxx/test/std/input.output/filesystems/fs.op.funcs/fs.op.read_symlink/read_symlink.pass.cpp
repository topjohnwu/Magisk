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

// path read_symlink(const path& p);
// path read_symlink(const path& p, error_code& ec);

#include "filesystem_include.hpp"

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_read_symlink_test_suite)

TEST_CASE(test_signatures)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_SAME_TYPE(decltype(fs::read_symlink(p)), fs::path);
    ASSERT_SAME_TYPE(decltype(fs::read_symlink(p, ec)), fs::path);

    ASSERT_NOT_NOEXCEPT(fs::read_symlink(p));
    // Not noexcept because of narrow contract
    ASSERT_NOT_NOEXCEPT(fs::read_symlink(p, ec));
}

TEST_CASE(test_error_reporting)
{
    auto checkThrow = [](path const& f, const std::error_code& ec)
    {
#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            fs::read_symlink(f);
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

    scoped_test_env env;
    const path cases[] = {
        env.make_env_path("dne"),
        env.create_file("file", 42),
        env.create_dir("dir")
    };
    for (path const& p : cases) {
        std::error_code ec;
        const path ret = fs::read_symlink(p, ec);
        TEST_REQUIRE(ec);
        TEST_CHECK(ret == path{});
        TEST_CHECK(checkThrow(p, ec));
    }

}

TEST_CASE(basic_symlink_test)
{
    scoped_test_env env;
    const path dne = env.make_env_path("dne");
    const path file = env.create_file("file", 42);
    const path dir = env.create_dir("dir");
    const path link = env.create_symlink(dne, "link");
    const path nested_link = env.make_env_path("nested_link");
    create_symlink(link, nested_link);
    struct TestCase {
      path symlink;
      path expected;
    } testCases[] = {
        {env.create_symlink(dne, "dne_link"), dne},
        {env.create_symlink(file, "file_link"), file},
        {env.create_symlink(dir, "dir_link"), dir},
        {nested_link, link}
    };
    for (auto& TC : testCases) {
        std::error_code ec = std::make_error_code(std::errc::address_in_use);
        const path ret = read_symlink(TC.symlink, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(ret == TC.expected);
    }
}

TEST_SUITE_END()
