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

// uintmax_t file_size(const path& p);
// uintmax_t file_size(const path& p, std::error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(file_size_test_suite)

TEST_CASE(signature_test)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_SAME_TYPE(decltype(file_size(p)), uintmax_t);
    ASSERT_SAME_TYPE(decltype(file_size(p, ec)), uintmax_t);
    ASSERT_NOT_NOEXCEPT(file_size(p));
    ASSERT_NOEXCEPT(file_size(p, ec));
}

TEST_CASE(file_size_empty_test)
{
    const path p = StaticEnv::EmptyFile;
    TEST_CHECK(file_size(p) == 0);
    std::error_code ec;
    TEST_CHECK(file_size(p, ec) == 0);
}

TEST_CASE(file_size_non_empty)
{
    scoped_test_env env;
    const path p = env.create_file("file", 42);
    TEST_CHECK(file_size(p) == 42);
    std::error_code ec;
    TEST_CHECK(file_size(p, ec) == 42);
}

TEST_CASE(symlink_test_case)
{
    const path p = StaticEnv::File;
    const path p2 = StaticEnv::SymlinkToFile;
    TEST_CHECK(file_size(p) == file_size(p2));
}

TEST_CASE(file_size_error_cases)
{
  struct {
    path p;
    std::errc expected_err;
  } TestCases[] = {
      {StaticEnv::Dir, std::errc::is_a_directory},
      {StaticEnv::SymlinkToDir, std::errc::is_a_directory},
      {StaticEnv::BadSymlink, std::errc::no_such_file_or_directory},
      {StaticEnv::DNE, std::errc::no_such_file_or_directory},
      {"", std::errc::no_such_file_or_directory}};
    const uintmax_t expect = static_cast<uintmax_t>(-1);
    for (auto& TC : TestCases) {
      std::error_code ec = GetTestEC();
      TEST_CHECK(file_size(TC.p, ec) == expect);
      TEST_CHECK(ErrorIs(ec, TC.expected_err));

      ExceptionChecker Checker(TC.p, TC.expected_err, "file_size");
      TEST_CHECK_THROW_RESULT(filesystem_error, Checker, file_size(TC.p));
    }
}

TEST_SUITE_END()
