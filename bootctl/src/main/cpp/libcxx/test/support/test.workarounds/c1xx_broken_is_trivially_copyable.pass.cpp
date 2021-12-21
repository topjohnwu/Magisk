//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// This workaround option is specific to MSVC's C1XX, so we don't care that
// it isn't set for older GCC versions.
// XFAIL: gcc-4.9

// Verify TEST_WORKAROUND_C1XX_BROKEN_IS_TRIVIALLY_COPYABLE.

#include <type_traits>

#include "test_workarounds.h"

struct S {
  S(S const&) = default;
  S(S&&) = default;
  S& operator=(S const&) = delete;
  S& operator=(S&&) = delete;
};

int main() {
#if defined(TEST_WORKAROUND_C1XX_BROKEN_IS_TRIVIALLY_COPYABLE)
  static_assert(!std::is_trivially_copyable<S>::value, "");
#else
  static_assert(std::is_trivially_copyable<S>::value, "");
#endif
}
