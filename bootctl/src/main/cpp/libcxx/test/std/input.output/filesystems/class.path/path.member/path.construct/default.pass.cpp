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

// path() noexcept

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"


int main() {
  using namespace fs;
  static_assert(std::is_nothrow_default_constructible<path>::value, "");
  const path p;
  assert(p.empty());
}
