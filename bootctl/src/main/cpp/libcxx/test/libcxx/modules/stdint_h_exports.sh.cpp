//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: modules-support

// Test that int8_t and the like are exported from stdint.h not inttypes.h

// RUN: %build_module

#include <stdint.h>

int main() {
  int8_t x; ((void)x);
}
