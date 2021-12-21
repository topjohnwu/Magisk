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

// uintmax_t hard_link_count() const;
// uintmax_t hard_link_count(error_code const&) const noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "filesystem_test_helper.hpp"
#include "rapid-cxx-test.hpp"

TEST_SUITE(directory_entry_obs_testsuite)

TEST_CASE(signatures) {
  using namespace fs;
  {
    const directory_entry e = {};
    std::error_code ec;
    static_assert(std::is_same<decltype(e.hard_link_count()), uintmax_t>::value, "");
    static_assert(std::is_same<decltype(e.hard_link_count(ec)), uintmax_t>::value,
                  "");
    static_assert(noexcept(e.hard_link_count()) == false, "");
    static_assert(noexcept(e.hard_link_count(ec)) == true, "");
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
    uintmax_t expect = hard_link_count(ent);

    // Remove the file to show that the results were already in the cache.
    LIBCPP_ONLY(remove(file));

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == expect);
    TEST_CHECK(!ec);
  }
  {
    directory_entry ent(dir);
    uintmax_t expect = hard_link_count(ent);

    LIBCPP_ONLY(remove(dir));

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == expect);
    TEST_CHECK(!ec);
  }
  env.create_file("file", 99);
  env.create_hardlink("file", "hl");
  {
    directory_entry ent(sym);
    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == 2);
    TEST_CHECK(!ec);
  }
}

TEST_CASE(not_regular_file) {
  using namespace fs;

  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path dir2 = env.create_dir("dir/dir2");
  const path fifo = env.create_fifo("dir/fifo");
  const path sym_to_fifo = env.create_symlink("dir/fifo", "dir/sym");

  const perms old_perms = status(dir).permissions();

  for (auto p : {dir2, fifo, sym_to_fifo}) {
    permissions(dir, old_perms);
    std::error_code dummy_ec = GetTestEC();
    directory_entry ent(p, dummy_ec);
    TEST_CHECK(!dummy_ec);

    uintmax_t expect = hard_link_count(p);

    LIBCPP_ONLY(permissions(dir, perms::none));

    std::error_code ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == expect);
    TEST_CHECK(!ec);
    TEST_CHECK_NO_THROW(ent.hard_link_count());
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
    TEST_CHECK(ec);
    TEST_REQUIRE(ent.path() == StaticEnv::DNE);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == uintmax_t(-1));
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ExceptionChecker Checker(StaticEnv::DNE,
                             std::errc::no_such_file_or_directory,
                             "directory_entry::hard_link_count");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.hard_link_count());
  }
  // test a dead symlink
  {
    directory_entry ent;

    std::error_code ec = GetTestEC();
    uintmax_t expect_bad = hard_link_count(StaticEnv::BadSymlink, ec);
    TEST_CHECK(expect_bad == uintmax_t(-1));
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ec = GetTestEC();
    ent.assign(StaticEnv::BadSymlink, ec);
    TEST_REQUIRE(ent.path() == StaticEnv::BadSymlink);
    TEST_CHECK(!ec);

    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == expect_bad);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    ExceptionChecker Checker(StaticEnv::BadSymlink,
                             std::errc::no_such_file_or_directory,
                             "directory_entry::hard_link_count");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.hard_link_count());
  }
  // test a file w/o appropriate permissions.
  {
    directory_entry ent;
    uintmax_t expect_good = hard_link_count(file);
    permissions(dir, perms::none);

    std::error_code ec = GetTestEC();
    ent.assign(file, ec);
    TEST_REQUIRE(ent.path() == file);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == uintmax_t(-1));
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ExceptionChecker Checker(file, std::errc::permission_denied,
                             "hard_link_count");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.hard_link_count());

    permissions(dir, old_perms);
    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == expect_good);
    TEST_CHECK(!ec);
    TEST_CHECK_NO_THROW(ent.hard_link_count());
  }
  permissions(dir, old_perms);
  // test a symlink w/o appropriate permissions.
  {
    directory_entry ent;
    uintmax_t expect_good = hard_link_count(sym_in_dir);
    permissions(dir, perms::none);

    std::error_code ec = GetTestEC();
    ent.assign(sym_in_dir, ec);
    TEST_REQUIRE(ent.path() == sym_in_dir);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == uintmax_t(-1));
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ExceptionChecker Checker(sym_in_dir, std::errc::permission_denied,
                             "hard_link_count");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.hard_link_count());

    permissions(dir, old_perms);
    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == expect_good);
    TEST_CHECK(!ec);
    TEST_CHECK_NO_THROW(ent.hard_link_count());
  }
  permissions(dir, old_perms);
  // test a symlink to a file w/o appropriate permissions
  {
    directory_entry ent;
    uintmax_t expect_good = hard_link_count(sym_out_of_dir);
    permissions(dir, perms::none);

    std::error_code ec = GetTestEC();
    ent.assign(sym_out_of_dir, ec);
    TEST_REQUIRE(ent.path() == sym_out_of_dir);
    TEST_CHECK(!ec);

    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == uintmax_t(-1));
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    ExceptionChecker Checker(sym_out_of_dir, std::errc::permission_denied,
                             "hard_link_count");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.hard_link_count());

    permissions(dir, old_perms);
    ec = GetTestEC();
    TEST_CHECK(ent.hard_link_count(ec) == expect_good);
    TEST_CHECK(!ec);
    TEST_CHECK_NO_THROW(ent.hard_link_count());
  }
}

TEST_SUITE_END()
