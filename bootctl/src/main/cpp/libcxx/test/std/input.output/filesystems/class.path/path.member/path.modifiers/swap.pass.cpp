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

// void swap(path& rhs) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"


struct SwapTestcase {
  const char* value1;
  const char* value2;
};

#define LONG_STR1 "_THIS_IS_LONG_THIS_IS_LONG_THIS_IS_LONG_THIS_IS_LONG_THIS_IS_LONG_THIS_IS_LONG_THIS_IS_LONG"
#define LONG_STR2 "_THIS_IS_LONG2_THIS_IS_LONG2_THIS_IS_LONG2_THIS_IS_LONG2_THIS_IS_LONG2_THIS_IS_LONG2_THIS_IS_LONG2"
const SwapTestcase TestCases[] =
  {
      {"", ""}
    , {"shortstr", LONG_STR1}
    , {LONG_STR1, "shortstr"}
    , {LONG_STR1, LONG_STR2}
  };
#undef LONG_STR1
#undef LONG_STR2

int main()
{
  using namespace fs;
  {
    path p;
    ASSERT_NOEXCEPT(p.swap(p));
    ASSERT_SAME_TYPE(void, decltype(p.swap(p)));
  }
  for (auto const & TC : TestCases) {
    path p1(TC.value1);
    path p2(TC.value2);
    {
      DisableAllocationGuard g;
      p1.swap(p2);
    }
    assert(p1 == TC.value2);
    assert(p2 == TC.value1);
    {
      DisableAllocationGuard g;
      p1.swap(p2);
    }
    assert(p1 == TC.value1);
    assert(p2 == TC.value2);
  }
  // self-swap
  {
    const char* Val = "aoeuaoeuaoeuaoeuaoeuaoeuaoeuaoeuaoeu";
    path p1(Val);
    assert(p1 == Val);
    {
      DisableAllocationGuard g;
      p1.swap(p1);
    }
    assert(p1 == Val);
  }
}
