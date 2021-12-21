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

// class file_status

// file_type type() const noexcept;
// perms permissions(p) const noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>


int main() {
  using namespace fs;

  const file_status st(file_type::regular, perms::owner_read);

  // type test
  {
    static_assert(noexcept(st.type()),
                  "operation must be noexcept");
    static_assert(std::is_same<decltype(st.type()), file_type>::value,
                 "operation must return file_type");
    assert(st.type() == file_type::regular);
  }
  // permissions test
  {
    static_assert(noexcept(st.permissions()),
                  "operation must be noexcept");
    static_assert(std::is_same<decltype(st.permissions()), perms>::value,
                 "operation must return perms");
    assert(st.permissions() == perms::owner_read);
  }
}
