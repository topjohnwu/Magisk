//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <vector>

// Test that <vector> provides all of the arithmetic, enum, and pointer
// hash specializations.

#include <vector>

#include "poisoned_hash_helper.hpp"
#include "min_allocator.h"

int main() {
  test_library_hash_specializations_available();
  {
    test_hash_enabled_for_type<std::vector<bool> >();
    test_hash_enabled_for_type<std::vector<bool, min_allocator<bool>>>();
  }
}
