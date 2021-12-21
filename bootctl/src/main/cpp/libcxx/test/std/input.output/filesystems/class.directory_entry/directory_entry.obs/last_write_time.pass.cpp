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

// class directory_entry

// file_time_type last_write_time() const;
// file_time_type last_write_time(error_code const&) const noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "filesystem_test_helper.hpp"
#include "rapid-cxx-test.hpp"

TEST_SUITE(directory_entry_obs_testsuite)

TEST_CASE(signatures) {
  using namespace fs;
  {
    const fs::directory_entry e = {};
    std::error_code ec;
    static_assert(std::is_same<decltype(e.last_write_time()), file_time_type>::value,
                  "");
    static_assert(std::is_same<decltype(e.last_write_time(ec)), file_time_type>::value,
                  "");
    static_assert(noexcept(e.last_write_time()) == false, "");
    static_assert(noexcept(e.last_write_time(ec)) == true, "");
  }
}

TEST_CASE(basic) {
  using namespace fs;

  scoped_test_env env;
  const path file = env.create_file("file", 42);
  const path dir = env.create_dir("dir");
  const path sym = env.create_symlink("file", "sym");

  {
    directory_entry ent(file);
    file_time_type expect = last_write_time(ent);

    // Remove the file to show that the results were already in the cache.
    LIBCPP_ONLY(remove(file));

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == expect);
    TEST_CHECK(!ec);
  }
  {
    directory_entry ent(dir);
    file_time_type expect = last_write_time(ent);

    LIBCPP_ONLY(remove(dir));

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == expect);
    TEST_CHECK(!ec);
  }
  env.create_file("file", 99);
  {
    directory_entry ent(sym);
    file_time_type expect = last_write_time(sym);

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == expect);
    TEST_CHECK(!ec);
  }
}

TEST_CASE(error_reporting) {
  using namespace fs;

  scoped_test_env env;

  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path file_out_of_dir = env.create_file("file2", 101);
  const path sym_out_of_dir = env.create_symlink("dir/file", "sym");
  const path sym_in_dir = env.create_symlink("file2", "dir/sym2");

  const perms old_perms = status(dir).permissions();

  // test a file which doesn't exist
  {
    directory_entry ent;

    std::error_code ec = GetTestEC();
    ent.assign(StaticEnv::DNE, ec);
    TEST_REQUIRE(ent.path() == StaticEnv::DNE);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ExceptionChecker Checker(StaticEnv::DNE,
                             std::errc::no_such_file_or_directory,
                             "directory_entry::last_write_time");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.last_write_time());
  }
  // test a dead symlink
  {
    directory_entry ent;

    std::error_code ec = GetTestEC();
    file_time_type expect_bad = last_write_time(StaticEnv::BadSymlink, ec);
    TEST_CHECK(expect_bad == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ec = GetTestEC();
    ent.assign(StaticEnv::BadSymlink, ec);
    TEST_REQUIRE(ent.path() == StaticEnv::BadSymlink);
    TEST_CHECK(!ec);

    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == expect_bad);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ExceptionChecker Checker(StaticEnv::BadSymlink,
                             std::errc::no_such_file_or_directory,
                             "directory_entry::last_write_time");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.last_write_time());
  }
  // test a file w/o appropriate permissions.
  {
    directory_entry ent;
    file_time_type expect_good = last_write_time(file);
    permissions(dir, perms::none);

    std::error_code ec = GetTestEC();
    ent.assign(file, ec);
    TEST_REQUIRE(ent.path() == file);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ExceptionChecker Checker(file, std::errc::permission_denied,
                             "last_write_time");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.last_write_time());

    permissions(dir, old_perms);
    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == expect_good);
    TEST_CHECK(!ec);
    TEST_CHECK_NO_THROW(ent.last_write_time());
  }
  permissions(dir, old_perms);
  // test a symlink w/o appropriate permissions.
  {
    directory_entry ent;
    file_time_type expect_good = last_write_time(sym_in_dir);
    permissions(dir, perms::none);

    std::error_code ec = GetTestEC();
    ent.assign(sym_in_dir, ec);
    TEST_REQUIRE(ent.path() == sym_in_dir);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ExceptionChecker Checker(sym_in_dir, std::errc::permission_denied,
                             "last_write_time");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.last_write_time());

    permissions(dir, old_perms);
    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == expect_good);
    TEST_CHECK(!ec);
    TEST_CHECK_NO_THROW(ent.last_write_time());
  }
  permissions(dir, old_perms);
  // test a symlink to a file w/o appropriate permissions
  {
    directory_entry ent;
    file_time_type expect_good = last_write_time(sym_out_of_dir);
    permissions(dir, perms::none);

    std::error_code ec = GetTestEC();
    ent.assign(sym_out_of_dir, ec);
    TEST_REQUIRE(ent.path() == sym_out_of_dir);
    TEST_CHECK(!ec);

    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == file_time_type::min());
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ExceptionChecker Checker(sym_out_of_dir, std::errc::permission_denied,
                             "last_write_time");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.last_write_time());

    permissions(dir, old_perms);
    ec = GetTestEC();
    TEST_CHECK(ent.last_write_time(ec) == expect_good);
    TEST_CHECK(!ec);
    TEST_CHECK_NO_THROW(ent.last_write_time());
  }
}

TEST_SUITE_END()
