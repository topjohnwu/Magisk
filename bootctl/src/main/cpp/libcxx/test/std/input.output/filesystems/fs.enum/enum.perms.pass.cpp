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

// enum class perms;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>
#include <sys/stat.h>

#include "test_macros.h"
#include "check_bitmask_types.hpp"


constexpr fs::perms ME(int val) { return static_cast<fs::perms>(val); }

int main() {
  typedef fs::perms E;
  static_assert(std::is_enum<E>::value, "");

  // Check that E is a scoped enum by checking for conversions.
  typedef std::underlying_type<E>::type UT;
  static_assert(!std::is_convertible<E, UT>::value, "");

  static_assert(std::is_same<UT, unsigned >::value, ""); // Implementation detail

  typedef check_bitmask_type<E, E::group_all, E::owner_all> BitmaskTester;
  assert(BitmaskTester::check());

  static_assert(
        E::none         == ME(0) &&

        E::owner_read   == ME(0400) &&
        E::owner_write  == ME(0200) &&
        E::owner_exec   == ME(0100) &&
        E::owner_all    == ME(0700) &&

        E::group_read   == ME(040) &&
        E::group_write  == ME(020) &&
        E::group_exec   == ME(010) &&
        E::group_all    == ME(070) &&

        E::others_read  == ME(04) &&
        E::others_write == ME(02) &&
        E::others_exec  == ME(01) &&
        E::others_all   == ME(07) &&
        E::all          == ME(0777) &&
        E::set_uid      == ME(04000) &&
        E::set_gid      == ME(02000) &&
        E::sticky_bit   == ME(01000) &&
        E::mask         == ME(07777) &&
        E::unknown      == ME(0xFFFF),
        "Expected enumeration values do not match");
}
