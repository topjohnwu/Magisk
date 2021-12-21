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

// directory_entry(const directory_entry&) = default;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"
#include "test_convertible.hpp"

TEST_SUITE(directory_entry_path_ctor_suite)

TEST_CASE(copy_ctor) {
  using namespace fs;
  // Copy
  {
    static_assert(std::is_copy_constructible<directory_entry>::value,
                  "directory_entry must be copy constructible");
    static_assert(!std::is_nothrow_copy_constructible<directory_entry>::value,
                  "directory_entry's copy constructor cannot be noexcept");
    const path p("foo/bar/baz");
    const directory_entry e(p);
    assert(e.path() == p);
    directory_entry e2(e);
    assert(e.path() == p);
    assert(e2.path() == p);
  }
}

TEST_CASE(copy_ctor_copies_cache) {
  using namespace fs;
  scoped_test_env env;
  const path dir = env.create_dir("dir");
  const path file = env.create_file("dir/file", 42);
  const path sym = env.create_symlink("dir/file", "sym");

  {
    directory_entry ent(sym);

    fs::remove(sym);

    directory_entry ent_cp(ent);
    TEST_CHECK(ent_cp.path() == sym);
    TEST_CHECK(ent_cp.is_symlink());
  }

  {
    directory_entry ent(file);

    fs::remove(file);

    directory_entry ent_cp(ent);
    TEST_CHECK(ent_cp.path() == file);
    TEST_CHECK(ent_cp.is_regular_file());
  }
}

TEST_SUITE_END()
