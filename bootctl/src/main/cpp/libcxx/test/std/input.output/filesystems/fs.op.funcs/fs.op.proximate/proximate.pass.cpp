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

// path proximate(const path& p, error_code &ec)
// path proximate(const path& p, const path& base = current_path())
// path proximate(const path& p, const path& base, error_code& ec);

#include "filesystem_include.hpp"
#include <type_traits>
#include <vector>
#include <iostream>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"


static int count_path_elems(const fs::path& p) {
  int count = 0;
  for (auto& elem : p) {
    if (elem != "/" && elem != "")
      ++count;
  }
  return count;
}

TEST_SUITE(filesystem_proximate_path_test_suite)


TEST_CASE(signature_test)
{
    using fs::path;
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(proximate(p));
    ASSERT_NOT_NOEXCEPT(proximate(p, p));
    ASSERT_NOT_NOEXCEPT(proximate(p, ec));
    ASSERT_NOT_NOEXCEPT(proximate(p, p, ec));
}

TEST_CASE(basic_test) {
  using fs::path;
  const path cwd = fs::current_path();
  const path parent_cwd = cwd.parent_path();
  const path curdir = cwd.filename();
  TEST_REQUIRE(!cwd.native().empty());
  int cwd_depth = count_path_elems(cwd);
  path dot_dot_to_root;
  for (int i=0; i < cwd_depth; ++i)
    dot_dot_to_root /= "..";
  path relative_cwd = cwd.native().substr(1);
  // clang-format off
  struct {
    std::string input;
    std::string base;
    std::string expect;
  } TestCases[] = {
      {"", "", "."},
      {cwd, "a", ".."},
      {parent_cwd, "a", "../.."},
      {"a", cwd, "a"},
#if !defined(__ANDROID__)
      // Android tests are run via a RemoteExecutor, where the parent directory
      // is a temporary directory with no fixed name.
      {"a", parent_cwd, "fs.op.proximate/a"},
#endif
      {"/", "a", dot_dot_to_root / ".."},
      {"/", "a/b", dot_dot_to_root / "../.."},
      {"/", "a/b/", dot_dot_to_root / "../.."},
      {"a", "/", relative_cwd / "a"},
      {"a/b", "/", relative_cwd / "a/b"},
      {"a", "/net", ".." / relative_cwd / "a"},
      {"//foo/", "//foo", "."},
      {"//foo", "//foo/", "."},
      {"//foo", "//foo", "."},
      {"//foo/", "//foo/", "."},
      {"//base", "a", dot_dot_to_root / "../base"},
      {"a", "a", "."},
      {"a/b", "a/b", "."},
      {"a/b/c/", "a/b/c/", "."},
      {"//foo/a/b", "//foo/a/b", "."},
      {"/a/d", "/a/b/c", "../../d"},
      {"/a/b/c", "/a/d", "../b/c"},
      {"a/b/c", "a", "b/c"},
      {"a/b/c", "a/b/c/x/y", "../.."},
      {"a/b/c", "a/b/c", "."},
      {"a/b", "c/d", "../../a/b"}
  };
  // clang-format on
  int ID = 0;
  for (auto& TC : TestCases) {
    ++ID;
    std::error_code ec = GetTestEC();
    fs::path p(TC.input);
    const fs::path output = fs::proximate(p, TC.base, ec);
    if (ec) {
      TEST_CHECK(!ec);
      std::cerr << "TEST CASE #" << ID << " FAILED: \n";
      std::cerr << "  Input: '" << TC.input << "'\n";
      std::cerr << "  Base: '" << TC.base << "'\n";
      std::cerr << "  Expected: '" << TC.expect << "'\n";

      std::cerr << std::endl;
    } else if (!PathEq(output, TC.expect)) {
      TEST_CHECK(PathEq(output, TC.expect));

      const path canon_input = fs::weakly_canonical(TC.input);
      const path canon_base = fs::weakly_canonical(TC.base);
      const path lexically_p = canon_input.lexically_proximate(canon_base);
      std::cerr << "TEST CASE #" << ID << " FAILED: \n";
      std::cerr << "  Input: '" << TC.input << "'\n";
      std::cerr << "  Base: '" << TC.base << "'\n";
      std::cerr << "  Expected: '" << TC.expect << "'\n";
      std::cerr << "  Output:   '" << output.native() << "'\n";
      std::cerr << "  Lex Prox: '" << lexically_p.native() << "'\n";
      std::cerr << "  Canon Input: " <<  canon_input << "\n";
      std::cerr << "  Canon Base: " << canon_base << "\n";

      std::cerr << std::endl;
    }
  }
}

TEST_SUITE_END()
