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

// const string_type& native() const noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "filesystem_test_helper.hpp"


int main()
{
  using namespace fs;
  const char* const value = "hello world";
  { // Check signature
    path p(value);
    ASSERT_SAME_TYPE(path::string_type const&, decltype(p.native()));
    ASSERT_NOEXCEPT(p.native());
  }
  { // native() is tested elsewhere
    path p(value);
    assert(p.native() == value);
  }
}
