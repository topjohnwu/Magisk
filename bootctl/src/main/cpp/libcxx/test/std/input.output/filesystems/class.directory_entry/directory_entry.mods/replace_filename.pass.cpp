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

TEST_CASE(test_replace_filename_method) {
  using namespace fs;

  {
    directory_entry e;
    path replace;
    static_assert(noexcept(e.replace_filename(replace)) == false,
                  "operation cannot be noexcept");
    static_assert(
        std::is_same<decltype(e.replace_filename(replace)), void>::value,
        "operation must return void");
  }
  {
    const path p("/path/to/foo.exe");
    const path replace("bar.out");
    const path expect("/path/to/bar.out");
    directory_entry e(p);
    TEST_CHECK(e.path() == p);
    e.replace_filename(replace);
    TEST_CHECK(e.path() == expect);
  }
}

TEST_CASE(test_replace_filename_ec_method) {
  using namespace fs;

  {
    directory_entry e;
    path replace;
    std::error_code ec;
    static_assert(noexcept(e.replace_filename(replace, ec)) == false,
                  "operation cannot be noexcept");
    static_assert(
        std::is_same<decltype(e.replace_filename(replace, ec)), void>::value,
        "operation must return void");
  }
  {
    const path p("/path/to/foo.exe");
    const path replace("bar.out");
    const path expect("/path/to/bar.out");
    directory_entry e(p);
    TEST_CHECK(e.path() == p);
    std::error_code ec = GetTestEC();
    e.replace_filename(replace, ec);
    TEST_CHECK(e.path() == expect);
    TEST_CHECK(ErrorIs(ec, std::errc::no_such_file_or_directory));
  }
  {
    const path p = StaticEnv::EmptyFile;
    const path expect = StaticEnv::NonEmptyFile;
    const path replace = StaticEnv::NonEmptyFile.filename();
    TEST_REQUIRE(expect.parent_path() == p.parent_path());
    directory_entry e(p);
    TEST_CHECK(e.path() == p);
    std::error_code ec = GetTestEC();
    e.replace_filename(replace, ec);
    TEST_CHECK(e.path() == expect);
    TEST_CHECK(!ec);
  }
}

TEST_CASE(test_replace_filename_calls_refresh) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path file_two = env.create_file("dir/file_two", 101);
  const path sym = env.create_symlink("dir/file", "sym");
  const path sym_two = env.create_symlink("dir/file_two", "sym_two");

  {
    directory_entry ent(file);
    ent.replace_filename(file_two.filename());
    TEST_REQUIRE(ent.path() == file_two);

    // removing the file demonstrates that the values where cached previously.
    LIBCPP_ONLY(remove(file_two));

    TEST_CHECK(ent.file_size() == 101);
  }
  env.create_file("dir/file_two", 99);
  {
    directory_entry ent(sym);
    ent.replace_filename(sym_two.filename());
    TEST_REQUIRE(ent.path() == sym_two);

    LIBCPP_ONLY(remove(file_two));
    LIBCPP_ONLY(remove(sym_two));

    TEST_CHECK(ent.is_symlink());
    TEST_CHECK(ent.is_regular_file());
    TEST_CHECK(ent.file_size() == 99);
  }
}

TEST_CASE(test_replace_filename_propagates_error) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path file_two = env.create_file("dir/file_two", 99);
  const path file_out_of_dir = env.create_file("file_three", 101);
  const path sym_out_of_dir = env.create_symlink("dir/file", "sym");
  const path sym_out_of_dir_two = env.create_symlink("dir/file", "sym_two");
  const path sym_in_dir = env.create_symlink("file_two", "dir/sym_three");
  const path sym_in_dir_two = env.create_symlink("file_two", "dir/sym_four");

  const perms old_perms = status(dir).permissions();

  {
    directory_entry ent(file);
    permissions(dir, perms::none);
    std::error_code ec = GetTestEC();
    ent.replace_filename(file_two.filename(), ec);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));
  }
  permissions(dir, old_perms);
  {
    directory_entry ent(sym_in_dir);
    permissions(dir, perms::none);
    std::error_code ec = GetTestEC();
    ent.replace_filename(sym_in_dir_two.filename(), ec);
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));
  }
  permissions(dir, old_perms);
  {
    directory_entry ent(sym_out_of_dir);
    permissions(dir, perms::none);
    std::error_code ec = GetTestEC();
    ent.replace_filename(sym_out_of_dir_two.filename(), ec);
    TEST_CHECK(!ec);
    TEST_CHECK(ent.is_symlink());
    ec = GetTestEC();
    TEST_CHECK(!ent.exists(ec));
    TEST_CHECK(ErrorIs(ec, std::errc::permission_denied));
  }
}

TEST_SUITE_END()
