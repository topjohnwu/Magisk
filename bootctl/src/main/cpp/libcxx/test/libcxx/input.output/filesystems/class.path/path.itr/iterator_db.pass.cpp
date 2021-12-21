//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03
// UNSUPPORTED: libcpp-no-exceptions

// MODULES_DEFINES: _LIBCPP_DEBUG_USE_EXCEPTIONS
// MODULES_DEFINES: _LIBCPP_DEBUG=0

// <filesystem>

// class path

#define _LIBCPP_DEBUG 0
#define _LIBCPP_DEBUG_USE_EXCEPTIONS
#include "filesystem_include.hpp"
#include <iterator>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "filesystem_test_helper.hpp"

int main() {
  using namespace fs;
  using ExType = std::__libcpp_debug_exception;
  // Test incrementing/decrementing a singular iterator
  {
    path::iterator singular;
    try {
      ++singular;
      assert(false);
    } catch (ExType const&) {}
    try {
      --singular;
      assert(false);
    } catch (ExType const&) {}
  }
  // Test decrementing the begin iterator
  {
    path p("foo/bar");
    auto it = p.begin();
    try {
      --it;
      assert(false);
    } catch (ExType const&) {}
    ++it;
    ++it;
    try {
      ++it;
      assert(false);
    } catch (ExType const&) {}
  }
  // Test incrementing the end iterator
  {
    path p("foo/bar");
    auto it = p.end();
    try {
      ++it;
      assert(false);
    } catch (ExType const&) {}
    --it;
    --it;
    try {
      --it;
      assert(false);
    } catch (ExType const&) {}
  }
}
