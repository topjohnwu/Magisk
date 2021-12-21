//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <string_view>

// Test that <string_view> provides all of the arithmetic, enum, and pointer
// hash specializations.

#include <string_view>

#include "poisoned_hash_helper.hpp"

int main() {
  test_library_hash_specializations_available();
  {
    test_hash_enabled_for_type<std::string_view>();
    test_hash_enabled_for_type<std::wstring_view>();
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test_hash_enabled_for_type<std::u8string_view>();
#endif
#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    test_hash_enabled_for_type<std::u16string_view>();
    test_hash_enabled_for_type<std::u32string_view>();
#endif
  }
}
