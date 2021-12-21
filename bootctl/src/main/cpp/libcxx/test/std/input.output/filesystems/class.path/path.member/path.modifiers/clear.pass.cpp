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

// void clear() noexcept

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"


int main() {
  using namespace fs;
  {
    path p;
    ASSERT_NOEXCEPT(p.clear());
    ASSERT_SAME_TYPE(void, decltype(p.clear()));
    p.clear();
    assert(p.empty());
  }
  {
    const path p("/foo/bar/baz");
    path p2(p);
    assert(p == p2);
    p2.clear();
    assert(p2.empty());
  }
}
