//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test fails on Windows because the underlying libc headers on Windows
// are not modular
// XFAIL: LIBCXX-WINDOWS-FIXME

// REQUIRES: modules-support
// UNSUPPORTED: c++98, c++03

// RUN: %build_module

#include <clocale>

#define TEST(...) do { using T = decltype( __VA_ARGS__ ); } while(false)

int main() {
  std::lconv l; ((void)l);

  TEST(std::setlocale(0, ""));
  TEST(std::localeconv());
}
