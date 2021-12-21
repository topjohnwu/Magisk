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

// class path;
// enum class format;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main() {
  typedef fs::path::format E;
  static_assert(std::is_enum<E>::value, "");

  // Check that E is a scoped enum by checking for conversions.
  typedef std::underlying_type<E>::type UT;
  static_assert(!std::is_convertible<E, UT>::value, "");

  LIBCPP_ONLY(static_assert(std::is_same<UT, unsigned char>::value, "")); // Implementation detail

  static_assert(
          E::auto_format   != E::native_format &&
          E::auto_format   != E::generic_format &&
          E::native_format != E::generic_format,
        "Expected enumeration values are not unique");
}
