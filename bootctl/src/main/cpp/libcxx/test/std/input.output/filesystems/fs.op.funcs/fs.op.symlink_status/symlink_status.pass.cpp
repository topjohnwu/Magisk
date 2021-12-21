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

// file_status symlink_status(const path& p);
// file_status symlink_status(const path& p, error_code& ec) noexcept;

#include "filesystem_include.hpp"

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_symlink_status_test_suite)

TEST_CASE(signature_test)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(symlink_status(p));
    ASSERT_NOEXCEPT(symlink_status(p, ec));
}

TEST_CASE(test_symlink_status_not_found)
{
    const std::error_code expect_ec =
        std::make_error_code(std::errc::no_such_file_or_directory);
    const path cases[] {
        StaticEnv::DNE
    };
    for (auto& p : cases) {
        std::error_code ec = std::make_error_code(std::errc::address_in_use);
        // test non-throwing overload.
        file_status st = symlink_status(p, ec);
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

TEST_CASE(test_symlink_status_cannot_resolve)
{
    scoped_test_env env;
    const path dir = env.create_dir("dir");
    const path file_in_dir = env.create_file("dir/file", 42);
    const path sym_in_dir = env.create_symlink("dir/file", "dir/bad_sym");
    const path sym_points_in_dir = env.create_symlink("dir/file", "sym");
    permissions(dir, perms::none);

    const std::error_code set_ec =
        std::make_error_code(std::errc::address_in_use);
    const std::error_code expect_ec =
        std::make_error_code(std::errc::permission_denied);

    const path fail_cases[] = {
        file_in_dir, sym_in_dir
    };
    for (auto& p : fail_cases)
    {
        { // test non-throwing case
            std::error_code ec = set_ec;
            file_status st = symlink_status(p, ec);
            TEST_CHECK(ec == expect_ec);
            TEST_CHECK(st.type() == file_type::none);
            TEST_CHECK(st.permissions() == perms::unknown);
        }
#ifndef TEST_HAS_NO_EXCEPTIONS
        { // test throwing case
            try {
                symlink_status(p);
            } catch (filesystem_error const& err) {
                TEST_CHECK(err.path1() == p);
                TEST_CHECK(err.path2() == "");
                TEST_CHECK(err.code() == expect_ec);
            }
        }
#endif
    }
    // Test that a symlink that points into a directory without read perms
    // can be stat-ed using symlink_status
    {
        std::error_code ec = set_ec;
        file_status st = symlink_status(sym_points_in_dir, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(st.type() == file_type::symlink);
        TEST_CHECK(st.permissions() != perms::unknown);
        // test non-throwing version
        TEST_REQUIRE_NO_THROW(st = symlink_status(sym_points_in_dir));
        TEST_CHECK(st.type() == file_type::symlink);
        TEST_CHECK(st.permissions() != perms::unknown);
    }
}


TEST_CASE(symlink_status_file_types_test)
{
    scoped_test_env env;
    struct TestCase {
      path p;
      file_type expect_type;
    } cases[] = {
        {StaticEnv::BadSymlink, file_type::symlink},
        {StaticEnv::File, file_type::regular},
        {StaticEnv::SymlinkToFile, file_type::symlink},
        {StaticEnv::Dir, file_type::directory},
        {StaticEnv::SymlinkToDir, file_type::symlink},
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
        file_status st = symlink_status(TC.p, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(st.type() == TC.expect_type);
        TEST_CHECK(st.permissions() != perms::unknown);
        // test throwing case
        TEST_REQUIRE_NO_THROW(st = symlink_status(TC.p));
        TEST_CHECK(st.type() == TC.expect_type);
        TEST_CHECK(st.permissions() != perms::unknown);
    }
}

TEST_CASE(test_block_file)
{
    const path possible_paths[] = {
        "/dev/drive0", // Apple
        "/dev/sda",    // Linux
        "/dev/loop0"   // Linux
        // No FreeBSD files known
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
    scoped_test_env env;
    { // test block file
        // test non-throwing case
        std::error_code ec = std::make_error_code(std::errc::address_in_use);
        file_status st = symlink_status(p, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(st.type() == file_type::block);
        TEST_CHECK(st.permissions() != perms::unknown);
        // test throwing case
        TEST_REQUIRE_NO_THROW(st = symlink_status(p));
        TEST_CHECK(st.type() == file_type::block);
        TEST_CHECK(st.permissions() != perms::unknown);
    }
    const path sym = env.make_env_path("sym");
    create_symlink(p, sym);
    { // test symlink to block file
        // test non-throwing case
        std::error_code ec = std::make_error_code(std::errc::address_in_use);
        file_status st = symlink_status(sym, ec);
        TEST_CHECK(!ec);
        TEST_CHECK(st.type() == file_type::symlink);
        TEST_CHECK(st.permissions() != perms::unknown);
        // test throwing case
        TEST_REQUIRE_NO_THROW(st = symlink_status(sym));
        TEST_CHECK(st.type() == file_type::symlink);
        TEST_CHECK(st.permissions() != perms::unknown);
    }
}

TEST_SUITE_END()
