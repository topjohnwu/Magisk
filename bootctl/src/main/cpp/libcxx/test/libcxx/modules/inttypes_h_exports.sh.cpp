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

// Test that intypes.h re-exports stdint.h

// RUN: %build_module

#include <inttypes.h>

int main() {
  int8_t x; ((void)x);
}
