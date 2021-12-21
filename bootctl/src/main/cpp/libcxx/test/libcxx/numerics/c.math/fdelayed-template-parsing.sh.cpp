//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test that cmath builds with -fdelayed-template-parsing

// REQUIRES: fdelayed-template-parsing

// RUN: %build -fdelayed-template-parsing
// RUN: %run

#include <cmath>
#include <cassert>

#include "test_macros.h"

int main() {
  assert(std::isfinite(1.0));
  assert(!std::isinf(1.0));
  assert(!std::isnan(1.0));
}

using namespace std;
