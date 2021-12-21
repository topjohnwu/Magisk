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

// bool remove(const path& p);
// bool remove(const path& p, error_code& ec) noexcept;

#include "filesystem_include.hpp"

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_remove_test_suite)

TEST_CASE(test_signatures)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_SAME_TYPE(decltype(fs::remove(p)), bool);
    ASSERT_SAME_TYPE(decltype(fs::remove(p, ec)), bool);

    ASSERT_NOT_NOEXCEPT(fs::remove(p));
    ASSERT_NOEXCEPT(fs::remove(p, ec));
}

TEST_CASE(test_error_reporting)
{
    auto checkThrow = [](path const& f, const std::error_code& ec)
    {
#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            fs::remove(f);
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
    const path non_empty_dir = env.create_dir("dir");
    env.create_file(non_empty_dir / "file1", 42);
    const path bad_perms_dir = env.create_dir("bad_dir");
    const path file_in_bad_dir = env.create_file(bad_perms_dir / "file", 42);
    permissions(bad_perms_dir, perms::none);
    const path testCases[] = {
        non_empty_dir,
        file_in_bad_dir,
    };
    for (auto& p : testCases) {
        std::error_code ec;

        TEST_CHECK(!fs::remove(p, ec));
        TEST_CHECK(ec);
        TEST_CHECK(checkThrow(p, ec));
    }

    // PR#35780
    const path testCasesNonexistant[] = {
        "",
        env.make_env_path("dne")
    };

    for (auto& p : testCasesNonexistant) {
        std::error_code ec;

        TEST_CHECK(!fs::remove(p, ec));
        TEST_CHECK(!ec);
    }
}

TEST_CASE(basic_remove_test)
{
    scoped_test_env env;
    const path dne = env.make_env_path("dne");
    const path link = env.create_symlink(dne, "link");
    const path nested_link = env.make_env_path("nested_link");
    create_symlink(link, nested_link);
    const path testCases[] = {
        env.create_file("file", 42),
        env.create_dir("empty_dir"),
        nested_link,
        link
    };
    for (auto& p : testCases) {
        std::error_code ec = std::make_error_code(std::errc::address_in_use);
        TEST_CHECK(remove(p, ec));
        TEST_CHECK(!ec);
        TEST_CHECK(!exists(symlink_status(p)));
    }
}

TEST_SUITE_END()
