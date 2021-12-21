// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// GCC 7 is the first version to introduce [[nodiscard]]
// UNSUPPORTED: gcc-4.9, gcc-5, gcc-6

// Test that _LIBCPP_DISABLE_NODISCARD_EXT only disables _LIBCPP_NODISCARD_EXT
// and not _LIBCPP_NODISCARD_AFTER_CXX17.

// MODULES_DEFINES: _LIBCPP_ENABLE_NODISCARD
// MODULES_DEFINES: _LIBCPP_DISABLE_NODISCARD_AFTER_CXX17
#define _LIBCPP_ENABLE_NODISCARD
#define _LIBCPP_DISABLE_NODISCARD_AFTER_CXX17
#include <__config>


_LIBCPP_NODISCARD_EXT int foo() { return 42; }
_LIBCPP_NODISCARD_AFTER_CXX17 int bar() { return 42; }

int main() {
  foo(); // expected-error-re {{ignoring return value of function declared with {{'nodiscard'|warn_unused_result}} attribute}}
  bar(); // OK.
  (void)foo(); // OK.
}
