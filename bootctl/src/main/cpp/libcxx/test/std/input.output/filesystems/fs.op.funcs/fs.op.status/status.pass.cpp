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

// file_status status(const path& p);
// file_status status(const path& p, error_code& ec) noexcept;

#include "filesystem_include.hpp"

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_status_test_suite)

TEST_CASE(signature_test)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(status(p));
    ASSERT_NOEXCEPT(status(p, ec));
}

TEST_CASE(test_status_not_found)
{
    const std::error_code expect_ec =
        std::make_error_code(std::errc::no_such_file_or_directory);
    const path cases[] {
        StaticEnv::DNE,
        StaticEnv::BadSymlink
    };
    for (auto& p : cases) {
        std::error_code ec = std::make_error_code(std::errc::address_in_use);
        // test non-throwing overload.
        file_status st = status(p, ec);
        TEST_CHECK(ec == expect_ec);
        TEST_CHECK(st.type() == file_type::not_found);
        TEST_CHECK(st.permissions() == perms::unknown);
        // test throwing overload. It should not throw even though it reports
        // that the file was not found.
        TEST_CHECK_NO_THROW(st = status(p));
        TEST_CHECK(st.type() == file_type::not_found);
        TEST_CHECK(st.permissions() == perms::unknown);
    }
}

TEST_CASE(test_status_cannot_resolve)
{
    scoped_test_env env;
    const path dir = env.create_dir("dir");
    const path file = env.create_file("dir/file", 42);
    const path sym = env.create_symlink("dir/file", "sym");
    permissions(dir, perms::none);

    const std::error_code set_ec =
        std::make_error_code(std::errc::address_in_use);
    const std::error_code perm_ec =
        std::make_error_code(std::errc::permission_denied);
    const std::error_code name_too_long_ec =
        std::make_error_code(std::errc::filename_too_long);

    struct TestCase {
      path p;
      std::error_code expect_ec;
    } const TestCases[] = {
      {file, perm_ec},
      {sym, perm_ec},
      {path(std::string(2500, 'a')), name_too_long_ec}
    };
    for (auto& TC : TestCases)
    {
        { // test non-throwing case
            std::error_code ec = set_ec;
            file_status st = status(TC.p, ec);
            TEST_CHECK(ec == TC.expect_ec);
            TEST_CHECK(st.type() == file_type::none);
            TEST_CHECK(st.permissions() == perms::unknown);
        }
#ifndef TEST_HAS_NO_EXCEPTIONS
        { // test throwing case
            try {
                status(TC.p);
            } catch (filesystem_error const& err) {
                TEST_CHECK(err.path1() == TC.p);
                TEST_CHECK(err.path2() == "");
                TEST_CHECK(err.code() == TC.expect_ec);
            }
        }
#endif
    }
}

TEST_CASE(status_file_types_test)
{
    scoped_test_env env;
    struct TestCase {
      path p;
      file_type expect_type;
    } cases[] = {
        {StaticEnv::File, file_type::regular},
        {StaticEnv::SymlinkToFile, file_type::regular},
        {StaticEnv::Dir, file_type::directory},
        {StaticEnv::SymlinkToDir, file_type::directory},
        // Block files tested elsewhere
        {StaticEnv::CharFile, file_type::character},
#if !defined(__APPLE__) && !defined(__FreeBSD__) // No support for domain sockets
        {env.create_socket("socket"), file_type::socket},
#endif
        {env.create_fifo("fifo"), file_type::fifo}
    };
    for (const auto& TC : cases) {
        // test non-throwing case
        std::error_code ec = std::make_error_code(std::errc::address_in_use);
        file_status st = status(TC.p, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(st.type() == TC.expect_type);
        TEST_CHECK(st.permissions() != perms::unknown);
        // test throwing case
        TEST_REQUIRE_NO_THROW(st = status(TC.p));
        TEST_CHECK(st.type() == TC.expect_type);
        TEST_CHECK(st.permissions() != perms::unknown);
    }
}

TEST_CASE(test_block_file)
{
    const path possible_paths[] = {
        "/dev/drive0", // Apple
        "/dev/sda",
        "/dev/loop0"
    };
    path p;
    for (const path& possible_p : possible_paths) {
        std::error_code ec;
        if (exists(possible_p, ec)) {
            p = possible_p;
            break;
        }
    }
    if (p == path{}) {
        TEST_UNSUPPORTED();
    }
    // test non-throwing case
    std::error_code ec = std::make_error_code(std::errc::address_in_use);
    file_status st = status(p, ec);
    TEST_CHECK(!ec);
    TEST_CHECK(st.type() == file_type::block);
    TEST_CHECK(st.permissions() != perms::unknown);
    // test throwing case
    TEST_REQUIRE_NO_THROW(st = status(p));
    TEST_CHECK(st.type() == file_type::block);
    TEST_CHECK(st.permissions() != perms::unknown);
}

TEST_SUITE_END()
