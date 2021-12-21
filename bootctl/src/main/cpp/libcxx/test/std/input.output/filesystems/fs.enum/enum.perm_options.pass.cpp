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

// enum class perm_options;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>
#include <sys/stat.h>

#include "test_macros.h"
#include "check_bitmask_types.hpp"


constexpr fs::perm_options ME(int val) {
  return static_cast<fs::perm_options>(val);
}

int main() {
  typedef fs::perm_options E;
  static_assert(std::is_enum<E>::value, "");

  // Check that E is a scoped enum by checking for conversions.
  typedef std::underlying_type<E>::type UT;
  static_assert(!std::is_convertible<E, UT>::value, "");

  static_assert(std::is_same<UT, unsigned char >::value, ""); // Implementation detail

  typedef check_bitmask_type<E, E::replace, E::nofollow> BitmaskTester;
  assert(BitmaskTester::check());

  static_assert(
        E::replace  == ME(1) &&
        E::add      == ME(2) &&
        E::remove   == ME(4) &&
        E::nofollow == ME(8),
        "Expected enumeration values do not match");
}
