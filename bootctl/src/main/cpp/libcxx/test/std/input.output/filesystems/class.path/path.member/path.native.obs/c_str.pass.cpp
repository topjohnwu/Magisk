
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

// const value_type* c_str() const noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "filesystem_test_helper.hpp"


int main()
{
  using namespace fs;
  const char* const value = "hello world";
  const std::string str_value = value;
  { // Check signature
    path p(value);
    ASSERT_SAME_TYPE(path::value_type const*, decltype(p.c_str()));
    ASSERT_NOEXCEPT(p.c_str());
  }
  {
    path p(value);
    assert(p.c_str() == str_value);
    assert(p.native().c_str() == p.c_str());
  }
}
