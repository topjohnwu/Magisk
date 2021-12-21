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

// directory_entry& operator=(directory_entry const&) = default;
// directory_entry& operator=(directory_entry&&) noexcept = default;
// void assign(path const&);
// void replace_filename(path const&);

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

TEST_SUITE(directory_entry_mods_suite)

TEST_CASE(test_refresh_method) {
  using namespace fs;
  {
    directory_entry e;
    static_assert(noexcept(e.refresh()) == false,
                  "operation cannot be noexcept");
    static_assert(std::is_same<decltype(e.refresh()), void>::value,
                  "operation must return void");
  }
  {
    directory_entry e;
    e.refresh();
    TEST_CHECK(!e.exists());
  }
}

TEST_CASE(test_refresh_ec_method) {
  using namespace fs;
  {
    directory_entry e;
    std::error_code ec;
    static_assert(noexcept(e.refresh(ec)), "operation should be noexcept");
    static_assert(std::is_same<decltype(e.refresh(ec)), void>::value,
                  "operation must return void");
  }
  {
    directory_entry e;
    std::error_code ec = GetTestEC();
    e.refresh(ec);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));
  }
}

TEST_CASE(refresh_on_file_dne) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);

  const perms old_perms = status(dir).permissions();

  // test file doesn't exist
  {
    directory_entry ent(file);
    remove(file);
    TEST_CHECK(ent.exists());

    ent.refresh();

    permissions(dir, perms::none);
    TEST_CHECK(!ent.exists());
  }
  permissions(dir, old_perms);
  env.create_file("dir/file", 101);
  {
    directory_entry ent(file);
    remove(file);
    TEST_CHECK(ent.exists());

    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    permissions(dir, perms::none);
    TEST_CHECK(!ent.exists());
  }
}

void remove_if_exists(const fs::path& p) {
  std::error_code ec;
  remove(p, ec);
}

TEST_CASE(refresh_on_bad_symlink) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path sym = env.create_symlink("dir/file", "sym");

  const perms old_perms = status(dir).permissions();

  // test file doesn't exist
  {
    directory_entry ent(sym);
    LIBCPP_ONLY(remove(file));
    TEST_CHECK(ent.is_symlink());
    TEST_CHECK(ent.is_regular_file());
    TEST_CHECK(ent.exists());

    remove_if_exists(file);
    ent.refresh();

    LIBCPP_ONLY(permissions(dir, perms::none));
    TEST_CHECK(ent.is_symlink());
    TEST_CHECK(!ent.is_regular_file());
    TEST_CHECK(!ent.exists());
  }
  permissions(dir, old_perms);
  env.create_file("dir/file", 101);
  {
    directory_entry ent(sym);
    LIBCPP_ONLY(remove(file));
    TEST_CHECK(ent.is_symlink());
    TEST_CHECK(ent.is_regular_file());
    TEST_CHECK(ent.exists());

    remove_if_exists(file);

    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(!ec); // we don't report bad symlinks as an error.

    LIBCPP_ONLY(permissions(dir, perms::none));
    TEST_CHECK(!ent.exists());
  }
}

TEST_CASE(refresh_cannot_resolve) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path file_out_of_dir = env.create_file("file1", 99);
  const path sym_out_of_dir = env.create_symlink("dir/file", "sym");
  const path sym_in_dir = env.create_symlink("file1", "dir/sym1");
  perms old_perms = status(dir).permissions();

  {
    directory_entry ent(file);
    permissions(dir, perms::none);

    TEST_CHECK(ent.is_regular_file());

    std::error_code ec = GetTestEC();
    ent.refresh(ec);

    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));
    TEST_CHECK(ent.path() == file);

    ExceptionChecker Checker(file, std::errc::permission_denied,
                             "directory_entry::refresh");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.refresh());
  }
  permissions(dir, old_perms);
  {
    directory_entry ent(sym_in_dir);
    permissions(dir, perms::none);
    TEST_CHECK(ent.is_symlink());

    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));
    TEST_CHECK(ent.path() == sym_in_dir);

    ExceptionChecker Checker(sym_in_dir, std::errc::permission_denied,
                             "directory_entry::refresh");
    TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ent.refresh());
  }
  permissions(dir, old_perms);
  {
    directory_entry ent(sym_out_of_dir);
    permissions(dir, perms::none);
    TEST_CHECK(ent.is_symlink());

    // Failure to resolve the linked entity due to permissions is not
    // reported as an error.
    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(!ec);
    TEST_CHECK(ent.is_symlink());

    ec = GetTestEC();
    TEST_CHECK(ent.exists(ec) == false);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));
    TEST_CHECK(ent.path() == sym_out_of_dir);
  }
  permissions(dir, old_perms);
  {
    directory_entry ent_file(file);
    directory_entry ent_sym(sym_in_dir);
    directory_entry ent_sym2(sym_out_of_dir);
    permissions(dir, perms::none);
    ((void)ent_file);
    ((void)ent_sym);

    TEST_CHECK_THROW(filesystem_error, ent_file.refresh());
    TEST_CHECK_THROW(filesystem_error, ent_sym.refresh());
    TEST_CHECK_NO_THROW(ent_sym2);
  }
}

TEST_CASE(refresh_doesnt_throw_on_dne_but_reports_it) {
  using namespace fs;
  scoped_test_env env;

  const path file = env.create_file("file1", 42);
  const path sym = env.create_symlink("file1", "sym");

  {
    directory_entry ent(file);
    TEST_CHECK(ent.file_size() == 42);

    remove(file);

    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));
    TEST_CHECK_NO_THROW(ent.refresh());

    ec = GetTestEC();
    TEST_CHECK(ent.file_size(ec) == uintmax_t(-1));
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    // doesn't throw!
    TEST_CHECK_THROW(filesystem_error, ent.file_size());
  }
  env.create_file("file1", 99);
  {
    directory_entry ent(sym);
    TEST_CHECK(ent.is_symlink());
    TEST_CHECK(ent.is_regular_file());
    TEST_CHECK(ent.file_size() == 99);

    remove(file);

    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(!ec);

    ec = GetTestEC();
    TEST_CHECK(ent.file_size(ec) == uintmax_t(-1));
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));

    TEST_CHECK_THROW(filesystem_error, ent.file_size());
  }
}

TEST_CASE(access_cache_after_refresh_fails) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path file_out_of_dir = env.create_file("file1", 101);
  const path sym = env.create_symlink("dir/file", "sym");
  const path sym_in_dir = env.create_symlink("dir/file", "dir/sym2");

  const perms old_perms = status(dir).permissions();

#define CHECK_ACCESS(func, expect)                                             \
  ec = GetTestEC();                                                            \
  TEST_CHECK(ent.func(ec) == expect);                                          \
  TEST_CHECK(ErrorIs(ec, std::errc::permission_denied))

  // test file doesn't exist
  {
    directory_entry ent(file);

    TEST_CHECK(!ent.is_symlink());
    TEST_CHECK(ent.is_regular_file());
    TEST_CHECK(ent.exists());

    permissions(dir, perms::none);
    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    CHECK_ACCESS(exists, false);
    CHECK_ACCESS(is_symlink, false);
    CHECK_ACCESS(last_write_time, file_time_type::min());
    CHECK_ACCESS(hard_link_count, uintmax_t(-1));
  }
  permissions(dir, old_perms);
  {
    directory_entry ent(sym_in_dir);
    TEST_CHECK(ent.is_symlink());
    TEST_CHECK(ent.is_regular_file());
    TEST_CHECK(ent.exists());

    permissions(dir, perms::none);
    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));

    CHECK_ACCESS(exists, false);
    CHECK_ACCESS(is_symlink, false);
    CHECK_ACCESS(last_write_time, file_time_type::min());
    CHECK_ACCESS(hard_link_count, uintmax_t(-1));
  }
  permissions(dir, old_perms);
  {
    directory_entry ent(sym);
    TEST_CHECK(ent.is_symlink());
    TEST_CHECK(ent.is_regular_file());
    TEST_CHECK(ent.exists());

    permissions(dir, perms::none);
    std::error_code ec = GetTestEC();
    ent.refresh(ec);
    TEST_CHECK(!ec);
    TEST_CHECK(ent.is_symlink());

    CHECK_ACCESS(exists, false);
    CHECK_ACCESS(is_regular_file, false);
    CHECK_ACCESS(last_write_time, file_time_type::min());
    CHECK_ACCESS(hard_link_count, uintmax_t(-1));
  }
#undef CHECK_ACCESS
}

TEST_SUITE_END()
