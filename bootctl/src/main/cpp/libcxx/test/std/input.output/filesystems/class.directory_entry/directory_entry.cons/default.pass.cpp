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

// class directory_entry

//          directory_entry() noexcept = default;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

int main() {
  using namespace fs;
  // Default
  {
    static_assert(std::is_nothrow_default_constructible<directory_entry>::value,
                  "directory_entry must have a nothrow default constructor");
    directory_entry e;
    assert(e.path() == path());
  }
}
