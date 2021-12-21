
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

// operator string_type() const;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "filesystem_test_helper.hpp"


int main()
{
  using namespace fs;
  using string_type = path::string_type;
  const char* const value = "hello world";
  { // Check signature
    path p(value);
    static_assert(std::is_convertible<path, string_type>::value, "");
    static_assert(std::is_constructible<string_type, path>::value, "");
    ASSERT_SAME_TYPE(string_type, decltype(p.operator string_type()));
    ASSERT_NOT_NOEXCEPT(p.operator string_type());
  }
  {
    path p(value);
    assert(p.native() == value);
    string_type s = p;
    assert(s == value);
    assert(p == value);
  }
}
