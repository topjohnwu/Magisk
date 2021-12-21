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

// explicit file_status() noexcept;
// explicit file_status(file_type, perms prms = perms::unknown) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_convertible.hpp"


int main() {
 using namespace fs;
  // Default ctor
  {
    static_assert(std::is_nothrow_default_constructible<file_status>::value,
                  "The default constructor must be noexcept");
    static_assert(test_convertible<file_status>(),
                  "The default constructor must not be explicit");
    const file_status f;
    assert(f.type()  == file_type::none);
    assert(f.permissions() == perms::unknown);
  }

  // Unary ctor
  {
    static_assert(std::is_nothrow_constructible<file_status, file_type>::value,
                  "This constructor must be noexcept");
    static_assert(!test_convertible<file_status, file_type>(),
                 "This constructor must be explicit");

    const file_status f(file_type::not_found);
    assert(f.type()  == file_type::not_found);
    assert(f.permissions() == perms::unknown);
  }
  // Binary ctor
  {
    static_assert(std::is_nothrow_constructible<file_status, file_type, perms>::value,
                  "This constructor must be noexcept");
    static_assert(!test_convertible<file_status, file_type, perms>(),
                  "This constructor must b explicit");
    const file_status f(file_type::regular, perms::owner_read);
    assert(f.type()  == file_type::regular);
    assert(f.permissions() == perms::owner_read);
  }
}
