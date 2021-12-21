//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <variant>

// Test that <variant> provides all of the arithmetic, enum, and pointer
// hash specializations.

#include <variant>

#include "poisoned_hash_helper.hpp"

int main() {
  test_library_hash_specializations_available();
}
