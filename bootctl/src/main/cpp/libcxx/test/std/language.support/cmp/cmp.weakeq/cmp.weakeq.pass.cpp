//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <compare>

// class weak_equality


#include <compare>
#include <cassert>
#include "test_macros.h"

const volatile void* volatile sink;

void test_static_members() {
  DoNotOptimize(&std::weak_equality::equivalent);
  DoNotOptimize(&std::weak_equality::nonequivalent);
}

void test_signatures() {
  auto& Eq = std::weak_equality::equivalent;

  ASSERT_NOEXCEPT(Eq == 0);
  ASSERT_NOEXCEPT(0 == Eq);
  ASSERT_NOEXCEPT(Eq != 0);
  ASSERT_NOEXCEPT(0 != Eq);
#ifndef TEST_HAS_NO_SPACESHIP_OPERATOR
  ASSERT_NOEXCEPT(0 <=> Eq);
  ASSERT_NOEXCEPT(Eq <=> 0);
  ASSERT_SAME_TYPE(decltype(Eq <=> 0), std::weak_equality);
  ASSERT_SAME_TYPE(decltype(0 <=> Eq), std::weak_equality);
#endif
}

constexpr bool test_constexpr() {
  auto& Eq = std::weak_equality::equivalent;
  auto& NEq = std::weak_equality::nonequivalent;
  assert((Eq == 0) == true);
  assert((0 == Eq) == true);
  assert((NEq == 0) == false);
  assert((0 == NEq) == false);

  assert((Eq != 0) == false);
  assert((0 != Eq) == false);
  assert((NEq != 0) == true);
  assert((0 != NEq) == true);

#ifndef TEST_HAS_NO_SPACESHIP_OPERATOR
  std::weak_equality res = (Eq <=> 0);
  ((void)res);
  res = (0 <=> Eq);
  ((void)res);
#endif

  return true;
}

int main() {
  test_static_members();
  test_signatures();
  static_assert(test_constexpr(), "constexpr test failed");
}
