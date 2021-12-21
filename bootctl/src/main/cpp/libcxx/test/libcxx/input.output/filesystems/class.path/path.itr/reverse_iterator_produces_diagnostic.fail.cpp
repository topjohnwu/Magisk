//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// class path

#include "filesystem_include.hpp"
#include <iterator>


int main() {
  using namespace fs;
  using RIt = std::reverse_iterator<path::iterator>;

  // expected-error-re@iterator:* {{static_assert failed{{.*}} "The specified iterator type cannot be used with reverse_iterator; Using stashing iterators with reverse_iterator causes undefined behavior"}}
  {
    RIt r;
    ((void)r);
  }
}
