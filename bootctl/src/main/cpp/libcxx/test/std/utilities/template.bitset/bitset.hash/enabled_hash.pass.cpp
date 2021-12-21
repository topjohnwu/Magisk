//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <bitset>

// Test that <bitset> provides all of the arithmetic, enum, and pointer
// hash specializations.

#include <bitset>

#include "poisoned_hash_helper.hpp"

int main() {
  test_library_hash_specializations_available();
  {
    test_hash_enabled_for_type<std::bitset<0> >();
    test_hash_enabled_for_type<std::bitset<1> >();
    test_hash_enabled_for_type<std::bitset<1024> >();
    test_hash_enabled_for_type<std::bitset<100000> >();
  }
}
