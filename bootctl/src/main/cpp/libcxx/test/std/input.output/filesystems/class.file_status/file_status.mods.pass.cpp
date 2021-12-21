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

// void type(file_type) noexcept;
// void permissions(perms) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>


int main() {
  using namespace fs;

  file_status st;

  // type test
  {
    static_assert(noexcept(st.type(file_type::regular)),
                  "operation must be noexcept");
    static_assert(std::is_same<decltype(st.type(file_type::regular)), void>::value,
                 "operation must return void");
    assert(st.type() != file_type::regular);
    st.type(file_type::regular);
    assert(st.type() == file_type::regular);
  }
  // permissions test
  {
    static_assert(noexcept(st.permissions(perms::owner_read)),
                  "operation must be noexcept");
    static_assert(std::is_same<decltype(st.permissions(perms::owner_read)), void>::value,
                 "operation must return void");
    assert(st.permissions() != perms::owner_read);
    st.permissions(perms::owner_read);
    assert(st.permissions() == perms::owner_read);
  }
}
