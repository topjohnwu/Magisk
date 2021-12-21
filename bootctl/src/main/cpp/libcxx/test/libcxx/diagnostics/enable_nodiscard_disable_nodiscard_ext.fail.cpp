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


// MODULES_DEFINES: _LIBCPP_ENABLE_NODISCARD
// MODULES_DEFINES: _LIBCPP_DISABLE_NODISCARD_EXT
#define _LIBCPP_ENABLE_NODISCARD
#define _LIBCPP_DISABLE_NODISCARD_EXT
#include <__config>


_LIBCPP_NODISCARD_EXT int foo() { return 42; }
_LIBCPP_NODISCARD_AFTER_CXX17 int bar() { return 42; }

int main() {
  bar(); // expected-error-re {{ignoring return value of function declared with {{'nodiscard'|warn_unused_result}} attribute}}
  foo(); // OK.
  (void)bar(); // OK.
}
