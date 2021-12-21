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

// bool create_directories(const path& p);
// bool create_directories(const path& p, error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(filesystem_create_directories_test_suite)

TEST_CASE(test_signatures)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_SAME_TYPE(decltype(fs::create_directories(p)), bool);
    ASSERT_SAME_TYPE(decltype(fs::create_directories(p, ec)), bool);
    ASSERT_NOT_NOEXCEPT(fs::create_directories(p));
    ASSERT_NOT_NOEXCEPT(fs::create_directories(p, ec));
}

TEST_CASE(create_existing_directory)
{
    scoped_test_env env;
    const path dir = env.create_dir("dir1");
    std::error_code ec;
    TEST_CHECK(fs::create_directories(dir, ec) == false);
    TEST_CHECK(!ec);
    TEST_CHECK(is_directory(dir));
}

TEST_CASE(create_directory_one_level)
{
    scoped_test_env env;
    const path dir = env.make_env_path("dir1");
    std::error_code ec;
    TEST_CHECK(fs::create_directories(dir, ec) == true);
    TEST_CHECK(!ec);
    TEST_CHECK(is_directory(dir));
}

TEST_CASE(create_directories_multi_level)
{
    scoped_test_env env;
    const path dir = env.make_env_path("dir1/dir2/dir3");
    std::error_code ec;
    TEST_CHECK(fs::create_directories(dir, ec) == true);
    TEST_CHECK(!ec);
    TEST_CHECK(is_directory(dir));
}

TEST_CASE(create_directory_symlinks) {
  scoped_test_env env;
  const path root = env.create_dir("dir");
  const path sym_dest_dead = env.make_env_path("dead");
  const path dead_sym = env.create_symlink(sym_dest_dead, "dir/sym_dir");
  const path target = env.make_env_path("dir/sym_dir/foo");
  {
    std::error_code ec = GetTestEC();
    TEST_CHECK(create_directories(target, ec) == false);
    TEST_CHECK(ec);
    TEST_CHECK(!exists(sym_dest_dead));
    TEST_CHECK(!exists(dead_sym));
  }
}


TEST_CASE(create_directory_through_symlinks) {
  scoped_test_env env;
  const path root = env.create_dir("dir");
  const path sym_dir = env.create_symlink(root, "sym_dir");
  const path target = env.make_env_path("sym_dir/foo");
  const path resolved_target = env.make_env_path("dir/foo");
  TEST_REQUIRE(is_directory(sym_dir));
  {
    std::error_code ec = GetTestEC();
    TEST_CHECK(create_directories(target, ec) == true);
    TEST_CHECK(!ec);
    TEST_CHECK(is_directory(target));
    TEST_CHECK(is_directory(resolved_target));
  }
}

TEST_SUITE_END()
