// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that _LIBCPP_NODISCARD_EXT and _LIBCPP_NODISCARD_AFTER_CXX17 are defined
// to the appropriate warning-generating attribute when _LIBCPP_ENABLE_NODISCARD
// is explicitly provided.

// UNSUPPORTED: c++98, c++03

// GCC 7 is the first version to introduce [[nodiscard]]
// UNSUPPORTED: gcc-4.9, gcc-5, gcc-6

// MODULES_DEFINES: _LIBCPP_ENABLE_NODISCARD
#define _LIBCPP_ENABLE_NODISCARD

#include <__config>

_LIBCPP_NODISCARD_EXT int foo() { return 42; }
_LIBCPP_NODISCARD_AFTER_CXX17 int bar() { return 42; }

int main() {
  foo(); // expected-error-re {{ignoring return value of function declared with {{'nodiscard'|warn_unused_result}} attribute}}
  bar(); // expected-error-re {{ignoring return value of function declared with {{'nodiscard'|warn_unused_result}} attribute}}
  (void)foo(); // OK. void casts disable the diagnostic.
  (void)bar();
}
