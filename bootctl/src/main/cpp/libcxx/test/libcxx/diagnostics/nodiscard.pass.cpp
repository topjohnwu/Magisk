// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that _LIBCPP_NODISCARD_EXT is not defined to [[nodiscard]] unless
// explicitly enabled by _LIBCPP_ENABLE_NODISCARD

#include <__config>

_LIBCPP_NODISCARD_EXT int foo() { return 42; }

int main() {
  foo(); // OK.
}
