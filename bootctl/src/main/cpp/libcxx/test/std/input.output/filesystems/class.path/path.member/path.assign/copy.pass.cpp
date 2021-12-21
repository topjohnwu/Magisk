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

// path& operator=(path const&);

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"


int main() {
  using namespace fs;
  static_assert(std::is_copy_assignable<path>::value, "");
  static_assert(!std::is_nothrow_copy_assignable<path>::value, "should not be noexcept");
  const std::string s("foo");
  const path p(s);
  path p2;
  path& pref = (p2 = p);
  assert(p.native() == s);
  assert(p2.native() == s);
  assert(&pref == &p2);
}
