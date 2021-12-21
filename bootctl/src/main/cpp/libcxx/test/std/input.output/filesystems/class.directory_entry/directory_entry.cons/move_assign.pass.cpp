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

TEST_SUITE(directory_entry_ctor_suite)

TEST_CASE(test_move_assign_operator) {
  using namespace fs;
  // Copy
  {
    static_assert(std::is_nothrow_move_assignable<directory_entry>::value,
                  "directory_entry is noexcept move assignable");
    const path p("foo/bar/baz");
    const path p2("abc");
    directory_entry e(p);
    directory_entry e2(p2);
    assert(e.path() == p && e2.path() == p2);
    e2 = std::move(e);
    assert(e2.path() == p);
    assert(e.path() != p); // testing moved from state
  }
}

TEST_CASE(move_assign_copies_cache) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path sym = env.create_symlink("dir/file", "sym");

  {
    directory_entry ent(sym);

    fs::remove(sym);

    directory_entry ent_cp;
    ent_cp = std::move(ent);
    TEST_CHECK(ent_cp.path() == sym);
    TEST_CHECK(ent_cp.is_symlink());
  }

  {
    directory_entry ent(file);

    fs::remove(file);

    directory_entry ent_cp;
    ent_cp = std::move(ent);
    TEST_CHECK(ent_cp.path() == file);
    TEST_CHECK(ent_cp.is_regular_file());
  }
}

TEST_SUITE_END()
