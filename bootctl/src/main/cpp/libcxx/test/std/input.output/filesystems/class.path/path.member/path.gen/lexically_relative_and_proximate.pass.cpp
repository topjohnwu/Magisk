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

// class path

// path lexically_relative(const path& p) const;
// path lexically_proximate(const path& p) const;

#include "filesystem_include.hpp"
#include <type_traits>
#include <vector>
#include <iostream>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"


int main() {
  // clang-format off
  struct {
    std::string input;
    std::string base;
    std::string expect;
  } TestCases[] = {
      {"", "", "."},
      {"/", "a", ""},
      {"a", "/", ""},
      {"//net", "a", ""},
      {"a", "//net", ""},
      {"//net/", "//net", "."},
      {"//net", "//net/", "."},
      {"//base", "a", ""},
      {"a", "a", "."},
      {"a/b", "a/b", "."},
      {"a/b/c/", "a/b/c/", "."},
      {"//net", "//net", "."},
      {"//net/", "//net/", "."},
      {"//net/a/b", "//net/a/b", "."},
      {"/a/d", "/a/b/c", "../../d"},
      {"/a/b/c", "/a/d", "../b/c"},
      {"a/b/c", "a", "b/c"},
      {"a/b/c", "a/b/c/x/y", "../.."},
      {"a/b/c", "a/b/c", "."},
      {"a/b", "c/d", "../../a/b"}
  };
  // clang-format on
  int ID = 0;
  bool Failed = false;
  for (auto& TC : TestCases) {
    ++ID;
    const fs::path p(TC.input);
    const fs::path output = p.lexically_relative(TC.base);
    auto ReportErr = [&](const char* Testing, fs::path const& Output,
                                              fs::path const& Expected) {
      Failed = true;
      std::cerr << "TEST CASE #" << ID << " FAILED: \n";
      std::cerr << "  Testing: " << Testing << "\n";
      std::cerr << "  Input: '" << TC.input << "'\n";
      std::cerr << "  Base: '" << TC.base << "'\n";
      std::cerr << "  Expected: '" << Expected << "'\n";
      std::cerr << "  Output: '" << Output.native() << "'";
      std::cerr << std::endl;
    };
    if (!PathEq(output, TC.expect))
      ReportErr("path::lexically_relative", output, TC.expect);
    const fs::path proximate_output = p.lexically_proximate(TC.base);
    // [path.gen] lexically_proximate
    // Returns: If the value of lexically_relative(base) is not an empty path,
    // return it.Otherwise return *this.
    const fs::path proximate_expected = output.native().empty() ? p
        : output;
    if (!PathEq(proximate_expected, proximate_output))
      ReportErr("path::lexically_proximate", proximate_output, proximate_expected);
  }
  return Failed;
}
